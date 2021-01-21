
#include "nmfConstants.h"
#include "nmfUtils.h"
#include "nmfUtilsQt.h"

#include "nmfMSVPATab07.h"



nmfMSVPATab7::nmfMSVPATab7(QTabWidget *tabs,
                           nmfLogger  *theLogger,
                           std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab7::nmfMSVPATab7");

    MSVPA_Tabs = tabs;
    ProjectDir = theProjectDir;
    Enabled = false;
    smodel = NULL;
    databasePtr = NULL;
    MSVPASizePrefCSVFile.clear();

    // Load ui as a widget from disk
    QFile file(":/forms/MSVPA/MSVPA_Tab07.ui");
    file.open(QFile::ReadOnly);
    MSVPA_Tab7_Widget = loader.load(&file,MSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    MSVPA_Tabs->addTab(MSVPA_Tab7_Widget, tr("7. Size/Consumption"));

    // Setup connections
    MSVPA_Tab7_LoadPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab7_LoadPB");
    MSVPA_Tab7_SavePB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab7_SavePB");
    MSVPA_Tab7_PrevPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab7_PrevPB");
    MSVPA_Tab7_NextPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab7_NextPB");
    MSVPA_Tab7_SpeciesCMB  = MSVPA_Tabs->findChild<QComboBox *>("MSVPA_Tab7_SpeciesCMB");
    MSVPA_Tab7_SizePrefsTV = MSVPA_Tabs->findChild<QTableView *>("MSVPA_Tab7_SizePrefsTV");

    MSVPA_Tab7_PrevPB->setText("\u25C1--");
    MSVPA_Tab7_NextPB->setText("--\u25B7");

    connect(MSVPA_Tab7_PrevPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab7_PrevPB(bool)));
    connect(MSVPA_Tab7_NextPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab7_NextPB(bool)));
    connect(MSVPA_Tab7_LoadPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab7_LoadPB(bool)));
    connect(MSVPA_Tab7_SavePB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab7_SavePB(bool)));
    connect(MSVPA_Tab7_SpeciesCMB, SIGNAL(currentIndexChanged(int)),
            this,                  SLOT(callback_MSVPA_Tab7_SpeciesCMB(int)));

    MSVPA_Tab7_SavePB->setEnabled(false);

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab7::nmfMSVPATab7 Complete");

}


nmfMSVPATab7::~nmfMSVPATab7()
{
    std::cout << "Freeing Tab6 memory" << std::endl;
    clearWidgets();
}


void
nmfMSVPATab7::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir


void
nmfMSVPATab7::clearWidgets()
{
    if (smodel)
        smodel->clear();
}


