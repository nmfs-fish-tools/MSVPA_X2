
#include "nmfUtils.h"
#include "nmfUtilsQt.h"
#include "nmfConstants.h"

#include "nmfMSVPATab01.h"


nmfMSVPATab1::nmfMSVPATab1(QTabWidget *tabs,
                           nmfLogger  *theLogger,
                           std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab1::nmfMSVPATab1");

    MSVPA_Tabs = tabs;
    MSVPAspeciesCSVFile.clear();
    ProjectDir = theProjectDir;
    Enabled = false;

    // Load ui as a widget from disk
    QFile file(":/forms/MSVPA/MSVPA_Tab01.ui");
    file.open(QFile::ReadOnly);
    MSVPA_Tab1_Widget = loader.load(&file,MSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    MSVPA_Tabs->addTab(MSVPA_Tab1_Widget, tr("1. Assign Species"));

    MSVPA_Tab1_SelectSpeciesLW = MSVPA_Tabs->findChild<QListWidget *>("MSVPA_Tab1_SelectSpeciesLW");
    MSVPA_Tab1_SelectBMPredLW  = MSVPA_Tabs->findChild<QListWidget *>("MSVPA_Tab1_SelectBMPredLW");
    MSVPA_Tab1_FullSpeciesLW   = MSVPA_Tabs->findChild<QListWidget *>("MSVPA_Tab1_FullSpeciesLW");
    MSVPA_Tab1_PreySpeciesLW   = MSVPA_Tabs->findChild<QListWidget *>("MSVPA_Tab1_PreySpeciesLW");
    MSVPA_Tab1_BMPredLW        = MSVPA_Tabs->findChild<QListWidget *>("MSVPA_Tab1_BMPredLW");

    // Setup connections
    MSVPA_Tab1_SavePB          = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab1_SavePB");
    MSVPA_Tab1_LoadPB          = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab1_LoadPB");
    MSVPA_Tab1_NextPB          = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab1_NextPB");
    QPushButton *MSVPA_Tab1_ResetPB          = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab1_ResetPB");
    QPushButton *MSVPA_Tab1_SelectFullPB     = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab1_SelectFullPB");
    QPushButton *MSVPA_Tab1_DeselectFullPB   = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab1_DeselectFullPB");
    QPushButton *MSVPA_Tab1_SelectPreyPB     = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab1_SelectPreyPB");
    QPushButton *MSVPA_Tab1_DeselectPreyPB   = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab1_DeselectPreyPB");
    QPushButton *MSVPA_Tab1_SelectBMPredPB   = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab1_SelectBMPredPB");
    QPushButton *MSVPA_Tab1_DeselectBMPredPB = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab1_DeselectBMPredPB");

    MSVPA_Tab1_NextPB->setText("--\u25B7");

    connect(MSVPA_Tab1_SavePB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab1_SavePB(bool)));
    connect(MSVPA_Tab1_LoadPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab1_LoadPB(bool)));
    connect(MSVPA_Tab1_NextPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab1_NextPagePB(bool)));
    connect(MSVPA_Tab1_ResetPB, SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab1_ResetPB(bool)));

    connect(MSVPA_Tab1_SelectFullPB, SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab1_SelectFullPB(bool)));
    connect(MSVPA_Tab1_DeselectFullPB, SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab1_DeselectFullPB(bool)));

    connect(MSVPA_Tab1_SelectPreyPB, SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab1_SelectPreyPB(bool)));
    connect(MSVPA_Tab1_DeselectPreyPB, SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab1_DeselectPreyPB(bool)));

    connect(MSVPA_Tab1_SelectBMPredPB, SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab1_SelectBMPredPB(bool)));
    connect(MSVPA_Tab1_DeselectBMPredPB, SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab1_DeselectBMPredPB(bool)));

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab1::nmfMSVPATab1 Complete");

}


nmfMSVPATab1::~nmfMSVPATab1()
{

}


void
nmfMSVPATab1::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir



