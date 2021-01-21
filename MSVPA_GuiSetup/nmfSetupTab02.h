#ifndef NMFSETUPTAB2_H
#define NMFSETUPTAB2_H

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <set>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QMap>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSettings>
#include <QTextEdit>
#include <QTextStream>
#include <QUiLoader>
#include <QVariant>
#include <QVariantList>

#include "nmfDatabase.h"
#include "nmfLogger.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

//Q_DECLARE_METATYPE(QList<std::string>)

class nmfSetup_Tab2: public QObject
{
    Q_OBJECT

    nmfLogger   *logger;

    QTabWidget  *Setup_Tabs;
    QWidget     *Setup_Tab2_Widget;
    QGroupBox   *Setup_Tab2_ProjectDataGB;
    QGroupBox   *Setup_Tab2_TimeDataGB;
    QComboBox   *Setup_Tab2_ProjectDatabaseCMB;
    QComboBox   *Setup_Tab2_SpeciesCMB;
    QComboBox   *Setup_Tab2_OtherPredatorCMB;
    QComboBox   *Setup_Tab2_PreyCMB;
    QPushButton *Setup_Tab2_ProjectDirBrowsePB;
    QPushButton *Setup_Tab2_SpeciesDelPB;
    QPushButton *Setup_Tab2_OtherPredatorDelPB;
    QPushButton *Setup_Tab2_PreyDelPB;
    QPushButton *Setup_Tab2_DelDatabasePB;
    QPushButton *Setup_Tab2_AddDatabasePB;
    QPushButton* Setup_Tab2_RefreshDatabasePB;
    QPushButton *Setup_Tab2_SaveProjectPB;
    QPushButton *Setup_Tab2_NewProjectPB;
    QPushButton *Setup_Tab2_BrowseProjectPB;
    QPushButton *Setup_Tab2_SetProjectPB;
    QLineEdit   *Setup_Tab2_ProjectNameLE;
    QLineEdit   *Setup_Tab2_ProjectAuthorLE;
    QLineEdit   *Setup_Tab2_ProjectDescLE;
    QLineEdit   *Setup_Tab2_ProjectDirLE;
    QLineEdit   *Setup_Tab2_MinCatchAgeLE;
    QLineEdit   *Setup_Tab2_MaxCatchAgeLE;
    QLineEdit   *Setup_Tab2_FirstYearLE;
    QLineEdit   *Setup_Tab2_LastYearLE;
    QLineEdit   *Setup_Tab2_NumSeasonsLE;
    QCheckBox   *Setup_Tab3_SpeciesCB;
    QCheckBox   *Setup_Tab3_OtherPredatorsCB;

    nmfDatabase *databasePtr;
    QTextEdit   *SetupOutputTE;

    int FirstYear;
    int LastYear;
    int NumSeasons;
    bool NewProject;

    QMap<std::string,int> MinCatchAge;
    QMap<std::string,int> MaxCatchAge;
    QString ProjectDir;
    QString LastProjectDatabase;
    QString ProjectDatabase;
    QString ProjectName;
    QString ProjectAuthor;
    QString ProjectDescription;
    std::set<std::string> ValidDatabases;
    QList<std::string> Species;
    QList<std::string> OtherPredators;
    QList<std::string> Prey;
    std::map<std::string,std::vector<std::string> > TableNames[3];  // RSK continue here.....

signals:
    void LoadDatabase(QString database);
    void WaitCursor();
    void CreateTables();
    void ProjectSet();

public:

    nmfSetup_Tab2(QTabWidget *tabs,
                 nmfLogger  *logger,
                 QTextEdit  *theSetupOutputTE);
    virtual ~nmfSetup_Tab2();
    void clearProjectData();
    void clearWidgets();
    void enableProjectData();
    void enableSetupTabs(bool enable);
    bool isProjectDataValid();
    void loadDatabaseNames(QString NameToSelect);
    void loadProject(QString fileName);
    void loadWidgets(nmfDatabase *theDatabasePtr);
    void readSettings();
    void saveSettings();
    /**
     * @brief Writes the author of the project into the appropriate widget
     * @param author : the author's name
     */
    void setProjectAuthor(QString author);
    /**
     * @brief Sets the current item in the pulldown to the passed database name
     * @param dbName : name of database to set the pulldown to
     */
    void setProjectDatabase(QString dbName);
    /**
     * @brief Writes the description of the project into the appropriate widget
     * @param desc : the description of the project
     */
    void setProjectDescription(QString desc);
    /**
     * @brief Writes the directory for the project into the appropriate widget
     * @param dir : the directory of the project
     */
    void setProjectDirectory(QString dir);
    /**
     * @brief Writes the name of the current project into the appropriate widget
     * @param name : the name of the project
     */
    void setProjectName(QString name);
    int validateFirstYear(int newFirstYear);
    int validateLastYear(int newFirstYear);
    bool validateYearSeasonFields(int newFirstYear, int newLastYear, int numSeasons);

public Q_SLOTS:
    void callback_ProjectDataGB(bool toggled);
    void callback_Setup_Tab2_AddDatabase();
    void callback_Setup_Tab2_BrowseProject();
    void callback_Setup_Tab2_DatabaseChanged(QString dbName);
    void callback_Setup_Tab2_DelDatabase();
    void callback_Setup_Tab2_RefreshDatabase();
    void callback_Setup_Tab2_FirstYearAdd();
    void callback_Setup_Tab2_LastYearAdd();
    void callback_Setup_Tab2_NewProject();
    void callback_Setup_Tab2_NumSeasonsAdd();
    void callback_Setup_Tab2_ProjectAuthorAdd();
    void callback_Setup_Tab2_ProjectDirAdd();
    void callback_Setup_Tab2_ProjectDirBrowsePB();
    void callback_Setup_Tab2_ProjectDescAdd();
    void callback_Setup_Tab2_ProjectNameAdd();
    void callback_Setup_Tab2_SaveProject();
    void callback_Setup_Tab2_SetProject();
};

#endif // NMFSETUPTAB2_H
