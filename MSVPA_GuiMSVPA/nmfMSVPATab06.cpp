
#include "nmfConstants.h"
#include "nmfUtils.h"
#include "nmfUtilsQt.h"

#include "nmfMSVPATab06.h"



nmfMSVPATab6::nmfMSVPATab6(QTabWidget *tabs,
                           nmfLogger *theLogger,
                           std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab6::nmfMSVPATab6");

    MSVPA_Tabs = tabs;
    ProjectDir = theProjectDir;
    Enabled = false;
    smodel = NULL;
    databasePtr = NULL;
    MSVPASpaceOCSVFile.clear();

    // Load ui as a widget from disk
    QFile file(":/forms/MSVPA/MSVPA_Tab06.ui");
    file.open(QFile::ReadOnly);
    MSVPA_Tab6_Widget = loader.load(&file,MSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    MSVPA_Tabs->addTab(MSVPA_Tab6_Widget, tr("6. Spatial Overlap"));

    // Setup connections
    MSVPA_Tab6_LoadPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab6_LoadPB");
    MSVPA_Tab6_SavePB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab6_SavePB");
    MSVPA_Tab6_ClearPB     = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab6_ClearPB");
    MSVPA_Tab6_PrevPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab6_PrevPB");
    MSVPA_Tab6_NextPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab6_NextPB");
    MSVPA_Tab6_SpeciesCMB  = MSVPA_Tabs->findChild<QComboBox   *>("MSVPA_Tab6_SpeciesCMB");
    MSVPA_Tab6_SpOverlapTV = MSVPA_Tabs->findChild<QTableView  *>("MSVPA_Tab6_SpOverlapTV");
    MSVPA_Tab6_SeasonCMB   = MSVPA_Tabs->findChild<QComboBox   *>("MSVPA_Tab6_SeasonCMB");

    MSVPA_Tab6_PrevPB->setText("\u25C1--");
    MSVPA_Tab6_NextPB->setText("--\u25B7");

    connect(MSVPA_Tab6_PrevPB,     SIGNAL(clicked(bool)),
            this,                  SLOT(callback_MSVPA_Tab6_PrevPB(bool)));
    connect(MSVPA_Tab6_NextPB,     SIGNAL(clicked(bool)),
            this,                  SLOT(callback_MSVPA_Tab6_NextPB(bool)));
    connect(MSVPA_Tab6_LoadPB,     SIGNAL(clicked(bool)),
            this,                  SLOT(callback_MSVPA_Tab6_LoadPB(bool)));
    connect(MSVPA_Tab6_SavePB,     SIGNAL(clicked(bool)),
            this,                  SLOT(callback_MSVPA_Tab6_SavePB(bool)));
    connect(MSVPA_Tab6_ClearPB,    SIGNAL(clicked(bool)),
            this,                  SLOT(callback_MSVPA_Tab6_ClearPB(bool)));
    connect(MSVPA_Tab6_SpeciesCMB, SIGNAL(currentIndexChanged(int)),
            this,                  SLOT(callback_MSVPA_Tab6_SpeciesCMB(int)));
    connect(MSVPA_Tab6_SeasonCMB,  SIGNAL(currentIndexChanged(int)),
            this,                  SLOT(callback_MSVPA_Tab6_SeasonCMB(int)));

    MSVPA_Tab6_SavePB->setEnabled(false);

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab6::nmfMSVPATab6 Complete");

}


nmfMSVPATab6::~nmfMSVPATab6()
{
    std::cout << "Freeing Tab5 memory" << std::endl;
    clearWidgets();
}


void
nmfMSVPATab6::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir

void
nmfMSVPATab6::clearWidgets()
{
    if (smodel)
        smodel->clear();
}


void
nmfMSVPATab6::callback_MSVPA_Tab6_ClearPB(bool unused)
{
    std::string errorMsg;
    QString qcmd;
    QString TableNameSpaceO = QString::fromStdString(nmfConstantsMSVPA::TableMSVPASpaceO);
    bool clearedOK = true;
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(MSVPA_Tabs,
                                  tr("Clear"),
                                  tr("\nOK to Clear MSVPASpaceO CSV and database tables?"),
                                  QMessageBox::No|QMessageBox::Yes);
    if (reply == QMessageBox::Yes)
    {
        // Clear CSV file
        errorMsg.clear();
        clearedOK = nmfUtilsQt::clearCSVFile(TableNameSpaceO.toStdString(),
                                             ProjectDir,
                                             errorMsg);
        if (! clearedOK)
            return;

        // Clear out the MSVPASpaceO database table
        qcmd = "TRUNCATE TABLE " + TableNameSpaceO;
        errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
        if (nmfUtilsQt::isAnError(errorMsg)) {
            nmfUtils::printError("MSVPA Clear(5): Clearing table error: ",
                                 errorMsg+": "+TableNameSpaceO.toStdString());
            return;
        }

        QMessageBox::information(MSVPA_Tabs,
                                 tr("Clear"),
                                 tr("\nMSVPASpaceO CSV and database tables cleared."),
                                 QMessageBox::Ok);

        // Reload tab
        loadWidgets(databasePtr, MSVPAName);

    } // end if reply == Yes

} // end callback_MSVPA_Tab6_ClearPB


void
nmfMSVPATab6::callback_MSVPA_Tab6_LoadPB(bool unused)
{
    int csvFileRow = 0;
    QString line;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(MSVPA_Tabs);
    QStringList NameFilters;
    QString MSVPASpaceOCSVFile;
    QStandardItem *item;
    std::string msg;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab6::callback_MSVPA_Tab6_LoadPB");

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile(QString::fromStdString(nmfConstantsMSVPA::TableMSVPASpaceO)+".csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load MSVPA Spatial Overlap CSV File");
    if (fileDlg.exec()) {

        // Open the file here....
        MSVPASpaceOCSVFile = fileDlg.selectedFiles()[0];
        QFile file(MSVPASpaceOCSVFile);
        if (! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(MSVPA_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);
        QString csvMSVPAName, csvSeason, csvSpeIndex, csvSpeType;
        QString csvSpeName, csvAge, csvPreyIndex, csvPreyName, csvSpOverlap;
        // Store data to load in next step
        QStringList fields;
        QString SpeName = MSVPA_Tab6_SpeciesCMB->currentText().trimmed();
        QString Season  = QString::number(MSVPA_Tab6_SeasonCMB->currentIndex()+1);
        int numRows = smodel->rowCount();
        int numCols = smodel->columnCount();
        std::vector<QString> SpaceOData;
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header
                fields = line.split(",");
                csvMSVPAName = fields[0].trimmed();
                csvSeason    = fields[1].trimmed();
                csvSpeIndex  = fields[2].trimmed();
                csvSpeType   = fields[3].trimmed();
                csvSpeName   = fields[4].trimmed();
                csvAge       = fields[5].trimmed();
                csvPreyIndex = fields[6].trimmed();
                csvPreyName  = fields[7].trimmed();
                csvSpOverlap = fields[8].trimmed();
                if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
                    (csvSpeName == SpeName) && (csvSeason == Season))
                {
                    SpaceOData.push_back(csvSpOverlap);  // temporarily store the data
                }
            }
            ++csvFileRow;
        }
        file.close();

        // Now load the data
        int m = 0;
        int NumRecords = SpaceOData.size();
        if (NumRecords == numRows*numCols) {
            for (int j=0; j<numCols; ++j) {
                for (int i=0; i < numRows; ++i) {
                    item = new QStandardItem(SpaceOData[m++]);
                    item->setTextAlignment(Qt::AlignCenter);
                    smodel->setItem(i, j, item);
                }
            }
        } else {
            msg = "\nNumber of pertinent records in MSVPASpaceO CSV file (" + std::to_string(NumRecords) + ")\n";
            msg += "does not equal number of visible table cells (" + std::to_string(numCols);
            msg += "x" + std::to_string(numRows) + ")";
            QMessageBox::information(MSVPA_Tabs,
                                     tr("Database Load Problem"),
                                     tr(msg.c_str()),
                                     QMessageBox::Ok);
            return;
        }


        MSVPA_Tab6_SavePB->setEnabled(true);
        MSVPA_Tab6_NextPB->setEnabled(false);

    } // end if dlg

    MSVPA_Tab6_LoadPB->clearFocus();

    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab6::callback_MSVPA_Tab6_LoadPB Complete");

} // end callback_MSVPA_Tab6_LoadPB


