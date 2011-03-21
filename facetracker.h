#ifndef FACETRACKER_H
#define FACETRACKER_H

#include "visionfilter.h"

#include <opencv2/objdetect/objdetect.hpp>

namespace Mirror {

class ScratchGraphics;

class FaceTracker : public Mirror::VisionFilter
{
    Q_OBJECT
public:
    explicit FaceTracker(Mirror::CompositeView * canvas, QObject *parent = 0);

    void configureGUI(Ui::MirrorWindow * ui);

signals:

public slots:
    void onScaleSelected(int index) { m_scale = 1.0 / (qreal)(1 << index); }

protected:
    void filter(const cv::Mat& frame);

    void detectEye(cv::CascadeClassifier& detector, const cv::Rect& roi, std::vector<cv::Rect>& rects);
    bool bestEyePair(const std::vector<cv::Rect>& l, const std::vector<cv::Rect>& r, QPointF& left, QPointF& right);

    cv::Mat m_grey, m_scaled, m_stretched;
    cv::CascadeClassifier m_faceDetector, m_lEyeDetector, m_rEyeDetector;
    Mirror::ScratchGraphics * m_faceGfx;
    QBrush m_eyeBrush;
    qreal m_scale;
};

} // namespace Mirror

#endif // FACETRACKER_H
