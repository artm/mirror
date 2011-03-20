#ifndef MIRRORWINDOW_H
#define MIRRORWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QGraphicsPixmapItem>
#include <QDir>
#include <QBrush>

#include <cv.h>
#include <highgui.h>

#include "cvlayer.h"
#include "visionfilter.h"

class QMimeData;

namespace Ui {
    class MirrorWindow;
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
    void showInput() { setVideoLayer(Input); }
    void showGrey() { setVideoLayer(Grey); }
    void showScaled() { setVideoLayer(Scaled); }
    void showStretched() { setVideoLayer(Stretched); }

    void onScaleSelected(int index) { m_scale = 1.0 / (qreal)(1 << index); }
    void setFullscreen(bool on);

    void loadFile(const QMimeData* mimeData);

private:
    enum Show {
        Input,
        Grey,
        Scaled,
        Stretched,

        FrameCount
    };
    void setVideoLayer(Show layer) { Q_ASSERT(layer < FrameCount); m_videoLayer = layer; }
    void loadDetector(cv::CascadeClassifier& detector, QString fname);

    void detectEye(cv::CascadeClassifier& detector, const cv::Rect& roi, std::vector<cv::Rect>& rects);
    bool bestEyePair(const std::vector<cv::Rect>& l, const std::vector<cv::Rect>& r, QPointF& left, QPointF& right);

    Ui::MirrorWindow *ui;
    QDir m_resourcesRoot;

    cv::VideoCapture m_camera;
    cv::CascadeClassifier m_faceDetector;
    cv::CascadeClassifier m_lEyeDetector;
    cv::CascadeClassifier m_rEyeDetector;

    QTimer m_metro;

    Mirror::CVLayer * m_faceGfx;
    QBrush m_eyeBrush;

    Show m_videoLayer;
    cv::Mat m_frames[FrameCount];

    qreal m_scale;
    Mirror::VisionFilter * m_filter;

};

#endif // MIRRORWINDOW_H
