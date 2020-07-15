#ifndef NMFMSVPATab6_H
#define NMFMSVPATab6_H


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

class nmfMSVPATab6: public QObject
{
    Q_OBJECT

    nmfLogger *logger;

    QTabWidget  *MSVPA_Tabs;
    QWidget     *MSVPA_Tab6_Widget;
    QPushButton *MSVPA_Tab6_PrevPB;
    QPushButton *MSVPA_Tab6_NextPB;
    QPushButton *MSVPA_Tab6_SavePB;
    QPushButton *MSVPA_Tab6_LoadPB;
    QPushButton *MSVPA_Tab6_ClearPB;
    QComboBox   *MSVPA_Tab6_SpeciesCMB;
    QComboBox   *MSVPA_Tab6_SeasonCMB;
    QTableView  *MSVPA_Tab6_SpOverlapTV;
    QStandardItemModel *smodel;
    std::vector<int> Nage;
    std::vector<int> PredType;
    std::vector<std::string> AllSpecies;
    std::vector<std::string> Species;
    std::vector<std::string> OthPred;
    std::vector<std::string> Prey;
    std::vector<std::string> OthPrey;
    int NPrey;
    int NSpecies;
    int NOtherPrey;
    int NOtherPred;
    int NPreds;
    int TotalNSpe;
    int RealNSeasons;
    int NSeasons;
    nmfDatabase *databasePtr;
    std::string MSVPAName;
    std::string ProjectDir;
    QString MSVPASpaceOCSVFile;
    bool Enabled;

public:
    nmfMSVPATab6(QTabWidget *tabs,
                 nmfLogger  *logger,
                 std::string &theProjectDir);
    virtual ~nmfMSVPATab6();
    bool loadWidgets(nmfDatabase *databasePtr,
                     std::string MSVPAName);
    void clearWidgets();
    void makeHeaders(int nCols, std::vector<std::string> AllSpecies);
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void MarkAsDirty(std::string tableName);
    void MarkAsClean();
    void updateProjectDir(std::string newProjectDir);

signals:
    void MSVPALoadWidgets(int TabNum);
    void UpdateNavigator(std::string type, int index);
    void TableDataChanged(std::string tableName);
    void MarkMSVPAAsClean();

public Q_SLOTS:
    void callback_MSVPA_Tab6_PrevPB(bool unused);
    void callback_MSVPA_Tab6_NextPB(bool unused);
    void callback_MSVPA_Tab6_SavePB(bool unused);
    void callback_MSVPA_Tab6_LoadPB(bool unused);
    void callback_MSVPA_Tab6_ClearPB(bool unused);
    void callback_MSVPA_Tab6_SpeciesCMB(int index);
    void callback_MSVPA_Tab6_SeasonCMB(int index);
    void callback_MSVPA_Tab6_ItemChanged(QStandardItem *item);

};

#endif // NMFMSVPATab6_H
