
#include "nmfConstants.h"
#include "nmfUtils.h"
#include "nmfUtilsQt.h"

#include "nmfMSVPATab05.h"



nmfMSVPATab5::nmfMSVPATab5(QTabWidget *tabs,
                           nmfLogger *theLogger,
                           std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab5::nmfMSVPATab5");

    MSVPA_Tabs = tabs;
    Enabled = false;
    smodel = NULL;
    databasePtr = NULL;
    MSVPAPrefsCSVFile.clear();
    ProjectDir = theProjectDir;

    // Load ui as a widget from disk
    QFile file(":/forms/MSVPA/MSVPA_Tab05.ui");
    file.open(QFile::ReadOnly);
    MSVPA_Tab5_Widget = loader.load(&file,MSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    MSVPA_Tabs->addTab(MSVPA_Tab5_Widget, tr("5. Prey Preferences"));

    // Setup connections
    MSVPA_Tab5_PrevPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab5_PrevPB");
    MSVPA_Tab5_NextPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab5_NextPB");
    MSVPA_Tab5_SavePB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab5_SavePB");
    MSVPA_Tab5_LoadPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab5_LoadPB");
    MSVPA_Tab5_ClearPB     = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab5_ClearPB");
    MSVPA_Tab5_SpeciesCMB  = MSVPA_Tabs->findChild<QComboBox   *>("MSVPA_Tab5_SpeciesCMB");
    MSVPA_Tab5_PreyPrefsTV = MSVPA_Tabs->findChild<QTableView  *>("MSVPA_Tab5_PreyPrefsTV");

    MSVPA_Tab5_PrevPB->setText("\u25C1--");
    MSVPA_Tab5_NextPB->setText("--\u25B7");

    connect(MSVPA_Tab5_LoadPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab5_LoadPB(bool)));
    connect(MSVPA_Tab5_SavePB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab5_SavePB(bool)));
    connect(MSVPA_Tab5_PrevPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab5_PrevPB(bool)));
    connect(MSVPA_Tab5_NextPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab5_NextPB(bool)));
    connect(MSVPA_Tab5_ClearPB, SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab5_ClearPB(bool)));
    connect(MSVPA_Tab5_SpeciesCMB, SIGNAL(currentIndexChanged(int)),
            this,                  SLOT(callback_MSVPA_Tab5_SpeciesCMB(int)));

    MSVPA_Tab5_SavePB->setEnabled(false);
    MSVPA_Tab5_NextPB->setEnabled(false);

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab5::nmfMSVPATab5 Complete");

}


nmfMSVPATab5::~nmfMSVPATab5()
{
    //std::cout << "Freeing Tab4 memory" << std::endl;
    clearWidgets();
}

void
nmfMSVPATab5::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir



void nmfMSVPATab5::clearWidgets()
{
    if (smodel)
        smodel->clear();
}


void
nmfMSVPATab5::callback_RemoveAPreySpecies(std::string,
                                          int numPreyBeforeDelete,
                                          int preyToDeleteIndex)
{
std::cout << "\n*** *** Remove a prey species...\n" << std::endl;

    int numRows = smodel->rowCount();

    smodel->removeRow(numRows-numPreyBeforeDelete+preyToDeleteIndex);

    callback_MSVPA_Tab5_SavePB(true);

} // end callback_RemoveAPreySpecies



