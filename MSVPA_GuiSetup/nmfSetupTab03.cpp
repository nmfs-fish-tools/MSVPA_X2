
#include "nmfSetupTab03.h"
#include "nmfConstants.h"
#include "nmfUtilsQt.h"
#include "nmfUtils.h"


nmfSetup_Tab3::nmfSetup_Tab3(QTabWidget  *tabs,
                           nmfLogger   *theLogger,
                           QTextEdit   *theSetupOutputTE,
                           std::string &theProjectDir)
{
    QUiLoader loader;

    Setup_Tabs    = tabs;
    databasePtr   = NULL;
    logger        = theLogger;
    SetupOutputTE = theSetupOutputTE;
    ProjectDir    = theProjectDir;
    smodelSpecies = NULL;
    smodelOtherPredSpecies = NULL;

    // Load ui as a widget from disk
    QFile file(":/forms/Setup/Setup_Tab03.ui");
    file.open(QFile::ReadOnly);
    Setup_Tab3_Widget = loader.load(&file,Setup_Tabs);
    Setup_Tab3_Widget->setObjectName("Setup_Tab3_Widget");
    file.close();

    // Add the loaded widget as the new tabbed page
    Setup_Tabs->addTab(Setup_Tab3_Widget, tr("3. Species Setup"));

    // Setup The FunctionMap map so don't have to use if...else...if...else... statement.
    FunctionMap["ForeEnergyDens"]       = buildCSVFileForeEnergyDens;       // Table  1 of 39
    FunctionMap["ForeOutput"]           = buildCSVFileForeOutput;           // Table  2 of 39
    FunctionMap["ForePredGrowth"]       = buildCSVFileForePredGrowth;       // Table  3 of 39
    FunctionMap["ForePredVonB"]         = buildCSVFileForePredVonB;         // Table  4 of 39
    FunctionMap["ForeSRQ"]              = buildCSVFileForeSRQ;              // Table  5 of 39
    FunctionMap["ForeSRR"]              = buildCSVFileForeSRR;              // Table  6 of 39
    FunctionMap["ForeSuitPreyBiomass"]  = buildCSVFileForeSuitPreyBiomass;  // Table  7 of 39
    FunctionMap["Forecasts"]            = buildCSVFileForecasts;            // Table  8 of 39
    FunctionMap["MSVPAEnergyDens"]      = buildCSVFileMSVPAEnergyDens;      // Table  9 of 39
    FunctionMap["MSVPAOthPrey"]         = buildCSVFileMSVPAOthPrey;         // Table 10 of 39
    FunctionMap["MSVPAOthPreyAnn"]      = buildCSVFileMSVPAOthPreyAnn;      // Table 11 of 39
    FunctionMap["MSVPASeasBiomass"]     = buildCSVFileMSVPASeasBiomass;     // Table 12 of 39
    FunctionMap["MSVPASeasInfo"]        = buildCSVFileMSVPASeasInfo;        // Table 13 of 39
    FunctionMap["MSVPASizePref"]        = buildCSVFileMSVPASizePref;        // Table 14 of 39
    FunctionMap["MSVPASpaceO"]          = buildCSVFileMSVPASpaceO;          // Table 15 of 39
    FunctionMap["MSVPAStomCont"]        = buildCSVFileMSVPAStomCont;        // Table 16 of 39
    FunctionMap["MSVPASuitPreyBiomass"] = buildCSVFileMSVPASuitPreyBiomass; // Table 17 of 39
    FunctionMap["MSVPAlist"]            = buildCSVFileMSVPAlist;            // Table 18 of 39
    FunctionMap["MSVPAprefs"]           = buildCSVFileMSVPAprefs;           // Table 19 of 39
    FunctionMap["MSVPAspecies"]         = buildCSVFileMSVPAspecies;         // Table 20 of 39
    FunctionMap["OthPredSizeData"]      = buildCSVFileOthPredSizeData;      // Table 21 of 39
    FunctionMap["OtherPredBM"]          = buildCSVFileOtherPredBM;          // Table 22 of 39
    FunctionMap["OtherPredSpecies"]     = buildCSVFileOtherPredSpecies;     // Table 23 of 39
    FunctionMap["SSVPAAgeM"]            = buildCSVFileSSVPAAgeM;            // Table 24 of 39
    FunctionMap["ScenarioF"]            = buildCSVFileScenarioF;            // Table 25 of 39
    FunctionMap["ScenarioOthPred"]      = buildCSVFileScenarioOthPred;      // Table 26 of 39
    FunctionMap["ScenarioOthPrey"]      = buildCSVFileScenarioOthPrey;      // Table 27 of 39
    FunctionMap["ScenarioRec"]          = buildCSVFileScenarioRec;          // Table 28 of 39
    FunctionMap["Scenarios"]            = buildCSVFileScenarios;            // Table 29 of 39
    FunctionMap["SpeCatch"]             = buildCSVFileSpeCatch;             // Table 30 of 39
    FunctionMap["SpeMaturity"]          = buildCSVFileSpeMaturity;          // Table 31 of 39
    FunctionMap["SpeSSVPA"]             = buildCSVFileSpeSSVPA;             // Table 32 of 39
    FunctionMap["SpeSize"]              = buildCSVFileSpeSize;              // Table 33 of 39
    FunctionMap["SpeTuneCatch"]         = buildCSVFileSpeTuneCatch;         // Table 34 of 39
    FunctionMap["SpeTuneEffort"]        = buildCSVFileSpeTuneEffort;        // Table 35 of 39
    FunctionMap["SpeWeight"]            = buildCSVFileSpeWeight;            // Table 36 of 39
    FunctionMap["SpeXSAData"]           = buildCSVFileSpeXSAData;           // Table 37 of 39
    FunctionMap["SpeXSAIndices"]        = buildCSVFileSpeXSAIndices;        // Table 38 of 39
    FunctionMap["Species"]              = buildCSVFileSpecies;              // Table 39 of 39

    //nmfUtils::readTableNames(TableNames);

    Setup_Tab3_SpeciesTW              = Setup_Tabs->findChild<QTableWidget *>("Setup_Tab3_SpeciesTW");
    Setup_Tab3_OtherPredatorsTW       = Setup_Tabs->findChild<QTableWidget *>("Setup_Tab3_OtherPredatorsTW");
    Setup_Tab3_CreateTemplatesPB      = Setup_Tabs->findChild<QPushButton  *>("Setup_Tab3_CreateTemplatesPB");
    Setup_Tab3_SpeciesCB              = Setup_Tabs->findChild<QCheckBox    *>("Setup_Tab3_SpeciesCB");
    Setup_Tab3_OtherPredatorsCB       = Setup_Tabs->findChild<QCheckBox    *>("Setup_Tab3_OtherPredatorsCB");
    Setup_Tab3_NumSpeciesLBL          = Setup_Tabs->findChild<QLabel       *>("Setup_Tab3_NumSpeciesLBL");
    Setup_Tab3_NumSpeciesSB           = Setup_Tabs->findChild<QSpinBox     *>("Setup_Tab3_NumSpeciesSB");
    Setup_Tab3_NumOtherPredatorsLBL   = Setup_Tabs->findChild<QLabel       *>("Setup_Tab3_NumOtherPredatorsLBL");
    Setup_Tab3_NumOtherPredatorsSB    = Setup_Tabs->findChild<QSpinBox     *>("Setup_Tab3_NumOtherPredatorsSB");
    Setup_Tab3_AddSpeciesPB           = Setup_Tabs->findChild<QPushButton  *>("Setup_Tab3_AddSpeciesPB");
    Setup_Tab3_DelSpeciesPB           = Setup_Tabs->findChild<QPushButton  *>("Setup_Tab3_DelSpeciesPB");
    Setup_Tab3_ReloadSpeciesPB        = Setup_Tabs->findChild<QPushButton  *>("Setup_Tab3_ReloadSpeciesPB");
    Setup_Tab3_AddOtherPredatorsPB    = Setup_Tabs->findChild<QPushButton  *>("Setup_Tab3_AddOtherPredatorsPB");
    Setup_Tab3_DelOtherPredatorsPB    = Setup_Tabs->findChild<QPushButton  *>("Setup_Tab3_DelOtherPredatorsPB");
    Setup_Tab3_ReloadOtherPredatorsPB = Setup_Tabs->findChild<QPushButton  *>("Setup_Tab3_ReloadOtherPredatorsPB");
    Setup_Tab3_SavePB                 = Setup_Tabs->findChild<QPushButton  *>("Setup_Tab3_SavePB");
    Setup_Tab3_LoadPB                 = Setup_Tabs->findChild<QPushButton  *>("Setup_Tab3_LoadPB");
    Setup_Tab2_ProjectDirLE           = Setup_Tabs->findChild<QLineEdit    *>("Setup_Tab2_ProjectDirLE");
    Setup_Tab2_FirstYearLE            = Setup_Tabs->findChild<QLineEdit    *>("Setup_Tab2_FirstYearLE");
    Setup_Tab2_LastYearLE             = Setup_Tabs->findChild<QLineEdit    *>("Setup_Tab2_LastYearLE");
    Setup_Tab2_NumSeasonsLE           = Setup_Tabs->findChild<QLineEdit    *>("Setup_Tab2_NumSeasonsLE");


    //Setup_Tab3_ContinuePB->setText("Continue to SSVPA Section \u261E");

    connect(Setup_Tab3_NumSpeciesSB,           SIGNAL(valueChanged(int)),
            this,                              SLOT(callback_Setup_Tab3_NumSpecies(int)));
    connect(Setup_Tab3_NumOtherPredatorsSB,    SIGNAL(valueChanged(int)),
            this,                              SLOT(callback_Setup_Tab3_NumOtherPredators(int)));
    connect(Setup_Tab3_SpeciesTW,              SIGNAL(itemChanged(QTableWidgetItem *)),
            this,                              SLOT(callback_Setup_Tab3_SpeciesChanged(QTableWidgetItem *)));
    connect(Setup_Tab3_OtherPredatorsTW,       SIGNAL(itemChanged(QTableWidgetItem *)),
            this,                              SLOT(callback_Setup_Tab3_OtherPredatorsChanged(QTableWidgetItem *)));
    connect(Setup_Tab3_SpeciesCB,              SIGNAL(toggled(bool)),
            this,                              SLOT(callback_Setup_Tab3_SpeciesCB(bool)));
    connect(Setup_Tab3_OtherPredatorsCB,       SIGNAL(toggled(bool)),
            this,                              SLOT(callback_Setup_Tab3_OtherPredatorsCB(bool)));
    connect(Setup_Tab3_CreateTemplatesPB,      SIGNAL(clicked(bool)),
            this,                              SLOT(callback_Setup_Tab3_CreateTemplates(bool)));
    connect(Setup_Tab3_AddSpeciesPB,           SIGNAL(clicked(bool)),
            this,                              SLOT(callback_Setup_Tab3_AddSpeciesPB(bool)));
    connect(Setup_Tab3_DelSpeciesPB,           SIGNAL(clicked(bool)),
            this,                              SLOT(callback_Setup_Tab3_DelSpeciesPB(bool)));
    connect(Setup_Tab3_ReloadSpeciesPB,        SIGNAL(clicked(bool)),
            this,                              SLOT(callback_Setup_Tab3_ReloadSpeciesPB(bool)));
    connect(Setup_Tab3_AddOtherPredatorsPB,    SIGNAL(clicked(bool)),
            this,                              SLOT(callback_Setup_Tab3_AddOtherPredatorsPB(bool)));
    connect(Setup_Tab3_DelOtherPredatorsPB,    SIGNAL(clicked(bool)),
            this,                              SLOT(callback_Setup_Tab3_DelOtherPredatorsPB(bool)));
    connect(Setup_Tab3_ReloadOtherPredatorsPB, SIGNAL(clicked(bool)),
            this,                              SLOT(callback_Setup_Tab3_ReloadOtherPredatorsPB(bool)));
    connect(Setup_Tab3_SavePB,                 SIGNAL(clicked(bool)),
            this,                              SLOT(callback_Setup_Tab3_SavePB(bool)));
    connect(Setup_Tab3_LoadPB,                 SIGNAL(clicked(bool)),
            this,                              SLOT(callback_Setup_Tab3_LoadPB(bool)));

    Setup_Tab3_NumSpeciesSB->setMaximum(nmfConstants::MaxNumberSpecies);

    Setup_Tab3_CreateTemplatesPB->setEnabled(false);
//    readSettings();

    Setup_Tab3_SpeciesCB->setChecked(false);
    Setup_Tab3_OtherPredatorsCB->setChecked(false);
}


nmfSetup_Tab3::~nmfSetup_Tab3()
{
}

void
nmfSetup_Tab3::callback_LoadDatabase(QString db)
{
    int NumSpecies;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;

    //databasePtr->nmfSetDatabase(db.toStdString());

    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM Species";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumSpecies = dataMap["SpeName"].size();
    Setup_Tab3_NumSpeciesSB->setValue(NumSpecies);
    Setup_Tab3_SpeciesCB->setChecked(NumSpecies > 0);

    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM OtherPredSpecies";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumSpecies = dataMap["SpeName"].size();
    Setup_Tab3_NumOtherPredatorsSB->setValue(NumSpecies);
    Setup_Tab3_OtherPredatorsCB->setChecked(NumSpecies > 0);

} // end callback_LoadDatabase

void
nmfSetup_Tab3::callback_Setup_Tab3_SpeciesChanged(QTableWidgetItem *item)
{
    int val;
    bool ok;

    disconnect(Setup_Tab3_SpeciesTW, SIGNAL(itemChanged(QTableWidgetItem *)),
               this,                 SLOT(callback_Setup_Tab3_SpeciesChanged(QTableWidgetItem *)));

    switch (item->column()) {
        case Column_SpeIndex:
            val = item->text().toInt(&ok,10);
            if ((! ok) || (val <= 0)) {
                QMessageBox::warning(Setup_Tabs,tr("Invalid Index"),
                                     tr("\nPlease enter a valid integer Species index value > 0."),
                                     QMessageBox::Ok);
                item->setText("");
            }
            break;
        case Column_SpeMaxAge:
            val = item->text().toInt(&ok,10);
            if ((! ok) || (val <= 0)) {
                QMessageBox::warning(Setup_Tabs,tr("Invalid MaxAge"),
                                     tr("\nPlease enter a valid integer MaxAge value > 0."),
                                     QMessageBox::Ok);
                item->setText("");
            }
            break;
        case Column_SpeMinCatchAge:
            val = item->text().toInt(&ok,10);
            if ((! ok) || (val < 0)) {
                QMessageBox::warning(Setup_Tabs,tr("Invalid MinCatchAge"),
                                     tr("\nPlease enter a valid integer Minimum Catch Age index value >= 0."),
                                     QMessageBox::Ok);
                item->setText("");
            }
            break;
        case Column_SpeMaxCatchAge:
            val = item->text().toInt(&ok,10);
            if ((! ok) || (val <= 0)) {
                QMessageBox::warning(Setup_Tabs,tr("Invalid MaxCatchAge"),
                                     tr("\nPlease enter a valid integer Maximum Catch Age index value > 0."),
                                     QMessageBox::Ok);
                item->setText("");
            }
            break;
        case Column_SpeFirstYear:
            val = item->text().toInt(&ok,10);
            if ((! ok) || (val <= 0)) {
                QMessageBox::warning(Setup_Tabs,tr("Invalid First Year"),
                                     tr("\nPlease enter a valid integer First Year value > 0."),
                                     QMessageBox::Ok);
                item->setText("");
            }
            break;
        case Column_SpeLastYear:
            val = item->text().toInt(&ok,10);
            if ((! ok) || (val <= 0)) {
                QMessageBox::warning(Setup_Tabs,tr("Invalid Last Year"),
                                     tr("\nPlease enter a valid integer Last Year value > 0."),
                                     QMessageBox::Ok);
                item->setText("");
            }
            break;
        default:
            break;

    } // end switch

    connect(Setup_Tab3_SpeciesTW, SIGNAL(itemChanged(QTableWidgetItem *)),
            this,                 SLOT(callback_Setup_Tab3_SpeciesChanged(QTableWidgetItem *)));
    Setup_Tab3_SpeciesTW->resizeColumnsToContents();

} // callback_Setup_Tab3_SpeciesChanged


