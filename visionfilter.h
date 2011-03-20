#ifndef VISIONFILTER_H
#define VISIONFILTER_H

#include "compositeview.h"

#include <cv.h>

#include <QObject>
#include <QHash>

namespace Mirror {

class VisionFilter : public QObject
{
    Q_OBJECT
public:
    explicit VisionFilter(Mirror::CompositeView * canvas, QObject *parent = 0);

    const QList<QString> slotsOrder() const { return m_slotsOrder; }

signals:

public slots:
    void incomingFrame(const cv::Mat& frame);
    void setVisibleSlot(const QString& slot) { Q_ASSERT( m_slots.contains(slot)); m_visibleSlot = slot; }

protected:
    virtual void filter(const cv::Mat& frame);

    void appendVideoSlot(const QString& name, const cv::Mat * slot);

    CompositeView * m_canvas;
    QGraphicsPixmapItem * m_videoLayer;

    QList<QString> m_slotsOrder;
    QHash<QString, const cv::Mat * > m_slots;
    QString m_visibleSlot;

    // FIXME move these to the apropriate ancestor
    cv::Mat m_scaled, m_hsv, m_hue, m_poster;
};

} // namespace Mirror

#endif // VISIONFILTER_H
