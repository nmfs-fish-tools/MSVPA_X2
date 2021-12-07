

#include "nmfConstants.h"
#include "nmfUtils.h"
#include "nmfUtilsQt.h"
#include "nmfEntityModel.h"
#include "nmfLogger.h"

#include "nmfForecastTab04.h"


nmfForecastTab4::nmfForecastTab4(QTabWidget  *tabs,
                                 nmfLogger   *theLogger,
                                 QListView   *theScenarioLV,
                                 std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::nmfForecastTab4");

    Forecast_Tabs = tabs;
    Forecast_ScenarioLV = theScenarioLV;
    ScenarioName.clear();
    ProjectDir = theProjectDir;
    varButtonPressed = 0;

    // Load ui as a widget from disk
    QFile file(":/forms/Forecast/Forecast_Tab04.ui");
    file.open(QFile::ReadOnly);
    Forecast_Tab4_Widget = loader.load(&file,Forecast_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    Forecast_Tabs->addTab(Forecast_Tab4_Widget, tr("4. Configure Forecast Scenarios"));

    Forecast_Tab4_DataTW        = Forecast_Tabs->findChild<QTableWidget *>("Forecast_Tab4_DataTW");
    Forecast_Tab4_PrevPB        = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_PrevPB");
    Forecast_Tab4_NextPB        = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_NextPB");
    Forecast_Tab4_Load1RB       = Forecast_Tabs->findChild<QRadioButton *>("Forecast_Tab4_Load1RB");
    Forecast_Tab4_Load2RB       = Forecast_Tabs->findChild<QRadioButton *>("Forecast_Tab4_Load2RB");
    Forecast_Tab4_Load1PB       = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_Load1PB");
    Forecast_Tab4_Load2PB       = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_Load2PB");
    Forecast_Tab4_Load3PB       = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_Load3PB");
    Forecast_Tab4_Load4PB       = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_Load4PB");
    Forecast_Tab4_Load1CB       = Forecast_Tabs->findChild<QCheckBox    *>("Forecast_Tab4_Load1CB");
    Forecast_Tab4_Load2CB       = Forecast_Tabs->findChild<QCheckBox    *>("Forecast_Tab4_Load2CB");
    Forecast_Tab4_Load3CB       = Forecast_Tabs->findChild<QCheckBox    *>("Forecast_Tab4_Load3CB");
    Forecast_Tab4_Load4CB       = Forecast_Tabs->findChild<QCheckBox    *>("Forecast_Tab4_Load4CB");
    Forecast_Tab4_SpeciesCMB    = Forecast_Tabs->findChild<QComboBox    *>("Forecast_Tab4_SpeciesCMB");
    Forecast_Tab4_SpeciesLBL    = Forecast_Tabs->findChild<QLabel       *>("Forecast_Tab4_SpeciesLBL");
    Forecast_Tab4_ScenarioLBL   = Forecast_Tabs->findChild<QLabel       *>("Forecast_Tab4_ScenarioLBL");
    Forecast_Tab4_Load1ArrowLBL = Forecast_Tabs->findChild<QLabel       *>("Forecast_Tab4_Load1ArrowLBL");
    Forecast_Tab4_Load2ArrowLBL = Forecast_Tabs->findChild<QLabel       *>("Forecast_Tab4_Load2ArrowLBL");
    Forecast_Tab4_Load3ArrowLBL = Forecast_Tabs->findChild<QLabel       *>("Forecast_Tab4_Load3ArrowLBL");
    Forecast_Tab4_Load4ArrowLBL = Forecast_Tabs->findChild<QLabel       *>("Forecast_Tab4_Load4ArrowLBL");
    Forecast_Tab4_ScenarioCMB   = Forecast_Tabs->findChild<QComboBox    *>("Forecast_Tab4_ScenarioCMB");
    Forecast_Tab4_FillSQPB      = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_FillSQPB");
    Forecast_Tab4_UnFillSQPB    = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_UnFillSQPB");
    Forecast_Tab4_AddScPB       = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_AddScPB");
    Forecast_Tab4_DelScPB       = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_DelScPB");
    Forecast_Tab4_ClearPB       = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_ClearPB");
    Forecast_Tab4_SavePB        = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_SavePB");
    //Forecast_Tab4_LoadPB      = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab4_LoadPB");



    Forecast_Tab4_SpeciesCMB->clear();
    Forecast_Tab4_Load1PB->setEnabled(false);
    Forecast_Tab4_Load2PB->setEnabled(false);
    Forecast_Tab4_Load3PB->setEnabled(false);
    Forecast_Tab4_Load4PB->setEnabled(false);

    Forecast_Tab4_PrevPB->setText("\u25C1--");
    Forecast_Tab4_NextPB->setText("--\u25B7");

    Forecast_Tab4_Load1ArrowLBL->setText("\u25b6");
    Forecast_Tab4_Load2ArrowLBL->setText("\u25b6");
    Forecast_Tab4_Load3ArrowLBL->setText("\u25b6");
    Forecast_Tab4_Load4ArrowLBL->setText("\u25b6");
    hideArrows();

    connect(Forecast_Tab4_PrevPB,  SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab4_PrevPB(bool)));
    connect(Forecast_Tab4_NextPB,  SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab4_NextPB(bool)));
    connect(Forecast_Tab4_Load1RB, SIGNAL(pressed()),
            this,                  SLOT(callback_Forecast_Tab4_Load1RB()));
    connect(Forecast_Tab4_Load2RB, SIGNAL(pressed()),
            this,                  SLOT(callback_Forecast_Tab4_Load2RB()));
    connect(Forecast_Tab4_Load1CB, SIGNAL(stateChanged(int)),
            this,                  SLOT(callback_Forecast_Tab4_Load1CB(int)));
    connect(Forecast_Tab4_Load2CB, SIGNAL(stateChanged(int)),
            this,                  SLOT(callback_Forecast_Tab4_Load2CB(int)));
    connect(Forecast_Tab4_Load3CB, SIGNAL(stateChanged(int)),
            this,                  SLOT(callback_Forecast_Tab4_Load3CB(int)));
    connect(Forecast_Tab4_Load4CB, SIGNAL(stateChanged(int)),
            this,                  SLOT(callback_Forecast_Tab4_Load4CB(int)));
    connect(Forecast_Tab4_Load1PB, SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab4_Load1PB(bool)));
    connect(Forecast_Tab4_Load2PB, SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab4_Load2PB(bool)));
    connect(Forecast_Tab4_Load3PB, SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab4_Load3PB(bool)));
    connect(Forecast_Tab4_Load4PB, SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab4_Load4PB(bool)));
    connect(Forecast_Tab4_ScenarioCMB, SIGNAL(currentIndexChanged(int)),
            this,                      SLOT(callback_Forecast_Tab4_ScenarioCMB(int)));
    connect(Forecast_Tab4_SpeciesCMB,  SIGNAL(currentIndexChanged(int)),
            this,                      SLOT(callback_Forecast_Tab4_SpeciesCMB(int)));
    connect(Forecast_Tab4_FillSQPB,    SIGNAL(clicked(bool)),
            this,                      SLOT(callback_Forecast_Tab4_FillSQPB(bool)));
    connect(Forecast_Tab4_UnFillSQPB,  SIGNAL(clicked(bool)),
            this,                      SLOT(callback_Forecast_Tab4_UnFillSQPB(bool)));
    connect(Forecast_Tab4_AddScPB,     SIGNAL(clicked(bool)),
            this,                      SLOT(callback_Forecast_Tab4_AddScPB(bool)));
    connect(Forecast_Tab4_DelScPB,     SIGNAL(clicked(bool)),
            this,                      SLOT(callback_Forecast_Tab4_DelScPB(bool)));
    connect(Forecast_Tab4_SavePB,      SIGNAL(clicked(bool)),
            this,                      SLOT(callback_Forecast_Tab4_SavePB(bool)));
    connect(Forecast_Tab4_DataTW,      SIGNAL(itemChanged(QTableWidgetItem*)),
            this,                      SLOT(callback_TableItemChanged(QTableWidgetItem*)));

    Forecast_Tab4_ClearPB->hide();

    Forecast_Tab4_SavePB->setToolTip("Save specific scenario tables based upon last data loaded");
    Forecast_Tab4_SavePB->setStatusTip("Save specific scenario tables based upon last data loaded");
    //Forecast_Tab4_LoadPB->setToolTip("Update table: ScenarioF. May also update other tables at the same time.");
    //Forecast_Tab4_LoadPB->setStatusTip("Update table: ScenarioF. May also update other tables at the same time.");
    Forecast_Tab4_SpeciesCMB->setToolTip("Changing the species will update the below table only if 1 checkbox to the left is checked.");
    Forecast_Tab4_SpeciesCMB->setStatusTip("Changing the species will update the below table only if 1 checkbox to the left is checked.");

    Forecast_Tab4_SavePB->setEnabled(false);

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::nmfForecastTab4 Complete");
}