void
nmfMSVPATab5::callback_MSVPA_Tab5_SavePB(bool unused)
{
    bool saveOK = true;
    bool FirstLine;
    QString fileName;
    QString filePath;
    QString fileNameWithPathPrefs;
    QString tmpFileNameWithPathPrefs;
    QString qcmd;
    QString line;
    QString TableNamePrefs;
    QString tableLine;
    QStringList qfields;
    std::string cmd;
    std::string errorMsg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    bool ignoreMSVPAName = false;
    int retv;
    QString csvMSVPAName;
    QString csvSpeName;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab5::callback_MSVPA_Tab5_SavePB");

    //
    // Save info to MSVPAprefs
    //
    if (! nmfUtilsQt::allCellsArePopulated(MSVPA_Tabs,MSVPA_Tab5_PreyPrefsTV,
                                           nmfConstants::ShowError))
        return;

    // Build a Species and OtherPredSpecies index and type map, you'll need these.
    std::map<std::string,int> SpeciesIndexMap;
    std::map<std::string,int> SpeciesTypeMap;
    fields   = {"MSVPAName", "SpeName", "SpeIndex", "Type"};
    queryStr = "SELECT MSVPAName,SpeName,SpeIndex,Type FROM MSVPAspecies WHERE MSVPAName = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
        SpeciesIndexMap[dataMap["SpeName"][i]] = std::stoi(dataMap["SpeIndex"][i]);
        SpeciesTypeMap[dataMap["SpeName"][i]]  = std::stoi(dataMap["Type"][i]);
    }

    TableNamePrefs = "MSVPAprefs";

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (MSVPAPrefsCSVFile.isEmpty()) {
        MSVPAPrefsCSVFile = TableNamePrefs + ".csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPathPrefs = QDir(filePath).filePath(MSVPAPrefsCSVFile);
        tmpFileNameWithPathPrefs = QDir(filePath).filePath("."+MSVPAPrefsCSVFile);
    } else {
        QFileInfo finfo(MSVPAPrefsCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPathPrefs    = MSVPAPrefsCSVFile;
        tmpFileNameWithPathPrefs = QDir(filePath).filePath("."+fileName);
    }


    // Read entire file and copy all lines that don't have the current MSVPAName into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    QFile fin(fileNameWithPathPrefs);
    QFile fout(tmpFileNameWithPathPrefs);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(4a) Opening Input File",
                                 "\n"+fin.errorString()+": "+fileNameWithPathPrefs);
        return;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(4a) Opening Output File",
                                 "\n"+fout.errorString()+": "+tmpFileNameWithPathPrefs);
        return;
    }

    QTextStream inStream(&fin);
    QTextStream outStream(&fout);

    QModelIndex index;
    ignoreMSVPAName = false;
    int numRows = smodel->rowCount();
    int numCols = smodel->columnCount();
    QString SpeName = MSVPA_Tab5_SpeciesCMB->currentText();
    QString Age;
    QString PreyName;
    QString csvAge,csvRank;

    //
    // PrefVal is calculated as = (PreyCount - RefRank + 1) /
    //                            (Sum of all PrefRanks in the age group)
    // But this needs to be done in a loop after the PrefRanks are initially saved.
    //
    QString PrefVal = "0"; // RSK - todo figure out how to calculate this....

    // Need to keep track of total number of non-zero rank prey and sum of all ranks
    // for each age group.
    std::map<QString,double> TotalNumNonZeroRankPrey;
    std::map<QString,double> SumOfAllRanks;
    SumOfAllRanks.clear();
    double PrefRank;
    TotalNumNonZeroRankPrey.clear();
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvMSVPAName = qfields[0].trimmed();
        csvSpeName   = qfields[3].trimmed();
        if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
            (csvSpeName == SpeName) &&
            (! csvMSVPAName.isEmpty())) { // dump qtableview data into output file
            // Skip over all other MSVPAName rows, since going through this once copies the qtableview data
            if (! ignoreMSVPAName) {
                for (int j=0; j<numCols; ++j) {
                    Age = smodel->horizontalHeaderItem(j)->text().split(" ")[1];
                    TotalNumNonZeroRankPrey[Age] = 0;
                    for (int i=0; i<numRows; ++i) {
                        PreyName = smodel->verticalHeaderItem(i)->text().trimmed();
                        index = MSVPA_Tab5_PreyPrefsTV->model()->index(i,j);
                        PrefRank = index.data().toDouble();
                        if (PrefRank > 0) {
                            ++TotalNumNonZeroRankPrey[Age];
                        }
                        SumOfAllRanks[Age] += PrefRank;
                        tableLine = QString::fromStdString(MSVPAName)    + ", " +
                                QString::number(SpeciesIndexMap[SpeName.toStdString()]) + ", " +
                                QString::number(SpeciesTypeMap[ SpeName.toStdString()]) + ", " +
                                SpeName                                  + ", " +
                                Age                                      + ", " +
                                QString::number(i)                       + ", " +
                                PreyName                                 + ", " +
                                index.data().toString()                  + ", " +
                                PrefVal;
                        outStream << tableLine << "\n";
                    } // end for i
                } // end for j
                ignoreMSVPAName = true;
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }
    }
    if (! ignoreMSVPAName) { // means never found the MSVPAName
        for (int j=0; j<numCols; ++j) {
            Age = smodel->horizontalHeaderItem(j)->text().split(" ")[1];
            TotalNumNonZeroRankPrey[Age] = 0;
            for (int i=0; i<numRows; ++i) {
                PreyName = smodel->verticalHeaderItem(i)->text().trimmed();
                index = MSVPA_Tab5_PreyPrefsTV->model()->index(i,j);
                PrefRank = index.data().toDouble();
                if (PrefRank > 0) {
                    ++TotalNumNonZeroRankPrey[Age];
                }
                SumOfAllRanks[Age] += PrefRank;
                tableLine = QString::fromStdString(MSVPAName)    + ", " +
                        QString::number(SpeciesIndexMap[SpeName.toStdString()]) + ", " +
                        QString::number(SpeciesTypeMap[ SpeName.toStdString()]) + ", " +
                        SpeName                                  + ", " +
                        Age                                      + ", " +
                        QString::number(i)                       + ", " +
                        PreyName                                 + ", " +
                        index.data().toString()                  + ", " +
                        PrefVal;
                outStream << tableLine << "\n";
            } // end for i
        } // end for j
    }
    fin.close();
    fout.close();

    // --------------------------------------------

    // Now, mv temp file to actual file.
    retv = nmfUtilsQt::rename(tmpFileNameWithPathPrefs,
                                  fileNameWithPathPrefs);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPathPrefs.toStdString() << " to " <<
                     fileNameWithPathPrefs.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return;
    }
    // --------------------------------------------

    double PrefValDbl;
    // One more step.  Go through the file again and this time update the PrefVal field.
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(4b) Opening Input File",
                                 "\n"+fin.errorString()+": "+fileNameWithPathPrefs);
        return;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(4b) Opening Output File",
                                 "\n"+fout.errorString()+": "+tmpFileNameWithPathPrefs);
        return;
    }

    inStream.setDevice(&fin);
    outStream.setDevice(&fout);
    ignoreMSVPAName = false;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvMSVPAName = qfields[0].trimmed();
        csvSpeName   = qfields[3].trimmed();
        csvAge       = qfields[4].trimmed();
        csvRank      = qfields[7].trimmed();
        if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
            (csvSpeName == SpeName) &&
            (! csvMSVPAName.isEmpty())) { // dump qtableview data into output file
                        if (csvRank.toInt() == 0)
                            PrefValDbl = 0.0;
                        else
                            PrefValDbl = (TotalNumNonZeroRankPrey[csvAge] - csvRank.toInt() + 1) / (double)SumOfAllRanks[csvAge];
                        tableLine = QString::fromStdString(MSVPAName) + ", " +
                                    qfields[1].trimmed() + ", " +
                                    qfields[2].trimmed() + ", " +
                                    qfields[3].trimmed() + ", " +
                                    qfields[4].trimmed() + ", " +
                                    qfields[5].trimmed() + ", " +
                                    qfields[6].trimmed() + ", " +
                                    qfields[7].trimmed() + ", " +
                                    QString::number(PrefValDbl);
                       outStream << tableLine << "\n";

        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }
    }
    fin.close();
    fout.close();

    // ----------------------------------

    // Now, mv temp file to actual file.
    retv = nmfUtilsQt::rename(tmpFileNameWithPathPrefs,
                                  fileNameWithPathPrefs);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPathPrefs.toStdString() << " to " <<
                     fileNameWithPathPrefs.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return;
    }
    // --------------------------------------------

    //
    // Save MSVPAprefs mysql table
    //
    // 1. Check that the table exists, if not, create it.
    //databasePtr->checkForTableAndCreate(TableNamePrefs);

    // 2. Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableNamePrefs;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
        saveOK = false;
        nmfUtils::printError("MSVPA Save(4a): Clearing table error: ",
                             errorMsg+": "+TableNamePrefs.toStdString());
    }

    // 3. Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableNamePrefs.toStdString();
    cmd += " (MSVPAName,SpeIndex,SpeType,SpeName,Age,PreyIndex,PreyName,PrefRank,PrefVal) VALUES ";

    // Read from csv file since it has all the MSVPANames in it
    fin.setFileName(fileNameWithPathPrefs);
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(4b) Opening Input File",
                                 "\n"+fin.errorString()+": " +fileNameWithPathPrefs);
        return;
    }
    logger->logMsg(nmfConstants::Normal,"Opened for reading: " + fileNameWithPathPrefs.toStdString());

    inStream.setDevice(&fin);
    //inStream.seek(0);

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
        cmd +=  "\"" + qfields[3].trimmed().toStdString() + "\",";
        cmd +=         qfields[4].trimmed().toStdString() + ",";
        cmd +=         qfields[5].trimmed().toStdString() + ",";
        cmd +=  "\"" + qfields[6].trimmed().toStdString() + "\",";
        cmd +=         qfields[7].trimmed().toStdString() + ",";
        cmd +=         qfields[8].trimmed().toStdString() + "), ";

    } // end for all rows in csv file
    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);

    fin.close();
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("MSVPA Save(4b): Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        return;
    }


    if (saveOK) {
        QMessageBox::information(MSVPA_Tabs, "Save",
                                 tr("\nMSVPA Prefs data saved.\n"));
    }


    MSVPAPrefsCSVFile.clear();

    MSVPA_Tab5_NextPB->setEnabled(true);

    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab5::callback_MSVPA_Tab5_SavePB Complete");

} // end callback_MSVPA_Tab5_SavePB