QList<QString>
nmfMSVPATab1::loadList(QListWidget *listWidget,
                            nmfDatabase* databasePtr,
                            std::string MSVPAName,
                            int Type)
{
    int numLoaded = 0;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    QList<QString> loadedSpecies;
    QString Species;

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab1::loadList");

    listWidget->clear();
    fields   = {"SpeName"};
    if (Type == -1)
        queryStr = "SELECT SpeName from MSVPAspecies WHERE MSVPAName='" + MSVPAName + "'";
    else
        queryStr = "SELECT SpeName from MSVPAspecies WHERE MSVPAName='" + MSVPAName +
                   "' and Type=" + std::to_string(Type);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        Species = QString::fromStdString(dataMap["SpeName"][i]);
        listWidget->addItem(Species);
        loadedSpecies.append(Species);
        ++numLoaded;
    }

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab1::loadList Complete");

    //return numLoaded;
    return loadedSpecies;
}

void
nmfMSVPATab1::clearWidgets()
{
    MSVPA_Tab1_SelectSpeciesLW->clear();
    MSVPA_Tab1_SelectBMPredLW->clear();
    MSVPA_Tab1_FullSpeciesLW->clear();
    MSVPA_Tab1_PreySpeciesLW->clear();
    MSVPA_Tab1_BMPredLW->clear();
}

void
nmfMSVPATab1::MarkAsDirty(std::string tableName)
{

    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfMSVPATab1::MarkAsClean()
{

    emit MarkMSVPAAsClean();

} // end MarkAsClean




bool
nmfMSVPATab1::loadWidgets(nmfDatabase *theDatabasePtr,
                               std::string theMSVPAName)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    QString SpeName;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab1::loadWidgets");

    emit UpdateNavigator("MSVPA",0);

    databasePtr = theDatabasePtr;
    MSVPAName   = theMSVPAName;

    clearWidgets();

    QList<QString> FullSpeciesList = loadList(MSVPA_Tab1_FullSpeciesLW, databasePtr, MSVPAName, 0);
    QList<QString> PreySpeciesList = loadList(MSVPA_Tab1_PreySpeciesLW, databasePtr, MSVPAName, 1);
    QList<QString> BiomassPredList = loadList(MSVPA_Tab1_BMPredLW,      databasePtr, MSVPAName, 3);

    // Then check and load any other species into the tables on the left
    fields   = {"SpeIndex","SpeName"};
    queryStr = "SELECT SpeIndex,SpeName FROM Species" ;
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
        SpeName  = QString::fromStdString(dataMap["SpeName"][i]);
        if (! FullSpeciesList.contains(SpeName) && (! PreySpeciesList.contains(SpeName))) {
            MSVPA_Tab1_SelectSpeciesLW->addItem(QString::fromStdString(dataMap["SpeName"][i]));
            Enabled = true;
        }
    }

    queryStr = "SELECT SpeIndex,SpeName FROM OtherPredSpecies" ;
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
        SpeName  = QString::fromStdString(dataMap["SpeName"][i]);
        if (! BiomassPredList.contains(SpeName)) {
            MSVPA_Tab1_SelectBMPredLW->addItem(QString::fromStdString(dataMap["SpeName"][i]));
            Enabled = true;
        }
    }

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab1::loadWidgets Complete");
    MSVPA_Tabs->setTabEnabled(0,true);

    MarkAsClean();

    return true;

} // end loadWidgets


