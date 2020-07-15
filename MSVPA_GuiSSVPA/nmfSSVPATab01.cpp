
#include "nmfSSVPATab01.h"
#include "nmfConstants.h"
#include "nmfUtils.h"
#include "nmfUtilsQt.h"

nmfSSVPATab1::nmfSSVPATab1(QTabWidget *tabs,
                           nmfLogger  *theLogger,
                           std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab1::nmfSSVPATab1");

    SSVPA_Tabs = tabs;
    databasePtr = NULL;
    SpeCatchCSVFile.clear();
    SpeciesIndex = 0;
    AllSpecies.clear();
    ProjectDir = theProjectDir;

    // Load ui as a widget from disk
    QFile file(":/forms/SSVPA/SSVPA_Tab01.ui");
    file.open(QFile::ReadOnly);
    SSVPA_Tab1_Widget = loader.load(&file,SSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    SSVPA_Tabs->addTab(SSVPA_Tab1_Widget, tr("1. Catch At Age"));


    SSVPA_Tab1_LoadPB    = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab1_LoadPB");
    SSVPA_Tab1_LoadAllPB = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab1_LoadAllPB");
    SSVPA_Tab1_SaveAllPB = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab1_SaveAllPB");
    SSVPA_Tab1_NextPB    = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab1_NextPB");
    SSVPA_Tab1_LabelLB   = SSVPA_Tabs->findChild<QLabel      *>("SSVPA_Tab1_LabelLB");
    SSVPACatchAtAgeTV    = SSVPA_Tabs->findChild<QTableView  *>("SSVPACatchAtAgeTV");

    SSVPA_Tab1_NextPB->setText("--\u25B7");

    connect(SSVPA_Tab1_NextPB,    SIGNAL(clicked(bool)),
            this,                 SLOT(callback_NextPB(bool)));
    connect(SSVPA_Tab1_LoadPB,    SIGNAL(clicked(bool)),
            this,                 SLOT(callback_LoadPB(bool)));
    connect(SSVPA_Tab1_LoadAllPB, SIGNAL(clicked(bool)),
            this,                 SLOT(callback_LoadAllPB(bool)));
    connect(SSVPA_Tab1_SaveAllPB, SIGNAL(clicked(bool)),
            this,                 SLOT(callback_SaveAllPB(bool)));

    SSVPA_Tab1_LoadPB->setEnabled(true);
    SSVPA_Tab1_LoadAllPB->setEnabled(true);
    SSVPA_Tab1_SaveAllPB->setEnabled(false);
    SSVPA_Tab1_NextPB->setEnabled(false);

    for (int i=0;i<nmfConstants::MaxNumberSpecies;++i)
        smodel[i] = NULL;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab1::nmfSSVPATab1 Complete");

} // end constructor


nmfSSVPATab1::~nmfSSVPATab1()
{

}

void
nmfSSVPATab1::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir


void
nmfSSVPATab1::callback_NextPB(bool unused)
{
    if (! nmfUtilsQt::allCellsArePopulated(SSVPA_Tabs,
                                           SSVPACatchAtAgeTV,
                                           nmfConstants::ShowError))
        return;

    QString Species = SSVPA_Tab1_LabelLB->text().split(":")[1].trimmed();

    int nextPage = SSVPA_Tabs->currentIndex()+1;
    SSVPA_Tabs->setCurrentIndex(nextPage);

    emit InitializePage("SSVPA",nextPage,true);
    emit ReselectSpecies(Species.toStdString(),true);

} // end callback_NextPB


void
nmfSSVPATab1::callback_TabChanged(int newTab)
{
//    QMessageBox::StandardButton reply;
    std::cout << "nmfSSVPATab1:: tab: " << newTab << std::endl;

//    reply = QMessageBox::question(SSVPA_Tabs, tr("Tab Clicked"),
//                                  tr("\nSave work from previous tab?\n"),
//                                  QMessageBox::No|QMessageBox::Yes);
//    if (reply == QMessageBox::Ok) {
//        return;
//    }
} // end callback_TabChanged



void
nmfSSVPATab1::callback_LoadAllPB(bool unused)
{

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab1::callback_LoadAllPB");

    loadAllSpeciesFromTableOrFile(databasePtr,0, "",AllSpecies,"FromFile");

    SSVPA_Tab1_SaveAllPB->setEnabled(true);

    emit ResetSpeciesList();
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab1::callback_LoadAllPB Complete");

} // end callback_LoadAllPB


void
nmfSSVPATab1::callback_LoadPB(bool unused)
{
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(SSVPA_Tabs);
    QStringList NameFilters;
    QString Species = SSVPA_Tab1_LabelLB->text().split(":")[1].trimmed();

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab1::callback_LoadPB");

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile("SpeCatch.csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load Catch at Age CSV File");
    if (fileDlg.exec()) {

        // Open the file here....
        SpeCatchCSVFile = fileDlg.selectedFiles()[0];
        QFile file(SpeCatchCSVFile);
        if (! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(SSVPA_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);

        loadSpeciesFromFile(inStream,Species);

        file.close();

        SSVPA_Tab1_SaveAllPB->setEnabled(true);
        SSVPA_Tab1_NextPB->setEnabled(false);
        SSVPA_Tab1_LoadPB->clearFocus();

    } // end if

    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab1::callback_LoadPB Complete");

} // end callback_LoadPB


void
nmfSSVPATab1::loadSpeciesFromFile(QTextStream &inStream,
                                  QString Species)
{
    int SpeciesIndex=0;
    std::string msg;
    std::string queryStr;
    QString line;
    QString csvSpecies;
    QStringList fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields2;
    QStandardItem *item;

    // Get SpeciesIndex of passed in Species name
    fields2  = {"SpeIndex","SpeName"};
    queryStr = "SELECT SpeIndex,SpeName FROM Species WHERE SpeName = '" + Species.toStdString() + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields2);
    if (dataMap["SpeIndex"].size() == 0)
        return;
    SpeciesIndex = std::stoi(dataMap["SpeIndex"][0]);

    int csvFileRow = 0;
    int numRows = smodel[SpeciesIndex]->rowCount();
    int numCols = smodel[SpeciesIndex]->columnCount();
    std::vector<QString> CatchData;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        if (csvFileRow > 0) { // skip row=0 as it's the header
            fields = line.split(",");
            csvSpecies = fields[0];
            if (csvSpecies == Species) {
                for (int i=2; i<fields.size(); ++i) {
                    // Omit blank fields (present if MinCatchAge is not 0)
                    if (! fields[i].trimmed().isEmpty()) {
                        CatchData.push_back(fields[i]);
                    }
                }
            }
        }
        ++csvFileRow;
    }

    // Now load the data
    int m = 0;
    int NumRecords = CatchData.size();
    if (NumRecords == numRows*numCols) {
        for (int i=0; i < numRows; ++i) {
            for (int j=0; j<numCols; ++j) {
                item = new QStandardItem(CatchData[m++]);
                item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                smodel[SpeciesIndex]->setItem(i, j, item);
            }
        }
    } else {
        msg = "\nNumber of pertinent records in SpeCatch CSV file (" + std::to_string(NumRecords) + ")\n";
        msg += "does not equal number of visible table cells (" + std::to_string(numCols);
        msg += "x" + std::to_string(numRows) + ")";
        QMessageBox::information(SSVPA_Tabs,
                                 tr("Database Load Problem"),
                                 tr(msg.c_str()),
                                 QMessageBox::Ok);
        return;
    }

} // end loadSpeciesFromFile

