#include "nmfConstants.h"
#include "nmfUtilsQt.h"
#include "nmfUtils.h"

#include "nmfMSVPATab02.h"

nmfMSVPATab2::nmfMSVPATab2(QTabWidget *tabs,
                           nmfLogger *theLogger,
                           std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::nmfMSVPATab2");

    MSVPA_Tabs = tabs;
    MSVPAlistCSVFile.clear();
    MSVPASeasInfoCSVFile.clear();
    ProjectDir = theProjectDir;
    Enabled = false;
    smodelSeasonLength = NULL;
    smodelSeasonTemps  = NULL;

    // Load ui as a widget from disk
    QFile file(":/forms/MSVPA/MSVPA_Tab02.ui");
    file.open(QFile::ReadOnly);
    MSVPA_Tab2_Widget = loader.load(&file,MSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    MSVPA_Tabs->addTab(MSVPA_Tab2_Widget, tr("2. Time Frame"));

    MSVPA_Tab2_PrevPB              = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab2_PrevPB");
    MSVPA_Tab2_NextPB              = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab2_NextPB");
    MSVPA_Tab2_LoadPB              = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab2_LoadPB");
    MSVPA_Tab2_SavePB              = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab2_SavePB");
    MSVPA_Tab2_YearsGB             = MSVPA_Tabs->findChild<QGroupBox *>("MSVPA_Tab2_YearsGB");
    MSVPA_Tab2_AnnTempVariCB       = MSVPA_Tabs->findChild<QCheckBox *>("MSVPA_Tab2_AnnTempVariCB");
    MSVPA_Tab2_SeasonalOverlapCB   = MSVPA_Tabs->findChild<QCheckBox *>("MSVPA_Tab2_SeasonalOverlapCB");
    MSVPA_Tab2_PredatorGrowthCB    = MSVPA_Tabs->findChild<QCheckBox *>("MSVPA_Tab2_PredatorGrowthCB");
    MSVPA_Tab2_EarliestYearLB      = MSVPA_Tabs->findChild<QLabel *>("MSVPA_Tab2_EarliestYearLB");
    MSVPA_Tab2_LatestYearLB        = MSVPA_Tabs->findChild<QLabel *>("MSVPA_Tab2_LatestYearLB");
    MSVPA_Tab2_FirstYearLE         = MSVPA_Tabs->findChild<QLineEdit *>("MSVPA_Tab2_FirstYearLE");
    MSVPA_Tab2_LastYearLE          = MSVPA_Tabs->findChild<QLineEdit *>("MSVPA_Tab2_LastYearLE");
    MSVPA_Tab2_NumberSeasonsLE     = MSVPA_Tabs->findChild<QLineEdit *>("MSVPA_Tab2_NumberSeasonsLE");
    MSVPA_Tab2_SeasonalDataTV      = MSVPA_Tabs->findChild<QTableView *>("MSVPA_Tab2_SeasonalDataTV");
    MSVPA_Tab2_SetSeasonalLengthPB = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab2_SetSeasonalLengthPB");
    MSVPA_Tab2_SetSeasonalTempPB   = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab2_SetSeasonalTempPB");
    MSVPA_Tab2_SeasonalDataLBL     = MSVPA_Tabs->findChild<QLabel *>("MSVPA_Tab2_SeasonalDataLBL");

    // Initialize the tab's widgets
    MSVPA_Tab2_PrevPB->setText("\u25C1--");
    MSVPA_Tab2_NextPB->setText("--\u25B7");
    callback_MSVPA_Tab2_PredatorGrowth(0);

    // Setup connections
    connect(MSVPA_Tab2_PrevPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab2_PrevPB(bool)));
    connect(MSVPA_Tab2_NextPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab2_NextPB(bool)));
    connect(MSVPA_Tab2_LoadPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab2_LoadPB(bool)));
    connect(MSVPA_Tab2_SavePB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab2_SavePB(bool)));
    connect(MSVPA_Tab2_PredatorGrowthCB, SIGNAL(stateChanged(int)),
            this,               SLOT(callback_MSVPA_Tab2_PredatorGrowth(int)));
    connect(MSVPA_Tab2_SetSeasonalLengthPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab2_SetSeasonalLengthPB(bool)));
    connect(MSVPA_Tab2_SetSeasonalTempPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab2_SetSeasonalTempPB(bool)));
    connect(MSVPA_Tab2_AnnTempVariCB, SIGNAL(stateChanged(int)),
            this,                     SLOT(callback_MSVPA_Tab2_AnnTempVariCB(int)));
    connect(MSVPA_Tab2_SeasonalOverlapCB, SIGNAL(stateChanged(int)),
            this,                     SLOT(callback_MSVPA_Tab2_SeasonalOverlapCB(int)));

    connect(MSVPA_Tab2_FirstYearLE,     SIGNAL(editingFinished()),
            this,                       SLOT(callback_StripFirstYearLE()));
    connect(MSVPA_Tab2_LastYearLE,      SIGNAL(editingFinished()),
            this,                       SLOT(callback_StripLastYearLE()));
    connect(MSVPA_Tab2_NumberSeasonsLE, SIGNAL(editingFinished()),
            this,                       SLOT(callback_StripNumberSeasonsLE()));

    MSVPA_Tab2_LoadPB->setEnabled(false);
    MSVPA_Tab2_NextPB->setEnabled(false);
    MSVPA_Tab2_SetSeasonalTempPB->setEnabled(false);

    //MSVPA_Tab2_NumberSeasonsLE->setReadOnly(true);

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::nmfMSVPATab2 Complete");

}


nmfMSVPATab2::~nmfMSVPATab2()
{
    clearWidgets();
}


void
nmfMSVPATab2::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir


void nmfMSVPATab2::clearWidgets()
{    
//std::cout << "nmfMSVPATab2::clearWidgets" << std::endl;

    if (smodelSeasonLength)
        smodelSeasonLength->clear();

    if (smodelSeasonTemps)
        smodelSeasonTemps->clear();

    MSVPA_Tab2_FirstYearLE->setText("0");
    MSVPA_Tab2_LastYearLE->setText("0");
    MSVPA_Tab2_NumberSeasonsLE->setText("0");
    MSVPA_Tab2_AnnTempVariCB->setChecked(false);
    MSVPA_Tab2_SeasonalOverlapCB->setChecked(false);
    MSVPA_Tab2_PredatorGrowthCB->setChecked(false);
}

