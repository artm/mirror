#ifndef VERILOOKDETECTORPRIVATE_H
#define VERILOOKDETECTORPRIVATE_H

#include "face.h"

/* this should only be included by the verilookdetector.cpp,
   so we may include verilook headers here ... */

#include <NCore.h>
#include <NLExtractor.h>
#include <NLicensing.h>

#include <QObject>
#include <QString>
#include <QList>
#include <QSharedPointer>
#include <QTimer>
#include <QTime>
#include <QMutex>

#include <opencv2/core/core.hpp>

namespace Mirror {

class VerilookMatchingThread;

class VerilookDetectorPrivate : public QObject {
    Q_OBJECT
public:
    static VerilookDetectorPrivate * make();
    virtual ~VerilookDetectorPrivate();
    static bool obtainLicense();
    static void releaseLicense();
    static QString errorString(NResult result);
    static bool isOk(NResult result,
                     QString errorSuffix = QString(),
                     QString successMessage = QString());
    void process(const cv::Mat& frame, QList<Face>& faces);

    bool detectEyes() const { return m_detectEyes; }
    void setDetectEyes(bool on) { m_detectEyes = on; }
    bool recognize() const { return m_recognize; }
    void setRecognize(bool on) { m_recognize = on; }

    void addDbFace(const QString& path);

public slots:

private:
    VerilookDetectorPrivate();

    QMutex * m_extractorMutex;
    HNLExtractor m_extractor;

    bool m_detectEyes, m_recognize;

    VerilookMatchingThread * m_matchingThread;

    static bool s_gotLicense;

    static const char * s_defaultPort;
    static const char * s_defaultServer;
    static const char * s_licenseList;

    static int s_refcount;
};

} // namespace Mirror

#endif // VERILOOKDETECTORPRIVATE_H
