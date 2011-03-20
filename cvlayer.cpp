#include "cvlayer.h"

#include <QGraphicsScene>

namespace Mirror {

QPen CVLayer::s_defaultPen = QPen( QColor(200, 255, 200, 128) );
QBrush CVLayer::s_defaultBrush = QBrush( QColor(200, 255, 200, 64) );

CVLayer::CVLayer(QGraphicsItem * parent , QGraphicsScene * scene)
    : QGraphicsItemGroup(parent, scene)
{
}

void CVLayer::clear()
{
    foreach(QGraphicsItem * child, childItems()) {
        scene()->removeItem( child );
    }
}

void CVLayer::addRect( const cv::Rect& rect, QPen pen, QBrush brush )
{
    QGraphicsRectItem * item = new QGraphicsRectItem(makeQt(rect));
    addToGroup( item );
    item->setPen( pen );
    item->setBrush( brush );
}

void CVLayer::addLine( const QLineF& line, QPen pen)
{
    QGraphicsLineItem * item = new QGraphicsLineItem(line);
    addToGroup( item );
    item->setPen( pen );
}

} // namespace Mirror
