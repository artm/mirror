#ifndef VERILOOKDETECTORPRIVATE_H
#define VERILOOKDETECTORPRIVATE_H

/* this should only be included by the verilookdetector.cpp,
   so we may include verilook headers here ... */

#include <NCore.h>
#include <NImages.h>
#include <NLExtractor.h>
#include <NMatcher.h>
#include <NLicensing.h>

#include <QString>
#include <QList>
#include <QSharedPointer>

#include <opencv2/core/core.hpp>

#include "face.h"

namespace Mirror {

class VerilookDetectorPrivate {
public:
    static VerilookDetectorPrivate * make();
    ~VerilookDetectorPrivate();
    static bool obtainLicense();
    static void releaseLicense();
    static QString errorString(NResult result);
    static bool isOk(NResult result,
                     QString errorSuffix = QString(),
                     QString successMessage = QString());
    void process(const cv::Mat& frame, QList<Face>& faces);

    bool detectEyes() const { return m_detectEyes; }
    void setDetectEyes(bool on) { m_detectEyes = on; }

    void addDbFace(const QString& path);

private:
    struct FaceTemplate {
        QString m_imgPath, m_tplPath;
        QByteArray m_data;

        FaceTemplate(const QString& imgPath,
                     const QString& tplPath,
                     const QByteArray& data);

    };

    typedef QSharedPointer<FaceTemplate> FaceTemplatePtr;

    VerilookDetectorPrivate();

    HNLExtractor m_extractor;
    bool m_detectEyes;
    QList< FaceTemplatePtr > m_templates;

    static bool s_gotLicense;

    static const char * s_defaultPort;
    static const char * s_defaultServer;
    static const char * s_licenseList;

    static int s_refcount;
};

} // namespace Mirror

#endif // VERILOOKDETECTORPRIVATE_H
