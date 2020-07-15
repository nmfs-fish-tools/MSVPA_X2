#-------------------------------------------------
#
# Project created by QtCreator 2020-05-11T22:21:04
#
#-------------------------------------------------

QT       += core gui charts sql datavisualization uitools

TARGET = MSVPA_GuiCharts
TEMPLATE = lib
PRECOMPILED_HEADER = /home/rklasky/workspaceQtCreator/MSVPA/MSVPA_Main/precompiled_header.h
CONFIG += precompile_header

CONFIG += c++14
DEFINES += MSVPA_GUICHARTS_LIBRARY

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
    ChartBar.cpp \
    ChartBarConsumptionRates.cpp \
    ChartBarFoodAvailability.cpp \
    ChartBarGrowth.cpp \
    ChartBarMortalityRates.cpp \
    ChartBarPopulationSize.cpp \
    ChartLine.cpp \
    ChartLineMultispeciesPopulations.cpp \
    ChartLineYieldPerRecruit.cpp \
    ChartStackedBar.cpp \
    ChartStackedBarDietComposition.cpp \
    MSVPAX2OutputChart.cpp \
    nmfMSVPAProgressWidget.cpp \
    nmfOutputChart3DBarModifier.cpp

HEADERS += \
    ChartBar.h \
    ChartBarConsumptionRates.h \
    ChartBarFoodAvailability.h \
    ChartBarGrowth.h \
    ChartBarMortalityRates.h \
    ChartBarPopulationSize.h \
    ChartLine.h \
    ChartLineMultispeciesPopulations.h \
    ChartLineYieldPerRecruit.h \
    ChartStackedBar.h \
    ChartStackedBarDietComposition.h \
    MSVPAX2OutputChart.h \
    nmfMSVPAProgressWidget.h \
    nmfOutputChart3DBarModifier.h

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