void
nmfMSVPATab5::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    TableName = "MSVPAprefs";
    fields    = {"MSVPAName","SpeIndex","SpeType","SpeName","Age",
                "PreyIndex","PreyName","PrefRank","PrefVal"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase


void
nmfMSVPATab5::callback_MSVPA_Tab5_LoadPB(bool unused)
{
    int m=0;
    int csvFileRow = 0;
    QString line;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(MSVPA_Tabs);
    QStringList NameFilters;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString MSVPAPrefsCSVFile;
    std::vector<double> PrefData;
    QStandardItem *item;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab5::callback_MSVPA_Tab5_LoadPB");

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile("MSVPAprefs.csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load MSVPA Prefs CSV File");
    if (fileDlg.exec()) {

        // Open the file here....
        MSVPAPrefsCSVFile = fileDlg.selectedFiles()[0];
        QFile file(MSVPAPrefsCSVFile);
        if(! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(MSVPA_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);

        PrefData.clear();
        QString csvMSVPAName, csvSpeIndex, csvSpeType, csvSpeName, csvAge;
        QString csvPreyIndex, csvPreyName, csvPrefRank, csvPrefVal;
        // Store data to load in next step
        QStringList fields2;
        QString PreyName = MSVPA_Tab5_SpeciesCMB->currentText().trimmed();
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header
                fields2 = line.split(",");
                csvMSVPAName = fields2[0].trimmed();
                csvSpeIndex  = fields2[1].trimmed();
                csvSpeType   = fields2[2].trimmed();
                csvSpeName   = fields2[3].trimmed();
                csvAge       = fields2[4].trimmed();
                csvPreyIndex = fields2[5].trimmed();
                csvPreyName  = fields2[6].trimmed();
                csvPrefRank  = fields2[7].trimmed();
                csvPrefVal   = fields2[8].trimmed();
                if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
                    (csvSpeName == PreyName))
                {
                    PrefData.push_back(csvPrefRank.toDouble());
                }
            }
            ++csvFileRow;
        }
        file.close();

        // Now load the data
        int numRows = smodel->rowCount();
        int numCols = smodel->columnCount();
        for (int j=0; j<numCols; ++j) {
            for (int i=0; i < numRows; ++i) {
                item = new QStandardItem(QString::number(PrefData[m++]));
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, j, item);
            }
        }

        MSVPA_Tab5_PreyPrefsTV->setModel(smodel);

        MSVPA_Tab5_SavePB->setEnabled(true);
        MSVPA_Tab5_NextPB->setEnabled(false);
    }

    MSVPA_Tab5_LoadPB->clearFocus();

    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab5::callback_MSVPA_Tab5_LoadPB Complete");

} // end callback_MSVPA_Tab5_LoadPB


