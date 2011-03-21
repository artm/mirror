#include "footballtracker.h"
#include "ui_mirrorwindow.h"
#include "mirrorwindow.h"

#include <QtDebug>
#include <QCoreApplication>
#include <QPushButton>
#include <QKeyEvent>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace Mirror {

FootballTracker::FootballTracker( Mirror::CompositeView * canvas, QObject *parent )
    : Mirror::VisionFilter(canvas, parent), m_foundField(false)
{
    QString prefix = QCoreApplication::applicationDirPath () + "/../../../Resources/";
    m_field = cv::imread((prefix+"field-sample.png").toStdString(), 3);
    cv::cvtColor( m_field, m_field, CV_BGR2HSV );
    int channels[] = { 0, 1 }; // hue, saturation
    int histSize[] = { 30, 30 }; // quantization of hue, saturation
    float hranges[] = { 0, 180 }, sranges[] = { 0, 256 };
    const float * ranges[] = { hranges, sranges };
    cv::calcHist( &m_field,
             1, // one array
             channels,
             cv::Mat(), // do not use mask
             m_fieldHist,
             2, // dimensions
             histSize,
             ranges,
             true, // the histogram is uniform
             false // don't accumulate
             );

    appendVideoSlot("hsv",&m_hsv);
    appendVideoSlot("fieldness", &m_fieldness);
    appendVideoSlot("fieldness-thresholded", &m_fieldnessBin);
    appendVideoSlot("field", &m_field);
    appendVideoSlot("see through", &m_seeThrough);

    QGraphicsScene *  scene = canvas->scene();
    m_fieldOverlay = new Mirror::ScratchGraphics();
    scene->addItem( m_fieldOverlay );
    m_playersOverlay = new Mirror::ScratchGraphics();
    scene->addItem( m_playersOverlay );
}

void FootballTracker::filter(const cv::Mat& frame)
{
    cv::cvtColor( frame, m_hsv, CV_BGR2HSV );

    int channels[] = { 0, 1 }; // hue, saturation
    float hranges[] = { 0, 180 }, sranges[] = { 0, 256 };
    const float * ranges[] = { hranges, sranges };

    if (!m_foundField) {
        cv::calcBackProject( &m_hsv, 1, channels, m_fieldHist, m_fieldness, ranges );
        cv::threshold( m_fieldness, m_fieldnessBin, 200, 255, cv::THRESH_BINARY);

        cv::Mat kernel = (cv::Mat_<unsigned char>(3,3) << 0, 255, 0, 255, 255, 255, 0, 255, 0);
        cv::morphologyEx( m_fieldnessBin, m_fieldnessBin, cv::MORPH_OPEN, kernel, cv::Point(-1,-1), 5);
        cv::morphologyEx( m_fieldnessBin, m_fieldnessBin, cv::MORPH_CLOSE, kernel, cv::Point(-1,-1), 5);

        std::vector< std::vector< cv::Point > > contours;
        cv::Mat copy = m_fieldnessBin.clone();
        cv::findContours( copy, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        m_fieldOverlay->clear();
        int longest = 0;
        for(unsigned i = 1; i<contours.size(); ++i) {
            if (contours[i].size() > contours[longest].size())
                longest = i;
        }

        std::vector<int> hull;
        m_fieldROI = cv::boundingRect( cv::Mat( contours[longest] ) );

        const cv::Point * longestPts[] = { &(contours[longest][0]) };
        int longestLen[] = { contours[longest].size() };
        cv::fillPoly( m_fieldnessBin, longestPts, longestLen, 1, 255 );
        m_fieldMask = cv::Mat( m_fieldnessBin, m_fieldROI ).clone();
        m_fieldOverlay->addRect(m_fieldROI, m_fieldOverlay->defaultPen(), Qt::NoBrush);
        m_fieldOverlay->addContour( contours[longest], QPen(QColor(255,128,128)), Qt::NoBrush);
        m_foundField = true;
    } else {
        // we have ROI and a mask - search for little guys
        m_seeThrough = cv::Mat( frame.rows, frame.cols, CV_8UC3, 0);
        cv::Mat frameFrag(frame, m_fieldROI),
                hsvFrag(m_hsv, m_fieldROI),
                fieldnessFrag(m_fieldness, m_fieldROI),
                fieldnessBinFrag(m_fieldnessBin, m_fieldROI),
                seeThroughFrag(m_seeThrough, m_fieldROI);

        cv::calcBackProject( &hsvFrag, 1, channels, m_fieldHist, fieldnessFrag, ranges );
        cv::threshold( fieldnessFrag, fieldnessBinFrag, 200, 255, cv::THRESH_BINARY_INV);
        cv::multiply( fieldnessBinFrag, m_fieldMask, fieldnessBinFrag );

        cv::Mat kernel = (cv::Mat_<unsigned char>(3,3) << 0, 255, 0, 255, 255, 255, 0, 255, 0);
        cv::morphologyEx( fieldnessBinFrag, fieldnessBinFrag, cv::MORPH_OPEN, kernel, cv::Point(-1,-1), 2);

        kernel = (cv::Mat_<unsigned char>(3,3) << 0, 255, 0, 0, 255, 0, 0, 0, 0);
        cv::morphologyEx( fieldnessBinFrag, fieldnessBinFrag, cv::MORPH_DILATE, kernel, cv::Point(-1,-1), 5);

        // why doesn't copying ROI versions work???
        // frameFrag.copyTo(seeThroughFrag, fieldnessBinFrag);
        frame.copyTo(m_seeThrough, m_fieldnessBin);

        // find blob contours
        std::vector< std::vector< cv::Point > > contours;
        cv::findContours( m_fieldnessBin, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        m_playersOverlay->clear();
        for(unsigned i = 1; i<contours.size(); ++i) {
            m_playersOverlay->addContour( contours[i], QPen(QColor(128,128,255)), QBrush(QColor(128,128,255,100)) );
        }

    }

}

void FootballTracker::toggleOverlay()
{
    if (m_fieldOverlay->isVisible()) {
        m_fieldOverlay->hide();
        m_playersOverlay->hide();
    } else {
        m_fieldOverlay->show();
        m_playersOverlay->show();
    }
}

void FootballTracker::configureGUI(Ui::MirrorWindow * ui)
{
    QFormLayout * layout = dynamic_cast<QFormLayout *>(ui->mirrorDockContents->layout());
    Q_ASSERT(layout);

    QPushButton * butt = new QPushButton("RelearnBg");
    layout->addRow("Act!", butt);

    connect( butt, SIGNAL(clicked()), SLOT(relearnBg()) );

    MirrorWindow * mainWin = dynamic_cast<MirrorWindow*>(ui->centralWidget->topLevelWidget());
    Q_ASSERT(mainWin);
    mainWin->loadFile( QCoreApplication::applicationDirPath() + "/../../../Resources/voet1.MP4" );

    mainWin->installEventFilter(this);
}

bool FootballTracker::eventFilter(QObject * obj, QEvent * event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Space:
            relearnBg();
            return true;
        case Qt::Key_O:
            toggleOverlay();
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}

} // namespace Mirror
