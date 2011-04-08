#-------------------------------------------------
#
# Project created by QtCreator 2011-03-04T15:50:11
#
#-------------------------------------------------

QT       += core gui
TARGET = Mirror
TEMPLATE = app

SOURCES += main.cpp\
        mirrorwindow.cpp \
    compositeview.cpp \
    visionfilter.cpp \
    util.cpp \
    footballtracker.cpp \
    scratchgraphics.cpp \
    facetracker.cpp \
    verilookdetector.cpp \
    face.cpp \
    verilookdetectorprivate.cpp \
    verilookmatchingthread.cpp \
    mutextrylocker.cpp

HEADERS  += mirrorwindow.h \
    compositeview.h \
    visionfilter.h \
    util.h \
    footballtracker.h \
    scratchgraphics.h \
    facetracker.h \
    verilookdetector.h \
    face.h \
    verilookdetectorprivate.h \
    verilookmatchingthread.h \
    mutextrylocker.h

FORMS    += mirrorwindow.ui

OTHER_FILES += \
    DEVLOG.txt \
    .gitignore

INCLUDEPATH += Shoulders/OpenCV-2.2.0/include/opencv
INCLUDEPATH += Shoulders/OpenCV-2.2.0/modules/calib3d/include
INCLUDEPATH += Shoulders/OpenCV-2.2.0/modules/contrib/include
INCLUDEPATH += Shoulders/OpenCV-2.2.0/modules/core/include
INCLUDEPATH += Shoulders/OpenCV-2.2.0/modules/features2d/include
INCLUDEPATH += Shoulders/OpenCV-2.2.0/modules/flann/include
INCLUDEPATH += Shoulders/OpenCV-2.2.0/modules/gpu/include
INCLUDEPATH += Shoulders/OpenCV-2.2.0/modules/highgui/include
INCLUDEPATH += Shoulders/OpenCV-2.2.0/modules/imgproc/include
INCLUDEPATH += Shoulders/OpenCV-2.2.0/modules/legacy/include
INCLUDEPATH += Shoulders/OpenCV-2.2.0/modules/ml/include
INCLUDEPATH += Shoulders/OpenCV-2.2.0/modules/objdetect/include
INCLUDEPATH += Shoulders/OpenCV-2.2.0/modules/video/include

macx {
    LIBS += -LShoulders/OpenCV-2.2.0/build/3rdparty/lib
    LIBS += -LShoulders/OpenCV-2.2.0/build/lib

    # OpenCV dependencies on a mac (needed when linking against static opencv)
    LIBS += -framework AGL -framework OpenGL  -framework Foundation -framework QTKit -framework Cocoa -framework QuartzCore

    # Verilook
    # TODO: make this optional (football tracker shouldn't depend on verilook)
    # or make vision filters plugins loadable at run-time... (much later)
    INCLUDEPATH += /Users/artm/SDK/VeriLook_4_0_Standard_SDK/include/MacOSX
    LIBS += -L/Library/Frameworks/Neurotechnology
}

LIBS += -llibjasper -llibjpeg -llibpng -llibtiff -lopencv_lapack -lzlib
LIBS += -lopencv_calib3d -lopencv_contrib -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_haartraining_engine
LIBS += -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_ts -lopencv_video -lopencv_core
LIBS += -lNExtractors -lNMatchers -lNTemplates -lNCore -lNImages -lNLicensing