/*
 *
 * This function updates the data structure.  I still need to implement the Save
 * mechanism to update the database....TBD.
 */
void
nmfMSVPATab5::callback_MSVPA_Tab5_ItemChanged(QStandardItem *item)
{
    item->setTextAlignment(Qt::AlignCenter);
    smodel->setItem(item->row(), item->column(), item);

    MarkAsDirty("MSVPAprefs");
    MSVPA_Tab5_SavePB->setEnabled(true);
    MSVPA_Tab5_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab5_ItemChanged

void
nmfMSVPATab5::MarkAsDirty(std::string tableName)
{
    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfMSVPATab5::MarkAsClean()
{
    emit MarkMSVPAAsClean();
} // end MarkAsClean

bool
nmfMSVPATab5::loadWidgets(nmfDatabase *theDatabasePtr,
                               std::string theMSVPAName)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::string species;
    std::vector<std::string> Species;
    std::vector<std::string> OthPred;
    std::vector<std::string> Prey;
    std::vector<std::string> OthPrey;

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab5::loadWidgets");

    databasePtr = theDatabasePtr;
    MSVPAName   = theMSVPAName;

    AllSpecies.clear();
    Nage.clear();
    PredType.clear();

    fields   = {"NSpe","NPreyOnly","NOther","NOtherPred"};
    queryStr = "SELECT NSpe,NPreyOnly,NOther,NOtherPred FROM MSVPAlist WHERE MSVPAName = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    int NPrey      = std::stoi(dataMap["NPreyOnly"][0]);
    int NSpe       = std::stoi(dataMap["NSpe"][0]);
    int NOtherPrey = std::stoi(dataMap["NOther"][0]);
    int NOtherPred = std::stoi(dataMap["NOtherPred"][0]);
    int NPreds     = NSpe + NOtherPred;
    int TotalNSpe  = NSpe + NPrey + NOtherPrey;
    if (TotalNSpe == 0) {
        QMessageBox::information(MSVPA_Tabs,
                                 tr("No Species Found"),
                                 tr("\nYou must define one or more Species before entering Prey Preferences."),
                                 QMessageBox::Ok);
        return false;
    }

    AllSpecies.clear();

    smodel = new QStandardItemModel(TotalNSpe, nmfConstants::MaxNumberAges );

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab5_ItemChanged(QStandardItem *)));

    // Load the species combo box in Tab4
    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Type=0";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        species = dataMap["SpeName"][i];
        Species.push_back(species);
        AllSpecies.push_back(species);
    }

    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Type=3";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        species = dataMap["SpeName"][i];
        OthPred.push_back(species);
    }

    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Type=1";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        species = dataMap["SpeName"][i];
        Prey.push_back(species);
        AllSpecies.push_back(species);
    }

    fields   = {"OthPreyName"};
    queryStr = "SELECT OthPreyName FROM MSVPAOthPrey WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["OthPreyName"].size(); ++i) {
        species = dataMap["OthPreyName"][i];
        OthPrey.push_back(species);
        AllSpecies.push_back(species);
    }

    fields = {"MaxAge","SpeIndex"};
    for (int i = 0; i < NSpe; ++i) {
        queryStr = "SELECT MaxAge,SpeIndex FROM Species WHERE Spename = '" + Species[i] + "'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
         Nage.push_back(std::stoi(dataMap["MaxAge"][0]));
        //PredIndex.push_back(std::stoi(dataMap["SpeIndex"][0]));
        PredType.push_back(0);
    } // end for i

    // Add other preds to the lists
    fields = {"NumSizeCats","SpeIndex"};
    for (int i = NSpe; i < NPreds; ++i) {
        queryStr = "SELECT NumSizeCats,SpeIndex FROM OtherPredSpecies WHERE SpeName = '" + OthPred[i-NSpe] + "'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        Nage.push_back(std::stoi(dataMap["NumSizeCats"][0])-1);
        //PredIndex.push_back(std::stoi(dataMap["SpeIndex"][0]));
        PredType.push_back(1);
    } // end for i

    smodel->setRowCount(AllSpecies.size());
    smodel->setColumnCount(Nage[0]+1);

    makeHeaders(AllSpecies,Nage[0],PredType[0]);

    // Load Species combo box
    MSVPA_Tab5_SpeciesCMB->blockSignals(true);
    MSVPA_Tab5_SpeciesCMB->clear();
    for (unsigned int i=0;i<Species.size(); ++i) {
        MSVPA_Tab5_SpeciesCMB->addItem(QString::fromStdString(Species[i]));
    }
    for (unsigned int i=0;i<OthPred.size(); ++i) {
        MSVPA_Tab5_SpeciesCMB->addItem(QString::fromStdString(OthPred[i]));
    }
    MSVPA_Tab5_SpeciesCMB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    MSVPA_Tab5_SpeciesCMB->blockSignals(false);

