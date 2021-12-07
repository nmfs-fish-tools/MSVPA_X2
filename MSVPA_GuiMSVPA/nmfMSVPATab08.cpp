
#include "nmfConstants.h"
#include "nmfUtils.h"
#include "nmfUtilsQt.h"

#include "nmfMSVPATab08.h"



nmfMSVPATab8::nmfMSVPATab8(QTabWidget *tabs,
                           nmfLogger  *theLogger,
                           std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab8::nmfMSVPATab8");

    MSVPA_Tabs = tabs;
    ProjectDir = theProjectDir;
    Enabled = false;
    smodel = NULL;
    databasePtr = NULL;
    MSVPAStomContCSVFile.clear();

    // Load ui as a widget from disk
    QFile file(":/forms/MSVPA/MSVPA_Tab08.ui");
    file.open(QFile::ReadOnly);
    MSVPA_Tab8_Widget = loader.load(&file,MSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    MSVPA_Tabs->addTab(MSVPA_Tab8_Widget, tr("8. Gut Fullness"));

    // Setup connections
    MSVPA_Tab8_LoadPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab8_LoadPB");
    MSVPA_Tab8_SavePB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab8_SavePB");
    MSVPA_Tab8_PrevPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab8_PrevPB");
    MSVPA_Tab8_NextPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab8_NextPB");
    MSVPA_Tab8_SpeciesCMB  = MSVPA_Tabs->findChild<QComboBox   *>("MSVPA_Tab8_SpeciesCMB");
    MSVPA_Tab8_PreyPrefsTV = MSVPA_Tabs->findChild<QTableView  *>("MSVPA_Tab8_PreyPrefsTV");

    MSVPA_Tab8_PrevPB->setText("\u25C1--");
    MSVPA_Tab8_NextPB->setText("--\u25B7");

    connect(MSVPA_Tab8_PrevPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab8_PrevPB(bool)));
    connect(MSVPA_Tab8_NextPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab8_NextPB(bool)));
    connect(MSVPA_Tab8_LoadPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab8_LoadPB(bool)));
    connect(MSVPA_Tab8_SavePB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab8_SavePB(bool)));
    connect(MSVPA_Tab8_SpeciesCMB, SIGNAL(currentIndexChanged(int)),
            this,                  SLOT(callback_MSVPA_Tab8_SpeciesCMB(int)));

    MSVPA_Tab8_SavePB->setEnabled(false);

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab8::nmfMSVPATab8 Complete");

}


nmfMSVPATab8::~nmfMSVPATab8()
{
    //std::cout << "Freeing Tab7 memory" << std::endl;
    clearWidgets();
}


void
nmfMSVPATab8::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir


void
nmfMSVPATab8::clearWidgets()
{
    if (smodel)
        smodel->clear();
}


