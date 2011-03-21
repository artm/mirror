#ifndef MIRRORWINDOW_H
#define MIRRORWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QGraphicsPixmapItem>
#include <QDir>
#include <QBrush>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "scratchgraphics.h"

class QMimeData;

namespace Ui {
    class MirrorWindow;
}

namespace Mirror {
    class VisionFilter;
}

class MirrorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MirrorWindow(QWidget *parent = 0);
    ~MirrorWindow();

signals:
    void incomingFrame(const cv::Mat& frame);

public slots:
    void setCapture(bool on);
    void tick();
    void setFullscreen(bool on);
    void loadFile(const QMimeData* mimeData);
    void loadFile(const QString& path);

protected:
    void keyReleaseEvent(QKeyEvent * event);

    Ui::MirrorWindow *ui;
    cv::VideoCapture * m_camera;
    QTimer m_metro;
    Mirror::VisionFilter * m_filter;
};

#endif // MIRRORWINDOW_H
