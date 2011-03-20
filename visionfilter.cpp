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

} // namespace Mirror
