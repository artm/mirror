#include "footballtracker.h"

namespace Mirror {

FootballTracker::FootballTracker( Mirror::CompositeView * canvas, QObject *parent )
    : Mirror::VisionFilter(canvas, parent)
{
    appendVideoSlot("downsampled", &m_scaled);
    appendVideoSlot("hue",&m_hue);
    appendVideoSlot("posterized", &m_poster);
}

void FootballTracker::filter(const cv::Mat& frame)
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
