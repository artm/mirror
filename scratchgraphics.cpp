#include "scratchgraphics.h"

#include <QGraphicsScene>
#include <QVector>
#include <QPointF>

namespace Mirror {

QPen ScratchGraphics::s_defaultPen = QPen( QColor(200, 255, 200, 128) );
QBrush ScratchGraphics::s_defaultBrush = QBrush( QColor(200, 255, 200, 64) );

ScratchGraphics::ScratchGraphics(QGraphicsItem * parent , QGraphicsScene * scene)
    : QGraphicsItemGroup(parent, scene)
{
}

void ScratchGraphics::clear()
{
    foreach(QGraphicsItem * child, childItems()) {
        scene()->removeItem( child );
    }
}

void ScratchGraphics::addRect( const cv::Rect& rect, QPen pen, QBrush brush )
{
    QGraphicsRectItem * item = new QGraphicsRectItem(makeQt(rect));
    addToGroup( item );
    item->setPen( pen );
    item->setBrush( brush );
}

void ScratchGraphics::addLine( const QLineF& line, QPen pen)
{
    QGraphicsLineItem * item = new QGraphicsLineItem(line);
    addToGroup( item );
    item->setPen( pen );
}

void ScratchGraphics::addContour( std::vector< cv::Point > contour, QPen pen, QBrush brush)
{
    QVector< QPointF > points;
    foreach(cv::Point p, contour) {
        points.append( QPointF( p.x, p.y ) );
    }
    QGraphicsPolygonItem * item = new QGraphicsPolygonItem( QPolygonF(points) );
    addToGroup( item );
    item->setPen( pen );
    item->setBrush( brush );
}

void ScratchGraphics::addContour( std::vector< cv::Point > contour, std::vector<int> idx, QPen pen, QBrush brush)
{
    QVector< QPointF > points;
    foreach(int i, idx) {
        points.append( QPointF( contour[i].x, contour[i].y ) );
    }
    QGraphicsPolygonItem * item = new QGraphicsPolygonItem( QPolygonF(points) );
    addToGroup( item );
    item->setPen( pen );
    item->setBrush( brush );
}

} // namespace Mirror