void
nmfMSVPATab1::callback_MSVPA_Tab1_LoadPB(bool unused)
{
    int csvType;
    int csvFileRow = 0;
    QString line;
    QModelIndex index;
    QStringList fields;
    QString Species;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(MSVPA_Tabs);
    QStringList NameFilters;
    QString csvMSVPAName, csvSpeciesName;
    std::vector<std::string> fields2;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab1::callback_MSVPA_Tab1_LoadPB");

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile("MSVPAspecies.csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load MSVPA Species CSV File");
    if (fileDlg.exec()) {

        // Open the file here....
        MSVPAspeciesCSVFile = fileDlg.selectedFiles()[0];
        QFile file(MSVPAspeciesCSVFile);
        if(! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(MSVPA_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);

        clearWidgets();
        QList<QString> SpeciesAssigned;

        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header
                fields = line.split(",");
                csvMSVPAName   = fields[0].trimmed();
                csvSpeciesName = fields[1].trimmed();
                csvType        = fields[3].toInt();
                if (csvMSVPAName == QString::fromStdString(MSVPAName)) {
                    if (csvType == nmfConstantsMSVPA::MSVPAFullSpecies) {
                        MSVPA_Tab1_FullSpeciesLW->addItem(csvSpeciesName);
                        SpeciesAssigned.append(csvSpeciesName);
                    } else if (csvType == nmfConstantsMSVPA::MSVPAPreyOnlySpecies) {
                        MSVPA_Tab1_PreySpeciesLW->addItem(csvSpeciesName);
                        SpeciesAssigned.append(csvSpeciesName);
                    } else if (csvType == nmfConstantsMSVPA::MSVPABiomassPredator) {
                        MSVPA_Tab1_BMPredLW->addItem(csvSpeciesName);
                        SpeciesAssigned.append(csvSpeciesName);
                    }
                }
            }
            ++csvFileRow;
        }
        file.close();

        // Now load any unassigned species into the Select list widgets on left
        fields2   = {"SpeIndex","SpeName"};
        queryStr = "SELECT SpeIndex,SpeName FROM Species" ;
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields2);
        for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
            Species  = QString::fromStdString(dataMap["SpeName"][i]);
            if (! SpeciesAssigned.contains(Species)) {
                MSVPA_Tab1_SelectSpeciesLW->addItem(QString::fromStdString(dataMap["SpeName"][i]));
            }
        }
        queryStr = "SELECT SpeIndex,SpeName FROM OtherPredSpecies" ;
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields2);
        for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
            Species  = QString::fromStdString(dataMap["SpeName"][i]);
            if (! SpeciesAssigned.contains(Species)) {
                MSVPA_Tab1_SelectBMPredLW->addItem(QString::fromStdString(dataMap["SpeName"][i]));
            }
        }

        MSVPA_Tab1_SavePB->setEnabled(true);
        MSVPA_Tab1_NextPB->setEnabled(false);
    }
    MSVPA_Tab1_LoadPB->clearFocus();
    MSVPA_Tab1_SavePB->setEnabled(true);

    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab1::callback_MSVPA_Tab1_LoadPB Complete");

} // end callback_MSVPA_Tab1_LoadPB


int
nmfMSVPATab1::getMSVPASpeciesType(QString csvSpeciesName)
{
    // MSVPA_Tab1_FullSpeciesLW => type of 0
    // MSVPA_Tab1_PreySpeciesLW => type of 1
    // MSVPA_Tab1_BMPredLW      => type of 3
    int retv = -1;

    if (MSVPA_Tab1_FullSpeciesLW->findItems(csvSpeciesName,Qt::MatchExactly).count() > 0)
        retv = 0;
    else if (MSVPA_Tab1_PreySpeciesLW->findItems(csvSpeciesName,Qt::MatchExactly).count() > 0)
        retv = 1;
    else if (MSVPA_Tab1_BMPredLW->findItems(csvSpeciesName,Qt::MatchExactly).count() > 0)
        retv = 3;

    return retv;

} // end getMSVPASpeciesType


