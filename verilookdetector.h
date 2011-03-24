#ifndef VERILOOKDETECTOR_H
#define VERILOOKDETECTOR_H

#include <QStringList>
#include <QDir>

#include "visionfilter.h"
#include "scratchgraphics.h"

class QPushButton;

namespace cv { class Mat; }

namespace Mirror {

class VerilookDetectorPrivate;

class VerilookDetector : public Mirror::VisionFilter
{
    Q_OBJECT
public:
    explicit VerilookDetector(Mirror::CompositeView * canvas, QObject *parent = 0);
    virtual ~VerilookDetector();

    void configureGUI(Ui::MirrorWindow * ui);

    bool detectEyes() const;
    bool recognize() const;

signals:

public slots:
    void setDetectEyes(bool on);
    void setRecognize(bool on);
    void loadNextFace();

protected:
    void filter(const cv::Mat& frame);

    class DbLoader {
    public:
        DbLoader();
        QString next();
    protected:
        QDir m_dir;
        QStringList m_fileList;
    };


    cv::Mat m_grey, m_normalized;
    ScratchGraphics * m_scratch;
    QTimer * m_loadTimer;
    DbLoader m_dbLoader;

    QPushButton * m_eyesButton;
    bool m_detectEyesSaved;

private:
    VerilookDetectorPrivate * m_private;
};

}

#endif // VERILOOKDETECTOR_H