void
nmfMSVPATab2::callback_StripFirstYearLE()
{
    MarkAsDirty("MSVPAlist");
    MSVPA_Tab2_FirstYearLE->setText(MSVPA_Tab2_FirstYearLE->text().trimmed());
}

void
nmfMSVPATab2::callback_StripLastYearLE()
{
    MarkAsDirty("MSVPAlist");
    MSVPA_Tab2_LastYearLE->setText(MSVPA_Tab2_LastYearLE->text().trimmed());
}

void
nmfMSVPATab2::callback_StripNumberSeasonsLE()
{
    MSVPA_Tab2_NumberSeasonsLE->setText(MSVPA_Tab2_NumberSeasonsLE->text().trimmed());
}

void
nmfMSVPATab2::callback_MSVPA_Tab2_AnnTempVariCB(int state)
{
    MarkAsDirty("MSVPAlist");

    MSVPA_Tab2_SavePB->setEnabled(true);
    MSVPA_Tab2_NextPB->setEnabled(true);
    MSVPA_Tab2_LoadPB->setEnabled(true);

} // end callback_MSVPA_Tab2_AnnTempVariCB


void
nmfMSVPATab2::callback_MSVPA_Tab2_SeasonalOverlapCB(int state)
{
    MarkAsDirty("MSVPAlist");

    MSVPA_Tab2_SavePB->setEnabled(true);
    MSVPA_Tab2_NextPB->setEnabled(true);
    MSVPA_Tab2_LoadPB->setEnabled(true);

} // end callback_MSVPA_Tab2_SeasonalOverlapCB



void
nmfMSVPATab2::callback_MSVPA_Tab2_ItemChanged_SeasonLength(QStandardItem *item)
{
    if (item->column() == Length_Column) {
        QString qstr = item->index().data().toString();
        SeasonLength[item->row()] = qstr.toStdString();
    }

    MarkAsDirty("MSVPASeasInfo");

    MSVPA_Tab2_SavePB->setEnabled(true);
    MSVPA_Tab2_NextPB->setEnabled(false);

    // Check total on Save
    /*
    int totalDays = 0;
    for (int i=0; i<NSeasons; ++i) {
        totalDays += std::stoi(SeasonLength[i]);
    }
    if (totalDays != 365) {
        std::cout << "Warning; Total season days = " << totalDays <<
                     ", should be 365. " << std::endl;
    }
    */

} // end callback_MSVPA_Tab2_ItemChanged_SeasonLength


void
nmfMSVPATab2::callback_MSVPA_Tab2_ItemChanged_SeasonTemps(QStandardItem *item)
{
    QString qstr = item->index().data().toString();
    int row = item->row();
    int col = item->column();
    SeasonTemps[row*NSeasons+col] = qstr.toStdString();

    MarkAsDirty("MSVPASeasInfo");

    MSVPA_Tab2_SavePB->setEnabled(true);
    MSVPA_Tab2_NextPB->setEnabled(false);
    MSVPA_Tab2_LoadPB->setEnabled(true);

} // end callback_MSVPA_Tab2_ItemChanged_SeasonTemps


