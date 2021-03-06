#include "footballtracker.h"
#include "ui_mirrorwindow.h"
#include "mirrorwindow.h"

#include <QtDebug>
#include <QCoreApplication>
#include <QPushButton>
#include <QKeyEvent>
#include <QtGui>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace Mirror {

const QSize FootballTracker::s_undistortSize(1024,1024);

FootballTracker::FootballTracker( Mirror::CompositeView * canvas, QObject *parent )
    : Mirror::VisionFilter(canvas, parent)
    , m_foundField(false)
    , m_dragCornerIdx(-1)
    , m_network(new QNetworkAccessManager(this))
{
    QString prefix = QCoreApplication::applicationDirPath () + "/../../../Resources/";
    qDebug() << prefix;
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

    appendVideoSlot("undistorted", &m_undistorted);
    appendVideoSlot("field mask", &m_fieldMaskUndistorted);
    appendVideoSlot("see through", &m_seeThrough);


    QGraphicsScene *  scene = canvas->scene();
    scene->addItem( m_playersOverlay = new ScratchGraphics );
    m_fieldQuadGfx = new QGraphicsPathItem(0,scene);
    m_fieldQuadGfx->setPen(QPen(Qt::red));

    m_canvas->setAttribute(Qt::WA_AcceptTouchEvents, true); //set this to receive touch events
    m_canvas->grabGesture(Qt::PinchGesture);

    connect(m_network, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(replyFinished(QNetworkReply*)));
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

        int longest = 0;
        for(unsigned i = 1; i<contours.size(); ++i) {
            if (contours[i].size() > contours[longest].size())
                longest = i;
        }

        m_fieldROI = cv::boundingRect( cv::Mat( contours[longest] ) );

        const cv::Point * longestPts[] = { &(contours[longest][0]) };
        int longestLen[] = { contours[longest].size() };
        cv::fillPoly( m_fieldnessBin, longestPts, longestLen, 1, 255 );
        m_fieldMask = m_fieldnessBin.clone();

        if (m_fieldQuad.size() == 0) {
            // initialize the field quad...
            float x0 = m_fieldROI.x, x1 = m_fieldROI.br().x,
                    y0 = m_fieldROI.y, y1 = m_fieldROI.br().y;
            m_fieldQuad << QPointF(x0,y0) << QPointF(x1,y0) << QPointF(x1,y1) << QPointF(x0,y1);

        }
        updateFieldQuadGfx();

        m_foundField = true;
    } else {
        // undistort...
        if (m_fieldQuad.size() == 4) {
            QString url = "http://sndrv.nl/soccar/data.php?";

            cv::warpPerspective( frame, m_undistorted, m_perspective, cv::Size2i(s_undistortSize.width(), s_undistortSize.height()));

            cv::cvtColor( m_undistorted, m_hsv, CV_BGR2HSV );
            // search for little guys in the whole thing...
            cv::calcBackProject( &m_hsv, 1, channels, m_fieldHist, m_fieldness, ranges );
            cv::threshold( m_fieldness, m_fieldnessBin, 200, 255, cv::THRESH_BINARY_INV);
            cv::multiply( m_fieldnessBin, m_fieldMaskUndistorted, m_fieldnessBin );

            cv::Mat kernel = (cv::Mat_<unsigned char>(3,3) << 0, 255, 0, 255, 255, 255, 0, 255, 0);
            cv::morphologyEx( m_fieldnessBin, m_fieldnessBin, cv::MORPH_OPEN, kernel, cv::Point(-1,-1), 2);
            kernel = (cv::Mat_<unsigned char>(3,3) << 0, 255, 0, 0, 255, 0, 0, 0, 0);
            cv::morphologyEx( m_fieldnessBin, m_fieldnessBin, cv::MORPH_DILATE, kernel, cv::Point(-1,-1), 5);

            // see through blob visualization...
            m_seeThrough = cv::Mat( m_undistorted.rows, m_undistorted.cols, CV_8UC3, 0);
            m_undistorted.copyTo(m_seeThrough, m_fieldnessBin);


            std::vector< std::vector< cv::Point > > contours;
            cv::findContours( m_fieldnessBin, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

            m_playersOverlay->clear();
            for(unsigned i = 0; i<contours.size(); ++i) {

                cv::approxPolyDP(cv::Mat(contours[i]), contours[i], 1.5, true);

                m_playersOverlay->addContour( contours[i], QPen(QColor(128,128,255)), QBrush(QColor(128,128,255,100)) );

                QPointF coord(contours[i][0].x, contours[i][0].y);
                foreach(cv::Point p, contours[i]) {
                    if (p.y > coord.y())
                        coord = QPointF( p.x, p.y );
                }

                QVector<QColor> colors(3);
                findTopColors(contours, i, colors);
                for( int c=0; c<colors.size(); ++c ) {
                    QGraphicsRectItem * r = new QGraphicsRectItem(0,0,10,10,m_playersOverlay);
                    r->setPos(coord.x()+c*7, coord.y()+c*7);
                    r->setBrush( QBrush(colors[c]) );
                }

                double area = cv::contourArea(cv::Mat(contours[i]));

                QString sCoord = QString("%1,%2\nsize:%3")
                        .arg(coord.x()/m_fieldnessBin.cols).arg(coord.y()/m_fieldnessBin.rows)
                        .arg(area);
                QGraphicsTextItem * text = new QGraphicsTextItem(sCoord, m_playersOverlay);
                text->setPos(coord);

                if (i) url += "&";

                url += QString("nr[]=%1&x[]=%2&y=%3&size=%4")
                        .arg(i)
                        .arg(coord.x()/m_fieldnessBin.cols,0,'f',2)
                        .arg(coord.y()/m_fieldnessBin.rows,0,'f',2)
                        .arg((int)area);
                for( int c=0; c<colors.size(); ++c ) {
                    url += QString("&color%1[]=%2").arg(c).arg( colors[c].name() );
                }


            }

            qDebug() << "Requesting: " << url;
            m_network->get(QNetworkRequest(QUrl(url)));

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

}

void FootballTracker::toggleOverlay()
{
    if (m_playersOverlay->isVisible()) {
        m_playersOverlay->hide();
        m_fieldQuadGfx->hide();
    } else {
        m_playersOverlay->show();
        m_fieldQuadGfx->show();
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
    m_canvas->installEventFilter(this);

    m_zoomSlider = new QSlider(Qt::Horizontal);
    layout->addRow("Zoom out", m_zoomSlider);
    m_zoomSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_zoomSlider->setRange(50,100);
    m_zoomSlider->setValue(100);
    connect(m_zoomSlider, SIGNAL(valueChanged(int)), m_canvas, SLOT(zoom(int)));

    layout->addRow( "<b>Space</b>", new QLabel(" - relearn background") );
    layout->addRow( "<b>o</b>", new QLabel("-  overlay on/off") );
}

bool FootballTracker::eventFilter(QObject * obj, QEvent * event)
{
    if (obj == m_canvas) {

        QMouseEvent * mevent = dynamic_cast<QMouseEvent*>(event);
        if (mevent) {
            mevent->accept();

            QPointF p = m_canvas->mapToScene( mevent->pos() );
            QPointF dpos = p - m_dragLastPos;
            m_dragLastPos = p;

            float minD = 1e6;
            switch (event->type()) {
            case QEvent::MouseButtonPress:
                // pick the closest corner of the field quad for dragging

                for(int i=0; i<4; ++i) {
                    float d = (p - m_fieldQuad[i]).manhattanLength();
                    if ((d < 75) && (d < minD)) {
                        minD = d;
                        m_dragCornerIdx = i;
                    }
                    if (m_dragCornerIdx >= 0)
                        m_canvas->grabMouse();
                }
                break;
            case QEvent::MouseButtonRelease:
                // stop dragging
                if (m_dragCornerIdx >= 0)
                    m_canvas->releaseMouse();
                m_dragCornerIdx = -1;
                break;
            case QEvent::MouseMove:
                // if dragging - drag
                if (m_dragCornerIdx >=0) {
                    m_fieldQuad[m_dragCornerIdx] += dpos;
                    updateFieldQuadGfx();
                }
                break;
            default:
                break;
            }

            return true;
        } else if (event->type() == QEvent::Gesture) {
            QGestureEvent * gevent = dynamic_cast<QGestureEvent *>(event);
            if (QPinchGesture *pinch = dynamic_cast<QPinchGesture *>(gevent->gesture(Qt::PinchGesture))) {
                //qDebug() << pinch->scaleFactor();
                m_zoomSlider->setValue( pinch->scaleFactor() * m_zoomSlider->value() );
            }
        }

    } else if (event->type() == QEvent::KeyRelease) {
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

void FootballTracker::updateFieldQuadGfx()
{
    if (m_fieldQuad.size() != 4)
        return;

    QPainterPath path(m_fieldQuad[0]);
    for(int i = 1; i<4; ++i)
        path.lineTo(m_fieldQuad[i]);
    path.closeSubpath();

    m_fieldQuadGfx->setPath(path);

    cv::Point2f src[4] = {
        cv::Point2f(m_fieldQuad[0].x(),m_fieldQuad[0].y()),
        cv::Point2f(m_fieldQuad[1].x(),m_fieldQuad[1].y()),
        cv::Point2f(m_fieldQuad[2].x(),m_fieldQuad[2].y()),
        cv::Point2f(m_fieldQuad[3].x(),m_fieldQuad[3].y()),
    };
    cv::Point2f dst[4] = {
        cv::Point2f(0,0),
        cv::Point2f(s_undistortSize.width(),0),
        cv::Point2f(s_undistortSize.width(),s_undistortSize.height()),
        cv::Point2f(0,s_undistortSize.height()),
    };
    m_perspective = cv::getPerspectiveTransform( src,  dst );
    cv::warpPerspective( m_fieldMask, m_fieldMaskUndistorted, m_perspective,
                        cv::Size2i(s_undistortSize.width(),s_undistortSize.height()));
}

} // namespace Mirror

/* helper struct for histogram sorting */
struct Bar
{
    int i, j;
    float v;
    Bar() : i(0),j(0),v(-1) {}
    Bar(int _i, int _j, int _v)
        : i(_i), j(_j), v(_v)
    {}

    static bool lesser(const Bar& a, const Bar& b)
    {
        return a.v < b.v;
    }
    static bool greater(const Bar& a, const Bar& b)
    {
        return a.v > b.v;
    }

};

void Mirror::FootballTracker::findTopColors(const std::vector< std::vector< cv::Point > >& blobs,
                                            int blobIndex,
                                            QVector<QColor>& colors)
{
    // make a blob-shaped mask
    cv::Rect bounds = cv::boundingRect( cv::Mat(blobs[blobIndex]) );
    cv::Mat mask( bounds.size(), CV_8UC1);
    // draw a blob on a mask
    cv::drawContours( mask, blobs, blobIndex, 255, -1 /* fill */, 8, std::vector<cv::Vec4i>(), INT_MAX,
                     cv::Point2d( -bounds.x, -bounds.y));
    // make a histogram of a blob

    int channels[] = { 0, 1 }; // hue, saturation
    int histSize[] = { 30, 30 }; // quantization of hue, saturation
    float hranges[] = { 0, 180 }, sranges[] = { 0, 256 };
    const float * ranges[] = { hranges, sranges };

    cv::Mat cutout(m_hsv,bounds), hist;

    cv::calcHist( &cutout,
             1, // one array
             channels,
             mask,
             hist,
             2, // dimensions
             histSize,
             ranges,
             true, // the histogram is uniform
             false // don't accumulate
             );

    // find the n tallest bars
    unsigned int nTallest = colors.size();

    std::vector<Bar> tallest;
    for(int i = 0; i< histSize[0]; ++i)
        for(int j = 0; j<histSize[1]; ++j) {
            Bar b(i,j,hist.at<float>(i, j));
            std::vector<Bar>::iterator place = std::lower_bound(tallest.begin(), tallest.end(), b, Bar::greater);
            if (tallest.size() < nTallest || place != tallest.end())
                tallest.insert(place, b);
            if (tallest.size() > nTallest) {
                tallest.resize(nTallest);
            }
        }

    // convert histogram bar indices to colors (i == hue, j == saturation)
    int n = 0;
    foreach(Bar bar, tallest) {
        colors[n++] = QColor::fromHsv(
                    // 2* is due to different ranges for Hue in OpenCV and Qt
                    2 * (hranges[0] + bar.i * hranges[1] / histSize[0]),
                    sranges[0] + bar.j * sranges[1] / histSize[1],
                    200); // intensity?
    }
}

void Mirror::FootballTracker::replyFinished(QNetworkReply * reply)
{
    qDebug() << "got network reply" << (reply->error() ? reply->errorString() : "no error") ;
    reply->deleteLater();
}


