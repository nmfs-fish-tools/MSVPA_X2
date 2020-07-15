#-------------------------------------------------
#
# Project created by QtCreator 2020-05-11T20:35:15
#
#-------------------------------------------------

QT       +=  core gui charts sql datavisualization uitools
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MSVPA_GuiSSVPA
TEMPLATE = lib
CONFIG += c++14

DEFINES += MSVPA_GUISSVPA_LIBRARY

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
    nmfSSVPATab01.cpp \
    nmfSSVPATab02.cpp \
    nmfSSVPATab03.cpp \
    nmfSSVPATab04.cpp

HEADERS += \
    nmfSSVPATab01.h \
    nmfSSVPATab02.h \
    nmfSSVPATab03.h \
    nmfSSVPATab04.h

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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-MSVPA_AlgorithmsSSVPA-Qt_5_12_3_gcc64-Release/release/ -lMSVPA_AlgorithmsSSVPA
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-MSVPA_AlgorithmsSSVPA-Qt_5_12_3_gcc64-Release/debug/ -lMSVPA_AlgorithmsSSVPA
else:unix: LIBS += -L$$PWD/../../build-MSVPA_AlgorithmsSSVPA-Qt_5_12_3_gcc64-Release/ -lMSVPA_AlgorithmsSSVPA

INCLUDEPATH += $$PWD/../MSVPA_AlgorithmsSSVPA
DEPENDPATH += $$PWD/../MSVPA_AlgorithmsSSVPA
