#include "verilookdetectorprivate.h"

#include <QtDebug>
#include <QtCore>

namespace Mirror {

bool VerilookDetectorPrivate::s_gotLicense = false;
int VerilookDetectorPrivate::s_refcount = 0;
const char * VerilookDetectorPrivate::s_defaultPort = "5000";
const char * VerilookDetectorPrivate::s_defaultServer = "/local";
const char * VerilookDetectorPrivate::s_licenseList =
"SingleComputerLicense:VLExtractor,SingleComputerLicense:VLMatcher";

VerilookDetectorPrivate * VerilookDetectorPrivate::make()
{
    if (!obtainLicense())
        return 0;
    s_refcount++;
    return new VerilookDetectorPrivate();
}

VerilookDetectorPrivate::VerilookDetectorPrivate()
    : m_detectEyes( true )
{
    if (! isOk(NleCreate(&m_extractor),
               "No verilook extractor created",
               "Verilook extractor created") ) {
        m_extractor = 0;
    }
}

VerilookDetectorPrivate::~VerilookDetectorPrivate() {
    if (m_extractor) {
        NleFree(m_extractor);
        m_extractor = 0;
    }

    m_templates.clear();

    if (--s_refcount == 0) {
        releaseLicense();
    }
}

bool VerilookDetectorPrivate::obtainLicense()
{
    if (!s_gotLicense) {
        NBool available;

        if ( !isOk( NLicenseObtain(s_defaultServer, s_defaultPort,
                                   s_licenseList, &available),
                   "NLicenseObtain failed") )
            return false;


        if (!available) {
            qWarning("License for %s not available.", s_licenseList);
        } else {
            qDebug("License successfully obtained.");
            s_gotLicense = true;
        }
    }

    return s_gotLicense;
}

void VerilookDetectorPrivate::releaseLicense()
{
    if (!s_gotLicense) return;

    if ( isOk( NLicenseRelease(s_licenseList),
              "NLicenseRelease failed",
              "License successfully released" ) ) {
        s_gotLicense = false;
    }

}

QString VerilookDetectorPrivate::errorString(NResult result)
{
    NInt length;
    NChar* message;
    length = NErrorGetDefaultMessage(result, NULL);
    message = (NChar*) malloc(sizeof(NChar) * (length + 1));
    NErrorGetDefaultMessage(result, message);
    QString qmessage(message);
    free(message);
    return qmessage;
}

bool VerilookDetectorPrivate::isOk(NResult result,
                 QString errorSuffix,
                 QString successMessage) {
    if (NFailed(result)) {
        qCritical()
                << QString("VLERR(%1): %2.%3")
                   .arg(result)
                   .arg(errorString(result))
                   .arg( !errorSuffix.isEmpty()
                        ? (", " + errorSuffix + ".") : "");
        return false;
    } else {
        if (!successMessage.isEmpty())
            qDebug() << successMessage;
        return true;
    }
}


void VerilookDetectorPrivate::process(const cv::Mat& frame, QList<Face>& faces)
{
    HNImage img;
    if ( !isOk( NImageCreateWrapper(
                   npfGrayscale,
                   frame.cols, frame.rows, frame.step.p[0],
                   0.0, 0.0, frame.data, NFalse, &img),
               "Coudn't wrap matrix for verilook"))
        return;

    /* detect faces */
    int faceCount = 0;
    NleFace * vlFaces;

    NleDetectFaces( m_extractor, img, &faceCount, &vlFaces);
    // convert to mirror faces
    for(int i = 0; i<faceCount; ++i) {
        Face face( QRectF(vlFaces[i].Rectangle.X,
                          vlFaces[i].Rectangle.Y,
                          vlFaces[i].Rectangle.Width,
                          vlFaces[i].Rectangle.Height));
        if (m_detectEyes) {
            NleDetectionDetails vlDetails;
            if ( isOk(NleDetectFacialFeatures(m_extractor, img,
                                              &vlFaces[i],
                                              &vlDetails))) {
                if (vlDetails.EyesAvailable == NTrue) {
                    face.setEyes(QPointF(vlDetails.Eyes.First.X,
                                         vlDetails.Eyes.First.Y),
                                 QPointF(vlDetails.Eyes.Second.X,
                                         vlDetails.Eyes.Second.Y));
                }
            }
        }

        faces.push_back( face );
    }

    NFree(vlFaces);
}

void VerilookDetectorPrivate::addDbFace(const QString& imgPath)
{
    QFileInfo fi(imgPath);
    Q_ASSERT( fi.exists() );
    QString tplPath = fi.path() + "/" + fi.baseName() + ".tpl";
    if (QFileInfo(tplPath).exists()) {
        // load from file
        QFile tplFile(tplPath);
        tplFile.open(QFile::ReadOnly);
        m_templates.push_back(FaceTemplatePtr(new FaceTemplate(imgPath, tplPath, tplFile.readAll())));
        //qDebug() << "Successfully loaded template for" << imgPath;
    } else {
        HNImage image, greyscale;
        NleDetectionDetails details;
        HNLTemplate tpl;
        NleExtractionStatus extrStatus;

        if (isOk(NImageCreateFromFile(imgPath.toLocal8Bit(), NULL, &image))) {

            if (isOk(NImageCreateFromImage(npfGrayscale, 0, image, &greyscale))) {
                if (isOk(NleExtract(m_extractor, greyscale,
                             &details, &extrStatus, &tpl))
                        && (extrStatus == nleesTemplateCreated)) {
                    // compress
                    HNLTemplate compTemplate;

                    if (isOk(NleCompressEx(tpl, nletsSmall, &compTemplate))) {

                        // free uncompressed template
                        NLTemplateFree(tpl);

                        // get the size of the template
                        NSizeType maxSize;
                        if (isOk(NLTemplateGetSize(compTemplate, 0, &maxSize))) {

                            // transform to byte array
                            NSizeType size;
                            QByteArray bytes(maxSize, 0);

                            if (isOk(NLTemplateSaveToMemory(compTemplate,
                                                            bytes.data_ptr(), maxSize,
                                                            0, &size))) {
                                bytes.truncate(size);
                                // save compressed template to file
                                QFile tplFile(tplPath);
                                tplFile.open(QFile::WriteOnly);
                                tplFile.write( bytes );

                                m_templates.push_back( FaceTemplatePtr(new FaceTemplate(imgPath, tplPath, bytes)) );
                                //qDebug() << "Successfully created template for" << imgPath;
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


VerilookDetectorPrivate::FaceTemplate::FaceTemplate(
    const QString& imgPath,
    const QString& tplPath,
    const QByteArray& data)
    : m_imgPath(imgPath), m_tplPath(tplPath), m_data(data)
{
}

} // namespace Mirror