nmfForecastTab4::~nmfForecastTab4()
{
}

/*
 * Returns part of the data used in the Tab5 text editor to inform the user of
 * Forecast settings.
 */
std::string
nmfForecastTab4::getConfigData()
{
    std::string retv = "Fishery removals entered as ";

    retv += (Forecast_Tab4_Load1RB->isChecked()) ? "Fishing Mortality Rates" :
                                                   "Catch Limits";
    retv += "<br>";
    retv += (Forecast_Tab4_Load1CB->isChecked()) ? "Variable Fishery Removals" :
                                                   "Status Quo Fishery Removals";
    retv += "<br>";
    retv += (Forecast_Tab4_Load2CB->isChecked()) ? "Variable Other Predator Biomass" :
                                                   "Status Quo Other Predator Biomass";
    retv += "<br>";
    retv += (Forecast_Tab4_Load3CB->isChecked()) ? "Variable Other Prey Biomass" :
                                                   "Status Quo Other Prey Biomass";
    retv += "<br>";
    retv += (Forecast_Tab4_Load4CB->isChecked()) ? "Variable Recruitment" :
                                                   "Status Quo Recruitment";
    retv += "<br>";

    return retv;

} // end getConfigData

void
nmfForecastTab4::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir


void nmfForecastTab4::hideArrows()
{
    Forecast_Tab4_Load1ArrowLBL->hide();
    Forecast_Tab4_Load2ArrowLBL->hide();
    Forecast_Tab4_Load3ArrowLBL->hide();
    Forecast_Tab4_Load4ArrowLBL->hide();
} // end hideArrows

void
nmfForecastTab4::refresh()
{
    Forecast_Tab4_Load1CB->setChecked(false);
    Forecast_Tab4_Load2CB->setChecked(false);
    Forecast_Tab4_Load3CB->setChecked(false);
    Forecast_Tab4_Load4CB->setChecked(false);

} // end refresh


void
nmfForecastTab4::MarkAsDirty(std::string tableName)
{
    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfForecastTab4::MarkAsClean()
{
    emit MarkForecastAsClean();
} // end MarkAsClean

void
nmfForecastTab4::updateMainWindowScenarioList()
{
    // Get the scenario model
    nmfEntityModel *scenario_model;
    scenario_model = qobject_cast<nmfEntityModel *>(Forecast_ScenarioLV->model());

    // Re-load Scenario list into model and scroll list view to top.
    scenario_model->removeRows(0, scenario_model->count(), QModelIndex());
    for (Scenarios aScenario : ScenarioList) {
        scenario_model->append( QString::fromStdString(aScenario.Name));
    }

    Forecast_ScenarioLV->setModel(scenario_model);
    Forecast_ScenarioLV->scrollToTop();
} // end updateMainScenarioListView


void
nmfForecastTab4::callback_TableItemChanged(QTableWidgetItem *item)
{
//std::cout << "Changed to: " << item->text().toStdString() << std::endl;
    int row = item->row();
    int col = item->column();
    int SpeciesIndex = Forecast_Tab4_SpeciesCMB->currentIndex();


    switch (varButtonPressed) {
        case 1:
            TableData[SpeciesIndex][row][col] = item->text().toDouble();
            MarkAsDirty(nmfConstantsMSVPA::TableScenarioF);
            break;
        case 2:
            TableData[SpeciesIndex][row][col] = item->text().toDouble();
            MarkAsDirty(nmfConstantsMSVPA::TableScenarioOthPred);
            break;
        case 3:
            TableData[SpeciesIndex][row][col] = item->text().toDouble();
            MarkAsDirty(nmfConstantsMSVPA::TableScenarioOthPrey);
            break;
        case 4:
            TableData[0][row-1][col] = item->text().toDouble();
            MarkAsDirty(nmfConstantsMSVPA::TableScenarioRec);
            break;
        default:
            break;
    }

    Forecast_Tab4_SavePB->setEnabled(true);

} // end callback_TableItemChanged


void
nmfForecastTab4::createScenario(std::string newScenarioName)
{
    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::createScenario");

    std::string cmd;
    std::string errorMsg;

    Scenarios *aScenario;

    aScenario = new Scenarios();
    aScenario->FishAsF    = 1;
    aScenario->VarF       = 0;
    aScenario->VarOthPred = 0;
    aScenario->VarOthPrey = 0;
    aScenario->VarRec     = 0;
    aScenario->Name       = newScenarioName;
    ScenarioList.push_back(*aScenario);
    Forecast_Tab4_ScenarioCMB->addItem(QString::fromStdString(newScenarioName));
    Forecast_Tab4_Load1CB->setChecked(aScenario->VarF);
    Forecast_Tab4_Load2CB->setChecked(aScenario->VarOthPred);
    Forecast_Tab4_Load3CB->setChecked(aScenario->VarOthPrey);
    Forecast_Tab4_Load4CB->setChecked(aScenario->VarRec);
    Forecast_Tab4_Load1RB->setChecked(aScenario->FishAsF);
    Forecast_Tab4_Load2RB->setChecked(! aScenario->FishAsF);

    // Update the database table with the new scenario
    cmd  = "INSERT INTO " + nmfConstantsMSVPA::TableScenarios +
           " (MSVPAName,ForeName,Scenario,VarF,VarOthPred,VarOthPrey,VarRec,FishAsF) values ('" +
           MSVPAName + "', '" + ForecastName + "', '" + newScenarioName + "', 0, 0, 0, 0, 1)";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if ( nmfUtilsQt::isAnError(errorMsg) ) nmfUtils::printError("Error createScenario",errorMsg);

    updateMainWindowScenarioList();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::createScenario Complete");

} // end createNewScenario

void
nmfForecastTab4::deleteScenario(std::string scenarioToDelete)
{
    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::deleteScenario");

    std::string cmd;
    std::string errorMsg;

    std::vector<Scenarios>::iterator it = ScenarioList.begin();
    while (it != ScenarioList.end()) {
        if ((*it).Name == scenarioToDelete) {
            it = ScenarioList.erase(it);
        } else {
            ++it;
        }
    }

    // Reload combobox
    Forecast_Tab4_ScenarioCMB->clear();
    for (Scenarios aScenario : ScenarioList) {
        Forecast_Tab4_ScenarioCMB->addItem(QString::fromStdString(aScenario.Name));
    }

    // Update the database with the deleted scenario
    cmd  = "DELETE FROM " + nmfConstantsMSVPA::TableScenarios +
           " WHERE MSVPAName='" + MSVPAName +
           "' AND ForeName='" + ForecastName +
           "' AND Scenario='" + scenarioToDelete + "'";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if ( nmfUtilsQt::isAnError(errorMsg) ) nmfUtils::printError("Error createScenario",errorMsg);

    updateMainWindowScenarioList();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::deleteScenario Complete");

} // end deleteScenario


void
nmfForecastTab4::Save_Scenarios()
{
    std::string cmd;
    std::string errorMsg;
    std::string ScenarioName = Forecast_Tab4_ScenarioCMB->currentText().toStdString();

    // Update Scenarios data
    cmd  = "INSERT INTO " + nmfConstantsMSVPA::TableScenarios;
    cmd += " (MSVPAName,ForeName,Scenario,VarF,VarOthPred,VarOthPrey,VarRec,FishAsF) values ";
    cmd += "(\"" + MSVPAName + "\"," +
            "\"" + ForecastName + "\"," +
            "\"" + ScenarioName + "\"," +
             std::to_string((int)Forecast_Tab4_Load1CB->isChecked()) + "," +
             std::to_string((int)Forecast_Tab4_Load2CB->isChecked()) + "," +
             std::to_string((int)Forecast_Tab4_Load3CB->isChecked()) + "," +
             std::to_string((int)Forecast_Tab4_Load4CB->isChecked()) + "," +
             std::to_string((int)Forecast_Tab4_Load1RB->isChecked()) + ") ";
    cmd += "ON DUPLICATE KEY UPDATE ";
    cmd += "VarF=values(VarF),VarOthPred=values(VarOthPred),VarOthPrey=values(VarOthPrey),";
    cmd += "VarRec=values(VarRec),FishAsF=values(FishAsF); ";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if ( nmfUtilsQt::isAnError(errorMsg) ) {
        std::cout << cmd << std::endl;
        nmfUtils::printError("Error: Forecast Tab4 Save_Scenarios: ",errorMsg);
    }
} // end Save_Scenarios

void
nmfForecastTab4::Save_ScenarioF()
{
    std::string cmd;
    std::string errorMsg;
    std::string ScenarioName = Forecast_Tab4_ScenarioCMB->currentText().toStdString();
    //std::string SpeciesName  = Forecast_Tab4_SpeciesCMB->currentText().toStdString();
    //int SpeciesIndex         = Forecast_Tab4_SpeciesCMB->currentIndex();
    std::string varType      = Forecast_Tab4_Load1RB->isChecked() ? "F" : "C";
    double val;
    int NumRecords;
    std::string SpeName;
    std::vector<int> NPredAge;
    std::map<std::string, std::vector<std::string> > dataMap,dataMap2;
    std::vector<std::string> fields,fields2;
    std::string queryStr,queryStr2;


    cmd = "DELETE FROM " + nmfConstantsMSVPA::TableScenarioF +
          " WHERE MSVPAName = '" + MSVPAName + "' " +
          "AND ForeName = '" + ForecastName + "' " +
          "AND Scenario = '" + ScenarioName + "' ";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if ( nmfUtilsQt::isAnError(errorMsg) ) {
        std::cout << cmd << std::endl;
        nmfUtils::printError("Error: Forecast Tab4 Save_ScenarioF Delete Failed: ",errorMsg);
        return;
    }

    // Load species combo box
    NPredAge.clear();
    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND (Type = 0 or Type = 1)";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpeName"].size();
    std::string species;
    for (int i=0; i<NumRecords; ++i)
    {
        species = dataMap["SpeName"][i];
        //PredList.push_back(species);

        fields2   = {"MaxAge"};
        queryStr2 = "SELECT MaxAge FROM " + nmfConstantsMSVPA::TableSpecies +
                    " WHERE SpeName = '" + species + "'";
        dataMap2  = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        NPredAge.push_back(std::stoi(dataMap2["MaxAge"][0]));
    }

    // Update ScenarioF data
    int year;
    int nrows = Forecast_Tab4_DataTW->rowCount();
    //int ncols = Forecast_Tab4_DataTW->columnCount();

    cmd.clear();
    for (int SpeIndex=0; SpeIndex<Forecast_Tab4_SpeciesCMB->count(); ++SpeIndex)  {
        SpeName = Forecast_Tab4_SpeciesCMB->itemText(SpeIndex).toStdString();
        for (int age=0;age<=NPredAge[SpeIndex];++age) {
            for (int i=0; i<nrows;++i) {
                year = Forecast_Tab4_DataTW->verticalHeaderItem(i)->text().toInt() - ForecastFirstYear;
                val  = TableData[SpeIndex][year][age];
                cmd += "INSERT INTO " + nmfConstantsMSVPA::TableScenarioF;
                cmd += " (MSVPAName,ForeName,Scenario,SpeName,SpeIndex,VarType,Age,Year,F) values ";
                cmd += "(\"" + MSVPAName + "\", " +
                        "\"" + ForecastName + "\", " +
                        "\"" + ScenarioName + "\", " +
                        "\"" + SpeName + "\", " +
                        std::to_string(SpeIndex) + ", " +
                        "\"" + varType + "\", " +
                        std::to_string(age) + ", " +
                        std::to_string(year) + ", " +
                        std::to_string(val) + "); ";
            } // end for row
        } // end for age
    } // end for SpeIndex

    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if ( nmfUtilsQt::isAnError(errorMsg) ) {
        std::cout << cmd << std::endl;
        nmfUtils::printError("Error: Forecast Tab4 Save ScenarioF: ",errorMsg);
    }

} // end Save_ScenarioF


