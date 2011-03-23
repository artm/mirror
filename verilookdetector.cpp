#include "verilookdetector.h"
#include "ui_mirrorwindow.h"
#include "face.h"
#include "verilookdetectorprivate.h"
#include "util.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QtDebug>
#include <QtGui>

namespace Mirror {

VerilookDetector::VerilookDetector(Mirror::CompositeView * canvas, QObject *parent)
    : VisionFilter(canvas, parent)
    , m_private( VerilookDetectorPrivate::make() )
{
    m_scratch = new ScratchGraphics();
    canvas->scene()->addItem(m_scratch);

    appendVideoSlot("greyscale", &m_grey);
    appendVideoSlot("stretched contrast", &m_normalized);

    m_loadTimer = new QTimer(this);
    connect(m_loadTimer,SIGNAL(timeout()), SLOT(loadNextFace()));
    m_loadTimer->start(0);
}

VerilookDetector::~VerilookDetector()
{
    if (m_private) {
        delete m_private;
        m_private = 0;
    }
}

VerilookDetector::DbLoader::DbLoader()
    : m_dir(findResourceFile("faces_db/faces"))
{
    QStringList nameFilter;
    nameFilter << "*.jpg";
    m_fileList = m_dir.entryList(nameFilter,QDir::Files,QDir::Name);
}

QString VerilookDetector::DbLoader::next()
{
    if (m_fileList.size()) {
        QString res = m_fileList.front();
        m_fileList.pop_front();
        return m_dir.filePath(res);
    } else
        return QString::null;
}


void VerilookDetector::loadNextFace()
{
    QString fname = m_dbLoader.next();
    if (fname.isNull()) {
        m_loadTimer->stop();
    } else {
        m_private->addDbFace(fname);
    }
}

void VerilookDetector::configureGUI(Ui::MirrorWindow * ui)
{
    /* Add controls and connect their signals to our slots and v.v.*/
    QFormLayout * layout = static_cast<QFormLayout *>
            (ui->mirrorDockContents->layout());

    QPushButton * eyes = new QPushButton("Eyes");
    eyes->setCheckable(true);
    eyes->setChecked(detectEyes());
    eyes->setFlat(true);
    layout->addRow(eyes);
    connect(eyes, SIGNAL(toggled(bool)), SLOT(setDetectEyes(bool)));
}

void VerilookDetector::filter(const cv::Mat& frame)
{
    if (!m_private)
        return;

    // grey will have the same dimensions as input
    cv::cvtColor( frame, m_grey, CV_RGB2GRAY);
    cv::normalize( m_grey, m_normalized, 0, 255, cv::NORM_MINMAX );

    QList<Face> faces;
    m_private->process( m_grey, faces );

    m_scratch->clear();
    foreach(Face f, faces) {
        if (f.hasFace())
            m_scratch->addRect( f.face() );
        if (f.hasEyes()) {
            float size = std::max(5.0, f.face().width() / 10.0);
            m_scratch->addRect( QRectF(f.eye(0) - QPointF(size/2.,size/2.),
                                       QSize(size,size) ) );
            m_scratch->addRect( QRectF(f.eye(1) - QPointF(size/2.,size/2.),
                                       QSize(size,size) ) );
        }
    }

}

bool VerilookDetector::detectEyes() const
{
    return m_private->detectEyes();
}

void VerilookDetector::setDetectEyes(bool on)
{
    m_private->setDetectEyes(on);
}

} // namespace Mirror

