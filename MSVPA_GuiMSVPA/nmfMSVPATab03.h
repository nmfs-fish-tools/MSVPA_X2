#ifndef NMFMSVPATAB3_H
#define NMFMSVPATAB3_H

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QModelIndex>
#include <QObject>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTextStream>
#include <QUiLoader>

#include "nmfDatabase.h"
#include "nmfLogger.h"

class nmfMSVPATab3: public QObject
{
    Q_OBJECT

    nmfLogger   *logger;

    QTabWidget  *MSVPA_Tabs;
    QWidget     *MSVPA_Tab3_Widget;
    QTabWidget  *MSVPA_Tab3_OtherPredTabs;
    QComboBox   *MSVPA_Tab3_OtherPredSpeciesCMB;
    QTableView  *MSVPA_Tab3_BiomassTV;
    QTableView  *MSVPA_Tab3_FeedingDataTV;
    QPushButton *MSVPA_Tab3_PrevPB;
    QPushButton *MSVPA_Tab3_NextPB;
    QPushButton *MSVPA_Tab3_LoadPB;
    QPushButton *MSVPA_Tab3_SavePB;
    QPushButton *MSVPA_Tab3SubTab2_SetNumCatsPB;
    QLabel      *MSVPA_Tab3SubTab1_BiomassLBL;
    QLabel      *MSVPA_Tab3SubTab2_FeedingLBL;
    bool Enabled;

    int NSpecies;
    int numRows;
    int numCols;
    int FirstYear;
    QStandardItemModel *smodelBiomass[100]; // model for each OtherPredSpecies
    QStandardItemModel *smodelFeeding[100];
    std::string MSVPAName;
    std::string ProjectDir;
    nmfDatabase *databasePtr;
    QString MSVPAOtherPredCSVFile;
    std::map<QString,int> OtherPredSpeciesIndexMap;
    void getInitData(QString OtherPredSpecies,
                     std::string &Units, int &NumSizeCats,
                     int &FirstYear,     int &LastYear);
    void updateFeedingTableHeader();
    void currentPredSpecies(QString &OtherPredSpecies,
                            int &OtherPredSpeciesIndex);
    void MarkAsDirty(std::string tableName);
    void MarkAsClean();

signals:
    void MSVPALoadWidgets(int TabNum);
    void UpdateNavigator(std::string type, int index);
    void MSVPATab3(std::string tabAction,
                   int NumPreyBeforeDelete,
                   int PreyToDeleteIndex);
    void TableDataChanged(std::string tableName);
    void MarkMSVPAAsClean();

public:
    nmfMSVPATab3(QTabWidget *tabs,
                 nmfLogger  *logger,
                 std::string &theProjectDir);
    virtual ~nmfMSVPATab3();
    bool loadWidgets(nmfDatabase *databasePtr,
                     std::string MSVPAName);
    void clearWidgets();
    void loadSubTab1(QString dataSource,
                     QString OtherPredSpecies,
                     int OtherPredSpeciesIndex,
                     std::string Units,
                     int NUmYears);
    void loadSubTab2(QString dataSource,
                     QString OtherPredSpecies,
                     int OtherPredSpeciesIndex);
    bool allFieldsAreFilled(QString dataSource);
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void updateProjectDir(std::string newProjectDir);

public Q_SLOTS:
    void callback_MSVPA_Tab3_LoadPB(bool unused);
    void callback_MSVPA_Tab3_SavePB(bool unused);
    void callback_MSVPA_Tab3_PrevPB(bool unused);
    void callback_MSVPA_Tab3_NextPB(bool unused);
    bool saveBiomassData();
    bool saveFeedingData();
    void loadBiomassData();
    bool loadFeedingData();
    void callback_MSVPA_Tab3SubTab1_ItemChanged(QStandardItem *item);
    void callback_MSVPA_Tab3SubTab2_ItemChanged(QStandardItem *item);
    void callback_MSVPA_Tab3SubTab2_SetNumCatsPB(bool unused);
    void callback_MSVPA_Tab3_OtherPredSpeciesCMB(QString species);
    void callback_FeedingDataEnabled(int tab);
};

#endif // NMFMSVPATAB3_H
