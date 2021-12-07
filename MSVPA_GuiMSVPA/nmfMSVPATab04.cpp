
#include "nmfConstants.h"
#include "nmfUtils.h"
#include "nmfUtilsQt.h"

#include "nmfMSVPATab04.h"



nmfMSVPATab4::nmfMSVPATab4(QTabWidget *tabs,
                           nmfLogger *theLogger,
                           std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::nmfMSVPATab4");

    MSVPA_Tabs = tabs;
    MSVPAOthPreyCSVFile.clear();
    MSVPAOthPreyAnnCSVFile.clear();
    MSVPAlistCSVFile.clear();
    ProjectDir = theProjectDir;
    Enabled = false;
    smodel = NULL;
    databasePtr = NULL;

    // Load ui as a widget from disk
    QFile file(":/forms/MSVPA/MSVPA_Tab04.ui");
    file.open(QFile::ReadOnly);
    MSVPA_Tab4_Widget = loader.load(&file,MSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    MSVPA_Tabs->addTab(MSVPA_Tab4_Widget, tr("4. Other Prey"));

    // Setup connections
    MSVPA_Tab4_AddPreyPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab4_AddPreyPB");
    MSVPA_Tab4_DelPreyPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab4_DelPreyPB");
    MSVPA_Tab4_PrevPB         = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab4_PrevPB");
    MSVPA_Tab4_NextPB         = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab4_NextPB");
    MSVPA_Tab4_LoadPB         = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab4_LoadPB");
    MSVPA_Tab4_SavePB         = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab4_SavePB");
    MSVPA_Tab4_PreySpeciesCMB = MSVPA_Tabs->findChild<QComboBox   *>("MSVPA_Tab4_PreySpeciesCMB");
    MSVPA_Tab4_MinLenLE       = MSVPA_Tabs->findChild<QLineEdit   *>("MSVPA_Tab4_MinLenLE");
    MSVPA_Tab4_MaxLenLE       = MSVPA_Tabs->findChild<QLineEdit   *>("MSVPA_Tab4_MaxLenLE");
    MSVPA_Tab4_AlphaLE        = MSVPA_Tabs->findChild<QLineEdit   *>("MSVPA_Tab4_AlphaLE");
    MSVPA_Tab4_BetaLE         = MSVPA_Tabs->findChild<QLineEdit   *>("MSVPA_Tab4_BetaLE");
    MSVPA_Tab4_BiomassTV      = MSVPA_Tabs->findChild<QTableView  *>("MSVPA_Tab4_BiomassTV");

    MSVPA_Tab4_PrevPB->setText("\u25C1--");
    MSVPA_Tab4_NextPB->setText("--\u25B7");

    connect(MSVPA_Tab4_PrevPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab4_PrevPB(bool)));
    connect(MSVPA_Tab4_NextPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab4_NextPB(bool)));
    connect(MSVPA_Tab4_SavePB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab4_SavePB(bool)));
    connect(MSVPA_Tab4_LoadPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab4_LoadPB(bool)));

    connect(MSVPA_Tab4_AddPreyPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab4_AddPreyPB(bool)));
    connect(MSVPA_Tab4_DelPreyPB,  SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab4_DelPreyPB(bool)));

    connect(MSVPA_Tab4_MinLenLE, SIGNAL(editingFinished()),
            this,                SLOT(callback_StripMinLenLE()));
    connect(MSVPA_Tab4_MaxLenLE, SIGNAL(editingFinished()),
            this,                SLOT(callback_StripMaxLenLE()));
    connect(MSVPA_Tab4_AlphaLE,  SIGNAL(editingFinished()),
            this,                SLOT(callback_StripAlphaLE()));
    connect(MSVPA_Tab4_BetaLE,   SIGNAL(editingFinished()),
            this,                SLOT(callback_StripBetaLE()));


    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::nmfMSVPATab4 Complete");
}


nmfMSVPATab4::~nmfMSVPATab4()
{
    std::cout << "Freeing Tab3 memory" << std::endl;

    smodel->clear();
}

void
nmfMSVPATab4::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir



void nmfMSVPATab4::clearWidgets()
{
    MSVPA_Tab4_MinLenLE->clear();
    MSVPA_Tab4_MaxLenLE->clear();
    MSVPA_Tab4_AlphaLE->clear();
    MSVPA_Tab4_BetaLE->clear();
    MSVPA_Tab4_PreySpeciesCMB->blockSignals(true);
    MSVPA_Tab4_PreySpeciesCMB->clear();
    MSVPA_Tab4_PreySpeciesCMB->blockSignals(false);
    if (smodel != NULL) {
        smodel->clear();
    }

} // end clearWidgets


void
nmfMSVPATab4::callback_StripMinLenLE()
{
    MSVPA_Tab4_MinLenLE->setText(MSVPA_Tab4_MinLenLE->text().trimmed());
}

void
nmfMSVPATab4::callback_StripMaxLenLE()
{
    MSVPA_Tab4_MaxLenLE->setText(MSVPA_Tab4_MaxLenLE->text().trimmed());
}

void
nmfMSVPATab4::callback_StripAlphaLE()
{
    MSVPA_Tab4_AlphaLE->setText(MSVPA_Tab4_AlphaLE->text().trimmed());
}

void
nmfMSVPATab4::callback_StripBetaLE()
{
    MSVPA_Tab4_BetaLE->setText(MSVPA_Tab4_BetaLE->text().trimmed());
}



void
nmfMSVPATab4::callback_MSVPA_Tab4_AddPreyPB(bool unused)
{
    bool ok;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    QString msg = "Enter new Prey name:";

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::callback_MSVPA_Tab4_AddPreyPB");

    QString NewPreyName = QInputDialog::getText(MSVPA_Tabs, "Add Prey",
                                             msg, QLineEdit::Normal,"", &ok);
    NewPreyName = NewPreyName.trimmed();
    if (ok && !NewPreyName.isEmpty() && (MSVPA_Tab4_PreySpeciesCMB->findText(NewPreyName) < 0))
    {
        MSVPA_Tab4_PreySpeciesCMB->blockSignals(true);
        MSVPA_Tab4_PreySpeciesCMB->addItem(NewPreyName);
        MSVPA_Tab4_PreySpeciesCMB->blockSignals(false);

        // First get the first and last years, number of seasons etc for the MSVPA
        fields   = {"FirstYear","LastYear","NSeasons"};
        queryStr = "SELECT FirstYear,LastYear,NSeasons FROM " +
                    nmfConstantsMSVPA::TableMSVPAlist +
                   " WHERE MSVPAname = '" + MSVPAName + "'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        int FirstYear = std::stoi(dataMap["FirstYear"][0]);
        int LastYear  = std::stoi(dataMap["LastYear"][0]);
        int NSeasons  = std::stoi(dataMap["NSeasons"][0]);
        int NYears    = LastYear - FirstYear + 1;

        smodel = new QStandardItemModel( NYears, NSeasons );
        MSVPA_Tab4_BiomassTV->setModel(smodel);
        makeHeaders(FirstYear, NYears, NSeasons);

        MSVPA_Tab4_MinLenLE->clear();
        MSVPA_Tab4_MaxLenLE->clear();
        MSVPA_Tab4_AlphaLE->clear();
        MSVPA_Tab4_BetaLE->clear();

        MSVPA_Tab4_PreySpeciesCMB->blockSignals(true);
        MSVPA_Tab4_PreySpeciesCMB->setCurrentText(NewPreyName);
        MSVPA_Tab4_PreySpeciesCMB->blockSignals(false);

        MSVPA_Tab4_SavePB->setEnabled(true);

        // Need to update MSVPAprefs.csv for MSVPA Tab 5. Prey Preferences
        updatePreyDatabaseTablesForOtherTabs(
                    QString::fromStdString(nmfConstantsMSVPA::TableMSVPAprefs), NewPreyName);

        // Need to update MSVPASpaceO.csv for MSVPA Tab 6. Spatial Overlap
        updatePreyDatabaseTablesForOtherTabs(
                    QString::fromStdString(nmfConstantsMSVPA::TableMSVPASpaceO),NewPreyName);

        msg = "Added Prey: " + NewPreyName;
        logger->logMsg(nmfConstants::Normal,msg.toStdString());
    } else {
        if (ok) {
            QMessageBox::information(MSVPA_Tabs,
                                     tr("Invalid Prey Name"),
                                     tr("\nPlease add a unique and valid Prey name."));
        }
        return;
    }

    QMessageBox::information(MSVPA_Tabs, "Save",
                             tr("\n"+msg.toLatin1()));

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::callback_MSVPA_Tab4_AddPreyPB Complete");

} // end callback_MSVPA_Tab4_AddPreyPB