void
nmfForecastTab4::Save_ScenarioOthPred()
{
    int year;
    std::string cmd;
    std::string errorMsg;
    std::string ScenarioName = Forecast_Tab4_ScenarioCMB->currentText().toStdString();
    std::string SpeciesName  = Forecast_Tab4_SpeciesCMB->currentText().toStdString();
    int SpeciesIndex         = Forecast_Tab4_SpeciesCMB->currentIndex();
    int NumRows              = Forecast_Tab4_DataTW->rowCount();
    int NumCols              = Forecast_Tab4_DataTW->columnCount();
    double val1,val2;

    cmd = "DELETE FROM " + nmfConstantsMSVPA::TableScenarioOthPred +
          " WHERE MSVPAName = '" + MSVPAName + "' " +
          "AND ForeName = '" + ForecastName + "' " +
          "AND Scenario = '" + ScenarioName + "' ";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        std::cout << cmd << std::endl;
        nmfUtils::printError("Error: Forecast Tab4 Save_ScenarioOthPred Delete Failed: ",errorMsg);
        return;
    }

    // Build the command
    cmd.clear();
    for (int size = 1; size < NumCols; ++ size) {
        for (int row = 0; row < NumRows; ++row) {
            year = Forecast_Tab4_DataTW->verticalHeaderItem(row)->text().toInt() - ForecastFirstYear;
            val1 = TableData[SpeciesIndex][year][size];
            val2 = TableData[SpeciesIndex][year][0];
            cmd += "INSERT INTO " + nmfConstantsMSVPA::TableScenarioOthPred;
            cmd += " (MSVPAName,ForeName,Scenario,SpeName,SpeIndex,SizeClass,Year,Biomass,PropBM) values ";
            cmd += "(\"" + MSVPAName + "\"," + "\"" + ForecastName + "\"," + "\"" + ScenarioName + "\"," +
                    "\"" + SpeciesName + "\"," + std::to_string(SpeciesIndex) + "," +
                    std::to_string(size) + "," + std::to_string(year) + "," +
                    std::to_string(val1 * val2) + ", " +
                    std::to_string(val1) + "); ";
        } // end for row
    } // end for size

    // Execute the command and update the ScenarioOthPred table
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if ( nmfUtilsQt::isAnError(errorMsg) ) {
        std::cout << cmd << std::endl;
        nmfUtils::printError("Error: Forecast Tab4 Save_ScenarioOthPred: ",errorMsg);
    }


} // end Save_ScenarioOthPred


void
nmfForecastTab4::Save_ScenarioOthPrey()
{
    int year;
    std::string cmd;
    std::string errorMsg;
    std::string preyName;
    std::string ScenarioName = Forecast_Tab4_ScenarioCMB->currentText().toStdString();
    //std::string SpeciesName  = Forecast_Tab4_SpeciesCMB->currentText().toStdString();
    //int SpeciesIndex         = Forecast_Tab4_SpeciesCMB->currentIndex();
    int NumRows              = Forecast_Tab4_DataTW->rowCount();
    int NumCols              = Forecast_Tab4_DataTW->columnCount();
    double val;

    cmd = "DELETE FROM " + nmfConstantsMSVPA::TableScenarioOthPrey +
          " WHERE MSVPAName = '" + MSVPAName + "' " +
          "AND ForeName = '" + ForecastName + "' " +
          "AND Scenario = '" + ScenarioName + "' ";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if ( nmfUtilsQt::isAnError(errorMsg) ) {
        std::cout << cmd << std::endl;
        nmfUtils::printError("Error: Forecast Tab4 Save_ScenarioOthPrey Delete Failed: ",errorMsg);
        return;
    }

    // Update ScenarioOthPred table
    cmd.clear();
    int NumPrey = Forecast_Tab4_SpeciesCMB->count();
    for (int prey=0; prey<NumPrey; ++prey) {
        preyName = Forecast_Tab4_SpeciesCMB->itemText(prey).toStdString();
        for (int season = 0; season < NumCols; ++ season) { // Number of seasons
            for (int row = 0; row < NumRows; ++row) { // Number of years
                year = Forecast_Tab4_DataTW->verticalHeaderItem(row)->text().toInt() - ForecastFirstYear;
                val = TableData[prey][year][season];
                cmd += "INSERT INTO " + nmfConstantsMSVPA::TableScenarioOthPrey;
                cmd += " (MSVPAName,ForeName,Scenario,SpeName,SpeIndex,Season,Year,Biomass) values ";
                cmd += "(\"" + MSVPAName + "\"," + "\"" + ForecastName + "\"," + "\"" + ScenarioName + "\"," +
                        "\"" + preyName + "\"," + std::to_string(prey) + "," +
                        std::to_string(season) + "," + std::to_string(year) + "," +
                        std::to_string(val) + "); ";
            } // end for row
        } // end for season
    } // end for prey

    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if ( nmfUtilsQt::isAnError(errorMsg) ) {
        std::cout << cmd << std::endl;
        nmfUtils::printError("Error: Forecast Tab4 Save_ScenarioOthPrey: ",errorMsg);
    }

} // end Save_ScenarioOthPrey