void
nmfSetup_Tab3::callback_Setup_Tab3_AddSpeciesPB(bool unused)
{

    int rowAdded;
    int ncols = Setup_Tab3_SpeciesTW->columnCount();
    QTableWidgetItem *item;

    if ((Setup_Tab3_SpeciesTW->rowCount() <= 0) &&
        (Setup_Tab3_SpeciesTW->currentIndex().row() < 0))
    {
        callback_Setup_Tab3_NumSpecies(1);
        return;
    }

    // Insert an empty row after the last row and put blanks in the vertical header.
    Setup_Tab3_SpeciesTW->insertRow(Setup_Tab3_SpeciesTW->rowCount());
    rowAdded = Setup_Tab3_SpeciesTW->rowCount()-1;
    item = new QTableWidgetItem();
    item->setText("    ");
    Setup_Tab3_SpeciesTW->setVerticalHeaderItem(rowAdded,item);

    disconnect(Setup_Tab3_SpeciesTW,      SIGNAL(itemChanged(QTableWidgetItem *)),
            this,                         SLOT(callback_Setup_Tab3_SpeciesChanged(QTableWidgetItem *)));

    // Populate the new row with defaults and widgets accordingly.
    PopulateARowSpecies(Setup_Tab3_SpeciesTW->rowCount()-1,ncols);
    item = Setup_Tab3_SpeciesTW->item(rowAdded,0);
    if (rowAdded >0) {
        item->setText(QString::number(Setup_Tab3_SpeciesTW->item(rowAdded-1,0)->text().toInt()+1));
    } else {
        item->setText("1");
    }

    connect(Setup_Tab3_SpeciesTW,      SIGNAL(itemChanged(QTableWidgetItem *)),
            this,                      SLOT(callback_Setup_Tab3_SpeciesChanged(QTableWidgetItem *)));


    // Lastly, update the value in the spin box to be value+1.
    int NumSpecies = Setup_Tab3_NumSpeciesSB->value();
    Setup_Tab3_NumSpeciesSB->blockSignals(true);
    Setup_Tab3_NumSpeciesSB->setValue(NumSpecies+1);
    Setup_Tab3_NumSpeciesSB->blockSignals(false);

} // end callback_Setup_Tab3_AddSpeciesPB

void
nmfSetup_Tab3::callback_Setup_Tab3_DelSpeciesPB(bool unused)
{
    int NumSpecies = Setup_Tab3_NumSpeciesSB->value();

    // Find number of selected rows
    QItemSelectionModel *selections = Setup_Tab3_SpeciesTW->selectionModel();
    QModelIndexList selected = selections->selectedRows(0);
    qSort(selected);
    int NumSelectedRows = selected.size();

    // Iterate through the rows and delete each one...starting with last one first.
    for (int i=0; i<NumSelectedRows; ++i) {
        Setup_Tab3_SpeciesTW->removeRow(selected[NumSelectedRows-1-i].row());
    }

    // Update the number of rows value in the spin box
    Setup_Tab3_NumSpeciesSB->blockSignals(true);
    Setup_Tab3_NumSpeciesSB->setValue(NumSpecies-NumSelectedRows);
    Setup_Tab3_NumSpeciesSB->blockSignals(false);

} // end callback_Setup_Tab3_DelSpeciesPB



void
nmfSetup_Tab3::callback_Setup_Tab3_ReloadSpeciesPB(bool unused)
{

    LoadSpeciesTable();

} // end callback_Setup_Tab3_ReloadSpeciesPB


void
nmfSetup_Tab3::callback_Setup_Tab3_ReloadOtherPredatorsPB(bool unused)
{

    LoadOtherPredatorsTable();

} // end callback_Setup_Tab3_ReloadOtherPredatorsPB


void
nmfSetup_Tab3::callback_Setup_Tab3_AddOtherPredatorsPB(bool unused)
{
    int rowAdded;
    int ncols = Setup_Tab3_OtherPredatorsTW->columnCount();
    QTableWidgetItem *item;

    if ((Setup_Tab3_OtherPredatorsTW->rowCount() <= 0) &&
        (Setup_Tab3_OtherPredatorsTW->currentIndex().row() < 0))
    {
        callback_Setup_Tab3_NumOtherPredators(1);
        return;
    }

    // Insert an empty row after the last row and put blanks in the vertical header.
    Setup_Tab3_OtherPredatorsTW->insertRow(Setup_Tab3_OtherPredatorsTW->rowCount());
    rowAdded = Setup_Tab3_OtherPredatorsTW->rowCount()-1;
    item = new QTableWidgetItem();
    item->setText("    ");
    Setup_Tab3_OtherPredatorsTW->setVerticalHeaderItem(rowAdded,item);

    disconnect(Setup_Tab3_OtherPredatorsTW, SIGNAL(itemChanged(QTableWidgetItem *)),
               this,                        SLOT(callback_Setup_Tab3_OtherPredatorsChanged(QTableWidgetItem *)));

    // Populate the new row with defaults and widgets accordingly.
    PopulateARowOtherPredators(Setup_Tab3_OtherPredatorsTW->rowCount()-1,ncols);
    item = Setup_Tab3_OtherPredatorsTW->item(rowAdded,0);
    if (rowAdded >0) {
        item->setText(QString::number(Setup_Tab3_OtherPredatorsTW->item(rowAdded-1,0)->text().toInt()+1));
    } else {
        item->setText("1");
    }

    connect(Setup_Tab3_OtherPredatorsTW, SIGNAL(itemChanged(QTableWidgetItem *)),
            this,                        SLOT(callback_Setup_Tab3_OtherPredatorsChanged(QTableWidgetItem *)));


    // Lastly, update the value in the spin box to be value+1.
    int NumOtherPredators = Setup_Tab3_NumOtherPredatorsSB->value();
    Setup_Tab3_NumOtherPredatorsSB->blockSignals(true);
    Setup_Tab3_NumOtherPredatorsSB->setValue(NumOtherPredators+1);
    Setup_Tab3_NumOtherPredatorsSB->blockSignals(false);

} // end callback_Setup_Tab3_AddOtherPredatorsPB


bool
nmfSetup_Tab3::SpeciesFieldsOK(int NumSpecies)
{
    QString msg = "";
    bool marked[10] = {false,false,false,false,false,false,false,false,false,false};

    // Check all fields in Species table and mark which ones are missing to display the
    // appropriate error message.
    for (int i=0; i<NumSpecies; ++i) {
        if ((Setup_Tab3_SpeciesTW->item(i, Column_SpeIndex)->text().isEmpty()) && (! marked[0])) {
            msg += "\nMissing Index field(s)";
            marked[0] = true;
        }
        if ((Setup_Tab3_SpeciesTW->item(i, Column_SpeName)->text().isEmpty()) && (! marked[1])) {
            msg += "\nMissing Name field(s)";
            marked[1] = true;
        }
        if ((Setup_Tab3_SpeciesTW->item(i, Column_SpeFirstYear)->text().isEmpty()) && (! marked[2])) {
            msg += "\nMissing First Year field(s)";
            marked[2] = true;
        }
        if ((Setup_Tab3_SpeciesTW->item(i, Column_SpeLastYear)->text().isEmpty()) && (! marked[3])) {
            msg += "\nMissing Last Year field(s)";
            marked[3] = true;
        }
        if ((Setup_Tab3_SpeciesTW->item(i, Column_SpeMaxAge)->text().isEmpty()) && (! marked[4])) {
            msg += "\nMissing Max Age field(s)";
            marked[4] = true;
        }
        if ((Setup_Tab3_SpeciesTW->item(i, Column_SpeMinCatchAge)->text().isEmpty()) && (! marked[5])) {
            msg += "\nMissing Min Catch Age field(s)";
            marked[5] = true;
        }
        if ((Setup_Tab3_SpeciesTW->item(i, Column_SpeMaxCatchAge)->text().isEmpty()) && (! marked[6])) {
            msg += "\nMissing Max Catch Age field(s)";
            marked[6] = true;
        }
    }
    if (! msg.isEmpty()) {
       msg = "\nPlease enter values in missing fields.\n" + msg + "\n";
       QMessageBox::warning(Setup_Tabs,tr("Missing Species Data"),
                            tr(msg.toLatin1()), QMessageBox::Ok);
       return false;
    }

    return true;

} // end SpeciesFieldsOK

bool
nmfSetup_Tab3::OtherPredatorsFieldsOK(int NumOtherPredators)
{
    QString msg = "";
    bool marked[10] = {false,false,false,false,false,false,false,false,false,false};

    // Check all fields in Other Predators table and mark which of those are missing.
    for (int i=0; i<NumOtherPredators; ++i) {
        if ((Setup_Tab3_OtherPredatorsTW->item(i, Column_OthIndex)->text().isEmpty()) && (! marked[0])) {
            msg += "\nMissing Index field(s)";
            marked[0] = true;
        }
        if ((Setup_Tab3_OtherPredatorsTW->item(i, Column_SpeName)->text().isEmpty()) && (! marked[1])) {
            msg += "\nMissing Name field(s)";
            marked[1] = true;
        }
        if ((Setup_Tab3_OtherPredatorsTW->item(i, Column_OthFirstYear)->text().isEmpty()) && (! marked[2])) {
            msg += "\nMissing First Year field(s)";
            marked[2] = true;
        }
        if ((Setup_Tab3_OtherPredatorsTW->item(i, Column_OthLastYear)->text().isEmpty()) && (! marked[3])) {
            msg += "\nMissing Last Year field(s)";
            marked[3] = true;
        }
        if ((Setup_Tab3_OtherPredatorsTW->item(i, Column_OthMinSize)->text().isEmpty()) && (! marked[4])) {
            msg += "\nMissing Min Length field(s)";
            marked[4] = true;
        }
        if ((Setup_Tab3_OtherPredatorsTW->item(i, Column_OthMaxSize)->text().isEmpty()) && (! marked[5])) {
            msg += "\nMissing Max Length field(s)";
            marked[5] = true;
        }
        if ((Setup_Tab3_OtherPredatorsTW->item(i, Column_OthNumSizeCats)->text().isEmpty()) && (! marked[6])) {
            msg += "\nMissing Num Size Categories field(s)";
            marked[6] = true;
        }
    }
    if (! msg.isEmpty()) {
       msg = "\nPlease enter values in missing fields.\n" + msg + "\n";
       QMessageBox::warning(Setup_Tabs,tr("Missing Other Predators Data"),
                            tr(msg.toLatin1()), QMessageBox::Ok);
       return false;
    }

    return true;

} // end OtherPredatorsFieldsOK


void
nmfSetup_Tab3::renameSpecies(std::string newSpeName, std::string existingSpeName)
{
    std::string cmd;
    std::string errorMsg;
    //int numTasks = nmfConstants::AllTables.size();
    //QProgressDialog progress("Updating tables...", "Cancel",numTasks,0);
    Setup_Tabs->setCursor(Qt::WaitCursor);
    Setup_Tabs->repaint();

    for (std::string table : nmfConstantsMSVPA::AllTables) {
       //progress.setValue(i++);
       //if (progress.wasCanceled) // Don't let user cancel, since that may leave some files updated and some not.
       //    break;
       cmd = "UPDATE " + table + " SET SpeName = REPLACE(SpeName,'" +
              existingSpeName + "','" + newSpeName + "'); ";
       cmd += "UPDATE " + table + " SET PredName = REPLACE(PredName,'" +
              existingSpeName + "','" + newSpeName + "'); ";
       errorMsg = databasePtr->nmfUpdateDatabase(cmd);
       if (errorMsg != " ") {
           //nmfUtils::printError("renameSpecies: ", errorMsg);
       }

    } // end for
    //progress.setValue(numTasks);
    Setup_Tabs->setCursor(Qt::ArrowCursor);

} // end renameSpecies


void
nmfSetup_Tab3::UpdateSpeciesTable(int NumSpecies)
{
    QWidget   *w;
    QCheckBox *cbPlusClass;
    QCheckBox *cbFixedMaturity;
    QComboBox *cmbWtAtAge;
    QComboBox *cmbWtUnits;
    QComboBox *cmbCatchUnits;
    QComboBox *cmbSzAtAge;
    QComboBox *cmbSzUnits;
    QString CatchUnitsStr;
    QString WtUnitsStr;
    QString SzUnitsStr;
    QString qcmd;
    QString newSpeName;
    QString existingSpeName;
    std::string errorMsg;
    std::string cmd;
    std::string WtUnits="";
    std::string CatchUnits="";
    std::string SizeUnits="";
    std::string SpeciesIndex="";
    QMessageBox::StandardButton reply;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    std::set<std::string> SpeciesSet;
    QString msg = "";
    std::set<std::string> SpeciesIndexesToKeep;
    bool renameSpeName;

    // Check if Species table exists and if it doesn't create it.
    qcmd  = "CREATE TABLE IF NOT EXISTS Species ";
    qcmd += "(SpeIndex      int(11) NOT NULL,";
    qcmd += " SpeName       VARCHAR(255),";
    qcmd += " MaxAge        int(11),";
    qcmd += " MinCatAge     int(11),";
    qcmd += " MaxCatAge     int(11),";
    qcmd += " FirstYear     int(11),";
    qcmd += " LastYear      int(11),";
    qcmd += " PlusClass     tinyint(1),";
    qcmd += " CatchUnits    float,";
    qcmd += " WtTypeIndex   int(11),";
    qcmd += " WtUnits       float,";
    qcmd += " SizeTypeIndex int(11),";
    qcmd += " SizeUnits     float,";
    qcmd += " FixedMaturity tinyint(1),";
    qcmd += " PRIMARY KEY (SpeIndex))";
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (errorMsg != " ") {
        nmfUtils::printError("Setup Save(3): Create table error: ", errorMsg);
    }


    // Find all Species indexes
    fields = {"SpeIndex"};
    queryStr = "SELECT SpeIndex FROM Species";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
        SpeciesSet.insert(dataMap["SpeIndex"][i]);
    }

    // Now save the Species table
    bool insertedSpeciesOK = true;
    for (int i=0; i<NumSpecies; ++i) {

        // Find Plus Class
        w = Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpePlusClass);
        cbPlusClass = w->findChild<QCheckBox *>();

        // Find Catch Units
        cmbCatchUnits = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeCatchUnits));
        CatchUnitsStr = cmbCatchUnits->currentText();
        if (CatchUnitsStr == "Hundreds of Fish")
            CatchUnits = "0.1";
        else if (CatchUnitsStr == "Thousands of Fish")
            CatchUnits = "1";
        else if (CatchUnitsStr == "Millions of Fish")
            CatchUnits = "1000";

        // Find Weight At Age and Weight Units
        cmbWtAtAge = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeWtAtAgeData));
        cmbWtUnits = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeWtUnits));
        WtUnitsStr = cmbWtUnits->currentText();
        if (WtUnitsStr == "Grams")
            WtUnits = "0.001";
        else if (WtUnitsStr == "Kilograms")
            WtUnits = "1";
        else if (WtUnitsStr == "Pounds")
            WtUnits = "2.2";

        // Find Size At Age and Size Units
        cmbSzAtAge = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeSzAtAgeData));
        cmbSzUnits = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeSzUnits));
        SzUnitsStr = cmbSzUnits->currentText();
        if (SzUnitsStr == "Millimeters")
            SizeUnits = "0.1";
        else if (SzUnitsStr == "Centimeters")
            SizeUnits = "1";
        else if (SzUnitsStr == "Inches")
            SizeUnits = "2.54";

        // Find Fixed Maturity
        w = Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeFixedMaturity);
        cbFixedMaturity = w->findChild<QCheckBox *>();

        // Get Species Index
        SpeciesIndex = Setup_Tab3_SpeciesTW->item(i,Column_SpeIndex)->text().toStdString();
        SpeciesIndexesToKeep.insert(SpeciesIndex);
        if (SpeciesSet.find(SpeciesIndex) != SpeciesSet.end()) { // then found the item
            msg = "\nSpecies with SpeIndex of " + QString::fromStdString(SpeciesIndex) + " already exists.";
            msg += "\n\nOverwrite? ";
            reply = QMessageBox::question(Setup_Tabs, tr("Species Exists"), tr(msg.toLatin1()),
                                          QMessageBox::No|QMessageBox::Yes,
                                          QMessageBox::Yes);
            if (reply == QMessageBox::No) {
               continue;
            }
        }

        // Check if user updated Species name and then propagate name change through all tables.
        renameSpeName = false;
        fields = {"SpeIndex","SpeName"};
        queryStr = "SELECT SpeIndex,SpeName FROM Species WHERE SpeIndex = " + SpeciesIndex;
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["SpeName"].size() == 1) {
            newSpeName = Setup_Tab3_SpeciesTW->item(i,Column_SpeName)->text();
            existingSpeName = QString::fromStdString(dataMap["SpeName"][0]);
            renameSpeName = (newSpeName != existingSpeName);
        }
        if (renameSpeName) {
            renameSpecies(newSpeName.toStdString(),existingSpeName.toStdString());
        }

        // Update the Species table
        cmd  = "INSERT INTO Species ";
        cmd += "(SpeIndex,SpeName,MaxAge,MinCatAge,MaxCatAge,FirstYear,LastYear,PlusClass,";
        cmd += "CatchUnits,WtTypeIndex,WtUnits,SizeTypeIndex,SizeUnits,FixedMaturity) values ";
        cmd += "("   + SpeciesIndex + ", " +
                "\"" + Setup_Tab3_SpeciesTW->item(i,Column_SpeName)->text().toStdString() + "\", " +
                Setup_Tab3_SpeciesTW->item(i,Column_SpeMaxAge)->text().toStdString() + ", " +
                Setup_Tab3_SpeciesTW->item(i,Column_SpeMinCatchAge)->text().toStdString() + ", " +
                Setup_Tab3_SpeciesTW->item(i,Column_SpeMaxCatchAge)->text().toStdString() + ", " +
                Setup_Tab3_SpeciesTW->item(i,Column_SpeFirstYear)->text().toStdString() + ", " +
                Setup_Tab3_SpeciesTW->item(i,Column_SpeLastYear)->text().toStdString() + ", " +
                std::to_string(cbPlusClass->isChecked()) + ", " +
                CatchUnits + ", " +
                std::to_string(cmbWtAtAge->currentIndex()) + ", " +
                WtUnits + ", " +
                std::to_string(cmbSzAtAge->currentIndex()) + ", " +
                SizeUnits + ", " +
                std::to_string(cbFixedMaturity->isChecked()) + ") ";
        cmd += "ON DUPLICATE KEY UPDATE ";
        cmd += "SpeName=values(SpeName), MaxAge=values(MaxAge), MinCatAge=values(MinCatAge), MaxCatAge=values(MaxCatAge), ";
        cmd += "FirstYear=values(FirstYear), LastYear=values(LastYear), PlusClass=values(PlusClass), CatchUnits=values(CatchUnits), ";
        cmd += "WtTypeIndex=values(WtTypeIndex), WtUnits=values(WtUnits), SizeTypeIndex=values(SizeTypeIndex), SizeUnits=values(SizeUnits), ";
        cmd += "FixedMaturity=values(FixedMaturity); ";
