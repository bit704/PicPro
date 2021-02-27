#-------------------------------------------------
#
# Project created by QtCreator 2020-12-08T22:13:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

RC_FILE = res.rc

TARGET = PicPro
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    qcustomplot.cpp \
    minus.cpp \
    bmp2txt.cpp \
    pedestrian.cpp \
    transform.cpp \
    chromatic.cpp \
    convert.cpp \
    enhance.cpp \
    segmentation.cpp \
    compress.cpp

HEADERS += \
        mainwindow.h \
    qcustomplot.h \
    minus.h \
    bmp2txt.h \
    pedestrian.h \
    transform.h \
    chromatic.h \
    convert.h \
    enhance.h \
    segmentation.h \
    compress.h

FORMS += \
        mainwindow.ui \
    minus.ui \
    bmp2txt.ui \
    pedestrian.ui \
    transform.ui \
    chromatic.ui \
    convert.ui \
    enhance.ui \
    segmentation.ui \
    compress.ui

INCLUDEPATH += $$PWD/include


LIBS += $$PWD/lib/libopencv_*.a \
           $$PWD/lib/libdlib.a

RESOURCES += \
    resources.qrc

DISTFILES += \
    --master/README.md \
    --master/720.480.avi \
    --master/qingshi1.avi

CONFIG += resources_big