void
nmfForecastTab4::Save_ScenarioRec()
{
    int year;
    std::string cmd;
    std::string errorMsg;
    std::string SpeName;
    std::string ScenarioName = Forecast_Tab4_ScenarioCMB->currentText().toStdString();
    int NumRows              = Forecast_Tab4_DataTW->rowCount();
    int NumCols              = Forecast_Tab4_DataTW->columnCount();
    double valRecAdjust;
    int valAbsRecruits;
    int SpeIndex;
    QCheckBox *absRecruitsCB;
    QWidget *w;

    cmd = "DELETE FROM " + nmfConstantsMSVPA::TableScenarioRec +
          " WHERE MSVPAName = '" + MSVPAName + "' " +
          "AND ForeName = '" + ForecastName + "' " +
          "AND Scenario = '" + ScenarioName + "' ";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if ( nmfUtilsQt::isAnError(errorMsg) ) {
        std::cout << cmd << std::endl;
        nmfUtils::printError("Error: Forecast Tab4 Save_ScenarioRec Delete Failed: ",errorMsg);
        return;
    }

    // Update ScenarioRec table
    cmd.clear();
    for (int col = 0; col < NumCols; ++col) { // Number of Species
        for (int row = 1; row < NumRows; ++row) { // Number of years+1
            year = Forecast_Tab4_DataTW->verticalHeaderItem(row)->text().toInt() - ForecastFirstYear;
            SpeName = Forecast_Tab4_DataTW->horizontalHeaderItem(col)->text().toStdString();
            SpeIndex = col; // RSK - not sure if this is correct....test!
            w  = Forecast_Tab4_DataTW->cellWidget(0, col);
            absRecruitsCB = w->findChild<QCheckBox *>();
            valAbsRecruits = absRecruitsCB->isChecked();
            valRecAdjust   = TableData[0][row-1][SpeIndex];
            cmd += "INSERT INTO " + nmfConstantsMSVPA::TableScenarioRec;
            cmd += " (MSVPAName,ForeName,Scenario,SpeName,SpeIndex,Year, RecAdjust, AbsRecruits) values ";
            cmd += "(\"" + MSVPAName + "\"," + "\"" + ForecastName + "\"," + "\"" + ScenarioName + "\"," +
                    "\"" + SpeName + "\"," + std::to_string(SpeIndex) + "," + std::to_string(year) + "," +
                    std::to_string(valRecAdjust)   + ", "  + // RecAdjust
                    std::to_string(valAbsRecruits) + "); "; // AbsRecruits
        } // end for row
    } // end for col

    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if ( nmfUtilsQt::isAnError(errorMsg) ) {
        std::cout << cmd << std::endl;
        nmfUtils::printError("Error: Forecast Tab4 Save_ScenarioOthPrey: ",errorMsg);
    }

} // end Save_ScenarioRec


void
nmfForecastTab4::callback_Forecast_Tab4_SavePB(bool unused)
{
    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_SavePB");

    bool skipRow1 = (varButtonPressed == 4);
    std::string msg;
    std::string tablesSaved = nmfConstantsMSVPA::TableScenarios;

    Save_Scenarios();
    if (! nmfUtilsQt::allCellsArePopulated(Forecast_Tabs,Forecast_Tab4_DataTW,false,skipRow1)) {
        QMessageBox::warning(Forecast_Tabs,
                             tr("Warning"),
                             tr("\nPlease complete missing table fields."),
                             QMessageBox::Ok);
        return;
    }
    switch (varButtonPressed) {
        case 1:
            Save_ScenarioF();
            tablesSaved += ", " + nmfConstantsMSVPA::TableScenarioF;
            break;
        case 2:
            Save_ScenarioOthPred();
            tablesSaved += ", " + nmfConstantsMSVPA::TableScenarioOthPred;
            break;
        case 3:
            Save_ScenarioOthPrey();
            tablesSaved += ", " + nmfConstantsMSVPA::TableScenarioOthPrey;
            break;
        case 4:
            Save_ScenarioRec();
            tablesSaved += ", " + nmfConstantsMSVPA::TableScenarioRec;
            break;
        default:
            break;
    }

    // Reload Forecast Tab4
    //emit ReloadForecast("Tab4");

    msg = "\nTables saved: " + tablesSaved;
    QMessageBox::information(Forecast_Tabs,
                         tr("Save"),
                         tr(msg.c_str()),
                         QMessageBox::Ok);

    Forecast_Tab4_SavePB->setEnabled(false);
    MarkAsClean();

    emit ReloadForecast("Tab5");

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_SavePB Complete");

} // end callback_Forecast_Tab4_SavePB


//void
//nmfForecastTab4::callback_Forecast_Tab4_LoadPB()
//{
//    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_LoadPB");
//    emit LoadDataTable(MSVPAName,ForecastName,
//                       "Forecast","ScenarioF",
//                       "Forecast",3);
//    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_LoadPB Complete");
//} // end callback_Forecast_Tab4_LoadPB

void
nmfForecastTab4::callback_Forecast_Tab4_AddScPB(bool unused)
{
    bool ok;
    bool done = false;
    bool scenarioExists;

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_AddScPB");

    while (! done) {
        scenarioExists = false;
        QString NewScenarioName = QInputDialog::getText(0, "New Scenario",
              "Enter new scenario name:", QLineEdit::Normal,"", &ok);
        NewScenarioName = NewScenarioName.trimmed();
        if (ok && ! NewScenarioName.isEmpty()) {
            // Check to see if name already exists...
            for (Scenarios aScenario : ScenarioList) {
                if (aScenario.Name == NewScenarioName.toStdString()) {
                    scenarioExists = true;
                    break;
                }
            }
            if (! scenarioExists) {
                createScenario(NewScenarioName.toStdString());
                done = true;
            } else {
                std::cout << "Warning: Scenario already exists. Please enter a different name." << std::endl;
            }
        } else {
            if (NewScenarioName.isEmpty()) {
                std::cout << "Error: No scenario name entered." << std::endl;
            }
            if (! ok) {
                done = true;
            }
        }
    } // end while

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_AddScPB Complete");

} // end callback_Forecast_Tab4_AddScPB


void
nmfForecastTab4::callback_Forecast_Tab4_DelScPB(bool unused)
{
    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_DelScPB");

    std::string currentScenario = Forecast_Tab4_ScenarioCMB->currentText().toStdString();
    QMessageBox::StandardButton reply;
    std::string msg = "\nOK to delete scenario: " + currentScenario;
    reply = QMessageBox::question(0, "Delete Scenario",
        msg.c_str(), QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        deleteScenario(currentScenario);
    }

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_DelScPB Complete");

} // end callback_Forecast_Tab4_DelScPB


void
nmfForecastTab4::callback_Forecast_Tab4_ScenarioCMB(int index)
{
    Forecast_Tab4_DataTW->clear();
    Forecast_Tab4_Load1CB->setChecked(ScenarioList[index].VarF);
    Forecast_Tab4_Load2CB->setChecked(ScenarioList[index].VarOthPred);
    Forecast_Tab4_Load3CB->setChecked(ScenarioList[index].VarOthPrey);
    Forecast_Tab4_Load4CB->setChecked(ScenarioList[index].VarRec);
    Forecast_Tab4_Load1RB->setChecked(ScenarioList[index].FishAsF);
    Forecast_Tab4_Load2RB->setChecked(! ScenarioList[index].FishAsF);
}


