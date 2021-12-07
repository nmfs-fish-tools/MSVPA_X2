
#include "nmfForecastTab01.h"
#include "nmfConstants.h"
#include "nmfUtils.h"


nmfForecastTab1::nmfForecastTab1(QTabWidget  *tabs,
                                 nmfLogger   *theLogger,
                                 std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfForecastTab1::nmfForecastTab1");

    Forecast_Tabs = tabs;
    ProjectDir = theProjectDir;
    databasePtr = NULL;

    // Load ui as a widget from disk
    QFile file(":/forms/Forecast/Forecast_Tab01.ui");
    file.open(QFile::ReadOnly);
    Forecast_Tab1_Widget = loader.load(&file,Forecast_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    Forecast_Tabs->addTab(Forecast_Tab1_Widget, tr("1. Forecast Configuration"));

    Forecast_Tab1_NextPB           = Forecast_Tabs->findChild<QPushButton *>("Forecast_Tab1_NextPB");
    Forecast_Tab1_SavePB           = Forecast_Tabs->findChild<QPushButton *>("Forecast_Tab1_SavePB");
    Forecast_Tab1_MsvpaNameLE      = Forecast_Tabs->findChild<QLineEdit   *>("Forecast_Tab1_MsvpaNameLE");
    Forecast_Tab1_ForecastNameLE   = Forecast_Tabs->findChild<QLineEdit   *>("Forecast_Tab1_ForecastNameLE");
    Forecast_Tab1_ScenarioNameLE   = Forecast_Tabs->findChild<QLineEdit   *>("Forecast_Tab1_ScenarioNameLE");
    Forecast_Tab1_ScenarioNameLBL  = Forecast_Tabs->findChild<QLabel      *>("Forecast_Tab1_ScenarioNameLBL");
    Forecast_Tab1_InitialYearCMB   = Forecast_Tabs->findChild<QComboBox   *>("Forecast_Tab1_InitialYearCMB");
    Forecast_Tab1_NumYearsLE       = Forecast_Tabs->findChild<QLineEdit   *>("Forecast_Tab1_NumYearsLE");
    Forecast_Tab1_PredatorGrowthCB = Forecast_Tabs->findChild<QCheckBox   *>("Forecast_Tab1_PredatorGrowthCB");

    // RSK - enable this if and when you enable growth in MSVPA and Forecast
    Forecast_Tab1_PredatorGrowthCB->setEnabled(false);

    Forecast_Tab1_NextPB->setText("--\u25B7");

    Forecast_Tab1_NumYearsLE->setValidator(new QIntValidator(Forecast_Tabs));

    connect(Forecast_Tab1_NextPB, SIGNAL(clicked(bool)),
            this,                 SLOT(callback_Forecast_Tab1_NextPB(bool)));
    connect(Forecast_Tab1_SavePB, SIGNAL(clicked(bool)),
            this,                 SLOT(callback_Forecast_Tab1_SavePB(bool)));

    connect(Forecast_Tab1_InitialYearCMB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(callback_TableDataChanged(int)));
    connect(Forecast_Tab1_NumYearsLE,     SIGNAL(returnPressed()),
            this, SLOT(callback_TableDataChanged()));

    connect(Forecast_Tab1_MsvpaNameLE,    SIGNAL(editingFinished()),
            this,                         SLOT(callback_StripMsvpaNameLE()));
    connect(Forecast_Tab1_ForecastNameLE, SIGNAL(editingFinished()),
            this,                         SLOT(callback_StripForecastNameLE()));
    connect(Forecast_Tab1_ScenarioNameLE, SIGNAL(editingFinished()),
            this,                         SLOT(callback_StripScenarioNameLE()));
    connect(Forecast_Tab1_NumYearsLE,     SIGNAL(editingFinished()),
            this,                         SLOT(callback_StripNumYearsLE()));

    Forecast_Tab1_MsvpaNameLE->setReadOnly(true);
    Forecast_Tab1_ForecastNameLE->setReadOnly(true);
    //Forecast_Tab1_NumYearsLE->setReadOnly(true);

//    Forecast_Tab1_ScenarioNameLE->setReadOnly(true);
//    Forecast_Tab1_ScenarioNameLBL->hide(); // unneeded here
//    Forecast_Tab1_ScenarioNameLBL->setFixedHeight(0);
//    Forecast_Tab1_ScenarioNameLE->hide();  // unneeded here
//    Forecast_Tab1_ScenarioNameLE->setFixedHeight(0);
//    QPalette pal = Forecast_Tab1_MsvpaNameLE->palette();
//    pal.setColor(Forecast_Tab1_MsvpaNameLE->backgroundRole(),QColor(Qt::lightGray));
//    Forecast_Tab1_MsvpaNameLE->setPalette(pal);
//    Forecast_Tab1_ForecastNameLE->setPalette(pal);
//    Forecast_Tab1_ScenarioNameLE->setPalette(pal);

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab1::nmfForecastTab1 Complete");
}


nmfForecastTab1::~nmfForecastTab1()
{
}


void
nmfForecastTab1::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir



void
nmfForecastTab1::saveTheForecast(std::string MSVPAName,
                                 std::string Forecast,
                                 std::string Scenario,
                                 int InitYear,
                                 int NumYears,
                                 int ModelPredatorGrowth)
{
    logger->logMsg(nmfConstants::Normal,"nmfForecastTab1::saveTheForecast");

    std::string cmd;
    std::string errorMsg= "";

    // Add the Forecast to the Forecast table
    cmd  = "INSERT INTO " + nmfConstantsMSVPA::TableForecasts;
    cmd += " (MSVPAName,ForeName,InitYear,NYears,Growth) values ";
    cmd += "(\"" + MSVPAName + "\",\"" + Forecast + "\"" +
            "," + std::to_string(InitYear) +
            "," + std::to_string(NumYears) +
            "," + std::to_string(ModelPredatorGrowth) + ") ";
    cmd += "ON DUPLICATE KEY UPDATE ";
    cmd += "InitYear=values(InitYear),NYears=values(NYears),Growth=values(Growth); ";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("saveTheForecast: INSERT INTO Forecasts...", errorMsg);
    }

    // Add the Scenario to the Scenario table
    cmd  = "INSERT INTO " + nmfConstantsMSVPA::TableScenarios;
    cmd += " (MSVPAName,ForeName,Scenario,VarF,VarOthPred,VarOthPrey,VarRec,FishAsF) values ";
    cmd += "(\"" + MSVPAName + "\",\"" + Forecast + "\",\"" + Scenario + "\"" +
            ", 1, 0, 0, 0, 1) ";
    cmd += "ON DUPLICATE KEY UPDATE ";
    cmd += "VarF=values(VarF),VarOthPred=values(VarOthPred),VarOthPrey=values(VarOthPrey),VarRec=values(VarRec),FishAsF=values(FishAsF); ";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("saveTheForecast: INSERT INTO Scenarios...", errorMsg);
    }

    emit UpdateScenarioList(Scenario);

    restoreCSVFromDatabase(databasePtr);

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab1::saveTheForecast Complete");

} // end saveTheForecast