void
nmfMSVPATab1::callback_MSVPA_Tab1_SavePB(bool unused)
{
    bool saveOK = true;
    bool FirstLine;
    QString fileName;
    QString filePath;
    QString fileNameWithPath;
    QString tmpFileNameWithPath;
    QString qcmd;
    QString line;
    QStringList qfields;
    std::string cmd;
    std::string errorMsg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString TableName = "MSVPAspecies";

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab1::callback_MSVPA_Tab1_SavePB");

//    if (! nmfQtUtils::allCellsArePopulated(SSVPA_Tabs,SSVPASizeAtAgeTV))
//        return;

    // Build a Species and OtherPredSpecies name and index map, you'll need these.
    std::map<std::string,int> SpeciesIndexMap;
    fields   = {"SpeIndex", "SpeName"};
    for (std::string SpeciesTable : {"Species", "OtherPredSpecies"}) {
        queryStr = "SELECT SpeIndex,SpeName FROM " + SpeciesTable;
        dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
        for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
            SpeciesIndexMap[dataMap["SpeName"][i]] = std::stoi(dataMap["SpeIndex"][i]);
        }
    }
    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (MSVPAspeciesCSVFile.isEmpty()) {
        MSVPAspeciesCSVFile = TableName + ".csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPath    = QDir(filePath).filePath(MSVPAspeciesCSVFile);
        tmpFileNameWithPath = QDir(filePath).filePath("."+MSVPAspeciesCSVFile);
    } else {
        QFileInfo finfo(MSVPAspeciesCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPath    = MSVPAspeciesCSVFile;
        tmpFileNameWithPath = QDir(filePath).filePath("."+fileName);
    }
    MSVPAspeciesCSVFile.clear();

    // Read entire file and copy all lines that don't have the current Species into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.

    QFile fin(fileNameWithPath);
    QFile fout(tmpFileNameWithPath);

    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs, "Error Opening Input File", fin.errorString());
        return;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(MSVPA_Tabs, "Error Opening Output File", fout.errorString());
        return;
    }
    QTextStream inStream(&fin);
    QTextStream outStream(&fout);

    QString Species;
    QString csvMSVPAName,csvSpeciesName;
    QString tableLine;
    bool ignoreMSVPAName = false;
    int numRows = MSVPA_Tab1_FullSpeciesLW->count() +
                  MSVPA_Tab1_PreySpeciesLW->count() +
                  MSVPA_Tab1_BMPredLW->count();

    std::vector<QListWidget *> ListWidgets ={MSVPA_Tab1_FullSpeciesLW,
                                             MSVPA_Tab1_PreySpeciesLW,
                                             MSVPA_Tab1_BMPredLW};

    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvMSVPAName   = qfields[0].trimmed();
        if (csvMSVPAName == QString::fromStdString(MSVPAName) && (! csvMSVPAName.isEmpty())) { // dump qtableview data into output file
            // Skip over all other MSVPAName rows, since going through this once copies the qtableview data
            if (! ignoreMSVPAName) {
                for (QListWidget *listWidget : ListWidgets)
                {
                    numRows = listWidget->count();
                    for (int i=0; i<numRows; ++i) {
                        Species = listWidget->item(i)->text();
                        tableLine = QString::fromStdString(MSVPAName) + ", " +
                                Species + ", " +
                                QString::number(SpeciesIndexMap[Species.toStdString()]) + ", " +
                                QString::number(getMSVPASpeciesType(Species)) +
                                ", , , , , ";
                        outStream << tableLine << "\n";
                    }
                }
                ignoreMSVPAName = true;
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }
    }
    if (!ignoreMSVPAName) { // means never found the MSVPAName
        for (QListWidget *listWidget : ListWidgets)
        {
            numRows = listWidget->count();
            for (int i=0; i<numRows; ++i) {
                Species = listWidget->item(i)->text();
                tableLine = QString::fromStdString(MSVPAName) + ", " +
                        Species + ", " +
                        QString::number(SpeciesIndexMap[Species.toStdString()]) + ", " +
                        QString::number(getMSVPASpeciesType(Species)) +
                        ", , , , , ";
                outStream << tableLine << "\n";
            }
        }
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    int retv = nmfUtilsQt::rename(tmpFileNameWithPath, fileNameWithPath);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPath.toStdString() << " to " <<
                     fileNameWithPath.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return;
    }

    //
    // Save to mysql table
    //
    // 1. Check that the table exists, if not, create it.
    databasePtr->checkForTableAndCreate(TableName);

    // 2. Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableName;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
        saveOK = false;
        nmfUtils::printError("MSVPA Save(1): Clearing table error: ", errorMsg);
    }

    // 3. Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableName.toStdString();
    cmd += "(MSVPAName,SpeName,SpeIndex,Type) VALUES ";

    // Read from csv file since it has all the Species in it
    QFile fin2(fileNameWithPath);
    if(! fin2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs, "Error(1) Opening Input File", fin.errorString());
        return;
    }
    inStream.setDevice(&fin2);

    FirstLine = true;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields = line.split(",");
        cmd += "(\"" + qfields[0].trimmed().toStdString() + "\",";
        cmd += "\""  + qfields[1].trimmed().toStdString() + "\",";
        cmd +=         qfields[2].trimmed().toStdString() + ",";
        cmd +=         qfields[3].trimmed().toStdString() + "), ";

    } // end for all rows in csv file

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    fin2.close();
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("MSVPA Save(3): Write table error: "+TableName.toStdString(), errorMsg);
        return;
    }

    MSVPA_Tab1_SavePB->setEnabled(false);
    MSVPA_Tab1_NextPB->setEnabled(true);
    if (saveOK) {
        QMessageBox::information(MSVPA_Tabs, "Save",
                                 tr("\nMSVPA Species types saved.\n"));
    }

    //MSVPAspeciesCSVFile.clear();  // RSK add this to every tab's Save function

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab1::callback_MSVPA_Tab1_SavePB Complete");

} // end callback_MSVPA_Tab1_SavePB


void
nmfMSVPATab1::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    TableName = "MSVPAspecies";
    fields    = {"MSVPAName","SpeName","SpeIndex","Type","TimeRec",
                 "LenRec","WtRec","SSVPAname","SSVPAindex"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);
}


