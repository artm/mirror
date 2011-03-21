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
    fitInView(sceneRect(), Qt::KeepAspectRatio);
}

void CompositeView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        qDebug() << "Accepting drag enter";
        event->acceptProposedAction();
    } else {
        qDebug() << "Didn't accept drag enter" << event->mimeData()->formats();
    }
}

void CompositeView::dragMoveEvent(QDragMoveEvent *event)
{
    //qDebug() << "Accepting drag move";
    event->acceptProposedAction();
}

void CompositeView::dragLeaveEvent(QDragLeaveEvent *event)
{
    qDebug() << "Accepting drag leave";
    event->accept();
}

void CompositeView::dropEvent(QDropEvent *event)
{
    qDebug() << "Accepting drop";
    const QMimeData *mimeData = event->mimeData();
    emit fileDrop(mimeData);
}

} // namespace Mirror