void
nmfMSVPATab7::callback_MSVPA_Tab7_SavePB(bool unused)
{
    bool saveOK = true;
    bool FirstLine;
    QString fileName;
    QString filePath;
    QString fileNameWithPathSizePref;
    QString tmpFileNameWithPathSizePref;
    QString qcmd;
    QString line;
    QString TableNameSizePref;
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

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab7::callback_MSVPA_Tab7_SavePB");

    //
    // Save info to MSVPAprefs
    //
    if (! nmfUtilsQt::allCellsArePopulated(MSVPA_Tabs,MSVPA_Tab7_SizePrefsTV,
                                           nmfConstants::ShowError))
        return;

    // Build a Species index map
    std::map<std::string,int> SpeciesIndexMap;
    fields   = {"MSVPAName", "SpeName", "SpeIndex", "Type"};
    queryStr = "SELECT MSVPAName,SpeName,SpeIndex,Type FROM MSVPAspecies WHERE MSVPAName = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
        SpeciesIndexMap[dataMap["SpeName"][i]] = std::stoi(dataMap["SpeIndex"][i]);
    }

    TableNameSizePref = "MSVPASizePref";

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (MSVPASizePrefCSVFile.isEmpty()) {
        MSVPASizePrefCSVFile = TableNameSizePref + ".csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPathSizePref    = QDir(filePath).filePath(MSVPASizePrefCSVFile);
        tmpFileNameWithPathSizePref = QDir(filePath).filePath("."+MSVPASizePrefCSVFile);
    } else {
        QFileInfo finfo(MSVPASizePrefCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPathSizePref    = MSVPASizePrefCSVFile;
        tmpFileNameWithPathSizePref = QDir(filePath).filePath("."+fileName);
    }


    // Read entire file and copy all lines that don't have the current MSVPAName into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    QFile fin(fileNameWithPathSizePref);
    QFile fout(tmpFileNameWithPathSizePref);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(6a) Opening Input File",
                                 "\n"+fin.errorString()+": "+fileNameWithPathSizePref);
        return;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(6a) Opening Output File",
                                 "\n"+fout.errorString()+": "+tmpFileNameWithPathSizePref);
        return;
    }

    QTextStream inStream(&fin);
    QTextStream outStream(&fout);
    QModelIndex index0,index1,index2,index3;
    ignoreMSVPAName = false;
    int numRows = smodel->rowCount();
    //int numCols = smodel->columnCount();
    QString SpeName = MSVPA_Tab7_SpeciesCMB->currentText();
    QString Age;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvMSVPAName = qfields[0].trimmed();
        csvSpeName   = qfields[2].trimmed();
        if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
            (csvSpeName == SpeName) &&
            (! csvMSVPAName.isEmpty())) { // dump qtableview data into output file
            // Skip over all other MSVPAName rows, since going through this once copies the qtableview data
            if (! ignoreMSVPAName) {
                for (int i=0; i<numRows; ++i) {
                    index0 = MSVPA_Tab7_SizePrefsTV->model()->index(i,0);
                    index1 = MSVPA_Tab7_SizePrefsTV->model()->index(i,1);
                    index2 = MSVPA_Tab7_SizePrefsTV->model()->index(i,2);
                    index3 = MSVPA_Tab7_SizePrefsTV->model()->index(i,3);
                    tableLine = QString::fromStdString(MSVPAName) + ", " +
                                QString::number(SpeciesIndexMap[SpeName.toStdString()]) + ", " +
                                SpeName                  + ", " +
                                QString::number(i)       + ", " +
                                index0.data().toString() + ", " +
                                index1.data().toString() + ", " +
                                index2.data().toString() + ", " +
                                index3.data().toString();
                    outStream << tableLine << "\n";
                } // end for i
                ignoreMSVPAName = true;
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }
    }
    if (! ignoreMSVPAName) { // means never found the MSVPAName
        for (int i=0; i<numRows; ++i) {
            index0 = MSVPA_Tab7_SizePrefsTV->model()->index(i,0);
            index1 = MSVPA_Tab7_SizePrefsTV->model()->index(i,1);
            index2 = MSVPA_Tab7_SizePrefsTV->model()->index(i,2);
            index3 = MSVPA_Tab7_SizePrefsTV->model()->index(i,3);
            tableLine = QString::fromStdString(MSVPAName) + ", " +
                        QString::number(SpeciesIndexMap[SpeName.toStdString()]) + ", " +
                        SpeName                  + ", " +
                        QString::number(i)       + ", " +
                        index0.data().toString() + ", " +
                        index1.data().toString() + ", " +
                        index2.data().toString() + ", " +
                        index3.data().toString();
            outStream << tableLine << "\n";
        } // end for i
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    retv = nmfUtilsQt::rename(tmpFileNameWithPathSizePref,
                                  fileNameWithPathSizePref);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPathSizePref.toStdString() << " to " <<
                     fileNameWithPathSizePref.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return;
    }

    // --------------------------------------------

    //
    // Save MSVPASizePref mysql table
    //
    // 1. Check that the table exists, if not, create it.
    //databasePtr->checkForTableAndCreate(TableNameSizePref);

    // 2. Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableNameSizePref;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
        saveOK = false;
        nmfUtils::printError("MSVPA Save(6a): Clearing table error: ",
                             errorMsg+": "+TableNameSizePref.toStdString());
    }

    // 3. Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableNameSizePref.toStdString();
    cmd += " (MSVPAName,SpeIndex,SpeName,Age,EvacAlpha,EvacBeta,SizeAlpha,SizeBeta) VALUES ";

    // Read from csv file since it has all the MSVPANames in it
    fin.setFileName(fileNameWithPathSizePref);
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(6b) Opening Input File",
                                 "\n"+fin.errorString()+": " + fileNameWithPathSizePref);
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
        cmd +=  "\"" + qfields[2].trimmed().toStdString() + "\",";
        cmd +=         qfields[3].trimmed().toStdString() + ",";
        cmd +=         qfields[4].trimmed().toStdString() + ",";
        cmd +=         qfields[5].trimmed().toStdString() + ",";
        cmd +=         qfields[6].trimmed().toStdString() + ",";
        cmd +=         qfields[7].trimmed().toStdString() + "), ";

    } // end for all rows in csv file
    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);

    fin.close();
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("MSVPA Save(6b): Write table error: ", errorMsg);
        return;
    }

    if (saveOK) {
        QMessageBox::information(MSVPA_Tabs, "Save",
                                 tr("\nMSVPA Size Pref data saved.\n"));
    }

    MSVPASizePrefCSVFile.clear();

    MSVPA_Tab7_NextPB->setEnabled(true);
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab7::callback_MSVPA_Tab7_SavePB Complete");

} // end callback_MSVPA_Tab7_SavePB