void
nmfMSVPATab2::MarkAsDirty(std::string tableName)
{

    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfMSVPATab2::MarkAsClean()
{

    emit MarkMSVPAAsClean();

} // end MarkAsClean


bool
nmfMSVPATab2::loadWidgets(nmfDatabase *theDatabasePtr,
                               std::string theMSVPAName)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    int MinYear = 0;
    int MaxYear = 5000;
    int NSpecies;
    int InitNSeasons;
    int firstYear;
    int lastYear;
    int InitFirstYear;
    int InitLastYear;
    int NumRecords;
    int AnnTempsChecked=0;
    int SeasonalSpaceOChecked=0;
    int InitSeasonSpaceO;
    int InitGrowth;
    int MSVPAGrowth;
    std::vector<int> SpeIndex;

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::loadWidgets");

    if (smodelSeasonLength != NULL)
        smodelSeasonLength->clear();

    SeasonLength.clear();
    SeasonTemps.clear();

    SpeIndex.clear();
    MSVPAName   = theMSVPAName;
    databasePtr = theDatabasePtr;

    smodelSeasonLength = new QStandardItemModel(nmfConstants::MaxNumberSeasons, 2); // 2 columns: Season and Length
    smodelSeasonTemps  = new QStandardItemModel(nmfConstants::MaxNumberYears, nmfConstants::MaxNumberSeasons);

    connect(smodelSeasonLength, SIGNAL(itemChanged(QStandardItem *)),
            this,               SLOT(callback_MSVPA_Tab2_ItemChanged_SeasonLength(QStandardItem *)));

    // Load year widgets
    // Get the species collection
    fields   = {"SpeIndex"};
    queryStr = "SELECT SpeIndex FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName +
               "' AND (Type = 0 or Type = 1)";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NSpecies = dataMap["SpeIndex"].size();
    for (int i = 0; i < NSpecies; ++i) {
     SpeIndex.push_back(std::stoi(dataMap["SpeIndex"][i]));
    } // end for i

    // Get the minimum and maximum years from the species collection
    for (int i = 0; i < NSpecies; ++i) {
        fields    = {"FirstYear","LastYear"};
        queryStr  = "SELECT FirstYear,LastYear FROM Species WHERE SpeIndex = " + std::to_string(SpeIndex[i]);
        dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
        firstYear = std::stoi(dataMap["FirstYear"][0]);
        lastYear  = std::stoi(dataMap["LastYear"][0]);
        MinYear   = (firstYear > MinYear) ? firstYear : MinYear;
        MaxYear   = (lastYear  < MaxYear) ? lastYear  : MaxYear;
    } // end for i

    // Do same for other preds...
    fields   = {"SpeIndex"};
    queryStr = "SELECT SpeIndex FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName +
               "' AND Type = 3";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpeIndex"].size();
    SpeIndex.clear();
    if (NumRecords > 0) {
        NSpecies = NumRecords;
        for (int i = 0; i < NSpecies; ++i) {
            SpeIndex.push_back( std::stoi(dataMap["SpeIndex"][i]));
        } // end for i
        // Get the minimum and maximum years from the species collection
         for (int i = 0; i < NSpecies; ++i) {
             fields    = {"FirstYear","LastYear"};
             queryStr  = "SELECT FirstYear,LastYear FROM OtherPredSpecies WHERE SpeIndex = " + std::to_string(SpeIndex[i]);
             dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
             firstYear = std::stoi(dataMap["FirstYear"][0]);
             lastYear  = std::stoi(dataMap["LastYear"][0]);
             MinYear   = (firstYear > MinYear) ? firstYear : MinYear;
             MaxYear   = (lastYear  < MaxYear) ? lastYear  : MaxYear;
        } // end for i
    } // end if

    // Load widgets
    MSVPA_Tab2_EarliestYearLB->setText(QString::number(MinYear));
    MSVPA_Tab2_LatestYearLB->setText(QString::number(MaxYear));
    MSVPA_Tab2_FirstYearLE->setText(QString::number(MinYear));
    MSVPA_Tab2_LastYearLE->setText(QString::number(MaxYear));

    fields   = {"NSeasons","FirstYear","LastYear","AnnTemps","SeasSpaceO","GrowthModel"};
    queryStr = "SELECT NSeasons,FirstYear,LastYear,AnnTemps,SeasSpaceO,GrowthModel FROM MSVPAlist WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (! dataMap["NSeasons"].empty()) {
        NSeasons = std::stoi(dataMap["NSeasons"][0]);
        MSVPA_Tab2_NumberSeasonsLE->setText(QString::number(NSeasons));
        InitNSeasons = NSeasons;
    } // end if
    if (! dataMap["FirstYear"].empty()) {
        FirstYear = std::stoi(dataMap["FirstYear"][0]);
        MSVPA_Tab2_FirstYearLE->setText(QString::number(FirstYear));
        InitFirstYear = FirstYear;
    } // end if
    if (! dataMap["LastYear"].empty()) {
        LastYear = std::stoi(dataMap["LastYear"][0]);
        MSVPA_Tab2_LastYearLE->setText(QString::number(LastYear));
        InitLastYear = LastYear;
    } // end if

    if (! dataMap["AnnTemps"].empty()) {
        AnnTempsChecked = std::stoi(dataMap["AnnTemps"][0]);
        MSVPA_Tab2_AnnTempVariCB->setChecked(AnnTempsChecked == 1);
    } // end if

    if (! dataMap["SeasSpaceO"].empty()) {
        SeasonalSpaceOChecked = std::stoi(dataMap["SeasSpaceO"][0]);
        MSVPA_Tab2_SeasonalOverlapCB->setChecked(SeasonalSpaceOChecked == 1);
        InitSeasonSpaceO = SeasonalSpaceOChecked;
    } // end if

    if (! dataMap["GrowthModel"].empty()) {
        InitGrowth = std::stoi(dataMap["GrowthModel"][0]);
        MSVPA_Tab2_PredatorGrowthCB->setChecked(InitGrowth == 1);
        MSVPAGrowth = InitGrowth;
    } // end if

            // continue with: gvim frmMSVPAyears3.frm

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::loadWidgets Complete");
    MSVPA_Tab2_LoadPB->setEnabled(false);
    MarkAsClean();

    return true;

} // end loadWidgets


std::vector<int>
nmfMSVPATab2::resaveMSVPASeasInfo(int NSeasons)
{
    std::vector<int> retVec;

    if (NSeasons <= 0)
        retVec = {};
    else if (NSeasons == 1)
        retVec = {365};
    else if (NSeasons == 2)
        retVec = {182,183};
    else if (NSeasons == 3)
        retVec = {121,122,122};
    else if (NSeasons == 4)
        retVec = {91,91,91,92};
    else if (NSeasons == 5)
        retVec ={73,73,73,73,73};
    else if (NSeasons == 6)
        retVec = {60,61,61,61,61,61};
    else {
        int seasonLength = 365/NSeasons;
        for (int i=0; i<NSeasons; ++i) {
            retVec.push_back(seasonLength); // Won't be exact....but shouldn't even have num seasons > 6
        }
    }

    return retVec;
} // end resaveMSVPASeasInfo



void
nmfMSVPATab2::callback_MSVPA_Tab2_SetSeasonalLengthPB(bool unused)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    int NumRecords=0;
    QStringList strList;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::callback_MSVPA_Tab2_SetSeasonalLengthPB");

    MSVPA_Tab2_SeasonalDataLBL->setText("Seasonal length data (days):");

    if (MSVPA_Tab2_FirstYearLE->text().toInt() *
        MSVPA_Tab2_LastYearLE->text().toInt() *
        MSVPA_Tab2_NumberSeasonsLE->text().toInt() == 0)
    {
        QMessageBox::information(MSVPA_Tabs,
                                 tr("Missing Data"),
                                 tr("\nPlease enter valid (ie, non-zero) data for MSVPA years and number of seasons."),
                                 QMessageBox::Ok);
        return;
    } else {
        // Resave fields to MSVPAlist
        if (! resaveMSVPAlistFields())
            return;
        MSVPA_Tab2_SavePB->setEnabled(true);
        MSVPA_Tab2_NextPB->setEnabled(false);
    }

//    smodelSeasonLength->setColumnCount(2);

//    MSVPA_Tab2_SeasonalDataTV->setModel(smodelSeasonLength);
//    // Hide row headers
//    MSVPA_Tab2_SeasonalDataTV->verticalHeader()->setVisible(false);