void
nmfMSVPATab4::updatePreyDatabaseTablesForOtherTabs(QString tableName,
                                                   QString newPreyName)
{
    int retv;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString MSVPACSVFile = tableName + ".csv";
    QString filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QString fileNameWithPathOthPrey    = QDir(filePath).filePath(MSVPACSVFile);
    QString tmpFileNameWithPathOthPrey = QDir(filePath).filePath("."+MSVPACSVFile);
    int NumSpeciesToSkip=0;

    // Find number of species in auxiliary prey tables (i.e., MSVPAprefs, MSVPASpaceO);
    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND (Type=0 OR Type=1)";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumSpeciesToSkip = dataMap["SpeName"].size();
    fields   = {"OthPreyName"};
    queryStr = "SELECT OthPreyName FROM " + nmfConstantsMSVPA::TableMSVPAOthPrey +
               " WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumSpeciesToSkip += dataMap["OthPreyName"].size();

    // Read entire file and copy all lines that don't have the current MSVPAName into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    QFile fin(fileNameWithPathOthPrey);
    QFile fout(tmpFileNameWithPathOthPrey);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(4pa) Opening Input File",
                                 "\n"+fin.errorString()+": "+fileNameWithPathOthPrey);
        return;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(4pa) Opening Output File",
                                 "\n"+fout.errorString()+": "+tmpFileNameWithPathOthPrey);
        return;
    }

    QTextStream inStream(&fin);
    QTextStream outStream(&fout);
    QString line;
    QStringList qfields;
    int lastPreyIndex;

    bool isFirstLine = true;
    int num = 0;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        if (isFirstLine) {
            isFirstLine = false;
            outStream << line << "\n";
            continue;
        }
        ++num;
        if (num == NumSpeciesToSkip) {
            outStream << line << "\n";
            qfields = line.split(",");
            if (tableName.toStdString() == nmfConstantsMSVPA::TableMSVPAprefs) {
                lastPreyIndex = qfields[5].trimmed().toInt();
                outStream << qfields[0].trimmed() + "," +
                             qfields[1].trimmed() + ", " +
                             qfields[2].trimmed() + ", " +
                             qfields[3].trimmed() + ", " +
                             qfields[4].trimmed() + ", " +
                             QString::number(lastPreyIndex+1) + ", " +
                             newPreyName + ", " +
                             "0" + ", " +
                             "0" + "\n";
            } else if (tableName.toStdString() == nmfConstantsMSVPA::TableMSVPASpaceO) {
                lastPreyIndex = qfields[6].trimmed().toInt();
                outStream << qfields[0].trimmed() + "," +
                             qfields[1].trimmed() + ", " +
                             qfields[2].trimmed() + ", " +
                             qfields[3].trimmed() + ", " +
                             qfields[4].trimmed() + ", " +
                             qfields[5].trimmed() + ", " +
                             QString::number(lastPreyIndex+1) + ", " +
                             newPreyName + ", " +
                             "0" + "\n";
            } else {
                std::cout << "Warning: Invalid tableName: " << tableName.toStdString() << std::endl;
                return;
            }

            num = 0;
        } else {
            outStream << line << "\n";
        }

    } // end while

    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    retv = nmfUtilsQt::rename(tmpFileNameWithPathOthPrey,
                              fileNameWithPathOthPrey);

    if (retv < 0) {
        std::cout << "Error updatePreyDatabaseTablesInOtherTabs: Couldn't rename " <<
                     tmpFileNameWithPathOthPrey.toStdString() << " to " <<
                     fileNameWithPathOthPrey.toStdString() <<
                     ". Save aborted." << std::endl;
        return;
    }
std::cout << "mv file to: " << fileNameWithPathOthPrey.toStdString() << std::endl;

    UpdatePreyDatabaseTable(tableName,fileNameWithPathOthPrey);

} // end updatePreyDatabaseTablesForOtherTabs


void
nmfMSVPATab4::UpdatePreyDatabaseTable(QString tableName, QString filename)
{
    //
    // Save to mysql table
    //
    std::string errorMsg;
    std::string cmd,dataCmd="";
    QString line;
    QStringList qfields;
std::cout << "filename: " << filename.toStdString() << std::endl;

    // Clear the current table contents
    QString qcmd = "TRUNCATE TABLE " + tableName;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("MSVPA Save(4p): Clearing table error: ",
                             errorMsg+": "+tableName.toStdString());
        return;
    }
std::cout << "TRUNCATE TABLE " << tableName.toStdString() << std::endl;

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + tableName.toStdString();
    if (tableName.toStdString() == nmfConstantsMSVPA::TableMSVPAprefs)
        cmd += " (MSVPAName,SpeIndex,SpeType,SpeName,Age,PreyIndex,PreyName,PrefRank,PrefVal) VALUES ";
    else if (tableName.toStdString() == nmfConstantsMSVPA::TableMSVPASpaceO)
        cmd += " (MSVPAName,Season,SpeIndex,SpeType,SpeName,Age,PreyIndex,PreyName,SpOverlap) VALUES ";

    QFile fin(filename);

    // Read from csv file since it has all the MSVPANames in it
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error (4p) Opening Input File",
                                 "\n"+fin.errorString()+": " +filename);
        return;
    }
    QTextStream inStream(&fin);

    bool isFirstLine = true;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
