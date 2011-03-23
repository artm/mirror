#ifndef FACE_H
#define FACE_H

#include <QRectF>
#include <QPointF>

namespace Mirror {

class Face {
public:
    Face() : m_faceRect(0,0,0,0), m_hasFace(false), m_hasEyes(false) {}
    Face(const QRectF& rect) : m_faceRect(rect), m_hasFace(true), m_hasEyes(false) {}
    Face(const QRectF& rect, QPointF eye1, QPointF eye2)
        : m_faceRect(rect) , m_hasFace(true)
    {
        setEyes(eye1,eye2);
    }
    void setEyes(QPointF eye1, QPointF eye2)
    {
        m_eyes[0] = eye1;
        m_eyes[1] = eye2;
        m_hasEyes = true;
    }

    bool hasFace() const { return m_hasFace; }
    bool hasEyes() const { return m_hasEyes; }

    /* it's client's duty to check for presence of the features */
    QRectF face() const { return m_faceRect; }
    QPointF eye(int i) const { Q_ASSERT(i<2); return m_eyes[i]; }
    QPointF faceCenter() const { return m_faceRect.center(); }
    QPointF eyesMidPoint() const { return m_eyes[0] + 0.5*(m_eyes[1]-m_eyes[0]); }

protected:
    QRectF m_faceRect;
    QPointF m_eyes[2];
    bool m_hasFace, m_hasEyes;
};

} // namespace Mirror

#endif // FACE_H