/*
 Can't use databasePtr->RestoreCSVFile(...) here since the format of this CSV
 file is a bit different than that of the database table.
 */
void
nmfSSVPATab1::restoreCSVFromDatabase()
{
    unsigned int NumRecords;
    std::string nameStr="",yearStr="",ageStr="",catchStr="",lastYearStr="",lastNameStr="";
    std::string totStr="";
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    QString firstLine;
    QString TableName = "SpeCatch";
    QString msg;

    QString path = QDir(QString::fromStdString(ProjectDir)).filePath("inputData");
    QString fileNameWithPath = QDir(path).filePath(TableName+".csv");
    QFile fin(fileNameWithPath);
    if (! fin.open(QIODevice::ReadOnly)) {
        return;
    }
    QTextStream inStream(&fin);
    firstLine = inStream.readLine();
    fin.close();

    QString tmpFileNameWithPath = QDir(path).filePath("."+TableName+".csv");
    QFile fout(tmpFileNameWithPath);
    if (! fout.open(QIODevice::WriteOnly)) {
        return;
    }
    QTextStream outStream(&fout);
    outStream << firstLine << "\n";

    fields     = {"SpeIndex", "SpeName","Year","Age","Catch"};
    queryStr   = "SELECT SpeIndex,SpeName,Year,Age,Catch FROM " + TableName.toStdString();
    dataMap    = databasePtr->nmfQueryDatabase(queryStr,fields);
    NumRecords = dataMap["SpeIndex"].size();
    if (NumRecords == 0) {
        msg = "\nNo data found in table: "+TableName;
        QMessageBox::warning(SSVPA_Tabs,
                             tr("Warning"),
                             tr(msg.toLatin1()),
                             QMessageBox::Ok);
        return;
    }

    lastYearStr = dataMap["Year"][0];
    lastNameStr = dataMap["SpeName"][0];
    for (unsigned int i=0; i<NumRecords; ++i) {
        nameStr  = dataMap["SpeName"][i];
        yearStr  = dataMap["Year"][i];
        ageStr   = dataMap["Age"][i];
        catchStr = dataMap["Catch"][i];
        if (yearStr == lastYearStr) {
            totStr += catchStr + ", ";
            if (i == 0) {
                for (int j=0; j<std::stoi(ageStr); ++j) {
                    totStr = ", " + totStr;
                }
            }
        } else {
             totStr = lastNameStr + ", " + lastYearStr + ", " + totStr;
             totStr = totStr.substr(0,totStr.size()-2);
             outStream << QString::fromStdString(totStr) << "\n";
             totStr = catchStr + ", ";
             for (int j=0; j<std::stoi(ageStr); ++j) {
                 totStr = ", " + totStr;
             }
        }
        lastYearStr = yearStr;
        lastNameStr = nameStr;
    } // end for
    totStr = lastNameStr + ", " + lastYearStr + ", " + totStr;
    totStr = totStr.substr(0,totStr.size()-2);
    outStream << QString::fromStdString(totStr) << "\n";
    fout.close();

    // Now, mv temp file to actual file.
    int retv = nmfUtilsQt::rename(tmpFileNameWithPath, fileNameWithPath);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPath.toStdString() << " to " <<
                     fileNameWithPath.toStdString() <<
                     ". Restore aborted." << std::endl;
        return;
    }

    std::cout << "Restored: " << TableName.toStdString()+".csv" << std::endl;

} // end restoreCSVFromDatabase