void
nmfMSVPATab7::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    TableName = "MSVPASizePref";
    fields    = {"MSVPAName","SpeIndex","SpeName","Age",
                 "EvacAlpha","EvacBeta","SizeAlpha","SizeBeta"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase



void
nmfMSVPATab7::callback_MSVPA_Tab7_LoadPB(bool unused)
{
    int csvFileRow = 0;
    QString line;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(MSVPA_Tabs);
    QStringList NameFilters;
    QString MSVPASizePrefCSVFile;
    QStandardItem *item;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab7::callback_MSVPA_Tab7_LoadPB");

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile("MSVPASizePref.csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load MSVPA Size Pref CSV File");
    if (fileDlg.exec()) {

        // Open the file here....
        MSVPASizePrefCSVFile = fileDlg.selectedFiles()[0];
        QFile file(MSVPASizePrefCSVFile);
        if (! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(MSVPA_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);

        QString csvMSVPAName, csvSpeIndex, csvSpeName, csvAge;
        QString csvEvacAlpha, csvEvacBeta, csvSizeAlpha, csvSizeBeta;
        // Store data to load in next step
        QStringList fields2;
        int row=0;
        QString SpeName = MSVPA_Tab7_SpeciesCMB->currentText().trimmed();
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header
                fields2 = line.split(",");
                csvMSVPAName = fields2[0].trimmed();
                csvSpeIndex  = fields2[1].trimmed();
                csvSpeName   = fields2[2].trimmed();
                csvAge       = fields2[3].trimmed();
                csvEvacAlpha = fields2[4].trimmed();
                csvEvacBeta  = fields2[5].trimmed();
                csvSizeAlpha = fields2[6].trimmed();
                csvSizeBeta  = fields2[7].trimmed();
                if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
                    (csvSpeName == SpeName))
                {
                    item = new QStandardItem(csvEvacAlpha);
                    item->setTextAlignment(Qt::AlignCenter);
                    smodel->setItem(row, 0, item);
                    item = new QStandardItem(csvEvacBeta);
                    item->setTextAlignment(Qt::AlignCenter);
                    smodel->setItem(row, 1, item);
                    item = new QStandardItem(csvSizeAlpha);
                    item->setTextAlignment(Qt::AlignCenter);
                    smodel->setItem(row, 2, item);
                    item = new QStandardItem(csvSizeBeta);
                    item->setTextAlignment(Qt::AlignCenter);
                    smodel->setItem(row, 3, item);
                    ++row;
                }
            }
            ++csvFileRow;
        }
        file.close();

        MSVPA_Tab7_SavePB->setEnabled(true);
        MSVPA_Tab7_NextPB->setEnabled(false);

    } // end if dlg

    MSVPA_Tab7_LoadPB->clearFocus();
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab7::callback_MSVPA_Tab7_LoadPB Complete");

} // end callback_MSVPA_Tab7_LoadPB



