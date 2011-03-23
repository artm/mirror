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
    verilookdetectorprivate.cpp

HEADERS  += mirrorwindow.h \
    compositeview.h \
    visionfilter.h \
    util.h \
    footballtracker.h \
    scratchgraphics.h \
    facetracker.h \
    verilookdetector.h \
    face.h \
    verilookdetectorprivate.h

FORMS    += mirrorwindow.ui

OTHER_FILES += \
    MEMO.txt \
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

LIBS += -LShoulders/OpenCV-2.2.0/build/3rdparty/lib -llibjasper -llibjpeg -llibpng -llibtiff -lopencv_lapack -lzlib
LIBS += -LShoulders/OpenCV-2.2.0/build/lib

LIBS += -lopencv_calib3d -lopencv_contrib -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_haartraining_engine
LIBS += -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_ts -lopencv_video -lopencv_core
# OpenCV dependencies on a mac (needed when linking against static opencv)
macx:LIBS += -framework AGL -framework OpenGL  -framework Foundation -framework QTKit -framework Cocoa -framework QuartzCore

# Verilook
# TODO: make this optional (football tracker shouldn't depend on verilook)
# or make vision filters plugins loadable at run-time... (much later)
macx:INCLUDEPATH += /Users/artm/SDK/VeriLook_4_0_Standard_SDK/include/MacOSX
macx:LIBS += -L/Library/Frameworks/Neurotechnology -lNExtractors -lNMatchers -lNTemplates -lNCore -lNImages -lNLicensing

RESOURCES +=