void
nmfSSVPATab1::callback_SaveAllPB(bool quiet)
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
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    int SpeciesIndex=0;
    QString TableName = "SpeCatch";

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab1::callback_SaveAllPB");

    if (! nmfUtilsQt::allCellsArePopulated(SSVPA_Tabs,
                                           SSVPACatchAtAgeTV,
                                           nmfConstants::ShowError))
        return;


    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (SpeCatchCSVFile.isEmpty()) {
        SpeCatchCSVFile = "SpeCatch.csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPath = QDir(filePath).filePath(SpeCatchCSVFile);
        tmpFileNameWithPath = QDir(filePath).filePath("."+SpeCatchCSVFile);
    } else {
        QFileInfo finfo(SpeCatchCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPath = SpeCatchCSVFile;
        tmpFileNameWithPath = QDir(filePath).filePath("."+fileName);
    }
    SpeCatchCSVFile.clear();

    // Read entire file and copy all lines that don't have the current Species into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.

    QFile fin(fileNameWithPath);
    QFile fout(tmpFileNameWithPath);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error Opening Input File", fin.errorString());
        return;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error Opening Output File", fout.errorString());
        return;
    }
    QTextStream inStream(&fin);
    QTextStream outStream(&fout);

    QString Species;
    QString tableLine;
    QString FirstAgeStr;
    QModelIndex index;

    int FirstAge;
    int numRows,numCols;
    bool done = false;
    bool isFirstLine = true;

    while ((! inStream.atEnd()) && (! done)) {
        line    = inStream.readLine();
        qfields = line.split(",");
        if (isFirstLine) {
            outStream << line + "\n";
            isFirstLine = false;
        } else {

            for (unsigned int n=0; n<AllSpecies.size(); ++n) {

                Species = AllSpecies[n];

                // Get SpeciesIndex of passed in Species name
                fields   = {"SpeIndex","SpeName"};
                queryStr = "SELECT SpeIndex,SpeName FROM Species WHERE SpeName = '" + Species.toStdString() + "'";
                dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
                if (dataMap["SpeIndex"].size() > 0) {
                    SpeciesIndex = std::stoi(dataMap["SpeIndex"][0]);
                    numRows = smodel[SpeciesIndex]->rowCount();
                    numCols = smodel[SpeciesIndex]->columnCount();
                    FirstAgeStr = smodel[SpeciesIndex]->headerData(0,Qt::Horizontal).toString();
                    FirstAge    = FirstAgeStr.split(" ")[1].toInt();
                    for (int i=0; i<numRows; ++i) {
                        tableLine = Species + ", " +
                                smodel[SpeciesIndex]->headerData(i,Qt::Vertical).toString() + ", ";
                        for (int k=0;k<FirstAge;++k) {
                            tableLine += ", ";
                        }
                        for (int j=0; j<numCols-1; ++j) {
                            index = smodel[SpeciesIndex]->index(i,j);
                            tableLine += index.data().toString() + ", ";
                        }
                        index = smodel[SpeciesIndex]->index(i,numCols-1);
                        tableLine += index.data().toString();
                        outStream << tableLine;
                        outStream << "\n";
                    }
                }
                done = true;

            } // end for
        }
    } // end while

    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    int retv = nmfUtilsQt::rename(tmpFileNameWithPath, fileNameWithPath);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPath.toStdString() << " to " <<
                     fileNameWithPath.toStdString() <<
                     ". Save aborted." << std::endl;
        return;
    }




    // Build a Species name and index map, you'll need this.
    std::map<std::string,int> SpeciesIndexMap;
    std::map<std::string,int> SpeciesMinCatchAgeMap;
    fields   = {"SpeIndex", "SpeName", "MinCatAge"};
    queryStr = "SELECT SpeIndex,SpeName,MinCatAge FROM Species";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
        SpeciesIndexMap[dataMap["SpeName"][i]]       = std::stoi(dataMap["SpeIndex"][i]);
        SpeciesMinCatchAgeMap[dataMap["SpeName"][i]] = std::stoi(dataMap["MinCatAge"][i]);
    }


    //
    // Save to mysql table
    //

    // Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableName;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (errorMsg != " ") {
        nmfUtils::printError("SSVPA Save(1): Clearing table error: ", errorMsg);
        return;
    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableName.toStdString();
    cmd += " (SpeIndex,SpeName,Year,Age,Catch) VALUES ";

    // Read from csv file since it has all the Species in it
    QFile fin2(fileNameWithPath);
    if (! fin2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(1) Opening Input File", fin.errorString());
        return;
    }
    QTextStream inStream2(&fin2);

    FirstLine = true;
    int NumAges=0;
    int CatchAge;
    int offset=2; // skip over first two fields: name and year to get to the catch values per age group
    std::string FileName,FileIndex,FileYear,FileAge,FileCatch; // SpeCatch table fields
    while (! inStream2.atEnd()) {
        line = inStream2.readLine();
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields   = line.split(",");

        NumAges   = qfields.size()-offset;
        FileName  = qfields[0].trimmed().toStdString();
        FileIndex = std::to_string(SpeciesIndexMap[FileName]);
        FileYear  = qfields[1].trimmed().toStdString();
        CatchAge  = SpeciesMinCatchAgeMap[FileName];
        for (int Age=0+offset; Age<NumAges+offset; ++Age) {
            if (! qfields[Age].trimmed().isEmpty()) {
                FileAge   = std::to_string(CatchAge++);
                FileCatch = qfields[Age].trimmed().toStdString();
                cmd += "(" + FileIndex + ",";
                cmd += "\"" + FileName + "\",";
                cmd += FileYear  + ",";
                cmd += FileAge   + ",";
                cmd += FileCatch + "), ";
            }
        } // end for Age...have completed all table entries for this one row of the csv file
    } // end for all rows in csv file

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    fin2.close();
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (errorMsg != " ") {
        nmfUtils::printError("SSVPA Save: Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        std::cout << "csv file: " << fileNameWithPath.toStdString() << std::endl;
        return;
    }

    SSVPA_Tab1_SaveAllPB->setEnabled(false);
    SSVPA_Tab1_NextPB->setEnabled(true);

    if (! quiet) {
        QMessageBox::information(SSVPA_Tabs, "Save",
                                 tr("\nCatch data saved.\n"));
    }

    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab1::callback_SaveAllPB Complete");

} // end callback_SaveAllPB