void
nmfMSVPATab7::callback_MSVPA_Tab7_ItemChanged(QStandardItem *item)
{
    item->setTextAlignment(Qt::AlignCenter);
    smodel->setItem(item->row(), item->column(), item);

    MarkAsDirty("MSVPASizePref");
    MSVPA_Tab7_SavePB->setEnabled(true);
    MSVPA_Tab7_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab7_ItemChanged

void
nmfMSVPATab7::MarkAsDirty(std::string tableName)
{
    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfMSVPATab7::MarkAsClean()
{
    emit MarkMSVPAAsClean();
} // end MarkAsClean


bool
nmfMSVPATab7::loadWidgets(nmfDatabase *theDatabasePtr,
                               std::string theMSVPAName)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::string species;
    QStandardItem *item;
    //std::vector<int> SpeIndex;
    int m=0;

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab7::loadWidgets");

    databasePtr = theDatabasePtr;
    MSVPAName   = theMSVPAName;

    Species.clear();
    Nage.clear();

    // First get some initial data
    fields       = {"NPreyOnly","NSpe","NOther","NOtherPred","NSeasons","SeasSpaceO"};
    queryStr     = "SELECT NPreyOnly,NSpe,NOther,NOtherPred,NSeasons,SeasSpaceO FROM MSVPAlist WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap      = databasePtr->nmfQueryDatabase(queryStr, fields);
    NPrey        = std::stoi(dataMap["NPreyOnly"][0]);
    NSpecies     = std::stoi(dataMap["NSpe"][0]);
    NOtherPrey   = std::stoi(dataMap["NOther"][0]);
    //NOtherPred   = std::stoi(dataMap["NOtherPred"][0]);
    NPreds       = NSpecies + NOtherPred;
    TotalNSpe    = NSpecies + NPrey + NOtherPrey;

    smodel = new QStandardItemModel(TotalNSpe, nmfConstants::MaxNumberAges );

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab7_ItemChanged(QStandardItem *)));

    // Load the species combo box in Tab6
    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Type=0";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        species = dataMap["SpeName"][i];
        Species.push_back(species);
    }

    // Get more data
    fields   = {"MaxAge","SpeIndex"};
    for (int i = 0; i < NSpecies; ++i) {
     queryStr = "SELECT MaxAge,SpeIndex FROM Species WHERE Spename = '" + Species[i] + "'";
     dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
     Nage.push_back(std::stoi(dataMap["MaxAge"][0]));
     //SpeIndex.push_back(std::stoi(dataMap["SpeIndex"][0]));
    } // end for i

    // Set up columns and rows
    smodel->setRowCount(Nage[0]+1);
    smodel->setColumnCount(NUM_COLUMNS);
    makeHeaders(Nage[0]);

    // Update species combobox
    MSVPA_Tab7_SpeciesCMB->blockSignals(true);
    MSVPA_Tab7_SpeciesCMB->clear();
    for (unsigned int i=0;i<Species.size(); ++i) {
        MSVPA_Tab7_SpeciesCMB->addItem(QString::fromStdString(Species[i]));
    }
    MSVPA_Tab7_SpeciesCMB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    MSVPA_Tab7_SpeciesCMB->blockSignals(false);

    std::string SpeName = MSVPA_Tab7_SpeciesCMB->currentText().toStdString();

    // Load the model which will then load the table
    fields   = {"SpeName","EvacAlpha","EvacBeta","SizeAlpha","SizeBeta","SpeIndex","Age"};
    queryStr = "SELECT SpeName,EvacAlpha,EvacBeta,SizeAlpha,SizeBeta,SpeIndex,Age FROM MSVPASizePref WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND SpeName = '" + SpeName + "'" +
               " ORDER BY SpeIndex, Age";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["EvacAlpha"].size() > 0) {
            for (int i = 0; i <= Nage[0]; ++i) {
                item = new QStandardItem(QString::fromStdString(dataMap["EvacAlpha"][m]));
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, 0, item);
                item = new QStandardItem(QString::fromStdString(dataMap["EvacBeta"][m]));
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, 1, item);
                item = new QStandardItem(QString::fromStdString(dataMap["SizeAlpha"][m]));
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, 2, item);
                item = new QStandardItem(QString::fromStdString(dataMap["SizeBeta"][m]));
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, 3, item);
                ++m;
            } // end for j
    } else {
        for (int i = 0; i <= Nage[0]; ++i) {
            for (int j=0; j<NUM_COLUMNS; ++j) {
                item = new QStandardItem("");
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, j, item);
            } // end for j
        } // end for i
    }

    MSVPA_Tab7_SizePrefsTV->setModel(smodel);

    MSVPA_Tab7_SavePB->setEnabled(false);

    bool ifDataExist = nmfUtilsQt::allCellsArePopulated(MSVPA_Tabs,
                                                        MSVPA_Tab7_SizePrefsTV,
                                                        nmfConstants::DontShowError);
    MSVPA_Tab7_NextPB->setEnabled(ifDataExist);
    MarkAsClean();

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab7::loadWidgets Complete");

    return true;
} // end loadWidgets



