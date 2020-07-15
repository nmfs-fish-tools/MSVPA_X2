#ifndef NMFMSVPATab8_H
#define NMFMSVPATab8_H

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

class nmfMSVPATab8: public QObject
{
    Q_OBJECT

    nmfLogger   *logger;

    QTabWidget  *MSVPA_Tabs;
    QWidget     *MSVPA_Tab8_Widget;
    QPushButton *MSVPA_Tab8_PrevPB;
    QPushButton *MSVPA_Tab8_NextPB;
    QPushButton *MSVPA_Tab8_SavePB;
    QPushButton *MSVPA_Tab8_LoadPB;
    QComboBox   *MSVPA_Tab8_SpeciesCMB;
    QTableView  *MSVPA_Tab8_PreyPrefsTV;
    QStandardItemModel *smodel;
    std::vector<int> Nage;
    std::vector<int> SpeType;
    std::vector<int> SpeIndex;
    std::vector<std::string> AllSpecies;
    std::vector<std::string> Species;
    std::vector<std::string> OthPred;
    std::string MSVPAName;
    std::string ProjectDir;
    nmfDatabase *databasePtr;
    int NSpecies;
    int NSeasons;
    int NOtherPred;
    int NPreds;
    QString MSVPAStomContCSVFile;
    bool Enabled;

signals:
    void MSVPALoadWidgets(int TabNum);
    void UpdateNavigator(std::string type, int index);
    void TableDataChanged(std::string tableName);
    void MarkMSVPAAsClean();

public:
    nmfMSVPATab8(QTabWidget *tabs,
                 nmfLogger *logger,
                 std::string &theProjectDir);
    virtual ~nmfMSVPATab8();
    bool loadWidgets(nmfDatabase *theDatabasePtr,
                     std::string theMSVPAName);
    void clearWidgets();
    void makeHeaders(int nRows, int nCols);
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void MarkAsDirty(std::string tableName);
    void MarkAsClean();
    void updateProjectDir(std::string newProjectDir);

public Q_SLOTS:
    void callback_MSVPA_Tab8_PrevPB(bool unused);
    void callback_MSVPA_Tab8_NextPB(bool unused);
    void callback_MSVPA_Tab8_LoadPB(bool unused);
    void callback_MSVPA_Tab8_SavePB(bool unused);
    void callback_MSVPA_Tab8_ItemChanged(QStandardItem *item);
    void callback_MSVPA_Tab8_SpeciesCMB(int index);

};

#endif // NMFMSVPATab8_H
