#ifndef FOOTBALLTRACKER_H
#define FOOTBALLTRACKER_H

#include "visionfilter.h"

namespace Mirror {

class FootballTracker : public Mirror::VisionFilter
{
    Q_OBJECT
public:
    explicit FootballTracker(Mirror::CompositeView * canvas, QObject *parent = 0);

signals:

public slots:

protected:
    void filter(const cv::Mat& frame);

    // FIXME move these to the apropriate ancestor
    cv::Mat m_scaled, m_hsv, m_hue, m_poster;
};

} // namespace Mirror

#endif // FOOTBALLTRACKER_H
