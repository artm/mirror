#include "verilookmatchingthread.h"
#include "verilookdetectorprivate.h"

#include <QtDebug>
#include <QtCore>

namespace Mirror {

VerilookMatchingThread::VerilookMatchingThread(
    HNLExtractor extractor,
    QMutex * mutex,
    QObject *parent)
    : QThread(parent), m_extractor(extractor), m_extractorMutex(mutex),
      m_img(0)
{
    if (! VerilookDetectorPrivate::isOk(NMCreate(&m_matcher),
                                        "No verilook matcher created",
                                        "Verilook matcher created")) {
        m_matcher = 0;
    }
}

VerilookMatchingThread::~VerilookMatchingThread()
{

    if (m_matcher) {
        NMFree(m_matcher);
        m_matcher = 0;
    }

    if (m_img)
        NObjectFree(m_img);

    m_templates.clear();

}

void VerilookMatchingThread::setupRecognition(
    HNImage img,
    const NleDetectionDetails& details)
{
    if (m_img)
        NObjectFree(m_img);
    VerilookDetectorPrivate::isOk(NImageClone(img, &m_img));
    m_details = details;
}

void VerilookMatchingThread::run()
{
    QTime m_clock;
    NMMatchDetails * m_matchDetails;

    NleExtractionStatus status;
    HNLTemplate tpl = 0;

    m_clock.start();

    QMutexLocker locker(m_extractorMutex);

    if ( VerilookDetectorPrivate::isOk(
                NleExtractUsingDetails(
                    m_extractor, m_img, &m_details,
                    &status, &tpl)) ) {

        locker.unlock();

        if (status == nleesTemplateCreated) {

            qDebug() << QString("Matching template created in %1 ms")
                        .arg(m_clock.elapsed());
            m_clock.restart();

            // extract template data
            NSizeType maxSize, size;
            QByteArray m_matchData;

            if (!VerilookDetectorPrivate::isOk(NLTemplateGetSize(tpl, 0, &maxSize)))
                goto TODO_scoped_verilook;

            m_matchData.resize(maxSize);

            if (!VerilookDetectorPrivate::isOk(NLTemplateSaveToMemory(
                          tpl,
                          m_matchData.data(),
                          m_matchData.size(),
                          0,
                          &size)))
                goto TODO_scoped_verilook;

            // schedule to match
            if (VerilookDetectorPrivate::isOk(NMIdentifyStart(
                         m_matcher,
                         m_matchData.data(),
                         m_matchData.size(),
                         &m_matchDetails),
                     "Couldn't start matching")) {

                QList< FaceTemplatePtr >::iterator iter = m_templates.begin();
                for(;iter != m_templates.end(); ++iter) {
                    NInt score;

                    Q_ASSERT (iter != m_templates.end());

                    FaceTemplatePtr dbFace(*iter);

                    if ( VerilookDetectorPrivate::isOk(
                                NMIdentifyNext(m_matcher,
                                               dbFace->m_data.data(),
                                               dbFace->m_data.size(),
                                               m_matchDetails,
                                               &score))) {
                        qDebug() << dbFace->m_imgPath << "matches with score:" << score;
                    }

                    if (++iter == m_templates.end()) {
                        qDebug() << QString("Reached the end of the database in %1 ms (%2 face/sec)")
                                    .arg(m_clock.elapsed())
                                    .arg((float)m_templates.size()/m_clock.elapsed()*1000);
                        VerilookDetectorPrivate::isOk(NMIdentifyEnd(m_matcher));
                        if (m_matchDetails)
                            NMMatchDetailsFree(m_matchDetails);
                    }
                }
            }

            TODO_scoped_verilook:

            if (tpl) NLTemplateFree(tpl);
        }
    }

}

void VerilookMatchingThread::addDbFace(const QString& imgPath)
{
    QFileInfo fi(imgPath);
    Q_ASSERT( fi.exists() );
    QString tplPath = fi.path() + "/" + fi.baseName() + ".tpl";
    if (QFileInfo(tplPath).exists()) {
        // load from file
        QFile tplFile(tplPath);
        tplFile.open(QFile::ReadOnly);
        m_templates.push_back(
                    FaceTemplatePtr(
                        new FaceTemplate(imgPath, tplPath,
                                         tplFile.readAll())));
    } else {
        HNImage image, greyscale;
        NleDetectionDetails details;
        HNLTemplate tpl;
        NleExtractionStatus extrStatus;

        if (VerilookDetectorPrivate::isOk(NImageCreateFromFile(imgPath.toLocal8Bit(), NULL, &image))) {

            if (VerilookDetectorPrivate::isOk(NImageCreateFromImage(npfGrayscale, 0, image, &greyscale))) {

                NResult result;

                {
                    QMutexLocker locker(m_extractorMutex);

                    result = NleExtract(
                                m_extractor,
                                greyscale,
                                &details,
                                &extrStatus,
                                &tpl);
                } // unlock the locker

                if (VerilookDetectorPrivate::isOk(result) && (extrStatus == nleesTemplateCreated)) {
                    // compress
                    HNLTemplate compTemplate;

                    if (VerilookDetectorPrivate::isOk(NleCompressEx(tpl, nletsSmall, &compTemplate))) {

                        // free uncompressed template
                        NLTemplateFree(tpl);

                        // get the size of the template
                        NSizeType maxSize;
                        if (VerilookDetectorPrivate::isOk(NLTemplateGetSize(compTemplate, 0, &maxSize))) {

                            // transform to byte array
                            NSizeType size;
                            QByteArray bytes(maxSize, 0);

                            if (VerilookDetectorPrivate::isOk(NLTemplateSaveToMemory(compTemplate,
                                                            bytes.data(), maxSize,
                                                            0, &size))) {
                                bytes.truncate(size);
                                // save compressed template to file
                                QFile tplFile(tplPath);
                                tplFile.open(QFile::WriteOnly);
                                tplFile.write( bytes );

                                m_templates.push_back(
                                            FaceTemplatePtr(
                                                new FaceTemplate(
                                                    imgPath,
                                                    tplPath,
                                                    bytes)) );
                            }
                        }
                    }

                } else {
                    if (tpl != 0)
                        qWarning("Leaking a templ that allegedly wasn't loaded");
                }
                NImageFree(greyscale);
            }
            NImageFree(image);
        }
    }
}

VerilookMatchingThread::FaceTemplate::FaceTemplate(
    const QString& imgPath,
    const QString& tplPath,
    const QByteArray& data)
    : m_imgPath(imgPath), m_tplPath(tplPath), m_data(data)
{
}




} // namespace Mirror