void
nmfMSVPATab6::callback_MSVPA_Tab6_SavePB(bool unused)
{
    bool saveOK = true;
    bool FirstLine;
    QString fileName;
    QString filePath;
    QString fileNameWithPathSpaceO;
    QString tmpFileNameWithPathSpaceO;
    QString qcmd;
    QString line;
    QString TableNameSpaceO;
    QString tableLine;
    QStringList qfields;
    std::string cmd;
    std::string errorMsg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    bool ignoreMSVPAName = false;
    int retv;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab6::callback_MSVPA_Tab6_SavePB");

    //
    // Save info to MSVPASpaceO
    //
    if (! nmfUtilsQt::allCellsArePopulated(MSVPA_Tabs,MSVPA_Tab6_SpOverlapTV,
                                           nmfConstants::ShowError))
        return;

    // Build Species and Type index maps
    std::map<std::string,int> SpeciesIndexMap;
    std::map<std::string,int> SpeciesTypeMap;
    fields   = {"MSVPAName", "SpeName", "SpeIndex", "Type"};
    queryStr = "SELECT MSVPAName,SpeName,SpeIndex,Type FROM " +
                nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAName = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);

    for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
        SpeciesIndexMap[dataMap["SpeName"][i]] = std::stoi(dataMap["SpeIndex"][i]);
        SpeciesTypeMap[dataMap["SpeName"][i]]  = std::stoi(dataMap["Type"][i]);
    }