//std::cout << cmd << std::endl;
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if (errorMsg != " ") {
            std::cout << cmd << std::endl;
            nmfUtils::printError("Setup: INSERT INTO Species...", errorMsg);
            insertedSpeciesOK = false;
        }

    } // end for i


    // Only delete extraneous records if there were no problems with any of the inserted ones.
    if (insertedSpeciesOK) {
        for (const std::string IndexInTable: SpeciesSet) {
            // If there's an Index in the database table that isn't in the GUI table,
            // then delete it from the database table.  It means the user doesn't want it anymore.
            if (SpeciesIndexesToKeep.find(IndexInTable) == SpeciesIndexesToKeep.end()) {
                cmd = "DELETE FROM Species where SpeIndex = " + IndexInTable;
//std::cout << cmd << std::endl;
                errorMsg = databasePtr->nmfUpdateDatabase(cmd);
                if (errorMsg != " ") {
                    std::cout << cmd << std::endl;
                    nmfUtils::printError("Setup: DELETE FROM Species...", errorMsg);
                } // end if
            } // end if
        } // end for IndexInTable
    } // end if



} // end UpdateSpeciesTable



void
nmfSetup_Tab3::UpdateOtherPredatorsTable(int NumOtherPredators)
{
    QWidget   *w;
    QCheckBox *cbSizeStruc;
    QComboBox *cmbLengthUnits;
    QComboBox *cmbWtUnits;
    QComboBox *cmbBMUnits;
    QString BMUnitsStr;
    QString WtUnitsStr;
    QString LengthUnitsStr;
    QString msg;
    QString qcmd;
    std::string errorMsg;
    std::string cmd;
    std::string WtUnits="";
    std::string BMUnits="";
    std::string LengthUnits="";
    std::string OtherPredatorsIndex="";
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    std::set<std::string> OtherPredatorsSet;
    QMessageBox::StandardButton reply;
    std::set<std::string> OtherPredatorsIndexesToKeep;
    QString newSpeName;
    QString existingSpeName;
    bool renameSpeName;

    OtherPredatorsIndexesToKeep.clear();

    // Check if OtherPredSpecies table exists and if it doesn't create it.
    qcmd  = "CREATE TABLE IF NOT EXISTS OtherPredSpecies ";
    qcmd += "(SpeIndex      int(11) NOT NULL,";
    qcmd += " SpeName       VARCHAR(255),";
    qcmd += " FirstYear     int(11),";
    qcmd += " LastYear      int(11),";
    qcmd += " BMUnitIndex   int(11),";
    qcmd += " BMUnits       float,";
    qcmd += " WtUnitIndex   int(11),";
    qcmd += " WtUnits       float,";
    qcmd += " SizeUnitIndex int(11),";
    qcmd += " SizeUnits     float,";
    qcmd += " MinSize       int(11),";
    qcmd += " MaxSize       int(11),";
    qcmd += " SizeStruc     int(11),";
    qcmd += " NumSizeCats   int(11),";
    qcmd += " PRIMARY KEY (SpeIndex))";
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (errorMsg != " ") {
        nmfUtils::printError("Setup Save(3): Create table error: ", errorMsg);
    }

    // Find all OtherPredators indexes
    fields = {"SpeIndex"};
    queryStr = "SELECT SpeIndex FROM OtherPredSpecies";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
        OtherPredatorsSet.insert(dataMap["SpeIndex"][i]);
    }

    // Now save the OtherPredators table, no need for a CSV file since it's
    // completely generated via the program.
    bool insertedOtherPredatorsOK = true;
    for (int i=0; i<NumOtherPredators; ++i) {

        // Find SizeStruc
        w = Setup_Tab3_OtherPredatorsTW->cellWidget(i,Column_OthSizeStruc);
        cbSizeStruc = w->findChild<QCheckBox *>();

        // Find BM (Biomass) Index and Weight Units
        cmbBMUnits = qobject_cast<QComboBox *>(Setup_Tab3_OtherPredatorsTW->cellWidget(i,Column_OthBMUnits));
        BMUnitsStr = cmbBMUnits->currentText();
        if (BMUnitsStr == "Kilograms")
            BMUnits = "0.001";
        else if (BMUnitsStr == "Metric Tons (000 kg)")
            BMUnits = "1";
        else if (BMUnitsStr == "000 Metric Tons")
            BMUnits = "1000";

        // Find Length Units
        cmbLengthUnits = qobject_cast<QComboBox *>(Setup_Tab3_OtherPredatorsTW->cellWidget(i,Column_OthSizeUnits));
        LengthUnitsStr = cmbLengthUnits->currentText();
        if (LengthUnitsStr == "Millimeters")
            LengthUnits = "0.1";
        else if (LengthUnitsStr == "Centimeters")
            LengthUnits = "1";
        else if (LengthUnitsStr == "Inches")
            LengthUnits = "1000";

        // Find Weight Index and Weight Units
        cmbWtUnits = qobject_cast<QComboBox *>(Setup_Tab3_OtherPredatorsTW->cellWidget(i,Column_OthWtUnits));
        WtUnitsStr = cmbWtUnits->currentText();
        if (WtUnitsStr == "Grams")
            WtUnits = "0.001";
        else if (WtUnitsStr == "Kilograms")
            WtUnits = "1";
        else if (WtUnitsStr == "Pounds")
            WtUnits = "2.2";

        // Get OtherPredators Index
        OtherPredatorsIndex = Setup_Tab3_OtherPredatorsTW->item(i,Column_OthIndex)->text().toStdString();
        OtherPredatorsIndexesToKeep.insert(OtherPredatorsIndex);
        if (OtherPredatorsSet.find(OtherPredatorsIndex) != OtherPredatorsSet.end()) { // then found the item
            msg = "\nOther Predators Species with SpeIndex of " + QString::fromStdString(OtherPredatorsIndex) + " already exists.";
            msg += "\n\nOverwrite? ";
            reply = QMessageBox::question(Setup_Tabs,
                                          tr("Other Predators Species Exists"),
                                          tr(msg.toLatin1()),
                                          QMessageBox::No|QMessageBox::Yes,
                                          QMessageBox::Yes);
            if (reply == QMessageBox::No) {
               continue;
            }
        }

        // Check if user updated Species name and then propagate name change through all tables.
        renameSpeName = false;
        fields = {"SpeIndex","SpeName"};
        queryStr = "SELECT SpeIndex,SpeName FROM OtherPredSpecies WHERE SpeIndex = " + OtherPredatorsIndex;
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["SpeName"].size() == 1) {
            newSpeName = Setup_Tab3_OtherPredatorsTW->item(i,Column_SpeName)->text();
            existingSpeName = QString::fromStdString(dataMap["SpeName"][0]);
            renameSpeName = (newSpeName != existingSpeName);
        }
        if (renameSpeName) {
            renameSpecies(newSpeName.toStdString(),existingSpeName.toStdString());
        }

        // Update the OtherPredators table
        cmd  = "INSERT INTO OtherPredSpecies ";
        cmd += "(SpeIndex,SpeName,FirstYear,LastYear,BMUnitIndex,BMUnits,";
        cmd += "WtUnitIndex,WtUnits,SizeUnitIndex,SizeUnits,MinSize,MaxSize,SizeStruc,NumSizeCats) values ";
        cmd += "("   + OtherPredatorsIndex + ", " +
                "\"" + Setup_Tab3_OtherPredatorsTW->item(i,Column_OthName)->text().toStdString() + "\", " +
                Setup_Tab3_OtherPredatorsTW->item(i,Column_OthFirstYear)->text().toStdString() + ", " +
                Setup_Tab3_OtherPredatorsTW->item(i,Column_OthLastYear)->text().toStdString() + ", " +
                std::to_string(cmbBMUnits->currentIndex()) + ", " +
                BMUnits + ", " +
                std::to_string(cmbWtUnits->currentIndex()) + ", " +
                WtUnits + ", " +
                std::to_string(cmbLengthUnits->currentIndex()) + ", " +
                LengthUnits + ", " +
                Setup_Tab3_OtherPredatorsTW->item(i,Column_OthMinSize)->text().toStdString() + ", " +
                Setup_Tab3_OtherPredatorsTW->item(i,Column_OthMaxSize)->text().toStdString() + ", " +
                std::to_string(cbSizeStruc->isChecked()) + ", " +
                Setup_Tab3_OtherPredatorsTW->item(i,Column_OthNumSizeCats)->text().toStdString() + ") ";

        cmd += "ON DUPLICATE KEY UPDATE ";
        cmd += "SpeName=values(SpeName), FirstYear=values(FirstYear), LastYear=values(LastYear), ";
        cmd += "BMUnitIndex=values(BMUnitIndex), BMUnits=values(BMUnits), ";
        cmd += "WtUnitIndex=values(WtUnitIndex), WtUnits=values(WtUnits), ";
        cmd += "SizeUnitIndex=values(SizeUnitIndex), SizeUnits=values(SizeUnits), ";
        cmd += "MinSize=values(MinSize), MaxSize=values(MaxSize), ";
        cmd += "SizeStruc=values(SizeStruc), NumSizeCats=values(NumSizeCats); ";
//std::cout << cmd << std::endl;
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if (errorMsg != " ") {
            std::cout << cmd << std::endl;
            nmfUtils::printError("Setup: INSERT INTO OtherPredSpecies...", errorMsg);
            insertedOtherPredatorsOK = false;
        }

    } // end for i

    // Only delete extraneous records if there were no problems with any of the inserted ones.
    if (insertedOtherPredatorsOK) {
        for (const std::string IndexInTable: OtherPredatorsSet) {
            // If there's an Index in the database table that isn't in the GUI table,
            // then delete it from the database table.  It means the user doesn't want it anymore.
            if (OtherPredatorsIndexesToKeep.find(IndexInTable) == OtherPredatorsIndexesToKeep.end()) {
                cmd = "DELETE FROM OtherPredSpecies where SpeIndex = " + IndexInTable;
                errorMsg = databasePtr->nmfUpdateDatabase(cmd);
                if (errorMsg != " ") {
                    std::cout << cmd << std::endl;
                    nmfUtils::printError("Setup: DELETE FROM OtherPredSpecies...", errorMsg);
                } // end if
            } // end if
        } // end for IndexInTable
    } // end if

} // end UpdateOtherPredatorsTable