void
nmfForecastTab4::updateTable(int rowOffset, std::vector<std::string> &headerList)
{
    // Load the table with the data
    int ForecastYear;
    QString colHeader;
    QTableWidgetItem *item;
    int NCols = headerList.size();
    int index=0;

    disconnect(Forecast_Tab4_DataTW,   SIGNAL(itemChanged(QTableWidgetItem*)),
               this,                   SLOT(callback_TableItemChanged(QTableWidgetItem*)));

    if (rowOffset == 1) {
        // A rowOffset of 1 means that the first row is checkboxes
        // First row is row of checkboxes
        Forecast_Tab4_DataTW->setRowCount(ForecastNYears+1);
        Forecast_Tab4_DataTW->setColumnCount(NCols);
        for (int j=0; j<NCols; ++j) {
            QWidget *w = new QWidget();
            QCheckBox *cb = new QCheckBox();
            cb->setObjectName(QString::fromStdString(PredList[j]));
            QHBoxLayout *l = new QHBoxLayout();
            l->setAlignment(Qt::AlignCenter);
            l->addWidget(cb);
            w->setLayout(l);
            Forecast_Tab4_DataTW->setCellWidget(0,j,w);
            connect(cb,   SIGNAL(stateChanged(int)),
                    this, SLOT(callback_Forecast_Tab4_RecCBs(int)));
        }
        Forecast_Tab4_DataTW->resizeRowsToContents();
        Forecast_Tab4_DataTW->setVerticalHeaderItem(0,new QTableWidgetItem("Abs. Recruits"));
    } else {
        index = Forecast_Tab4_SpeciesCMB->currentIndex();
        Forecast_Tab4_DataTW->clear();
        Forecast_Tab4_DataTW->setColumnCount(NCols); // +1 because the range is [0,the number] inclusive
        Forecast_Tab4_DataTW->setRowCount(ForecastNYears);
    }

    for (int i=0; i<ForecastNYears; ++i) {
        ForecastYear = ForecastFirstYear + i;
        Forecast_Tab4_DataTW->setVerticalHeaderItem(i+rowOffset,new QTableWidgetItem(QString::number(ForecastYear)));
        for (int j=0; j<NCols; ++j) {
            if (i == 0) {
                colHeader = QString::fromStdString(headerList[j]);
                Forecast_Tab4_DataTW->setHorizontalHeaderItem(j,new QTableWidgetItem(colHeader));
            }
            item = new QTableWidgetItem(QString::number(TableData[index][i][j]));
            item->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
            if (i == 0) { // Make cell read-only for the first row
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            }
            Forecast_Tab4_DataTW->setItem(i+rowOffset,j,item);
        }
    }

    connect(Forecast_Tab4_DataTW,      SIGNAL(itemChanged(QTableWidgetItem*)),
            this,                      SLOT(callback_TableItemChanged(QTableWidgetItem*)));

} // end updateTable


void
nmfForecastTab4::callback_Forecast_Tab4_SpeciesCMB(int index)
{
    int NPreds = Forecast_Tab4_SpeciesCMB->count();
    if ((NPreds == 0) || (NPredAge.size() == 0))
        return;

    Forecast_Tab4_SavePB->setEnabled(true);

    std::vector<std::string> headerList;

    if ((Forecast_Tab4_Load1CB->checkState() == Qt::Checked) && (varButtonPressed == 1)) {
        if (Forecast_Tab4_Load1CB->isChecked()) {
            for (int i=0; i<=NPredAge[index]; ++i) {
                headerList.push_back("Age "+std::to_string(i));
            }
        }
        updateTable(0,headerList);

    } else if ((Forecast_Tab4_Load2CB->checkState() == Qt::Checked) && (varButtonPressed == 2)) {
        int index = Forecast_Tab4_SpeciesCMB->currentIndex();
        headerList.push_back("Biomass");
        for (int i=1; i<=NPredAge[index]; ++i) {
            headerList.push_back("Size "+std::to_string(i));
        }
        updateTable(0,headerList);

    } else if ((Forecast_Tab4_Load3CB->checkState() == Qt::Checked) && (varButtonPressed == 3)) {
        for (int i=0; i<NSeasons; ++i) {
            headerList.push_back("Season "+std::to_string(i+1));
        }
        updateTable(0,headerList);

    }


/*
    // Let species combobox automatically refresh table only if one checkbox is checked.
    if ((Forecast_Tab4_Load1CB->checkState() == Qt::Checked) ||
        (Forecast_Tab4_Load2CB->checkState() == Qt::Checked) ||
        (Forecast_Tab4_Load3CB->checkState() == Qt::Checked) ||
        (Forecast_Tab4_Load4CB->checkState() == Qt::Checked))
    {
        std::vector<std::string> headerList;

        if (Forecast_Tab4_Load1CB->isChecked()) {
            for (int i=0; i<=NPredAge[index]; ++i) {
                headerList.push_back("Age "+std::to_string(i));
            }
        } else  if (Forecast_Tab4_Load2CB->isChecked()) {
            int index = Forecast_Tab4_SpeciesCMB->currentIndex();
            headerList.push_back("Biomass");
            for (int i=1; i<=NPredAge[index]; ++i) {
                headerList.push_back("Size "+std::to_string(i));
            }
        } else  if (Forecast_Tab4_Load3CB->isChecked()) {
            for (int i=0; i<NSeasons; ++i) {
                headerList.push_back("Season "+std::to_string(i+1));
            }
        } else  if (Forecast_Tab4_Load4CB->isChecked()) {
            // Nothing to do here
        }
        std::cout << 111.2 << std::endl;
for (std::string item : headerList)
    std::cout << "item: " << item << std::endl;

        updateTable(0,headerList);
        std::cout << 111.4 << std::endl;

    }
std::cout << 222 << std::endl;
*/
} // end callback_Forecast_Tab4_SpeciesCMB


void
nmfForecastTab4::callback_Forecast_Tab4_Load1PB(bool unused)
{
    std::map<std::string, std::vector<std::string> > dataMap,dataMap2;
    std::vector<std::string> fields,fields2;
    std::string queryStr,queryStr2;
    int m;
    int NPreds;
    int NumRecords;
    std::string ScenarioName = Forecast_Tab4_ScenarioCMB->currentText().toStdString();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_Load1PB");

    varButtonPressed = 1;
    Forecast_Tab4_SavePB->setEnabled(true);
    hideArrows();
    Forecast_Tab4_Load1ArrowLBL->show();

    Forecast_Tab4_SavePB->setToolTip("Save tables: Scenarios and ScenarioF");
    Forecast_Tab4_SavePB->setStatusTip("Save tables: Scenarios and ScenarioF");

    Forecast_Tab4_SpeciesLBL->show();
    Forecast_Tab4_SpeciesCMB->show();
    Forecast_Tab4_SpeciesCMB->clear();
    PredList.clear();
    NPredAge.clear();
    nmfUtils::initialize(TableData);
    bool FishAsF = Forecast_Tab4_Load1RB->isChecked();

    Forecast_Tab4_DataTW->clear();

    // Load species combo box
    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND (Type = 0 or Type = 1)";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpeName"].size();
    std::string species;
    for (int i=0; i<NumRecords; ++i)
    {
        species = dataMap["SpeName"][i];
        PredList.push_back(species);
        Forecast_Tab4_SpeciesCMB->addItem(QString::fromStdString(species));

        fields2   = {"MaxAge"};
        queryStr2 = "SELECT MaxAge FROM " + nmfConstantsMSVPA::TableSpecies +
                    " WHERE SpeName = '" + species + "'";
        dataMap2  = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        NPredAge.push_back(std::stoi(dataMap2["MaxAge"][0]));

    }
    Forecast_Tab4_SpeciesCMB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    NPreds = NumRecords;

    if (FishAsF) {
        for (int i = 0; i <= NPreds-1; ++i) {
            m = 0;
            fields = {"SpeName","Age","F"};
            queryStr = "SELECT SpeName, Age, Sum(SeasF) as F FROM " +
                        nmfConstantsMSVPA::TableMSVPASeasBiomass +
                       " WHERE MSVPAName = '" + MSVPAName + "'" +
                       " AND SpeName = '" + PredList[i] + "'" +
                       " AND Year = " + std::to_string(ForecastFirstYear-MSVPAFirstYear) +
                    " GROUP BY SpeName, Age";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            NumRecords = dataMap["F"].size();
            // Load the first year worth of F or Catch data.
            for (int j = 0; j <= NPredAge[i]; ++j) {
                if (m < NumRecords) {
                    TableData[i][0][j] = std::stod(dataMap["F"][m++]);
                }
            }

       }
    } else {
        for (int i = 0; i <= NPreds-1; ++i) {
            for (int j = 0; j <= NPredAge[i]; ++j) {
                fields = {"SpeName","Age","Catch"};
                queryStr = "SELECT SpeName, Age, Catch FROM " + nmfConstantsMSVPA::TableSpeCatch +
                           " WHERE SpeName = '" + PredList[i] + "'" +
                           " AND Age = " + std::to_string(j) +
                           " AND Year = " + std::to_string(ForecastFirstYear) +
                           " ORDER BY SpeName, Age";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                NumRecords = dataMap["SpeName"].size();
                TableData[i][0][j] = (NumRecords > 0) ? std::stod(dataMap["Catch"][0]) : 0;
            } // end if
        }

    } // end if

    if (FishAsF) {

            fields = {"F"};
            queryStr = "SELECT F FROM " + nmfConstantsMSVPA::TableScenarioF +
                       " WHERE MSVPAname = '" + MSVPAName + "'" +
                       " AND ForeName = '" + ForecastName + "'" +
                       " AND Scenario = '" + ScenarioName + "'" +
                       " AND VarType = 'F' ORDER By SpeIndex, Age, Year";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            NumRecords = dataMap["F"].size();
            if (NumRecords > 0) {
                m = 0;
                for (int i = 0; i <= NPreds - 1; ++i) {
                    for (int j = 0; j <= NPredAge[i]; ++j) {
                        for (int k = 0; k < ForecastNYears; ++k) {
                            TableData[i][k][j] = std::stod(dataMap["F"][m++]);
                        }
                    }
                } // end for i
            } // end if

    } else {

        fields = {"F"};
        queryStr = "SELECT F FROM " + nmfConstantsMSVPA::TableScenarioF +
                   " WHERE MSVPAname = '" + MSVPAName + "'" +
                   " AND ForeName = '" + ForecastName + "'" +
                   " AND Scenario = '" + ScenarioName + "'" +
                   " AND VarType = 'C' ORDER By SpeIndex, Age, Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        NumRecords = dataMap["F"].size();
        if (NumRecords > 0) {
            m = 0;
            for (int i = 0; i <= NPreds - 1; ++i) {
                for (int j = 0; j <= NPredAge[i]; ++j) {
                    for (int k = 0; k < ForecastNYears; ++k) {
                        TableData[i][k][j] = std::stod(dataMap["F"][m++]);
                    }
                }
            }
        } // end if

    } // end if

    int index = Forecast_Tab4_SpeciesCMB->currentIndex();
    std::vector<std::string> headerList;
    for (int i=0; i<=NPredAge[index]; ++i) {
        headerList.push_back("Age "+std::to_string(i));
    }
    updateTable(0,headerList);

    // Load the appropriate table with its data
    //callback_Forecast_Tab4_SpeciesCMB(0);
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_Load1PB Complete");

} // end callback_Forecast_Tab4_Load1PB


