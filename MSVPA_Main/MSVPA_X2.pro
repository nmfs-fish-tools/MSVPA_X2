#-------------------------------------------------
#
# Project created by QtCreator 2017-03-06T13:50:44
#
#-------------------------------------------------

QT       += core gui charts sql datavisualization uitools concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MSVPA_X2
TEMPLATE = app

PRECOMPILED_HEADER = /home/rklasky/workspaceQtCreator/MSVPA/MSVPA_Main/precompiled_header_main.h
CONFIG += precompile_header

CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += -std=c++0x

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


LIBS += -lboost_system -lboost_filesystem

SOURCES += \
    main.cpp \
    nmfMainWindow.cpp \
    PreferencesDialog.cpp

HEADERS  += \
    nmfMainWindow.h \
    PreferencesDialog.h
    #nmfOutputChartStackedBar2.h \
    #nmfOutputChartBase2.h

FORMS    += \
    nmfMainWindow.ui


RESOURCES += \
    resource.qrc \
    qdarkstyle/style.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-nmfGuiComponentsMain-Qt_5_12_3_gcc64-Release/release/ -lnmfGuiComponentsMain
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-nmfGuiComponentsMain-Qt_5_12_3_gcc64-Release/debug/ -lnmfGuiComponentsMain
else:unix: LIBS += -L$$PWD/../../build-nmfGuiComponentsMain-Qt_5_12_3_gcc64-Release/ -lnmfGuiComponentsMain

INCLUDEPATH += $$PWD/../../nmfSharedUtilities/nmfGuiComponentsMain
DEPENDPATH += $$PWD/../../nmfSharedUtilities/nmfGuiComponentsMain

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-nmfGuiDialogs-Qt_5_12_3_gcc64-Release/release/ -lnmfGuiDialogs
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-nmfGuiDialogs-Qt_5_12_3_gcc64-Release/debug/ -lnmfGuiDialogs
else:unix: LIBS += -L$$PWD/../../build-nmfGuiDialogs-Qt_5_12_3_gcc64-Release/ -lnmfGuiDialogs

INCLUDEPATH += $$PWD/../../nmfSharedUtilities/nmfGuiDialogs
DEPENDPATH += $$PWD/../../nmfSharedUtilities/nmfGuiDialogs

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-MSVPA_AlgorithmsSSVPA-Qt_5_12_3_gcc64-Release/release/ -lMSVPA_AlgorithmsSSVPA
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-MSVPA_AlgorithmsSSVPA-Qt_5_12_3_gcc64-Release/debug/ -lMSVPA_AlgorithmsSSVPA
else:unix: LIBS += -L$$PWD/../../build-MSVPA_AlgorithmsSSVPA-Qt_5_12_3_gcc64-Release/ -lMSVPA_AlgorithmsSSVPA

INCLUDEPATH += $$PWD/../MSVPA_AlgorithmsSSVPA
DEPENDPATH += $$PWD/../MSVPA_AlgorithmsSSVPA

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-MSVPA_AlgorithmsMSVPA-Qt_5_12_3_gcc64-Release/release/ -lMSVPA_AlgorithmsMSVPA
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-MSVPA_AlgorithmsMSVPA-Qt_5_12_3_gcc64-Release/debug/ -lMSVPA_AlgorithmsMSVPA
else:unix: LIBS += -L$$PWD/../../build-MSVPA_AlgorithmsMSVPA-Qt_5_12_3_gcc64-Release/ -lMSVPA_AlgorithmsMSVPA

INCLUDEPATH += $$PWD/../MSVPA_AlgorithmsMSVPA
DEPENDPATH += $$PWD/../MSVPA_AlgorithmsMSVPA

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-MSVPA_AlgorithmsForecast-Qt_5_12_3_gcc64-Release/release/ -lMSVPA_AlgorithmsForecast
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-MSVPA_AlgorithmsForecast-Qt_5_12_3_gcc64-Release/debug/ -lMSVPA_AlgorithmsForecast
else:unix: LIBS += -L$$PWD/../../build-MSVPA_AlgorithmsForecast-Qt_5_12_3_gcc64-Release/ -lMSVPA_AlgorithmsForecast

