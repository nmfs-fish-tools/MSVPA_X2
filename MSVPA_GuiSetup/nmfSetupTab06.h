#ifndef NMFSETUPTAB6_H
#define NMFSETUPTAB6_H

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QMainWindow>
#include <QObject>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QStringList>
#include <QDebug>
#include <QSettings>
#include <QTableWidget>
#include <QMap>
#include <QFile>
#include <QHeaderView>

#include "nmfDatabase.h"
#include "nmfLogger.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm_ext/erase.hpp>


class nmfSetupTab6: public QObject
{
    Q_OBJECT

    QTabWidget   *Setup_Tabs;
    QComboBox    *Setup_Tab6_ModelListCMB;
    QComboBox    *Setup_Tab6_TableListCMB;
    QTextEdit    *SetupOutputTE;
    QLineEdit    *Setup_Tab6_CSVFileLE;
    QTableWidget *Setup_Tab6_TableDescTW;
    QTableWidget *Setup_Tab6_CSVFileTW;
    QPushButton  *Setup_Tab6_RefreshPB;
    QPushButton  *Setup_Tab6_RunValidityCheckPB;
    QPushButton  *Setup_Tab6_RunAllValidityChecksPB;
    QPushButton  *Setup_Tab6_SaveToTablePB;
    QPushButton  *Setup_Tab6_BackPB;
    QPushButton  *Setup_Tab6_TableToCSVPB;

    nmfDatabase  *databasePtr;

    std::string returnToSection;
    int returnToTab;
    std::string returnMSVPAName;
    std::string returnForecastName;

    std::map<std::string,std::vector<std::string> > TableNames[3];
    std::map<std::string,std::string> TableDescriptions;
    QString ProjectDir;
    QString ProjectDatabase;

signals:
    void SelectTab(std::string returnToSection, int returnToTab,
                   std::string MSVPAName, std::string ForecastName);
    void ExportTableToCSVFile(std::string table, std::string csvFile);

public:


    //std::vector<std::vector<std::string> > descData;
    nmfSetupTab6(QTabWidget *tabs,
                 QTextEdit  *theSetupOutputTE);
    virtual ~nmfSetupTab6();
    void clearWidgets();
    void loadWidgets(nmfDatabase *theDatabasePtr);
    void loadDatabaseNames();
    void updateOutputWidget();
    void readSettings();
    std::string moreIntuitiveType(std::string oldType);
    void loadModelTables(int modelType);
    int numColumnsTable(QString table);
    void setReturnData(std::string section, int tab,
                       std::string MSVPAName, std::string ForecastName);

public Q_SLOTS:
    void callback_Setup_Tab6_TableChanged(QString table);
    void callback_Setup_Tab6_Refresh();
    void callback_Setup_Tab6_RunValidityCheck();
    void callback_Setup_Tab6_RunAllValidityChecks();
    void callback_Setup_Tab6_SaveToTable();
    //    void callback_Forecast_Tab1_NextPage();
    void callback_Setup_Tab6_Back();
    void callback_Setup_Tab6_TableToCSVPB();
};

#endif // NMFSETUPTAB6_H
