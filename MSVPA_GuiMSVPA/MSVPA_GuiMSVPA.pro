#-------------------------------------------------
#
# Project created by QtCreator 2020-05-11T21:04:42
#
#-------------------------------------------------

QT       += core gui charts sql datavisualization uitools
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MSVPA_GuiMSVPA
TEMPLATE = lib
CONFIG += c++14

DEFINES += MSVPA_GUIMSVPA_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    nmfMSVPATab01.cpp \
    nmfMSVPATab02.cpp \
    nmfMSVPATab03.cpp \
    nmfMSVPATab04.cpp \
    nmfMSVPATab05.cpp \
    nmfMSVPATab06.cpp \
    nmfMSVPATab07.cpp \
    nmfMSVPATab08.cpp \
    nmfMSVPATab09.cpp \
    nmfMSVPATab10.cpp \
    nmfMSVPATab11.cpp \
    nmfMSVPATab12.cpp

HEADERS += \
    nmfMSVPATab01.h \
    nmfMSVPATab02.h \
    nmfMSVPATab03.h \
    nmfMSVPATab04.h \
    nmfMSVPATab05.h \
    nmfMSVPATab06.h \
    nmfMSVPATab07.h \
    nmfMSVPATab08.h \
    nmfMSVPATab09.h \
    nmfMSVPATab10.h \
    nmfMSVPATab11.h \
    nmfMSVPATab12.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}



win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-nmfDatabase-Qt_5_12_3_gcc64-Release/release/ -lnmfDatabase
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-nmfDatabase-Qt_5_12_3_gcc64-Release/debug/ -lnmfDatabase
else:unix: LIBS += -L$$PWD/../../build-nmfDatabase-Qt_5_12_3_gcc64-Release/ -lnmfDatabase

INCLUDEPATH += $$PWD/../../nmfSharedUtilities/nmfDatabase
DEPENDPATH += $$PWD/../../nmfSharedUtilities/nmfDatabase

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-nmfUtilities-Qt_5_12_3_gcc64-Release/release/ -lnmfUtilities
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-nmfUtilities-Qt_5_12_3_gcc64-Release/debug/ -lnmfUtilities
else:unix: LIBS += -L$$PWD/../../build-nmfUtilities-Qt_5_12_3_gcc64-Release/ -lnmfUtilities

INCLUDEPATH += $$PWD/../../nmfSharedUtilities/nmfUtilities
DEPENDPATH += $$PWD/../../nmfSharedUtilities/nmfUtilities

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-nmfDataModels-Qt_5_12_3_gcc64-Release/release/ -lnmfDataModels
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-nmfDataModels-Qt_5_12_3_gcc64-Release/debug/ -lnmfDataModels
else:unix: LIBS += -L$$PWD/../../build-nmfDataModels-Qt_5_12_3_gcc64-Release/ -lnmfDataModels

INCLUDEPATH += $$PWD/../../nmfSharedUtilities/nmfDataModels
DEPENDPATH += $$PWD/../../nmfSharedUtilities/nmfDataModels