void
nmfSetup_Tab3::callback_Setup_Tab3_LoadPB(bool unused)
{
   int csvFileRow = 0;
   QString line;
   QStringList fields;
   QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
   QString SpeciesCSVFile;
   QString OtherPredSpeciesCSVFile;
   QStringList NameFilters = QStringList() << "*.csv" << "*.*";
   std::vector<QString> csvIndex;
   std::vector<QString> csvName;
   std::vector<QString> csvMaxAge;
   std::vector<QString> csvMinCatAge;
   std::vector<QString> csvMaxCatAge;
   std::vector<QString> csvFirstYear;
   std::vector<QString> csvLastYear;
   std::vector<QString> csvPlusClass;
   std::vector<QString> csvCatchUnits;
   std::vector<QString> csvWtTypeIndex;
   std::vector<QString> csvWtUnits;
   std::vector<QString> csvSizeTypeIndex;
   std::vector<QString> csvSizeUnits;
   std::vector<QString> csvFixedMaturity;
   std::vector<QString> csvBMUnitIndex;
   std::vector<QString> csvBMUnits;
   std::vector<QString> csvWtUnitIndex;
   std::vector<QString> csvSizeUnitIndex;
   std::vector<QString> csvMinSize;
   std::vector<QString> csvMaxSize;
   std::vector<QString> csvSizeStruc;
   std::vector<QString> csvNumSizeCats;
   QTableWidgetItem *item;
   QCheckBox *cbox;
   QComboBox *cmbox;
   QWidget *w;
   int CatchUnitIndex=0;
   int WtTypeIndex=0;
   int WtUnitIndex=0;
   int SzTypeIndex=0;
   int SizeUnitIndex=0;
   int BMUnitIndex=0;
   QString CatchUnits;
   QString WtUnits;
   QString SizeUnits;
   QFileDialog *fileDlgSpecies;
   QFileDialog *fileDlgOtherPredSpecies;
   bool SpeciesChecked = true;

   if (Setup_Tab3_SpeciesCB->isChecked()) {

       fileDlgSpecies = new QFileDialog(Setup_Tabs);

       // Setup Load dialog
       fileDlgSpecies->setDirectory(path);
       fileDlgSpecies->selectFile("Species.csv");
       fileDlgSpecies->setNameFilters(NameFilters);
       fileDlgSpecies->setWindowTitle("Load Species CSV File");
       if (fileDlgSpecies->exec()) {

           // Open the file here....
           SpeciesCSVFile = fileDlgSpecies->selectedFiles()[0];
           QFile file(SpeciesCSVFile);
           if(! file.open(QIODevice::ReadOnly)) {
               QMessageBox::information(Setup_Tabs, "File Read Error", file.errorString());
               return;
           }
           QTextStream inStream(&file);

           // Store data to load in next step
           while (! inStream.atEnd()) {
               line = inStream.readLine();
               if (csvFileRow > 0) { // skip row=0 as it's the header
                   fields = line.split(",");
                   csvIndex.push_back(         fields[0].trimmed());
                   csvName.push_back(          fields[1].trimmed());
                   csvMaxAge.push_back(        fields[2].trimmed());
                   csvMinCatAge.push_back(     fields[3].trimmed());
                   csvMaxCatAge.push_back(     fields[4].trimmed());
                   csvFirstYear.push_back(     fields[5].trimmed());
                   csvLastYear.push_back(      fields[6].trimmed());
                   csvPlusClass.push_back(     fields[7].trimmed());
                   csvCatchUnits.push_back(    fields[8].trimmed());
                   csvWtTypeIndex.push_back(   fields[9].trimmed());
                   csvWtUnits.push_back(       fields[10].trimmed());
                   csvSizeTypeIndex.push_back( fields[11].trimmed());
                   csvSizeUnits.push_back(     fields[12].trimmed());
                   csvFixedMaturity.push_back( fields[13].trimmed());
               }
               ++csvFileRow;
           }
           file.close();

           // Set up table to accept the Species in the CSV file.
           int numRows = csvIndex.size();
           int numCols = 14;
           callback_Setup_Tab3_NumSpecies(numRows);
           Setup_Tab3_SpeciesTW->blockSignals(true);

           for (int i=0; i<numRows; ++i) {
               PopulateARowSpecies(i,numCols);
               item = Setup_Tab3_SpeciesTW->item(i,Column_SpeIndex);
               item->setText(csvIndex[i]);
               item = Setup_Tab3_SpeciesTW->item(i,Column_SpeName);
               item->setText(csvName[i]);
               item = Setup_Tab3_SpeciesTW->item(i,Column_SpeMaxAge);
               item->setText(csvMaxAge[i]);
               item = Setup_Tab3_SpeciesTW->item(i,Column_SpeMinCatchAge);
               item->setText(csvMinCatAge[i]);
               item = Setup_Tab3_SpeciesTW->item(i,Column_SpeMaxCatchAge);
               item->setText(csvMaxCatAge[i]);
               item = Setup_Tab3_SpeciesTW->item(i,Column_SpeFirstYear);
               item->setText(csvFirstYear[i]);
               item = Setup_Tab3_SpeciesTW->item(i,Column_SpeLastYear);
               item->setText(csvLastYear[i]);

               // Plus class
               w = Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpePlusClass);
               cbox = w->findChild<QCheckBox *>();
               cbox->setChecked(csvPlusClass[i] == "1");

               // Fixed Maturity
               w = Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeFixedMaturity);
               cbox = w->findChild<QCheckBox *>();
               cbox->setChecked(csvFixedMaturity[i] == "1");

               // Set the 5 combobox fields
               CatchUnits = csvCatchUnits[i];
               if (CatchUnits == "0.1")
                   CatchUnitIndex = 0;
               else if (CatchUnits == "1")
                   CatchUnitIndex = 1;
               else if (CatchUnits == "1000")
                   CatchUnitIndex = 2;
               cmbox = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeCatchUnits));
               cmbox->setCurrentIndex(CatchUnitIndex);

               WtTypeIndex = csvWtTypeIndex[i].toInt();
               cmbox = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeWtAtAgeData));
               cmbox->setCurrentIndex(WtTypeIndex);

               WtUnits = csvWtUnits[i];
               if (WtUnits == "0.001")    // Grams
                   WtUnitIndex = 0;
               else if (WtUnits == "1")   // Kilograms
                   WtUnitIndex = 1;
               else if (WtUnits == "2.2") // Pounds
                   WtUnitIndex = 2;
               cmbox = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeWtUnits));
               cmbox->setCurrentIndex(WtUnitIndex);

               SzTypeIndex = csvSizeTypeIndex[i].toInt();
               cmbox = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeSzAtAgeData));
               cmbox->setCurrentIndex(SzTypeIndex);

               SizeUnits = csvSizeUnits[i];
               if (SizeUnits == "0.1")
                   SizeUnitIndex = 0;
               else if (SizeUnits == "1")
                   SizeUnitIndex = 1;
               else if (SizeUnits == "2.54")
                   SizeUnitIndex = 2;
               cmbox = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeSzUnits));
               cmbox->setCurrentIndex(SizeUnitIndex);
           }
           Setup_Tab3_SpeciesTW->blockSignals(false);
           Setup_Tab3_SpeciesTW->resizeColumnsToContents();
       }
       fileDlgSpecies->close();
       fileDlgSpecies->deleteLater();

   } else {
       SpeciesChecked = false;
   }

   if (Setup_Tab3_OtherPredatorsCB->isChecked()) {

       fileDlgOtherPredSpecies = new QFileDialog(Setup_Tabs);

       // Setup Load dialog
       fileDlgOtherPredSpecies->setDirectory(path);
       fileDlgOtherPredSpecies->selectFile("OtherPredSpecies.csv");
       fileDlgOtherPredSpecies->setNameFilters(NameFilters);
       fileDlgOtherPredSpecies->setWindowTitle("Load the Other Predator Species CSV File");
       if (fileDlgOtherPredSpecies->exec()) {

           // Open the file here....
           OtherPredSpeciesCSVFile = fileDlgOtherPredSpecies->selectedFiles()[0];
           QFile file(OtherPredSpeciesCSVFile);
           if(! file.open(QIODevice::ReadOnly)) {
               QMessageBox::information(Setup_Tabs, "File Read Error", file.errorString());
               return;
           }
           QTextStream inStream(&file);

           // Store data to load in next step
           csvIndex.clear();
           csvName.clear();
           csvFirstYear.clear();
           csvLastYear.clear();
           csvWtUnits.clear();
           csvSizeUnits.clear();
           csvFileRow = 0;
           while (! inStream.atEnd()) {
               line = inStream.readLine();
               if (csvFileRow > 0) { // skip row=0 as it's the header
                   fields = line.split(",");
                   csvIndex.push_back(         fields[0].trimmed());
                   csvName.push_back(          fields[1].trimmed());
                   csvFirstYear.push_back(     fields[2].trimmed());
                   csvLastYear.push_back(      fields[3].trimmed());
                   csvBMUnitIndex.push_back(   fields[4].trimmed());
                   csvBMUnits.push_back(       fields[5].trimmed());
                   csvWtUnitIndex.push_back(   fields[6].trimmed());
                   csvWtUnits.push_back(       fields[7].trimmed());
                   csvSizeUnitIndex.push_back( fields[8].trimmed());
                   csvSizeUnits.push_back(     fields[9].trimmed());
                   csvMinSize.push_back(       fields[10].trimmed());
                   csvMaxSize.push_back(       fields[11].trimmed());
                   csvSizeStruc.push_back(     fields[12].trimmed());
                   csvNumSizeCats.push_back(   fields[13].trimmed());
               }
               ++csvFileRow;
           }
           file.close();

           // Set up table to accept the Species in the CSV file.
           int numRows = csvIndex.size();
           int numCols = 14;
           callback_Setup_Tab3_NumOtherPredators(numRows);
           Setup_Tab3_OtherPredatorsTW->blockSignals(true);

           for (int i=0; i<numRows; ++i) {
               PopulateARowOtherPredators(i,numCols);
               item = Setup_Tab3_OtherPredatorsTW->item(i,Column_OthIndex);
               item->setText(csvIndex[i]);
               item = Setup_Tab3_OtherPredatorsTW->item(i,Column_OthName);
               item->setText(csvName[i]);
               item = Setup_Tab3_OtherPredatorsTW->item(i,Column_OthFirstYear);
               item->setText(csvFirstYear[i]);
               item = Setup_Tab3_OtherPredatorsTW->item(i,Column_OthLastYear);
               item->setText(csvLastYear[i]);
               item = Setup_Tab3_OtherPredatorsTW->item(i,Column_OthMinSize);
               item->setText(csvMinSize[i]);
               item = Setup_Tab3_OtherPredatorsTW->item(i,Column_OthMaxSize);
               item->setText(csvMaxSize[i]);
               item = Setup_Tab3_OtherPredatorsTW->item(i,Column_OthNumSizeCats);
               item->setText(csvNumSizeCats[i]);

               // Include size struct
               w = Setup_Tab3_OtherPredatorsTW->cellWidget(i,Column_OthSizeStruc);
               cbox = w->findChild<QCheckBox *>();
               cbox->setChecked(csvSizeStruc[i] == "1");

               // Set the 3 combobox fields
               SizeUnitIndex = csvSizeUnitIndex[i].toInt();
               cmbox = qobject_cast<QComboBox *>(Setup_Tab3_OtherPredatorsTW->cellWidget(i,Column_OthSizeUnits));
               cmbox->setCurrentIndex(SizeUnitIndex);

               BMUnitIndex = csvBMUnitIndex[i].toInt();
               cmbox = qobject_cast<QComboBox *>(Setup_Tab3_OtherPredatorsTW->cellWidget(i,Column_OthBMUnits));
               cmbox->setCurrentIndex(BMUnitIndex);

               WtUnitIndex = csvWtUnitIndex[i].toInt();
               cmbox = qobject_cast<QComboBox *>(Setup_Tab3_OtherPredatorsTW->cellWidget(i,Column_OthWtUnits));
               cmbox->setCurrentIndex(WtUnitIndex);
           }
           Setup_Tab3_OtherPredatorsTW->blockSignals(false);
           Setup_Tab3_OtherPredatorsTW->resizeColumnsToContents();
       }
       fileDlgOtherPredSpecies->close();
       fileDlgOtherPredSpecies->deleteLater();

   } else {
       if (! SpeciesChecked) {
           QMessageBox::information(Setup_Tabs,
                                    tr("Nothing to Load"),
                                    tr("\nPlease check whatever you'd like to load."),
                                    QMessageBox::Ok);
       }
   }

   Setup_Tab3_LoadPB->clearFocus();

} // end callback_Setup_Tab3_LoadPB

void
nmfSetup_Tab3::callback_Setup_Tab3_SavePB(bool unused)
{
    bool saveOK = false;
    int NumSpecies        = Setup_Tab3_SpeciesTW->rowCount();
    int NumOtherPredators = Setup_Tab3_OtherPredatorsTW->rowCount();

    // Disable spin boxes if there are already species there.
    Setup_Tab3_NumSpeciesSB->setEnabled(NumSpecies == 0);
    Setup_Tab3_NumOtherPredatorsSB->setEnabled(NumOtherPredators == 0);

    // Check both Species and Other Predators before allow user to save either.
    if (Setup_Tab3_SpeciesCB->isChecked()) {
        if (! SpeciesFieldsOK(NumSpecies))
            return;
    }
    if (Setup_Tab3_OtherPredatorsCB->isChecked()) {
        if (! OtherPredatorsFieldsOK(NumOtherPredators))
            return;
    }

    // Only check and save Species table if it's been enabled.
    if (Setup_Tab3_SpeciesCB->isChecked()) {
        UpdateSpeciesTable(NumSpecies);
        saveOK = true;
    }

    // Only check and save OtherPredators table if it's been enabled.
    if (Setup_Tab3_OtherPredatorsCB->isChecked()) {
        UpdateOtherPredatorsTable(NumOtherPredators);
        saveOK = true;
    }

    Setup_Tab3_CreateTemplatesPB->setEnabled(true);
    //Setup_Tab3_ContinuePB->setEnabled(false);

    restoreCSVFromDatabase(databasePtr);

    if (NumSpecies > 0) {
        emit EnableNavigator();
    }

    if (saveOK) {
        QMessageBox::information(Setup_Tabs, "Save",
                                 tr("\nInitial species data saved.\n"));
    }
} // end callback_Setup_Tab3_SavePB


void
nmfSetup_Tab3::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

std::cout << "ProjectDir: " << ProjectDir << std::endl;


    TableName = "Species";
    fields    = {"SpeIndex","SpeName","MaxAge","MinCatAge","MaxCatAge",
                 "FirstYear","LastYear","PlusClass","CatchUnits","WtTypeIndex",
                 "WtUnits","SizeTypeIndex","SizeUnits","FixedMaturity"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = "OtherPredSpecies";
    fields    = {"SpeIndex","SpeName","FirstYear","LastYear",
                 "BMUnitIndex","BMUnits","WtUnitIndex","WtUnits",
                 "SizeUnitIndex","SizeUnits","MinSize","MaxSize","SizeStruc","NumSizeCats"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase


void
nmfSetup_Tab3::callback_Setup_Tab3_DelOtherPredatorsPB(bool unused)
{
    int NumOtherPredators = Setup_Tab3_NumOtherPredatorsSB->value();

    // Find number of selected rows
    QItemSelectionModel *selections = Setup_Tab3_OtherPredatorsTW->selectionModel();
    QModelIndexList selected = selections->selectedRows(0);
    qSort(selected);
    int NumSelectedRows = selected.size();

    // Iterate through the rows and delete each one...starting with last one first.
    for (int i=0; i<NumSelectedRows; ++i) {
        Setup_Tab3_OtherPredatorsTW->removeRow(selected[NumSelectedRows-1-i].row());
    }

    // Update the number of rows value in the spin box
    Setup_Tab3_NumOtherPredatorsSB->blockSignals(true);
    Setup_Tab3_NumOtherPredatorsSB->setValue(NumOtherPredators-NumSelectedRows);
    Setup_Tab3_NumOtherPredatorsSB->blockSignals(false);

} // end callback_Setup_Tab3_DelOtherPredatorsPB


void
nmfSetup_Tab3::callback_Setup_Tab3_OtherPredatorsChanged(QTableWidgetItem *item)
{
    int val;
    bool ok;

    disconnect(Setup_Tab3_OtherPredatorsTW, SIGNAL(itemChanged(QTableWidgetItem *)),
               this,                        SLOT(callback_Setup_Tab3_OtherPredatorsChanged(QTableWidgetItem *)));

    switch (item->column()) {
        case Column_OthIndex:
            val = item->text().toInt(&ok,10);
            if ((! ok) || (val <= 0)) {
                QMessageBox::warning(Setup_Tabs,tr("Invalid Index"),
                                     tr("\nPlease enter a valid integer Predator index value > 0."),
                                     QMessageBox::Ok);
                item->setText("");
            }
            break;
        case Column_OthFirstYear:
            val = item->text().toInt(&ok,10);
            if ((! ok) || (val <= 0)) {
                QMessageBox::warning(Setup_Tabs,tr("Invalid First Year"),
                                     tr("\nPlease enter a valid integer First Year value > 0."),
                                     QMessageBox::Ok);
                item->setText("");
            }
            break;
        case Column_OthLastYear:
            val = item->text().toInt(&ok,10);
            if ((! ok) || (val <= 0)) {
                QMessageBox::warning(Setup_Tabs,tr("Invalid Last Year"),
                                     tr("\nPlease enter a valid integer Last Year value > 0."),
                                     QMessageBox::Ok);
                item->setText("");
            }
            break;
        case Column_OthMinSize:
            val = item->text().toInt(&ok,10);
            if ((! ok) || (val < 0)) {
                QMessageBox::warning(Setup_Tabs,tr("Invalid Min Length"),
                                     tr("\nPlease enter a valid integer Min Length value >= 0."),
                                     QMessageBox::Ok);
                item->setText("");
            }
            break;
        case Column_OthMaxSize:
            val = item->text().toInt(&ok,10);
            if ((! ok) || (val < 0)) {
                QMessageBox::warning(Setup_Tabs,tr("Invalid Max Length"),
                                     tr("\nPlease enter a valid integer Max Length value >= 0."),
                                     QMessageBox::Ok);
                item->setText("");
            }
            break;
        case Column_OthNumSizeCats:
            val = item->text().toInt(&ok,10);
            if ((! ok) || (val < 0)) {
                QMessageBox::warning(Setup_Tabs,tr("Invalid Num Size Categories"),
                                     tr("\nPlease enter a valid integer Num Size Categories value >= 0."),
                                     QMessageBox::Ok);
                item->setText("");
            }
            break;
        default:
            break;

    } // end switch



    connect(Setup_Tab3_OtherPredatorsTW, SIGNAL(itemChanged(QTableWidgetItem *)),
            this,                        SLOT(callback_Setup_Tab3_OtherPredatorsChanged(QTableWidgetItem *)));
    Setup_Tab3_OtherPredatorsTW->resizeColumnsToContents();

} // end callback_Setup_Tab3_OtherPredatorsChanged



void
nmfSetup_Tab3::PopulateARowSpecies(int row, int ncols)
{
    QWidget *widg;
    QHBoxLayout *layt;
    QComboBox *cbox;
    QCheckBox *chbox;
    QTableWidgetItem *item;

    QStringList CatchUnitsValues = QStringList() << tr("Hundreds of Fish")
                                                 << tr("Thousands of Fish")
                                                 << tr("Millions of Fish");
    QStringList WtAtAgeValues = QStringList() << tr("Fixed Von-B Coefficients")
                                              << tr("Variable Von-B Coefficients")
                                              << tr("Fixed W-L Regression")
                                              << tr("Variable W-L Regression")
                                              << tr("Fixed Weight at Age")
                                              << tr("Variable Weight at Age");
    QStringList WtUnitsValues = QStringList() << tr("Grams")
                                              << tr("Kilograms")
                                              << tr("Pounds");
    QStringList SzAtAgeValues = QStringList() << tr("Fixed Von-B Coefficients")
                                              << tr("Variable Von-B Coefficients")
                                              << tr("Fixed Size at Age")
                                              << tr("Variable Size at Age");
    QStringList SzUnitsValues = QStringList() << tr("Millimeters")
                                              << tr("Centimeters")
                                              << tr("Inches");


    for (int j=0;j<ncols; ++j) {
        switch (j) {
        case Column_SpePlusClass:
            chbox = new QCheckBox();
            widg  = new QWidget();
            layt  = new QHBoxLayout();
            layt->setAlignment(Qt::AlignCenter);
            layt->addWidget(chbox);
            widg->setLayout(layt);
            Setup_Tab3_SpeciesTW->setCellWidget(row,j,widg);
            break;
        case Column_SpeCatchUnits:
            cbox = new QComboBox();
            cbox->addItems(CatchUnitsValues);
            Setup_Tab3_SpeciesTW->setCellWidget(row,j,cbox);
            break;
        case Column_SpeWtAtAgeData:
            cbox = new QComboBox();
            cbox->addItems(WtAtAgeValues);
            Setup_Tab3_SpeciesTW->setCellWidget(row,j,cbox);
            break;
        case Column_SpeWtUnits:
            cbox = new QComboBox();
            cbox->addItems(WtUnitsValues);
            Setup_Tab3_SpeciesTW->setCellWidget(row,j,cbox);
            break;
        case Column_SpeSzAtAgeData:
            cbox = new QComboBox();
            cbox->addItems(SzAtAgeValues);
            Setup_Tab3_SpeciesTW->setCellWidget(row,j,cbox);
            break;
        case Column_SpeSzUnits:
            cbox = new QComboBox();
            cbox->addItems(SzUnitsValues);
            Setup_Tab3_SpeciesTW->setCellWidget(row,j,cbox);
            break;
        case Column_SpeFixedMaturity:
            chbox = new QCheckBox();
            widg  = new QWidget();
            layt  = new QHBoxLayout();
            layt->setAlignment(Qt::AlignCenter);
            layt->addWidget(chbox);
            widg->setLayout(layt);
            Setup_Tab3_SpeciesTW->setCellWidget(row,j,widg);
            break;
        case Column_SpeName:
            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            Setup_Tab3_SpeciesTW->setItem(row,j,item);
            break;
        case Column_SpeFirstYear:
            item = new QTableWidgetItem();
            item->setText(Setup_Tab2_FirstYearLE->text());
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::ItemIsEnabled);
            Setup_Tab3_SpeciesTW->setItem(row,j,item);
            break;
        case Column_SpeLastYear:
            item = new QTableWidgetItem();
            item->setText(Setup_Tab2_LastYearLE->text());
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::ItemIsEnabled);
            Setup_Tab3_SpeciesTW->setItem(row,j,item);
            break;
        case Column_SpeIndex:
            item = new QTableWidgetItem();
            item->setText(QString::number(row+1));
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::ItemIsEnabled);
            Setup_Tab3_SpeciesTW->setItem(row,j,item);
            break;
        default:
            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            Setup_Tab3_SpeciesTW->setItem(row,j,item);
            break;
        }
    }

} // end PopulateARowSpecies


