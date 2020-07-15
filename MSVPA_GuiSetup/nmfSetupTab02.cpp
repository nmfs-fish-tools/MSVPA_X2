
#include "nmfSetupTab02.h"
#include "nmfConstants.h"
#include "nmfUtilsQt.h"
#include "nmfUtils.h"


nmfSetup_Tab2::nmfSetup_Tab2(QTabWidget *tabs,
                           nmfLogger  *theLogger,
                           QTextEdit  *theSetupOutputTE)
{
    QUiLoader loader;

    Setup_Tabs = tabs;
    logger = theLogger;

    // Load ui as a widget from disk
    QFile file(":/forms/Setup/Setup_Tab02.ui");
    file.open(QFile::ReadOnly);
    Setup_Tab2_Widget = loader.load(&file,Setup_Tabs);
    Setup_Tab2_Widget->setObjectName("Setup_Tab2_Widget");
    file.close();

    // Add the loaded widget as the new tabbed page
    Setup_Tabs->addTab(Setup_Tab2_Widget, tr("2. Project Setup"));

    databasePtr = NULL;
    SetupOutputTE = theSetupOutputTE;

//    FirstYear       = 0;
//    LastYear        = 0;
//    NumSeasons      = 0;
    ProjectDir.clear();
    LastProjectDatabase.clear();
    ProjectDatabase.clear();
    ProjectName.clear();
    ProjectAuthor.clear();
    ProjectDescription.clear();
    ValidDatabases  = {};
    NewProject = false;
//    Species         = {};
//    OtherPredators  = {};
//    Prey            = {};
//    MinCatchAge.clear();
//    MaxCatchAge.clear();

    Setup_Tab2_ProjectNameLE      = Setup_Tabs->findChild<QLineEdit   *>("Setup_Tab2_ProjectNameLE");
    Setup_Tab2_ProjectDirBrowsePB = Setup_Tabs->findChild<QPushButton *>("Setup_Tab2_BrowseProjectDirPB");
    Setup_Tab2_ProjectDirLE       = Setup_Tabs->findChild<QLineEdit   *>("Setup_Tab2_ProjectDirLE");
    Setup_Tab2_ProjectDatabaseCMB = Setup_Tabs->findChild<QComboBox   *>("Setup_Tab2_ProjectDatabaseCMB");
    Setup_Tab2_ProjectAuthorLE    = Setup_Tabs->findChild<QLineEdit   *>("Setup_Tab2_ProjectAuthorLE");
    Setup_Tab2_ProjectDescLE      = Setup_Tabs->findChild<QLineEdit   *>("Setup_Tab2_ProjectDescriptionLE");
    Setup_Tab2_AddDatabasePB      = Setup_Tabs->findChild<QPushButton *>("Setup_Tab2_AddDatabasePB");
    Setup_Tab2_DelDatabasePB      = Setup_Tabs->findChild<QPushButton *>("Setup_Tab2_DelDatabasePB");
    Setup_Tab2_RefreshDatabasePB  = Setup_Tabs->findChild<QPushButton *>("Setup_Tab2_RefreshDatabasePB");
    Setup_Tab2_FirstYearLE        = Setup_Tabs->findChild<QLineEdit   *>("Setup_Tab2_FirstYearLE");
    Setup_Tab2_LastYearLE         = Setup_Tabs->findChild<QLineEdit   *>("Setup_Tab2_LastYearLE");
    Setup_Tab2_NumSeasonsLE       = Setup_Tabs->findChild<QLineEdit   *>("Setup_Tab2_NumSeasonsLE");
    Setup_Tab2_SaveProjectPB      = Setup_Tabs->findChild<QPushButton *>("Setup_Tab2_SaveProjectPB");
    Setup_Tab2_NewProjectPB       = Setup_Tabs->findChild<QPushButton *>("Setup_Tab2_NewProjectPB");
    Setup_Tab2_BrowseProjectPB    = Setup_Tabs->findChild<QPushButton *>("Setup_Tab2_BrowseProjectPB");
    Setup_Tab2_SetProjectPB       = Setup_Tabs->findChild<QPushButton *>("Setup_Tab2_SetProjectPB");
    Setup_Tab2_TimeDataGB         = Setup_Tabs->findChild<QGroupBox   *>("Setup_Tab2_TimeDataGB");
    Setup_Tab2_ProjectDataGB      = Setup_Tabs->findChild<QGroupBox   *>("Setup_Tab2_ProjectDataGB");
//    Setup_Tab3_SpeciesCB          = Setup_Tabs->findChild<QCheckBox   *>("Setup_Tab3_SpeciesCB");
//    Setup_Tab3_OtherPredatorsCB   = Setup_Tabs->findChild<QCheckBox   *>("Setup_Tab3_OtherPredatorsCB");

    /*
    Setup_Tab2_SpeciesDelPB       = Setup_Tabs->findChild<QPushButton *>("Setup_Tab2_SpeciesDelPB");
    Setup_Tab2_OtherPredatorDelPB = Setup_Tabs->findChild<QPushButton *>("Setup_Tab2_OtherPredatorDelPB");
    Setup_Tab2_PreyDelPB          = Setup_Tabs->findChild<QPushButton *>("Setup_Tab2_PreyDelPB");

    Setup_Tab2_MinCatchAgeLE      = Setup_Tabs->findChild<QLineEdit *>("Setup_Tab2_MinCatchAgeLE");
    Setup_Tab2_MaxCatchAgeLE      = Setup_Tabs->findChild<QLineEdit *>("Setup_Tab2_MaxCatchAgeLE");
    Setup_Tab2_SpeciesCMB         = Setup_Tabs->findChild<QComboBox *>("Setup_Tab2_SpeciesCMB");
    Setup_Tab2_OtherPredatorCMB   = Setup_Tabs->findChild<QComboBox *>("Setup_Tab2_OtherPredatorCMB");
    Setup_Tab2_PreyCMB            = Setup_Tabs->findChild<QComboBox *>("Setup_Tab2_PreyCMB");


    // Setup validator for catch age lineedits
    QRegExp rxMin("[0-9][0-9]*");
    Setup_Tab2_MinCatchAgeLE->setValidator(new QRegExpValidator(rxMin, Setup_Tab2_MinCatchAgeLE));
    QRegExp rxMax("[1-9][0-9]*");
    Setup_Tab2_MaxCatchAgeLE->setValidator(new QRegExpValidator(rxMax, Setup_Tab2_MaxCatchAgeLE));

    // Setup validator for first year, last year, and num seasons
    QRegExp rxFirstYear("[1-9][0-9]{3}");
    Setup_Tab2_FirstYearLE->setValidator(new QRegExpValidator(rxFirstYear, Setup_Tab2_FirstYearLE));
    QRegExp rxLastYear("[1-9][0-9]{3}");
    Setup_Tab2_LastYearLE->setValidator(new QRegExpValidator(rxLastYear, Setup_Tab2_LastYearLE));
    QRegExp rxNumSeasons("[1-6]");
    Setup_Tab2_NumSeasonsLE->setValidator(new QRegExpValidator(rxNumSeasons, Setup_Tab2_NumSeasonsLE));
*/
/*
    Forecast_Tab1_NextPB->setText("--\u25B7");
*/

    connect(Setup_Tab2_ProjectDataGB,      SIGNAL(toggled(bool)),
            this,                          SLOT(callback_ProjectDataGB(bool)));

    connect(Setup_Tab2_ProjectDirBrowsePB, SIGNAL(clicked()),
            this,                          SLOT(callback_Setup_Tab2_ProjectDirBrowsePB()));
    connect(Setup_Tab2_DelDatabasePB,      SIGNAL(clicked()),
            this,                          SLOT(callback_Setup_Tab2_DelDatabase()));
    connect(Setup_Tab2_SaveProjectPB,      SIGNAL(clicked()),
            this,                          SLOT(callback_Setup_Tab2_SaveProject()));
    connect(Setup_Tab2_NewProjectPB,       SIGNAL(clicked()),
            this,                          SLOT(callback_Setup_Tab2_NewProject()));
    connect(Setup_Tab2_BrowseProjectPB,    SIGNAL(clicked()),
            this,                          SLOT(callback_Setup_Tab2_BrowseProject()));

    connect(Setup_Tab2_SetProjectPB,       SIGNAL(clicked()),
            this,                          SLOT(callback_Setup_Tab2_SetProject()));

    connect(Setup_Tab2_ProjectNameLE,      SIGNAL(editingFinished()),
            this,                          SLOT(callback_Setup_Tab2_ProjectNameAdd()));
    connect(Setup_Tab2_ProjectAuthorLE,    SIGNAL(editingFinished()),
            this,                          SLOT(callback_Setup_Tab2_ProjectAuthorAdd()));
    connect(Setup_Tab2_ProjectDescLE,      SIGNAL(editingFinished()),
            this,                          SLOT(callback_Setup_Tab2_ProjectDescAdd()));
    connect(Setup_Tab2_FirstYearLE,        SIGNAL(editingFinished()),
            this,                          SLOT(callback_Setup_Tab2_FirstYearAdd()));
    connect(Setup_Tab2_LastYearLE,         SIGNAL(editingFinished()),
            this,                          SLOT(callback_Setup_Tab2_LastYearAdd()));
    connect(Setup_Tab2_NumSeasonsLE,       SIGNAL(editingFinished()),
            this,                          SLOT(callback_Setup_Tab2_NumSeasonsAdd()));
    connect(Setup_Tab2_ProjectDirLE,       SIGNAL(returnPressed()),
            this,                          SLOT(callback_Setup_Tab2_ProjectDirAdd()));
    connect(Setup_Tab2_AddDatabasePB,      SIGNAL(clicked()),
            this,                          SLOT(callback_Setup_Tab2_AddDatabase()));
    connect(Setup_Tab2_RefreshDatabasePB,  SIGNAL(clicked()),
            this,                          SLOT(callback_Setup_Tab2_RefreshDatabase()));
    connect(Setup_Tab2_ProjectDatabaseCMB, SIGNAL(currentTextChanged(QString)),
            this,                          SLOT(callback_Setup_Tab2_DatabaseChanged(QString)));
/*
    connect(Setup_Tab2_MinCatchAgeLE,      SIGNAL(editingFinished()),
            this,                          SLOT(callback_Setup_Tab2_MinCatchAgeAdd()));
    connect(Setup_Tab2_MaxCatchAgeLE,      SIGNAL(editingFinished()),
            this,                          SLOT(callback_Setup_Tab2_MaxCatchAgeAdd()));
    connect(Setup_Tab2_SpeciesCMB->lineEdit(),       SIGNAL(returnPressed()),
            this,                                    SLOT(callback_Setup_Tab2_SpeciesAdd()));
    connect(Setup_Tab2_SpeciesCMB,                   SIGNAL(currentTextChanged(QString)),
            this,                                    SLOT(callback_Setup_Tab2_CurrentSpeciesChanged(QString)));
    connect(Setup_Tab2_OtherPredatorCMB->lineEdit(), SIGNAL(returnPressed()),
            this,                                    SLOT(callback_Setup_Tab2_OtherPredatorAdd()));
    connect(Setup_Tab2_PreyCMB->lineEdit(),          SIGNAL(returnPressed()),
            this,                                    SLOT(callback_Setup_Tab2_PreyAdd()));
    connect(Setup_Tab2_SpeciesDelPB,       SIGNAL(pressed()),
            this,                          SLOT(callback_Setup_Tab2_SpeciesDel()));
    connect(Setup_Tab2_OtherPredatorDelPB, SIGNAL(pressed()),
            this,                          SLOT(callback_Setup_Tab2_OtherPredatorDel()));
    connect(Setup_Tab2_PreyDelPB,          SIGNAL(pressed()),
            this,                          SLOT(callback_Setup_Tab2_PreyDel()));

*/
    readSettings();
}


