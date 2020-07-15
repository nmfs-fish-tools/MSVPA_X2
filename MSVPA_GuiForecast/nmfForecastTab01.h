#ifndef NMFFORECASTTAB1_H
#define NMFFORECASTTAB1_H

#include <QCheckBox>
#include <QComboBox>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QTableWidget>
#include <QUiLoader>
#include <QValidator>

#include "nmfDatabase.h"
#include "nmfLogger.h"

class nmfForecastTab1: public QObject
{
    Q_OBJECT

    int FirstYear;
    int LastYear;
    nmfDatabase *databasePtr;
    QTabWidget  *Forecast_Tabs;
    QWidget     *Forecast_Tab1_Widget;
    QPushButton *Forecast_Tab1_NextPB;
    QPushButton *Forecast_Tab1_SavePB;
    QLineEdit   *Forecast_Tab1_MsvpaNameLE;
    QLineEdit   *Forecast_Tab1_ForecastNameLE;
    QLineEdit   *Forecast_Tab1_ScenarioNameLE;
    QLabel      *Forecast_Tab1_ScenarioNameLBL;
    QComboBox   *Forecast_Tab1_InitialYearCMB;
    QLineEdit   *Forecast_Tab1_NumYearsLE;
    QCheckBox   *Forecast_Tab1_PredatorGrowthCB;
    nmfLogger   *logger;
    std::string  ProjectDir;

public:

    nmfForecastTab1(QTabWidget *tabs,
                    nmfLogger *logger,
                    std::string &theProjectDir);
    virtual ~nmfForecastTab1();
    void clearWidgets();
    void loadWidgets(nmfDatabase *databasePtr,
                     std::string MSVPAName,
                     std::string ForecastName,
                     int FirstYear, int LastYear);
    void setupNewForecast(nmfDatabase *databasePtr,
                          std::string MSVPAName);
    void saveTheForecast(std::string MSVPAName,
                         std::string Forecast,
                         std::string Scenario,
                         int InitYear,
                         int NumYears,
                         int ModelPredatorGrowth);
    void refresh(std::string MSVPAName,
                 std::string ForecastName,
                 std::string ScenarioName);
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void MarkAsDirty(std::string tableName);
    void MarkAsClean();
    void updateProjectDir(std::string newProjectDir);

signals:
    void TableDataChanged(std::string tableName);
    void UpdateScenarioList(std::string ScenarioToSelect);
    void MarkForecastAsClean();
    void ReloadForecast(std::string tab);

public Q_SLOTS:
    void callback_Forecast_Tab1_NextPB(bool unused);
    void callback_TableDataChanged();
    void callback_TableDataChanged(int unused);
    void callback_Forecast_Tab1_SavePB(bool unused);
    void callback_StripMsvpaNameLE();
    void callback_StripForecastNameLE();
    void callback_StripScenarioNameLE();
    void callback_StripNumYearsLE();

};

#endif // NMFFORECASTTAB1_H