/*
    std::string SpeName = MSVPA_Tab5_SpeciesCMB->currentText().toStdString();

    // Load the model which will then load the table
    int m = 0;
    int numRows = smodel->rowCount();
    fields   = {"PrefRank"};
    queryStr = "SELECT PrefRank FROM MSVPAprefs WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND SpeName = '" + SpeName + "'" +
               " ORDER By Age";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["PrefRank"].size();
    if (NumRecords > 0) {
        m = 0;
        if (NumRecords != Nage[0]*numRows) {  // In case user has added a prey in Tab 4, show a blank row
            numRows = int(NumRecords/Nage[0])-1;  // RSK - recheck this logic
        }
        for (int j = 0; j <= Nage[0]; ++j) {
            for (int i=0; i < numRows; ++i) {
                item = new QStandardItem(QString::fromStdString(dataMap["PrefRank"][m++]));
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, j, item);
            } // end for i
        } // end for j
    } else {
        for (int j = 0; j <= Nage[0]; ++j) {
            for (int i=0; i < numRows; ++i) {
                item = new QStandardItem("");
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, j, item);
            } // end for i
        } // end for j
    }
*/
    MSVPA_Tab5_PreyPrefsTV->setModel(smodel);

    // Update widgets
    callback_MSVPA_Tab5_SpeciesCMB(0);

    MSVPA_Tab5_SavePB->setEnabled(false);

    bool ifDataExist = nmfUtilsQt::allCellsArePopulated(MSVPA_Tabs,
                                                        MSVPA_Tab5_PreyPrefsTV,
                                                        nmfConstants::DontShowError);
    MSVPA_Tab5_NextPB->setEnabled(ifDataExist);

    MarkAsClean();

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab5::loadWidgets Complete");

    return true;
} // end loadWidgets


