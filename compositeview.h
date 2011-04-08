#ifndef COMPOSITEVIEW_H
#define COMPOSITEVIEW_H

#include <QGraphicsView>

class QMimeData;

namespace Mirror {

class CompositeView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CompositeView(QWidget *parent = 0);

public slots:
    void zoom(int percent);
    void zoomFit();

signals:
     void fileDrop(const QMimeData *mimeData = 0);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

    void resizeEvent(QResizeEvent *event);

    float m_zoomFactor;

};

} // namespace Mirror

#endif // COMPOSITEVIEW_H