std::cout << "line: " << line.toStdString() << std::endl;

        if (isFirstLine) { // skip first line since it's a header
            isFirstLine = false;
            continue;
        }
        qfields = line.split(",");
        if (tableName.toStdString() == nmfConstantsMSVPA::TableMSVPAprefs) {
            dataCmd += "(\"" + qfields[0].trimmed().toStdString() + "\",";
            dataCmd +=         qfields[1].trimmed().toStdString() + ",";
            dataCmd +=         qfields[2].trimmed().toStdString() + ",";
            dataCmd +=  "\"" + qfields[3].trimmed().toStdString() + "\",";
            dataCmd +=         qfields[4].trimmed().toStdString() + ",";
            dataCmd +=         qfields[5].trimmed().toStdString() + ",";
            dataCmd +=  "\"" + qfields[6].trimmed().toStdString() + "\",";
            dataCmd +=         qfields[7].trimmed().toStdString() + ",";
            dataCmd +=         qfields[8].trimmed().toStdString() + "), ";
        } else if (tableName.toStdString() == nmfConstantsMSVPA::TableMSVPASpaceO) {
            dataCmd += "(\"" + qfields[0].trimmed().toStdString() + "\",";
            dataCmd +=         qfields[1].trimmed().toStdString() + ",";
            dataCmd +=         qfields[2].trimmed().toStdString() + ",";
            dataCmd +=         qfields[3].trimmed().toStdString() + ",";
            dataCmd +=  "\"" + qfields[4].trimmed().toStdString() + "\",";
            dataCmd +=         qfields[5].trimmed().toStdString() + ",";
            dataCmd +=         qfields[6].trimmed().toStdString() + ",";
            dataCmd +=  "\"" + qfields[7].trimmed().toStdString() + "\",";
            dataCmd +=         qfields[8].trimmed().toStdString() + "), ";
        }
    } // end for all rows in csv file
    if (dataCmd.empty()) {
        std::cout << "Warning: No data found in MSVPAprefs. This may not be an error." << std::endl;
        return;
    }
    // Remove last two characters of dataCmd string since we have an extra ", "
    dataCmd = dataCmd.substr(0,dataCmd.size()-2);
    cmd += dataCmd;

    fin.close();
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("MSVPA Save(4p): Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        return;
    }


} // end UpdatePreyDatabaseTable


void
nmfMSVPATab4::callback_MSVPA_Tab4_DelPreyPB(bool unused)
{
    std::string msg;
    QMessageBox::StandardButton reply;
    QString filePath;
    QString fileNameWithPath;
    QString tmpFileNameWithPath;
    QString line;
    QStringList qfields;
    QString csvMSVPAName,csvPreyName;

    QString PreyToDelete  = MSVPA_Tab4_PreySpeciesCMB->currentText();
    if (PreyToDelete.isEmpty())
        return;
    //int PreyToDeleteIndex = MSVPA_Tab4_PreySpeciesCMB->currentIndex();
    //int NumPreyBeforeDelete = MSVPA_Tab4_PreySpeciesCMB->count();
    msg = "\nOK to delete Prey: " + PreyToDelete.toStdString() + " ?";
    reply = QMessageBox::question(MSVPA_Tabs,
                                  tr("Delete Prey"),
                                  tr(msg.c_str()),
                                  QMessageBox::Cancel|QMessageBox::Ok);
    if (reply == QMessageBox::Ok) {

        // Remove from combo box
        int indexToRemove = MSVPA_Tab4_PreySpeciesCMB->currentIndex();
        MSVPA_Tab4_PreySpeciesCMB->blockSignals(true);
        MSVPA_Tab4_PreySpeciesCMB->removeItem(indexToRemove);
        MSVPA_Tab4_PreySpeciesCMB->blockSignals(false);

        // Set current prey to previous one to the deleted one.
        if (indexToRemove > 1) {
            MSVPA_Tab4_PreySpeciesCMB->setCurrentIndex(indexToRemove-2);
            MSVPA_Tab4_PreySpeciesCMB->setCurrentIndex(indexToRemove-1);
        }else if (indexToRemove > 0) {
            MSVPA_Tab4_PreySpeciesCMB->setCurrentIndex(indexToRemove-1);
        }

        //
        // Remove from MSVPAOthPrey.csv and from MSVPAOthPreyAnn.csv
        //
        std::vector<QString> fileNameVec;
        std::vector<QString> tmpFileNameVec;
        std::map<QString,QString> tableFileMap;

        for (QString CSVFileName : {
             QString::fromStdString(nmfConstantsMSVPA::TableMSVPAOthPrey)+".csv",
             QString::fromStdString(nmfConstantsMSVPA::TableMSVPAOthPreyAnn)+".csv",
             QString::fromStdString(nmfConstantsMSVPA::TableMSVPAprefs)+".csv",
             QString::fromStdString(nmfConstantsMSVPA::TableMSVPASpaceO)+".csv"})
        {
            filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
            fileNameWithPath    = QDir(filePath).filePath(CSVFileName);
            tmpFileNameWithPath = QDir(filePath).filePath("." + CSVFileName);
            tableFileMap[CSVFileName] = fileNameWithPath;

            fileNameVec.push_back(fileNameWithPath);
            tmpFileNameVec.push_back(tmpFileNameWithPath);

            QFile fin(fileNameWithPath);
            QFile fout(tmpFileNameWithPath);

            // Open the files here....
            if (! fin.open(QIODevice::ReadOnly)) {
                QMessageBox::information(MSVPA_Tabs,
                                         "Error(3p) Opening Input File",
                                         "\n"+fin.errorString()+": "+fileNameWithPath);
                return;
            }
            if (! fout.open(QIODevice::WriteOnly)) {
                QMessageBox::information(MSVPA_Tabs,
                                         "Error(3p) Opening Output File",
                                         "\n"+fout.errorString()+": "+tmpFileNameWithPath);
                return;
            }

            QTextStream inStream(&fin);
            QTextStream outStream(&fout);

            while (! inStream.atEnd()) {
                line = inStream.readLine().trimmed();
                if (! line.isEmpty())
                {
                    qfields = line.split(",");
                    csvPreyName.clear();
                    csvMSVPAName = qfields[0].trimmed();
                    if ((CSVFileName == QString::fromStdString(nmfConstantsMSVPA::TableMSVPAOthPrey)+".csv") ||
                        (CSVFileName == QString::fromStdString(nmfConstantsMSVPA::TableMSVPAOthPreyAnn)+".csv"))
                    {
                        csvPreyName  = qfields[1].trimmed();
                    } else if (CSVFileName == QString::fromStdString(nmfConstantsMSVPA::TableMSVPAprefs)+".csv") {
                        csvPreyName  = qfields[6].trimmed();
                    } else if (CSVFileName == QString::fromStdString(nmfConstantsMSVPA::TableMSVPASpaceO)+".csv") {
                        csvPreyName  = qfields[7].trimmed();
                    }
                    if ((csvMSVPAName != QString::fromStdString(MSVPAName)) ||
                        (csvPreyName  != PreyToDelete))
                    {
                        outStream << line << "\n";
                    }
                }
            }
            fin.close();
            fout.close();

            nmfUtilsQt::rename(tmpFileNameWithPath,fileNameWithPath);
        } // end for

        updatePreyDatabaseTables(fileNameVec[0],
                                 tmpFileNameVec[0],
                                 fileNameVec[1],
                                 tmpFileNameVec[1]);

        msg = "Deleted Prey: " + PreyToDelete.toStdString();
        logger->logMsg(nmfConstants::Normal,msg);
        msg = "\n" + msg;
        QMessageBox::information(MSVPA_Tabs,
                                 tr("Delete OK"),
                                 tr(msg.c_str()),
                                 QMessageBox::Ok);

        // Update related tables on other tabs
        UpdatePreyDatabaseTable(QString::fromStdString(nmfConstantsMSVPA::TableMSVPAprefs),
                                tableFileMap[QString::fromStdString(nmfConstantsMSVPA::TableMSVPAprefs)+".csv"]);
        UpdatePreyDatabaseTable(QString::fromStdString(nmfConstantsMSVPA::TableMSVPASpaceO),
                                tableFileMap[QString::fromStdString(nmfConstantsMSVPA::TableMSVPASpaceO)+".csv"]);

    } // end if OK to delete

} // end callback_MSVPA_Tab4_DelPreyPB