void
nmfMSVPATab5::callback_MSVPA_Tab5_ClearPB(bool unused)
{
    std::string errorMsg;
    QString qcmd;
    QString TableName = "MSVPAprefs";
    bool clearedOK = true;
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(MSVPA_Tabs,
                                  tr("Clear"),
                                  tr("\nOK to Clear MSVPAprefs CSV and database tables?"),
                                  QMessageBox::No|QMessageBox::Yes);
    if (reply == QMessageBox::Yes)
    {
        // Clear CSV file
        errorMsg.clear();
        clearedOK = nmfUtilsQt::clearCSVFile(TableName.toStdString(),
                                             ProjectDir,
                                             errorMsg);
        if (! clearedOK)
            return;

        // Clear out the MSVPAprefs database table
        qcmd = "TRUNCATE TABLE " + TableName;
        errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
        if (nmfUtilsQt::isAnError(errorMsg)) {
            nmfUtils::printError("MSVPA Clear(5): Clearing table error: ",
                                 errorMsg+": "+TableName.toStdString());
            return;
        }

        QMessageBox::information(MSVPA_Tabs,
                                 tr("Clear"),
                                 tr("\nMSVPAprefs CSV and database tables cleared."),
                                 QMessageBox::Ok);

        // Reload tab
        loadWidgets(databasePtr, MSVPAName);

    } // end if reply == Yes

} // end callback_MSVPA_Tab5_ClearPB

