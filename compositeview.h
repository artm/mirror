#ifndef COMPOSITEVIEW_H
#define COMPOSITEVIEW_H

#include <QGraphicsView>

namespace Mirror {

class CompositeView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CompositeView(QWidget *parent = 0);

signals:

public slots:

protected:
    void resizeEvent(QResizeEvent *event);

};

} // namespace Mirror

#endif // COMPOSITEVIEW_H