//    // Make column headers
//    strList.clear();
//    strList << "Season" << "Length (days)";
//    smodelSeasonLength->setHorizontalHeaderLabels(strList);
//return;
    bool ok;
    int NSeasons = MSVPA_Tab2_NumberSeasonsLE->text().toInt(&ok);
    if (!ok || NSeasons <= 0) {
        QMessageBox::information(MSVPA_Tabs,
                                 tr("Missing Data"),
                                 tr("Please enter valid Number of Seasons"),
                                 QMessageBox::Ok);
        return;
    }


    smodelSeasonLength->setColumnCount(2);
    smodelSeasonLength->setRowCount(NSeasons);

    // Check first if there are current data in table.  If not, display empty table.
    fields   = {"Value"};
    queryStr = "SELECT Value FROM MSVPASeasInfo WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Variable='SeasLen' AND Year=0";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["Value"].size();
    if (NumRecords > 0) {
        for (int i=0; i<NumRecords; ++i) {
            SeasonLength.push_back(dataMap["Value"][i]);
        }
    } else {
        std::vector<int> SeasonLengths;
        SeasonLengths = resaveMSVPASeasInfo(NSeasons);
        for (unsigned int i=0; i<SeasonLengths.size(); ++i) {
            SeasonLength.push_back(std::to_string(SeasonLengths[i]));
        }
        MSVPA_Tab2_SavePB->setEnabled(true);
        MSVPA_Tab2_NextPB->setEnabled(false);
    }


    // Set up model and load data
    if (smodelSeasonLength != NULL) {
        smodelSeasonLength->clear();
    }
    MSVPA_Tab2_SeasonalDataTV->setModel(smodelSeasonLength);


    disconnect(smodelSeasonLength, SIGNAL(itemChanged(QStandardItem *)),
               this,               SLOT(callback_MSVPA_Tab2_ItemChanged_SeasonLength(QStandardItem *)));

    // Load the model
    for (int i=0; i<NSeasons; ++i){
        QStandardItem *item0 = new QStandardItem("Season "+QString::number(i+1));
        item0->setTextAlignment(Qt::AlignCenter);
        item0->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled); // Season is read-only
        QStandardItem *item1 = new QStandardItem(QString::fromStdString(SeasonLength[i]));
        item1->setTextAlignment(Qt::AlignCenter);
        smodelSeasonLength->setItem(i, Season_Column, item0);
        smodelSeasonLength->setItem(i, Length_Column, item1);
    }

    MSVPA_Tab2_SeasonalDataTV->setModel(smodelSeasonLength);

    // Hide row headers
    MSVPA_Tab2_SeasonalDataTV->verticalHeader()->setVisible(false);

    // Make column headers
    strList.clear();
    strList << "Season" << "Length (days)";
    smodelSeasonLength->setHorizontalHeaderLabels(strList);

    connect(smodelSeasonLength, SIGNAL(itemChanged(QStandardItem *)),
            this,               SLOT(callback_MSVPA_Tab2_ItemChanged_SeasonLength(QStandardItem *)));

    MSVPA_Tab2_LoadPB->setEnabled(true);
    MSVPA_Tab2_SetSeasonalTempPB->setEnabled(true);

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::callback_MSVPA_Tab2_SetSeasonalLengthPB Complete");

} // end callback_MSVPA_Tab2_SetSeasonalLengthPB


bool
nmfMSVPATab2::resaveMSVPAlistFields()
{
std::cout << "in resave, MSVPAName = " << MSVPAName << std::endl;
    std::string cmd;
    std::string errorMsg;
    int NSpe = 0;
    int NPreyOnly = 0;
    int NOtherPred = 0;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    bool retv = true;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::resaveMSVPAlistFields");

    // Find NSpe, NPreyOnly, and NOtherPred from MSVPAspecies
    fields   = {"MSVPAName","Type"};
    queryStr = "SELECT MSVPAName,Type FROM MSVPAspecies WHERE MSVPAName = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    int NumRecords = dataMap["MSVPAName"].size();
    for (int i=0; i<NumRecords; ++i) {
        if (std::stoi(dataMap["Type"][i])      == nmfConstantsMSVPA::MSVPAFullSpecies)
            ++NSpe;
        else if (std::stoi(dataMap["Type"][i]) == nmfConstantsMSVPA::MSVPAPreyOnlySpecies)
            ++NPreyOnly;
        else if (std::stoi(dataMap["Type"][i]) == nmfConstantsMSVPA::MSVPABiomassPredator)
            ++NOtherPred;
    }

    int FirstYear   = MSVPA_Tab2_FirstYearLE->text().toInt();
    int LastYear    = MSVPA_Tab2_LastYearLE->text().toInt();
    int NSeasons    = MSVPA_Tab2_NumberSeasonsLE->text().toInt();
    int AnnTemps    = MSVPA_Tab2_AnnTempVariCB->isChecked();
    int SeasSpaceO  = MSVPA_Tab2_SeasonalOverlapCB->isChecked();
    int GrowthModel = MSVPA_Tab2_PredatorGrowthCB->isChecked();

    cmd += "INSERT INTO MSVPAlist ";
    cmd += "(MSVPAName,NSpe,NPreyOnly,NOtherPred,FirstYear,LastYear,NSeasons,AnnTemps,SeasSpaceO,GrowthModel) values ";
    cmd +=  "(\"" + MSVPAName + "\"" +
            "," + std::to_string(NSpe) +
            "," + std::to_string(NPreyOnly) +
            "," + std::to_string(NOtherPred) +
            "," + std::to_string(FirstYear) +
            "," + std::to_string(LastYear) +
            "," + std::to_string(NSeasons) +
            "," + std::to_string(AnnTemps) +
            "," + std::to_string(SeasSpaceO) +
            "," + std::to_string(GrowthModel) + ") ";
    cmd += " ON DUPLICATE KEY UPDATE ";
    cmd += " NSpe=values(NSpe), NPreyOnly=values(NPreyOnly), NOtherPred=values(NOtherPred), ";
    cmd += " FirstYear=values(FirstYear), LastYear=values(LastYear), NSeasons=values(NSeasons), ";
    cmd += " AnnTemps=values(AnnTemps), SeasSpaceO=values(SeasSpaceO), GrowthModel=values(GrowthModel); ";
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (errorMsg != " ") {
        nmfUtils::printError("resaveMSVPAlistFields: INSERT INTO MSVPAlist...", errorMsg);
        retv = false;
    }

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::resaveMSVPAlistFields Complete");

    return retv;
} // end resaveMSVPAlistFields