nmfSetup_Tab2::~nmfSetup_Tab2()
{
    saveSettings();
}

void
nmfSetup_Tab2::enableProjectData()
{
    Setup_Tab2_ProjectDataGB->setChecked(true);
}

void
nmfSetup_Tab2::enableSetupTabs(bool enable)
{
    for (int i=2; i<Setup_Tabs->count(); ++i) {
        Setup_Tabs->setTabEnabled(i,enable);
    }
}

void
nmfSetup_Tab2::clearProjectData()
{
    setProjectAuthor("");
    setProjectDescription("");
    setProjectDirectory("");
    setProjectName("");
}


void
nmfSetup_Tab2::setProjectName(QString name)
{
    Setup_Tab2_ProjectNameLE->setText(name);
}

void
nmfSetup_Tab2::setProjectDirectory(QString dir)
{
    Setup_Tab2_ProjectDirLE->setText(dir);
}

void
nmfSetup_Tab2::setProjectAuthor(QString author)
{
    Setup_Tab2_ProjectAuthorLE->setText(author);
}
void
nmfSetup_Tab2::setProjectDescription(QString desc)
{
    Setup_Tab2_ProjectDescLE->setText(desc);
}

void
nmfSetup_Tab2::setProjectDatabase(QString dbName)
{
    if (nmfUtils::isOSWindows()) {
        Setup_Tab2_ProjectDatabaseCMB->setCurrentText(dbName.toLower());
    } else {
        Setup_Tab2_ProjectDatabaseCMB->setCurrentText(dbName);
    }
}

