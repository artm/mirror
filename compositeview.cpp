#include "compositeview.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

#include <QtDebug>

namespace Mirror {

CompositeView::CompositeView(QWidget *parent) :
    QGraphicsView(parent)
{
    setAcceptDrops(true);
}

void CompositeView::resizeEvent(QResizeEvent * /*event*/)
{
    fitInView(0,0,640,480,Qt::KeepAspectRatio);
}

void CompositeView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CompositeView::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void CompositeView::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void CompositeView::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    emit fileDrop(mimeData);
}

} // namespace Mirror