void
nmfSSVPATab1::callback_ItemChanged(QStandardItem *item)
{
    item->setTextAlignment(Qt::AlignRight|Qt::AlignCenter);
    smodel[SpeciesIndex]->setItem(item->row(), item->column(), item);

    MarkAsDirty("SpeCatch");

    SSVPA_Tab1_SaveAllPB->setEnabled(true);
    SSVPA_Tab1_NextPB->setEnabled(false);

} // end callback_ItemChanged

void
nmfSSVPATab1::MarkAsDirty(std::string tableName)
{

    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfSSVPATab1::MarkAsClean()
{

    emit MarkSSVPAAsClean();

} // end MarkAsClean


bool
nmfSSVPATab1::loadAllSpeciesFromTableOrFile(nmfDatabase *theDatabasePtr,
                                            int currSpeciesIndex,
                                            std::string currSpeciesName,
                                            std::vector<QString> theAllSpecies,
                                            std::string FromTableOrFile) // "FromTable" or "FromFile"
{
    QString Species;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    std::string SpeciesIndex;
    std::string fileName;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(SSVPA_Tabs);
    QStringList NameFilters;
    std::map<int,QString> nameMap;
//std::cout << "~~~~~~~~ 1 current db name: " << databasePtr->nmfGetCurrentDatabase() << std::endl;
    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab1::loadAllSpeciesFromTableOrFile");

    AllSpecies = theAllSpecies;

    // Query user for SpeCatch file name
    fileName = "";
    if (FromTableOrFile == "FromFile") {
        // Setup Load dialog
        fileDlg.setDirectory(path);
        fileDlg.selectFile("SpeCatch.csv");
        NameFilters << "*.csv" << "*.*";
        fileDlg.setNameFilters(NameFilters);
        fileDlg.setWindowTitle("Load Catch at Age CSV File");
        if (fileDlg.exec()) {
            fileName = fileDlg.selectedFiles()[0].toStdString();
        } else {
            return false;
        }
    }

    // test code - RSK
//std::cout << "~~~~~~~~ 2 current db name: " << databasePtr->nmfGetCurrentDatabase() << std::endl;

    for (int i=AllSpecies.size()-1; i>=0; --i) {

        Species = AllSpecies[i];

        fields   = {"SpeIndex"};
        queryStr = "SELECT SpeIndex FROM Species WHERE SpeName = '" + Species.toStdString() + "'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["SpeIndex"].size() != 0) {
            SpeciesIndex = dataMap["SpeIndex"][0];
            nameMap[std::stoi(SpeciesIndex)] = Species;
            loadWidgets(theDatabasePtr,
                        AllSpecies,
                        SpeciesIndex,
                        Species.toStdString(),
                        FromTableOrFile,
                        fileName);
        }
    }

    // Show the Species that's currently selected in the EntityList
    SSVPACatchAtAgeTV->setModel(smodel[currSpeciesIndex]);
    if (currSpeciesName.empty())
        currSpeciesName = nameMap.begin()->second.toStdString(); //AllSpecies[0].toStdString();
    else
        currSpeciesName = nameMap[currSpeciesIndex-1].toStdString();
    updateMainLabel(currSpeciesName);

    // Set state of pushbuttons
    SSVPA_Tab1_LoadPB->setEnabled(true);
    SSVPA_Tab1_SaveAllPB->setEnabled(false);
    if (SSVPACatchAtAgeTV->model() != NULL)
        SSVPA_Tab1_NextPB->setEnabled(nmfUtilsQt::allCellsArePopulated(SSVPA_Tabs,
                                                                   SSVPACatchAtAgeTV,
                                                                   nmfConstants::DontShowError));

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab1::loadAllSpeciesFromTableOrFile Complete");

    return true;

} // end loadAllSpeciesFromTableOrFile


