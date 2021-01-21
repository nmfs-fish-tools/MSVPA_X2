#ifndef NMFMSVPATAB2_H
#define NMFMSVPATAB2_H

#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
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
#include <QTableWidget>
#include <QTextStream>
#include <QUiLoader>

#include "nmfDatabase.h"
#include "nmfLogger.h"

class nmfMSVPATab2: public QObject
{
    Q_OBJECT

    const int Season_Column = 0;
    const int Length_Column = 1;

    nmfLogger   *logger;

    QTabWidget  *MSVPA_Tabs;
    QWidget     *MSVPA_Tab2_Widget;
    QPushButton *MSVPA_Tab2_PrevPB;
    QPushButton *MSVPA_Tab2_NextPB;
    QPushButton *MSVPA_Tab2_LoadPB;
    QPushButton *MSVPA_Tab2_SavePB;
    QGroupBox   *MSVPA_Tab2_YearsGB;
    QCheckBox   *MSVPA_Tab2_AnnTempVariCB;
    QCheckBox   *MSVPA_Tab2_SeasonalOverlapCB;
    QCheckBox   *MSVPA_Tab2_PredatorGrowthCB;
    QLabel      *MSVPA_Tab2_EarliestYearLB;
    QLabel      *MSVPA_Tab2_LatestYearLB;
    QLabel      *MSVPA_Tab2_SeasonalDataLBL;
    QLineEdit   *MSVPA_Tab2_FirstYearLE;
    QLineEdit   *MSVPA_Tab2_LastYearLE;
    QLineEdit   *MSVPA_Tab2_NumberSeasonsLE;
    QTableView  *MSVPA_Tab2_SeasonalDataTV;
    QPushButton *MSVPA_Tab2_SetSeasonalLengthPB;
    QPushButton *MSVPA_Tab2_SetSeasonalTempPB;
    std::vector<std::string> SeasonLength; // std::string cause may need to put blanks in table
    std::vector<std::string> SeasonTemps;  // std::string cause may need to put blanks in table
    int NSeasons;
    QStandardItemModel *smodelSeasonLength;
    QStandardItemModel *smodelSeasonTemps;
    std::string MSVPAName;
    std::string ProjectDir;
    nmfDatabase *databasePtr;
    int FirstYear;
    int LastYear;
    QString MSVPAlistCSVFile;
    QString MSVPASeasInfoCSVFile;
    bool Enabled;

    void MarkAsDirty(std::string tableName);
    void MarkAsClean();

signals:
    void MSVPALoadWidgets(int TabNum);
    void UpdateNavigator(std::string type, int index);
    void TableDataChanged(std::string tableName);
    void MarkMSVPAAsClean();

public:

    nmfMSVPATab2(QTabWidget *tabs,
                 nmfLogger *logger,
                 std::string &ProjectDir);
    virtual ~nmfMSVPATab2();
    bool loadWidgets(nmfDatabase *databasePtr, std::string MSVPAName);
    void clearWidgets();
    bool resaveMSVPAlistFields();
    std::vector<int> resaveMSVPASeasInfo(int NSeasons);
    bool checkNumTotalDays();
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void updateProjectDir(std::string newProjectDir);

public Q_SLOTS:
    void callback_MSVPA_Tab2_PrevPB(bool unused);
    void callback_MSVPA_Tab2_NextPB(bool unused);
    void callback_MSVPA_Tab2_LoadPB(bool unused);
    void callback_MSVPA_Tab2_SavePB(bool unused);
    void callback_MSVPA_Tab2_PredatorGrowth(int state);
    void callback_MSVPA_Tab2_SetSeasonalLengthPB(bool unused);
    void callback_MSVPA_Tab2_SetSeasonalTempPB(bool unused);
    void callback_MSVPA_Tab2_ItemChanged_SeasonLength(QStandardItem *item);
    void callback_MSVPA_Tab2_ItemChanged_SeasonTemps(QStandardItem *item);
    void callback_MSVPA_Tab2_AnnTempVariCB(int state);
    void callback_MSVPA_Tab2_SeasonalOverlapCB(int state);
    void callback_StripFirstYearLE();
    void callback_StripLastYearLE();
    void callback_StripNumberSeasonsLE();
};

#endif // NMFMSVPATAB2_H