void
nmfMSVPATab8::callback_MSVPA_Tab8_SavePB(bool unused)
{
    bool saveOK = true;
    bool FirstLine;
    QString fileName;
    QString filePath;
    QString fileNameWithPathStomCont;
    QString tmpFileNameWithPathStomCont;
    QString qcmd;
    QString line;
    QString TableNameStomCont;
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

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab8::callback_MSVPA_Tab8_SavePB");

    //
    // Save info to MSVPAStomCont
    //
    if (! nmfUtilsQt::allCellsArePopulated(MSVPA_Tabs,MSVPA_Tab8_PreyPrefsTV,
                                           nmfConstants::ShowError))
        return;

    // Build a Species index map
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

    TableNameStomCont = QString::fromStdString(nmfConstantsMSVPA::TableMSVPAStomCont);

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (MSVPAStomContCSVFile.isEmpty()) {
        MSVPAStomContCSVFile = TableNameStomCont + ".csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPathStomCont    = QDir(filePath).filePath(MSVPAStomContCSVFile);
        tmpFileNameWithPathStomCont = QDir(filePath).filePath("."+MSVPAStomContCSVFile);
    } else {
        QFileInfo finfo(MSVPAStomContCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPathStomCont    = MSVPAStomContCSVFile;
        tmpFileNameWithPathStomCont = QDir(filePath).filePath("."+fileName);
    }

    // Read entire file and copy all lines that don't have the current MSVPAName into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    QFile fin(fileNameWithPathStomCont);
    QFile fout(tmpFileNameWithPathStomCont);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(7a) Opening Input File",
                                 "\n"+fin.errorString()+": "+fileNameWithPathStomCont);
        return;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(7a) Opening Output File",
                                 "\n"+fout.errorString()+": "+tmpFileNameWithPathStomCont);
        return;
    }

    // Write to CSV file
    QTextStream inStream(&fin);
    QTextStream outStream(&fout);
    QModelIndex index;
    ignoreMSVPAName = false;
    int numRows = smodel->rowCount();
    int numCols = smodel->columnCount();
    QString SpeName = MSVPA_Tab8_SpeciesCMB->currentText();
    QString Age;
    QString PreyName;
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
                for (int i=0; i<numRows; ++i) {
                    for (int j=0; j<numCols; ++j) {
                        index = MSVPA_Tab8_PreyPrefsTV->model()->index(i,j);
                        tableLine = QString::fromStdString(MSVPAName) + ", " +
                                QString::number(SpeciesIndexMap[SpeName.toStdString()]) + ", " +
                                QString::number(SpeciesTypeMap[SpeName.toStdString()]) + ", " +
                                SpeName              + ", " +
                                QString::number(i)   + ", " +
                                QString::number(j+1) + ", " +
                                index.data().toString();
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
        for (int i=0; i<numRows; ++i) {
            for (int j=0; j<numCols; ++j) {
                index = MSVPA_Tab8_PreyPrefsTV->model()->index(i,j);
                tableLine = QString::fromStdString(MSVPAName) + ", " +
                        QString::number(SpeciesIndexMap[SpeName.toStdString()]) + ", " +
                        QString::number(SpeciesTypeMap[SpeName.toStdString()]) + ", " +
                        SpeName              + ", " +
                        QString::number(i)   + ", " +
                        QString::number(j+1) + ", " +
                        index.data().toString();
                outStream << tableLine << "\n";
            } // end for j
        } // end for i
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    retv = nmfUtilsQt::rename(tmpFileNameWithPathStomCont,
                                  fileNameWithPathStomCont);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPathStomCont.toStdString() << " to " <<
                     fileNameWithPathStomCont.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return;
    }

    // --------------------------------------------

    //
    // Save MSVPAStomCont mysql table
    //
    // 1. Check that the table exists, if not, create it.
    databasePtr->checkForTableAndCreate(TableNameStomCont);

    // 2. Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableNameStomCont;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
        saveOK = false;
        nmfUtils::printError("MSVPA Save(7a): Clearing table error: ",
                             errorMsg+": "+TableNameStomCont.toStdString());
    }

    // 3. Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableNameStomCont.toStdString();
    cmd += " (MSVPAName,SpeIndex,SpeType,SpeName,Age,Season,MeanGutFull) VALUES ";

    // Read from csv file since it has all the MSVPANames in it
    fin.setFileName(fileNameWithPathStomCont);
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(7b) Opening Input File",
                                 "\n"+fin.errorString()+": " + fileNameWithPathStomCont);
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
        cmd +=  "\"" + qfields[3].trimmed().toStdString() + "\",";
        cmd +=         qfields[4].trimmed().toStdString() + ",";
        cmd +=         qfields[5].trimmed().toStdString() + ",";
        cmd +=         qfields[6].trimmed().toStdString() + "), ";

    } // end for all rows in csv file
    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);

    fin.close();
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("MSVPA Save(7b): Write table error: ", errorMsg);
        return;
    }

    if (saveOK) {
        QMessageBox::information(MSVPA_Tabs, "Save",
                                 tr("\nMSVPA Stomach Contents data saved.\n"));
    }

    MSVPAStomContCSVFile.clear();

    MSVPA_Tab8_NextPB->setEnabled(true);
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab8::callback_MSVPA_Tab8_SavePB Complete");

} // end callback_MSVPA_Tab8_SavePB



void
nmfMSVPATab8::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableMSVPAStomCont);
    fields    = {"MSVPAName","SpeIndex","SpeType","SpeName",
                 "Age","Season","MeanGutFull"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase





void
nmfMSVPATab8::callback_MSVPA_Tab8_LoadPB(bool unused)
{
    int csvFileRow = 0;
    QString line;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(MSVPA_Tabs);
    QStringList NameFilters;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString MSVPAStomContCSVFile;
    QStandardItem *item;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab8::callback_MSVPA_Tab8_LoadPB");

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile(QString::fromStdString(nmfConstantsMSVPA::TableMSVPAStomCont)+".csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load MSVPA Stomach Contents CSV File");
    if (fileDlg.exec()) {

        // Open the file here....
        MSVPAStomContCSVFile = fileDlg.selectedFiles()[0];
        QFile file(MSVPAStomContCSVFile);
        if(! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(MSVPA_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);

        QString csvMSVPAName, csvSpeIndex, csvSpeType, csvSpeName;
        QString csvAge, csvSeason, csvMeanGutFull;
        // Store data to load in next step
        QStringList fields2;
        int m=0;
        QString SpeName = MSVPA_Tab8_SpeciesCMB->currentText().trimmed();
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header
                fields2 = line.split(",");
                csvMSVPAName   = fields2[0].trimmed();
                csvSpeIndex    = fields2[1].trimmed();
                csvSpeType     = fields2[2].trimmed();
                csvSpeName     = fields2[3].trimmed();
                csvAge         = fields2[4].trimmed();
                csvSeason      = fields2[5].trimmed();
                csvMeanGutFull = fields2[6].trimmed();
                if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
                    (csvSpeName == SpeName))
                {
                    item = new QStandardItem(csvMeanGutFull);
                    item->setTextAlignment(Qt::AlignCenter);
                    smodel->setItem(int(m/NSeasons), m % NSeasons, item);
                    ++m;
                }
            }
            ++csvFileRow;
        }
        file.close();

        MSVPA_Tab8_SavePB->setEnabled(true);
        MSVPA_Tab8_NextPB->setEnabled(false);

    } // end if dlg

    MSVPA_Tab8_LoadPB->clearFocus();
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab8::callback_MSVPA_Tab8_LoadPB Complete");

} // end callback_MSVPA_Tab8_LoadPB


