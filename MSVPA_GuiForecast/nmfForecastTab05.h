#ifndef NMFFORECASTTAB5_H
#define NMFFORECASTTAB5_H

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QObject>
#include <QProgressBar>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QUiLoader>

#include "nmfDatabase.h"
#include "nmfLogger.h"

class nmfForecastTab5: public QObject
{
    Q_OBJECT

    nmfLogger    *logger;
    QTabWidget   *Forecast_Tabs;
    QWidget      *Forecast_Tab5_Widget;
    QPushButton  *Forecast_Tab5_PrevPB;
    QPushButton  *Forecast_Tab5_RunPB;
    QTextEdit    *Forecast_Tab5_CurrentConfigurationTE;
    QProgressBar *Forecast_Tab5_ProgressBarPBR;
    QLabel       *Forecast_Tab5_ProgressBarLBL;


public:

    nmfForecastTab5(QTabWidget *tabs,
                    nmfLogger  *logger);
    virtual ~nmfForecastTab5();
    void clearWidgets();
    void makeCurrent();
    void loadWidgets(std::string MSVPAName,
                     std::string ForecastName,
                     std::string ScenarioName,
                     int InitYear,
                     int NYears,
                     bool isGrowthModel,
                     std::string &configInfo);

signals:
    void RunForecast();

public Q_SLOTS:
    void callback_Forecast_Tab5_PrevPB(bool);
    void callback_Forecast_Tab5_RunPB(bool);

};

#endif // NMFFORECASTTAB5_H
