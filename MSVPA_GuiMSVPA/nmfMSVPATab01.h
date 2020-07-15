#ifndef NMFMSVPATAB1_H
#define NMFMSVPATAB1_H

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QTableWidget>
#include <QTextStream>
#include <QUiLoader>

#include "nmfDatabase.h"
#include "nmfLogger.h"

class nmfMSVPATab1: public QObject
{
    Q_OBJECT

    nmfLogger   *logger;

    QTabWidget  *MSVPA_Tabs;

    QWidget     *MSVPA_Tab1_Widget;
    QListWidget *MSVPA_Tab1_SelectSpeciesLW;
    QListWidget *MSVPA_Tab1_SelectBMPredLW;
    QListWidget *MSVPA_Tab1_FullSpeciesLW;
    QListWidget *MSVPA_Tab1_PreySpeciesLW;
    QListWidget *MSVPA_Tab1_BMPredLW;
    QPushButton *MSVPA_Tab1_SavePB;
    QPushButton *MSVPA_Tab1_LoadPB;
    QPushButton *MSVPA_Tab1_NextPB;

    nmfDatabase *databasePtr;
    QString MSVPAspeciesCSVFile;
    std::string ProjectDir;
    std::string MSVPAName;
    bool Enabled;

    void MarkAsDirty(std::string tableName);
    void MarkAsClean();

signals:
    void MSVPALoadWidgets(int TabNum);
    void UpdateNavigator(std::string type, int index);
    void TableDataChanged(std::string tableName);
    void MarkMSVPAAsClean();

public:

    nmfMSVPATab1(QTabWidget *tabs,
                 nmfLogger *logger,
                 std::string &theProjectDir);
    virtual ~nmfMSVPATab1();
    bool loadWidgets(nmfDatabase *databasePtr, std::string MSVPAName);
    QList<QString> loadList(QListWidget *listWidget, nmfDatabase* databasePtr, std::string MSVPAName, int Type);
    void clearWidgets();
    void moveItemsFromTo(std::vector<QListWidget *> &listWidgets,
                         QListWidget *ListWidget);
    int getMSVPASpeciesType(QString csvSpeciesName);
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void updateProjectDir(std::string newProjectDir);

public Q_SLOTS:
    void callback_MSVPA_Tab1_NextPagePB(bool unused);
    void callback_MSVPA_Tab1_ResetPB(bool unused);
    void callback_MSVPA_Tab1_SelectFullPB(bool unused);
    void callback_MSVPA_Tab1_DeselectFullPB(bool unused);
    void callback_MSVPA_Tab1_SelectPreyPB(bool unused);
    void callback_MSVPA_Tab1_DeselectPreyPB(bool unused);
    void callback_MSVPA_Tab1_SelectBMPredPB(bool unused);
    void callback_MSVPA_Tab1_DeselectBMPredPB(bool unused);
    void callback_MSVPA_Tab1_LoadPB(bool unused);
    void callback_MSVPA_Tab1_SavePB(bool unused);

};

#endif // NMFMSVPATAB1_H
