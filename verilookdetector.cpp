#include "verilookdetector.h"
#include "ui_mirrorwindow.h"

#include <opencv2/core/core.hpp>

namespace Mirror {

VerilookDetector::VerilookDetector(Mirror::CompositeView * canvas, QObject *parent)
    : VisionFilter(canvas, parent)
{

}

void VerilookDetector::configureGUI(Ui::MirrorWindow * ui)
{
    /* Add controls and connect their signals to our slots and v.v.*/
}

void VerilookDetector::filter(const cv::Mat& frame)
{
    /* process frame and do stuff */
}

} // namespace Mirror

