#ifndef VERILOOKDETECTOR_H
#define VERILOOKDETECTOR_H

#include "visionfilter.h"

namespace cv { class Mat; }

namespace Mirror {

class VerilookDetector : public Mirror::VisionFilter
{
    Q_OBJECT
public:
    explicit VerilookDetector(Mirror::CompositeView * canvas, QObject *parent = 0);

    void configureGUI(Ui::MirrorWindow * ui);

signals:

public slots:

protected:
    void filter(const cv::Mat& frame);
};

}

#endif // VERILOOKDETECTOR_H


