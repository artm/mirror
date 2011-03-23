#include "facetracker.h"
#include "scratchgraphics.h"
#include "ui_mirrorwindow.h"
#include "util.h"

#include <opencv2/imgproc/imgproc.hpp>

namespace Mirror {

FaceTracker::FaceTracker(Mirror::CompositeView * canvas, QObject *parent)
    : Mirror::VisionFilter(canvas, parent)
    , m_eyeBrush( QColor(255, 200, 200, 128) )
    , m_scale(0.5)
{
    QGraphicsScene *  scene = canvas->scene();
    m_faceGfx = new Mirror::ScratchGraphics();
    scene->addItem( m_faceGfx );

    m_faceDetector.load(
                findResourceFile("lbpcascades/lbpcascade_frontalface.xml")
                .toStdString());
    m_lEyeDetector.load(
                findResourceFile("haarcascades/haarcascade_mcs_lefteye.xml")
                .toStdString());
    m_rEyeDetector.load(
                findResourceFile("haarcascades/haarcascade_mcs_righteye.xml")
                .toStdString());

    appendVideoSlot("greyscale", &m_grey);
    appendVideoSlot("downsampled", &m_scaled);
    appendVideoSlot("contrast-stretched", &m_stretched);
}

void FaceTracker::configureGUI(Ui::MirrorWindow * ui)
{
    QComboBox * scaleSelector = new QComboBox();
    QFormLayout * layout = dynamic_cast<QFormLayout *>(ui->mirrorDockContents->layout());
    layout->addRow("Downsample", scaleSelector);
    connect( scaleSelector, SIGNAL(activated(int)), SLOT(onScaleSelected(int)) );
    QStringList items = (QStringList() << "none" << "x 1/2" << "x 1/4" << "x 1/8");
    scaleSelector->addItems(items);
    scaleSelector->setCurrentIndex(1);
}

void FaceTracker::filter(const cv::Mat& frame)
{
    // grey will have the same dimensions as input
    cv::cvtColor( frame, m_grey, CV_RGB2GRAY);

    if (fabs(m_scale - 1.0) > 1e-2)
        cv::resize( m_grey, m_scaled, cv::Size(0,0), m_scale, m_scale );
    else
        m_scaled = m_grey;
    cv::normalize( m_scaled, m_stretched, 0, 255, cv::NORM_MINMAX );

    std::vector<cv::Rect> rects;
    float smallest = 40.0 * m_scale;
    m_faceDetector.detectMultiScale( m_stretched, rects, 1.1, 3, 0, cv::Size(smallest, smallest));

    // remove old face rects
    m_faceGfx->clear();

    // insert new face rects. At the same time detect eyes...
    foreach(cv::Rect r, rects) {

        r.x /= m_scale;
        r.y /= m_scale;
        r.width /= m_scale;
        r.height /= m_scale;

        m_faceGfx->addRect(r);
        // where should look for eyes?
        float eyeAreaWidth = r.width / 3.0;
        float eyeAreaHeight = r.height / 4.0;

        // detect eyes
        std::vector<cv::Rect> lEyes, rEyes;
        detectEye(m_lEyeDetector, cv::Rect(r.x + 0.5 * eyeAreaWidth, r.y + eyeAreaHeight, eyeAreaWidth, eyeAreaHeight), lEyes);
        detectEye(m_rEyeDetector, cv::Rect(r.x + 1.5 * eyeAreaWidth, r.y + eyeAreaHeight, eyeAreaWidth, eyeAreaHeight), rEyes);

        QPointF leftEye, rightEye;
        if (bestEyePair(lEyes, rEyes, leftEye, rightEye)) {
            m_faceGfx->addLine( QLineF(leftEye, rightEye) );
        }
    }
}

void FaceTracker::detectEye(cv::CascadeClassifier& detector, const cv::Rect& roi, std::vector<cv::Rect>& rects)
{
    cv::Mat eyePatch;
    cv::normalize( cv::Mat( m_grey, roi ), eyePatch, 0, 255, cv::NORM_MINMAX );
    cv::Size smallest(eyePatch.cols / 6, eyePatch.rows / 4);
    detector.detectMultiScale( eyePatch, rects, 1.2, 2, CV_HAAR_DO_CANNY_PRUNING, smallest );
    for(unsigned i=0; i<rects.size(); ++i) {
        rects[i] += roi.tl();
        m_faceGfx->addRect(rects[i], Mirror::ScratchGraphics::defaultPen(), m_eyeBrush);
    }
}

bool FaceTracker::bestEyePair(const std::vector<cv::Rect>& l, const std::vector<cv::Rect>& r, QPointF& left, QPointF& right)
{
    if (!l.size() || !r.size())
        return false;

    double score[ l.size() ][ r.size() ];

    for(unsigned i = 0; i<l.size(); ++i)
        for(unsigned j=0; j<r.size(); ++j) {
            // prefer larger
            double lArea = l[i].width*l[i].height;
            double rArea = r[j].width*r[j].height;
            score[i][j] = lArea + rArea;
            // prefer on the same horizontal level
            score[i][j] -= fabs(l[i].x-r[j].x);
            // prefer similar size
            score[i][j] *= std::min( lArea/rArea, rArea/lArea );
        }

    int bestI = 0, bestJ = 0;
    for(unsigned i = 0; i<l.size(); ++i)
        for(unsigned j=0; j<r.size(); ++j) {
            if (score[i][j] > score[bestI][bestJ]) {
                bestI = i;
                bestJ = j;
            }
        }

    left.setX( l[bestI].x + l[bestI].width / 2 );
    left.setY( l[bestI].y + l[bestI].height / 2 );
    right.setX( r[bestJ].x + r[bestJ].width / 2 );
    right.setY( r[bestJ].y + r[bestJ].height / 2 );

    return true;
}

} // namespace Mirror
