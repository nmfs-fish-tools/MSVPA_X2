#ifndef NMFMSVPATab7_H
#define NMFMSVPATab7_H

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

class nmfMSVPATab7: public QObject
{
    Q_OBJECT

    nmfLogger *logger;

    QTabWidget  *MSVPA_Tabs;
    QWidget     *MSVPA_Tab7_Widget;
    QPushButton *MSVPA_Tab7_PrevPB;
    QPushButton *MSVPA_Tab7_NextPB;
    QPushButton *MSVPA_Tab7_SavePB;
    QPushButton *MSVPA_Tab7_LoadPB;
    QComboBox   *MSVPA_Tab7_SpeciesCMB;
    QTableView  *MSVPA_Tab7_SizePrefsTV;
    QStandardItemModel *smodel;
    std::vector<std::string> Species;
    std::vector<int> Nage;
    std::string MSVPAName;
    std::string ProjectDir;
    nmfDatabase *databasePtr;
    QString MSVPASizePrefCSVFile;
    bool Enabled;

    int NPrey;
    int NSpecies;
    int NOtherPrey;
    int NOtherPred;
    int NPreds;
    int TotalNSpe;
    const int NUM_COLUMNS = 4;

signals:
    void MSVPALoadWidgets(int TabNum);
    void UpdateNavigator(std::string type, int index);
    void TableDataChanged(std::string tableName);
    void MarkMSVPAAsClean();

public:
    nmfMSVPATab7(QTabWidget *tabs,
                 nmfLogger  *logger,
                 std::string &theProjectDir);
    virtual ~nmfMSVPATab7();
    bool loadWidgets(nmfDatabase *theDatabasePtr,
                     std::string theMSVPAName);
    void clearWidgets();
    void makeHeaders(int nRows);
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void MarkAsDirty(std::string tableName);
    void MarkAsClean();
    void updateProjectDir(std::string newProjectDir);

public Q_SLOTS:
    void callback_MSVPA_Tab7_LoadPB(bool unused);
    void callback_MSVPA_Tab7_SavePB(bool unused);
    void callback_MSVPA_Tab7_PrevPB(bool unused);
    void callback_MSVPA_Tab7_NextPB(bool unused);
    void callback_MSVPA_Tab7_ItemChanged(QStandardItem *item);
    void callback_MSVPA_Tab7_SpeciesCMB(int idx);

};

#endif // NMFMSVPATab7_H
