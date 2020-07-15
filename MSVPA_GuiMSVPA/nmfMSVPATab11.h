#ifndef NMFMSVPATab11_H
#define NMFMSVPATab11_H


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

class nmfMSVPATab11: public QObject
{
    Q_OBJECT

    nmfLogger    *logger;

    QTabWidget   *MSVPA_Tabs;
    QWidget      *MSVPA_Tab11_Widget;
    QPushButton  *MSVPA_Tab11_PrevPB;
    QPushButton  *MSVPA_Tab11_NextPB;
    QPushButton  *MSVPA_Tab11_LoadPB;
    QPushButton  *MSVPA_Tab11_SavePB;
    QTableWidget *MSVPA_Tab11_SSVPADataTW;
    nmfDatabase  *databasePtr;
    std::string   MSVPAName;
    std::string   ProjectDir;
    QString       MSVPAspeciesCSVFile;
    bool Enabled;

    int NSpecies;
    std::vector<int> SpeIndex;
    std::vector<std::string> SpeName;

signals:
    void MSVPALoadWidgets(int TabNum);
    void UpdateNavigator(std::string type, int index);
    void CheckAndEnableMSVPAExecutePage();
    void TableDataChanged(std::string tableName);
    void MarkMSVPAAsClean();

public:

    nmfMSVPATab11(QTabWidget *tabs,
                  nmfLogger  *logger,
                  std::string &theProjectDir);
    virtual ~nmfMSVPATab11();
    bool loadWidgets(nmfDatabase *databasePtr,
                     std::string  MSVPAName);
    void clearWidgets();
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void enableNextButton(bool state);
    void MarkAsDirty(std::string tableName);
    void MarkAsClean();
    void updateProjectDir(std::string newProjectDir);

public Q_SLOTS:
    void callback_MSVPA_Tab11_PrevPB(bool unused);
    void callback_MSVPA_Tab11_NextPB(bool unused);
    void callback_MSVPA_Tab11_LoadPB(bool unused);
    void callback_MSVPA_Tab11_SavePB(bool unused);

};

#endif // NMFMSVPATab11_H