void
nmfMSVPATab5::makeHeaders(std::vector<std::string> AllSpecies,
                         int nCols,
                         int PredType)
{
    QStringList strList;

    // Make row headers
    for (unsigned int j=0; j<AllSpecies.size(); ++j) {
        strList << " " + QString::fromStdString(AllSpecies[j]) + " ";
    }
    smodel->setVerticalHeaderLabels(strList);

    // Make column headers
    strList.clear();
    if (PredType == 0) {
        for (int l=0; l<=nCols; ++l) {
            strList << "Age " + QString::number(l);
        }
    } else {
        if (nCols == 0) {
            strList << "Pref";
        } else {
            for (int l=0; l<=nCols; ++l) {
                strList << "Size " + QString::number(l);
            }
        }
    }
    smodel->setHorizontalHeaderLabels(strList);

} // end makeHeaders


void
nmfMSVPATab5::callback_MSVPA_Tab5_SpeciesCMB(int index)
{
    int m;
    int NumRecords;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    QStandardItem *item;
    std::string SpeName = MSVPA_Tab5_SpeciesCMB->currentText().toStdString();

    smodel->clear();

    disconnect(smodel, SIGNAL(itemChanged(QStandardItem *)),
               this,   SLOT(callback_MSVPA_Tab5_ItemChanged(QStandardItem *)));

    makeHeaders(AllSpecies,Nage[index],PredType[index]);

    int numRows = AllSpecies.size();

    smodel->setRowCount(numRows);
    smodel->setColumnCount(Nage[index]+1);

    // Load the model which will then load the table
    fields   = {"PrefRank"};
    queryStr = "SELECT PrefRank FROM MSVPAprefs WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND SpeName = '" + SpeName + "'" +
               " ORDER By Age";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["PrefRank"].size();
    if (NumRecords > 0) {
        m = 0;
        for (int j = 0; j <= Nage[index]; ++j) {
            for (int i=0; i < numRows; ++i) {
                if (m < NumRecords) {
                    item = new QStandardItem(QString::fromStdString(dataMap["PrefRank"][m++]));
                    item->setTextAlignment(Qt::AlignCenter);
                    smodel->setItem(i, j, item);
                }
            } // end for i
        } // end for j
    } else {
        for (int j = 0; j <= Nage[index]; ++j) {
            for (int i=0; i < numRows; ++i) {
                item = new QStandardItem("");
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, j, item);
            } // end for i
        } // end for j
    }

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab5_ItemChanged(QStandardItem *)));

} // end callback_MSVPA_Tab5_SpeciesCMB


void
nmfMSVPATab5::callback_MSVPA_Tab5_PrevPB(bool unused)
{
    int prevPage = MSVPA_Tabs->currentIndex()-1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(prevPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(prevPage);
    emit UpdateNavigator("MSVPA",prevPage);
}

void
nmfMSVPATab5::callback_MSVPA_Tab5_NextPB(bool unused)
{
    int nextPage = MSVPA_Tabs->currentIndex()+1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(nextPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(nextPage);
    emit UpdateNavigator("MSVPA",nextPage);
}