//void
//nmfSetupTab2::updateOutputWidget()
//{
//    char buf[200];
//
//    nmfQtUtils::sendToOutputWindow(SetupOutputTE, nmfConstants::Clear);
//
//    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "\n~~~~~~~~~~~~~~~~~~~", "");
//    nmfQtUtils::sendToOutputWindow(SetupOutputTE,   "Project Information", "");
//    nmfQtUtils::sendToOutputWindow(SetupOutputTE,   "~~~~~~~~~~~~~~~~~~~", "");
//
//    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "\nProject directory: ", ProjectDir);
//    nmfQtUtils::sendToOutputWindow(SetupOutputTE,   "Project database:  ", ProjectDatabase);
//    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "All Databases: ", "");
//    for (std::string dbName : ValidDatabases) {
//        nmfQtUtils::sendToOutputWindow(SetupOutputTE, "    ", dbName);
//    }
//    std::string timePeriod = std::to_string(FirstYear) + " to " + std::to_string(LastYear);
//    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "\nTime Period: ", timePeriod);
//    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "Num Seasons: ", std::to_string(NumSeasons));
//
//    //ageStr = "Min: " + std::to_string(MinCatchAge) + ", Max: " + std::to_string(MaxCatchAge);
//    //nmfQtUtils::sendToOutputWindow(SetupOutputTE, "\nSpecies Catch Age: ", ageStr);
//    sprintf(buf,"\n%-20s          %14s %14s","Species","Min Catch Age","Max Catch Age");
//    nmfQtUtils::sendToOutputWindow(SetupOutputTE, std::string(buf), "");
//    for (std::string species : Species) {
//        sprintf(buf,"%-20s %14d %14d",species.c_str(),MinCatchAge[species],MaxCatchAge[species]);
//        nmfQtUtils::sendToOutputWindow(SetupOutputTE, "    ", std::string(buf));
//    }
//    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "Other Predators: ", "");
//    for (std::string othPred : OtherPredators) {
//        nmfQtUtils::sendToOutputWindow(SetupOutputTE, "    ", othPred);
//    }
//    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "Prey: ", "");
//    for (std::string prey : Prey) {
//        nmfQtUtils::sendToOutputWindow(SetupOutputTE, "    ", prey);
//    }
//} // end updateOutputWidget


//void
//nmfSetupTab2::callback_Setup_Tab2_MinCatchAgeAdd()
//{
//    std::string currentSpecies = Setup_Tab2_SpeciesCMB->currentText().toStdString();
//    if (! currentSpecies.empty()) {
//        MinCatchAge[currentSpecies] = Setup_Tab2_MinCatchAgeLE->text().toInt();
//    }
//    updateOutputWidget();
//}

