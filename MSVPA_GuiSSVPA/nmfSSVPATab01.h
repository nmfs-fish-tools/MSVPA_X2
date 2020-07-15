#ifndef NMFSSVPATAB1_H
#define NMFSSVPATAB1_H

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QModelIndex>
#include <QObject>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QTableWidget>
#include <QTextStream>
#include <QUiLoader>

#include "nmfDatabase.h"
#include "nmfLogger.h"
#include "nmfConstantsMSVPA.h"

class nmfSSVPATab1: public QObject
{
    Q_OBJECT

    nmfLogger   *logger;

    QTabWidget  *SSVPA_Tabs;
    QWidget     *SSVPA_Tab1_Widget;
    QTableView  *SSVPACatchAtAgeTV;
    QPushButton *SSVPA_Tab1_SaveAllPB;
    QPushButton *SSVPA_Tab1_LoadPB;
    QPushButton *SSVPA_Tab1_LoadAllPB;
    QPushButton *SSVPA_Tab1_NextPB;
    QLabel      *SSVPA_Tab1_LabelLB;
    QStandardItemModel *smodel[nmfConstantsMSVPA::MaxNumSmodels]; // nmfConstants::MaxNumberSpecies];

    int SpeciesIndex;
    std::string ProjectDir;
    std::string Species;
    QString     SpeCatchCSVFile;
    std::vector<QString> AllSpecies;

    nmfDatabase *databasePtr;

public:
    nmfSSVPATab1(QTabWidget *tabs,
                 nmfLogger  *logger,
                 std::string &ProjectDir);
    virtual ~nmfSSVPATab1();
    void clearWidgets();
    bool loadWidgets(nmfDatabase *theDatabasePtr,
                     std::vector<QString> AllSpecies,
                     std::string  SpeciesIndex,
                     std::string  Species,
                     std::string  dataSource,
                     std::string  fileName);
    void makeHeaders(int FirstYear,
                     int LastYear,
                     int MinCatchAge,
                     int NumAges);
    bool loadSpecies(int SpeciesIndex);
    bool loadAllSpeciesFromTableOrFile(nmfDatabase *theDatabasePtr,
                                       int currSpeciesIndex,
                                       std::string currSpeciesName,
                                       std::vector<QString> AllSpecies,
                                       std::string FromTableOrFile);
    void loadSpeciesFromFile(QTextStream &inStream,
                             QString Species);
    void restoreCSVFromDatabase();
    void updateMainLabel(std::string Species);
    void MarkAsDirty(std::string tableName);
    void MarkAsClean();
    void updateProjectDir(std::string newProjectDir);

signals:
    void InitializePage(std::string type, int index, bool withCallback);
    void ResetSpeciesList();
    void ReselectSpecies(std::string currentSpecies, bool withCallback);
    void TableDataChanged(std::string tableName);
    void MarkSSVPAAsClean();


public Q_SLOTS:
    void callback_LoadPB(bool toggle);
    void callback_LoadAllPB(bool toggle);
    void callback_SaveAllPB(bool quiet);
    void callback_NextPB(bool unused);
    void callback_ItemChanged(QStandardItem *item);
    void callback_TabChanged(int newTab);

};

#endif // NMFSSVPATAB1_H
