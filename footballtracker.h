#ifndef FOOTBALLTRACKER_H
#define FOOTBALLTRACKER_H

#include "visionfilter.h"
#include "scratchgraphics.h"

class QSlider;

namespace Mirror {

class FootballTracker : public VisionFilter
{
    Q_OBJECT
public:
    explicit FootballTracker(CompositeView * canvas, QObject *parent = 0);

    void configureGUI(Ui::MirrorWindow * ui);

signals:

public slots:
    void relearnBg() { m_foundField = false; }
    void toggleOverlay();

protected:
    void filter(const cv::Mat& frame);
    bool eventFilter(QObject *, QEvent *);

    void updateFieldQuadGfx();

    cv::Mat m_hsv, m_field, m_fieldness, m_fieldnessBin, m_fieldMask, m_seeThrough;
    cv::Mat m_undistorted, m_fieldMaskUndistorted;
    cv::Mat m_perspective;

    cv::MatND m_fieldHist;
    cv::Rect m_fieldROI;
    bool m_foundField;
    QPolygonF m_fieldQuad;
    ScratchGraphics * m_playersOverlay;
    QGraphicsPathItem * m_fieldQuadGfx;
    int m_dragCornerIdx;
    QPointF m_dragLastPos;
    QSlider * m_zoomSlider;

    static const QSize s_undistortSize;
};

} // namespace Mirror

#endif // FOOTBALLTRACKER_H