//    // Build SpeIndex to SSVPAName and SSVPAIndex map
//    std::map<int,QString> SSVPANameMap;
//    std::map<int,QString> SSVPAIndexMap;
//    fields   = {"SpeIndex","SSVPAName","SSVPAIndex"};
//    queryStr = "SELECT SpeIndex,SSVPAName,SSVPAIndex FROM " + nmfConstantsMSVPA::TableSpeSSVPA;
//    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
//    for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
//        SSVPANameMap[dataMap["SpeName"][i]]  = QString::fromStdString(dataMap["SSVPAName"][i]);
//        SSVPAIndexMap[dataMap["SpeName"][i]] = QString::fromStdString(dataMap["SSVPAIndex"][i]);
//    }

    TableNameSpaceO = QString::fromStdString(nmfConstantsMSVPA::TableMSVPASpaceO);

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (MSVPASpaceOCSVFile.isEmpty()) {
        MSVPASpaceOCSVFile = TableNameSpaceO + ".csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPathSpaceO    = QDir(filePath).filePath(MSVPASpaceOCSVFile);
        tmpFileNameWithPathSpaceO = QDir(filePath).filePath("."+MSVPASpaceOCSVFile);
    } else {
        QFileInfo finfo(MSVPASpaceOCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPathSpaceO    = MSVPASpaceOCSVFile;
        tmpFileNameWithPathSpaceO = QDir(filePath).filePath("."+fileName);
    }
    MSVPASpaceOCSVFile.clear();

    // Read entire file and copy all lines that don't have the current MSVPAName into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    QFile fin(fileNameWithPathSpaceO);
    QFile fout(tmpFileNameWithPathSpaceO);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(5a) Opening Input File",
                                 "\n"+fin.errorString()+": "+fileNameWithPathSpaceO);
        return;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(5a) Opening Output File",
                                 "\n"+fout.errorString()+": "+tmpFileNameWithPathSpaceO);
        return;
    }

    QTextStream inStream(&fin);
    QTextStream outStream(&fout);
    QModelIndex index;
    ignoreMSVPAName = false;
    int numRows = smodel->rowCount();
    int numCols = smodel->columnCount();
    QString SpeName = MSVPA_Tab6_SpeciesCMB->currentText();
    int Season      = MSVPA_Tab6_SeasonCMB->currentIndex()+1;
    QString csvMSVPAName, csvSeason, csvSpeName;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvMSVPAName = qfields[0].trimmed();
        csvSeason    = qfields[1].trimmed();
        csvSpeName   = qfields[4].trimmed();
        if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
            (csvSpeName == SpeName) && (csvSeason == QString::number(Season)) &&
            (! csvMSVPAName.isEmpty())) { // dump qtableview data into output file
            // Skip over all other MSVPAName rows, since going through this once copies the qtableview data
            if (! ignoreMSVPAName) {
                for (int j=0; j<numCols; ++j) {
                    for (int i=0; i<numRows; ++i) {
                        index = MSVPA_Tab6_SpOverlapTV->model()->index(i,j);
                        tableLine = QString::fromStdString(MSVPAName)                   + ", " + // MSVPAName
                                QString::number(Season)                                 + ", " + // Season
                                QString::number(SpeciesIndexMap[SpeName.toStdString()]) + ", " + // N.B. SpeIndex was left blank in original
                                QString::number(SpeciesTypeMap[SpeName.toStdString()])  + ", " + // SpeType
                                QString::fromStdString(SpeName.toStdString())           + ", " + // SpeName
                                QString::number(j)                                      + ", " + // Age
                                QString::number(i)                                      + ", " + // PreyIndex
                                smodel->verticalHeaderItem(i)->text()                   + ", " + // PreyName
                                index.data().toString();                                         // SpOverlap
                        outStream << tableLine << "\n";
                    } // end for j
                } // end for i
                ignoreMSVPAName = true;
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }
    }
    if (! ignoreMSVPAName) { // means never found the MSVPAName
        for (int j=0; j<numCols; ++j) {
            for (int i=0; i<numRows; ++i) {
                index = MSVPA_Tab6_SpOverlapTV->model()->index(i,j);
                tableLine = QString::fromStdString(MSVPAName)                   + ", " + // MSVPAName
                        QString::number(Season)                                 + ", " + // Season
                        QString::number(SpeciesIndexMap[SpeName.toStdString()]) + ", " + // N.B. SpeIndex was left blank in original
                        QString::number(SpeciesTypeMap[SpeName.toStdString()])  + ", " + // SpeType
                        QString::fromStdString(SpeName.toStdString())           + ", " + // SpeName
                        QString::number(j)                                      + ", " + // Age
                        QString::number(i)                                      + ", " + // PreyIndex
                        smodel->verticalHeaderItem(i)->text()                   + ", " + // PreyName
                        index.data().toString();                                         // SpOverlap
                outStream << tableLine << "\n";
            } // end for j
        } // end for i
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    retv = nmfUtilsQt::rename(tmpFileNameWithPathSpaceO,
                                  fileNameWithPathSpaceO);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPathSpaceO.toStdString() << " to " <<
                     fileNameWithPathSpaceO.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return;
    }

    // --------------------------------------------

    //
    // Save MSVPASpaceO mysql table
    //
    // 1. Check that the table exists, if not, create it.
    databasePtr->checkForTableAndCreate(TableNameSpaceO);

    // 2. Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableNameSpaceO;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
        saveOK = false;
        nmfUtils::printError("MSVPA Save(5a): Clearing table error: ",
                             errorMsg+": "+TableNameSpaceO.toStdString());
    }

    // 3. Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableNameSpaceO.toStdString();
    cmd += " (MSVPAName,Season,SpeIndex,SpeType,SpeName,Age,PreyIndex,PreyName,SpOverlap) VALUES ";

    // Read from csv file since it has all the MSVPANames in it
    fin.setFileName(fileNameWithPathSpaceO);
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(5b) Opening Input File",
                                 "\n"+fin.errorString()+": " + fileNameWithPathSpaceO);
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
        cmd += "(\"" + qfields[0].trimmed().toStdString() + "\",";
        cmd +=         qfields[1].trimmed().toStdString() + ",";
        cmd +=         qfields[2].trimmed().toStdString() + ",";
        cmd +=         qfields[3].trimmed().toStdString() + ",";
        cmd +=  "\"" + qfields[4].trimmed().toStdString() + "\",";
        cmd +=         qfields[5].trimmed().toStdString() + ",";
        cmd +=         qfields[6].trimmed().toStdString() + ",";
        cmd +=  "\"" + qfields[7].trimmed().toStdString() + "\",";
        cmd +=         qfields[8].trimmed().toStdString() + "), ";

    } // end for all rows in csv file
    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);

    fin.close();
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("MSVPA Save(5b): Write table error: ", errorMsg);
        return;
    }

    if (saveOK) {
        QMessageBox::information(MSVPA_Tabs, "Save",
                                 tr("\nMSVPA Spatial Overlap data saved.\n"));
    }

    MSVPASpaceOCSVFile.clear();

    MSVPA_Tab6_NextPB->setEnabled(true);

    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab6::callback_MSVPA_Tab6_SavePB Complete");

} // end callback_MSVPA_Tab6_SavePB