void
nmfMSVPATab7::makeHeaders(int nRows)
{
    QStringList strList;

    // Make row headers
    for (int k=0; k<=nRows; ++k) {
        strList << "Age " + QString::number(k);
    }
    smodel->setVerticalHeaderLabels(strList);

    // Make column headers
    strList.clear();
    strList << "Evac Rate Alpha";
    strList << "Evac Rate Beta";
    strList << "Size Pref Alpha";
    strList << "Size Pref Beta";
    smodel->setHorizontalHeaderLabels(strList);

} // end makeHeaders


void
nmfMSVPATab7::callback_MSVPA_Tab7_SpeciesCMB(int index)
{
    int m;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    QStandardItem *item;
    std::string SpeName = MSVPA_Tab7_SpeciesCMB->currentText().toStdString();

    smodel->clear();

    disconnect(smodel, SIGNAL(itemChanged(QStandardItem *)),
               this,   SLOT(callback_MSVPA_Tab7_ItemChanged(QStandardItem *)));

    makeHeaders(Nage[index]);

    smodel->setRowCount(Nage[index]+1);
    smodel->setColumnCount(NUM_COLUMNS);

    // Load the model which will then load the table
    fields   = {"Age","SpeName","EvacAlpha","EvacBeta","SizeAlpha","SizeBeta"};
    queryStr = "SELECT Age,SpeName,EvacAlpha,EvacBeta,SizeAlpha,SizeBeta FROM MSVPASizePref WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND SpeName = '" + SpeName + "'" +
               " ORDER By Age";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["EvacAlpha"].size() > 0) {
        m = 0;
        for (int i=0; i <= Nage[index]; ++i) {
            item = new QStandardItem(QString::fromStdString(dataMap["EvacAlpha"][m]));
            item->setTextAlignment(Qt::AlignCenter);
            smodel->setItem(i, 0, item);
            item = new QStandardItem(QString::fromStdString(dataMap["EvacBeta"][m]));
            item->setTextAlignment(Qt::AlignCenter);
            smodel->setItem(i, 1, item);
            item = new QStandardItem(QString::fromStdString(dataMap["SizeAlpha"][m]));
            item->setTextAlignment(Qt::AlignCenter);
            smodel->setItem(i, 2, item);
            item = new QStandardItem(QString::fromStdString(dataMap["SizeBeta"][m]));
            item->setTextAlignment(Qt::AlignCenter);
            smodel->setItem(i, 3, item);
            ++m;
        } // end for i
    } else {
        for (int i = 0; i <= Nage[index]; ++i) {
            for (int j=0; j < NUM_COLUMNS; ++j) {
                item = new QStandardItem("");
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, j, item);
            } // end for j
        } // end for i
    }

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab7_ItemChanged(QStandardItem *)));

} // end callback_MSVPA_Tab7_SpeciesCMB


void
nmfMSVPATab7::callback_MSVPA_Tab7_PrevPB(bool unused)
{
    int prevPage = MSVPA_Tabs->currentIndex()-1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(prevPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(prevPage);
    emit UpdateNavigator("MSVPA",prevPage);
}

void
nmfMSVPATab7::callback_MSVPA_Tab7_NextPB(bool unused)
{
    int nextPage = MSVPA_Tabs->currentIndex()+1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(nextPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(nextPage);
    emit UpdateNavigator("MSVPA",nextPage);
}
