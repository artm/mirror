#include <QtDebug>
#include <QFileInfo>
#include <QResizeEvent>

#include "mirrorwindow.h"
#include "ui_mirrorwindow.h"

QVector<QRgb> MirrorWindow::s_greyTable;

MirrorWindow::MirrorWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MirrorWindow)
    , m_metro(this)
    , m_eyeBrush( QColor(255, 200, 200, 128) )
    , m_videoLayer(Input)
    , m_scale(0.5)
{
    for (int i = 0; i < 256; i++){
        s_greyTable.push_back(qRgb(i, i, i));
    }

    ui->setupUi(this);

    QGraphicsScene * scene = new QGraphicsScene(ui->composite);
    ui->composite->setScene( scene );

    QPixmap p(640, 480);
    p.fill(Qt::black);
    m_videoItem = scene->addPixmap( p );

    m_faceGfx = new Mirror::CVLayer();
    scene->addItem( m_faceGfx );

    ui->composite->setSceneRect(0,0,640,480);

    QActionGroup * showGroup = new QActionGroup(this);
    showGroup->addAction(ui->actionInput);
    showGroup->addAction(ui->actionGreyscale);
    showGroup->addAction(ui->actionDownsampled);
    showGroup->addAction(ui->actionContrast);
    ui->actionInput->setChecked(true);

    connect( ui->actionCapture, SIGNAL(toggled(bool)), SLOT(setCapture(bool)) );
    connect( ui->actionInput, SIGNAL(triggered()), SLOT(showInput()) );
    connect( ui->actionGreyscale, SIGNAL(triggered()), SLOT(showGrey()) );
    connect( ui->actionDownsampled, SIGNAL(triggered()), SLOT(showScaled()) );
    connect( ui->actionContrast, SIGNAL(triggered()), SLOT(showStretched()) );
    connect( ui->actionFullscreen, SIGNAL(toggled(bool)), SLOT(setFullscreen(bool)) );

    connect( ui->scaleSelector, SIGNAL(activated(int)), SLOT(onScaleSelected(int)) );

    connect( &m_metro, SIGNAL(timeout()), SLOT(tick()) );

    m_resourcesRoot = QDir(QCoreApplication::applicationDirPath () + "/../../../Resources/");
    loadDetector(m_faceDetector, "lbpcascades/lbpcascade_frontalface.xml");
    //loadDetector(m_faceDetector, "haarcascades/haarcascade_frontalface_alt2.xml");
    loadDetector(m_lEyeDetector, "haarcascades/haarcascade_mcs_lefteye.xml");
    loadDetector(m_rEyeDetector, "haarcascades/haarcascade_mcs_righteye.xml");
    //loadDetector(m_lEyeDetector, "haarcascades/ojoI.xml");
    //loadDetector(m_rEyeDetector, "haarcascades/ojoD.xml");


    ui->actionCapture->setChecked(true);
}

MirrorWindow::~MirrorWindow()
{
    delete ui;
}

void MirrorWindow::loadDetector(cv::CascadeClassifier& detector, QString fname)
{
    QFileInfo fi(m_resourcesRoot.filePath(fname));
    Q_ASSERT(fi.exists());
    detector.load(fi.absoluteFilePath().toStdString());
}

void MirrorWindow::setCapture(bool on)
{
    if (on != m_camera.isOpened()) {
        if (on) {
            m_camera.open(0); // default camera
            m_metro.start(50); // ms
        } else {
            m_metro.stop();
            m_camera.release();
        }
    }
}

QImage MirrorWindow::CvMat2QImage(const cv::Mat& cvmat)
{
    int height = cvmat.rows;
    int width = cvmat.cols;

    if (cvmat.depth() == CV_8U && cvmat.channels() == 3) {
        QImage img((const uchar*)cvmat.data, width, height, cvmat.step.p[0], QImage::Format_RGB888);
        return img.rgbSwapped();
    } else if (cvmat.depth() == CV_8U && cvmat.channels() == 1) {
        QImage img((const uchar*)cvmat.data, width, height, cvmat.step.p[0], QImage::Format_Indexed8);
        img.setColorTable(s_greyTable);
        return img;
    } else {
        qWarning() << "Image cannot be converted.";
        return QImage();
    }
}

void MirrorWindow::tick()
{
    m_camera >> m_frames[Input];

    // grey will have the same dimensions as input
    cv::cvtColor( m_frames[Input], m_frames[Grey], CV_RGB2GRAY);

    if (ui->scaleSelector->currentIndex())
        cv::resize( m_frames[Grey], m_frames[Scaled], cv::Size(0,0), m_scale, m_scale );
    else
        m_frames[Scaled] = m_frames[Grey];
    cv::normalize( m_frames[Scaled], m_frames[Stretched], 0, 255, cv::NORM_MINMAX );

    std::vector<cv::Rect> rects;
    float smallest = 40.0 * m_scale;
    m_faceDetector.detectMultiScale( m_frames[Stretched], rects, 1.1, 3, 0, cv::Size(smallest, smallest));

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

    // show one of the images...
    QPixmap p;
    p.convertFromImage(CvMat2QImage(m_frames[m_videoLayer]));
    m_videoItem->setPixmap( p );
    m_videoItem->setScale( 640.0 / (float)p.width() );
}

void MirrorWindow::detectEye(cv::CascadeClassifier& detector, const cv::Rect& roi, std::vector<cv::Rect>& rects)
{
    cv::Mat eyePatch;
    cv::normalize( cv::Mat( m_frames[Grey], roi ), eyePatch, 0, 255, cv::NORM_MINMAX );
    cv::Size smallest(eyePatch.cols / 6, eyePatch.rows / 4);
    detector.detectMultiScale( eyePatch, rects, 1.2, 2, CV_HAAR_DO_CANNY_PRUNING, smallest );
    for(unsigned i=0; i<rects.size(); ++i) {
        rects[i] += roi.tl();
        m_faceGfx->addRect(rects[i], Mirror::CVLayer::defaultPen(), m_eyeBrush);
    }
}

bool MirrorWindow::bestEyePair(const std::vector<cv::Rect>& l, const std::vector<cv::Rect>& r, QPointF& left, QPointF& right)
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

void MirrorWindow::setFullscreen(bool on)
{
    if (on) {
        hide();
        ui->mainToolBar->hide();
        ui->dockWidget->hide();
        ui->statusBar->hide();
        ui->composite->setFrameShape( QFrame::NoFrame );
        showFullScreen();
    } else {
        hide();
        ui->composite->setFrameShape( QFrame::StyledPanel );
        ui->dockWidget->show();
        ui->mainToolBar->show();
        ui->statusBar->show();
        showNormal();
    }
}