void
nmfMSVPATab1::callback_MSVPA_Tab1_NextPagePB(bool unused)
{
    int nextPage = MSVPA_Tabs->currentIndex()+1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(nextPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(nextPage);
    emit UpdateNavigator("MSVPA",nextPage);
}

void
nmfMSVPATab1::moveItemsFromTo(std::vector<QListWidget *> &listWidgets,
                              QListWidget *DestListWidget)
{
    int numItems;
    QListWidget *listWidget;
    QListWidgetItem *item;

    for (unsigned int i=0; i<listWidgets.size(); ++i) {
        listWidget = listWidgets[i];
        if (listWidget->selectedItems().size() == 0) {
            numItems = listWidget->count();
            for (int j=0; j<numItems; ++j) {
                DestListWidget->addItem(listWidget->takeItem(0));
            }
        } else {
            QModelIndexList mlist = listWidget->selectionModel()->selectedIndexes();
            for (auto it=mlist.begin(); it != mlist.end(); ++it) {
                item = listWidget->takeItem(it->row());
                DestListWidget->addItem(item);
            }
        }
    }

    MarkAsDirty("MSVPAspecies");

} // end moveItemsFromTo


/*
 * Move all species from right lists into left lists.
 */
void
nmfMSVPATab1::callback_MSVPA_Tab1_ResetPB(bool unused)
{
    // Reset Species to include in the MSVPA
    std::vector<QListWidget *> listWidgets = {MSVPA_Tab1_FullSpeciesLW,
                                              MSVPA_Tab1_PreySpeciesLW};
    moveItemsFromTo(listWidgets,MSVPA_Tab1_SelectSpeciesLW);
    MSVPA_Tab1_SavePB->setEnabled(true);
    MSVPA_Tab1_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab1_ResetPB


void
nmfMSVPATab1::callback_MSVPA_Tab1_SelectFullPB(bool unused)
{
    std::vector<QListWidget *> listWidgets = {MSVPA_Tab1_SelectSpeciesLW};

    moveItemsFromTo(listWidgets,MSVPA_Tab1_FullSpeciesLW);
    MSVPA_Tab1_SavePB->setEnabled(true);
    MSVPA_Tab1_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab1_SelectFullPB


void
nmfMSVPATab1::callback_MSVPA_Tab1_DeselectFullPB(bool unused)
{
    std::vector<QListWidget *> listWidgets = {MSVPA_Tab1_FullSpeciesLW};

    moveItemsFromTo(listWidgets,MSVPA_Tab1_SelectSpeciesLW);
    MSVPA_Tab1_SavePB->setEnabled(true);
    MSVPA_Tab1_NextPB->setEnabled(false);

}  // end callback_MSVPA_Tab1_DeselectFullPB


void
nmfMSVPATab1::callback_MSVPA_Tab1_SelectPreyPB(bool unused)
{
    std::vector<QListWidget *> listWidgets = {MSVPA_Tab1_SelectSpeciesLW};

    moveItemsFromTo(listWidgets,MSVPA_Tab1_PreySpeciesLW);
    MSVPA_Tab1_SavePB->setEnabled(true);
    MSVPA_Tab1_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab1_SelectPreyPB


void
nmfMSVPATab1::callback_MSVPA_Tab1_DeselectPreyPB(bool unused)
{
    std::vector<QListWidget *> listWidgets = {MSVPA_Tab1_PreySpeciesLW};

    moveItemsFromTo(listWidgets,MSVPA_Tab1_SelectSpeciesLW);
    MSVPA_Tab1_SavePB->setEnabled(true);
    MSVPA_Tab1_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab1_DeselectPreyPB


void
nmfMSVPATab1::callback_MSVPA_Tab1_SelectBMPredPB(bool unused)
{
    std::vector<QListWidget *> listWidgets = {MSVPA_Tab1_SelectBMPredLW};

    moveItemsFromTo(listWidgets,MSVPA_Tab1_BMPredLW);
    MSVPA_Tab1_SavePB->setEnabled(true);
    MSVPA_Tab1_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab1_SelectBMPredPB


void
nmfMSVPATab1::callback_MSVPA_Tab1_DeselectBMPredPB(bool unused)
{
    std::vector<QListWidget *> listWidgets = {MSVPA_Tab1_BMPredLW};

    moveItemsFromTo(listWidgets,MSVPA_Tab1_SelectBMPredLW);
    MSVPA_Tab1_SavePB->setEnabled(true);
    MSVPA_Tab1_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab1_DeselectBMPredPB
