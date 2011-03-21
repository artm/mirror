#ifndef CVLAYER_H
#define CVLAYER_H

#include <QGraphicsItemGroup>
#include <QPen>

#include <opencv2/core/core.hpp>

#include <vector>

namespace Mirror {

class ScratchGraphics : public QGraphicsItemGroup
{
public:
    ScratchGraphics(QGraphicsItem * parent = 0, QGraphicsScene * scene = 0);

    QRectF makeQt(const cv::Rect& rect) { return QRectF( rect.x, rect.y, rect.width, rect.height ); }

    static const QPen& defaultPen() { return s_defaultPen; }
    static const QBrush& defaultBrush() { return s_defaultBrush; }

signals:

public slots:
    void clear();
    void addRect( const cv::Rect& rect, QPen pen = s_defaultPen, QBrush brush = s_defaultBrush);
    void addLine( const QLineF& line, QPen pen = s_defaultPen);
    void addContour( std::vector< cv::Point > contour, QPen pen = s_defaultPen, QBrush brush = s_defaultBrush);
    void addContour( std::vector< cv::Point > contour, std::vector<int> idx, QPen pen, QBrush brush);

protected:
    static QPen s_defaultPen;
    static QBrush s_defaultBrush;
};

} // namespace Mirror

#endif // CVLAYER_H