void
nmfMSVPATab8::callback_MSVPA_Tab8_ItemChanged(QStandardItem *item)
{

   item->setTextAlignment(Qt::AlignCenter);
   smodel->setItem(item->row(), item->column(), item);

   MarkAsDirty(nmfConstantsMSVPA::TableMSVPAStomCont);
   MSVPA_Tab8_SavePB->setEnabled(true);
   MSVPA_Tab8_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab8_ItemChanged


void
nmfMSVPATab8::MarkAsDirty(std::string tableName)
{
    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfMSVPATab8::MarkAsClean()
{
    emit MarkMSVPAAsClean();
} // end MarkAsClean


bool
nmfMSVPATab8::loadWidgets(nmfDatabase *theDatabasePtr,
                               std::string theMSVPAName)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::string species;
    QStandardItem *item;
    int m;

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab8::loadWidgets");

    databasePtr = theDatabasePtr;
    MSVPAName   = theMSVPAName;

    Nage.clear();
    Species.clear();
    OthPred.clear();
    AllSpecies.clear();
    SpeType.clear();
    SpeIndex.clear();

    // First get some initial data
    fields      = {"NSpe","NSeasons","NOtherPred"};
    queryStr    = "SELECT NSpe,NSeasons,NOtherPred FROM " +
                   nmfConstantsMSVPA::TableMSVPAlist +
                  " WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap     = databasePtr->nmfQueryDatabase(queryStr, fields);
    NSpecies    = std::stoi(dataMap["NSpe"][0]);
    NSeasons    = std::stoi(dataMap["NSeasons"][0]);
    NOtherPred  = std::stoi(dataMap["NOtherPred"][0]);
    NPreds      = NSpecies + NOtherPred;

    smodel = new QStandardItemModel(nmfConstants::MaxNumberAges, nmfConstants::MaxNumberSeasons );

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab8_ItemChanged(QStandardItem *)));

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

    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Type=3";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        species = dataMap["SpeName"][i];
        OthPred.push_back(species);
        AllSpecies.push_back(species);
    }

    for (int i = 0; i < NPreds; ++i) {
        if (i < NSpecies) {
            fields = {"MaxAge","SpeIndex"};
            queryStr = "SELECT MaxAge,SpeIndex FROM " + nmfConstantsMSVPA::TableSpecies +
                       " WHERE Spename = '" + Species[i] + "'";
            SpeType.push_back(0);
            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
            Nage.push_back(std::stoi(dataMap["MaxAge"][0]));
        } else {
            fields = {"NumSizeCats","SpeIndex"};
            queryStr = "SELECT NumSizeCats,SpeIndex FROM " + nmfConstantsMSVPA::TableOtherPredSpecies +
                       " WHERE Spename = '" + OthPred[i-NSpecies] + "'";
            SpeType.push_back(1);
            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
            Nage.push_back(std::stoi(dataMap["NumSizeCats"][0])-1);
        } // end if
        SpeIndex.push_back(std::stoi(dataMap["SpeIndex"][0]));
    } // end for i

    // Set up columns and rows
    smodel->setRowCount(Nage[0]+1);
    smodel->setColumnCount(NSeasons);
    makeHeaders(Nage[0],NSeasons);

    // Update species combobox
    MSVPA_Tab8_SpeciesCMB->blockSignals(true);
    MSVPA_Tab8_SpeciesCMB->clear();
    for (unsigned int i=0;i<AllSpecies.size(); ++i) {
        MSVPA_Tab8_SpeciesCMB->addItem(QString::fromStdString(AllSpecies[i]));
    }
    MSVPA_Tab8_SpeciesCMB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    MSVPA_Tab8_SpeciesCMB->blockSignals(false);

    std::string SpeName = MSVPA_Tab8_SpeciesCMB->currentText().toStdString();

    // Code to load saved data from the database
    fields = {"MeanGutFull"};
    queryStr = "SELECT MeanGutFull FROM " + nmfConstantsMSVPA::TableMSVPAStomCont +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND SpeName = '" + SpeName + "'" +
               " ORDER By Age, Season";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    int NumRecords = dataMap["MeanGutFull"].size();
    m = 0;
    if (NumRecords > 0) {
        for (int i=0; i<NumRecords; ++i) {
            item = new QStandardItem(QString::fromStdString(dataMap["MeanGutFull"][m]));
            item->setTextAlignment(Qt::AlignCenter);
            smodel->setItem(int(i/NSeasons), m % NSeasons, item);
            ++m;
        }
    } else {
        for (int i=0; i<NumRecords; ++i) {
            item = new QStandardItem("");
            item->setTextAlignment(Qt::AlignCenter);
            smodel->setItem(int(i/NSeasons), m % NSeasons, item);
            ++m;
        }
    }


    MSVPA_Tab8_PreyPrefsTV->setModel(smodel);

    MSVPA_Tab8_SavePB->setEnabled(false);
    bool ifDataExist = nmfUtilsQt::allCellsArePopulated(MSVPA_Tabs,
                                                        MSVPA_Tab8_PreyPrefsTV,
                                                        nmfConstants::DontShowError);
    MSVPA_Tab8_NextPB->setEnabled(ifDataExist);
    MarkAsClean();

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab8::loadWidgets Complete");

    return true;

} // end loadWidgets


