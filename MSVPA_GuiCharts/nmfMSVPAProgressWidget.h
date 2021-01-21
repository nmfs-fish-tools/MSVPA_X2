
/**
 @file nmfMSVPAProgressWidget.h
 @author rklasky
 @copyright 2017 NOAA - National Marine Fisheries Service
 @brief This file contains the definition for the Progress Chart dock widget.
 @date Feb 16, 2018
 */

#ifndef NMFMSVPAPROGRESSWIDGET_H
#define NMFMSVPAPROGRESSWIDGET_H

#include <QtCharts/QChartView>
#include <QtCharts/QChart>

#include <QAbstractSeries>
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLineSeries>
#include <QObject>
#include <QPushButton>
#include <QMessageBox>
#include <QScatterSeries>
#include <QSlider>
#include <QSpinBox>
#include <QSpacerItem>
#include <QTimer>
#include <QValueAxis>

#include "nmfUtils.h"
#include "nmfLogger.h"

//#include "nmfguicomponentsmain_global.h"


#include <set>

QT_CHARTS_USE_NAMESPACE

class nmfMSVPAProgressWidget : public QObject

{

    Q_OBJECT

private:
    nmfLogger   *logger;

public:

    QChartView  *chartView;
    QChart      *chart;
    QGroupBox   *controlsGB;
    QHBoxLayout *hMainLayt;
    QVBoxLayout *vChartLayt;
    QHBoxLayout *hMinLayt;
    QHBoxLayout *hMaxLayt;
    QHBoxLayout *hRangeLayt;
    QVBoxLayout *vGroupLayt;
    QHBoxLayout *buttonLayt;
    QLabel      *statusLBL;
    QLabel      *pointLBL;
    QLabel      *rangeLBL;
    QLabel      *minLBL;
    QLabel      *maxLBL;
    QCheckBox   *labelsCB;
    QSpinBox    *minSB;
    QSpinBox    *maxSB;
    QPushButton *clearPB;
    QPushButton *stopPB;
    QTimer      *timer;

    std::string RunType;

    nmfMSVPAProgressWidget(QTimer *theTimer,
                      nmfLogger *logger,
                      std::string runType,
                      QString Title,
                      QString XAxisLabel,
                      QString YAxisLabel);
   ~nmfMSVPAProgressWidget();

    void SetupConnections();
    void StopRun();
    void StartRun();
    bool isStopped();
    void startTimer();
    void updateChart();
    void readChartDataFile(std::string type,
                           std::string inputFileName,
                           std::string inputLabelFileName,
                           std::string title,
                           std::string xLabel,
                           std::string yLabel);
    void updateChartDataLabel(std::string inputLabelFileName,
                              std::string overrideMsg);

public Q_SLOTS:
    void callback_stopPB(bool unused);
    void callback_clearPB(bool unused);
    void callback_labelsCB(int state);
    void callback_minSB(int value);
    void callback_maxSB(int value);
    void callback_lineHovered(QPointF point, bool state);
    void callback_scatterSeriesHovered(QPointF point, bool state);
};


#endif // NMFMSVPAPROGRESSWIDGET_H