void
nmfMSVPATab2::callback_MSVPA_Tab2_SetSeasonalTempPB(bool unused)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    int NumRecords=0;
    QStringList strList;
    int m;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::callback_MSVPA_Tab2_SetSeasonalTempPB");

    MSVPA_Tab2_SeasonalDataLBL->setText("Seasonal temperature data (degrees Celsius):");

    if (MSVPA_Tab2_FirstYearLE->text().toInt() *
        MSVPA_Tab2_LastYearLE->text().toInt() *
        MSVPA_Tab2_NumberSeasonsLE->text().toInt() == 0)
    {
        QMessageBox::information(MSVPA_Tabs,
                                 tr("Missing Data"),
                                 tr("\nPlease enter valid (ie, non-zero) data for MSVPA years and number of seasons."),
                                 QMessageBox::Ok);
        return;
    } else {
        // Resave fields to MSVPAlist
        if (! resaveMSVPAlistFields())
            return;
        MSVPA_Tab2_SavePB->setEnabled(true);
        MSVPA_Tab2_NextPB->setEnabled(false);
    }

    bool ok,okFirstYear,okLastYear;
    int NSeasons  = MSVPA_Tab2_NumberSeasonsLE->text().toInt(&ok);
    int FirstYear = MSVPA_Tab2_FirstYearLE->text().toInt(&okFirstYear);
    int LastYear  = MSVPA_Tab2_LastYearLE->text().toInt(&okLastYear);
    if (! okFirstYear || ! okLastYear || LastYear < FirstYear || FirstYear*LastYear == 0) {
        QMessageBox::information(MSVPA_Tabs,
                                 tr("Invalid Year Data"),
                                 tr("Please enter valid, non-zero values for FirstYear and LastYear."),
                                 QMessageBox::Ok);
                return;
    }


    smodelSeasonTemps->setRowCount(LastYear-FirstYear+1);
    smodelSeasonTemps->setColumnCount(NSeasons);

    MSVPA_Tab2_SeasonalDataTV->setModel(smodelSeasonTemps);

    // Make row headers
    strList.clear();
    for (int k=FirstYear; k<=LastYear; ++k) {
        strList << " " + QString::number(k) + " ";
    }
    smodelSeasonTemps->setVerticalHeaderLabels(strList);
    MSVPA_Tab2_SeasonalDataTV->verticalHeader()->setVisible(true);

    // Make column headers
    strList.clear();
    for (int k=0; k<NSeasons; ++k) {
        strList << "Season " + QString::number(k+1);
    }
    smodelSeasonTemps->setHorizontalHeaderLabels(strList);

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::callback_MSVPA_Tab2_SetSeasonalTempPB Complete");
    MSVPA_Tab2_NextPB->setEnabled(true);


    // Check first if there are current data in table.  If not, display empty table.
    m = 0;
    fields   = {"Year","Season","Value"};
    queryStr = "SELECT Year,Season,Value FROM MSVPASeasInfo WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Variable='SeasTemp' ORDER BY Year,Season";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["Value"].size();
    if (NumRecords > 0) {
        for (int i=FirstYear; i <=LastYear; ++i) {
            for (int j=0; j<NSeasons; ++j) {
                SeasonTemps.push_back(dataMap["Value"][m++]);
            }
        }
    } else {
        for (int i=FirstYear; i <=LastYear; ++i) {
            for (int j=0; j<NSeasons; ++j) {
                SeasonTemps.push_back("");
            }
        }
    }

    // Set up model and load data
    if (smodelSeasonTemps != NULL) {
        smodelSeasonTemps->clear();
    }
    MSVPA_Tab2_SeasonalDataTV->setModel(smodelSeasonTemps);

    disconnect(smodelSeasonTemps, SIGNAL(itemChanged(QStandardItem *)),
               this,              SLOT(callback_MSVPA_Tab2_ItemChanged_SeasonTemps(QStandardItem *)));

    // Load the model
    m = 0;
    for (int i=FirstYear; i <=LastYear; ++i) {
        for (int j=0; j<NSeasons; ++j) {
            QStandardItem *item = new QStandardItem(QString::fromStdString(SeasonTemps[m++]));
            item->setTextAlignment(Qt::AlignCenter);
            smodelSeasonTemps->setItem(i-FirstYear, j, item);
        }
    }

    MSVPA_Tab2_SeasonalDataTV->setModel(smodelSeasonTemps);

    // Make row headers
    strList.clear();
    for (int k=FirstYear; k<=LastYear; ++k) {
        strList << " " + QString::number(k) + " ";
    }
    smodelSeasonTemps->setVerticalHeaderLabels(strList);
    MSVPA_Tab2_SeasonalDataTV->verticalHeader()->setVisible(true);

    // Make column headers
    strList.clear();
    for (int k=0; k<NSeasons; ++k) {
        strList << "Season " + QString::number(k+1);
    }
    smodelSeasonTemps->setHorizontalHeaderLabels(strList);

    connect(smodelSeasonTemps, SIGNAL(itemChanged(QStandardItem *)),
            this,              SLOT(callback_MSVPA_Tab2_ItemChanged_SeasonTemps(QStandardItem *)));


} // end callback_MSVPA_Tab2_SetSeasonalTempPB