void
nmfForecastTab4::callback_Forecast_Tab4_Load2PB(bool unused)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    int m;
    int NPreds;
    int NPredAgeVal;
    std::string species;
    std::string ScenarioName = Forecast_Tab4_ScenarioCMB->currentText().toStdString();
    double val;

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_Load2PB");
    Forecast_Tab4_SavePB->setEnabled(true);
    hideArrows();
    Forecast_Tab4_Load2ArrowLBL->show();

    Forecast_Tab4_SavePB->setToolTip("Save tables: Scenarios and ScenarioOthPred");
    Forecast_Tab4_SavePB->setStatusTip("Save tables: Scenarios and ScenarioOthPred");
    varButtonPressed = 2;

    PredList.clear();
    NPredAge.clear();
    Forecast_Tab4_SpeciesLBL->show();
    Forecast_Tab4_SpeciesCMB->show();
    Forecast_Tab4_SpeciesCMB->clear();
    nmfUtils::initialize(TableData);

    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Type = 3";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    int NumRecords = dataMap["SpeName"].size();
    NPreds = NumRecords;
    for (int i=0; i<NumRecords; ++i)
    {
        species = dataMap["SpeName"][i];
        PredList.push_back(species);
        Forecast_Tab4_SpeciesCMB->addItem(QString::fromStdString(species));
    }
    Forecast_Tab4_SpeciesCMB->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    // Load the data for the table
    for (int i = 0; i < NPreds; ++i) {

        fields = {"Biomass"};
        queryStr = "SELECT Biomass FROM " + nmfConstantsMSVPA::TableOtherPredBM +
                   " WHERE SpeName = '" + PredList[i] + "'" +
                   " AND Year = " + std::to_string(ForecastFirstYear);
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        TableData[i][0][0] = std::stod(dataMap["Biomass"][0]);

        fields = {"SizeStruc","NumSizeCats"};
        queryStr = "SELECT SizeStruc,NumSizeCats FROM " +
                    nmfConstantsMSVPA::TableOtherPredSpecies +
                   " WHERE SpeName = '" + PredList[i] + "'";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        NPredAgeVal = (dataMap["SizeStruc"][0] == "1") ? std::stoi(dataMap["NumSizeCats"][0]) : 1;
        NPredAge.push_back(NPredAgeVal);

        if (NPredAgeVal > 1) {
            fields = {"PropBM"};
            queryStr = "SELECT PropBM FROM " + nmfConstantsMSVPA::TableOthPredSizeData +
                       " WHERE SpeName = '" + PredList[i] + "'";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            for (int j = 0; j < NPredAge[i]; ++j) {
                TableData[i][0][j+1] = std::stod(dataMap["PropBM"][j]);
            }
        } // end if

    } // end for i

    // initialize data - load data if it exists
    for (int i = 0; i < NPreds; ++i) {
        fields = {"Year","AnnTot"};
        queryStr = "SELECT Year, Sum(Biomass) as AnnTot FROM " +
                    nmfConstantsMSVPA::TableScenarioOthPred +
                   " WHERE MSVPAname = '" + MSVPAName + "'" +
                   " AND ForeName = '" + ForecastName + "'" +
                   " AND Scenario = '" + ScenarioName + "'" +
                   " AND SpeName = '" + PredList[i] +
                   "' GROUP BY Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        NumRecords = dataMap["Year"].size();
        if (NumRecords > 0) {
            for (int j = 0; j < ForecastNYears; ++j) {
                TableData[i][j][0] = std::stod(dataMap["AnnTot"][j]);
            } // end for j
        } // end if

        fields = {"SpeName","Year","SizeClass","Biomass"};
        queryStr = "SELECT SpeName,Year,SizeClass,Biomass FROM " +
                     nmfConstantsMSVPA::TableScenarioOthPred +
                   " WHERE MSVPAname = '" + MSVPAName + "'" +
                   " AND ForeName = '" + ForecastName + "'" +
                   " AND Scenario = '" + ScenarioName + "'" + \
                   " AND SpeName = '" +  PredList[i] + "'" +
                   " ORDER BY SpeName, Year, SizeClass";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        NumRecords = dataMap["Year"].size();
        if (NumRecords > 0) {
            m = 0;
            for (int j = 0; j < ForecastNYears; ++j) {
                for (int k = 0; k < NPredAge[i]; ++k) {
                    val = nmfUtils::round( std::stod(dataMap["Biomass"][m++]) / TableData[i][j][0], 3);
                    val = (std::isnan(val)) ? 0 : val;
                    TableData[i][j][k+1] = val;
                } // end for k
            } // end for j
        } // end if

    } // end for i

    int index = Forecast_Tab4_SpeciesCMB->currentIndex();
    std::vector<std::string> headerList;
    headerList.push_back("Biomass");
    for (int i=1; i<=NPredAge[index]; ++i) {
        headerList.push_back("Size "+std::to_string(i));
    }
    updateTable(0,headerList);
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_Load2PB Complete");

} // end callback_Forecast_Tab4_Load2PB


void
nmfForecastTab4::callback_Forecast_Tab4_Load3PB(bool unused)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    int m;
    int NPreds;
    //int NPredAgeVal;
    std::string species;
    std::string ScenarioName = Forecast_Tab4_ScenarioCMB->currentText().toStdString();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_Load3PB");
    Forecast_Tab4_SavePB->setEnabled(true);
    hideArrows();
    Forecast_Tab4_Load3ArrowLBL->show();

    Forecast_Tab4_SavePB->setToolTip("Save tables: Scenarios and ScenarioOthPrey");
    Forecast_Tab4_SavePB->setStatusTip("Save tables: Scenarios and ScenarioOthPrey");
    varButtonPressed = 3;

    PredList.clear();
    NPredAge.clear();
    Forecast_Tab4_SpeciesLBL->show();
    Forecast_Tab4_SpeciesCMB->show();
    Forecast_Tab4_SpeciesCMB->clear();
    nmfUtils::initialize(TableData);
    NPredAge.push_back(0); // to get the callback working correctly.


    // Load species combo box with prey names
    fields = {"OthPreyName"};
    queryStr = "SELECT OthPreyName FROM " + nmfConstantsMSVPA::TableMSVPAOthPrey +
               " WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    int NumRecords = dataMap["OthPreyName"].size();
    for (int i=0; i<NumRecords; ++i)
    {
        species = dataMap["OthPreyName"][i];
        PredList.push_back(species);
        Forecast_Tab4_SpeciesCMB->addItem(QString::fromStdString(species));
    }
    Forecast_Tab4_SpeciesCMB->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    // Find number of seasons
    fields   = {"NSeasons"};
    queryStr = "SELECT NSeasons FROM " + nmfConstantsMSVPA::TableMSVPAlist +
               " WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NSeasons = std::stoi(dataMap["NSeasons"][0]);

    NPreds = PredList.size();
    for (int i = 0; i < NPreds; ++i) {
        fields   = {"Biomass"};
        queryStr = "SELECT Biomass FROM " + nmfConstantsMSVPA::TableMSVPAOthPreyAnn +
                   " WHERE MSVPAname = '" + MSVPAName + "'" +
                   " AND OthPreyName = '" + PredList[i] + "'" +
                   " AND Year = " + std::to_string(ForecastFirstYear) +
                   " ORDER BY Season";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (int j = 0; j < NSeasons; ++j) {
            TableData[i][0][j] = std::stod(dataMap["Biomass"][j]);

        }
    } // end for

    // initialize data - load data if it exists
    fields   = {"Biomass"};
    queryStr = "SELECT Biomass FROM " + nmfConstantsMSVPA::TableScenarioOthPrey +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND ForeName = '" + ForecastName + "'" +
               " AND Scenario = '" + ScenarioName + "'" +
               " ORDER By SpeIndex, Season, Year";

    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["Biomass"].size();
    if (NumRecords > 0) {
        m = 0;
        for (int i = 0; i< NPreds; ++i) {
            for (int j = 0; j < NSeasons; ++j) {
                for (int k = 0; k < ForecastNYears; ++k) {
                    TableData[i][k][j] = std::stod(dataMap["Biomass"][m++]);
                } // end for k
            } // end for j
        } // end for i
    } // end if

    // Set the horizontal header and update the table
    std::vector<std::string> headerList;
    for (int i=0; i<NSeasons; ++i) {
        headerList.push_back("Season "+std::to_string(i+1));
    }
    updateTable(0,headerList);
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_Load3PB Complete");

} // end callback_Forecast_Tab4_Load3PB


