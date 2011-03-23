#ifndef VERILOOKDETECTOR_H
#define VERILOOKDETECTOR_H

#include "visionfilter.h"
#include "scratchgraphics.h"

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

signals:

public slots:
    void setDetectEyes(bool on);

protected:
    void filter(const cv::Mat& frame);


    cv::Mat m_grey, m_normalized;
    ScratchGraphics * m_scratch;

private:
    VerilookDetectorPrivate * m_private;
};

}

#endif // VERILOOKDETECTOR_H