void
nmfMSVPATab6::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableMSVPASpaceO);
    fields    = {"MSVPAName","Season","SpeIndex","SpeType","SpeName",
                "Age","PreyIndex","PreyName","SpOverlap"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase


void
nmfMSVPATab6::callback_MSVPA_Tab6_ItemChanged(QStandardItem *item)
{
    item->setTextAlignment(Qt::AlignCenter);
    smodel->setItem(item->row(), item->column(), item);

    MarkAsDirty(nmfConstantsMSVPA::TableMSVPASpaceO);
    MSVPA_Tab6_SavePB->setEnabled(true);
    MSVPA_Tab6_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab6_ItemChanged


void
nmfMSVPATab6::MarkAsDirty(std::string tableName)
{
    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfMSVPATab6::MarkAsClean()
{
    emit MarkMSVPAAsClean();
} // end MarkAsClean


bool
nmfMSVPATab6::loadWidgets(nmfDatabase *theDatabasePtr,
                          std::string theMSVPAName)
{
    int NumRecords;
    int SeasSpaceO;
    std::string msg;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::string species;
    std::vector<std::string> Species;
    std::vector<std::string> OthPred;
    QStandardItem *item;

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab6::loadWidgets");

    databasePtr = theDatabasePtr;
    MSVPAName   = theMSVPAName;

    Nage.clear();
    PredType.clear();
    AllSpecies.clear();
    Species.clear();
    OthPred.clear();
    Prey.clear();
    OthPrey.clear();

    // First get some initial data
    fields       = {"NPreyOnly","NSpe","NOther","NOtherPred","NSeasons","SeasSpaceO"};
    queryStr     = "SELECT NPreyOnly,NSpe,NOther,NOtherPred,NSeasons,SeasSpaceO FROM " +
                    nmfConstantsMSVPA::TableMSVPAlist +
                   " WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap      = databasePtr->nmfQueryDatabase(queryStr, fields);
    NPrey        = std::stoi(dataMap["NPreyOnly"][0]);
    NSpecies     = std::stoi(dataMap["NSpe"][0]);
    NOtherPrey   = std::stoi(dataMap["NOther"][0]);
    NOtherPred   = std::stoi(dataMap["NOtherPred"][0]);
    NPreds       = NSpecies + NOtherPred;
    TotalNSpe    = NSpecies + NPrey + NOtherPrey;
    RealNSeasons = std::stoi(dataMap["NSeasons"][0]);

    AllSpecies.clear();

    smodel = new QStandardItemModel(TotalNSpe, nmfConstants::MaxNumberAges );

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab6_ItemChanged(QStandardItem *)));

    if (std::stoi(dataMap["SeasSpaceO"][0]) == 1) {
        SeasSpaceO = 1;
        NSeasons   = std::stoi(dataMap["NSeasons"][0]);
    } else {
        SeasSpaceO = 0;
        NSeasons   = 1;
    } // end if

    // Load the species combo box in Tab5
    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Type=0";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        species = dataMap["SpeName"][i];
        Species.push_back(species);
        AllSpecies.push_back(species);
    }

    fields = {"MaxAge","SpeIndex"};
    for (int i = 0; i < NSpecies; ++i) {
        queryStr = "SELECT MaxAge,SpeIndex FROM " + nmfConstantsMSVPA::TableSpecies +
                   " WHERE Spename = '" + Species[i] + "'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
         Nage.push_back(std::stoi(dataMap["MaxAge"][0]));
        //PredIndex.push_back(std::stoi(dataMap["SpeIndex"][0]));
        PredType.push_back(0);
    } // end for i

    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Type=3";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        species = dataMap["SpeName"][i];
        OthPred.push_back(species);
    }

    // Add other preds to the lists
    fields = {"NumSizeCats","SpeIndex"};
    for (int i = NSpecies; i < NPreds; ++i) {
        queryStr = "SELECT NumSizeCats,SpeIndex FROM " +
                    nmfConstantsMSVPA::TableOtherPredSpecies +
                   " WHERE SpeName = '" + OthPred[i-NSpecies] + "'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        Nage.push_back(std::stoi(dataMap["NumSizeCats"][0])-1);
        //PredIndex.push_back(std::stoi(dataMap["SpeIndex"][0]));
        PredType.push_back(1);
    } // end for i

    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Type=1";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        species = dataMap["SpeName"][i];
        Prey.push_back(species);
        AllSpecies.push_back(species);
    }

    fields   = {"OthPreyName"};
    queryStr = "SELECT OthPreyName FROM " + nmfConstantsMSVPA::TableMSVPAOthPrey +
               " WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["OthPreyName"].size(); ++i) {
        species = dataMap["OthPreyName"][i];
        OthPrey.push_back(species);
        AllSpecies.push_back(species);
    }

    if (Nage.size() == 0) {
        msg = "\nNo age structured species found.";
        QMessageBox::information(MSVPA_Tabs,
                                 tr("Error"),
                                 tr(msg.c_str()),
                                 QMessageBox::Ok);
        return false;
    }

    int numRows = AllSpecies.size();
    int numCols = Nage[0]+1;

    // Set up columns and rows
    smodel->setRowCount(AllSpecies.size());
    smodel->setColumnCount(numCols);
    makeHeaders(numCols,AllSpecies);

    // Update species combobox
    MSVPA_Tab6_SpeciesCMB->blockSignals(true);
    MSVPA_Tab6_SpeciesCMB->clear();
    for (unsigned int i=0;i<Species.size(); ++i) {
        MSVPA_Tab6_SpeciesCMB->addItem(QString::fromStdString(Species[i]));
    }
    for (unsigned int i=0;i<OthPred.size(); ++i) {
        MSVPA_Tab6_SpeciesCMB->addItem(QString::fromStdString(OthPred[i]));
    }
    MSVPA_Tab6_SpeciesCMB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    MSVPA_Tab6_SpeciesCMB->blockSignals(false);

    // Update seasons combobox
    QString str = "Season ";
    MSVPA_Tab6_SeasonCMB->blockSignals(true);
    MSVPA_Tab6_SeasonCMB->clear();
    for (int i=0;i<NSeasons; ++i) {
        MSVPA_Tab6_SeasonCMB->addItem(str+QString::number(i+1));
    }
    MSVPA_Tab6_SeasonCMB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    MSVPA_Tab6_SeasonCMB->blockSignals(false);

    std::string SpeName = MSVPA_Tab6_SpeciesCMB->currentText().toStdString();

    // Load the model which will then load the table
    int m = 0;

    fields = {"SpOverlap"};
    queryStr = "SELECT SpOverlap FROM " + nmfConstantsMSVPA::TableMSVPASpaceO +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND SpeName = '"  + SpeName  + "'" +
               " AND Season = 1" +
               " ORDER By Age";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpOverlap"].size();
    if (NumRecords > 0) {
        if (NumRecords == numRows*numCols) {
            for (int j = 0; j < numCols; ++j) {
                for (int i=0; i<numRows; ++i) {
                    item = new QStandardItem(QString::fromStdString(dataMap["SpOverlap"][m]));
                    item->setTextAlignment(Qt::AlignCenter);
                    smodel->setItem(i,j,item);
                    ++m;
                } // end for i
            } // end for j
        } else {
            msg = "\nNumber of pertinent records in MSVPASpaceO (" + std::to_string(NumRecords) + ")\n";
            msg += "does not equal number of visible table cells (" + std::to_string(numCols);
            msg += "x" + std::to_string(numRows) + ")";
            QMessageBox::information(MSVPA_Tabs,
                                     tr("Database Problem"),
                                     tr(msg.c_str()),
                                     QMessageBox::Ok);
            return false;
        }
    } else {
        for (int j = 0; j < numCols; ++j) {
            for (int i=0; i<numRows; ++i) {
                item = new QStandardItem("");
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i,j,item);
            } // end for i
        } // end for j
    }

    MSVPA_Tab6_SpOverlapTV->setModel(smodel);

    MSVPA_Tab6_SavePB->setEnabled(false);

    bool ifDataExist = nmfUtilsQt::allCellsArePopulated(MSVPA_Tabs,
                                                        MSVPA_Tab6_SpOverlapTV,
                                                        nmfConstants::DontShowError);
    MSVPA_Tab6_NextPB->setEnabled(ifDataExist);

    MarkAsClean();

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab6::loadWidgets Complete");

    return true;

} // end loadWidgets