bool
nmfSSVPATab1::loadSpecies(int theSpeciesIndex)
{
    double CatchUnits;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString catchUnitsStr = "(unknown units found)";
    std::string SpeciesName="";

    fields   = {"SpeIndex","SpeName","CatchUnits"};
    queryStr = "SELECT SpeIndex,SpeName,CatchUnits FROM Species WHERE SpeIndex="+ std::to_string(theSpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    if (dataMap["SpeName"].size() > 0) {
        CatchUnits = std::stod(dataMap["CatchUnits"][0]);
        if (CatchUnits == 0.1)
            catchUnitsStr = "Hundreds";
        else if (CatchUnits == 1.0)
            catchUnitsStr = "Thousands";
        else if (CatchUnits == 1000.0)
            catchUnitsStr = "Millions";
        SpeciesName = dataMap["SpeName"][0];
    }
    QString labelText = "Load Catch Data in " + catchUnitsStr +
            " of Fish for:  " + QString::fromStdString(SpeciesName);
    SSVPA_Tab1_LabelLB->setText(labelText);

    SpeciesIndex = theSpeciesIndex;

    SSVPACatchAtAgeTV->setModel(smodel[SpeciesIndex]);

    SSVPA_Tab1_NextPB->setEnabled(nmfUtilsQt::allCellsArePopulated(SSVPA_Tabs,
                                                                   SSVPACatchAtAgeTV,
                                                                   nmfConstants::DontShowError));

    return true;

} // end loadSpecies


bool
nmfSSVPATab1::loadWidgets(nmfDatabase *theDatabasePtr,
                          std::vector<QString> theAllSpecies,
                          std::string theSpeciesIndex,
                          std::string theSpecies,
                          std::string dataSource,
                          std::string fileName)
{
    int MinCatchAge;
    int MaxCatchAge;
    int NumAges;
    int FirstYear;
    int LastYear;
    int NumYears;
    int NumRecords;
    std::string msg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QStandardItem *item;
    QStringList qfields;
    std::vector<QString> CatchData;
    QString line;
    QString csvSpecies;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab1::loadWidgets: "+theSpecies);

    SpeciesIndex = std::stoi(theSpeciesIndex);
    Species      = theSpecies;
    databasePtr  = theDatabasePtr;
    AllSpecies   = theAllSpecies;

    // Get initial data from Species table
    fields   = {"SpeIndex", "SpeName","MinCatAge","MaxCatAge","FirstYear","LastYear"};
    queryStr = "SELECT SpeIndex,SpeName,MinCatAge,MaxCatAge,FirstYear,LastYear FROM Species WHERE SpeIndex=" + theSpeciesIndex;
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    MinCatchAge = std::stoi(dataMap["MinCatAge"][0]);
    MaxCatchAge = std::stoi(dataMap["MaxCatAge"][0]);
    NumAges     = MaxCatchAge - MinCatchAge + 1;
    FirstYear   = std::stoi(dataMap["FirstYear"][0]);
    LastYear    = std::stoi(dataMap["LastYear"][0]);
    NumYears    = LastYear - FirstYear + 1;

    // Create new model for Catch
    smodel[SpeciesIndex] = new QStandardItemModel(NumYears,NumAges);

    connect(smodel[SpeciesIndex], SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_ItemChanged(QStandardItem *)));


    int m = 0;
    int numRows = NumYears;
    int numCols = NumAges;

    // Set up columns and rows
    smodel[SpeciesIndex]->setRowCount(numRows);
    smodel[SpeciesIndex]->setColumnCount(numCols);
    makeHeaders(FirstYear,LastYear,MinCatchAge,NumAges);

    if (dataSource == "FromTable") {

        // If you find data....load it.
        fields     = {"SpeIndex", "SpeName","Year","Age","Catch"};
        queryStr   = "SELECT SpeIndex,SpeName,Year,Age,Catch FROM SpeCatch WHERE SpeName='" + Species + "'";
        dataMap    = databasePtr->nmfQueryDatabase(queryStr,fields);
        NumRecords = dataMap["SpeIndex"].size();

    } else if (dataSource == "FromFile") {

        // Open the file here....
        QString SpeCatchCSVFile = QString::fromStdString(fileName); //"SpeCatch.csv";
        QString filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        QString fileNameWithPath = QDir(filePath).filePath(SpeCatchCSVFile);
        QFile   file(fileNameWithPath);
        if (! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(SSVPA_Tabs, "nmfSSVPATab1::loadWidgets: File Read Error", file.errorString());
            return false;
        }
        QTextStream inStream(&file);

        int csvFileRow = 0;

        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header
                qfields = line.split(",");
                csvSpecies = qfields[0].trimmed();
                if (csvSpecies == QString::fromStdString(Species)) {
                    for (int i=2; i<qfields.size(); ++i) {
                        // Omit blank fields (present if MinCatchAge is not 0)
                        if (! qfields[i].trimmed().isEmpty()) {
                            CatchData.push_back(qfields[i].trimmed());
                        }
                    }
                }
            }
            ++csvFileRow;
        }
        file.close();

        NumRecords = CatchData.size();
    }
    if (NumRecords > 0) {
        if (NumRecords == numRows*numCols) {
            for (int i=0; i<numRows; ++i) {
                for (int j=0; j<numCols; ++j) {
                    if (dataSource == "FromTable")
                        item = new QStandardItem(QString::fromStdString(dataMap["Catch"][m]));
                    else if (dataSource == "FromFile")
                        item = new QStandardItem(CatchData[m]);
                    item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                    smodel[SpeciesIndex]->setItem(i,j,item);
                    ++m;
                }
            }
        } else {
            msg = "\nNumber of pertinent records in SpeCatch (" + std::to_string(NumRecords) + ")\n";
            msg += "does not equal number of visible table cells (" + std::to_string(NumYears);
            msg += "x" + std::to_string(NumAges) + ")";
            QMessageBox::information(SSVPA_Tabs,
                                     tr("Data Problem"),
                                     tr(msg.c_str()),
                                     QMessageBox::Ok);
            SSVPA_Tab1_NextPB->setEnabled(nmfUtilsQt::allCellsArePopulated(SSVPA_Tabs,
                                                                           SSVPACatchAtAgeTV,
                                                                           nmfConstants::DontShowError));
            return false;
        }
    } else {
        for (int i=0; i<numRows; ++i) {
            for (int j=0; j<numCols; ++j) {
                item = new QStandardItem("");
                item->setTextAlignment(Qt::AlignRight);
                smodel[SpeciesIndex]->setItem(i,j,item);
            }
        }
    }

    SSVPACatchAtAgeTV->setModel(smodel[SpeciesIndex]);

    SSVPA_Tab1_LoadPB->setEnabled(true);
    SSVPA_Tab1_SaveAllPB->setEnabled(false);

    QString SpeciesOnLabel = SSVPA_Tab1_LabelLB->text().split(":")[1].trimmed();
    if (theSpecies == SpeciesOnLabel.toStdString())
    {
        SSVPA_Tab1_NextPB->setEnabled(nmfUtilsQt::allCellsArePopulated(SSVPA_Tabs,
                                                                       SSVPACatchAtAgeTV,
                                                                       nmfConstants::DontShowError));
    }

    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab1::loadWidgets Complete");

    return true;

} // end loadWidgets