// RSK test the above code!!!

void
nmfSetup_Tab3::callback_Setup_Tab3_NumSpecies(int numSpecies)
{
    int ncols;
    int nrows;

    Setup_Tab3_SpeciesTW->clear();

    /*
     Table fields:

     SpeIndex,    SpeName,  MaxAge,        MinCatAge,   MaxCatAge,
     FirstYear,   LastYear, PlusClass,     CatchUnits,
     WtTypeIndex, WtUnits,  SizeTypeIndex, SizeUnits,   FixedMaturity
     */

    QStringList colLabels = {"Index", "Name", "First Year", "Last Year",
                             "Max Age", "Min Catch Age", "Max Catch Age",
                             "Plus Class", "Catch Units",
                             "Weight at Age Data", "Weight Units", "Size at Age Data", "Size Units",
                             "FixedMaturity"};


    ncols = colLabels.size();
    nrows = numSpecies;

    Setup_Tab3_SpeciesTW->setRowCount(nrows);
    Setup_Tab3_SpeciesTW->setColumnCount(ncols);

    disconnect(Setup_Tab3_SpeciesTW,      SIGNAL(itemChanged(QTableWidgetItem *)),
            this,                         SLOT(callback_Setup_Tab3_SpeciesChanged(QTableWidgetItem *)));

    // Put widgets items in each cell
    QStringList VerticalHeaderList = QStringList();
    for (int i=0; i<nrows; ++i)  {

        VerticalHeaderList << tr("    ");
        PopulateARowSpecies(i,ncols);

    }
    Setup_Tab3_SpeciesTW->setVerticalHeaderLabels(VerticalHeaderList);
    Setup_Tab3_SpeciesTW->setHorizontalHeaderLabels(colLabels);
    Setup_Tab3_SpeciesTW->resizeColumnsToContents();
    //Setup_Tab3_SpeciesTW->verticalHeader()->hide();
    connect(Setup_Tab3_SpeciesTW,         SIGNAL(itemChanged(QTableWidgetItem *)),
            this,                         SLOT(callback_Setup_Tab3_SpeciesChanged(QTableWidgetItem *)));

} // end callback_Setup_Tab3_NumSpecies


void
nmfSetup_Tab3::PopulateARowOtherPredators(int row, int ncols)
{
    QWidget *widg;
    QHBoxLayout *layt;
    QComboBox *cbox;
    QCheckBox *chbox;
    QTableWidgetItem *item;

    QStringList BMUnitsValues = QStringList() << tr("Kilograms")
                                              << tr("Metric Tons (000 kg)")
                                              << tr("000 Metric Tons");
    QStringList WtUnitsValues = QStringList() << tr("Grams")
                                              << tr("Kilograms")
                                              << tr("Pounds");
    QStringList SzUnitsValues = QStringList() << tr("Millimeters")
                                              << tr("Centimeters")
                                              << tr("Inches");

    for (int j=0;j<ncols; ++j) {
        switch (j) {
        case Column_OthSizeStruc:
            chbox = new QCheckBox();
            chbox->setToolTip("Include size structure in consumption parameters");
            chbox->setStatusTip("Include size structure in consumption parameters");
            widg  = new QWidget();
            layt  = new QHBoxLayout();
            layt->setAlignment(Qt::AlignCenter);
            layt->addWidget(chbox);
            widg->setLayout(layt);
            Setup_Tab3_OtherPredatorsTW->setCellWidget(row,j,widg);
            break;
        case Column_OthBMUnits:
            cbox = new QComboBox();
            cbox->addItems(BMUnitsValues);
            Setup_Tab3_OtherPredatorsTW->setCellWidget(row,j,cbox);
            break;
        case Column_OthSizeUnits:
            cbox = new QComboBox();
            cbox->addItems(SzUnitsValues);
            Setup_Tab3_OtherPredatorsTW->setCellWidget(row,j,cbox);
            break;
        case Column_OthWtUnits:
            cbox = new QComboBox();
            cbox->setToolTip("Units for weight per individual");
            cbox->setStatusTip("Units for weight per individual");
            cbox->addItems(WtUnitsValues);
            Setup_Tab3_OtherPredatorsTW->setCellWidget(row,j,cbox);
            break;
        case Column_OthName:
            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            Setup_Tab3_OtherPredatorsTW->setItem(row,j,item);
            break;
        case Column_OthFirstYear:
            item = new QTableWidgetItem();
            item->setText(Setup_Tab2_FirstYearLE->text());
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::ItemIsEnabled);
            Setup_Tab3_OtherPredatorsTW->setItem(row,j,item);
            break;
        case Column_OthLastYear:
            item = new QTableWidgetItem();
            item->setText(Setup_Tab2_LastYearLE->text());
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::ItemIsEnabled);
            Setup_Tab3_OtherPredatorsTW->setItem(row,j,item);
            break;
        case Column_OthIndex:
            item = new QTableWidgetItem();
            item->setText(QString::number(row+1));
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::ItemIsEnabled);
            Setup_Tab3_OtherPredatorsTW->setItem(row,j,item);
            break;
        default:
            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            Setup_Tab3_OtherPredatorsTW->setItem(row,j,item);
            break;
        } // end switch
    } // end for j

} // end PopulateARowOtherPredators


void
nmfSetup_Tab3::callback_Setup_Tab3_NumOtherPredators(int numOtherPredators)
{
    int nrows;
    int ncols;
    QStringList VerticalHeaderList = QStringList();

    Setup_Tab3_OtherPredatorsTW->clear();

    /*
     Table fields:

     SpeIndex, SpeName,     FirstYear, LastYear,      BMUnitIndex,
     BMUnits,  WtUnitIndex, WtUnits,   SizeUnitIndex, SizeUnits,
     MinSize,  MaxSize,     SizeStruc, NumSizeCats
     */

    QStringList colLabels = {"Index", "Name", "First Year", "Last Year",
                             "Min Length", "Max Length",
                             "Length Units", "BM Units", "Wt Units",
                             "Incl Size Struc", "Num Size Categories"};



    ncols = colLabels.size();
    nrows = numOtherPredators;

    Setup_Tab3_OtherPredatorsTW->setRowCount(nrows);
    Setup_Tab3_OtherPredatorsTW->setColumnCount(ncols);

    disconnect(Setup_Tab3_OtherPredatorsTW, SIGNAL(itemChanged(QTableWidgetItem *)),
               this,                        SLOT(callback_Setup_Tab3_OtherPredatorsChanged(QTableWidgetItem *)));

    // Put widgets items in each cell
    for (int i=0; i<nrows; ++i)  {
        VerticalHeaderList << tr("    ");
        PopulateARowOtherPredators(i,ncols);
    } // end for i

    Setup_Tab3_OtherPredatorsTW->setVerticalHeaderLabels(VerticalHeaderList);
    Setup_Tab3_OtherPredatorsTW->setHorizontalHeaderLabels(colLabels);
    Setup_Tab3_OtherPredatorsTW->resizeColumnsToContents();
    connect(Setup_Tab3_OtherPredatorsTW, SIGNAL(itemChanged(QTableWidgetItem *)),
            this,                        SLOT(callback_Setup_Tab3_OtherPredatorsChanged(QTableWidgetItem *)));

} // end callback_Setup_Tab3_NumOtherPreds



void
nmfSetup_Tab3::updateOutputWidget()
{
    /*
    char buf[200];

    nmfQtUtils::sendToOutputWindow(SetupOutputTE, nmfConstants::Clear);

    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "\n~~~~~~~~~~~~~~~~~~~", "");
    nmfQtUtils::sendToOutputWindow(SetupOutputTE,   "Project Information", "");
    nmfQtUtils::sendToOutputWindow(SetupOutputTE,   "~~~~~~~~~~~~~~~~~~~", "");

    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "\nProject directory: ", ProjectDir);
    nmfQtUtils::sendToOutputWindow(SetupOutputTE,   "Project database:  ", ProjectDatabase);
    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "All Databases: ", "");
    for (std::string dbName : ValidDatabases) {
        nmfQtUtils::sendToOutputWindow(SetupOutputTE, "    ", dbName);
    }
    std::string timePeriod = std::to_string(FirstYear) + " to " + std::to_string(LastYear);
    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "\nTime Period: ", timePeriod);
    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "Num Seasons: ", std::to_string(NumSeasons));

    //ageStr = "Min: " + std::to_string(MinCatchAge) + ", Max: " + std::to_string(MaxCatchAge);
    //nmfQtUtils::sendToOutputWindow(SetupOutputTE, "\nSpecies Catch Age: ", ageStr);
    sprintf(buf,"\n%-20s          %14s %14s","Species","Min Catch Age","Max Catch Age");
    nmfQtUtils::sendToOutputWindow(SetupOutputTE, std::string(buf), "");

    for (std::string species : Species) {
        sprintf(buf,"%-20s %14d %14d",species.c_str(),MinCatchAge[species],MaxCatchAge[species]);
        nmfQtUtils::sendToOutputWindow(SetupOutputTE, "    ", std::string(buf));
    }
    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "Other Predators: ", "");
    for (std::string othPred : OtherPredators) {
        nmfQtUtils::sendToOutputWindow(SetupOutputTE, "    ", othPred);
    }
    nmfQtUtils::sendToOutputWindow(SetupOutputTE, "Prey: ", "");
    for (std::string prey : Prey) {
        nmfQtUtils::sendToOutputWindow(SetupOutputTE, "    ", prey);
    }
*/
} // end updateOutputWidget


void
nmfSetup_Tab3::readSettings() {
//    // Read the settings and load into class variables.
    QSettings* settings = nmfUtilsQt::createSettings(nmfConstantsMSVPA::SettingsDirWindows,"MSVPA_X2");
    settings->beginGroup("SetupTab");
    //ProjectDir = settings.value("ProjectDir","").toString();
//    ProjectDatabase = settings.value("ProjectDatabase","").toString().toStdString();
//    FirstYear  = settings.value("FirstYear",1900).toInt();
//    LastYear   = settings.value("LastYear",2000).toInt();
//    NumSeasons = settings.value("NumSeasons",4).toInt();
//    Species.clear();
//    foreach (QVariant species, settings.value("Species").toList()) {
//        Species << species.toString().toStdString();
//    }
//    OtherPredators.clear();
//    foreach (QVariant otherPredator, settings.value("OtherPredators").toList()) {
//        OtherPredators << otherPredator.toString().toStdString();
//    }
//    Prey.clear();
//    foreach (QVariant prey, settings.value("Prey").toList()) {
//        Prey << prey.toString().toStdString();
//    }
//    settings.endGroup();
//    settings.beginGroup("SetupTab/MinCatchAge");
//    QStringList keys = settings.childKeys();
//    foreach (QString key, keys) {
//         MinCatchAge[key.toStdString()] = settings.value(key).toInt();
//    }
//    settings.endGroup();
//    settings.beginGroup("SetupTab/MaxCatchAge");
//    keys = settings.childKeys();
//    foreach (QString key, keys) {
//         MaxCatchAge[key.toStdString()] = settings.value(key).toInt();
//    }
    settings->endGroup();

    delete settings;

//    // Load class variables into appropriate widgets.
    //Setup_Tab2_ProjectDirLE->setText(ProjectDir);
//    Setup_Tab2_FirstYearLE->setText(QString::number(FirstYear));
//    Setup_Tab2_LastYearLE->setText(QString::number(LastYear));
//    Setup_Tab2_NumSeasonsLE->setText(QString::number(NumSeasons));
//    Setup_Tab2_SpeciesCMB->clear();
//    for (std::string species : Species) {
//        Setup_Tab2_SpeciesCMB->addItem(QString::fromStdString(species));
//    }
//    for (std::string otherPredator : OtherPredators) {
//        Setup_Tab2_OtherPredatorCMB->addItem(QString::fromStdString(otherPredator));
//    }
//    for (std::string prey : Prey) {
//        Setup_Tab2_PreyCMB->addItem(QString::fromStdString(prey));
//    }
//    Setup_Tab2_MinCatchAgeLE->setText(QString::number(MinCatchAge[Species[0]]));
//    Setup_Tab2_MaxCatchAgeLE->setText(QString::number(MaxCatchAge[Species[0]]));

} // end readSettings

void
nmfSetup_Tab3::saveSettings()
{
//    QSettings settings("NOAA", "MSVPA_X2");

//    settings.beginGroup("SetupTab");
//    settings.setValue("ProjectDir", ProjectDir.c_str());
//    settings.setValue("ProjectDatabase", ProjectDatabase.c_str());
//    settings.setValue("FirstYear", FirstYear);
//    settings.setValue("LastYear", LastYear);
//    settings.setValue("NumSeasons", NumSeasons);

//    QVariantList SpeciesVariant;
//    foreach (std::string species, Species) {
//        SpeciesVariant << QString::fromStdString(species);
//    }
//    settings.setValue("Species", SpeciesVariant);

//    QVariantList OtherPredatorsVariant;
//    foreach (std::string otherPredator, OtherPredators) {
//        OtherPredatorsVariant << QString::fromStdString(otherPredator);
//    }
//    settings.setValue("OtherPredators", OtherPredatorsVariant);

//    QVariantList PreyVariant;
//    foreach (std::string prey, Prey) {
//        PreyVariant << QString::fromStdString(prey);
//    }
//    settings.setValue("Prey", PreyVariant);
//    settings.endGroup();

//    settings.beginGroup("SetupTab/MinCatchAge");
//    QMap<std::string,int>::const_iterator itMin = MinCatchAge.constBegin();
//    while (itMin != MinCatchAge.constEnd()) {
//         settings.setValue(QString::fromStdString(itMin.key()), itMin.value());
//         ++itMin;
//     }
//    settings.endGroup();

//    settings.beginGroup("SetupTab/MaxCatchAge");
//    QMap<std::string,int>::const_iterator itMax = MaxCatchAge.constBegin();
//    while (itMax != MaxCatchAge.constEnd()) {
//         settings.setValue(QString::fromStdString(itMax.key()), itMax.value());
//         ++itMax;
//     }
//    settings.endGroup();

} // end saveSettings

//void
//nmfSetupTab3::callback_Forecast_Tab1_NextPage()
//{
//    Forecast_Tabs->setCurrentIndex(Forecast_Tabs->currentIndex()+1);
//}