void
nmfMSVPATab6::makeHeaders(int nCols, std::vector<std::string> AllSpecies)
{
    QStringList strList;

    // Make row headers
    for (unsigned int k=0; k<AllSpecies.size(); ++k) {
        strList << QString::fromStdString(AllSpecies[k]);
    }
    smodel->setVerticalHeaderLabels(strList);

    // Make column headers
    strList.clear();
    for (int k=0; k<nCols; ++k) {
        strList << "Age " + QString::number(k);
    }
    smodel->setHorizontalHeaderLabels(strList);

} // end makeHeaders





void
nmfMSVPATab6::callback_MSVPA_Tab6_SpeciesCMB(int index)
{
    int m;
    int NumRecords;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    QStandardItem *item;
    std::string SpeName = MSVPA_Tab6_SpeciesCMB->currentText().toStdString();
    std::string Season  = std::to_string(MSVPA_Tab6_SeasonCMB->currentIndex()+1);

    smodel->clear();

    disconnect(smodel, SIGNAL(itemChanged(QStandardItem *)),
               this,   SLOT(callback_MSVPA_Tab6_ItemChanged(QStandardItem *)));

    int numRows = AllSpecies.size();
    int numCols = Nage[index]+1;

    makeHeaders(numCols,AllSpecies);
    smodel->setRowCount(numRows);
    smodel->setColumnCount(numCols);

    // Load the model which will then load the table
    fields   = {"SpOverlap"};
    queryStr = "SELECT SpOverlap FROM " + nmfConstantsMSVPA::TableMSVPASpaceO +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND SpeName = '" + SpeName + "'" +
               " AND Season = "   + Season +
               " ORDER By Age";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpOverlap"].size();
    if (NumRecords > 0) {
        m = 0;
        for (int j = 0; j < numCols; ++j) {
            for (int i=0; i < numRows; ++i) {
                item = new QStandardItem(QString::fromStdString(dataMap["SpOverlap"][m++]));
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, j, item);
            } // end for i
        } // end for j
    } else {
        for (int j = 0; j < numCols; ++j) {
            for (int i=0; i < numRows; ++i) {
                item = new QStandardItem("");
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, j, item);
            } // end for i
        } // end for j
    }

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab6_ItemChanged(QStandardItem *)));


} // end callback_MSVPA_Tab6_SpeciesCMB