bool
nmfMSVPATab4::loadMSVPAOthPrey(QString Prey)
{
    int csvFileRow = 0;
    QString line;
    QStringList fields;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(MSVPA_Tabs);
    QStringList NameFilters;
    QString MSVPAOthPreyCSVFile;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::loadMSVPAOthPrey");

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile(QString::fromStdString(nmfConstantsMSVPA::TableMSVPAOthPrey)+".csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load MSVPA Other Prey CSV File");
    if (fileDlg.exec()) {

        // Open the file here....
        MSVPAOthPreyCSVFile = fileDlg.selectedFiles()[0];
        QFile file(MSVPAOthPreyCSVFile);
        if(! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(MSVPA_Tabs, "File Read Error", file.errorString());
            return false;
        }
        QTextStream inStream(&file);

        QString csvMSVPAName, csvOthPreyName, csvMinSize, csvMaxSize, csvSizeAlpha, csvSizeBeta;
        // Store data to load in next step
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header
                fields = line.split(",");
                csvMSVPAName   = fields[0].trimmed();
                csvOthPreyName = fields[1].trimmed();
                csvMinSize     = fields[2].trimmed();
                csvMaxSize     = fields[3].trimmed();
                csvSizeAlpha   = fields[4].trimmed();
                csvSizeBeta    = fields[5].trimmed();
                if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
                   (csvOthPreyName == Prey))
                {
                    MSVPA_Tab4_MinLenLE->setText(csvMinSize);
                    MSVPA_Tab4_MaxLenLE->setText(csvMaxSize);
                    MSVPA_Tab4_AlphaLE->setText(csvSizeAlpha);
                    MSVPA_Tab4_BetaLE->setText(csvSizeBeta);
                }
            }
            ++csvFileRow;
        }
        file.close();

        MSVPA_Tab4_SavePB->setEnabled(true);
        MSVPA_Tab4_NextPB->setEnabled(false);
    } // end fileDlg

    MSVPA_Tab4_LoadPB->clearFocus();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::loadMSVPAOthPrey Complete");

    return true;

} // end loadMSVPAOthPrey


void
nmfMSVPATab4::getPreyMinMaxInfo(QString Prey,
                                QString &MinStr,
                                QString &MaxStr,
                                QString &AlphaStr,
                                QString &BetaStr)
{
    std::string msg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;

    fields    = {"OthPreyName","MinSize","MaxSize","SizeAlpha","SizeBeta"};
    queryStr  = "SELECT OthPreyName,MinSize,MaxSize,SizeAlpha,SizeBeta FROM " +
                 nmfConstantsMSVPA::TableMSVPAOthPrey +
                " WHERE MSVPAName = '" + MSVPAName +
                "' AND OthPreyName = '" + Prey.toStdString() + "'";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);

    if (dataMap["OthPreyName"].size() > 0) {
        MinStr   = QString::fromStdString(dataMap["MinSize"][0]);
        MaxStr   = QString::fromStdString(dataMap["MaxSize"][0]);
        AlphaStr = QString::fromStdString(dataMap["SizeAlpha"][0]);
        BetaStr  = QString::fromStdString(dataMap["SizeBeta"][0]);
    } else {
        //        msg = "\nCouldn't find data in MSVPAOthPrey for Prey: " + Prey.toStdString();
        msg = "\nPlease Enter at least 1 prey species.";
        QMessageBox::information(MSVPA_Tabs,
                                 tr("Missing Data"),
                                 tr(msg.c_str()),
                                 QMessageBox::Ok);
        return;
    }

} // end getPreyMinMaxInfo


void
nmfMSVPATab4::loadMSVPAOthPreyAnn(QString Prey)
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
    QString MSVPAOthPreyAnnCSVFile;
    std::vector<double> BiomassData;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::loadMSVPAOthPreyAnn");

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile(QString::fromStdString(nmfConstantsMSVPA::TableMSVPAOthPreyAnn)+".csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load MSVPA Other Prey Ann (Biomass) CSV File");
    if (fileDlg.exec()) {

        // Open the file here....
        MSVPAOthPreyAnnCSVFile = fileDlg.selectedFiles()[0];
        QFile file(MSVPAOthPreyAnnCSVFile);
        if(! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(MSVPA_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);

        BiomassData.clear();
        QString csvMSVPAName, csvOthPreyName, csvYear, csvSeason, csvBiomass;
        // Store data to load in next step
        QStringList fields2;
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header
                fields2 = line.split(",");
                csvMSVPAName   = fields2[0].trimmed();
                csvOthPreyName = fields2[1].trimmed();
                csvYear        = fields2[2].trimmed();
                csvSeason      = fields2[3].trimmed();
                csvBiomass     = fields2[4].trimmed();
                if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
                    (csvOthPreyName == Prey))
                {
                    BiomassData.push_back(csvBiomass.toDouble());
                }
            }
            ++csvFileRow;
        }
        file.close();

        fields   = {"NSeasons","FirstYear","LastYear"};
        queryStr = "SELECT NSeasons,FirstYear,LastYear FROM " +
                    nmfConstantsMSVPA::TableMSVPAlist +
                   " WHERE MSVPAname = '" + MSVPAName + "'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if ( dataMap["FirstYear"].empty()) {
            QMessageBox::information(MSVPA_Tabs,
                                     tr("Missing Data"),
                                     tr("\nPlease complete all for this MSVPA configuration in Tab 2."),
                                     QMessageBox::Ok);
            return;
        }
        int NSeasons  = std::stoi(dataMap["NSeasons"][0]);
        int FirstYear = std::stoi(dataMap["FirstYear"][0]);
        int LastYear  = std::stoi(dataMap["LastYear"][0]);
        int NYears    = LastYear-FirstYear+1;

        // Now load the data
        if (smodel != NULL) {
            smodel->clear();
        }
        smodel->setRowCount(NYears);
        smodel->setColumnCount(NSeasons);
        for (int i=0; i < NYears; ++i) {
            for (int j=0; j<NSeasons; ++j) {
                QStandardItem *item = new QStandardItem(QString::number(BiomassData[m++]));
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(i, j, item);
            }
        }

        makeHeaders(FirstYear, NYears, NSeasons);
        MSVPA_Tab4_BiomassTV->setModel(smodel);

        MSVPA_Tab4_SavePB->setEnabled(true);
        MSVPA_Tab4_NextPB->setEnabled(false);
    }

    MSVPA_Tab4_LoadPB->clearFocus();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::loadMSVPAOthPreyAnn Complete");

} // end loadMSVPAOthPreyAnn


void
nmfMSVPATab4::makeHeaders(int FirstYear, int NYears, int NSeasons)
{
    QStringList strList;

    // Make row headers
    strList.clear();
    for (int j=0; j<NYears; ++j) {
        strList << " " + QString::number(FirstYear+j) + " ";
    }
    smodel->setVerticalHeaderLabels(strList);

    // Make column headers
    strList.clear();
    for (int k=0; k<NSeasons; ++k) {
        strList << "Season " + QString::number(k+1);
    }

} // end makeHeaders