void
nmfForecastTab1::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableScenarios);
    fields    = {"MSVPAName","ForeName","Scenario","VarF", "VarOthPrey","VarRec","FishAsF"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableForecasts);
    fields    = {"MSVPAName","ForeName","InitYear","NYears", "Growth"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase


void
nmfForecastTab1::callback_StripMsvpaNameLE()
{
    Forecast_Tab1_MsvpaNameLE->setText(Forecast_Tab1_MsvpaNameLE->text().trimmed());
    MarkAsDirty(nmfConstantsMSVPA::TableForecasts);
    MarkAsDirty(nmfConstantsMSVPA::TableScenarios);

}

void
nmfForecastTab1::callback_StripForecastNameLE()
{
    Forecast_Tab1_ForecastNameLE->setText(Forecast_Tab1_ForecastNameLE->text().trimmed());
    MarkAsDirty(nmfConstantsMSVPA::TableForecasts);
    MarkAsDirty(nmfConstantsMSVPA::TableScenarios);
}

void
nmfForecastTab1::callback_StripScenarioNameLE()
{
    Forecast_Tab1_ScenarioNameLE->setText(Forecast_Tab1_ScenarioNameLE->text().trimmed());
    MarkAsDirty(nmfConstantsMSVPA::TableScenarios);
}

void
nmfForecastTab1::callback_StripNumYearsLE()
{
    Forecast_Tab1_NumYearsLE->setText(Forecast_Tab1_NumYearsLE->text().trimmed());
    MarkAsDirty(nmfConstantsMSVPA::TableForecasts);
}


void
nmfForecastTab1::callback_Forecast_Tab1_SavePB(bool unused)
{
    int InitYear;
    int NumRecords;
    int ModelPredatorGrowth;

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab1::callback_Forecast_Tab1_SavePB");

    std::string queryStr = "";
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    std::string MSVPAName = Forecast_Tab1_MsvpaNameLE->text().toStdString();
    std::string Forecast  = Forecast_Tab1_ForecastNameLE->text().toStdString();
    int NumYears          = Forecast_Tab1_NumYearsLE->text().toInt();
    std::string Scenario  = Forecast_Tab1_ScenarioNameLE->text().toStdString();

    if (databasePtr == NULL) {
        std::cout << "Error: Can't save forecast.  databasePtr is NULL." << std::endl;
        return;
    }

    // Save the newly defined Forecast.
    if ((! Forecast.empty()) &&
        (! Scenario.empty()) &&
        (! Forecast_Tab1_NumYearsLE->text().isEmpty()))
    {
        InitYear            = Forecast_Tab1_InitialYearCMB->currentText().toInt();
        ModelPredatorGrowth = Forecast_Tab1_PredatorGrowthCB->isChecked();

        // Check if num years is out of range
        if ((NumYears <= 2) || (NumYears >20)) {
            QMessageBox::warning(Forecast_Tabs,
                    tr("New Forecast"),
                    tr("\nInvalid Forecast Duration. Value must be between 3 and 20, inclusive."),
                    QMessageBox::Ok);
            return;
        }

        // Check if Forecast already exists
//        fields   = {"MSVPAName"};
//        queryStr = "SELECT MSVPAName FROM " + nmfConstantsMSVPA::TableForecasts + " WHERE MSVPAName = '" + MSVPAName + "'" +
//                   " AND ForeName = '" + Forecast + "'";
//        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
//        NumRecords = dataMap["MSVPAName"].size();
//        if (NumRecords > 0) {
//            reply = QMessageBox::warning(Forecast_Tabs,
//                                     tr("New Forecast"),
//                                     tr("\nForecast exists. OK to overwrite?"),
//                                     QMessageBox::No|QMessageBox::Yes);
//            if (reply == QMessageBox::No) {
//                return;
//            } else {

                fields   = {"MSVPAName"};
                queryStr = "SELECT MSVPAName FROM " + nmfConstantsMSVPA::TableScenarios +
                           " WHERE MSVPAName = '" + MSVPAName + "'" +
                           " AND ForeName = '" + Forecast + "'" +
                           " AND Scenario = '" + Scenario + "'";
                dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
                NumRecords = dataMap["MSVPAName"].size();
                if (NumRecords > 0) {
                    if (QMessageBox::No == QMessageBox::warning(Forecast_Tabs,
                                             tr("New Forecast"),
                                             tr("\nScenario exists. OK to overwrite?"),
                                             QMessageBox::No|QMessageBox::Yes)) {
                        return;
                    }
                }
                // Overwrite the forecast
                saveTheForecast(MSVPAName,Forecast,Scenario,InitYear,
                                NumYears,ModelPredatorGrowth);
//            }
//        } else {
//            saveTheForecast(MSVPAName,Forecast,Scenario,InitYear,
//                            NumYears,ModelPredatorGrowth);
//        }

          MarkAsClean();

          emit ReloadForecast("Tab5");


    } else {
        QMessageBox::warning(Forecast_Tabs,
                                 tr("New Forecast"),
                                 tr("\nPlease complete all fields."),
                                 QMessageBox::Ok);
    }

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab1::callback_Forecast_Tab1_SavePB Complete");

} // end callback_Forecast_Tab1_SavePB


void
nmfForecastTab1::setupNewForecast(nmfDatabase *theDatabasePtr,
                                  std::string MSVPAName)
{
    logger->logMsg(nmfConstants::Normal,"nmfForecastTab1::setupNewForecast");

    std::string queryStr = "";
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    databasePtr = theDatabasePtr;

    // Get MSVPA info
    fields    = {"FirstYear","LastYear"};
    queryStr  = "SELECT FirstYear,LastYear FROM " + nmfConstantsMSVPA::TableMSVPAlist +
                " WHERE MSVPAName = '" + MSVPAName + "'";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    FirstYear = std::stod(dataMap["FirstYear"][0]);
    LastYear  = std::stod(dataMap["LastYear"][0]);

    //Forecast_Tab1_NextPB->setEnabled(false);
    Forecast_Tab1_ForecastNameLE->setReadOnly(false);
    Forecast_Tab1_ScenarioNameLE->setReadOnly(false);
    Forecast_Tab1_NumYearsLE->setReadOnly(false);
    Forecast_Tab1_ForecastNameLE->clear();
    Forecast_Tab1_ScenarioNameLE->clear();
    Forecast_Tab1_NumYearsLE->clear();

    for (int i=FirstYear; i<=LastYear; ++i) {
        Forecast_Tab1_InitialYearCMB->addItem(QString::number(i));
    }

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab1::setupNewForecast Complete");

}

void
nmfForecastTab1::refresh(std::string MSVPAName,
                         std::string ForecastName,
                         std::string ScenarioName)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    Forecast_Tab1_MsvpaNameLE->setText(QString::fromStdString(MSVPAName));
    Forecast_Tab1_ForecastNameLE->setText(QString::fromStdString(ForecastName));
    Forecast_Tab1_ScenarioNameLE->setText(QString::fromStdString(ScenarioName));

    fields = {"InitYear","NYears","Growth"};
    queryStr = "SELECT InitYear,NYears,Growth FROM " + nmfConstantsMSVPA::TableForecasts +
               " WHERE MSVPAName='" + MSVPAName + "'" +
               " AND ForeName = '" + ForecastName + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    Forecast_Tab1_InitialYearCMB->setCurrentText(QString::fromStdString(dataMap["InitYear"][0]));
    Forecast_Tab1_NumYearsLE->setText(QString::fromStdString(dataMap["NYears"][0]));

} // end refresh