void
nmfSSVPATab1::updateMainLabel(std::string Species)
{
    double CatchUnits;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString catchUnitsStr = "(unknown units found)";

    // Update main label
    fields   = {"SpeName","CatchUnits"};
    queryStr = "SELECT SpeName,CatchUnits FROM Species WHERE SpeName='"+ Species + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    if (nmfUtils::checkForError(logger,"Species","SpeName",dataMap,queryStr))
        return; // false;
    if (dataMap["SpeName"].size() > 0) {
        CatchUnits = std::stod(dataMap["CatchUnits"][0]);
        if (CatchUnits == 0.1)
            catchUnitsStr = "Hundreds";
        else if (CatchUnits == 1.0)
            catchUnitsStr = "Thousands";
        else if (CatchUnits == 1000.0)
            catchUnitsStr = "Millions";
    }
    QString labelText = "Load Catch Data in " + catchUnitsStr + " of Fish for:  " + QString::fromStdString(Species);
    SSVPA_Tab1_LabelLB->setText(labelText);

} // end updateMainLabel




void
nmfSSVPATab1::makeHeaders(int FirstYear,
                          int LastYear,
                          int MinCatchAge,
                          int NumAges)
{
    QStringList strList;

    // Write column labels
    for (int i=0; i<NumAges; ++i) {
        strList << QString("Age "+QString::number(MinCatchAge++));
    }
    smodel[SpeciesIndex]->setHorizontalHeaderLabels(strList);

    // Write row labels
    strList.clear();
    for (int i=FirstYear; i<=LastYear; ++i) {
        strList << QString::number(i);
    }
    smodel[SpeciesIndex]->setVerticalHeaderLabels(strList);

} // end makeHeaders


void
nmfSSVPATab1::clearWidgets()
{
    for (int i=0; i<nmfConstants::MaxNumberSpecies; ++i) {
        if (smodel[i])
            smodel[i]->clear();
    }
}