void
nmfMSVPATab4::callback_MSVPA_Tab4_LoadPB(bool unused)
{
    bool retv;

    QString Prey = MSVPA_Tab4_PreySpeciesCMB->currentText();
    retv = loadMSVPAOthPrey(Prey);
    if (retv) {
        loadMSVPAOthPreyAnn(Prey);
    }

    MarkAsClean();

} // end callback_MSVPA_Tab4_LoadPB


void
nmfMSVPATab4::getYearSeasonData(int &FirstYear,
                                int &LastYear,
                                int &NYears,
                                int &NSeasons)
{
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;

    // First get the first and last years, number of seasons etc for the MSVPA
    fields   = {"FirstYear","LastYear","NSeasons"};
    queryStr = "SELECT FirstYear,LastYear,NSeasons FROM " +
                nmfConstantsMSVPA::TableMSVPAlist +
               " WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["FirstYear"].size() == 0) {
        std::cout << "Error: No data found in MSVPAlist for MSVPA config: " << MSVPAName << std::endl;
        return;
    }
    FirstYear = std::stoi(dataMap["FirstYear"][0]);
    LastYear  = std::stoi(dataMap["LastYear"][0]);
    NSeasons  = std::stoi(dataMap["NSeasons"][0]);
    NYears    = LastYear - FirstYear + 1;

} // end getYearSeasonData


