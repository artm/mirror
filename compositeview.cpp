#include "compositeview.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

#include <QtDebug>

namespace Mirror {

CompositeView::CompositeView(QWidget *parent)
    : QGraphicsView(parent)
    , m_zoomFactor(1.0)
{
    setAcceptDrops(true);
}

void CompositeView::resizeEvent(QResizeEvent * /*event*/)
{
    zoomFit();
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

void CompositeView::zoom(int percent)
{
    m_zoomFactor = 1.0f / (0.01f * percent);
    zoomFit();
}


void CompositeView::zoomFit()
{
    QSizeF sz = sceneRect().size() * m_zoomFactor;
    QRectF visible(sceneRect().center() - QPointF(sz.width(), sz.height()) * 0.5, sz);

    fitInView(visible, Qt::KeepAspectRatio);
}

} // namespace Mirror