void
nmfForecastTab4::callback_Forecast_Tab4_Load4PB(bool unused)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    //int m;
    int NPreds;
    int NumRecords;
    int absRecruits;
    double val;
    //int NPredAgeVal;
    std::string species;
    //std::string ScenarioName = Forecast_Tab4_ScenarioCMB->currentText().toStdString();
    std::vector<std::string> headerList;

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_Load4PB");
    Forecast_Tab4_SavePB->setEnabled(true);
    hideArrows();
    Forecast_Tab4_Load4ArrowLBL->show();

    Forecast_Tab4_SavePB->setToolTip("Save tables: Scenarios and ScenarioRec");
    Forecast_Tab4_SavePB->setStatusTip("Save tables: Scenarios and ScenarioRec");
    varButtonPressed = 4;

    PredList.clear();
    NPredAge.clear();
    nmfUtils::initialize(TableData);
    NPredAge.push_back(0); // to get the callback working correctly.

    Forecast_Tab4_SpeciesCMB->clear();
    Forecast_Tab4_SpeciesLBL->hide();
    Forecast_Tab4_SpeciesCMB->hide();

    Forecast_Tab4_DataTW->clear();

    // Select the predator list..associated with the MSVPA
    // Load MSVPA predator name list.
    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND (Type = 0 or Type = 1)";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NPreds = dataMap["SpeName"].size();

    for (int i = 0; i < NPreds; ++i) {
        species = dataMap["SpeName"][i];
        headerList.push_back(species);
        PredList.push_back(species);
    }

    for (int j = 0; j < NPreds; ++j) {
        for (int k = 0; k < ForecastNYears; ++k) {
            TableData[0][k][j] = 1;
        }
    }

    updateTable(1,headerList);


    // initialize data - load data if they exist
    QCheckBox *absRecruitsCB;
    QWidget *w;
    QTableWidgetItem *item;
    int m,n;
    fields   = {"AbsRecruits","RecAdjust"};
    queryStr = "SELECT AbsRecruits,RecAdjust FROM " + nmfConstantsMSVPA::TableScenarioRec +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND ForeName = '" + ForecastName + "'" +
               " AND Scenario = '" + ScenarioName + "'" +
               " ORDER By SpeIndex,Year";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["AbsRecruits"].size();
    if (NumRecords > 0) {
        m = 0;
        n = 0;
        for (int col=0; col<=NPreds-1; ++col) {
            // Set the checkbox state.
            // There's only 1 checkbox, so assume the first AbsRecruits should
            // be the same as all the others for the species
            absRecruits = std::stoi(dataMap["AbsRecruits"][m]);
            w  = Forecast_Tab4_DataTW->cellWidget(0, col);
            absRecruitsCB = w->findChild<QCheckBox *>();
            absRecruitsCB->setChecked(absRecruits);

            // Set all the year values below the checkbox
            for (int j=0; j<ForecastNYears; ++j) {
                val = std::stod(dataMap["RecAdjust"][n++]);
                item = Forecast_Tab4_DataTW->item(j+1,col);
                item->setText(QString::number(val));
                TableData[0][j+1][col] = val;
                m++;
            }
        } // end for
    } // end if
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::callback_Forecast_Tab4_Load4PB Complete");

} // end callback_Forecast_Tab4_Load4PB


void
nmfForecastTab4::callback_Forecast_Tab4_RecCBs(int state)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    double val;
    int col = 0;
    QTableWidgetItem *item;

    QCheckBox *cb = qobject_cast<QCheckBox*>(sender());
    std::vector<std::string>::iterator iter;

    std::string species = cb->objectName().toStdString();

    // Find index of species
    iter = std::find(PredList.begin(),PredList.end(),species);
    if (iter != PredList.end()) {
        col = std::distance(PredList.begin(),iter);
    }

    if (state == Qt::Checked) {
        fields = {"SpeName","Age","AnnAbund"};
        queryStr = "SELECT SpeName, Age, AnnAbund FROM " + nmfConstantsMSVPA::TableMSVPASeasBiomass +
                   " WHERE SpeName = '" + species + "'" +
                   " AND Year = " + std::to_string(ForecastFirstYear-MSVPAFirstYear) +
                   " AND Season = 0 and Age = 0";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        val = std::stod(dataMap["AnnAbund"][0]);

        // update data table
        TableData[0][0][col] = val;

        // Redraw table widget item
        item = Forecast_Tab4_DataTW->item(1,col);
        item->setText(QString::number(val));
    } else {
        TableData[0][0][col] = 1;
        item = Forecast_Tab4_DataTW->item(1,col);
        item->setText("1");
    }

} // end callback_Forecast_Tab4_RecCBs


void
nmfForecastTab4::callback_Forecast_Tab4_Load1CB(int state)
{
    Forecast_Tab4_Load1PB->setEnabled(state==Qt::Checked);
    if (state != Qt::Checked) {
        Forecast_Tab4_SpeciesCMB->clear();
    }
    Forecast_Tab4_SavePB->setEnabled(true);
    MarkAsDirty(nmfConstantsMSVPA::TableScenarios);

}

void
nmfForecastTab4::callback_Forecast_Tab4_Load2CB(int state)
{
    Forecast_Tab4_Load2PB->setEnabled(state==Qt::Checked);
    Forecast_Tab4_SavePB->setEnabled(true);
    MarkAsDirty(nmfConstantsMSVPA::TableScenarios);

}

void
nmfForecastTab4::callback_Forecast_Tab4_Load3CB(int state)
{
    Forecast_Tab4_Load3PB->setEnabled(state==Qt::Checked);
    Forecast_Tab4_SavePB->setEnabled(true);
    MarkAsDirty(nmfConstantsMSVPA::TableScenarios);

}

void
nmfForecastTab4::callback_Forecast_Tab4_Load4CB(int state)
{
    Forecast_Tab4_Load4PB->setEnabled(state==Qt::Checked);
    Forecast_Tab4_SavePB->setEnabled(true);
    MarkAsDirty(nmfConstantsMSVPA::TableScenarios);

}



void
nmfForecastTab4::processRadioButton()
{
    Forecast_Tab4_DataTW->clear();

    Forecast_Tab4_SavePB->setEnabled(true);

    switch (varButtonPressed) {
        case 1:
            callback_Forecast_Tab4_Load1PB(true);
            break;
        case 2:
            callback_Forecast_Tab4_Load2PB(true);
            break;
        case 3:
            callback_Forecast_Tab4_Load3PB(true);
            break;
        case 4:
            callback_Forecast_Tab4_Load4PB(true);
            break;
        default:
            break;
    }

} // end processRadioButton

void
nmfForecastTab4::callback_Forecast_Tab4_Load1RB()
{
    Forecast_Tab4_Load1PB->setText("Load F");
    Forecast_Tab4_Load1CB->setText("Variable Fishing Mortality Rates");

    // Have to do this because the radio button isn't marked as checked until after this function.
    Forecast_Tab4_Load1RB->blockSignals(true);
    Forecast_Tab4_Load1RB->setChecked(true);
    Forecast_Tab4_Load1RB->blockSignals(false);
    MarkAsDirty(nmfConstantsMSVPA::TableScenarios);

    processRadioButton();

} // end callback_Forecast_Tab4_Load1RB