void
nmfMSVPATab2::callback_MSVPA_Tab2_PrevPB(bool unused)
{
    int prevPage = MSVPA_Tabs->currentIndex()-1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(prevPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(prevPage);
    emit UpdateNavigator("MSVPA",prevPage);

} // end callback_MSVPA_Tab2_PrevPB


void
nmfMSVPATab2::callback_MSVPA_Tab2_NextPB(bool unused)
{   
    int nextPage = MSVPA_Tabs->currentIndex()+1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(nextPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(nextPage);
    emit UpdateNavigator("MSVPA",nextPage);

} // end callback_MSVPA_Tab2_NextPB


void
nmfMSVPATab2::callback_MSVPA_Tab2_LoadPB(bool unused)
{
    int csvFileRow = 0;
    QString line;
    QStringList fields;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(MSVPA_Tabs);
    QStringList NameFilters;
    QString MSVPASeasInfoCSVFile;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::callback_MSVPA_Tab2_LoadPB");

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile("MSVPASeasInfo.csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load MSVPA Seasonal Information CSV File");
    if (fileDlg.exec()) {

        // Open the file here....
        MSVPASeasInfoCSVFile = fileDlg.selectedFiles()[0];
        QFile file(MSVPASeasInfoCSVFile);
        if(! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(MSVPA_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);

        SeasonTemps.clear();
        QStringList YearLengths;
        int MinYear=9999;
        int MaxYear=0;
        int FirstYear, LastYear=0;
        QString csvMSVPAName, csvYear, csvSeason, csvVariable, csvValue;
        QStringList SeasonTemps;
        // Store data to load in next step
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header
                fields = line.split(",");
                csvMSVPAName = fields[0].trimmed();
                csvYear      = fields[1].trimmed();
                csvSeason    = fields[2].trimmed();
                csvVariable  = fields[3].trimmed();
                csvValue     = fields[4].trimmed();
                if (csvMSVPAName == QString::fromStdString(MSVPAName)) {
                    if (csvYear == "0") {    // If it's a length record
                        YearLengths << csvValue;
                    } else {                 // If it's a temperature record
                        if (csvYear.toInt() < MinYear) {
                            FirstYear = csvYear.toInt();
                            MinYear = FirstYear;
                        }
                        if (csvYear.toInt() > MaxYear) {
                            LastYear = csvYear.toInt();
                            MaxYear = LastYear;
                        }
                        SeasonTemps.push_back(csvValue);
                    }
                }
            }
            ++csvFileRow;
        }

        // Now load the data
        int m = 0;
        int NSeasons = YearLengths.count();
        MSVPA_Tab2_NumberSeasonsLE->setText(QString::number(NSeasons));
        for (int i=0; i<NSeasons; ++i) {
            QStandardItem *item = new QStandardItem(YearLengths[m++]);
            item->setTextAlignment(Qt::AlignCenter);
            smodelSeasonLength->setItem(i,1,item);
        }
        m = 0;
        for (int i=FirstYear; i <=LastYear; ++i) {
            for (int j=0; j<NSeasons; ++j) {
                QStandardItem *item = new QStandardItem(SeasonTemps[m++]);
                item->setTextAlignment(Qt::AlignCenter);
                smodelSeasonTemps->setItem(i-FirstYear, j, item);
            }
        }

        file.close();

        MSVPA_Tab2_SavePB->setEnabled(true);
        MSVPA_Tab2_NextPB->setEnabled(false);
    }

    MSVPA_Tab2_LoadPB->clearFocus();
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::callback_MSVPA_Tab2_LoadPB Complete");

} // end callback_MSVPA_Tab2_LoadPB


bool
nmfMSVPATab2::checkNumTotalDays()
{
    // Total days must sum to 265;
    bool retv = true;
    QModelIndex modelIndex;

    int numRows = smodelSeasonLength->rowCount();
    int TotalDays = 0;
    for (int i=0; i<numRows; ++i) {
        modelIndex = smodelSeasonLength->index(i,1);
        TotalDays += modelIndex.data().toInt();
    }
    if (TotalDays != 365) {
        QMessageBox::information(MSVPA_Tabs,
                                 tr("Invalid Season Length"),
                                 tr("\nSum of all Season days must be 365.\n"),
                                 QMessageBox::Ok);
        retv = false;
    }

    return retv;
} // end checkNumTotalDays


void
nmfMSVPATab2::callback_MSVPA_Tab2_SavePB(bool unused)
{
    bool saveOK = true;
    bool FirstLine;
    QString fileName;
    QString filePath;
    QString fileNameWithPathList;
    QString fileNameWithPathSeasInfo;
    QString tmpFileNameWithPathList;
    QString tmpFileNameWithPathSeasInfo;
    QString qcmd;
    QString line;
    QString TableNameList;
    QString TableNameSeasInfo;
    QStringList qfields;
    std::string cmd;
    std::string errorMsg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    bool ignoreMSVPAName = false;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::callback_MSVPA_Tab2_SavePB");

    if (! nmfUtilsQt::allCellsArePopulated(MSVPA_Tabs,MSVPA_Tab2_SeasonalDataTV,
                                           nmfConstants::ShowError))
        return;

    if (! checkNumTotalDays())
        return;

    // Save MSVPA general data to MSVPAlist
    TableNameList = "MSVPAlist";

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (MSVPAlistCSVFile.isEmpty()) {
        MSVPAlistCSVFile = TableNameList + ".csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPathList    = QDir(filePath).filePath(MSVPAlistCSVFile);
        tmpFileNameWithPathList = QDir(filePath).filePath("."+MSVPAlistCSVFile);
    } else {
        QFileInfo finfo(MSVPAlistCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPathList    = MSVPAlistCSVFile;
        tmpFileNameWithPathList = QDir(filePath).filePath("."+fileName);
    }
    MSVPAlistCSVFile.clear();

    // Read entire file and copy all lines that don't have the current MSVPAName into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    QFile fin(fileNameWithPathList);
    QFile fout(tmpFileNameWithPathList);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(1) Opening Input File",
                                 "\n"+fin.errorString()+": "+fileNameWithPathList);
        return;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(1) Opening Output File",
                                 "\n"+fout.errorString()+": "+tmpFileNameWithPathList);
        return;
    }
    QTextStream inStream(&fin);
    QTextStream outStream(&fout);
    QString NSpe,NPreyOnly,NOther,NOtherPred,FirstYear,LastYear,NSeasons,AnnTemps;
    QString SeasSpaceO,GrowthModel,Complete;

    // Find NSpe, NPreyOnly, and NOtherPred from MSVPAspecies
    int NSpeVal       = 0;
    int NPreyOnlyVal  = 0;
    int NOtherPredVal = 0;
    fields   = {"MSVPAName","Type"};
    queryStr = "SELECT MSVPAName,Type FROM MSVPAspecies WHERE MSVPAName = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    int NumRecords = dataMap["MSVPAName"].size();
    for (int i=0; i<NumRecords; ++i) {
        if (std::stoi(dataMap["Type"][i])      == nmfConstantsMSVPA::MSVPAFullSpecies)
            ++NSpeVal;
        else if (std::stoi(dataMap["Type"][i]) == nmfConstantsMSVPA::MSVPAPreyOnlySpecies)
            ++NPreyOnlyVal;
        else if (std::stoi(dataMap["Type"][i]) == nmfConstantsMSVPA::MSVPABiomassPredator)
            ++NOtherPredVal;
    }
    NSpe       = QString::number(NSpeVal);
    NPreyOnly  = QString::number(NPreyOnlyVal);
    NOtherPred = QString::number(NOtherPredVal);
    NOther     = "0"; // Hardcode a default value here. This gets set in a later Tab.

    QString tableLine;
    QString csvMSVPAName;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvMSVPAName   = qfields[0].trimmed();
        if (csvMSVPAName == QString::fromStdString(MSVPAName) && (! csvMSVPAName.isEmpty())) { // dump qtableview data into output file
            // Skip over all other MSVPAName rows, since going through this once copies the qtableview data
            if (! ignoreMSVPAName) {
                FirstYear   = MSVPA_Tab2_FirstYearLE->text();
                LastYear    = MSVPA_Tab2_LastYearLE->text();
                NSeasons    = MSVPA_Tab2_NumberSeasonsLE->text();
                AnnTemps    = QString::number(MSVPA_Tab2_AnnTempVariCB->isChecked());
                SeasSpaceO  = QString::number(MSVPA_Tab2_SeasonalOverlapCB->isChecked());
                GrowthModel = QString::number(MSVPA_Tab2_PredatorGrowthCB->isChecked());
                // Complete =  // Don't know where this comes from??
                tableLine = QString::fromStdString(MSVPAName) + ", " +
                        NSpe        + ", " +
                        NPreyOnly   + ", " +
                        NOther      + ", " +
                        NOtherPred  + ", " +
                        FirstYear   + ", " +
                        LastYear    + ", " +
                        NSeasons    + ", " +
                        AnnTemps    + ", " +
                        SeasSpaceO  + ", " +
                        GrowthModel + ", 0 ";
                        outStream << tableLine << "\n";
                ignoreMSVPAName = true;
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }
    }
    if (! ignoreMSVPAName) { // means never found the MSVPAName
        FirstYear   = MSVPA_Tab2_FirstYearLE->text();
        LastYear    = MSVPA_Tab2_LastYearLE->text();
        NSeasons    = MSVPA_Tab2_NumberSeasonsLE->text();
        AnnTemps    = QString::number(MSVPA_Tab2_AnnTempVariCB->isChecked());
        SeasSpaceO  = QString::number(MSVPA_Tab2_SeasonalOverlapCB->isChecked());
        GrowthModel = QString::number(MSVPA_Tab2_PredatorGrowthCB->isChecked());
        // Complete =  // Don't know where this comes from??
        tableLine = QString::fromStdString(MSVPAName) + ", " +
                NSpe        + ", " +
                NPreyOnly   + ", " +
                NOther      + ", " +
                NOtherPred  + ", " +
                FirstYear   + ", " +
                LastYear    + ", " +
                NSeasons    + ", " +
                AnnTemps    + ", " +
                SeasSpaceO  + ", " +
                GrowthModel + ", 0 ";
                outStream << tableLine << "\n";
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    int retv = nmfUtilsQt::rename(tmpFileNameWithPathList,
                                  fileNameWithPathList);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPathList.toStdString() << " to " <<
                     fileNameWithPathList.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return;
    }

   // --------------------------------------------------------------------------

    // Save season length data to MSVPASeasInfo
    TableNameSeasInfo = "MSVPASeasInfo";

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (MSVPASeasInfoCSVFile.isEmpty()) {
        MSVPASeasInfoCSVFile = TableNameSeasInfo + ".csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPathSeasInfo    = QDir(filePath).filePath(MSVPASeasInfoCSVFile);
        tmpFileNameWithPathSeasInfo = QDir(filePath).filePath("."+MSVPASeasInfoCSVFile);
    } else {
        QFileInfo finfo(MSVPASeasInfoCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPathSeasInfo    = MSVPASeasInfoCSVFile;
        tmpFileNameWithPathSeasInfo = QDir(filePath).filePath("."+fileName);
    }
    MSVPASeasInfoCSVFile.clear();

    // Read entire file and copy all lines that don't have the current MSVPAName into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    fin.setFileName(fileNameWithPathSeasInfo);
    fout.setFileName(tmpFileNameWithPathSeasInfo);

    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs, "Error(2) Opening Input File", fin.errorString()+": "+fileNameWithPathList);
        return;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(MSVPA_Tabs, "Error(2) Opening Output File", fout.errorString());
        return;
    }
    inStream.setDevice(&fin);
    outStream.setDevice(&fout);

    ignoreMSVPAName = false;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvMSVPAName = qfields[0].trimmed();
        if (csvMSVPAName == QString::fromStdString(MSVPAName) && (! csvMSVPAName.isEmpty())) { // dump qtableview data into output file
            // Skip over all other MSVPAName rows, since going through this once copies the qtableview data
            if (! ignoreMSVPAName) {
                // Write out SeasLen data
                for (int i=0; i<NSeasons.toInt(); ++i) {
                    tableLine = QString::fromStdString(MSVPAName) + ", 0, " +
                                QString::number(i+1) + ", SeasLen, " +
                                smodelSeasonLength->index(i,1).data().toString();
                    outStream << tableLine << "\n";
                }

                // Write out SeasTemp data
                int firstYear = FirstYear.toInt();
                int lastYear  = LastYear.toInt();
                for (int i=firstYear; i<=lastYear; ++i) {
                    for (int j=0; j<NSeasons.toInt(); ++j) {
                        tableLine = QString::fromStdString(MSVPAName) + ", " +
                                QString::number(i)   + ", " +
                                QString::number(j+1) + ", SeasTemp, " +
                                smodelSeasonTemps->index(i-firstYear,j).data().toString();
                        outStream << tableLine << "\n";
                    }
                }
                ignoreMSVPAName = true;
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }
    }
    if (! ignoreMSVPAName) { // means never found the MSVPAName

        // Write out SeasLen data
        for (int i=0; i<NSeasons.toInt(); ++i) {
            tableLine = QString::fromStdString(MSVPAName) + ", 0, " +
                        QString::number(i+1) + ", SeasLen, " +
                        smodelSeasonLength->index(i,1).data().toString();
            outStream << tableLine << "\n";
        }

        // Write out SeasTemp data
        int firstYear = FirstYear.toInt();
        int lastYear  = LastYear.toInt();
        for (int i=firstYear; i<=lastYear; ++i) {
            for (int j=0; j<NSeasons.toInt(); ++j) {
                tableLine = QString::fromStdString(MSVPAName) + ", " +
                        QString::number(i)   + ", " +
                        QString::number(j+1) + ", SeasTemp, " +
                        smodelSeasonTemps->index(i-firstYear,j).data().toString();
                outStream << tableLine << "\n";
            }
        }
    }

    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    retv = nmfUtilsQt::rename(tmpFileNameWithPathSeasInfo,
                                  fileNameWithPathSeasInfo);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPathSeasInfo.toStdString() << " to " <<
                     fileNameWithPathSeasInfo.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return;
    }

    // ----------------------------------------------

    //
    // Save MSVPAlist mysql table
    //

    // Clear the current table contents
