#ifndef NMFMSVPATab5_H
#define NMFMSVPATab5_H

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


class nmfMSVPATab5: public QObject
{
    Q_OBJECT

    nmfLogger *logger;

    QTabWidget         *MSVPA_Tabs;
    QWidget            *MSVPA_Tab5_Widget;
    QPushButton        *MSVPA_Tab5_PrevPB;
    QPushButton        *MSVPA_Tab5_NextPB;
    QPushButton        *MSVPA_Tab5_SavePB;
    QPushButton        *MSVPA_Tab5_LoadPB;
    QPushButton        *MSVPA_Tab5_ClearPB;
    QComboBox          *MSVPA_Tab5_SpeciesCMB;
    QTableView         *MSVPA_Tab5_PreyPrefsTV;
    std::vector<std::string> AllSpecies;
    std::vector<int> Nage;
    std::vector<int> PredType;
    QStandardItemModel* smodel;
    nmfDatabase *databasePtr;
    std::string MSVPAName;
    std::string ProjectDir;
    QString MSVPAPrefsCSVFile;
    bool Enabled;

signals:
    void MSVPALoadWidgets(int TabNum);
    void UpdateNavigator(std::string type, int index);
    void TableDataChanged(std::string tableName);
    void MarkMSVPAAsClean();

public:
    nmfMSVPATab5(QTabWidget *tabs,
                 nmfLogger *logger,
                 std::string &theProjectDir);
    virtual ~nmfMSVPATab5();
    bool loadWidgets(nmfDatabase *databasePtr,
                     std::string MSVPAName);
    void clearWidgets();
    void makeHeaders(std::vector<std::string> AllSpecies,
                     int nCols,
                     int PredType);
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void MarkAsDirty(std::string tableName);
    void MarkAsClean();
    void updateProjectDir(std::string newProjectDir);

public Q_SLOTS:
    void callback_MSVPA_Tab5_PrevPB(bool);
    void callback_MSVPA_Tab5_NextPB(bool);
    void callback_MSVPA_Tab5_SavePB(bool);
    void callback_MSVPA_Tab5_LoadPB(bool);
    void callback_MSVPA_Tab5_SpeciesCMB(int index);
    void callback_MSVPA_Tab5_ItemChanged(QStandardItem *item);
    void callback_RemoveAPreySpecies(std::string,
                                     int numPreyBeforeDelete,
                                     int preyToDeleteIndex);
    void callback_MSVPA_Tab5_ClearPB(bool);
};

#endif // NMFMSVPATab5_H
