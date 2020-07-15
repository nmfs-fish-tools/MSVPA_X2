#include "nmfConstants.h"
#include "nmfUtils.h"
#include "nmfUtilsQt.h"

#include "nmfMSVPATab11.h"



nmfMSVPATab11::nmfMSVPATab11(QTabWidget  *tabs,
                             nmfLogger   *theLogger,
                             std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab11::nmfMSVPATab11");

    MSVPA_Tabs = tabs;
    MSVPAspeciesCSVFile.clear();
    ProjectDir = theProjectDir;
    Enabled = false;
    databasePtr = NULL;

    // Load ui as a widget from disk
    QFile file(":/forms/MSVPA/MSVPA_Tab11.ui");
    file.open(QFile::ReadOnly);
    MSVPA_Tab11_Widget = loader.load(&file,MSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    MSVPA_Tabs->addTab(MSVPA_Tab11_Widget, tr("11. SSVPA"));

    // Setup connections
    MSVPA_Tab11_LoadPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab11_LoadPB");
    MSVPA_Tab11_SavePB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab11_SavePB");
    MSVPA_Tab11_PrevPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab11_PrevPB");
    MSVPA_Tab11_NextPB      = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab11_NextPB");
    MSVPA_Tab11_SSVPADataTW = MSVPA_Tabs->findChild<QTableWidget *>("MSVPA_Tab11_SSVPADataTW");

    MSVPA_Tab11_PrevPB->setText("\u25C1--");
    MSVPA_Tab11_NextPB->setText("--\u25B7");


    connect(MSVPA_Tab11_PrevPB, SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab11_PrevPB(bool)));
    connect(MSVPA_Tab11_NextPB, SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab11_NextPB(bool)));
    connect(MSVPA_Tab11_LoadPB, SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab11_LoadPB(bool)));
    connect(MSVPA_Tab11_SavePB, SIGNAL(clicked(bool)),
            this,               SLOT(callback_MSVPA_Tab11_SavePB(bool)));

    MSVPA_Tab11_SavePB->setEnabled(true);
    MSVPA_Tab11_NextPB->setEnabled(false);

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab11::nmfMSVPATab11 Complete");

}


nmfMSVPATab11::~nmfMSVPATab11()
{

}

void
nmfMSVPATab11::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir


void nmfMSVPATab11::clearWidgets()
{

}


