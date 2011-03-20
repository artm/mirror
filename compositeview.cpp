#include "compositeview.h"

namespace Mirror {

CompositeView::CompositeView(QWidget *parent) :
    QGraphicsView(parent)
{
}

void CompositeView::resizeEvent(QResizeEvent *event)
{
    fitInView(0,0,640,480,Qt::KeepAspectRatio);
}


} // namespace Mirror
