#include "mirrorwindow.h"
#include "ui_mirrorwindow.h"

#include "footballtracker.h"
#include "facetracker.h"
#include "verilookdetector.h"

#include <QtDebug>
#include <QFileInfo>
#include <QResizeEvent>
#include <QUrl>

MirrorWindow::MirrorWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MirrorWindow)
    , m_camera(0)
    , m_metro(this)
{
    ui->setupUi(this);

    QGraphicsScene * scene = new QGraphicsScene(ui->composite);
    ui->composite->setScene( scene );

    connect( ui->composite, SIGNAL(fileDrop(const QMimeData*)), SLOT(loadFile(const QMimeData*)));

    connect( ui->actionCapture, SIGNAL(toggled(bool)), SLOT(setCapture(bool)) );
    connect( ui->actionFullscreen, SIGNAL(toggled(bool)), SLOT(setFullscreen(bool)) );

    connect( &m_metro, SIGNAL(timeout()), SLOT(tick()) );

    //m_filter = new Mirror::FaceTracker( ui->composite );
    m_filter = new Mirror::FootballTracker( ui->composite );
    //m_filter = new Mirror::VerilookDetector( ui->composite, this );

    connect( this, SIGNAL(incomingFrame(const cv::Mat&)), m_filter, SLOT(incomingFrame(const cv::Mat&)) );

    foreach(QString stage, m_filter->slotsOrder()) {
        ui->stageSelector->addItem(stage);
    }
    connect( ui->stageSelector, SIGNAL(currentIndexChanged(QString)), m_filter, SLOT(setVisibleSlot(QString)) );
    m_filter->configureGUI( ui );
}

MirrorWindow::~MirrorWindow()
{
    delete ui;
    if (m_camera)
        delete m_camera;
}

void MirrorWindow::loadFile(const QString& path)
{
    m_camera = new cv::VideoCapture(path.toStdString());
    setCapture(true);
}

void MirrorWindow::loadFile(const QMimeData* mimeData)
{
    qDebug() << "URLs: " << mimeData->urls();

    if (m_camera) {
        setCapture(false);
        delete m_camera;
    }
    loadFile(mimeData->urls()[0].path());
}

void MirrorWindow::setCapture(bool on)
{
    if (on) {
        if (!m_camera)
            m_camera = new cv::VideoCapture(0);
        m_metro.start(50);
    } else {
        m_metro.stop();
    }
}

void MirrorWindow::tick()
{
    cv::Mat input;
    *m_camera >> input;

    ui->composite->setSceneRect(0,0,input.cols,input.rows);

    emit incomingFrame(input);
}

void MirrorWindow::setFullscreen(bool on)
{
    if (on) {
        hide();
        ui->mirrorDock->hide();
        ui->statusBar->hide();
        ui->composite->setFrameShape( QFrame::NoFrame );
        showFullScreen();
    } else {
        hide();
        ui->composite->setFrameShape( QFrame::StyledPanel );
        ui->mirrorDock->show();
        ui->statusBar->show();
        showNormal();
    }
}

void MirrorWindow::keyReleaseEvent(QKeyEvent * event)
{
    if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9) {
        int number = event->key() - Qt::Key_1;
        if (number < ui->stageSelector->count()) {
            ui->stageSelector->setCurrentIndex( number );
        }
        event->accept();
    } else {
        event->ignore();
    }
}