void
nmfMSVPATab11::callback_MSVPA_Tab11_SavePB(bool unused)
{
    bool saveOK = true;
    bool FirstLine = true;
    QString fileName;
    QString filePath;
    QString dir;
    QString fileNameWithPath;
    QString tmpFileNameWithPath;
    QString qcmd;
    QString line;
    QStringList qfields;
    std::string cmd,cmd1,cmd2,cmd3;
    std::string errorMsg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString TableName = "MSVPAspecies";

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab11::callback_MSVPA_Tab11_SavePB");

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

    QString tableLine;
    QLabel *clabel;
    QComboBox *cbox;
    QList<QString> SpeNames;
    QList<QString> SSVPANames;
    QList<QString> SSVPAIndices;
    int numRows = MSVPA_Tab11_SSVPADataTW->rowCount();
    for (int i=0; i<numRows; ++i) {
        clabel = qobject_cast<QLabel *>(MSVPA_Tab11_SSVPADataTW->cellWidget(i,0));
        SpeNames.push_back(clabel->text().trimmed());
        cbox = qobject_cast<QComboBox *>(MSVPA_Tab11_SSVPADataTW->cellWidget(i,1));
        SSVPANames.push_back(cbox->currentText().trimmed());
        SSVPAIndices.push_back(QString::number(cbox->currentIndex()+1));
    }

    // We're going to save a bit differently than in previous tabs, since
    // we're only looking at the first year's worth of data.  The original code uses
    // the first year's data for every year's data.
    QString csvMSVPAName,csvSpeName,csvSpeIndex,csvType,csvTimeRec,csvLenRec,csvWtRec;
    bool found;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvMSVPAName  = qfields[0].trimmed();
        csvSpeName    = qfields[1].trimmed();
        csvSpeIndex   = qfields[2].trimmed();
        csvType       = qfields[3].trimmed();
        csvTimeRec    = qfields[4].trimmed();
        csvLenRec     = qfields[5].trimmed();
        csvWtRec      = qfields[6].trimmed();
        if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
            (! csvMSVPAName.isEmpty())) {
            found = false;
            for (int i=0; i<SpeNames.size(); ++i) {
                if (csvSpeName == SpeNames[i]) {
                    found = true;
                    tableLine  = QString::fromStdString(MSVPAName) + ", " +
                            SpeNames[i]   + ", " +
                            csvSpeIndex   + ", " +
                            csvType       + ", " +
                            csvTimeRec    + ", " +
                            csvLenRec     + ", " +
                            csvWtRec      + ", " +
                            SSVPANames[i] + ", " +
                            SSVPAIndices[i];
                    outStream << tableLine << "\n";
                } // end if
            } // end for SpeName
            if (! found) {
                outStream << line << "\n";
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    int retv = nmfUtilsQt::rename(tmpFileNameWithPath,
                                  fileNameWithPath);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPath.toStdString() << " to " <<
                     fileNameWithPath.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return;
    }

    // --------------------------------------------

    //
    // Save MSVPAspecies mysql table
    //
    // 1. Check that the table exists, if not, create it.
    databasePtr->checkForTableAndCreate(TableName);

    // 2. Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableName;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (errorMsg != " ") {
        saveOK = false;
        nmfUtils::printError("MSVPA Save(10a): Clearing table error: ",
                             errorMsg+": "+TableName.toStdString());
    }


    QStringList FieldNames = {"MSVPAName", "SpeName", "SpeIndex",
                              "Type", "TimeRec", "LenRec",
                              "WtRec","SSVPAname", "SSVPAindex"};
    int NumFields = FieldNames.size();

    // 3. Write the contents from the CSV file into the MySQL table
    cmd1  = "INSERT INTO " + TableName.toStdString() + " (";

    // Read from csv file (and not from the GUI) since it has all the Species in it.
    QFile fin2(fileNameWithPath);
    if(! fin2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(MSVPA_Tabs, "\nError(10) Opening Input File", fin2.errorString());
        return;
    }
    QTextStream inStream2(&fin2);
    QString val;

    while (! inStream2.atEnd()) {
        cmd2.clear();
        cmd3 = ") VALUES ";

        line = inStream2.readLine();
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields   = line.split(",");
        cmd3 += "(\"" + qfields[0].trimmed().toStdString() + "\",";
        cmd3 += "\""  + qfields[1].trimmed().toStdString() + "\",";
        cmd2 += FieldNames[0].toStdString() + ",";
        cmd2 += FieldNames[1].toStdString() + ",";
        for (int i=2; i<NumFields; ++i) {
            val = qfields[i].trimmed();
            if (! val.isEmpty()) {
                cmd2 += FieldNames[i].toStdString() + ",";
                if (FieldNames[i] == "SSVPAname")
                    cmd3 += "\"" + val.toStdString() + "\",";
                else
                    cmd3 += val.toStdString() + ",";
            }
        }
        cmd2 = cmd2.substr(0,cmd2.size()-1);
        cmd3 = cmd3.substr(0,cmd3.size()-1) + ")";

        // Remove last two characters of cmd string since we have an extra ", "
        cmd = cmd1 + cmd2 + cmd3; //cmd3.substr(0,cmd3.size()-2);
        fin2.close();
//std::cout << cmd << std::endl;
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if (errorMsg != " ") {
            nmfUtils::printError("SSVPA Save(10): Write table error: ", errorMsg);
            std::cout << cmd << std::endl;
            return;
        }
    } // end for all rows in csv file

    if (saveOK) {
        QMessageBox::information(MSVPA_Tabs, "Save",
                                 tr("\nMSVPA Species SSVPA data saved.\n"));
    }

    MSVPAspeciesCSVFile.clear();

    //MSVPA_Tab11_NextPB->setEnabled(true);
    // Enable "Next" button only if all previous pages have been enabled
    emit CheckAndEnableMSVPAExecutePage();
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab11::callback_MSVPA_Tab11_SavePB Complete");

} // end callback_MSVPA_Tab11_SavePB