void
nmfSetup_Tab3::LoadSpeciesTable()
{
    int NumSpecies;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    int PlusClass;
    int FixedMaturity;
    int WtTypeIndex;
    int SzTypeIndex;
    int WtUnitIndex=0;
    int CatchUnitIndex=0;
    int SizeUnitIndex=0;
    std::string WtUnits;
    std::string CatchUnits;
    std::string SizeUnits;
    QWidget   *widg;
    QCheckBox *cbPlusClass;
    QCheckBox *cbFixedMaturity;
    QComboBox *cmbWtTypeIndex;
    QComboBox *cmbSzTypeIndex;
    QComboBox *cmbWtUnits;
    QComboBox *cmbCatchUnits;
    QComboBox *cmbSzUnits;

    databasePtr->checkForTableAndCreate("Species");

    // Get Species data from database
    fields = {"SpeIndex","SpeName","MaxAge","MinCatAge","MaxCatAge","FirstYear","LastYear","PlusClass",
              "CatchUnits","WtTypeIndex","WtUnits","SizeTypeIndex","SizeUnits","FixedMaturity"};
    queryStr =  "SELECT SpeIndex,SpeName,MaxAge,MinCatAge,MaxCatAge,FirstYear,LastYear,PlusClass,";
    queryStr += "CatchUnits,WtTypeIndex,WtUnits,SizeTypeIndex,SizeUnits,FixedMaturity FROM Species";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumSpecies = dataMap["SpeIndex"].size();

    // Load up all of the Species widgets with data from the database.
    if (NumSpecies > 0) {
        Setup_Tab3_NumSpeciesSB->setValue(NumSpecies);
        for (int i=0; i<NumSpecies; ++i) {

            // Populate text fields
            Setup_Tab3_SpeciesTW->item(i,Column_SpeIndex)->setText(QString::fromStdString(dataMap["SpeIndex"][i]));
            Setup_Tab3_SpeciesTW->item(i,Column_SpeName)->setText(QString::fromStdString(dataMap["SpeName"][i]));
            Setup_Tab3_SpeciesTW->item(i,Column_SpeMaxAge)->setText(QString::fromStdString(dataMap["MaxAge"][i]));
            Setup_Tab3_SpeciesTW->item(i,Column_SpeMinCatchAge)->setText(QString::fromStdString(dataMap["MinCatAge"][i]));
            Setup_Tab3_SpeciesTW->item(i,Column_SpeMaxCatchAge)->setText(QString::fromStdString(dataMap["MaxCatAge"][i]));
            Setup_Tab3_SpeciesTW->item(i,Column_SpeFirstYear)->setText(QString::fromStdString(dataMap["FirstYear"][i]));
            Setup_Tab3_SpeciesTW->item(i,Column_SpeLastYear)->setText(QString::fromStdString(dataMap["LastYear"][i]));

            // Set the 2 checkbox fields
            PlusClass = std::stoi(dataMap["PlusClass"][i]);
            if (PlusClass) {
                widg = Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpePlusClass);
                cbPlusClass = widg->findChild<QCheckBox *>();
                cbPlusClass->setCheckState(Qt::Checked);
            }
            FixedMaturity = std::stoi(dataMap["FixedMaturity"][i]);
            if (FixedMaturity) {
                widg = Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeFixedMaturity);
                cbFixedMaturity = widg->findChild<QCheckBox *>();
                cbFixedMaturity->setCheckState(Qt::Checked);
            }

            // Set the 5 combobox fields
            CatchUnits = dataMap["CatchUnits"][i];
            if (CatchUnits == "0.1")
                CatchUnitIndex = 0;
            else if (CatchUnits == "1")
                CatchUnitIndex = 1;
            else if (CatchUnits == "1000")
                CatchUnitIndex = 2;
            cmbCatchUnits = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeCatchUnits));
            cmbCatchUnits->setCurrentIndex(CatchUnitIndex);

            WtTypeIndex = std::stoi(dataMap["WtTypeIndex"][i]);
            cmbWtTypeIndex = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeWtAtAgeData));
            cmbWtTypeIndex->setCurrentIndex(WtTypeIndex);

            WtUnits = dataMap["WtUnits"][i];
            if (WtUnits == "0.001")    // Grams
                WtUnitIndex = 0;
            else if (WtUnits == "1")   // Kilograms
                WtUnitIndex = 1;
            else if (WtUnits == "2.2") // Pounds
                WtUnitIndex = 2;
            cmbWtUnits = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeWtUnits));
            cmbWtUnits->setCurrentIndex(WtUnitIndex);

            SzTypeIndex = std::stoi(dataMap["SizeTypeIndex"][i]);
            cmbSzTypeIndex = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeSzAtAgeData));
            cmbSzTypeIndex->setCurrentIndex(SzTypeIndex);

            SizeUnits = dataMap["SizeUnits"][i];
            if (SizeUnits == "0.1")
                SizeUnitIndex = 0;
            else if (SizeUnits == "1")
                SizeUnitIndex = 1;
            else if (SizeUnits == "2.54")
                SizeUnitIndex = 2;
            cmbSzUnits = qobject_cast<QComboBox *>(Setup_Tab3_SpeciesTW->cellWidget(i,Column_SpeSzUnits));
            cmbSzUnits->setCurrentIndex(SizeUnitIndex);

        } // end for i
    } // end if

} // end LoadSpeciesTable


void
nmfSetup_Tab3::LoadOtherPredatorsTable()
{
    int NumOtherPredators;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QCheckBox *cbSizeStruc;
    QComboBox *cmbWtUnits;
    QComboBox *cmbSzUnits;
    QComboBox *cmbBMUnits;
    QWidget *widg;
    int SizeStruc;
    int WtUnitIndex;
    int SizeUnitIndex;
    int BMUnitIndex;
    std::string WtUnits;
    std::string SizeUnits;
    std::string BMUnits;

    //databasePtr->checkForTableAndCreate("OtherPredSpecies");

    // Get Other Predators data from database
    fields = {"SpeIndex","SpeName","FirstYear","LastYear","BMUnitIndex","BMUnits","WtUnitIndex","WtUnits",
              "SizeUnitIndex","SizeUnits","MinSize","MaxSize","SizeStruc","NumSizeCats"};
    queryStr  = "SELECT SpeIndex,SpeName,FirstYear,LastYear,BMUnitIndex,BMUnits,WtUnitIndex,WtUnits,";
    queryStr += "SizeUnitIndex,SizeUnits,MinSize,MaxSize,SizeStruc,NumSizeCats FROM OtherPredSpecies";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumOtherPredators = dataMap["SpeIndex"].size();

    // Load up all of the Species widgets with data from the database.
    if (NumOtherPredators > 0) {
        Setup_Tab3_NumOtherPredatorsSB->setValue(NumOtherPredators);
        for (int i=0; i<NumOtherPredators; ++i)
        {
            // Populate text fields
            Setup_Tab3_OtherPredatorsTW->item(i,Column_OthIndex)->setText(QString::fromStdString(dataMap["SpeIndex"][i]));
            Setup_Tab3_OtherPredatorsTW->item(i,Column_OthName)->setText(QString::fromStdString(dataMap["SpeName"][i]));
            Setup_Tab3_OtherPredatorsTW->item(i,Column_OthFirstYear)->setText(QString::fromStdString(dataMap["FirstYear"][i]));
            Setup_Tab3_OtherPredatorsTW->item(i,Column_OthLastYear)->setText(QString::fromStdString(dataMap["LastYear"][i]));
            Setup_Tab3_OtherPredatorsTW->item(i,Column_OthMinSize)->setText(QString::fromStdString(dataMap["MinSize"][i]));
            Setup_Tab3_OtherPredatorsTW->item(i,Column_OthMaxSize)->setText(QString::fromStdString(dataMap["MaxSize"][i]));
            Setup_Tab3_OtherPredatorsTW->item(i,Column_OthNumSizeCats)->setText(QString::fromStdString(dataMap["NumSizeCats"][i]));

            // Set the 1 checkbox field
            SizeStruc = std::stoi(dataMap["SizeStruc"][i]);
            if (SizeStruc) {
                widg = Setup_Tab3_OtherPredatorsTW->cellWidget(i,Column_OthSizeStruc);
                cbSizeStruc = widg->findChild<QCheckBox *>();
                cbSizeStruc->setCheckState(Qt::Checked);
            }

            // Set the 3 combo box fields

            SizeUnitIndex = std::stoi(dataMap["SizeUnitIndex"][i]);
//            if (SizeUnits == "0.1")
//                SizeUnitIndex = 0;
//            else if (SizeUnits == "1")
//                SizeUnitIndex = 1;
//            else if (SizeUnits == "2.54")
//                SizeUnitIndex = 2;
            cmbSzUnits = qobject_cast<QComboBox *>(Setup_Tab3_OtherPredatorsTW->cellWidget(i,Column_OthSizeUnits));
            cmbSzUnits->setCurrentIndex(SizeUnitIndex);

            BMUnitIndex = std::stoi(dataMap["BMUnitIndex"][i]);
//            if (BMUnits == "0.001") // Kilograms
//                BMUnitIndex = 0;
//            else if (BMUnits == "1") // Metric Tons
//                BMUnitIndex = 1;
//            else if (BMUnits == "1000") // 000 Metric Tons
//                BMUnitIndex = 2;
            cmbBMUnits = qobject_cast<QComboBox *>(Setup_Tab3_OtherPredatorsTW->cellWidget(i,Column_OthBMUnits));
            cmbBMUnits->setCurrentIndex(BMUnitIndex);

            WtUnitIndex = std::stoi(dataMap["WtUnitIndex"][i]);
//            if (WtUnits == "0.001")    // Grams
//                WtUnitIndex = 0;
//            else if (WtUnits == "1")   // Kilograms
//                WtUnitIndex = 1;
//            else if (WtUnits == "2.2") // Pounds
//                WtUnitIndex = 2;
            cmbWtUnits = qobject_cast<QComboBox *>(Setup_Tab3_OtherPredatorsTW->cellWidget(i,Column_OthWtUnits));
            cmbWtUnits->setCurrentIndex(WtUnitIndex);

        }
    }


} // end LoadOtherPredatorsTable


void
nmfSetup_Tab3::loadWidgets(nmfDatabase *theDatabasePtr)
{
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    int NumSpecies;
    int NumOtherPredators;

    databasePtr = theDatabasePtr;

    LoadSpeciesTable();
    LoadOtherPredatorsTable();

    // Find number of Species
    fields = {"SpeIndex"};
    queryStr =  "SELECT SpeIndex FROM Species";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumSpecies = dataMap["SpeIndex"].size();
    if (NumSpecies > 0)
        Setup_Tab3_NumSpeciesSB->setEnabled(false);
    //Setup_Tab3_NumSpeciesSB->setEnabled(NumSpecies == 0);

    // Find number of Other Predator Species
    queryStr =  "SELECT SpeIndex FROM OtherPredSpecies";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumOtherPredators = dataMap["SpeIndex"].size();
    if (NumOtherPredators > 0)
        Setup_Tab3_NumSpeciesSB->setEnabled(false);
    //Setup_Tab3_NumOtherPredatorsSB->setEnabled(NumOtherPredators == 0);

} // end loadWidgets


void
nmfSetup_Tab3::clearWidgets()
{

}

void
nmfSetup_Tab3::callback_Setup_Tab3_SpeciesCB(bool state)
{
    int NumSpecies;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;

    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM Species";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumSpecies = dataMap["SpeName"].size();


    Setup_Tab3_NumSpeciesLBL->setEnabled(state);
    //Setup_Tab3_NumSpeciesSB->setEnabled(NumSpecies == 0);

    if (state) {
        if (NumSpecies > 0)
            Setup_Tab3_NumSpeciesSB->setEnabled(false);
        else
            Setup_Tab3_NumSpeciesSB->setEnabled(true);
    } else {
        Setup_Tab3_NumSpeciesSB->setEnabled(false);
    }
    Setup_Tab3_AddSpeciesPB->setEnabled(state);
    Setup_Tab3_DelSpeciesPB->setEnabled(state);
    Setup_Tab3_ReloadSpeciesPB->setEnabled(state);
    Setup_Tab3_SpeciesTW->setEnabled(state);

} // end callback_Setup_Tab3_SpeciesCB



void
nmfSetup_Tab3::callback_Setup_Tab3_OtherPredatorsCB(bool state)
{
    int NumPredSpecies;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;

    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM OtherPredSpecies";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumPredSpecies = dataMap["SpeName"].size();

    Setup_Tab3_NumOtherPredatorsLBL->setEnabled(state);
    //Setup_Tab3_NumOtherPredatorsSB->setEnabled(NumPredSpecies == 0);
    if (state) {
        if (NumPredSpecies > 0)
            Setup_Tab3_NumOtherPredatorsSB->setEnabled(false);
        else
            Setup_Tab3_NumOtherPredatorsSB->setEnabled(true);
    } else {
        Setup_Tab3_NumOtherPredatorsSB->setEnabled(false);
    }
    Setup_Tab3_AddOtherPredatorsPB->setEnabled(state);
    Setup_Tab3_DelOtherPredatorsPB->setEnabled(state);
    Setup_Tab3_ReloadOtherPredatorsPB->setEnabled(state);
    Setup_Tab3_OtherPredatorsTW->setEnabled(state);

} // end callback_Setup_Tab3_SpeciesCB


void
nmfSetup_Tab3::callback_Setup_Tab3_CreateTemplates(bool unused)
{
    createTheTemplates(nmfConstantsMSVPA::AllTables,true);

} // end callback_Setup_Tab3_CreateTemplates

// Create CSV files
void
nmfSetup_Tab3::createTheTemplates(const std::vector<std::string> &tableVec,
                                 bool verbose)
{
    logger->logMsg(nmfConstants::Normal,"nmfSetupTab3::createTheTemplates");

    bool ok;
    bool yesToAll = false;
    bool noToAll = false;
    int NumSpecies;
    QString csvFile;
    QString path;
    std::string csvFileStr;
    std::string filename;
    std::string msg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QMessageBox::StandardButton reply;
    QString ProjectDir = Setup_Tab2_ProjectDirLE->text();

    //int FirstYear  = Setup_Tab2_FirstYearLE->text().toInt();   // RSK read these from the Species database later
    //int LastYear   = Setup_Tab2_LastYearLE->text().toInt();
    int NumSeasons = Setup_Tab2_NumSeasonsLE->text().toInt();
    QMap<std::string,int> FirstYear;
    QMap<std::string,int> LastYear;
    QMap<std::string,int> MinCatchAge;
    QMap<std::string,int> MaxCatchAge;
    QStringList Species = {};
    std::string species;
    QStringList OtherPredators = {};
    QMap<std::string,int> OtherPredatorCategories;

    OtherPredatorCategories.clear();

    // Load data structures from tables.  Start with Species data.
    fields   = {"SpeName","MinCatAge","MaxCatAge","FirstYear","LastYear"};
    queryStr = "SELECT SpeName,MinCatAge,MaxCatAge,FirstYear,LastYear FROM Species";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumSpecies = dataMap["SpeName"].size();
    for (int i=0; i<NumSpecies; ++i) {
        species = dataMap["SpeName"][i];
        Species << QString::fromStdString(species);
        MinCatchAge[species] = std::stoi(dataMap["MinCatAge"][i]);
        MaxCatchAge[species] = std::stoi(dataMap["MaxCatAge"][i]);
        FirstYear[species]   = std::stoi(dataMap["FirstYear"][i]);
        LastYear[species]    = std::stoi(dataMap["LastYear"][i]);
    }

    // Next load Other Predator data
    fields   = {"SpeName","FirstYear","LastYear","NumSizeCats"};
    queryStr = "SELECT SpeName,FirstYear,LastYear,NumSizeCats FROM OtherPredSpecies";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumSpecies = dataMap["SpeName"].size();
    for (int i=0; i<NumSpecies; ++i) {
        species = dataMap["SpeName"][i];
        OtherPredators << QString::fromStdString(species);
        FirstYear[species]   = std::stoi(dataMap["FirstYear"][i]);
        LastYear[species]    = std::stoi(dataMap["LastYear"][i]);
        OtherPredatorCategories[species] = std::stoi(dataMap["NumSizeCats"][i]);
    }

    // Make sub dir for csv files
    // Make images sub dir in the project dir if it's not already there.
    path = QDir(ProjectDir).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    if (! QDir(path).exists()) {
        ok = QDir().mkdir(path);
        if (!ok) {
            nmfUtils::printError("callback_Setup_Tab3f_CreateTemplates: Couldn't create directory: "+path.toStdString(), "");
        } else {
            logger->logMsg(nmfConstants::Normal,"Setup: Created directory: "+ path.toStdString());
        }
    }

    for (std::string tableName : tableVec)
    {
        filename = tableName+".csv";
        csvFile = QDir(path.toLatin1()).filePath(filename.c_str());
        csvFileStr = csvFile.toStdString();

        if (! QFileInfo(csvFileStr.c_str()).exists() || yesToAll) {
            buildCSVFile(csvFileStr,tableName,FirstYear,LastYear,NumSeasons,
                         MinCatchAge,MaxCatchAge,Species,OtherPredators,OtherPredatorCategories);
        } else {
            msg  = "\nThe following CSV file already exists:\n\n" + csvFile.toStdString() + "\n\n";
            msg += "OK to overwrite it?\n";
            reply = QMessageBox::YesToAll;
            if (verbose) {
                reply = QMessageBox::question(Setup_Tabs,
                                              tr("File Found"),
                                              tr(msg.c_str()),
                                              QMessageBox::NoToAll|QMessageBox::Yes|QMessageBox::YesToAll|QMessageBox::No,
                                              QMessageBox::YesToAll);
            }
            if ((reply != QMessageBox::No) && (reply != QMessageBox::NoToAll)) {
                buildCSVFile(csvFileStr,tableName,FirstYear,LastYear,NumSeasons,
                             MinCatchAge,MaxCatchAge,Species,OtherPredators,OtherPredatorCategories);
            }
            yesToAll = (reply == QMessageBox::YesToAll);
            noToAll  = (reply == QMessageBox::NoToAll);
            if (noToAll) {
                //i = nmfConstants::NumModelTypes; // to break out of outer loop
                break;
            }
        }
    }

    Setup_Tab3_CreateTemplatesPB->setEnabled(false);
    //Setup_Tab3_ContinuePB->setEnabled(true);

    if (verbose) {
        msg = "\nTemplate CSV files have been created and are located in:\n\n" + path.toStdString();
        msg += "\n\nThese files are now ready to be populated, either within MSVPA_X2 or ";
        msg += "via the user's Spreadsheet program.\n\nPlease continue to the SSVPA section ";
        msg += "in the Navigator to the left.";
        QMessageBox::information(Setup_Tabs, tr("Templates Created"),
                                 tr(msg.c_str()), QMessageBox::Ok);
    }
    emit DeactivateRunButtons(); // Disable all Run buttons (SSVPA,MSVPA,Forecast) until user loads all data

    emit CreateAllDatabaseTables();

    // Create Species and OtherPredSpecies .csv file (they weren't populated by the above calls)
    restoreCSVFromDatabase(databasePtr);


    logger->logMsg(nmfConstants::Normal,"nmfSetupTab3::createTheTemplates Complete");

} // end createTheTemplates


