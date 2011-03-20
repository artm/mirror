#ifndef UTIL_H
#define UTIL_H

#include <QImage>
#include <cv.h>

namespace Mirror {

QImage CvMat2QImage(const cv::Mat& cvmat);

} // namespace Mirror

#endif // UTIL_H