INCLUDEPATH += $$PWD/../MSVPA_AlgorithmsForecast
DEPENDPATH += $$PWD/../MSVPA_AlgorithmsForecast

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-MSVPA_GuiCharts-Qt_5_12_3_gcc64-Release/release/ -lMSVPA_GuiCharts
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-MSVPA_GuiCharts-Qt_5_12_3_gcc64-Release/debug/ -lMSVPA_GuiCharts
else:unix: LIBS += -L$$PWD/../../build-MSVPA_GuiCharts-Qt_5_12_3_gcc64-Release/ -lMSVPA_GuiCharts

INCLUDEPATH += $$PWD/../MSVPA_GuiCharts
DEPENDPATH += $$PWD/../MSVPA_GuiCharts


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-MSVPA_GuiOutput-Qt_5_12_3_gcc64-Release/release/ -lMSVPA_GuiOutput
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-MSVPA_GuiOutput-Qt_5_12_3_gcc64-Release/debug/ -lMSVPA_GuiOutput
else:unix: LIBS += -L$$PWD/../../build-MSVPA_GuiOutput-Qt_5_12_3_gcc64-Release/ -lMSVPA_GuiOutput

INCLUDEPATH += $$PWD/../MSVPA_GuiOutput
DEPENDPATH += $$PWD/../MSVPA_GuiOutput

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-MSVPA_GuiSetup-Qt_5_12_3_gcc64-Release/release/ -lMSVPA_GuiSetup
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-MSVPA_GuiSetup-Qt_5_12_3_gcc64-Release/debug/ -lMSVPA_GuiSetup
else:unix: LIBS += -L$$PWD/../../build-MSVPA_GuiSetup-Qt_5_12_3_gcc64-Release/ -lMSVPA_GuiSetup

INCLUDEPATH += $$PWD/../MSVPA_GuiSetup
DEPENDPATH += $$PWD/../MSVPA_GuiSetup

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-MSVPA_GuiForecast-Qt_5_12_3_gcc64-Release/release/ -lMSVPA_GuiForecast
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-MSVPA_GuiForecast-Qt_5_12_3_gcc64-Release/debug/ -lMSVPA_GuiForecast
else:unix: LIBS += -L$$PWD/../../build-MSVPA_GuiForecast-Qt_5_12_3_gcc64-Release/ -lMSVPA_GuiForecast

INCLUDEPATH += $$PWD/../MSVPA_GuiForecast
DEPENDPATH += $$PWD/../MSVPA_GuiForecast

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-MSVPA_GuiMSVPA-Qt_5_12_3_gcc64-Release/release/ -lMSVPA_GuiMSVPA
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-MSVPA_GuiMSVPA-Qt_5_12_3_gcc64-Release/debug/ -lMSVPA_GuiMSVPA
else:unix: LIBS += -L$$PWD/../../build-MSVPA_GuiMSVPA-Qt_5_12_3_gcc64-Release/ -lMSVPA_GuiMSVPA

INCLUDEPATH += $$PWD/../MSVPA_GuiMSVPA
DEPENDPATH += $$PWD/../MSVPA_GuiMSVPA

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-MSVPA_GuiSSVPA-Qt_5_12_3_gcc64-Release/release/ -lMSVPA_GuiSSVPA
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-MSVPA_GuiSSVPA-Qt_5_12_3_gcc64-Release/debug/ -lMSVPA_GuiSSVPA
else:unix: LIBS += -L$$PWD/../../build-MSVPA_GuiSSVPA-Qt_5_12_3_gcc64-Release/ -lMSVPA_GuiSSVPA

INCLUDEPATH += $$PWD/../MSVPA_GuiSSVPA
DEPENDPATH += $$PWD/../MSVPA_GuiSSVPA


DISTFILES += \
    icons/whatsthis.png \
    icons/new.png \
    icons/new_old.png \
    icons/new_old2.png \
    icons/scrgrb.png \
    icons/NOAA.png


