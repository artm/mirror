QT       += core gui network
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
    face.cpp \
    mutextrylocker.cpp

HEADERS  += mirrorwindow.h \
    compositeview.h \
    visionfilter.h \
    util.h \
    footballtracker.h \
    scratchgraphics.h \
    facetracker.h \
    face.h \
    mutextrylocker.h

FORMS    += mirrorwindow.ui

OTHER_FILES += \
    DEVLOG.txt \
    .gitignore

# OpenCV dependencies on a mac (needed when linking against static opencv)
LIBS += -framework AGL -framework OpenGL  -framework Foundation -framework QTKit -framework Cocoa -framework QuartzCore
LIBS += -lopencv_calib3d -lopencv_contrib -lopencv_features2d -lopencv_flann -lopencv_gpu
LIBS += -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_ts -lopencv_video -lopencv_core