void
nmfMSVPATab4::callback_MSVPA_Tab4_SavePB(bool unused)
{
    bool saveOK = true;
    QString fileName;
    QString filePath;
    QString fileNameWithPathOthPrey;
    QString fileNameWithPathOthPreyAnn;
    QString fileNameWithPathlist;
    QString tmpFileNameWithPathOthPrey;
    QString tmpFileNameWithPathOthPreyAnn;
    QString tmpFileNameWithPathlist;
    QString qcmd;
    QString line;
    QString TableNameOthPrey;
    QString TableNameOthPreyAnn;
    QString TableNamelist;
    QString tableLine;
    QStringList qfields;

    bool ignoreMSVPAName = false;
    int retv;
    QString csvMSVPAName,csvPreyName;
    int FirstYear=0;
    int LastYear=0;
    int NSeasons=0;
    int NYears=0;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::callback_MSVPA_Tab4_SavePB");

    //
    // Save info to MSVPAOthPrey
    //
    if (! nmfUtilsQt::allCellsArePopulated(MSVPA_Tabs,MSVPA_Tab4_BiomassTV,
                                           nmfConstants::ShowError))
        return;

    // Get FirstYear,LastYear,NSeasons
    getYearSeasonData(FirstYear,LastYear,NYears,NSeasons);

    bool okMin,okMax,okAlpha,okBeta;
    double MinVal   = MSVPA_Tab4_MinLenLE->text().toDouble(&okMin);
    double MaxVal   = MSVPA_Tab4_MaxLenLE->text().toDouble(&okMax);
    double AlphaVal = MSVPA_Tab4_AlphaLE->text().toDouble(&okAlpha);
    double BetaVal  = MSVPA_Tab4_BetaLE->text().toDouble(&okBeta);
    if (! okMin || ! okMax || ! okAlpha || ! okBeta) {
        QMessageBox::information(MSVPA_Tabs,
                                 tr("Missing Data"),
                                 tr("\nPlease complete all Min,Max,Alpha,Beta fields."),
                                 QMessageBox::Ok);
        return;
    }

    TableNameOthPrey = QString::fromStdString(nmfConstantsMSVPA::TableMSVPAOthPrey);

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (MSVPAOthPreyCSVFile.isEmpty()) {
        MSVPAOthPreyCSVFile = TableNameOthPrey + ".csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPathOthPrey    = QDir(filePath).filePath(MSVPAOthPreyCSVFile);
        tmpFileNameWithPathOthPrey = QDir(filePath).filePath("."+MSVPAOthPreyCSVFile);
    } else {
        QFileInfo finfo(MSVPAOthPreyCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPathOthPrey    = MSVPAOthPreyCSVFile;
        tmpFileNameWithPathOthPrey = QDir(filePath).filePath("."+fileName);
    }

    // Read entire file and copy all lines that don't have the current MSVPAName into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    QFile fin(fileNameWithPathOthPrey);
    QFile fout(tmpFileNameWithPathOthPrey);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(3a) Opening Input File",
                                 "\n"+fin.errorString()+": "+fileNameWithPathOthPrey);
        return;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(3a) Opening Output File",
                                 "\n"+fout.errorString()+": "+tmpFileNameWithPathOthPrey);
        return;
    }

    QTextStream inStream(&fin);
    QTextStream outStream(&fout);

    QString NewPreyName = MSVPA_Tab4_PreySpeciesCMB->currentText();
    ignoreMSVPAName = false;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvMSVPAName = qfields[0].trimmed();
        csvPreyName  = qfields[1].trimmed();
        if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
            (csvPreyName == NewPreyName) &&
            (! csvMSVPAName.isEmpty())) { // dump qtableview data into output file
            // Skip over all other MSVPAName rows, since going through this once copies the qtableview data
            if (! ignoreMSVPAName) {
                tableLine = QString::fromStdString(MSVPAName) + ", " +
                            NewPreyName                       + ", " +
                            QString::number(MinVal)   + ", " + // MSVPA_Tab4_MinLenLE->text()       + ", " +
                            QString::number(MaxVal)   + ", " + // MSVPA_Tab4_MaxLenLE->text()       + ", " +
                            QString::number(AlphaVal) + ", " + // MSVPA_Tab4_AlphaLE->text()        + ", " +
                            QString::number(BetaVal);          // MSVPA_Tab4_BetaLE->text();
                outStream << tableLine << "\n";
                ignoreMSVPAName = true;
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }
    }
    if (! ignoreMSVPAName) { // means never found the MSVPAName
        tableLine = QString::fromStdString(MSVPAName)        + ", " +
                    MSVPA_Tab4_PreySpeciesCMB->currentText() + ", " +
                    QString::number(MinVal)   + ", " + // MSVPA_Tab4_MinLenLE->text()              + ", " +
                    QString::number(MaxVal)   + ", " + // MSVPA_Tab4_MaxLenLE->text()              + ", " +
                    QString::number(AlphaVal) + ", " + // MSVPA_Tab4_AlphaLE->text()               + ", " +
                    QString::number(BetaVal);          // MSVPA_Tab4_BetaLE->text();
        outStream << tableLine << "\n";
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    retv = nmfUtilsQt::rename(tmpFileNameWithPathOthPrey,
                                  fileNameWithPathOthPrey);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPathOthPrey.toStdString() << " to " <<
                     fileNameWithPathOthPrey.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return;
    }

    // ----------------------------------------------

    //
    // Save data to MSVPAOthPreyAnn
    //
    TableNameOthPreyAnn = QString::fromStdString(nmfConstantsMSVPA::TableMSVPAOthPreyAnn);

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (MSVPAOthPreyAnnCSVFile.isEmpty()) {
        MSVPAOthPreyAnnCSVFile = TableNameOthPreyAnn + ".csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPathOthPreyAnn    = QDir(filePath).filePath(MSVPAOthPreyAnnCSVFile);
        tmpFileNameWithPathOthPreyAnn = QDir(filePath).filePath("."+MSVPAOthPreyAnnCSVFile);
    } else {
        QFileInfo finfo(MSVPAOthPreyAnnCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPathOthPreyAnn    = MSVPAOthPreyAnnCSVFile;
        tmpFileNameWithPathOthPreyAnn = QDir(filePath).filePath("."+fileName);
    }

    // Read entire file and copy all lines that don't have the current MSVPAName into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    fin.setFileName(fileNameWithPathOthPreyAnn);
    fout.setFileName(tmpFileNameWithPathOthPreyAnn);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(3b) Opening Input File",
                                 "\n"+fin.errorString()+": "+fileNameWithPathOthPreyAnn);
        return;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(3b) Opening Output File",
                                 "\n"+fout.errorString()+": "+tmpFileNameWithPathOthPreyAnn);
        return;
    }

    inStream.setDevice(&fin);
    outStream.setDevice(&fout);

    ignoreMSVPAName = false;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvMSVPAName = qfields[0].trimmed();
        csvPreyName  = qfields[1].trimmed();
        if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
            (csvPreyName == NewPreyName) &&
            (! csvMSVPAName.isEmpty()))
        { // dump qtableview data into output file
            // Skip over all other MSVPAName rows, since going through this once copies the qtableview data
            if (! ignoreMSVPAName) {
                for (int i=FirstYear; i<=LastYear; ++i) {
                    for (int j=0; j<NSeasons; ++j) {
                        tableLine = QString::fromStdString(MSVPAName) + ", " +
                                    NewPreyName                       + ", " +
                                    QString::number(i)                + ", " +
                                    QString::number(j)                + ", " +
                                    smodel->index(i-FirstYear,j).data().toString();
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
        for (int i=FirstYear; i<=LastYear; ++i) {
            for (int j=0; j<NSeasons; ++j) {
                tableLine = QString::fromStdString(MSVPAName) + ", " +
                            NewPreyName                       + ", " +
                            QString::number(i)                + ", " +
                            QString::number(j)                + ", " +
                            smodel->index(i-FirstYear,j).data().toString();
                outStream << tableLine << "\n";
            }
        }
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    retv = nmfUtilsQt::rename(tmpFileNameWithPathOthPreyAnn,
                                  fileNameWithPathOthPreyAnn);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPathOthPreyAnn.toStdString() << " to " <<
                     fileNameWithPathOthPreyAnn.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return;
    }

    bool retv2 = updatePreyDatabaseTables(fileNameWithPathOthPrey,
                             tmpFileNameWithPathOthPrey,
                             fileNameWithPathOthPreyAnn,
                             tmpFileNameWithPathOthPreyAnn);



    if (retv2) {
        QMessageBox::information(MSVPA_Tabs,
                                 tr("Save OK"),
                                 tr("\nNew Prey data saved."),
                                 QMessageBox::Ok);
        MarkAsClean();

    } else {
        return;
    }

} // end callback_MSVPA_Tab4_SavePB



void
nmfMSVPATab4::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableMSVPAOthPrey);
    fields    = {"MSVPAName","OthPreyName","MinSize","MaxSize","SizeAlpha","SizeBeta"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableMSVPAOthPreyAnn);
    fields    = {"MSVPAName","OthPreyName","Year","Season","Biomass"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase



bool
nmfMSVPATab4::updatePreyDatabaseTables(QString fileNameWithPathOthPrey,
                                       QString tmpFileNameWithPathOthPrey,
                                       QString fileNameWithPathOthPreyAnn,
                                       QString tmpFileNameWithPathOthPreyAnn)
{
//    bool saveOK = true;
    bool FirstLine;
    int retv;
    std::string cmd;
    std::string errorMsg;
    QString qcmd;
    QString line;
    QString TableNameOthPrey    = QString::fromStdString(nmfConstantsMSVPA::TableMSVPAOthPrey);
    QString TableNameOthPreyAnn = QString::fromStdString(nmfConstantsMSVPA::TableMSVPAOthPreyAnn);
    QString TableNamelist       = QString::fromStdString(nmfConstantsMSVPA::TableMSVPAlist);
    QString tableLine;
    QString filePath;
    QString fileName;
    QString fileNameWithPathlist;
    QString tmpFileNameWithPathlist;
    QStringList qfields;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;

    //
    // Now update database tables. First for MSVPAOthPrey and then for MSVPAOthPreyAnn
    //
    //
    // Save MSVPAlist mysql table
    //

    // Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableNameOthPrey;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
//        saveOK = false;
        nmfUtils::printError("MSVPA Save(3a): Clearing table error: ",
                             errorMsg+": "+TableNameOthPrey.toStdString());
        return false;
    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableNameOthPrey.toStdString();
    cmd += " (MSVPAName,OthPreyName,MinSize,MaxSize,SizeAlpha,SizeBeta) VALUES ";

    QFile fin(fileNameWithPathOthPrey);
    QFile fout(tmpFileNameWithPathOthPrey);

    // Read from csv file since it has all the MSVPANames in it
    //fin.setFileName(fileNameWithPathOthPrey);
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(3b) Opening Input File",
                                 "\n"+fin.errorString()+": " +fileNameWithPathOthPrey);
        return false;
    }
    QTextStream inStream(&fin);
    QTextStream outStream(&fout);

    FirstLine = true;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields = line.split(",");
        cmd += "(\"" + qfields[0].trimmed().toStdString() + "\",";
        cmd +=  "\"" + qfields[1].trimmed().toStdString() + "\",";
        cmd +=         qfields[2].trimmed().toStdString() + ",";
        cmd +=         qfields[3].trimmed().toStdString() + ",";
        cmd +=         qfields[4].trimmed().toStdString() + ",";
        cmd +=         qfields[5].trimmed().toStdString() + "), ";

    } // end for all rows in csv file
    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);

    fin.close();
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("MSVPA Save(3c): Write table error: ", errorMsg);
        return false;
    }

    // ----------------------------------------------

    //
    // Save MSVPAOthPreyAnn mysql table
    //
    // 1. Check that the table exists, if not, create it.
    //databasePtr->checkForTableAndCreate(TableNameOthPreyAnn);

    // 2. Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableNameOthPreyAnn;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
