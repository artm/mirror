#ifndef VERILOOKMATCHINGTHREAD_H
#define VERILOOKMATCHINGTHREAD_H

#include <NCore.h>
#include <NImages.h>
#include <NLExtractor.h>
#include <NMatcher.h>

#include <QThread>
#include <QSharedPointer>

class QMutex;

namespace Mirror {

class VerilookMatchingThread : public QThread
{
    Q_OBJECT
public:
    explicit VerilookMatchingThread(HNLExtractor extractor,
                                    QMutex * mutex,
                                    QObject * parent);

    virtual ~VerilookMatchingThread();

    void setupRecognition(HNImage img, const NleDetectionDetails& details);
    void run();
    void addDbFace(const QString& path);

signals:

public slots:

protected:
    struct FaceTemplate {
        QString m_imgPath, m_tplPath;
        QByteArray m_data;

        FaceTemplate(const QString& imgPath,
                     const QString& tplPath,
                     const QByteArray& data);

    };
    typedef QSharedPointer<FaceTemplate> FaceTemplatePtr;

    HNLExtractor m_extractor;
    QMutex * m_extractorMutex;

    HNMatcher m_matcher;
    QList< FaceTemplatePtr > m_templates;

    HNImage m_img;
    NleDetectionDetails m_details;
};

} // namespace Mirror

#endif // VERILOOKMATCHINGTHREAD_H