void
nmfSetup_Tab3::buildCSVFile(std::string &csvPathFileName,
                           std::string &tableName,
                           QMap<std::string,int> &FirstYear,
                           QMap<std::string,int> &LastYear,
                           int NumSeasons,
                           QMap<std::string,int> &MinCatchAge,
                           QMap<std::string,int> &MaxCatchAge,
                           QStringList Species,
                           QStringList OtherPredators,
                           QMap<std::string,int> &OtherPredatorCategories)
{
    // No need to load Application table.  It's used just to filter
    // the available database names in Setup Tab #2.
    if (tableName == "Application") {
        return;
    }
    if (FunctionMap.find(tableName) == FunctionMap.end()) {

        std::cout << "Warning: \"" << tableName << "\" not found in nmfSetupTab3::FunctionMap." << std::endl;

    } else {

        FunctionMap[tableName](
                //TableNames,
                csvPathFileName,
                tableName,
                FirstYear,
                LastYear,
                MinCatchAge,
                MaxCatchAge,
                Species,
                OtherPredators,
                OtherPredatorCategories
                );
    }

} // end buildCSVFile


// Table  1 of 39
void
nmfSetup_Tab3::buildCSVFileForeEnergyDens(//std::map<std::string,std::vector<std::string> > *TableNames,
                                         std::string &csvPathFileName, std::string &tableName,
                                         QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                         QMap<std::string,int> &MinCatchAge,
                                         QMap<std::string,int> &MaxCatchAge,
                                         QStringList  &Species,
                                         QStringList &OtherPredSpecies,
                                         QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, SpeName, SpeType, SpeIndex, "
            << "Age, Season, EnergyDens, AvgDietE\n";

    outFile.close();

} // end buildCSVFileForeEnergyDens

// Table  2 of 39
void
nmfSetup_Tab3::buildCSVFileForeOutput(//std::map<std::string,std::vector<std::string> > *TableNames,
                                     std::string &csvPathFileName,std::string &tableName,
                                     QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                     QMap<std::string,int> &MinCatchAge,
                                     QMap<std::string,int> &MaxCatchAge,
                                     QStringList  &Species,
                                     QStringList &OtherPredSpecies,
                                     QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, Scenario, Year, Season, SpeName, "
            << "Age, SpeType, InitAbund, EndAbund, InitBiomass, EndBiomass, "
            << "SeasM2, SeasF, SeasM1, AvgSize, AvgWeight, TotalSBM, "
            << "TotalBMConsumed, StomCont, InitWt, EndWt, SeasCatch, SeasYield\n";

    outFile.close();

} // end buildCSVFileForeOutput

// Table  3 of 39
void
nmfSetup_Tab3::buildCSVFileForePredGrowth(//std::map<std::string,std::vector<std::string> > *TableNames,
                                         std::string &csvPathFileName,std::string &tableName,
                                         QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                         QMap<std::string,int> &MinCatchAge,
                                         QMap<std::string,int> &MaxCatchAge,
                                         QStringList &Species,
                                         QStringList &OtherPredSpecies,
                                         QMap<std::string,int> &OtherPredCategories)
{

    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, PredName, PredIndex, PredType, Age, "
            << "RAlpha, RBeta, RQ, ACT, E, U, SDA, LWAlpha, LWBeta\n";

    outFile.close();

} // end buildCSVFileForePredGrowth

// Table  4 of 39
void
nmfSetup_Tab3::buildCSVFileForePredVonB(//std::map<std::string,std::vector<std::string> > *TableNames,
                                       std::string &csvPathFileName,std::string &tableName,
                                       QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                       QMap<std::string,int> &MinCatchAge,
                                       QMap<std::string,int> &MaxCatchAge,
                                       QStringList &Species,
                                       QStringList &OtherPredSpecies,
                                       QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, PredName, PredIndex, PredType, Linf, GrowthK, TZero, LWAlpha, LWBeta\n";

    outFile.close();

} // end buildCSVFileForePredVonB

// Table  5 of 39
void
nmfSetup_Tab3::buildCSVFileForeSRQ(//std::map<std::string,std::vector<std::string> > *TableNames,
                                  std::string &csvPathFileName,std::string &tableName,
                                  QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                  QMap<std::string,int> &MinCatchAge,
                                  QMap<std::string,int> &MaxCatchAge,
                                  QStringList &Species,
                                  QStringList &OtherPredSpecies,
                                  QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, SpeName, SpeIndex, Quartile, MinSSB, MaxSSB, MinRec, MaxRec, MeanRec\n";

    outFile.close();

} // end buildCSVFileForeSRQ

// Table  6 of 39
void
nmfSetup_Tab3::buildCSVFileForeSRR(//std::map<std::string,std::vector<std::string> > *TableNames,
                                  std::string &csvPathFileName,std::string &tableName,
                                  QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                  QMap<std::string,int> &MinCatchAge,
                                  QMap<std::string,int> &MaxCatchAge,
                                  QStringList &Species,
                                  QStringList &OtherPredSpecies,
                                  QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, SpeName, SpeIndex, SRRType, SRRA, SRRB, SRRK, Userdefined\n";

    outFile.close();

} // end buildCSVFileForeSRR

// Table  7 of 39
void
nmfSetup_Tab3::buildCSVFileForeSuitPreyBiomass(//std::map<std::string,std::vector<std::string> > *TableNames,
                                              std::string &csvPathFileName,std::string &tableName,
                                              QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                              QMap<std::string,int> &MinCatchAge,
                                              QMap<std::string,int> &MaxCatchAge,
                                              QStringList &Species,
                                              QStringList &OtherPredSpecies,
                                              QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, Scenario, PredName, PredAge, PreyName, PreyAge, "
            << "Year, Season, SuitPreyBiomass, PropDiet, EDens, BMConsumed, PredType\n";

    outFile.close();

} // end buildCSVFileForeSuitPreyBiomass

// Table  8 of 39
void
nmfSetup_Tab3::buildCSVFileForecasts(//std::map<std::string,std::vector<std::string> > *TableNames,
                                    std::string &csvPathFileName,std::string &tableName,
                                    QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                    QMap<std::string,int> &MinCatchAge,
                                    QMap<std::string,int> &MaxCatchAge,
                                    QStringList &Species,
                                    QStringList &OtherPredSpecies,
                                    QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, InitYear, NYears, Growth\n";

    outFile.close();

} // end buildCSVFileForecasts

// Table  9 of 39
void
nmfSetup_Tab3::buildCSVFileMSVPAEnergyDens(//std::map<std::string,std::vector<std::string> > *TableNames,
                                          std::string &csvPathFileName,std::string &tableName,
                                          QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                          QMap<std::string,int> &MinCatchAge,
                                          QMap<std::string,int> &MaxCatchAge,
                                          QStringList &Species,
                                          QStringList &OtherPredSpecies,
                                          QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, SpeName, SpeType, SpeIndex, "
            << "Age, Season, EnergyDens, AvgDietE\n";

    outFile.close();

} // end buildCSVFileMSVPAEnergyDens

// Table 10 of 39
void
nmfSetup_Tab3::buildCSVFileMSVPAOthPrey(//std::map<std::string,std::vector<std::string> > *TableNames,
                                       std::string &csvPathFileName,std::string &tableName,
                                       QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                       QMap<std::string,int> &MinCatchAge,
                                       QMap<std::string,int> &MaxCatchAge,
                                       QStringList &Species,
                                       QStringList &OtherPredSpecies,
                                       QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, OthPreyName, MinSize, MaxSize, SizeAlpha, SizeBeta\n";

    outFile.close();

} // end buildCSVFileMSVPAOthPrey

// Table 11 of 39
void
nmfSetup_Tab3::buildCSVFileMSVPAOthPreyAnn(//std::map<std::string,std::vector<std::string> > *TableNames,
                                          std::string &csvPathFileName,std::string &tableName,
                                          QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                          QMap<std::string,int> &MinCatchAge,
                                          QMap<std::string,int> &MaxCatchAge,
                                          QStringList &Species,
                                          QStringList &OtherPredSpecies,
                                          QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, OthPreyName, Year, Season, Biomass\n";

    outFile.close();

} // end buildCSVFileMSVPAOthPreyAnn

// Table 12 of 39
void
nmfSetup_Tab3::buildCSVFileMSVPASeasBiomass(//std::map<std::string,std::vector<std::string> > *TableNames,
                                           std::string &csvPathFileName,std::string &tableName,
                                           QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                           QMap<std::string,int> &MinCatchAge,
                                           QMap<std::string,int> &MaxCatchAge,
                                           QStringList &Species,
                                           QStringList &OtherPredSpecies,
                                           QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, SpeName, Season, SpeType, Year, Age, Biomass, "
            << "TotalPredCons, SeasM2, AnnAbund, SeasAbund, SeasF, "
            << "SeasM1, SeasWt, SeasSize, AnnBiomass, StomCont\n";

    outFile.close();

} // end buildCSVFileMSVPASeasBiomass

// Table 13 of 39
void
nmfSetup_Tab3::buildCSVFileMSVPASeasInfo(//std::map<std::string,std::vector<std::string> > *TableNames,
                                        std::string &csvPathFileName,std::string &tableName,
                                        QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                        QMap<std::string,int> &MinCatchAge,
                                        QMap<std::string,int> &MaxCatchAge,
                                        QStringList &Species,
                                        QStringList &OtherPredSpecies,
                                        QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, Year, Season, Variable, Value\n";

    outFile.close();

} // end buildCSVFileMSVPASeasInfo

// Table 14 of 39
void
nmfSetup_Tab3::buildCSVFileMSVPASizePref(//std::map<std::string,std::vector<std::string> > *TableNames,
                                        std::string &csvPathFileName,std::string &tableName,
                                        QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                        QMap<std::string,int> &MinCatchAge,
                                        QMap<std::string,int> &MaxCatchAge,
                                        QStringList &Species,
                                        QStringList &OtherPredSpecies,
                                        QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, SpeIndex, SpeName, Age, EvacAlpha, EvacBeta, SizeAlpha, SizeBeta\n";

    outFile.close();

} // end buildCSVFileMSVPASizePref

// Table 15 of 39
void
nmfSetup_Tab3::buildCSVFileMSVPASpaceO(//std::map<std::string,std::vector<std::string> > *TableNames,
                                      std::string &csvPathFileName,std::string &tableName,
                                      QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                      QMap<std::string,int> &MinCatchAge,
                                      QMap<std::string,int> &MaxCatchAge,
                                      QStringList &Species,
                                      QStringList &OtherPredSpecies,
                                      QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, Season, SpeIndex, SpeType, SpeName, Age, PreyIndex, PreyName, SpOverlap\n";

    outFile.close();

} // end buildCSVFileMSVPASpaceO

// Table 16 of 39
void
nmfSetup_Tab3::buildCSVFileMSVPAStomCont(//std::map<std::string,std::vector<std::string> > *TableNames,
                                        std::string &csvPathFileName,std::string &tableName,
                                        QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                        QMap<std::string,int> &MinCatchAge,
                                        QMap<std::string,int> &MaxCatchAge,
                                        QStringList &Species,
                                        QStringList &OtherPredSpecies,
                                        QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, SpeIndex, SpeType, SpeName, Age, Season, MeanGutFull\n";

    outFile.close();

} // end buildCSVFileMSVPAStomCont

// Table 17 of 39
void
nmfSetup_Tab3::buildCSVFileMSVPASuitPreyBiomass(//std::map<std::string,std::vector<std::string> > *TableNames,
                                               std::string &csvPathFileName,std::string &tableName,
                                               QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                               QMap<std::string,int> &MinCatchAge,
                                               QMap<std::string,int> &MaxCatchAge,
                                               QStringList &Species,
                                               QStringList &OtherPredSpecies,
                                               QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, PredName, PredAge, PreyName, PreyAge, Year, "
            << "Season, SuitPreyBiomass, PropDiet, BMConsumed, PreyEDens\n";

    outFile.close();

} // end buildCSVFileMSVPASuitPreyBiomass

// Table 18 of 39
void
nmfSetup_Tab3::buildCSVFileMSVPAlist(//std::map<std::string,std::vector<std::string> > *TableNames,
                                    std::string &csvPathFileName,std::string &tableName,
                                    QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                    QMap<std::string,int> &MinCatchAge,
                                    QMap<std::string,int> &MaxCatchAge,
                                    QStringList &Species,
                                    QStringList &OtherPredSpecies,
                                    QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, NSpe, NPreyOnly, NOther, "
            << "NOtherPred, FirstYear, LastYear, NSeasons, "
            << "AnnTemps, SeasSpaceO, GrowthModel, Complete\n";

    outFile.close();


} // end buildCSVFileMSVPAlist

// Table 19 of 39
void
nmfSetup_Tab3::buildCSVFileMSVPAprefs(//std::map<std::string,std::vector<std::string> > *TableNames,
                                     std::string &csvPathFileName,std::string &tableName,
                                     QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                     QMap<std::string,int> &MinCatchAge,
                                     QMap<std::string,int> &MaxCatchAge,
                                     QStringList &Species,
                                     QStringList &OtherPredSpecies,
                                     QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, SpeIndex, SpeType, SpeName, Age, PreyIndex, PreyName, PrefRank, PrefVal\n";

    outFile.close();

} // end buildCSVFileMSVPAprefs

// Table 20 of 39
void
nmfSetup_Tab3::buildCSVFileMSVPAspecies(//std::map<std::string,std::vector<std::string> > *TableNames,
                                       std::string &csvPathFileName,std::string &tableName,
                                       QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                       QMap<std::string,int> &MinCatchAge,
                                       QMap<std::string,int> &MaxCatchAge,
                                       QStringList &Species,
                                       QStringList &OtherPredSpecies,
                                       QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, SpeName, SpeIndex, Type, TimeRec, LenRec, WtRec, SSVPAname, SSVPAindex\n";

    outFile.close();

} // end buildCSVFileMSVPAspecies

// Table 21 of 39
void
nmfSetup_Tab3::buildCSVFileOthPredSizeData(//std::map<std::string,std::vector<std::string> > *TableNames,
                                          std::string &csvPathFileName,std::string &tableName,
                                          QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                          QMap<std::string,int> &MinCatchAge,
                                          QMap<std::string,int> &MaxCatchAge,
                                          QStringList &Species,
                                          QStringList &OtherPredSpecies,
                                          QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;
    std::string species;
    int NumSizeCategories = 5;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());

    outFile << "SpeIndex, SpeName, SizeCat, MinLen, MaxLen, PropBM, "
            << "ConsAlpha, ConsBeta, SizeSelAlpha, SizeSelBeta\n";

    for (int i=0; i<OtherPredSpecies.size(); ++i) {
        species = OtherPredSpecies[i].toStdString();
        NumSizeCategories = OtherPredCategories[species];
        for (int j=0; j<NumSizeCategories; ++j) {
            //otherPredSpecies = OtherPredSpecies[SpeIndex-1].toStdString();
            outFile << (i+1) << ", " << species << ", " << std::to_string(j) << ", , , , , , , \n";
        }
    }
    outFile.close();

} // end buildCSVFileOthPredSizeData