//        saveOK = false;
        nmfUtils::printError("MSVPA Save(3d): Clearing table error: ",
                             errorMsg+": "+TableNameOthPreyAnn.toStdString());
    }

    // 3. Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableNameOthPreyAnn.toStdString();
    cmd += " (MSVPAName,OthPreyName,Year,Season,Biomass) VALUES ";

    // Read from csv file since it has all the MSVPANames in it
    fin.setFileName(fileNameWithPathOthPreyAnn);
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(3e) Opening Input File",
                                 "\n"+fin.errorString()+": " +fileNameWithPathOthPreyAnn);
        return false;
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
        cmd +=  "\"" + qfields[1].trimmed().toStdString() + "\",";
        cmd +=         qfields[2].trimmed().toStdString() + ",";
        cmd +=         qfields[3].trimmed().toStdString() + ",";
        cmd +=         qfields[4].trimmed().toStdString() + "), ";

    } // end for all rows in csv file
    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);

    fin.close();
    //std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("MSVPA Save(3f): Write table error: ", errorMsg);
        //std::cout << cmd << std::endl;
        return false;
    }

    // ----------------------------------------------


    // Need to also update MSVPAlist with the total number of Prey for the MSVPAName

    fields     = {"MSVPAName","OthPreyName"};
    queryStr   = "SELECT MSVPAName,OthPreyName FROM " +
                  nmfConstantsMSVPA::TableMSVPAOthPrey +
                 " WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap    = databasePtr->nmfQueryDatabase(queryStr, fields);
    int NOther = dataMap["OthPreyName"].size();

    // Now update the csv file and database table with this info

    //
    // Save data to MSVPAOthPreyAnn
    //
    TableNamelist = QString::fromStdString(nmfConstantsMSVPA::TableMSVPAlist);


    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (MSVPAlistCSVFile.isEmpty()) {
        MSVPAlistCSVFile = TableNamelist+ ".csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPathlist    = QDir(filePath).filePath(MSVPAlistCSVFile);
        tmpFileNameWithPathlist = QDir(filePath).filePath("."+MSVPAlistCSVFile);
    } else {
        QFileInfo finfo(MSVPAlistCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPathlist    = MSVPAlistCSVFile;
        tmpFileNameWithPathlist = QDir(filePath).filePath("."+fileName);
    }

    // Read entire file and copy all lines that don't have the current MSVPAName into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    fin.setFileName(fileNameWithPathlist);
    fout.setFileName(tmpFileNameWithPathlist);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(3g) Opening Input File",
                                 "\n"+fin.errorString()+": "+fileNameWithPathlist);
        return false;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(3g) Opening Output File",
                                 "\n"+fout.errorString()+": "+tmpFileNameWithPathlist);
        return false;
    }

    inStream.setDevice(&fin);
    outStream.setDevice(&fout);

    bool ignoreMSVPAName = false;
    QString csvMSVPAName;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvMSVPAName = qfields[0].trimmed();
        if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
            (! csvMSVPAName.isEmpty()))
        { // dump qtableview data into output file
            // Skip over all other MSVPAName rows, since going through this once copies the qtableview data
            if (! ignoreMSVPAName) {
                tableLine = QString::fromStdString(MSVPAName) + ", " +
                            qfields[1].trimmed()              + ", " +
                            qfields[2].trimmed()              + ", " +
                            QString::number(NOther)           + ", " +
                            qfields[4].trimmed()              + ", " +
                            qfields[5].trimmed()              + ", " +
                            qfields[6].trimmed()              + ", " +
                            qfields[7].trimmed()              + ", " +
                            qfields[8].trimmed()              + ", " +
                            qfields[9].trimmed()              + ", " +
                            qfields[10].trimmed()             + ", " +
                            qfields[11].trimmed();
                outStream << tableLine << "\n";
                ignoreMSVPAName = true;
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }
    }

    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    retv = nmfUtilsQt::rename(tmpFileNameWithPathlist,
                                  fileNameWithPathlist);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPathlist.toStdString() << " to " <<
                     fileNameWithPathlist.toStdString() <<
                     ". Save aborted." << std::endl;
//        saveOK = false;
        return false;
    }

    // -------------------------------------------


    //
    // Save MSVPAlist mysql table
    //

    // Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableNamelist;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
//        saveOK = false;
        nmfUtils::printError("MSVPA Save(3g): Clearing table error: ",
                             errorMsg+": "+TableNamelist.toStdString());
    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableNamelist.toStdString();
    cmd += " (MSVPAName,NSpe,NPreyOnly,NOther,NOtherPred,FirstYear,LastYear,NSeasons,";
    cmd += "AnnTemps,SeasSpaceO,GrowthModel,Complete) VALUES ";

    // Read from csv file since it has all the MSVPANames in it
    fin.setFileName(fileNameWithPathlist);
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs,
                                 "Error(3j) Opening Input File",
                                 "\n"+fin.errorString()+": " +fileNameWithPathlist);
        return false;
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

    fin.close();
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("MSVPA Save(3k): Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        return false;
    }

    // ----------------------------------------------

//    if (saveOK) {
//        QMessageBox::information(MSVPA_Tabs, "Save",
//                                 tr("\nMSVPA Other Prey data saved.\n"));
//    }

    MSVPAOthPreyCSVFile.clear();
    MSVPAOthPreyAnnCSVFile.clear();
    MSVPAlistCSVFile.clear();

    MSVPA_Tab4_NextPB->setEnabled(true);

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::callback_MSVPA_Tab4_SavePB Complete");

    return true;

} // end updatePreyDatabaseTables


/*
 *
 * This function updates the data structure.
 */
void
nmfMSVPATab4::callback_MSVPA_Tab4_ItemChanged(QStandardItem *item)
{
    item->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
    smodel->setItem(item->row(), item->column(), item);

    MarkAsDirty(nmfConstantsMSVPA::TableMSVPAOthPreyAnn);

    MSVPA_Tab4_SavePB->setEnabled(true);
    MSVPA_Tab4_NextPB->setEnabled(false);

} // end callback_MSVPA_Tab4_ItemChanged


void
nmfMSVPATab4::MarkAsDirty(std::string tableName)
{
    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfMSVPATab4::MarkAsClean()
{
    emit MarkMSVPAAsClean();
} // end MarkAsClean


bool
nmfMSVPATab4::loadWidgets(nmfDatabase *theDatabasePtr,
                               std::string theMSVPAName)
{

    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    int NSpecies;
//    int m;
    std::string species;
    int FirstYear=0;
    int LastYear=0;
    int NYears=0;
    int NSeasons=0;

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::loadWidgets");

    databasePtr = theDatabasePtr;
    MSVPAName   = theMSVPAName;

    clearWidgets();

    // Get FirstYear,LastYear,NSeasons
    getYearSeasonData(FirstYear,LastYear,NYears,NSeasons);

    smodel = new QStandardItemModel( NYears, NSeasons );

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab4_ItemChanged(QStandardItem *)));

    // If there's other species stored..then get 'em and put 'em in the species list
    fields   = {"OthPreyName","MinSize","MaxSize","SizeAlpha","SizeBeta"};
    queryStr = "SELECT OthPreyName,MinSize,MaxSize,SizeAlpha,SizeBeta FROM " +
                nmfConstantsMSVPA::TableMSVPAOthPrey +
               " WHERE MSVPAName = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NSpecies = dataMap["OthPreyName"].size();
