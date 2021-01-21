#ifndef NMFFORECASTTAB4_H
#define NMFFORECASTTAB4_H

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>
#include <QTableWidget>
#include <QTableView>
#include <QUiLoader>

#include "nmfDatabase.h"
#include "nmfUtils.h"
#include "nmfLogger.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/multi_array.hpp>

typedef boost::multi_array<double, 3> Boost3DArrayDouble;
typedef boost::multi_array<double, 4> Boost4DArrayDouble;
typedef Boost3DArrayDouble::index index3D;
typedef Boost4DArrayDouble::index index4D;



class nmfForecastTab4: public QObject
{
    Q_OBJECT

    nmfLogger    *logger;

    QTabWidget   *Forecast_Tabs;
    QWidget      *Forecast_Tab4_Widget;
    QPushButton  *Forecast_Tab4_PrevPB;
    QPushButton  *Forecast_Tab4_NextPB;
    QPushButton  *Forecast_Tab4_Load1PB;
    QPushButton  *Forecast_Tab4_Load2PB;
    QPushButton  *Forecast_Tab4_Load3PB;
    QPushButton  *Forecast_Tab4_Load4PB;
    QRadioButton *Forecast_Tab4_Load1RB;
    QRadioButton *Forecast_Tab4_Load2RB;
    QCheckBox    *Forecast_Tab4_Load1CB;
    QCheckBox    *Forecast_Tab4_Load2CB;
    QCheckBox    *Forecast_Tab4_Load3CB;
    QCheckBox    *Forecast_Tab4_Load4CB;
    QComboBox    *Forecast_Tab4_SpeciesCMB;
    QLabel       *Forecast_Tab4_SpeciesLBL;
    QLabel       *Forecast_Tab4_ScenarioLBL;
    QLabel       *Forecast_Tab4_Load1ArrowLBL;
    QLabel       *Forecast_Tab4_Load2ArrowLBL;
    QLabel       *Forecast_Tab4_Load3ArrowLBL;
    QLabel       *Forecast_Tab4_Load4ArrowLBL;
    QComboBox    *Forecast_Tab4_ScenarioCMB;
    QTableWidget *Forecast_Tab4_DataTW;
    QPushButton  *Forecast_Tab4_FillSQPB;
    QPushButton  *Forecast_Tab4_UnFillSQPB;
    QPushButton  *Forecast_Tab4_AddScPB;
    QPushButton  *Forecast_Tab4_DelScPB;
    QListView    *Forecast_ScenarioLV;
    QPushButton  *Forecast_Tab4_ClearPB;
    QPushButton  *Forecast_Tab4_SavePB;
    //QPushButton  *Forecast_Tab4_LoadPB;

    nmfDatabase *databasePtr;
    std::string MSVPAName;
    std::string ForecastName;
    std::string ProjectDir;
    std::vector<Scenarios> ScenarioList;
    std::vector<std::string> PredList;
    std::vector<int> NPredAge;
    int ForecastFirstYear;
    int ForecastNYears;
    int MSVPAFirstYear;
    int NSeasons;
    int MSVPANYears;
    int varButtonPressed;
    Boost3DArrayDouble TableData; // Holds the F or Catch data associated with first button

signals:
    void TableDataChanged(std::string tableName);
    void LoadDataTable(std::string MSVPAName, std::string ForecastName,
                       std::string model, std::string table,
                       std::string originSection, int originTab);
    void SelectTab(std::string returnToSection, int returnToTab);
    void ReloadForecast(std::string tab);
    void MarkForecastAsClean();

public:
    std::string ScenarioName;

    nmfForecastTab4(QTabWidget  *tabs,
                    nmfLogger   *logger,
                    QListView   *theScenarioLV,
                    std::string &theProjectDir);
    virtual ~nmfForecastTab4();
    void clearWidgets();
    void updateTable(int rowOffset,
                     std::vector<std::string> &headerList);
    void load(nmfDatabase *databasePtr,
              std::string MSVPAName,
              std::string ForecastName,
              int theForecastFirstYear,
              int theForecastNYears,
              int theMSVPAFirstYear,
              int theMSVPALastYear);
    void createScenario(std::string newScenarioName);
    void deleteScenario(std::string scenarioName);
    void updateMainWindowScenarioList();
    void refresh();
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);    
    void Save_Scenarios();
    void Save_ScenarioF();
    void Save_ScenarioOthPred();
    void Save_ScenarioOthPrey();
    void Save_ScenarioRec();
    void hideArrows();
    void processRadioButton();
    void MarkAsDirty(std::string tableName);
    void MarkAsClean();
    void updateProjectDir(std::string newProjectDir);
    std::string getConfigData();

public Q_SLOTS:
    void callback_Forecast_Tab4_PrevPB(bool unused);
    void callback_Forecast_Tab4_NextPB(bool unused);
    void callback_Forecast_Tab4_Load1RB();
    void callback_Forecast_Tab4_Load2RB();
    void callback_Forecast_Tab4_Load1CB(int state);
    void callback_Forecast_Tab4_Load2CB(int state);
    void callback_Forecast_Tab4_Load3CB(int state);
    void callback_Forecast_Tab4_Load4CB(int state);
    void callback_Forecast_Tab4_Load1PB(bool unused);
    void callback_Forecast_Tab4_Load2PB(bool unused);
    void callback_Forecast_Tab4_Load3PB(bool unused);
    void callback_Forecast_Tab4_Load4PB(bool unused);
    void callback_Forecast_Tab4_ScenarioCMB(int index);
    void callback_Forecast_Tab4_SpeciesCMB(int index);
    void callback_Forecast_Tab4_UnFillSQPB(bool unused);
    void callback_Forecast_Tab4_FillSQPB(bool unused);
    void callback_Forecast_Tab4_RecCBs(int state);
    void callback_Forecast_Tab4_AddScPB(bool unused);
    void callback_Forecast_Tab4_DelScPB(bool unused);
    void callback_Forecast_Tab4_SavePB(bool unused);
    void callback_TableItemChanged(QTableWidgetItem *item);

};

#endif // NMFFORECASTTAB4_H