void
nmfMSVPATab8::makeHeaders(int nRows, int nCols)
{
    QStringList strList;

    // Make row headers
    strList.clear();
    for (int k=0; k<=nRows; ++k) {
        strList << "Age " + QString::number(k);
    }
    smodel->setVerticalHeaderLabels(strList);

    // Make column headers
    strList.clear();
    for (int k=1; k<=nCols; ++k) {
        strList << "Season " + QString::number(k);
    }
    smodel->setHorizontalHeaderLabels(strList);

} // end makeHeaders


void
nmfMSVPATab8::callback_MSVPA_Tab8_SpeciesCMB(int index)
{
    int m;
    int NSeasons;
    int NumRecords;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    QStandardItem *item;
    std::string SpeName = MSVPA_Tab8_SpeciesCMB->currentText().toStdString();

    // Get NSeasons
    fields      = {"NSeasons"};
    queryStr    = "SELECT NSeasons FROM " + nmfConstantsMSVPA::TableMSVPAlist +
                  " WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap     = databasePtr->nmfQueryDatabase(queryStr, fields);
    NSeasons    = std::stoi(dataMap["NSeasons"][0]);

    smodel->clear();

    disconnect(smodel, SIGNAL(itemChanged(QStandardItem *)),
               this,   SLOT(callback_MSVPA_Tab8_ItemChanged(QStandardItem *)));

    makeHeaders(Nage[index],NSeasons);
    smodel->setRowCount(Nage[index]+1);
    smodel->setColumnCount(NSeasons);

    // Load the model which will then load the table
    // Code to load saved data from the database
    fields = {"MeanGutFull"};
    queryStr = "SELECT MeanGutFull FROM " + nmfConstantsMSVPA::TableMSVPAStomCont +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND SpeName = '" + SpeName + "'" +
               " ORDER By Age, Season";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["MeanGutFull"].size();
    m = 0;
    if (NumRecords > 0) {
        for (int i=0; i<NumRecords; ++i) {
            item = new QStandardItem(QString::fromStdString(dataMap["MeanGutFull"][m++]));
            item->setTextAlignment(Qt::AlignCenter);
            smodel->setItem(int(i/NSeasons), i % NSeasons, item);
        }
    } else {
        for (int i=0; i<NumRecords; ++i) {
            item = new QStandardItem("");
            item->setTextAlignment(Qt::AlignCenter);
            smodel->setItem(int(i/NSeasons), i % NSeasons, item);
        }
    }


    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab8_ItemChanged(QStandardItem *)));

} // end callback_MSVPA_Tab8_SpeciesCMB




void
nmfMSVPATab8::callback_MSVPA_Tab8_PrevPB(bool unused)
{
    int prevPage = MSVPA_Tabs->currentIndex()-1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(prevPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(prevPage);
    emit UpdateNavigator("MSVPA",prevPage);
}

void
nmfMSVPATab8::callback_MSVPA_Tab8_NextPB(bool unused)
{
    int nextPage;

    QCheckBox *MSVPA_Tab2_PredatorGrowthCB = MSVPA_Tabs->findChild<QCheckBox *>("MSVPA_Tab2_PredatorGrowthCB");

    if (MSVPA_Tab2_PredatorGrowthCB->isChecked()) {
        nextPage = MSVPA_Tabs->currentIndex()+1;
    } else {
        nextPage = MSVPA_Tabs->currentIndex()+3; // skip 2 tabs
    }
    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(nextPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(nextPage);
    emit UpdateNavigator("MSVPA",nextPage);
}

