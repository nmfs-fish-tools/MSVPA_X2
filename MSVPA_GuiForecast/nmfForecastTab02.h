#ifndef NMFFORECASTTAB2_H
#define NMFFORECASTTAB2_H

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QModelIndex>
#include <QObject>
#include <QPushButton>
#include <QTableWidget>
#include <QUiLoader>

#include "nmfDatabase.h"
#include "nmfVonBertModel.h"
#include "nmfLogger.h"



class nmfForecastTab2: public QObject
{
    Q_OBJECT

    QTabWidget  *Forecast_Tabs;
    QWidget     *Forecast_Tab2_Widget;
    QPushButton *Forecast_Tab2_PrevPB;
    QPushButton *Forecast_Tab2_NextPB;
    QPushButton *Forecast_Tab2_LoadPB;
    QPushButton *Forecast_Tab2_SavePB;
    QTableView  *Forecast_Tab2_GrowthParametersTV;

    nmfVonBertModel *vonBert_model;

    std::string MSVPAName;
    std::string ForecastName;
    nmfDatabase *databasePtr;
    std::string ProjectDir;
    nmfLogger   *logger;

public:

    nmfForecastTab2(QTabWidget  *tabs,
                    nmfLogger   *logger,
                    std::string &theProjectDir);
    virtual ~nmfForecastTab2();
    void clearWidgets();
    void loadWidgets(
            nmfDatabase *databasePtr,
            nmfVonBertModel &vonBert_model,
            std::string MSVPAName,
            std::string ForecastName);
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void MarkAsDirty(std::string tableName);
    void MarkAsClean();
    void updateProjectDir(std::string newProjectDir);

signals:
    void TableDataChanged(std::string tableName);
    void LoadDataTable(std::string MSVPAName, std::string ForecastName,
                       std::string model, std::string table,
                       std::string originSection, int originTab);
    void SelectTab(std::string returnToSection, int returnToTab);
    void MarkForecastAsClean();

public Q_SLOTS:
    void callback_Forecast_Tab2_PrevPB(bool unused);
    void callback_Forecast_Tab2_NextPB(bool unused);
    void callback_TableDataChanged(QModelIndex unused);
    //void callback_TableDataChanged(QModelIndex &unused1, QModelIndex &unused2);
    void callback_Forecast_Tab2_LoadPB(bool unused);
    void callback_Forecast_Tab2_SavePB(bool unused);

};

#endif // NMFFORECASTTAB2_H