//void
//nmfSetupTab2::callback_Setup_Tab2_MaxCatchAgeAdd()
//{
//    //MaxCatchAge = Setup_Tab2_MaxCatchAgeLE->text().toInt();
//    std::string currentSpecies = Setup_Tab2_SpeciesCMB->currentText().toStdString();
//    if (! currentSpecies.empty()) {
//        MaxCatchAge[currentSpecies] = Setup_Tab2_MaxCatchAgeLE->text().toInt();
//    }
//    updateOutputWidget();
//}


bool
nmfSetup_Tab2::isProjectDataValid()
{
    return (! (Setup_Tab2_ProjectNameLE->text().isEmpty()             ||
               Setup_Tab2_ProjectDirLE->text().isEmpty()              ||
               Setup_Tab2_ProjectDatabaseCMB->currentText().isEmpty() ||
               Setup_Tab2_ProjectAuthorLE->text().isEmpty()           ||
               Setup_Tab2_ProjectDescLE->text().isEmpty()             ||
               Setup_Tab2_FirstYearLE->text().isEmpty()               ||
               Setup_Tab2_LastYearLE->text().isEmpty()                ||
               Setup_Tab2_NumSeasonsLE->text().isEmpty()));

} // end validProjectData


bool
nmfSetup_Tab2::validateYearSeasonFields(int newFirstYear,
                                       int newLastYear,
                                       int newNumSeasons)
{
    int NumRecordsSpecies = 0;
    int NumRecordsOtherPredSpecies = 0;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    int speciesFirstYear;
    int speciesLastYear;
    std::string msg;
    bool isValid = false;

std::cout << "NewProject: " << NewProject << std::endl;
    //
    // See if FirstYear, LastYear, or Num Seasons/Year have changed from any of the species.
    // If they have don't allow it since many tables will be out of sync.
    //
    fields = {"FirstYear","LastYear"};
    queryStr = "SELECT FirstYear,LastYear FROM Species";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecordsSpecies = dataMap["FirstYear"].size();
    msg  = "\nWarning:\n";
    msg += "\nCan't change year or season data once Species have been created.";
    msg += "\n\nPlease either:\n\n";
    msg += "(1) Create another Project with new Species if you'd like to ";
    msg += "use different year or season data.\n\n   or\n\n";
    msg += "(2) Create new Species in this Project and Re-Create the Template files.\n";
    for (int i=0; i< NumRecordsSpecies; ++i) {
        speciesFirstYear = std::stoi(dataMap["FirstYear"][i]);
        speciesLastYear  = std::stoi(dataMap["LastYear"][i]);
        if (! NewProject && ((newFirstYear != speciesFirstYear) ||
                             (newLastYear  != speciesLastYear)))
        {
            QMessageBox::warning(Setup_Tabs, tr("Warning"), tr(msg.c_str()), QMessageBox::Ok);
            isValid = false;
        }
    }

    fields = {"FirstYear","LastYear"};
    queryStr = "SELECT FirstYear,LastYear FROM OtherPredSpecies";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecordsOtherPredSpecies = dataMap["FirstYear"].size();
    for (int i=0; i< NumRecordsOtherPredSpecies; ++i) {
        speciesFirstYear = std::stoi(dataMap["FirstYear"][i]);
        speciesLastYear  = std::stoi(dataMap["LastYear"][i]);
        if (! NewProject && ((newFirstYear != speciesFirstYear) ||
                             (newLastYear  != speciesLastYear)))
        {
            QMessageBox::warning(Setup_Tabs, tr("Warning"), tr(msg.c_str()), QMessageBox::Ok);
            isValid = false;
        }
    }

    // if reach here, years are OK.  Don't allow num seasons to change if there are already
    // some species or otherpredspecies.
    if (! NewProject && (NumSeasons != newNumSeasons)) {
        if ((NumRecordsSpecies > 0) || (NumRecordsOtherPredSpecies > 0)) {
            QMessageBox::warning(Setup_Tabs, tr("Warning"), tr(msg.c_str()), QMessageBox::Ok);
            isValid = false;
        }
    }

    if (! isValid) {
        Setup_Tab2_SaveProjectPB->setDown(false);
        Setup_Tab2_SaveProjectPB->clearMask();
        Setup_Tab2_SaveProjectPB->clearFocus();
    }

    return true;
} // end validateYearSeasonFields