//    m = 0;
    if (NSpecies > 0) {
        MSVPA_Tab4_PreySpeciesCMB->blockSignals(true);
        MSVPA_Tab4_PreySpeciesCMB->clear();
        MSVPA_Tab4_PreySpeciesCMB->blockSignals(false);
        // Load Min,Max,Alpha,Beta
        for (int i = 0; i < NSpecies; ++i) {
            species = dataMap["OthPreyName"][i];
            MSVPA_Tab4_PreySpeciesCMB->addItem(QString::fromStdString(species));
        } // end for i
    } else { // end if NSpecies
        logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::loadWidgets No Species found in MSVPAOthPrey");
    }
    connect(MSVPA_Tab4_MinLenLE, SIGNAL(textEdited(QString)),
            this,                SLOT(callback_theMinSettingWasChanged(QString)));
    connect(MSVPA_Tab4_MaxLenLE, SIGNAL(textEdited(QString)),
            this,                SLOT(callback_theMaxSettingWasChanged(QString)));
    connect(MSVPA_Tab4_AlphaLE,  SIGNAL(textEdited(QString)),
            this,                SLOT(callback_theAlphaSettingWasChanged(QString)));
    connect(MSVPA_Tab4_BetaLE,   SIGNAL(textEdited(QString)),
            this,                SLOT(callback_theBetaSettingWasChanged(QString)));

    connect(MSVPA_Tab4_PreySpeciesCMB, SIGNAL(currentIndexChanged(int)),
            this,                     SLOT(callback_MSVPA_Tab4_PreySpeciesCMB(int)));

    // Update widgets
    MSVPA_Tab4_NextPB->setEnabled(false);
    if (MSVPA_Tab4_PreySpeciesCMB->count() > 0) {
        callback_MSVPA_Tab4_PreySpeciesCMB(0);
        MSVPA_Tab4_NextPB->setEnabled(true);
    }

    MarkAsClean();

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab4::loadWidgets Complete");

    return true;
} // loadWidgets


void
nmfMSVPATab4::callback_theMinSettingWasChanged(QString value)
{
//    int species = MSVPA_Tab4_PreySpeciesCMB->currentIndex();
//    MinLen[species] = value.toDouble();
    MSVPA_Tab4_SavePB->setEnabled(true);
    MarkAsDirty(nmfConstantsMSVPA::TableMSVPAOthPrey);

}
void
nmfMSVPATab4::callback_theMaxSettingWasChanged(QString value)
{
//    int species = MSVPA_Tab4_PreySpeciesCMB->currentIndex();
//    MaxLen[species] = value.toDouble();
    MSVPA_Tab4_SavePB->setEnabled(true);
    MarkAsDirty(nmfConstantsMSVPA::TableMSVPAOthPrey);

}
void
nmfMSVPATab4::callback_theAlphaSettingWasChanged(QString value)
{
    //int species = MSVPA_Tab4_PreySpeciesCMB->currentIndex();
    //Alpha[species] = value.toDouble();
    MSVPA_Tab4_SavePB->setEnabled(true);
    MarkAsDirty(nmfConstantsMSVPA::TableMSVPAOthPrey);
}
void
nmfMSVPATab4::callback_theBetaSettingWasChanged(QString value)
{
    //int species = MSVPA_Tab4_PreySpeciesCMB->currentIndex();
    //Beta[species] = value.toDouble();
    MSVPA_Tab4_SavePB->setEnabled(true);
    MarkAsDirty(nmfConstantsMSVPA::TableMSVPAOthPrey);
}



void
nmfMSVPATab4::callback_MSVPA_Tab4_PreySpeciesCMB(int index)
{
    int m;
    std::string msg;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    int FirstYear=0;
    int LastYear=0;
    int NSeasons=0;
    int NYears=0;

    QString Prey = MSVPA_Tab4_PreySpeciesCMB->currentText();

//    if ((MinLen.size() == 0) || (index >= MinLen.size()))
//        return;

    if (smodel != NULL) {
        smodel->clear();
    }


    // Get FirstYear,LastYear,NSeasons
    getYearSeasonData(FirstYear,LastYear,NYears,NSeasons);



    // Load the 4 lineEdits from the database table data
    fields   = {"OthPreyName","MinSize","MaxSize","SizeAlpha","SizeBeta"};
    queryStr  = "SELECT OthPreyName,MinSize,MaxSize,SizeAlpha,SizeBeta FROM " +
                 nmfConstantsMSVPA::TableMSVPAOthPrey +
                " WHERE MSVPAName = '" + MSVPAName +
                "' AND OthPreyName = '" + Prey.toStdString() + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["OthPreyName"].size() > 0) {
        MSVPA_Tab4_MinLenLE->setText(QString::fromStdString(dataMap["MinSize"][0]));
        MSVPA_Tab4_MaxLenLE->setText(QString::fromStdString(dataMap["MaxSize"][0]));
        MSVPA_Tab4_AlphaLE->setText(QString::fromStdString(dataMap["SizeAlpha"][0]));
        MSVPA_Tab4_BetaLE->setText(QString::fromStdString(dataMap["SizeBeta"][0]));
    } else {
        //msg  = "\nCouldn't find data in MSVPAOthPrey for Prey: " + Prey.toStdString();
        msg  = "\nPlease Enter at least 1 Prey species.";
        QMessageBox::information(MSVPA_Tabs,
                                 tr("Data Error"),
                                 tr(msg.c_str()),
                                 QMessageBox::Ok);
    }

//    MSVPA_Tab4_MinLenLE->setText(QString::number(MinLen[index]));
//    MSVPA_Tab4_MaxLenLE->setText(QString::number(MaxLen[index]));
//    MSVPA_Tab4_AlphaLE->setText(QString::number(Alpha[index]));
//    MSVPA_Tab4_BetaLE->setText(QString::number(Beta[index]));

    disconnect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab4_ItemChanged(QStandardItem *)));

    // Load the tableview model from the database table data
    m = 0;
    fields   = {"MSVPAName","OthPreyName","Year","Season","Biomass"};
    queryStr = "SELECT MSVPAName,OthPreyName,Year,Season,Biomass FROM " + nmfConstantsMSVPA::TableMSVPAOthPreyAnn +
               " WHERE MSVPAName = '" + MSVPAName +
               "' AND OthPreyName = '" + Prey.toStdString() + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    int NumRecords = dataMap["MSVPAName"].size();
    if (NumRecords == NYears*NSeasons) {
        for (int j=0; j<NYears; ++j) {
            for (int k=0; k<NSeasons; ++k) {
                QStandardItem *item = new QStandardItem(QString::fromStdString(dataMap["Biomass"][m++]));
                item->setTextAlignment(Qt::AlignCenter);
                smodel->setItem(j, k, item);
            }
        }
    } else {
        msg  = "\nNumber of records in table MSVPAOthPreyAnn(" + std::to_string(NumRecords) + ") is not\n";
        msg += "correct for the given number of Years*Seasons (" + std::to_string(NYears) + "*" +
                std::to_string(NSeasons) + ").";
        QMessageBox::information(MSVPA_Tabs,
                                 tr("Data Error"),
                                 tr(msg.c_str()),
                                 QMessageBox::Ok);
        return;
    }
    makeHeaders(FirstYear, NYears, NSeasons);
    MSVPA_Tab4_BiomassTV->setModel(smodel);

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab4_ItemChanged(QStandardItem *)));

    MarkAsDirty(nmfConstantsMSVPA::TableMSVPAOthPrey);

} // end callback_MSVPA_Tab4_PreySpeciesCMB



void
nmfMSVPATab4::callback_MSVPA_Tab4_PrevPB(bool unused)
{    
    int prevPage = MSVPA_Tabs->currentIndex()-1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(prevPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(prevPage);
    emit UpdateNavigator("MSVPA",prevPage);
}

void
nmfMSVPATab4::callback_MSVPA_Tab4_NextPB(bool unused)
{
    int nextPage = MSVPA_Tabs->currentIndex()+1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(nextPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(nextPage);
    emit UpdateNavigator("MSVPA",nextPage);
}

