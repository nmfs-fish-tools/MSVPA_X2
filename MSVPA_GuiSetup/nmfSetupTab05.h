#ifndef NMFSETUPTAB5_H
#define NMFSETUPTAB5_H

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


class nmfSetupTab5: public QObject
{
    Q_OBJECT

    QTabWidget   *Setup_Tabs;
    QComboBox    *Setup_Tab5_ModelListCMB;
    QComboBox    *Setup_Tab5_TableListCMB;
    QTextEdit    *SetupOutputTE;
    QLineEdit    *Setup_Tab5_CSVFileLE;
    QTableWidget *Setup_Tab5_TableDescTW;
    QTableWidget *Setup_Tab5_CSVFileTW;
    QPushButton  *Setup_Tab5_RefreshPB;
    QPushButton  *Setup_Tab5_RunValidityCheckPB;
    QPushButton  *Setup_Tab5_RunAllValidityChecksPB;
    QPushButton  *Setup_Tab5_SaveToTablePB;

    nmfDatabase  *databasePtr;

    std::map<std::string,std::vector<std::string> > TableNames[3];
    std::map<std::string,std::string> TableDescriptions;
    QString ProjectDir;
    QString ProjectDatabase;

public:

    //std::vector<std::vector<std::string> > descData;
    nmfSetupTab5(QTabWidget *tabs,
                 QTextEdit  *theSetupOutputTE);
    virtual ~nmfSetupTab5();
    void clearWidgets();
    void loadWidgets(nmfDatabase *theDatabasePtr);
    void loadDatabaseNames();
    void updateOutputWidget();
    void readSettings();
    std::string moreIntuitiveType(std::string oldType);
    void loadModelTables(int modelType);
    int numColumnsTable(QString table);

public Q_SLOTS:
    void callback_Setup_Tab5_TableChanged(QString table);
    void callback_Setup_Tab5_Refresh();
    void callback_Setup_Tab5_RunValidityCheck();
    void callback_Setup_Tab5_RunAllValidityChecks();
    void callback_Setup_Tab5_SaveToTable();
    //    void callback_Forecast_Tab1_NextPage();

};

#endif // NMFSETUPTAB5_H
