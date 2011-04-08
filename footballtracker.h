#ifndef FOOTBALLTRACKER_H
#define FOOTBALLTRACKER_H

#include "visionfilter.h"
#include "scratchgraphics.h"

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
    cv::MatND m_fieldHist;
    cv::Rect m_fieldROI;
    bool m_foundField;
    QPolygonF m_fieldQuad;
    ScratchGraphics * m_fieldOverlay, * m_playersOverlay;
    QGraphicsPathItem * m_fieldQuadGfx;
    int m_dragCornerIdx;
    QPointF m_dragLastPos;
};

} // namespace Mirror

#endif // FOOTBALLTRACKER_H