//    qcmd = "TRUNCATE TABLE " + TableNameList;
//    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
//    if (errorMsg != " ") {
//        saveOK = false;
//        nmfUtils::printError("MSVPA Save(2): Clearing table error: ", errorMsg);
//    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableNameList.toStdString();
    cmd += " (MSVPAName,NSpe,NPreyOnly,NOther,NOtherPred,FirstYear,LastYear,";
    cmd += "NSeasons,AnnTemps,SeasSpaceO,GrowthModel,Complete) VALUES ";

    // Read from csv file since it has all the Species in it
    fin.setFileName(fileNameWithPathList);
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(2) Opening Input File",
                                 "\n"+fin.errorString()+": " +fileNameWithPathList);
        return;
    }
    inStream.setDevice(&fin);

    FirstLine = true;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields = line.split(",");
        cmd += "(\"" + qfields[0].trimmed().toStdString()  + "\",";
        cmd +=         qfields[1].trimmed().toStdString()  + ",";
        cmd +=         qfields[2].trimmed().toStdString()  + ",";
        cmd +=         qfields[3].trimmed().toStdString()  + ",";
        cmd +=         qfields[4].trimmed().toStdString()  + ",";
        cmd +=         qfields[5].trimmed().toStdString()  + ",";
        cmd +=         qfields[6].trimmed().toStdString()  + ",";
        cmd +=         qfields[7].trimmed().toStdString()  + ",";
        cmd +=         qfields[8].trimmed().toStdString()  + ",";
        cmd +=         qfields[9].trimmed().toStdString()  + ",";
        cmd +=         qfields[10].trimmed().toStdString() + ",";
        cmd +=         qfields[11].trimmed().toStdString() + "), ";

    } // end for all rows in csv file
    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);

    cmd += " ON DUPLICATE KEY UPDATE ";
    cmd += " NSpe=values(NSpe), NPreyOnly=values(NPreyOnly), NOther=values(NOther),";
    cmd += " NOtherPred=values(NOtherPred), FirstYear=values(FirstYear), LastYear=values(LastYear),";
    cmd += " NSeasons=values(NSeasons), AnnTemps=values(AnnTemps), SeasSpaceO=values(SeasSpaceO),";
    cmd += " GrowthModel=values(GrowthModel), Complete=values(Complete); ";

    fin.close();
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (errorMsg != " ") {
        std::cout << cmd << std::endl;
        nmfUtils::printError("MSVPA Save(2a): Write table error: ", errorMsg);
        return;
    }

    // ----------------------------------------------

    //
    // Save MSVPASeasInfo mysql table
    //

    // Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableNameSeasInfo;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (errorMsg != " ") {
        saveOK = false;
        nmfUtils::printError("MSVPA Save(2): Clearing table error: ", errorMsg);
    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableNameSeasInfo.toStdString();
    cmd += " (MSVPAName,Year,Season,Variable,Value) VALUES ";

    // Read from csv file since it has all the Species in it
    fin.setFileName(fileNameWithPathSeasInfo);
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(2) Opening Input File",
                                 "\n"+fin.errorString()+": " +fileNameWithPathSeasInfo );
        return;
    }
    inStream.setDevice(&fin);

    FirstLine = true;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields = line.split(",");
        cmd += "(\"" + qfields[0].trimmed().toStdString()  + "\",";
        cmd +=         qfields[1].trimmed().toStdString()  + ",";
        cmd +=         qfields[2].trimmed().toStdString()  + ",";
        cmd += "\"" +  qfields[3].trimmed().toStdString()  + "\",";
        cmd +=         qfields[4].trimmed().toStdString() + "), ";
    } // end for all rows in csv file

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    fin.close();
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (errorMsg != " ") {
        std::cout << cmd << std::endl;
        nmfUtils::printError("MSVPA Save(2b): Write table error: ", errorMsg);
        return;
    }

    if (saveOK) {
        QMessageBox::information(MSVPA_Tabs, "Save",
                                 tr("\nMSVPA Seasonal Length and Temperature data saved.\n"));
    }

    MarkAsClean();

    MSVPA_Tab2_SavePB->setEnabled(false);
    MSVPA_Tab2_NextPB->setEnabled(true);

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab2::callback_MSVPA_Tab2_SavePB Complete");


} // end callback_MSVPA_Tab2_SavePB

void
nmfMSVPATab2::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    TableName = "MSVPAlist";
    fields    = {"MSVPAName","NSpe","NPreyOnly","NOther","NOtherPred",
                 "FirstYear","LastYear","NSeasons","AnnTemps",
                 "SeasSpaceO","GrowthModel","Complete"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = "MSVPASeasInfo";
    fields    = {"MSVPAName","Year","Season","Variable","Value"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase



void
nmfMSVPATab2::callback_MSVPA_Tab2_PredatorGrowth(int state)
{
    MSVPA_Tabs->setTabEnabled(7, state==Qt::Checked);
    MSVPA_Tabs->setTabEnabled(8, state==Qt::Checked);
}