void
nmfSetup_Tab2::callback_Setup_Tab2_SaveProject()
{
    QString fullFilename;
    std::string msg;
    QMessageBox::StandardButton reply;
    QString fileSuffix;
    std::ofstream outFile;

    if (! isProjectDataValid()) {
        QMessageBox::warning(Setup_Tabs,
                             tr("Missing Data"),
                             tr("\nPlease enter missing data above."),
                             QMessageBox::Ok);
        return;
    }

    ProjectName = Setup_Tab2_ProjectNameLE->text();


    if (! validateYearSeasonFields(FirstYear,LastYear,NumSeasons))
        return;

    // Build the filename, add the .prj prefix if one isn't already there and
    // check to see if the file already exists.
    fullFilename = QDir(ProjectDir).filePath(ProjectName);
    fileSuffix =  QFileInfo(fullFilename).suffix();
    if (fileSuffix != "prj")
        fullFilename += ".prj";
//std::cout << "fn: " << fullFilename.toStdString() << std::endl;
    if (QFileInfo(fullFilename).exists()) {
        msg  = "\nThe project file already exists:\n\n" + fullFilename.toStdString() + "\n\n";
        msg += "OK to overwrite it?\n";
        reply = QMessageBox::question(Setup_Tabs,
                                      tr("File Found"),
                                      tr(msg.c_str()),
                                      QMessageBox::Yes|QMessageBox::No,
                                      QMessageBox::Yes);
        if (reply == QMessageBox::No) {
           return;
        }
    }

    // Write out the project file
    outFile = std::ofstream(fullFilename.toLatin1());
    outFile << "#\n";
    outFile << "# This is an MSVPA_X2 Project file.\n";
    outFile << "#\n";
    outFile << "# It defines the Project database as well as other\n";
    outFile << "# information for the Project. The following data are:\n";
    outFile << "# Project directory, Project database, Author, Description,\n";
    outFile << "# FirstYear, LastYear, and Num Seasons.\n";
    outFile << "#\n";
    outFile << ProjectDir.toStdString() << "\n";
    outFile << ProjectDatabase.toStdString() << "\n";
    outFile << ProjectAuthor.toStdString() << "\n";
    outFile << ProjectDescription.toStdString() << "\n";
    outFile << FirstYear << "\n";
    outFile << LastYear << "\n";
    outFile << NumSeasons << "\n";
    outFile.close();

    if (LastProjectDatabase != ProjectDatabase) {
        msg  = "\nWarning: Changing databases may require you to re-create your CSV file templates for this project.  ";
        msg += "Failure to do so will result in the previous project's CSV files being used with the new database, ";
        msg += "which may have undesirable effects.\n\nTo re-create CSV files:\n\nFile -> Regenerate CSV Files from Database...";
        QMessageBox::warning(Setup_Tabs,
                             tr("Warning"),
                             tr(msg.c_str()),
                             QMessageBox::Ok);
    }

    LastProjectDatabase = ProjectDatabase;

    emit ProjectSet();

} // end callback_Setup_Tab2_SaveProject



void
nmfSetup_Tab2::callback_Setup_Tab2_AddDatabase()
{
    bool ok;
    std::string msg;
    std::string cmd;
    std::string errorMsg="";

    QString enteredName = QInputDialog::getText(Setup_Tabs,
                                         tr("Add Database"),
                                         tr("Enter new database name:"),
                                         QLineEdit::Normal,
                                         "", &ok);
    if (! ok || enteredName.isEmpty())
        return;

    if (ValidDatabases.find(enteredName.toStdString()) != ValidDatabases.end()) // Means it found it
    {
        msg = "\nDatabase already exists. Please enter another database name.";
        QMessageBox::critical(Setup_Tabs,tr("Invalid Database Name"),tr(msg.c_str()),QMessageBox::Ok);
        return;
    }

    Setup_Tabs->setCursor(Qt::WaitCursor);

    // OK to now add the database
    cmd = "CREATE database " + enteredName.toStdString();
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (errorMsg != " ") {
        nmfUtils::printError("Function: callback_Setup_Tab2_AddDatabase ",errorMsg);
        errorMsg = "\n" + errorMsg;
        QMessageBox::information(Setup_Tabs, tr("Create Error"),
                                 tr(errorMsg.c_str()));
    } else {
        emit LoadDatabase(enteredName);
        emit CreateTables();
        loadDatabaseNames(enteredName);
        Setup_Tab2_ProjectDatabaseCMB->setCurrentText(enteredName);
        std::string msg = "\nDatabase created: " + enteredName.toStdString() + "\n";
        QMessageBox::information(Setup_Tabs, "Set up Project",
                                 tr(msg.c_str()));
    }

    // Turn off other Setup groups
    Setup_Tab2_TimeDataGB->setChecked(false);
    Setup_Tabs->findChild<QCheckBox *>("Setup_Tab3_SpeciesCB")->setChecked(false);
    Setup_Tabs->findChild<QCheckBox *>("Setup_Tab3_OtherPredatorsCB")->setChecked(false);

    Setup_Tabs->setCursor(Qt::ArrowCursor);

} // end callback_Setup_Tab2_AddDatabase


