#ifndef UTIL_H
#define UTIL_H

#include <QImage>
#include <opencv2/core/core.hpp>

namespace Mirror {

QImage CvMat2QImage(const cv::Mat& cvmat);
QString findResourceFile(const QString& relPath);

} // namespace Mirror

#endif // UTIL_H