void
nmfMSVPATab11::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;
/*
 *  Don't dump these since they take up a lot of room, the format is a bit
 *  complicated for users to parse, and users should extract these data
 *  by clicking the camera icon over the desired output table.
 *
    TableName = "MSVPASeasBiomass";
    fields    = {"MSVPAName", "SpeName", "Season", "SpeType", "Year", "Age", "Biomass",
                 "TotalPredCons", "SeasM2", "AnnAbund", "SeasAbund", "SeasF",
                 "SeasM1", "SeasWt", "SeasSize", "AnnBiomass", "StomCont"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = "MSVPASuitPreyBiomass";
    fields    = {"MSVPAName", "PredName", "PredAge", "PreyName", "PreyAge", "Year",
                 "Season", "SuitPreyBiomass", "PropDiet", "BMConsumed", "PreyEDens"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);
*/
    TableName = "MSVPAEnergyDens";
    fields    = {"MSVPAName", "SpeName", "SpeType", "SpeIndex",
                 "Age", "Season", "EnergyDens", "AvgDietE"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase



void
nmfMSVPATab11::callback_MSVPA_Tab11_LoadPB(bool unused)
{
    QString line;
    QModelIndex index;
    QStringList fields;
    QString csvSpecies;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(MSVPA_Tabs);
    QStringList NameFilters;
    QString MSVPAspeciesCSVFile;

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab11::callback_MSVPA_Tab11_LoadPB");

    QLabel *clabel;
    QComboBox *cbox;
    QList<QString> SpeNames;
    QList<QString> SSVPANames;
    QList<QString> SSVPAIndices;
    int numRows = MSVPA_Tab11_SSVPADataTW->rowCount();
    for (int i=0; i<numRows; ++i) {
        clabel = qobject_cast<QLabel *>(MSVPA_Tab11_SSVPADataTW->cellWidget(i,0));
        SpeNames.push_back(clabel->text().trimmed());
        cbox = qobject_cast<QComboBox *>(MSVPA_Tab11_SSVPADataTW->cellWidget(i,1));
        SSVPANames.push_back(cbox->currentText().trimmed());
        SSVPAIndices.push_back(QString::number(cbox->currentIndex()+1));
    }
    QString csvMSVPAName,csvSpeName,csvSSVPAname,csvSSVPAindex;
    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile("MSVPAspecies.csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load MSVPA species CSV File");
    if (fileDlg.exec()) {

        // Open the file here....
        MSVPAspeciesCSVFile = fileDlg.selectedFiles()[0];
        QFile file(MSVPAspeciesCSVFile);
        if(! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(MSVPA_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);
        int csvFileRow = 0;
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header

                fields = line.split(",");
                csvMSVPAName  = fields[0];
                csvSpeName    = fields[1];
                csvSSVPAname  = fields[2];
                csvSSVPAindex = fields[3];

                for (int i=0; i<SpeNames.size(); ++i) {

                    if ((csvMSVPAName == QString::fromStdString(MSVPAName)) &&
                        (csvSpeName == SpeNames[i]))
                    {
                        cbox = qobject_cast<QComboBox *>(MSVPA_Tab11_SSVPADataTW->cellWidget(i,1));
                        cbox->setCurrentText(SpeNames[i]);
                        break;
                    } // end if
                } // end for
            } // end if
            ++csvFileRow;
        } // end while
        file.close();

        MSVPA_Tab11_SavePB->setEnabled(true);
        MSVPA_Tab11_NextPB->setEnabled(false);
    }

    MSVPA_Tab11_LoadPB->clearFocus();
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab11::callback_MSVPA_Tab11_LoadPB Complete");

} // end callback_MSVPA_Tab11_LoadPB