// Table 22 of 39
void
nmfSetup_Tab3::buildCSVFileOtherPredBM(//std::map<std::string,std::vector<std::string> > *TableNames,
                                      std::string &csvPathFileName,std::string &tableName,
                                      QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                      QMap<std::string,int> &MinCatchAge,
                                      QMap<std::string,int> &MaxCatchAge,
                                      QStringList &Species,
                                      QStringList &OtherPredSpecies,
                                      QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;
    std::string otherPredSpecies;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());

    outFile << "Biomass" << std::endl;

    // Write out rows
    for (int SpeIndex=1; SpeIndex<=OtherPredSpecies.size(); ++SpeIndex) {
        otherPredSpecies = OtherPredSpecies[SpeIndex-1].toStdString();
        for (int Year=FirstYear[otherPredSpecies]; Year<=LastYear[otherPredSpecies]; ++Year) {
            outFile << otherPredSpecies << ", " << Year << ", " << std::endl;

        }
    }
    outFile.close();

} // end buildCSVFileOtherPredBM


// Table 23 of 39
void
nmfSetup_Tab3::buildCSVFileOtherPredSpecies(//std::map<std::string,std::vector<std::string> > *TableNames,
                                           std::string &csvPathFileName,std::string &tableName,
                                           QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                           QMap<std::string,int> &MinCatchAge,
                                           QMap<std::string,int> &MaxCatchAge,
                                           QStringList &Species,
                                           QStringList &OtherPredSpecies,
                                           QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "SpeIndex, SpeName, FirstYear, LastYear, "
            << "BMUnitIndex, BMUnits, WtUnitIndex, WtUnits, "
            << "SizeUnitIndex, SizeUnits, MinSize, MaxSize, "
            << "SizeStruc, NumSizeCats\n";

    outFile.close();



//    std::ofstream outFile;
//    std::string otherPredSpecies;
//    if (OtherPredSpecies.size() == 0)
//        return;

//    // Write out field names as first row
//    int NumFields = TableNames[0][tableName].size();
//    outFile = std::ofstream(csvPathFileName.c_str());
//    for (int i=0; i<NumFields-1; ++i) {
//        outFile << TableNames[0][tableName][i] << ",";
//    }
//    outFile << TableNames[0][tableName][NumFields-1] << std::endl;

//    // Write out rows
//    for (int SpeIndex=1; SpeIndex<=OtherPredSpecies.size(); ++SpeIndex) {
//        otherPredSpecies = OtherPredSpecies[SpeIndex-1].toStdString();
//        outFile << SpeIndex          << ", "; // Field 1
//        outFile << otherPredSpecies  << ", "; // Field 2
//        outFile << FirstYear         << ", "; // Field 3
//        outFile << LastYear          << ", "; // Field 4
//        outFile << ", ";                      // Field 5
//        outFile << ", ";                      // Field 6
//        outFile << ", ";                      // Field 7
//        outFile << ", ";                      // Field 8
//        outFile << ", ";                      // Field 9
//        outFile << ", ";                      // Field 10
//        outFile << ", ";                      // Field 11
//        outFile << ", ";                      // Field 12
//        outFile << ", ";                      // Field 13
//        outFile << std::endl;                 // Field 14 is empty
//    }

//    outFile.close();

} // end buildCSVFileOtherPredSpecies

// Table 24 of 39
void
nmfSetup_Tab3::buildCSVFileSSVPAAgeM(//std::map<std::string,std::vector<std::string> > *TableNames,
                                    std::string &csvPathFileName,std::string &tableName,
                                    QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                    QMap<std::string,int> &MinCatchAge,
                                    QMap<std::string,int> &MaxCatchAge,
                                    QStringList &Species,
                                    QStringList &OtherPredSpecies,
                                    QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "SpeIndex, SSVPAIndex, SSVPAName, Age, AgeM1, AgeM2, PRF\n";

    outFile.close();

} // end buildCSVFileSSVPAAgeM

// Table 25 of 39
void
nmfSetup_Tab3::buildCSVFileScenarioF(//std::map<std::string,std::vector<std::string> > *TableNames,
                                    std::string &csvPathFileName,std::string &tableName,
                                    QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                    QMap<std::string,int> &MinCatchAge,
                                    QMap<std::string,int> &MaxCatchAge,
                                    QStringList &Species,
                                    QStringList &OtherPredSpecies,
                                    QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, Scenario, VarType, SpeName, SpeIndex, Age, Year, F\n";

    outFile.close();

} // end buildCSVFileScenarioF

// Table 26 of 39
void
nmfSetup_Tab3::buildCSVFileScenarioOthPred(//std::map<std::string,std::vector<std::string> > *TableNames,
                                          std::string &csvPathFileName,std::string &tableName,
                                          QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                          QMap<std::string,int> &MinCatchAge,
                                          QMap<std::string,int> &MaxCatchAge,
                                          QStringList &Species,
                                          QStringList &OtherPredSpecies,
                                          QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, Scenario, SpeName, SpeIndex, SizeClass, Year, Biomass, PropBM\n";

    outFile.close();

} // end buildCSVFileScenarioOthPred

// Table 27 of 39
void
nmfSetup_Tab3::buildCSVFileScenarioOthPrey(//std::map<std::string,std::vector<std::string> > *TableNames,
                                          std::string &csvPathFileName,std::string &tableName,
                                          QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                          QMap<std::string,int> &MinCatchAge,
                                          QMap<std::string,int> &MaxCatchAge,
                                          QStringList &Species,
                                          QStringList &OtherPredSpecies,
                                          QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, Scenario, SpeName, SpeIndex, Year, Season, Biomass\n";

    outFile.close();

} // end buildCSVFileScenarioOthPrey

// Table 28 of 39
void
nmfSetup_Tab3::buildCSVFileScenarioRec(//std::map<std::string,std::vector<std::string> > *TableNames,
                                      std::string &csvPathFileName,std::string &tableName,
                                      QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                      QMap<std::string,int> &MinCatchAge,
                                      QMap<std::string,int> &MaxCatchAge,
                                      QStringList &Species,
                                      QStringList &OtherPredSpecies,
                                      QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, Scenario, SpeName, SpeIndex, Year, RecAdjust, AbsRecruits\n";

    outFile.close();

} // end buildCSVFileScenarioRec

// Table 29 of 39
void
nmfSetup_Tab3::buildCSVFileScenarios(//std::map<std::string,std::vector<std::string> > *TableNames,
                                    std::string &csvPathFileName,std::string &tableName,
                                    QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                    QMap<std::string,int> &MinCatchAge,
                                    QMap<std::string,int> &MaxCatchAge,
                                    QStringList &Species,
                                    QStringList &OtherPredSpecies,
                                    QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "MSVPAName, ForeName, Scenario, VarF, VarOthPred, VarOthPrey, VarRec, FishAsF\n";

    outFile.close();

} // end buildCSVFileScenarios

// Table 30 of 39
void
nmfSetup_Tab3::buildCSVFileSpeCatch(//std::map<std::string,std::vector<std::string> > *TableNames,
                                   std::string &csvPathFileName,std::string &tableName,
                                   QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                   QMap<std::string,int> &MinCatchAge,
                                   QMap<std::string,int> &MaxCatchAge,
                                   QStringList &Species,
                                   QStringList &OtherPredSpecies,
                                   QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;
    std::string species;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());

    outFile << "Species, Year, ";
    for (int i=0; i<nmfConstants::MaxNumberAges-1; ++i) {
        outFile << "Age " << i << ", ";
    }
    outFile << "Age " << nmfConstants::MaxNumberAges-1 << std::endl;

    // Write out rows
    for (int SpeIndex=1; SpeIndex<=Species.size(); ++SpeIndex) {
        species = Species[SpeIndex-1].toStdString();
        for (int Year=FirstYear[species]; Year<=LastYear[species]; ++Year) {
            outFile << species << ", " << Year << ", ";
            for (int Age=0; Age<MaxCatchAge[species]; ++Age) {
                outFile << ", ";
            }
            outFile << std::endl;
        }
    }

    outFile.close();

} // end buildCSVFileSpeCatch

// Table 31 of 39
void
nmfSetup_Tab3::buildCSVFileSpeMaturity(//std::map<std::string,std::vector<std::string> > *TableNames,
                                      std::string &csvPathFileName,std::string &tableName,
                                      QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                      QMap<std::string,int> &MinCatchAge,
                                      QMap<std::string,int> &MaxCatchAge,
                                      QStringList &Species,
                                      QStringList &OtherPredSpecies,
                                      QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;
    std::string species;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "Species, Year, Age, PMature" << std::endl;

    // Write out rows
    for (int SpeIndex=1; SpeIndex<=Species.size(); ++SpeIndex) {
        species = Species[SpeIndex-1].toStdString();
        for (int Year=FirstYear[species]; Year<=LastYear[species]; ++Year) {
            for (int Age=0; Age<=MaxCatchAge[species]; ++Age) {
                outFile << species << ", " << Year << ", " << Age << ", " << std::endl;
            }
        }
    }

    outFile.close();

} // end buildCSVFileSpeMaturity


// Table 32 of 39
void
nmfSetup_Tab3::buildCSVFileSpeSSVPA(//std::map<std::string,std::vector<std::string> > *TableNames,
                                   std::string &csvPathFileName,std::string &tableName,
                                   QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                   QMap<std::string,int> &MinCatchAge,
                                   QMap<std::string,int> &MaxCatchAge,
                                   QStringList &Species,
                                   QStringList &OtherPredSpecies,
                                   QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "SpeIndex, SSVPAName, SSVPAIndex, Type, AgeM, TermZType, ";
    outFile << "TermF, RefAge, LogCatchAge1, LogCatchAge2, FullRecAge, ";
    outFile << "PartSVPA, pSVPANCatYrs, pSVPANMortYrs, pSVPARefAge, ";
    outFile << "NFleets, NXSAIndex, Downweight, DownweightType, ";
    outFile << "DownweightYear, Shrink, ShrinkCV, ShrinkYears, ShrinkAge\n";

    outFile.close();

} // end buildCSVFileSpeSSVPA

// Table 33 of 39
void
nmfSetup_Tab3::buildCSVFileSpeSize(//std::map<std::string,std::vector<std::string> > *TableNames,
                                  std::string &csvPathFileName,std::string &tableName,
                                  QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                  QMap<std::string,int> &MinCatchAge,
                                  QMap<std::string,int> &MaxCatchAge,
                                  QStringList &Species,
                                  QStringList &OtherPredSpecies,
                                  QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;
    std::string species;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());

    outFile << "Species, Year, ";
    for (int i=0; i<nmfConstants::MaxNumberAges-1; ++i) {
        outFile << "Age " << i << ", ";
    }
    outFile << "Age " << nmfConstants::MaxNumberAges-1 << std::endl;

    // Write out rows
    for (int SpeIndex=1; SpeIndex<=Species.size(); ++SpeIndex) {
        species = Species[SpeIndex-1].toStdString();
        for (int Year=FirstYear[species]; Year<=LastYear[species]; ++Year) {
            outFile << species << ", " << Year << ", ";
            for (int Age=0; Age<MaxCatchAge[species]; ++Age) {
                outFile << ", ";
            }
            outFile << std::endl;
        }
    }

    outFile.close();

} // end buildCSVFileSpeSize

// Table 34 of 39
void
nmfSetup_Tab3::buildCSVFileSpeTuneCatch(//std::map<std::string,std::vector<std::string> > *TableNames,
                                       std::string &csvPathFileName,std::string &tableName,
                                       QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                       QMap<std::string,int> &MinCatchAge,
                                       QMap<std::string,int> &MaxCatchAge,
                                       QStringList &Species,
                                       QStringList &OtherPredSpecies,
                                       QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "SpeName, SpeIndex, SSVPAName, Fleet, Year, Age, Catch\n";

    outFile.close();

} // end buildCSVFileSpeTuneCatch


// Table 35 of 39
void
nmfSetup_Tab3::buildCSVFileSpeTuneEffort(//std::map<std::string,std::vector<std::string> > *TableNames,
                                        std::string &csvPathFileName,std::string &tableName,
                                        QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                        QMap<std::string,int> &MinCatchAge,
                                        QMap<std::string,int> &MaxCatchAge,
                                        QStringList &Species,
                                        QStringList &OtherPredSpecies,
                                        QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "SpeName, SpeIndex, SSVPAName, Fleet, Year, Effort\n";

    outFile.close();

} // end buildCSVFileSpeTuneEffort

// Table 36 of 39
void
nmfSetup_Tab3::buildCSVFileSpeWeight(//std::map<std::string,std::vector<std::string> > *TableNames,
                                    std::string &csvPathFileName,std::string &tableName,
                                    QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                    QMap<std::string,int> &MinCatchAge,
                                    QMap<std::string,int> &MaxCatchAge,
                                    QStringList &Species,
                                    QStringList &OtherPredSpecies,
                                    QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;
    std::string species;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());

    outFile << "Species, Year, ";
    for (int i=0; i<nmfConstants::MaxNumberAges-1; ++i) {
        outFile << "Age " << i << ", ";
    }
    outFile << "Age " << nmfConstants::MaxNumberAges-1 << std::endl;

    // Write out rows
    for (int SpeIndex=1; SpeIndex<=Species.size(); ++SpeIndex) {
        species = Species[SpeIndex-1].toStdString();
        for (int Year=FirstYear[species]; Year<=LastYear[species]; ++Year) {
            outFile << species << ", " << Year << ", ";
            for (int Age=0; Age<MaxCatchAge[species]; ++Age) {
                outFile << ", ";
            }
            outFile << std::endl;
        }
    }

    outFile.close();

} // end buildCSVFileSpeWeight

// Table 37 of 39
void
nmfSetup_Tab3::buildCSVFileSpeXSAData(//std::map<std::string,std::vector<std::string> > *TableNames,
                                     std::string &csvPathFileName,std::string &tableName,
                                     QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                     QMap<std::string,int> &MinCatchAge,
                                     QMap<std::string,int> &MaxCatchAge,
                                     QStringList &Species,
                                     QStringList &OtherPredSpecies,
                                     QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "SpeIndex, SSVPAName, \"Index\", Year, Age, Value\n";

    outFile.close();

} // end buildCSVFileSpeXSAData


// Table 38 of 39
void
nmfSetup_Tab3::buildCSVFileSpeXSAIndices(//std::map<std::string,std::vector<std::string> > *TableNames,
                                        std::string &csvPathFileName,
                                        std::string &tableName,
                                        QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                        QMap<std::string,int> &MinCatchAge,
                                        QMap<std::string,int> &MaxCatchAge,
                                        QStringList &Species,
                                        QStringList &OtherPredSpecies,
                                        QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;
    std::string   species;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "SpeIndex, SSVPAName, \"Index\", IndexName, Alpha, Beta\n";

    outFile.close();

} // end buildCSVFileSpeXSAIndices



// RSK remove this function.  Not needed anymore.



// Table 39 of 39
void
nmfSetup_Tab3::buildCSVFileSpecies(//std::map<std::string,std::vector<std::string> > *TableNames,
                                  std::string &csvPathFileName,
                                  std::string &tableName,
                                  QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                  QMap<std::string,int> &MinCatchAge,
                                  QMap<std::string,int> &MaxCatchAge,
                                  QStringList &Species,
                                  QStringList &OtherPredSpecies,
                                  QMap<std::string,int> &OtherPredCategories)
{
    std::ofstream outFile;

    // Write out header as first row
    outFile = std::ofstream(csvPathFileName.c_str());
    outFile << "SpeIndex, SpeName, MaxAge, MinCatAge, MaxCatAge, FirstYear, "
            << "LastYear, PlusClass, CatchUnits, WtTypeIndex, WtUnits, "
            << "SizeTypeIndex, SizeUnits, FixedMaturity\n";

    outFile.close();

//    std::ofstream outFile;
//    int NumIndex=1;
//    int NumFields = TableNames[0][tableName].size();
//    QStringList::iterator it = Species.begin();

//    outFile = std::ofstream(csvPathFileName.c_str());
//    for (int i=0; i<NumFields-1; ++i) {
//        outFile << TableNames[0][tableName][i] << ",";
//    }
//    outFile << TableNames[0][tableName][NumFields-1] << std::endl;

//    while (it != Species.end()) {
//        outFile << NumIndex++ << ", ";       // Field 1
//        outFile << (*it).toStdString() << ", ";            // Field 2
//        outFile << ", ";                     // Field 3
//        outFile << MinCatchAge[(*it).toStdString()] << ", "; // Field 4
//        outFile << MaxCatchAge[(*it).toStdString()] << ", "; // Field 5
//        outFile << FirstYear << ", ";        // Field 6
//        outFile << LastYear <<  ", ";        // Field 7
//        outFile << ", ";                     // Field 8
//        outFile << ", ";                     // Field 9
//        outFile << ", ";                     // Field 10
//        outFile << ", ";                     // Field 11
//        outFile << ", ";                     // Field 12
//        outFile << ", ";                     // Field 13
//        outFile << "";                       // Field 14
//        outFile << std::endl;
//        it++;
//    }
//    outFile.close();

} // end buildCSVFileSpecies