void
nmfSetup_Tab2::callback_Setup_Tab2_DelDatabase()
{
    bool deleteOK=true;
    std::string cmd;
    std::string msg;
    std::string errorMsg="";
    QString databaseToDelete = Setup_Tab2_ProjectDatabaseCMB->currentText();
    int databaseIndexToDelete = Setup_Tab2_ProjectDatabaseCMB->currentIndex();

    //std::cout << "Database delete not fully implemented." << std::endl;
    QMessageBox::StandardButton reply;

    msg = "\nOK to permanently delete the database: " +
            databaseToDelete.toStdString() + " ?\n";
    reply = QMessageBox::question(Setup_Tabs, tr("Delete Database"),
         tr(msg.c_str()), QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {

        QApplication::setOverrideCursor(Qt::WaitCursor);

        // Remove database from mysql and reload widget
        cmd = "DROP database " + databaseToDelete.toStdString();
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if (errorMsg != " ") {
            Setup_Tabs->setCursor(Qt::ArrowCursor);
            nmfUtils::printError("Function: callback_Setup_Tab2_DelDatabase ",errorMsg);
            deleteOK = false;
        }
        loadDatabaseNames("");
        Setup_Tab2_ProjectDatabaseCMB->setCurrentIndex(databaseIndexToDelete-1);

        QApplication::restoreOverrideCursor();

        if (deleteOK) {
            msg = "\nDatabase deleted: " + databaseToDelete.toStdString() + "\n";
            QMessageBox::information(Setup_Tabs, "Delete",
                                     tr(msg.c_str()));
        }

    }
    //Setup_Tab2_SpeciesCMB->removeItem(Setup_Tab2_SpeciesCMB->currentIndex());
    //updateOutputWidget();


} // end callback_Setup_Tab2_DelDatabase

void
nmfSetup_Tab2::callback_Setup_Tab2_RefreshDatabase()
{
    loadDatabaseNames(Setup_Tab2_ProjectDatabaseCMB->currentText());
    QMessageBox::information(Setup_Tabs, "Refresh", "\nDatabase list refreshed.\n", QMessageBox::Ok);
}

void
nmfSetup_Tab2::callback_Setup_Tab2_SetProject()
{
    logger->logMsg(nmfConstants::Normal,"nmfSetupTab2::callback_Setup_Tab2_SetProject");

    // Set project variables
    ProjectName        = Setup_Tab2_ProjectNameLE->text();
    ProjectDir         = Setup_Tab2_ProjectDirLE->text();
    ProjectDatabase    = Setup_Tab2_ProjectDatabaseCMB->currentText();
    ProjectAuthor      = Setup_Tab2_ProjectAuthorLE->text();
    ProjectDescription = Setup_Tab2_ProjectDescLE->text();
    FirstYear          = Setup_Tab2_FirstYearLE->text().toInt();
    LastYear           = Setup_Tab2_LastYearLE->text().toInt();
    NumSeasons         = Setup_Tab2_NumSeasonsLE->text().toInt();

    saveSettings();
    emit LoadDatabase(ProjectDatabase);
    readSettings();

    int index = Setup_Tab2_ProjectDatabaseCMB->findText(ProjectDatabase);
    Setup_Tab2_ProjectDatabaseCMB->setCurrentIndex(index);
    emit ProjectSet();

    logger->logMsg(nmfConstants::Normal,"nmfSetupTab2::callback_Setup_Tab2_SetProject Complete");

} // end callback_Setup_Tab2_SetProject




void
nmfSetup_Tab2::callback_Setup_Tab2_DatabaseChanged(QString db)
{
    LastProjectDatabase = ProjectDatabase;
    ProjectDatabase = db; //Setup_Tab2_ProjectDatabaseCMB->currentText();
    //updateOutputWidget();
} // end callback_Setup_Tab2_DatabaseChanged



void
nmfSetup_Tab2::callback_ProjectDataGB(bool checked)
{
    Setup_Tab2_TimeDataGB->setChecked(checked);
    Setup_Tabs->findChild<QCheckBox *>("Setup_Tab3_SpeciesCB")->setChecked(checked);
    Setup_Tabs->findChild<QCheckBox *>("Setup_Tab3_OtherPredatorsCB")->setChecked(checked);
} // end callback_ProjectDataGB

void
nmfSetup_Tab2::callback_Setup_Tab2_ProjectDirBrowsePB()
{
    // Launch file dialog
    QString dir = QFileDialog::getExistingDirectory(
                Setup_Tabs,
                tr("Open Project Directory"),
                ProjectDir+"/..",
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (! dir.isEmpty()) {
        Setup_Tab2_ProjectDirLE->setText(dir);
        ProjectDir = dir;
    }
}

void
nmfSetup_Tab2::callback_Setup_Tab2_ProjectDirAdd()
{
    ProjectDir = Setup_Tab2_ProjectDirLE->text();
    //updateOutputWidget();
}

void
nmfSetup_Tab2::callback_Setup_Tab2_ProjectNameAdd()
{
    ProjectName = Setup_Tab2_ProjectNameLE->text();
    NewProject = true;
    //updateOutputWidget();
}

void
nmfSetup_Tab2::callback_Setup_Tab2_ProjectAuthorAdd()
{
    ProjectAuthor = Setup_Tab2_ProjectAuthorLE->text();
    //updateOutputWidget();
}

void
nmfSetup_Tab2::callback_Setup_Tab2_ProjectDescAdd()
{
    ProjectDescription = Setup_Tab2_ProjectDescLE->text();
    //updateOutputWidget();
}

void
nmfSetup_Tab2::callback_Setup_Tab2_FirstYearAdd()
{
    int yearEntered = Setup_Tab2_FirstYearLE->text().toInt();


    if (validateYearSeasonFields(yearEntered,LastYear,NumSeasons))
    {
        FirstYear = yearEntered;
    }
    Setup_Tab2_FirstYearLE->blockSignals(true);
    Setup_Tab2_FirstYearLE->setText(QString::number(FirstYear));
    Setup_Tab2_FirstYearLE->blockSignals(false);

    //updateOutputWidget();
}

void
nmfSetup_Tab2::callback_Setup_Tab2_LastYearAdd()
{    
    int yearEntered = Setup_Tab2_LastYearLE->text().toInt();

    if (validateYearSeasonFields(FirstYear,yearEntered,NumSeasons))
    {
        LastYear = yearEntered;
    }
    Setup_Tab2_LastYearLE->blockSignals(true);
    Setup_Tab2_LastYearLE->setText(QString::number(LastYear));
    Setup_Tab2_LastYearLE->blockSignals(false);

    //updateOutputWidget();
}

void
nmfSetup_Tab2::callback_Setup_Tab2_NumSeasonsAdd()
{
    int numSeasonsEntered = Setup_Tab2_NumSeasonsLE->text().toInt();

    if (validateYearSeasonFields(FirstYear,LastYear,numSeasonsEntered))
    {
        NumSeasons = numSeasonsEntered;
    }
    Setup_Tab2_NumSeasonsLE->blockSignals(true);
    Setup_Tab2_NumSeasonsLE->setText(QString::number(NumSeasons));
    Setup_Tab2_NumSeasonsLE->blockSignals(false);

    //updateOutputWidget();
}


void
nmfSetup_Tab2::loadProject(QString fileName)
{    

    // If the file doesn't have an extension, add .prj
    if (QFileInfo(fileName).suffix().isEmpty()) {
        fileName += ".prj";
    }

    // Read file and load data......
    QFile file(fileName);
    if (! file.open(QIODevice::ReadOnly)) {
        logger->logMsg(nmfConstants::Normal,"Error nmfSetupTab2::loadProject: Cannot open file for reading: "+fileName.toStdString());
        return;
    }
    QTextStream in(&file);
    logger->logMsg(nmfConstants::Normal,"Reading Project File: " + fileName.toStdString());

    // Skip initial comments
    QString line = in.readLine();
    while (line.trimmed()[0] == "#") {
        line = in.readLine();
    }
    ProjectDir         = line;
    ProjectDatabase    = in.readLine();
    ProjectAuthor      = in.readLine();
    ProjectDescription = in.readLine();
    FirstYear          = in.readLine().toInt();
    LastYear           = in.readLine().toInt();
    NumSeasons         = in.readLine().toInt();
    Setup_Tab2_ProjectNameLE->clear();
    Setup_Tab2_ProjectDirLE->clear();
    //Setup_Tab2_ProjectDatabaseCMB->clear();
    Setup_Tab2_ProjectAuthorLE->clear();
    Setup_Tab2_ProjectDescLE->clear();
    Setup_Tab2_FirstYearLE->clear();
    Setup_Tab2_LastYearLE->clear();
    Setup_Tab2_NumSeasonsLE->clear();

    QFileInfo filenameNoPath(fileName);
    Setup_Tab2_ProjectNameLE->setText(filenameNoPath.baseName());
    Setup_Tab2_ProjectDirLE->setText(ProjectDir);
    Setup_Tab2_ProjectDatabaseCMB->setCurrentText(ProjectDatabase);
    Setup_Tab2_ProjectAuthorLE->setText(ProjectAuthor);
    Setup_Tab2_ProjectDescLE->setText(ProjectDescription);
    Setup_Tab2_FirstYearLE->setText(QString::number(FirstYear));
    Setup_Tab2_LastYearLE->setText(QString::number(LastYear));
    Setup_Tab2_NumSeasonsLE->setText(QString::number(NumSeasons));

    file.close();

    logger->logMsg(nmfConstants::Normal,"loadProject end");

} // end loadProject



void
nmfSetup_Tab2::callback_Setup_Tab2_BrowseProject()
{
    QString fileName = QFileDialog::getOpenFileName(Setup_Tabs,
        tr("Load a Project"), ProjectDir, tr("Project Files (*.prj)"));

    if (! fileName.isEmpty()) {
        loadProject(fileName);
        //updateOutputWidget();
        NewProject = true;
    }

   Setup_Tab2_BrowseProjectPB->clearFocus();

} // end callback_Setup_Tab2_BrowseProject



void
nmfSetup_Tab2::callback_Setup_Tab2_NewProject()
{
    Setup_Tab2_ProjectNameLE->clear();
    Setup_Tab2_ProjectAuthorLE->clear();
    Setup_Tab2_ProjectDescLE->clear();
    Setup_Tab2_ProjectDatabaseCMB->clearEditText();
    Setup_Tab2_FirstYearLE->clear();
    Setup_Tab2_LastYearLE->clear();
    Setup_Tab2_NumSeasonsLE->clear();
} // end callback_Setup_Tab2_NewProject




//void
//nmfSetupTab2::callback_Setup_Tab2_SpeciesAdd()
//{
//    QLineEdit *SpeciesLE = Setup_Tab2_SpeciesCMB->lineEdit();
//    std::string species = SpeciesLE->text().toStdString();
//    Species.push_back(species);

//    SpeciesLE->clear();
//    updateOutputWidget();
//}

//void
//nmfSetupTab2::callback_Setup_Tab2_OtherPredatorAdd()
//{
//    QLineEdit *OtherPredatorLE = Setup_Tab2_OtherPredatorCMB->lineEdit();
//    std::string otherPred = OtherPredatorLE->text().toStdString();
//    OtherPredators.push_back(otherPred);
//    OtherPredatorLE->clear();
//    updateOutputWidget();
//}

//void
//nmfSetupTab2::callback_Setup_Tab2_PreyAdd()
//{
//    QLineEdit *PreyLE = Setup_Tab2_PreyCMB->lineEdit();
//    std::string prey = PreyLE->text().toStdString();
//    Prey.push_back(prey);
//    PreyLE->clear();
//    updateOutputWidget();
//}






//void
//nmfSetupTab2::callback_Setup_Tab2_SpeciesDel()
//{
//    std::set<std::string>::iterator it;
//    std::string speciesToRemove;

//    speciesToRemove = Setup_Tab2_SpeciesCMB->currentText().toStdString();

//    int sizeBefore = Species.size();
//    boost::remove_erase(Species,speciesToRemove);
//    int sizeAfter  = Species.size();
//    if (sizeBefore != sizeAfter) {
//        Setup_Tab2_SpeciesCMB->removeItem(Setup_Tab2_SpeciesCMB->currentIndex());
//    }

//    updateOutputWidget();
//}

//void
//nmfSetupTab2::callback_Setup_Tab2_OtherPredatorDel()
//{
//    std::set<std::string>::iterator it;
//    std::string otherPredatorsToRemove;

//    otherPredatorsToRemove = Setup_Tab2_OtherPredatorCMB->currentText().toStdString();

//    int sizeBefore = OtherPredators.size();
//    boost::remove_erase(OtherPredators,otherPredatorsToRemove);
//    int sizeAfter  = OtherPredators.size();
//    if (sizeBefore != sizeAfter) {
//        Setup_Tab2_OtherPredatorCMB->removeItem(Setup_Tab2_OtherPredatorCMB->currentIndex());
//    }


//    updateOutputWidget();
//}

//void
//nmfSetupTab2::callback_Setup_Tab2_PreyDel()
//{
//    std::set<std::string>::iterator it;
//    std::string preyToRemove;

////    it = Prey.find(Setup_Tab2_PreyCMB->currentText().toStdString());
////    if (it != Prey.end()) {
////        Prey.erase(it);
////        Setup_Tab2_PreyCMB->removeItem(Setup_Tab2_PreyCMB->currentIndex());
////    }


//    preyToRemove = Setup_Tab2_PreyCMB->currentText().toStdString();

//    int sizeBefore = Prey.size();
//    boost::remove_erase(Prey,preyToRemove);
//    int sizeAfter  = Prey.size();
//    if (sizeBefore != sizeAfter) {
//        Setup_Tab2_PreyCMB->removeItem(Setup_Tab2_PreyCMB->currentIndex());
//    }
//    updateOutputWidget();
//}


//void
//nmfSetupTab2::callback_Setup_Tab2_CurrentSpeciesChanged(QString species)
//{
//    Setup_Tab2_MinCatchAgeLE->setText(QString::number(MinCatchAge[species.toStdString()]));
//    Setup_Tab2_MaxCatchAgeLE->setText(QString::number(MaxCatchAge[species.toStdString()]));
//}



void
nmfSetup_Tab2::readSettings()
{
    // Read the settings and load into class variables.
    QSettings* settings = nmfUtilsQt::createSettings(nmfConstantsMSVPA::SettingsDirWindows,"MSVPA_X2");

    settings->beginGroup("SetupTab");
    ProjectName        = settings->value("ProjectName","").toString();
    ProjectDir         = settings->value("ProjectDir","").toString();
    ProjectDatabase    = settings->value("ProjectDatabase","").toString();
    ProjectAuthor      = settings->value("ProjectAuthor","").toString();
    ProjectDescription = settings->value("ProjectDescription","").toString();
    FirstYear          = settings->value("FirstYear",1900).toInt();
    LastYear           = settings->value("LastYear",2000).toInt();
    NumSeasons         = settings->value("NumSeasons",4).toInt();
    settings->endGroup();

    // Load class variables into appropriate widgets.
    Setup_Tab2_ProjectNameLE->setText(ProjectName);
    Setup_Tab2_ProjectDirLE->setText(ProjectDir);
    Setup_Tab2_ProjectAuthorLE->setText(ProjectAuthor);
    Setup_Tab2_ProjectDescLE->setText(ProjectDescription);
    Setup_Tab2_FirstYearLE->setText(QString::number(FirstYear));
    Setup_Tab2_LastYearLE->setText(QString::number(LastYear));
    Setup_Tab2_NumSeasonsLE->setText(QString::number(NumSeasons));
    Setup_Tab2_ProjectDatabaseCMB->setCurrentText(ProjectDatabase);

    delete settings;

} // end readSettings


void
nmfSetup_Tab2::saveSettings()
{
    QSettings* settings = nmfUtilsQt::createSettings(nmfConstantsMSVPA::SettingsDirWindows,"MSVPA_X2");

    settings->beginGroup("SetupTab");
    settings->setValue("ProjectName",        ProjectName);
    settings->setValue("ProjectDir",         ProjectDir);
    settings->setValue("ProjectDatabase",    ProjectDatabase);
    settings->setValue("ProjectAuthor",      ProjectAuthor);
    settings->setValue("ProjectDescription", ProjectDescription);
    settings->setValue("FirstYear",          FirstYear);
    settings->setValue("LastYear",           LastYear);
    settings->setValue("NumSeasons",         NumSeasons);
    settings->endGroup();

    delete settings;

} // end saveSettings


void
nmfSetup_Tab2::loadDatabaseNames(QString NameToSelect)
{
    QList<QString> authenticatedDatabases = {};
    ValidDatabases.clear();
    Setup_Tab2_ProjectDatabaseCMB->clear();
    // Get the list of authenticated databases and add them to the appropriate widget
    if (databasePtr->getListOfAuthenticatedDatabaseNames(authenticatedDatabases)) {
        foreach (QString item, authenticatedDatabases) {
            ValidDatabases.insert(item.toStdString());
            Setup_Tab2_ProjectDatabaseCMB->addItem(item);
        }
    }
    if (! NameToSelect.isEmpty()) {
        Setup_Tab2_ProjectDatabaseCMB->setCurrentText(NameToSelect);
    } else {
        Setup_Tab2_ProjectDatabaseCMB->setCurrentIndex(0);
    }
}


void
nmfSetup_Tab2::loadWidgets(nmfDatabase *theDatabasePtr)
{

    databasePtr = theDatabasePtr;
    loadDatabaseNames("");
    readSettings();

} // end loadWidgets


void
nmfSetup_Tab2::clearWidgets()
{

}