void
nmfForecastTab4::callback_Forecast_Tab4_Load2RB()
{
    Forecast_Tab4_Load1PB->setText("Load Catch");
    Forecast_Tab4_Load1CB->setText("Variable Fishery Catches");

    // Have to do this because the radio button isn't marked as checked until after this function.
    Forecast_Tab4_Load2RB->blockSignals(true);
    Forecast_Tab4_Load2RB->setChecked(true);
    Forecast_Tab4_Load2RB->blockSignals(false);
    MarkAsDirty(nmfConstantsMSVPA::TableScenarios);

    processRadioButton();

} // end callback_Forecast_Tab4_Load2RB


void
nmfForecastTab4::callback_Forecast_Tab4_PrevPB(bool unused)
{
    Forecast_Tabs->setCurrentIndex(Forecast_Tabs->currentIndex()-1);
}

void
nmfForecastTab4::callback_Forecast_Tab4_NextPB(bool unused)
{
    Forecast_Tabs->setCurrentIndex(Forecast_Tabs->currentIndex()+1);
}

void
nmfForecastTab4::callback_Forecast_Tab4_UnFillSQPB(bool unused)
{
    QTableWidgetItem *item;
    int index = 0;
    int rowOffset=0;
    double val = 0;

    if (varButtonPressed == 4)
        val = 1;

    if (Forecast_Tab4_DataTW->verticalHeaderItem(0)->text() == "Abs. Recruits") {
        rowOffset = 1;
    } else {
        index = Forecast_Tab4_SpeciesCMB->currentIndex();
    }

    // Every row gets the data that's in the first row.
    for (int i=1; i<ForecastNYears; ++i) {
        for (int j=0; j<Forecast_Tab4_DataTW->columnCount(); ++j) {
            // Update table data
            TableData[index][i][j] = val;
            // Refresh table gui
            item = new QTableWidgetItem(QString::number(val));
            item->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
            Forecast_Tab4_DataTW->setItem(i+rowOffset,j,item);
        }
    }

} // end callback_Forecast_Tab4_UnFillSQPB

void
nmfForecastTab4::callback_Forecast_Tab4_FillSQPB(bool unused)
{
    QTableWidgetItem *item;
    int index = 0;
    int rowOffset=0;
    double val;

    if (Forecast_Tab4_DataTW->verticalHeaderItem(0)->text() == "Abs. Recruits") {
        rowOffset = 1;
    } else {
        index = Forecast_Tab4_SpeciesCMB->currentIndex();
    }

    // Every row gets the data that's in the first row.
    for (int i=1; i<ForecastNYears; ++i) {
        for (int j=0; j<Forecast_Tab4_DataTW->columnCount(); ++j) {
            val = TableData[index][0][j];
            // Update table data
            TableData[index][i][j] = val;
            // Refresh table gui
            item = new QTableWidgetItem(QString::number(val));
            item->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
            Forecast_Tab4_DataTW->setItem(i+rowOffset,j,item);
        }
    }

} // end callback_Forecast_Tab4_FillSQPB


void
nmfForecastTab4::load(nmfDatabase *theDatabasePtr,
                      std::string theMSVPAName,
                      std::string theForecastName,
                      int theForecastFirstYear,
                      int theForecastNYears,
                      int theMSVPAFirstYear,
                      int theMSVPALastYear)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    databasePtr = theDatabasePtr;
    MSVPAName   = theMSVPAName;
    ForecastName = theForecastName;
    ForecastFirstYear = theForecastFirstYear;
    MSVPAFirstYear = theMSVPAFirstYear;
    ForecastNYears = theForecastNYears;
    MSVPANYears    = theMSVPALastYear-theMSVPAFirstYear+1;
    NSeasons = 0;

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::load");

    // Mark as disabled until user changes something.
    Forecast_Tab4_SavePB->setEnabled(false);

    TableData.resize(boost::extents[nmfConstants::MaxNumberSpecies]
                                    [nmfConstants::MaxNumberYears]
                                    [nmfConstants::MaxNumberAges]);

    Scenarios *aScenario;
    ScenarioList.clear();
    Forecast_Tab4_ScenarioCMB->clear();

    fields = {"Scenario","VarF","VarOthPred","VarOthPrey","VarRec","FishAsF"};
    queryStr = "SELECT Scenario,VarF,VarOthPred,VarOthPrey,VarRec,FishAsF FROM " +
                nmfConstantsMSVPA::TableScenarios +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND ForeName = '" + ForecastName + "'" +
               " ORDER By Scenario";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    int NumRecords = dataMap["Scenario"].size();
    for (int i=0; i<NumRecords; ++i)
    {
        aScenario = new Scenarios();
        aScenario->FishAsF    = std::stoi(dataMap["FishAsF"][i]);
        aScenario->VarF       = std::stoi(dataMap["VarF"][i]);
        aScenario->VarOthPred = std::stoi(dataMap["VarOthPred"][i]);
        aScenario->VarOthPrey = std::stoi(dataMap["VarOthPrey"][i]);
        aScenario->VarRec     = std::stoi(dataMap["VarRec"][i]);
        aScenario->Name       = dataMap["Scenario"][i];
        ScenarioList.push_back(*aScenario);
        Forecast_Tab4_ScenarioCMB->addItem(QString::fromStdString(dataMap["Scenario"][i]));
        if (i==0) { // adjust all widgets accordingly
            Forecast_Tab4_Load1CB->setChecked(aScenario->VarF);
            Forecast_Tab4_Load2CB->setChecked(aScenario->VarOthPred);
            Forecast_Tab4_Load3CB->setChecked(aScenario->VarOthPrey);
            Forecast_Tab4_Load4CB->setChecked(aScenario->VarRec);
            Forecast_Tab4_Load1RB->setChecked(aScenario->FishAsF);
            Forecast_Tab4_Load2RB->setChecked(! aScenario->FishAsF);
        }
    }
    Forecast_Tab4_ScenarioCMB->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    Forecast_Tab4_SavePB->setEnabled(false);

    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab4::load Complete");

} // end load


void
nmfForecastTab4::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableScenarios);
    fields    = {"MSVPAName","ForeName","Scenario","VarF",
                 "VarOthPred","VarOthPrey","VarRec","FishAsF"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableScenarioF);
    fields    = {"MSVPAName","ForeName","Scenario","VarType",
                 "SpeName","SpeIndex","Age","Year","F"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableScenarioOthPred);
    fields    = {"MSVPAName","ForeName","Scenario","SpeName",
                 "SpeIndex","SizeClass","Year","Biomass","PropBM"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableScenarioOthPrey);
    fields    = {"MSVPAName","ForeName","Scenario","SpeName",
                 "SpeIndex","Year","Season","Biomass"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableScenarioRec);
    fields    = {"MSVPAName","ForeName","Scenario","SpeName",
                 "SpeIndex","Year","RecAdjust","AbsRecruits"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);
/*
 *  Don't dump these since they take up a lot of room, the format is a bit
 *  complicated for users to parse, and users should extract this data
 *  by clicking the camera icon over the desired output table.
 *
    TableName = QString::fromStdString(nmfConstantsMSVPA::TableForeOutput);
    fields    = {"MSVPAName", "ForeName", "Scenario", "Year", "Season", "SpeName",
                 "Age", "SpeType", "InitAbund", "EndAbund", "InitBiomass", "EndBiomass",
                 "SeasM2", "SeasF", "SeasM1", "AvgSize", "AvgWeight", "TotalSBM",
                 "TotalBMConsumed", "StomCont", "InitWt", "EndWt", "SeasCatch", "SeasYield"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableForeSuitPreyBiomass);
    fields    = {"MSVPAName", "ForeName", "Scenario", "PredName", "PredAge", "PreyName", "PreyAge",
                 "Year", "Season", "SuitPreyBiomass", "PropDiet", "EDens", "BMConsumed", "PredType"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);
*/
    TableName = QString::fromStdString(nmfConstantsMSVPA::TableForePredGrowth);
    fields    = {"MSVPAName", "ForeName", "PredName", "PredIndex", "PredType", "Age",
                 "RAlpha", "RBeta", "RQ", "ACT", "E", "U", "SDA", "LWAlpha", "LWBeta"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableForeEnergyDens);
    fields    = {"MSVPAName", "ForeName", "SpeName", "SpeType", "SpeIndex",
                 "Age", "Season", "EnergyDens", "AvgDietE"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);



} // end restoreCSVFromDatabase




void
nmfForecastTab4::clearWidgets()
{
//    Forecast_Tab4_MsvpaNameLE->clear();
//    Forecast_Tab4_ForecastNameLE->clear();
//    Forecast_Tab4_ScenarioNameLE->clear();
//    Forecast_Tab4_InitialYearCMB->clear();
//    Forecast_Tab4_NumYearsLE->clear();
//    Forecast_Tab4_PredatorGrowthCB->setChecked(false);
}
