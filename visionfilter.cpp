#include "visionfilter.h"
#include "util.h"

#include <QGraphicsPixmapItem>

namespace Mirror {

VisionFilter::VisionFilter(Mirror::CompositeView * canvas, QObject *parent)
    : QObject(parent)
    , m_canvas(canvas)
{
    QPixmap p(640, 480);
    p.fill(Qt::black);
    m_videoLayer = m_canvas->scene()->addPixmap( p );

    m_visibleSlot = "input";
    m_slotsOrder.append("input");

    // FIXME -> descendant
    appendVideoSlot("downsampled", &m_scaled);
    appendVideoSlot("hue",&m_hue);
    appendVideoSlot("posterized", &m_poster);
}

void VisionFilter::appendVideoSlot(const QString& name, const cv::Mat * slot)
{
    m_slots[name] = slot;
    m_slotsOrder.append(name);
}

void VisionFilter::incomingFrame(const cv::Mat& frame)
{
    m_slots["input"] = &frame;

    filter(frame);

    if (m_slots.contains(m_visibleSlot)) {
        // show one of the images...
        QPixmap p;
        p.convertFromImage(Mirror::CvMat2QImage( *m_slots[m_visibleSlot] ));
        m_videoLayer->setPixmap( p );
        // fit width (?)
        m_videoLayer->setScale( 640.0 / (float)p.width() );

    }
}

void VisionFilter::filter(const cv::Mat& frame)
{
    cv::resize( frame, m_scaled, cv::Size(0,0), 0.5, 0.5, cv::INTER_NEAREST );
    cv::cvtColor( frame, m_hsv, CV_BGR2HSV );

    int fromto[] = { 0, 0 };
    m_hue.create( m_hsv.rows, m_hsv.cols, CV_8UC1 );
    cv::mixChannels( &m_hsv, 1, &m_hue, 1, fromto, 1 );

    m_poster = m_scaled.clone();
    //segmented.create(frame.rows, frame.cols, CV_8U);
    CvMat cScaled = (CvMat)m_scaled, cSegmented = (CvMat)m_poster;
    cvPyrMeanShiftFiltering( &cScaled, &cSegmented, 5.0, 45.0 );

}

} // namespace Mirror