void
nmfMSVPATab11::MarkAsDirty(std::string tableName)
{
    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfMSVPATab11::MarkAsClean()
{
    emit MarkMSVPAAsClean();
} // end MarkAsClean


bool
nmfMSVPATab11::loadWidgets(nmfDatabase *theDatabasePtr,
                                std::string  theMSVPAName)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    int NumRecords=0;

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab11::loadWidgets");


    databasePtr = theDatabasePtr;
    MSVPAName   = theMSVPAName;

    SpeIndex.clear();
    SpeName.clear();

    fields   = {"SpeIndex","SpeName"};
    queryStr = "SELECT SpeIndex,SpeName FROM MSVPAspecies WHERE MSVPAName = '" + MSVPAName + "'" +
                    " AND (Type = 0 or Type = 1) ORDER By SpeIndex";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NSpecies = dataMap["SpeName"].size();

    for (int i = 0; i < NSpecies; ++i) {
        SpeIndex.push_back(std::stoi(dataMap["SpeIndex"][i]));
        SpeName.push_back(dataMap["SpeName"][i]);
    } // end for i


    //
    // Set up table
    //
    int numRows = SpeName.size();
    int numCols = 2; // Species name and SSVPA type
    MSVPA_Tab11_SSVPADataTW->setRowCount(numRows);
    MSVPA_Tab11_SSVPADataTW->setColumnCount(numCols);

    // Populate 1st column of table
    QLabel *label;
    for (int i=0; i<numRows; ++i) {
        label = new QLabel(QString::fromStdString(SpeName[i]));
        MSVPA_Tab11_SSVPADataTW->setCellWidget(i,0,label);
    }

    //
    // Populate 2nd column of table
    // Get SSVPA types and load into combo boxes
    bool allPopulated=true;
    QComboBox *cbox;
    for (int rowIndex=1; rowIndex<=numRows; ++rowIndex) {
        cbox       = new QComboBox();
        fields     = {"SSVPAName","SSVPAIndex","Type"};
        queryStr   = "SELECT SSVPAName,SSVPAIndex,Type FROM MSVPAspecies " ;
        queryStr  += " WHERE MSVPAName = '" + MSVPAName + "' ";
        queryStr  += " AND SpeIndex = " + std::to_string(rowIndex);
        queryStr  += " AND (Type = 1 OR Type = 0)";
        dataMap    = databasePtr->nmfQueryDatabase(queryStr, fields);
        NumRecords = dataMap["SSVPAName"].size();
        if (NumRecords > 0) {
            for (int i=0; i<NumRecords; ++i) {
                cbox->addItem(QString::fromStdString(dataMap["SSVPAName"][i]));
            }
        } else {
            allPopulated = false;
        }

        // Set cell widget as combo box
        MSVPA_Tab11_SSVPADataTW->setCellWidget(rowIndex-1,1,cbox);
    } // end for rowIndex

    // Hide row labels
    MSVPA_Tab11_SSVPADataTW->verticalHeader()->setVisible(false);

    // Set column headings
    MSVPA_Tab11_SSVPADataTW->setHorizontalHeaderItem(0,
        new QTableWidgetItem("Species"));
    MSVPA_Tab11_SSVPADataTW->setHorizontalHeaderItem(1,
        new QTableWidgetItem("Select SSVPA Configuration"));
    MSVPA_Tab11_SSVPADataTW->resizeColumnsToContents();

    MSVPA_Tab11_NextPB->setEnabled(allPopulated);

    emit CheckAndEnableMSVPAExecutePage();
    MarkAsClean();

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab11::loadWidgets Complete");

    return true;

} // end loadWidgets

void
nmfMSVPATab11::callback_MSVPA_Tab11_PrevPB(bool unused)
{
    int prevPage;

    QCheckBox *MSVPA_Tab2_PredatorGrowthCB   = MSVPA_Tabs->findChild<QCheckBox *>("MSVPA_Tab2_PredatorGrowthCB");
    if (MSVPA_Tab2_PredatorGrowthCB->isChecked()) {
        prevPage = MSVPA_Tabs->currentIndex()-1;
    } else {
        prevPage = MSVPA_Tabs->currentIndex()-3;
    }
    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(prevPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(prevPage);
    emit UpdateNavigator("MSVPA",prevPage);
}

void
nmfMSVPATab11::callback_MSVPA_Tab11_NextPB(bool unused)
{
    int nextPage = MSVPA_Tabs->currentIndex()+1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(nextPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(nextPage);
    emit UpdateNavigator("MSVPA",nextPage);
}

void
nmfMSVPATab11::enableNextButton(bool state)
{

    MSVPA_Tab11_NextPB->setEnabled(state);

} // end enableNextButton