void
nmfForecastTab1::callback_TableDataChanged()
{    
    MarkAsDirty(nmfConstantsMSVPA::TableForecasts);

} // end callback_TableDataChanged

void
nmfForecastTab1::callback_TableDataChanged(int unused)
{
    MarkAsDirty(nmfConstantsMSVPA::TableForecasts);

} // end callback_TableDataChanged


void
nmfForecastTab1::callback_Forecast_Tab1_NextPB(bool unused)
{
    Forecast_Tab1_ForecastNameLE->setReadOnly(true);
    //Forecast_Tab1_ScenarioNameLE->setReadOnly(true);
    Forecast_Tabs->setCurrentIndex(Forecast_Tabs->currentIndex()+1);
}

void
nmfForecastTab1::MarkAsDirty(std::string tableName)
{
    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfForecastTab1::MarkAsClean()
{
    emit MarkForecastAsClean();
} // end MarkAsClean


void
nmfForecastTab1::loadWidgets(nmfDatabase *theDatabasePtr,
                             std::string MSVPAName,
                             std::string ForecastName,
                             int theFirstYear, int theLastYear)
{

    int NYears;
    int InitYear;
    int index;
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab1::loadWidgets");

    databasePtr = theDatabasePtr;

    //Forecast_Tab1_ScenarioNameLE->clear();
    Forecast_Tab1_ForecastNameLE->setText(QString::fromStdString(ForecastName));
    FirstYear = theFirstYear;
    LastYear  = theLastYear;

    fields = {"InitYear","NYears","Growth"};
    queryStr = "SELECT InitYear,NYears,Growth FROM " +
                nmfConstantsMSVPA::TableForecasts +
               " WHERE MSVPAName='" + MSVPAName + "'" +
               " AND ForeName = '" + ForecastName + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    InitYear = std::stoi(dataMap["InitYear"][0]);
    NYears   = std::stoi(dataMap["NYears"][0]);

    // Populate Initial Year for the Forecast combo box and set
    Forecast_Tab1_InitialYearCMB->clear();
    for (int year=FirstYear; year<=LastYear; ++year) {
        Forecast_Tab1_InitialYearCMB->addItem(QString::number(year));
    }
    index = Forecast_Tab1_InitialYearCMB->findText(QString::number(InitYear));
    Forecast_Tab1_InitialYearCMB->setCurrentIndex(index);

    // Populate Forecast Duration
    Forecast_Tab1_NumYearsLE->setText(QString::number(NYears));
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab1::loadWidgets Complete");

} // end loadWidgets

void
nmfForecastTab1::clearWidgets()
{
    Forecast_Tab1_MsvpaNameLE->clear();
    Forecast_Tab1_ForecastNameLE->clear();
    Forecast_Tab1_ScenarioNameLE->clear();
    //Forecast_Tab1_InitialYearCMB->clear();
    Forecast_Tab1_NumYearsLE->clear();
    Forecast_Tab1_PredatorGrowthCB->setChecked(false);
}
