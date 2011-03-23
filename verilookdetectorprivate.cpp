#include "verilookdetectorprivate.h"

#include <QtDebug>

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

} // namespace Mirror