void
nmfMSVPATab6::callback_MSVPA_Tab6_SeasonCMB(int index)
{
    int m;
    int NumRecords;
    std::string msg;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    QStandardItem *item;
    std::string SpeName  = MSVPA_Tab6_SpeciesCMB->currentText().toStdString();
    int SpeIndex         = MSVPA_Tab6_SpeciesCMB->currentIndex();
    std::string Season   = std::to_string(index+1);

    smodel->clear();

    disconnect(smodel, SIGNAL(itemChanged(QStandardItem *)),
               this,   SLOT(callback_MSVPA_Tab6_ItemChanged(QStandardItem *)));

    int numRows = AllSpecies.size();
    int numCols = Nage[SpeIndex]+1;
    makeHeaders(numCols,AllSpecies);
    smodel->setRowCount(numRows);
    smodel->setColumnCount(numCols);

    // Load the model which will then load the table
    fields   = {"SpOverlap"};
    queryStr = "SELECT SpOverlap FROM " + nmfConstantsMSVPA::TableMSVPASpaceO +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND SpeName = '" + SpeName + "'" +
               " AND Season = "   + Season +
               " ORDER By Age";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpOverlap"].size();
    if (NumRecords > 0) {
        if (NumRecords == numRows*numCols) {
            m = 0;
            for (int j = 0; j < numCols; ++j) {
                for (int i=0; i < numRows; ++i) {
                    item = new QStandardItem(QString::fromStdString(dataMap["SpOverlap"][m++]));
                    item->setTextAlignment(Qt::AlignCenter);
                    smodel->setItem(i, j, item);
                } // end for i
            } // end for j
        } else {
            msg = "\nNumber of pertinent records in MSVPASpaceO CSV file (" + std::to_string(NumRecords) + ")\n";
            msg += "does not equal number of visible table cells ( " + std::to_string(numCols);
            msg += "cols x " + std::to_string(numRows) + "rows )";
            QMessageBox::information(MSVPA_Tabs,
                                     tr("Database Load Problem"),
                                     tr(msg.c_str()),
                                     QMessageBox::Ok);
            return;
        }
    } else {
        for (int j = 0; j < numCols; ++j) {
            for (int i=0; i < numRows; ++i) {
                item = new QStandardItem("");
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, j, item);
            } // end for i
        } // end for j
    }

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab6_ItemChanged(QStandardItem *)));


} // end callback_MSVPA_Tab6_SpeciesCMB

void
nmfMSVPATab6::callback_MSVPA_Tab6_PrevPB(bool unused)
{
    int prevPage = MSVPA_Tabs->currentIndex()-1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(prevPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(prevPage);
    emit UpdateNavigator("MSVPA",prevPage);
}

void
nmfMSVPATab6::callback_MSVPA_Tab6_NextPB(bool unused)
{
    int nextPage = MSVPA_Tabs->currentIndex()+1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(nextPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(nextPage);
    emit UpdateNavigator("MSVPA",nextPage);
}
