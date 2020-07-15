#ifndef NMFMSVPATAB4_H
#define NMFMSVPATAB4_H

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

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/multi_array.hpp>

typedef boost::multi_array<double, 3> Boost3DArrayDouble;
typedef boost::multi_array<double, 4> Boost4DArrayDouble;
typedef Boost3DArrayDouble::index index3D;
typedef Boost4DArrayDouble::index index4D;

class nmfMSVPATab4: public QObject
{
    Q_OBJECT

    nmfLogger *logger;

    QTabWidget  *MSVPA_Tabs;
    QWidget     *MSVPA_Tab4_Widget;
    QComboBox   *MSVPA_Tab4_PreySpeciesCMB;
    QLineEdit   *MSVPA_Tab4_MinLenLE;
    QLineEdit   *MSVPA_Tab4_MaxLenLE;
    QLineEdit   *MSVPA_Tab4_AlphaLE;
    QLineEdit   *MSVPA_Tab4_BetaLE;
    QTableView  *MSVPA_Tab4_BiomassTV;
    QPushButton *MSVPA_Tab4_PrevPB;
    QPushButton *MSVPA_Tab4_NextPB;
    QPushButton *MSVPA_Tab4_LoadPB;
    QPushButton *MSVPA_Tab4_SavePB;
    QPushButton *MSVPA_Tab4_AddPreyPB;
    QPushButton *MSVPA_Tab4_DelPreyPB;

    int NSpecies;
    int numRows;
    int numCols;
    QStandardItemModel* smodel;
    std::string MSVPAName;
    std::string ProjectDir;
    nmfDatabase *databasePtr;
    QString MSVPAOthPreyCSVFile;
    QString MSVPAOthPreyAnnCSVFile;
    QString MSVPAlistCSVFile;
    bool Enabled;

signals:
    void MSVPALoadWidgets(int TabNum);
    void UpdateNavigator(std::string type, int index);
    void MSVPATab4(std::string tabAction,
                   int NumPreyBeforeDelete,
                   int PreyToDeleteIndex);
    void TableDataChanged(std::string tableName);
    void MarkMSVPAAsClean();

public:

    nmfMSVPATab4(QTabWidget *tabs,
                 nmfLogger  *logger,
                 std::string &theProjectDir);
    virtual ~nmfMSVPATab4();
    bool loadWidgets(nmfDatabase *databasePtr,
                     std::string MSVPAName);
    void clearWidgets();
    bool loadMSVPAOthPrey(QString Prey);
    void loadMSVPAOthPreyAnn(QString Prey);
    void makeHeaders(int FirstYear, int NYears, int NSeasons);
    void getPreyMinMaxInfo(QString Prey,
                           QString &MinStr,
                           QString &MaxStr,
                           QString &AlphaStr,
                           QString &BetaStr);
    void getYearSeasonData(int &FirstYear,
                           int &LastYear,
                           int &NYears,
                           int &NSeasons);
    bool updatePreyDatabaseTables(QString fileNameWithPathOthPrey,
                                  QString tmpFileNameWithPathOthPrey,
                                  QString fileNameWithPathOthPreyAnn,
                                  QString tmpFileNameWithPathOthPreyAnn);
    void updatePreyDatabaseTablesForOtherTabs(QString tableName,
                                              QString preyName);
    void UpdatePreyDatabaseTable(QString tableName, QString fileNameWithPathOthPrey);
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void MarkAsDirty(std::string tableName);
    void MarkAsClean();
    void updateProjectDir(std::string newProjectDir);

public Q_SLOTS:
    void callback_MSVPA_Tab4_PrevPB(bool unused);
    void callback_MSVPA_Tab4_NextPB(bool unused);
    void callback_MSVPA_Tab4_SavePB(bool unused);
    void callback_MSVPA_Tab4_LoadPB(bool unused);
    void callback_MSVPA_Tab4_AddPreyPB(bool unused);
    void callback_MSVPA_Tab4_DelPreyPB(bool unused);
    void callback_MSVPA_Tab4_PreySpeciesCMB(int index);
    void callback_MSVPA_Tab4_ItemChanged(QStandardItem *item);
    void callback_theMaxSettingWasChanged(QString value);
    void callback_theMinSettingWasChanged(QString value);
    void callback_theAlphaSettingWasChanged(QString value);
    void callback_theBetaSettingWasChanged(QString value);
    void callback_StripMinLenLE();
    void callback_StripMaxLenLE();
    void callback_StripAlphaLE();
    void callback_StripBetaLE();

};

#endif // NMFMSVPATAB4_H
