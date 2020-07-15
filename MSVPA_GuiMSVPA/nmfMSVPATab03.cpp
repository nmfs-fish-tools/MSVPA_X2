
#include "nmfConstants.h"
#include "nmfUtils.h"
#include "nmfUtilsQt.h"

#include "nmfMSVPATab03.h"


nmfMSVPATab3::nmfMSVPATab3(QTabWidget *tabs,
                           nmfLogger *theLogger,
                           std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab3::nmfMSVPATab3");

    MSVPA_Tabs = tabs;
    databasePtr = NULL;
    MSVPAOtherPredCSVFile.clear();
    FirstYear = 0;
    ProjectDir = theProjectDir;
    Enabled = false;

    // Load ui as a widget from disk
    QFile file(":/forms/MSVPA/MSVPA_Tab03.ui");
    file.open(QFile::ReadOnly);
    MSVPA_Tab3_Widget = loader.load(&file,MSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    MSVPA_Tabs->addTab(MSVPA_Tab3_Widget, tr("3. Biomass Predators"));

    MSVPA_Tab3_PrevPB        = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab3_PrevPB");
    MSVPA_Tab3_NextPB        = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab3_NextPB");
    MSVPA_Tab3_LoadPB        = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab3_LoadPB");
    MSVPA_Tab3_SavePB        = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab3_SavePB");
    MSVPA_Tab3SubTab2_SetNumCatsPB  = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab3SubTab2_SetNumCatsPB");
    MSVPA_Tab3_OtherPredSpeciesCMB  = MSVPA_Tabs->findChild<QComboBox   *>("MSVPA_Tab3_OtherPredSpeciesCMB");
    MSVPA_Tab3_BiomassTV            = MSVPA_Tabs->findChild<QTableView  *>("MSVPA_Tab3_BiomassTV");
    MSVPA_Tab3_FeedingDataTV        = MSVPA_Tabs->findChild<QTableView  *>("MSVPA_Tab3_FeedingDataTV");
    MSVPA_Tab3_OtherPredTabs        = MSVPA_Tabs->findChild<QTabWidget  *>("MSVPA_Tab3_OtherPredTabs");
    MSVPA_Tab3SubTab1_BiomassLBL    = MSVPA_Tabs->findChild<QLabel      *>("MSVPA_Tab3SubTab1_BiomassLBL");
    MSVPA_Tab3SubTab2_FeedingLBL    = MSVPA_Tabs->findChild<QLabel      *>("MSVPA_Tab3SubTab2_FeedingLBL");

    MSVPA_Tab3_PrevPB->setText("\u25C1--");
    MSVPA_Tab3_NextPB->setText("--\u25B7");

    connect(MSVPA_Tab3_LoadPB, SIGNAL(clicked(bool)),
            this,              SLOT(callback_MSVPA_Tab3_LoadPB(bool)));
    connect(MSVPA_Tab3_SavePB, SIGNAL(clicked(bool)),
            this,              SLOT(callback_MSVPA_Tab3_SavePB(bool)));
    connect(MSVPA_Tab3_PrevPB, SIGNAL(clicked(bool)),
            this,              SLOT(callback_MSVPA_Tab3_PrevPB(bool)));
    connect(MSVPA_Tab3_NextPB, SIGNAL(clicked(bool)),
            this,              SLOT(callback_MSVPA_Tab3_NextPB(bool)));
    connect(MSVPA_Tab3SubTab2_SetNumCatsPB, SIGNAL(clicked(bool)),
            this,                           SLOT(callback_MSVPA_Tab3SubTab2_SetNumCatsPB(bool)));
    connect(MSVPA_Tab3_OtherPredSpeciesCMB, SIGNAL(currentIndexChanged(QString)),
            this,                           SLOT(callback_MSVPA_Tab3_OtherPredSpeciesCMB(QString)));
    connect(MSVPA_Tab3_OtherPredTabs,       SIGNAL(currentChanged(int)),
            this,                           SLOT(callback_FeedingDataEnabled(int)));

    MSVPA_Tab3_LoadPB->setEnabled(true);
    MSVPA_Tab3_NextPB->setEnabled(true);
    MSVPA_Tab3_LoadPB->setEnabled(true);
    MSVPA_Tab3_PrevPB->setEnabled(true);
    MSVPA_Tab3SubTab2_SetNumCatsPB->setEnabled(true);
    MSVPA_Tab3SubTab2_SetNumCatsPB->hide(); // This functionality is determined by when users create the Other Pred Species;

    for (int i=0; i<nmfConstants::MaxNumberSpecies; ++i) {
        smodelBiomass[i] = NULL;
        smodelFeeding[i] = NULL;
    }

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab3::nmfMSVPATab3 Complete");

} // end nmfMSVPATab3 constructor


nmfMSVPATab3::~nmfMSVPATab3()
{

} // end nmfMSVPATab3 destructor


void
nmfMSVPATab3::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir


void nmfMSVPATab3::clearWidgets()
{
    for (int i=0; i<nmfConstants::MaxNumberSpecies; ++i) {
        if (smodelBiomass[i])
            smodelBiomass[i]->clear();
        if (smodelFeeding[i])
            smodelFeeding[i]->clear();
    }
} // end clearWidgets



bool
nmfMSVPATab3::allFieldsAreFilled(QString dataSource)
{
    int numRows,numCols;
    QString OtherPredSpecies;
    int OtherPredSpeciesIndex;
    int NumOtherPredSpecies = MSVPA_Tab3_OtherPredSpeciesCMB->count();
    QStandardItem *item;

    // Check Biomass Data for missing fields
    if (dataSource == "BiomassData") {
        for (int i=0; i<NumOtherPredSpecies; ++i) {
            OtherPredSpecies      = MSVPA_Tab3_OtherPredSpeciesCMB->itemText(i);
            OtherPredSpeciesIndex = OtherPredSpeciesIndexMap[OtherPredSpecies];
            numRows = smodelBiomass[OtherPredSpeciesIndex]->rowCount();
            numCols = smodelBiomass[OtherPredSpeciesIndex]->columnCount();
            for (int j=0; j<numRows; ++j) {
                for (int k=0; k<numCols; ++k) {
                    item = smodelBiomass[OtherPredSpeciesIndex]->item(j,k);
                    if (item) {
                        if (item->text().isEmpty()) {
                            return false;
                        }
                    } else {
                        return false;
                    }
                }
            }
        }
    } else if (dataSource == "FeedingData") {
        for (int i=0; i<NumOtherPredSpecies; ++i) {
            OtherPredSpecies      = MSVPA_Tab3_OtherPredSpeciesCMB->itemText(i);
            OtherPredSpeciesIndex = OtherPredSpeciesIndexMap[OtherPredSpecies];
            numRows = smodelFeeding[OtherPredSpeciesIndex]->rowCount();
            numCols = smodelFeeding[OtherPredSpeciesIndex]->columnCount();
            for (int j=0; j<numRows; ++j) {
                for (int k=0; k<numCols; ++k) {
                    item = smodelFeeding[OtherPredSpeciesIndex]->item(j,k);
                    if (item) {
                        if (item->text().isEmpty()) {
                            return false;
                        }
                    } else {
                        return false;
                    }
                }
            }
        }
    }

    return true;

} // end allFieldsAreFilled


void
nmfMSVPATab3::callback_FeedingDataEnabled(int tab)
{
// Disable this functionality for now.  It may not be needed.

//    if (tab == 1)
//        MSVPA_Tab3SubTab2_SetNumCatsPB->show();
//    else
//        MSVPA_Tab3SubTab2_SetNumCatsPB->hide();

} // end callback_FeedingDataEnabled


void
nmfMSVPATab3::callback_MSVPA_Tab3_LoadPB(bool unused)
{
    bool loadedOK;

    MSVPA_Tabs->setCursor(Qt::WaitCursor);

    loadBiomassData();
    loadedOK = loadFeedingData();

    MSVPA_Tabs->setCursor(Qt::ArrowCursor);

    if (loadedOK) {
        QMessageBox::information(MSVPA_Tab3_OtherPredTabs, "Load",
                                 tr("\nOther Pred Biomass and Other Pred Size Data loaded.\n"));
        MarkAsClean();
    }

} // end callback_MSVPA_Tab3_LoadPB


/*
 * This saves the Biomass and Feeding Data for all of the Other Predator Species.
 */
void
nmfMSVPATab3::callback_MSVPA_Tab3_SavePB(bool unused)
{

    MSVPA_Tabs->setCursor(Qt::WaitCursor);

    bool biomassOK = saveBiomassData();
    if (! biomassOK) {
        MSVPA_Tabs->setCursor(Qt::ArrowCursor);
        return;
    }
    bool feedingOK = saveFeedingData();
    if (! feedingOK) {
        MSVPA_Tabs->setCursor(Qt::ArrowCursor);
        return;
    }

    MSVPA_Tabs->setCursor(Qt::ArrowCursor);

    if (biomassOK && feedingOK) {
        QMessageBox::information(MSVPA_Tab3_OtherPredTabs, "Save",
                                 tr("\nOther Pred Biomass and Other Pred Size Data saved.\n"));
    }

    MarkAsClean();

} // end callback_MSVPA_Tab3_SavePB


void
nmfMSVPATab3::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    // Restore Biomass Data
    TableName = "OtherPredBM";
    fields    = {"SpeName","Year","Biomass"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    // Restore Feeding Data
    TableName = "OthPredSizeData";
    fields    = {"SpeIndex","SpeName","SizeCat","MinLen","MaxLen","PropBM",
                 "ConsAlpha","ConsBeta","SizeSelAlpha","SizeSelBeta"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase


void
nmfMSVPATab3::loadBiomassData()
{
    QString line;
    QModelIndex index;
    QStringList fields;
    QString csvSpecies;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(MSVPA_Tab3_OtherPredTabs);
    QStringList NameFilters;
    QString OtherPredBMCSVFile;
    QString OtherPredSpecies;
    int OtherPredSpeciesIndex;
    int NumOtherPredSpecies = MSVPA_Tab3_OtherPredSpeciesCMB->count();

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile("OtherPredBM.csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load Other Predator Biomass CSV File");
    if (fileDlg.exec()) {

        for (int i=0; i<NumOtherPredSpecies; ++i)
        {
            OtherPredSpecies      = MSVPA_Tab3_OtherPredSpeciesCMB->itemText(i);
            OtherPredSpeciesIndex = OtherPredSpeciesIndexMap[OtherPredSpecies];

            // Open the file here....
            OtherPredBMCSVFile = fileDlg.selectedFiles()[0];
            QFile file(OtherPredBMCSVFile);
            if(! file.open(QIODevice::ReadOnly)) {
                QMessageBox::information(MSVPA_Tab3_OtherPredTabs, "File Read Error", file.errorString());
                return;
            }
            QTextStream inStream(&file);
            int modelRow = 0;
            int csvFileRow = 0;

            while (! inStream.atEnd()) {
                line = inStream.readLine();
                if (csvFileRow > 0) { // skip row=0 as it's the header
                    fields = line.split(",");
                    csvSpecies = fields[0].trimmed();
                    if (csvSpecies == OtherPredSpecies) {
                        for (int j=2; j<fields.size(); ++j) {
                            index = smodelBiomass[OtherPredSpeciesIndex]->index(modelRow,j-2);
                            smodelBiomass[OtherPredSpeciesIndex]->setData(index,fields[j].trimmed());
                        }
                        ++modelRow;
                    }
                }
                ++csvFileRow;
            }
            file.close();

        }
        MSVPA_Tab3_SavePB->setEnabled(true);
        MSVPA_Tab3_NextPB->setEnabled(false);
    }
    MSVPA_Tab3_LoadPB->clearFocus();
    MSVPA_Tab3_BiomassTV->resizeColumnsToContents();

} // end loadBiomassData


bool
nmfMSVPATab3::loadFeedingData()
{
    bool retv = false;
    QString line;
    QStringList fields;
    QStringList HorizHeader;
    QString csvSpecies;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(MSVPA_Tab3_OtherPredTabs);
    QStringList NameFilters;
    QString OtherPredBMCSVFile;
    int NumSizeCats = 0;
    QString OtherPredSpecies;
    int OtherPredSpeciesIndex=0;
    int NumOtherPredSpecies = MSVPA_Tab3_OtherPredSpeciesCMB->count();

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile("OthPredSizeData.csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load Other Predator Size Data CSV File");

    if (fileDlg.exec()) {
        QStandardItem *item;
        int modelRow = 0;
        int csvFileRow = 0;

        for (int i=0; i<NumOtherPredSpecies; ++i)
        {
            NumSizeCats = 0;
            HorizHeader.clear();
            OtherPredSpecies      = MSVPA_Tab3_OtherPredSpeciesCMB->itemText(i);
            OtherPredSpeciesIndex = OtherPredSpeciesIndexMap[OtherPredSpecies];

            disconnect(smodelFeeding[OtherPredSpeciesIndex], SIGNAL(itemChanged(QStandardItem *)),
                       this,          SLOT(callback_MSVPA_Tab3SubTab2_ItemChanged(QStandardItem *)));

            // Open the file here....
            OtherPredBMCSVFile = fileDlg.selectedFiles()[0];
            QFile file(OtherPredBMCSVFile);
            if(! file.open(QIODevice::ReadOnly)) {
                QMessageBox::information(MSVPA_Tab3_OtherPredTabs, "File Read Error", file.errorString());
                return false;
            }

            QTextStream inStream(&file);
            while (! inStream.atEnd()) {
                line = inStream.readLine();
                if (csvFileRow > 0) { // skip row=0 as it's the header
                    fields = line.split(",");
                    csvSpecies = fields[1].trimmed();
                    if (csvSpecies == OtherPredSpecies) {
                        ++NumSizeCats;
                    }
                }
                ++csvFileRow;
            }

            // Rewind file handle and this time re-read and parse data
            inStream.seek(0);
            csvFileRow = 0;
            modelRow = 0;
            smodelFeeding[OtherPredSpeciesIndex]->setColumnCount(NumSizeCats);
            smodelFeeding[OtherPredSpeciesIndex]->setRowCount(nmfConstants::NumFeedingDataRows);
            while (! inStream.atEnd()) {
                line = inStream.readLine();
                if (csvFileRow > 0) { // skip row=0 as it's the header
                    fields = line.split(",");
                    csvSpecies = fields[1].trimmed();
                    if (csvSpecies == OtherPredSpecies) {
                        for (int j=3; j<fields.size(); ++j) {
                            item = new QStandardItem(fields[j].trimmed());
                            item->setTextAlignment(Qt::AlignCenter);
                            smodelFeeding[OtherPredSpeciesIndex]->setItem(j-3,modelRow,item);
                        }
                        HorizHeader << (fields[3].trimmed() + "-" + fields[4].trimmed());
                        ++modelRow;
                    }
                }
                ++csvFileRow;
            }
            file.close();

            connect(smodelFeeding[OtherPredSpeciesIndex], SIGNAL(itemChanged(QStandardItem *)),
                    this,          SLOT(callback_MSVPA_Tab3SubTab2_ItemChanged(QStandardItem *)));
        }

        MSVPA_Tab3_SavePB->setEnabled(true);
        MSVPA_Tab3_NextPB->setEnabled(false);

        // Re-set the column headers
        smodelFeeding[OtherPredSpeciesIndex]->setHorizontalHeaderLabels(HorizHeader);

        retv = true;

    } else {

        retv = false;
    }
    MSVPA_Tab3_LoadPB->clearFocus();


    MSVPA_Tab3_SavePB->setEnabled(true);
    MSVPA_Tab3_FeedingDataTV->resizeColumnsToContents();

    return retv;

} // end loadFeedingData


bool
nmfMSVPATab3::saveBiomassData()
{
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
    QString TableName = "OtherPredBM";
    QString OtherPredBMCSVFile;
    QString OtherPredSpecies;
    int OtherPredSpeciesIndex;

    if (! allFieldsAreFilled("BiomassData")) {

        QMessageBox::information(MSVPA_Tabs,
                                 tr("Missing Data"),
                                 tr("\nPlease fill in any missing Biomass Data for all Predator Species."),
                                 QMessageBox::Ok);
        return false;
    }

//    if (! nmfQtUtils::allCellsArePopulated(MSVPA_Tab3_OtherPredTabs,
//                                           MSVPA_Tab3_BiomassTV,
//                                           nmfConstants::ShowError))
//        return false;

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (OtherPredBMCSVFile.isEmpty()) {
        OtherPredBMCSVFile = "OtherPredBM.csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPath    = QDir(filePath).filePath(OtherPredBMCSVFile);
        tmpFileNameWithPath = QDir(filePath).filePath("."+OtherPredBMCSVFile);
    } else {
        QFileInfo finfo(OtherPredBMCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPath    = OtherPredBMCSVFile;
        tmpFileNameWithPath = QDir(filePath).filePath("."+fileName);
    }
    OtherPredBMCSVFile.clear();

    // Read entire file and copy all lines that don't have the current Species
    // into temp output file which begins with ".". For the lines that need updating,
    // read the data from the qtableview, create a line out output, and write it to
    // the output file.

    int NumOtherPredSpecies = MSVPA_Tab3_OtherPredSpeciesCMB->count();
    for (int i=0; i< NumOtherPredSpecies; ++i)
    {
        OtherPredSpecies      = MSVPA_Tab3_OtherPredSpeciesCMB->itemText(i);
        OtherPredSpeciesIndex = OtherPredSpeciesIndexMap[OtherPredSpecies];

        QFile fin(fileNameWithPath);
        QFile fout(tmpFileNameWithPath);
        // Open the files here....
        if (! fin.open(QIODevice::ReadOnly)) {
            QMessageBox::information(MSVPA_Tab3_OtherPredTabs,
                                     "Error Opening Input File",
                                     fin.errorString());
            return false;
        }
        if (! fout.open(QIODevice::WriteOnly)) {
            QMessageBox::information(MSVPA_Tab3_OtherPredTabs,
                                     "Error Opening Output File",
                                     fout.errorString());
            return false;
        }
        QTextStream inStream(&fin);
        QTextStream outStream(&fout);

        QString csvSpecies;
        QString tableLine;
        QModelIndex index;
        int numRows = smodelBiomass[OtherPredSpeciesIndex]->rowCount();
        bool ignoreSpecies = false;
        while (! inStream.atEnd()) {
            line    = inStream.readLine();
            qfields = line.split(",");
            csvSpecies = qfields[0].trimmed();
            if (csvSpecies == OtherPredSpecies) { // dump qtableview data into output file
                // Skip over all other Species rows, since going through this once copies the qtableview data
                if (! ignoreSpecies) {
                    for (int i=0; i<numRows; ++i) {
                        tableLine = OtherPredSpecies + ", " + QString::number(i+FirstYear) + ", ";
                        index = smodelBiomass[OtherPredSpeciesIndex]->index(i,0);
                        tableLine += index.data().toString();
                        outStream << tableLine << "\n";
                    }
                    ignoreSpecies = true;
                }
            } else { // if it's not data for the current Species (ie, header), copy it to the new "." file
                outStream << line << "\n";
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
            return false;
        }
    } // end for OtherPredSpecies

    //
    // Save to mysql table
    //

    // Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableName;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (errorMsg != " ") {
        logger->logMsg(nmfConstants::Normal,"nmfMSVPATab3 Save(1a): Clearing table error: " + errorMsg);
        return false;
    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableName.toStdString();
    cmd += "(SpeIndex,SpeName,Year,Biomass) VALUES ";

    // Read from csv file since it has all the Species in it
    QFile fin2(fileNameWithPath);
    if (! fin2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tab3_OtherPredTabs,
                                 "Error OtherPredSpecies(1) Opening Input File",
                                 fin2.errorString());
        return false;
    }
    QTextStream inStream2(&fin2);

    FirstLine = true;
    QString FileSpeName;
    while (! inStream2.atEnd()) {
        line = inStream2.readLine();
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields   = line.split(",");
        FileSpeName = qfields[0].trimmed();
        cmd += "(" + std::to_string(OtherPredSpeciesIndexMap[FileSpeName]) + ",";
        cmd += "\"" + FileSpeName.toStdString() + "\",";
        cmd += qfields[1].trimmed().toStdString() + ", ";
        cmd += qfields[2].trimmed().toStdString() + "), ";
    } // end for all rows in csv file

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    fin2.close();
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (errorMsg != " ") {
        std::cout << cmd << std::endl;
        logger->logMsg(nmfConstants::Normal,"nmfMSVPATab3 Save(2a): Write table error: " + errorMsg);
        return false;
    }

    MSVPA_Tab3_SavePB->setEnabled(false);
    MSVPA_Tab3_NextPB->setEnabled(true);

    return true;

} // end saveBiomassData


bool
nmfMSVPATab3::saveFeedingData()
{
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
    QString TableName = "OthPredSizeData";
    QString OtherPredSizeDataCSVFile;
    QString OtherPredSpecies;
    int OtherPredSpeciesIndex;

    if (! allFieldsAreFilled("FeedingData")) {

        QMessageBox::information(MSVPA_Tabs,
                                 tr("Missing Data"),
                                 tr("\nPlease fill in any missing Feeding Data for all Predator Species."),
                                 QMessageBox::Ok);
        return false;
    }

//    if (! nmfQtUtils::allCellsArePopulated(MSVPA_Tab3_OtherPredTabs,
//                                           MSVPA_Tab3_FeedingDataTV,
//                                           nmfConstants::ShowError))
//        return;

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (OtherPredSizeDataCSVFile.isEmpty()) {
        OtherPredSizeDataCSVFile = "OthPredSizeData.csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPath    = QDir(filePath).filePath(OtherPredSizeDataCSVFile);
        tmpFileNameWithPath = QDir(filePath).filePath("."+OtherPredSizeDataCSVFile);
    } else {
        QFileInfo finfo(OtherPredSizeDataCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPath    = OtherPredSizeDataCSVFile;
        tmpFileNameWithPath = QDir(filePath).filePath("."+fileName);
    }

    // Read entire file and copy all lines that don't have the current Species into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    int NumOtherPredSpecies = MSVPA_Tab3_OtherPredSpeciesCMB->count();
    for (int i=0; i< NumOtherPredSpecies; ++i)
    {
        OtherPredSpecies      = MSVPA_Tab3_OtherPredSpeciesCMB->itemText(i);
        OtherPredSpeciesIndex = OtherPredSpeciesIndexMap[OtherPredSpecies];

        QFile fin(fileNameWithPath);
        QFile fout(tmpFileNameWithPath);
        // Open the files here....
        if (! fin.open(QIODevice::ReadOnly)) {
            QMessageBox::information(MSVPA_Tab3_OtherPredTabs, "Error Opening Input File", fin.errorString());
            return false;
        }
        if (! fout.open(QIODevice::WriteOnly)) {
            QMessageBox::information(MSVPA_Tab3_OtherPredTabs, "Error Opening Output File", fout.errorString());
            return false;
        }
        QTextStream inStream(&fin);
        QTextStream outStream(&fout);

        //int NumCategoriesInFile=0;
        //int NumCategoriesInModel = smodelFeeding[OtherPredSpeciesIndex]->columnCount();

        QString csvSpecies;
        QString tableLine;
        QModelIndex index;
        int numRows = smodelFeeding[OtherPredSpeciesIndex]->rowCount();
        int col = 0;
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            qfields = line.split(",");
            csvSpecies = qfields[1].trimmed();
            if (csvSpecies == OtherPredSpecies) { // dump qtableview data into output file
                // Skip over all other Species rows, since going through this once copies the qtableview data
                tableLine =  QString::number(OtherPredSpeciesIndexMap[OtherPredSpecies]) + ", " +
                             OtherPredSpecies + ", " +
                             QString::number(col);
                for (int i=0; i<numRows; ++i) {
                    index = smodelFeeding[OtherPredSpeciesIndex]->index(i,col);
                    tableLine += ", " + index.data().toString();
                }
                outStream << tableLine + "\n";
                ++col;
    //            if (col >= NumCategoriesInModel)
    //                break;
            } else { // if it's not data for the current Species (ie, header), copy it to the new "." file
                outStream << line << "\n";
            }
        }
    //    if (col < NumCategoriesInModel) {
    //        for (int i=0; i<=(NumCategoriesInModel-col); ++i) {
    //            // Skip over all other Species rows, since going through this once copies the qtableview data
    //            tableLine =  QString::number(OtherPredSpeciesIndexMap[OtherPredSpecies]) + ", " +
    //                         OtherPredSpecies + ", " +
    //                         QString::number(col);
    //            for (int i=0; i<numRows; ++i) {
    //                index = smodelFeeding[OtherPredSpeciesIndex]->index(i,col);
    //                tableLine += ", " + index.data().toString();
    //            }
    //            ++col;
    //            outStream << tableLine + "\n";
    //        }
    //    }

        fin.close();
        fout.close();

        // Now, mv temp file to actual file.
        int retv = nmfUtilsQt::rename(tmpFileNameWithPath, fileNameWithPath);
        if (retv < 0) {
            std::cout << "Error: Couldn't rename " <<
                         tmpFileNameWithPath.toStdString() << " to " <<
                         fileNameWithPath.toStdString() <<
                         ". Save aborted." << std::endl;
            return false;
        }

    } // end while OtherPredSpecies

    //
    // Save to mysql table
    //

    // Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableName;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (errorMsg != " ") {
        logger->logMsg(nmfConstants::Normal,"nmfMSVPATab3 Save(2b): Clearing table error: " + errorMsg);
        return false;
    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableName.toStdString();
    cmd += "(SpeIndex,SpeName,SizeCat,MinLen,MaxLen,PropBM,ConsAlpha,ConsBeta,SizeSelAlpha,SizeSelBeta) VALUES ";

    // Read from csv file since it has all the Species in it
    QFile fin2(fileNameWithPath);
    if (! fin2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tab3_OtherPredTabs,
                                 "Error OtherPredSpecies(1) Opening Input File",
                                 fin2.errorString());
        return false;
    }
    QTextStream inStream2(&fin2);

    FirstLine = true;
    QString FileSpeName;

    while (! inStream2.atEnd()) {
        line = inStream2.readLine();
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields   = line.split(",");
        FileSpeName = qfields[1].trimmed();
        cmd += "(" + std::to_string(OtherPredSpeciesIndexMap[FileSpeName]) + ",";
        cmd += "\"" + FileSpeName.toStdString() + "\",";
        cmd += qfields[2].trimmed().toStdString() + ", ";
        cmd += qfields[3].trimmed().toStdString() + ", ";
        cmd += qfields[4].trimmed().toStdString() + ", ";
        cmd += qfields[5].trimmed().toStdString() + ", ";
        cmd += qfields[6].trimmed().toStdString() + ", ";
        cmd += qfields[7].trimmed().toStdString() + ", ";
        cmd += qfields[8].trimmed().toStdString() + ", ";
        cmd += qfields[9].trimmed().toStdString() + "), ";
//        ++NumCategoriesInFile;
//        if (NumCategoriesInFile >= NumCategoriesInModel)
//            break;
    } // end for all rows in csv file

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    fin2.close();
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (errorMsg != " ") {
        std::cout << cmd << std::endl;
        logger->logMsg(nmfConstants::Normal,"nmfMSVPATab3 Save(2c): Write table error: " + errorMsg);
        logger->logMsg(nmfConstants::Normal,"cmd: " + cmd);
        return false;
    }

    MSVPA_Tab3_SavePB->setEnabled(false);
    MSVPA_Tab3_NextPB->setEnabled(true);

    return true;
} // end saveFeedingData


/*
 *
 * This function updates the data structure.
 */
void
nmfMSVPATab3::callback_MSVPA_Tab3SubTab1_ItemChanged(QStandardItem *currItem)
{
    double val;
    QString OtherPredSpecies;
    int OtherPredSpeciesIndex;

    currentPredSpecies(OtherPredSpecies, OtherPredSpeciesIndex);

    val = currItem->index().data().toDouble();
    //BiomassData[0][currItem->row()][currItem->column()] = val;

    QStandardItem *item = new QStandardItem(QString::number(val));
    item->setTextAlignment(Qt::AlignCenter);
    smodelBiomass[OtherPredSpeciesIndex]->setItem(item->row(), item->column(), item);

    MarkAsDirty("OtherPredBM");

    MSVPA_Tab3_BiomassTV->resizeColumnsToContents();
    MSVPA_Tab3_SavePB->setEnabled(true);
    MSVPA_Tab3_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab3SubTab1_ItemChanged


void
nmfMSVPATab3::callback_MSVPA_Tab3SubTab2_ItemChanged(QStandardItem *currItem)
{
    int row;
    int col;
    QStandardItem *item;
    QString OtherPredSpecies;
    int OtherPredSpeciesIndex;

    currentPredSpecies(OtherPredSpecies, OtherPredSpeciesIndex);

    item = new QStandardItem(QString::number(currItem->index().data().toDouble()));
    item->setTextAlignment(Qt::AlignCenter);
    row = item->row();
    col = item->column();
    smodelFeeding[OtherPredSpeciesIndex]->setItem(row, col, item);

    // If MinSize or MaxSize changed then update the header
    if (row <= 1) {
        updateFeedingTableHeader();
    }

    MarkAsDirty("OthPredSizeData");

    MSVPA_Tab3_FeedingDataTV->resizeColumnsToContents();

    MSVPA_Tab3_SavePB->setEnabled(true);
    MSVPA_Tab3_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab3SubTab2_ItemChanged

void
nmfMSVPATab3::MarkAsDirty(std::string tableName)
{
    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfMSVPATab3::MarkAsClean()
{
    emit MarkMSVPAAsClean();
} // end MarkAsClean


bool
nmfMSVPATab3::loadWidgets(nmfDatabase *theDatabasePtr,
                               std::string theMSVPAName)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::string Units = "";
    int NumYears=0;
    int OtherPredSpeciesIndex;
    int NumOtherPredSpecies;
    int speciesIndex;
    int NumSizeCats=0;
    int LastYear;
    int index;
    QString OtherPredSpecies;
    QString species;
    std::vector<int> OtherPredSpeciesIndexes;
    QStringList strList;

    databasePtr  = theDatabasePtr;
    OtherPredSpeciesIndexMap.clear();

    MSVPA_Tab3_OtherPredSpeciesCMB->blockSignals(true);
    // Load OtherPredSpecies to combobox
    fields   = {"SpeIndex","SpeName","FirstYear"};
    queryStr = "SELECT SpeIndex,SpeName,FirstYear FROM OtherPredSpecies;";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    MSVPA_Tab3_OtherPredSpeciesCMB->clear();
    NumOtherPredSpecies = dataMap["SpeName"].size();
    for (int i=0; i<NumOtherPredSpecies; i++) {
        species      = QString::fromStdString(dataMap["SpeName"][i]);
        speciesIndex = std::stoi(dataMap["SpeIndex"][i]);
        OtherPredSpeciesIndexMap[species] = speciesIndex;
        MSVPA_Tab3_OtherPredSpeciesCMB->addItem(species);
        OtherPredSpeciesIndexes.push_back(speciesIndex);

        // Set up models for each Other Pred Species
        smodelBiomass[speciesIndex] = new QStandardItemModel(nmfConstants::MaxNumberYears, nmfConstants::MaxNumberAges+1);
        smodelFeeding[speciesIndex] = new QStandardItemModel(nmfConstants::NumFeedingDataRows, nmfConstants::MaxNumberSizeCats);
    }
    MSVPA_Tab3_OtherPredSpeciesCMB->blockSignals(false);

    currentPredSpecies(OtherPredSpecies, OtherPredSpeciesIndex);

    // Make connections
    getInitData(OtherPredSpecies,Units,NumSizeCats,FirstYear,LastYear);
    NumYears = LastYear - FirstYear + 1;
    for (int i=0; i<NumOtherPredSpecies; ++i) {
        index = OtherPredSpeciesIndexes[i];
        smodelBiomass[index]->setRowCount(NumYears);
        smodelBiomass[index]->setColumnCount(1);

        smodelFeeding[index]->setRowCount(nmfConstants::NumFeedingDataRows);
        smodelFeeding[index]->setColumnCount(0);

        connect(smodelBiomass[index], SIGNAL(itemChanged(QStandardItem *)),
                this,                 SLOT(callback_MSVPA_Tab3SubTab1_ItemChanged(QStandardItem *)));
        connect(smodelFeeding[index], SIGNAL(itemChanged(QStandardItem *)),
                this,                 SLOT(callback_MSVPA_Tab3SubTab2_ItemChanged(QStandardItem *)));

        // Make row header for Biomass
        strList.clear();
        for (int j=0; j<NumYears; ++j) {
            strList << " " + QString::number(FirstYear+j) + " ";
        }
        smodelBiomass[index]->setVerticalHeaderLabels(strList);
    }
    MSVPA_Tab3_BiomassTV->setModel(smodelBiomass[OtherPredSpeciesIndexes[0]]);
    MSVPA_Tab3_FeedingDataTV->setModel(smodelFeeding[OtherPredSpeciesIndexes[0]]);

    MSVPA_Tab3_NextPB->setEnabled(true);
    for (int i=0; i<NumOtherPredSpecies; ++i) {
        OtherPredSpecies      = MSVPA_Tab3_OtherPredSpeciesCMB->itemText(i);
        OtherPredSpeciesIndex = OtherPredSpeciesIndexMap[OtherPredSpecies];
        loadSubTab1("fromTable", OtherPredSpecies, OtherPredSpeciesIndex, Units, NumYears);
        loadSubTab2("fromTable", OtherPredSpecies, OtherPredSpeciesIndex);
    }

    MarkAsClean();

    return true;

} // loadWidgets


void
nmfMSVPATab3::currentPredSpecies(QString &OtherPredSpecies, int &OtherPredSpeciesIndex)
{

     OtherPredSpecies = MSVPA_Tab3_OtherPredSpeciesCMB->currentText();
     OtherPredSpeciesIndex = OtherPredSpeciesIndexMap[OtherPredSpecies];

} // end currentPredSpecies

void
nmfMSVPATab3::loadSubTab1(QString dataSource,
                          QString OtherPredSpecies,
                          int OtherPredSpeciesIndex,
                          std::string Units,
                          int NumYears)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::string labelTxt;
    QStringList strList;
    int LastYear;
    QStandardItem *item;
    bool csvFileEmpty = false;

    smodelBiomass[OtherPredSpeciesIndex]->blockSignals(true);

    //
    // Set up Sub Tab: Biomass
    //
    labelTxt = "Enter annual biomass in " + Units + " for: " + OtherPredSpecies.toStdString();
    MSVPA_Tab3SubTab1_BiomassLBL->setText(labelTxt.c_str());

    if (dataSource == "fromTable") {
        // Get table data for Biomass tableView and load into model
        fields   = {"SpeIndex","SpeName","Year","Biomass"};
        queryStr = "SELECT SpeIndex,SpeName,Year,Biomass FROM OtherPredBM WHERE SpeName='" + OtherPredSpecies.toStdString() +"'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
        csvFileEmpty = (dataMap["SpeIndex"].size() == 0);
        for (int i=0; i<NumYears; ++i) {
            if (csvFileEmpty) {
                item = smodelBiomass[OtherPredSpeciesIndex]->item(i,0);
                if (item) {
                    item = new QStandardItem(item->text());
                } else {
                    item = new QStandardItem("");
                }
            } else {
                item = new QStandardItem(QString::fromStdString(dataMap["Biomass"][i]));
            }
            item->setTextAlignment(Qt::AlignCenter);
            smodelBiomass[OtherPredSpeciesIndex]->setItem(i, 0, item);
        }
        if (csvFileEmpty) {
            MSVPA_Tab3_NextPB->setEnabled(false);
        }

    } else {
        for (int i=0; i<NumYears; ++i) {
            item = smodelBiomass[OtherPredSpeciesIndex]->item(i,0);
            if (item) {
                item = new QStandardItem(item->text());
            } else {
                item = new QStandardItem("");
            }
            item->setTextAlignment(Qt::AlignCenter);
            smodelBiomass[OtherPredSpeciesIndex]->setItem(i, 0, item);
        }
    }

    smodelBiomass[OtherPredSpeciesIndex]->blockSignals(false);

    // Make Row headers
    strList.clear();
    for (int k=FirstYear; k<=LastYear; ++k) {
        strList << QString::number(k);
    }
    smodelBiomass[OtherPredSpeciesIndex]->setVerticalHeaderLabels(strList);

    // Make Column header
    strList.clear();
    strList << "Biomass";
    smodelBiomass[OtherPredSpeciesIndex]->setHorizontalHeaderLabels(strList);
    MSVPA_Tab3_BiomassTV->resizeColumnsToContents();

} // end loadSubTab1()


void
nmfMSVPATab3::loadSubTab2(QString dataSource,
                          QString OtherPredSpecies,
                          int OtherPredSpeciesIndex)
{
    std::map<std::string, std::vector<std::string> > dataMap,dataMap2;
    std::vector<std::string> fields,fields2;
    std::string queryStr,queryStr2;
    QStringList strList;
    std::string labelTxt;
    QStandardItem *item,*itemMinSize,*itemMaxSize;
    bool csvFileEmpty = false;
    int NumCats=0;
    bool useTableData = true;
    std::vector<double> data;
    std::string msg;

    //
    // Set up Sub Tab: Feeding Data
    //

    labelTxt = "Enter feeding data for each size class of: " + OtherPredSpecies.toStdString();
    MSVPA_Tab3SubTab2_FeedingLBL->setText(labelTxt.c_str());


    smodelFeeding[OtherPredSpeciesIndex]->blockSignals(true);

    // Load table data for Feeding
    fields   = {"SpeIndex","SpeName","SizeCat","MinLen","MaxLen","PropBM","ConsAlpha","ConsBeta","SizeSelAlpha","SizeSelBeta"};
    queryStr = "SELECT SpeIndex,SpeName,SizeCat,MinLen,MaxLen,PropBM,ConsAlpha,ConsBeta,SizeSelAlpha,SizeSelBeta FROM OthPredSizeData WHERE SpeName='" +
                OtherPredSpecies.toStdString() +"'" +
               " ORDER by SizeCat";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    if (dataMap["SpeIndex"].size() == 0) {
        csvFileEmpty = true;
        //std::cout << "Warning nmfMSVPATab3::loadSubTab2 No records found for cmd:\n" << queryStr << std::endl;
    }
    if (dataSource == "fromTable") {
        NumCats  = dataMap["SpeName"].size();
    } else {
        NumCats =  smodelFeeding[OtherPredSpeciesIndex]->columnCount();
    }
//    if (NumCats == 0) {
//        useTableData = false;
//        NumCats =  smodelFeeding[OtherPredSpeciesIndex]->columnCount();
//    }


    for (int i=0; i<NumCats; ++i) {
        if (dataSource == "fromTable") {
            data = { std::stoi(dataMap["MinLen"][i]),
                     std::stoi(dataMap["MaxLen"][i]),
                     std::stod(dataMap["PropBM"][i]),
                     std::stod(dataMap["ConsAlpha"][i]),
                     std::stod(dataMap["ConsBeta"][i]),
                     std::stod(dataMap["SizeSelAlpha"][i]),
                     std::stod(dataMap["SizeSelBeta"][i]) };
            for (int j=0; j<nmfConstants::NumFeedingDataRows; ++j) {
                QStandardItem *item = new QStandardItem(QString::number(data[j]));
                item->setTextAlignment(Qt::AlignCenter);
                smodelFeeding[OtherPredSpeciesIndex]->setItem(j, i, item);
            }
        } else {
            item = smodelFeeding[OtherPredSpeciesIndex]->item(0,i);
            if (item) {
                for (int j=0; j<nmfConstants::NumFeedingDataRows; ++j) {
                    item = smodelFeeding[OtherPredSpeciesIndex]->item(j,i);
                    item = new QStandardItem(item->text());
                    item->setTextAlignment(Qt::AlignCenter);
                    smodelFeeding[OtherPredSpeciesIndex]->setItem(j, i, item);
                }
            } else {
                for (int j=0; j<nmfConstants::NumFeedingDataRows; ++j) {
                    item = new QStandardItem("");
                    item->setTextAlignment(Qt::AlignCenter);
                    smodelFeeding[OtherPredSpeciesIndex]->setItem(j, i, item);
                }
            }
        }

    } // end for i

 /*
    for (int i=0; i<NumCats; ++i) {
        if (csvFileEmpty) {
            item = smodelFeeding[OtherPredSpeciesIndex]->item(0,i);
            if (item) {
                for (int j=0; j<nmfConstants::NumFeedingDataRows; ++j) {
                    item = smodelFeeding[OtherPredSpeciesIndex]->item(j,i);
                    item = new QStandardItem(item->text());
                    item->setTextAlignment(Qt::AlignCenter);
                    smodelFeeding[OtherPredSpeciesIndex]->setItem(j, i, item);
                }
            } else {
                for (int j=0; j<nmfConstants::NumFeedingDataRows; ++j) {
                    item = new QStandardItem("");
                    item->setTextAlignment(Qt::AlignCenter);
                    smodelFeeding[OtherPredSpeciesIndex]->setItem(j, i, item);
                }
            }
        } else {
            data = { std::stoi(dataMap["MinLen"][i]),
                     std::stoi(dataMap["MaxLen"][i]),
                     std::stod(dataMap["PropBM"][i]),
                     std::stod(dataMap["ConsAlpha"][i]),
                     std::stod(dataMap["ConsBeta"][i]),
                     std::stod(dataMap["SizeSelAlpha"][i]),
                     std::stod(dataMap["SizeSelBeta"][i]) };
            for (int j=0; j<nmfConstants::NumFeedingDataRows; ++j) {
                QStandardItem *item = new QStandardItem(QString::number(data[j]));
                item->setTextAlignment(Qt::AlignCenter);
                smodelFeeding[OtherPredSpeciesIndex]->setItem(j, i, item);
            }
        }
    }
    */

    smodelFeeding[OtherPredSpeciesIndex]->blockSignals(false);

    // Make Row headers
    strList.clear();
    strList << "Minimum Length" << "Maximum Length" <<
               "Consumption Alpha" << "Consumption Beta" <<
               "Size Selection Alpha" << "Size Selection Beta" <<
               "Proportion of Biomass";
    smodelFeeding[OtherPredSpeciesIndex]->setVerticalHeaderLabels(strList);


    // Make Column header
    strList.clear();
    QString prefix = "";
    QString suffix = "";
    for (int i=0; i<NumCats; ++i) {
        //if (useTableData) {
        if (dataSource == "fromTable") {
            prefix = QString::fromStdString(dataMap["MinLen"][i]);
            suffix = QString::fromStdString(dataMap["MaxLen"][i]);
        } else {
            itemMinSize = smodelFeeding[OtherPredSpeciesIndex]->item(0,i);
            itemMaxSize = smodelFeeding[OtherPredSpeciesIndex]->item(1,i);
            if (itemMinSize && itemMaxSize) {
                prefix = smodelFeeding[OtherPredSpeciesIndex]->item(0,i)->text();
                suffix = smodelFeeding[OtherPredSpeciesIndex]->item(1,i)->text();
            }
        }
        strList << prefix + "-" + suffix;
    }
    smodelFeeding[OtherPredSpeciesIndex]->setHorizontalHeaderLabels(strList);
    MSVPA_Tab3_FeedingDataTV->resizeColumnsToContents();

    if (NumCats == 0) {
        fields2   = {"SpeIndex","SpeName","NumSizeCats"};
        queryStr2 = "SELECT SpeIndex,SpeName,NumSizeCats FROM OtherPredSpecies WHERE SpeName='" +
                OtherPredSpecies.toStdString() +"'";
        dataMap2  = databasePtr->nmfQueryDatabase(queryStr2,fields2);
        if (dataMap2["SpeIndex"].size() == 0) {
            msg  = "\nCould not find NumSizeCats data in OtherPredSPecies table for Species: ";
            msg +=  OtherPredSpecies.toStdString();
            QMessageBox::information(MSVPA_Tabs,
                                     tr("Missing Data"),
                                     tr(msg.c_str()),
                                     QMessageBox::Ok);
            return;
        } else {
            NumCats = std::stoi(dataMap2["NumSizeCats"][0]);
        }

        for (int i=0; i<NumCats; ++i) {
            for (int j=0; j<nmfConstants::NumFeedingDataRows; ++j) {
                item = new QStandardItem("");
                item->setTextAlignment(Qt::AlignCenter);
                smodelFeeding[OtherPredSpeciesIndex]->setItem(j, i, item);
            }
        }
    }

    if (csvFileEmpty) {
        MSVPA_Tab3_NextPB->setEnabled(false);
    }

} // end loadSubTab2


void
nmfMSVPATab3::getInitData(QString OtherPredSpecies,
                          std::string &Units, int &NumSizeCats,
                          int &FirstYear, int &LastYear)
{
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    int BMUnits;

    fields   = { "SpeIndex", "SpeName", "BMUnits", "NumSizeCats", "FirstYear", "LastYear" };
    queryStr = "SELECT SpeIndex,SpeName,BMUnits,NumSizeCats,FirstYear,LastYear FROM OtherPredSpecies WHERE SpeName='" +
                OtherPredSpecies.toStdString() + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    if (dataMap["SpeIndex"].size() == 0) {
        std::cout << "Error: nmfMSVPATab3::getInitData. No data found with following cmd:\n" << queryStr << std::endl;
        return;
    }
    BMUnits     = std::stoi(dataMap["BMUnits"][0]);
    NumSizeCats = std::stoi(dataMap["NumSizeCats"][0]);
    FirstYear   = std::stoi(dataMap["FirstYear"][0]);
    LastYear    = std::stoi(dataMap["LastYear"][0]);
    switch (BMUnits) {
        case 0:
            Units = "kilograms";
            break;
        case 1:
            Units = "Metric tons";
            break;
        case 2:
            Units = "000s of Metric tons";
            break;
        default:
            Units = "unknown";
            break;
    }

} // end getInitData


void
nmfMSVPATab3::callback_MSVPA_Tab3_OtherPredSpeciesCMB(QString species)
{
    int NumYears=0;
    QString OtherPredSpecies;
    int OtherPredSpeciesIndex;

    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;

    fields   = { "SpeIndex", "SpeName", "BMUnits", "NumSizeCats", "FirstYear", "LastYear" };
    queryStr = "SELECT SpeIndex,SpeName,BMUnits,NumSizeCats,FirstYear,LastYear FROM OtherPredSpecies WHERE SpeName='" +
                species.toStdString() + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    if (dataMap["SpeIndex"].size() > 0) {
        NumYears = std::stoi(dataMap["LastYear"][0]) -
                   std::stoi(dataMap["FirstYear"][0]) + 1;
    } else {
        std::cout << "Error nmfMSVPATab3::callback_MSVPA_Tab3_OtherPredSpeciesCMB cmd:\n" << queryStr << std::endl;
        return;
    }

    currentPredSpecies(OtherPredSpecies, OtherPredSpeciesIndex);

    MSVPA_Tab3_BiomassTV->setModel(smodelBiomass[OtherPredSpeciesIndex]);
    MSVPA_Tab3_FeedingDataTV->setModel(smodelFeeding[OtherPredSpeciesIndex]);
    MSVPA_Tab3_BiomassTV->resizeColumnsToContents();
    MSVPA_Tab3_FeedingDataTV->resizeColumnsToContents();

} // end callback_MSVPA_Tab3_PreySpeciesCMB



void
nmfMSVPATab3::callback_MSVPA_Tab3_PrevPB(bool unused)
{    
    int prevPage = MSVPA_Tabs->currentIndex()-1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(prevPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(prevPage);
    emit UpdateNavigator("MSVPA",prevPage);

} // end callback_MSVPA_Tab3_PrevPB


void
nmfMSVPATab3::callback_MSVPA_Tab3_NextPB(bool unused)
{
    int nextPage = MSVPA_Tabs->currentIndex()+1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(nextPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(nextPage);
    emit UpdateNavigator("MSVPA",nextPage);

} // end callback_MSVPA_Tab3_NextPB


void
nmfMSVPATab3::callback_MSVPA_Tab3SubTab2_SetNumCatsPB(bool unused)
{
    QStringList HorizHeaders;
    bool ok;
    QString OtherPredSpecies;
    int OtherPredSpeciesIndex;

    currentPredSpecies(OtherPredSpecies, OtherPredSpeciesIndex);

    int NumSizeCats = QInputDialog::getInt(MSVPA_Tab3_OtherPredTabs,
                                           tr("Number of Size Categories"),
                                           tr("Please enter number of Size categories."),
                                           1, 0, nmfConstants::MaxNumberOtherPredators, 1,
                                           &ok);
    if (ok && NumSizeCats > 0) {

        disconnect(smodelFeeding[OtherPredSpeciesIndex], SIGNAL(itemChanged(QStandardItem *)),
                   this,          SLOT(callback_MSVPA_Tab3SubTab2_ItemChanged(QStandardItem *)));

        smodelFeeding[OtherPredSpeciesIndex]->setColumnCount(NumSizeCats);
        for (int i=0; i<NumSizeCats; ++i) {
            HorizHeaders << "Size Category " + QString::number(i+1);
        }
        smodelFeeding[OtherPredSpeciesIndex]->setHorizontalHeaderLabels(HorizHeaders);
        MSVPA_Tab3_FeedingDataTV->setModel(smodelFeeding[OtherPredSpeciesIndex]);
        MSVPA_Tab3_FeedingDataTV->resizeColumnsToContents();

        connect(smodelFeeding[OtherPredSpeciesIndex], SIGNAL(itemChanged(QStandardItem *)),
                this,          SLOT(callback_MSVPA_Tab3SubTab2_ItemChanged(QStandardItem *)));

        updateFeedingTableHeader();

        MSVPA_Tab3_SavePB->setEnabled(true);
    }

} // end callback_MSVPA_Tab3SubTab2_SetNumCatsPB


void
nmfMSVPATab3::updateFeedingTableHeader()
{
    QStringList HorizHeader;
    QString OtherPredSpecies;
    int OtherPredSpeciesIndex;
    QStandardItem *itemMinSize,*itemMaxSize;

    currentPredSpecies(OtherPredSpecies, OtherPredSpeciesIndex);

    for (int i=0; i<smodelFeeding[OtherPredSpeciesIndex]->columnCount(); ++i) {
        itemMinSize = smodelFeeding[OtherPredSpeciesIndex]->item(0,i);
        itemMaxSize = smodelFeeding[OtherPredSpeciesIndex]->item(1,i);
        if (itemMinSize && itemMaxSize) {
            HorizHeader << itemMinSize->text() + "-" + itemMaxSize->text();
        } else {
            HorizHeader << "Size Category " + QString::number(i+1);
        }
    }
    smodelFeeding[OtherPredSpeciesIndex]->setHorizontalHeaderLabels(HorizHeader);

} // end updateFeedingTableHeader



