
#include "nmfSetupTab06.h"
#include "nmfConstants.h"
#include "nmfQtUtils.h"
#include "nmfUtils.h"



nmfSetupTab6::nmfSetupTab6(QTabWidget *tabs,
                           QTextEdit *theSetupOutputTE)
{
    Setup_Tabs = tabs;
    databasePtr = NULL;
    SetupOutputTE = theSetupOutputTE;
    ProjectDir.clear();
    ProjectDatabase.clear();
    returnToSection.clear();
    returnToTab = 0;
    returnMSVPAName.clear();
    returnForecastName.clear();

    readSettings();

    Setup_Tab6_TableListCMB           = Setup_Tabs->findChild<QComboBox *>(   "Setup_Tab6_TableListCMB");
    Setup_Tab6_CSVFileLE              = Setup_Tabs->findChild<QLineEdit *>(   "Setup_Tab6_CSVFileLE");
    Setup_Tab6_TableDescTW            = Setup_Tabs->findChild<QTableWidget *>("Setup_Tab6_TableDescTW");
    Setup_Tab6_CSVFileTW              = Setup_Tabs->findChild<QTableWidget *>("Setup_Tab6_CSVFileTW");
    Setup_Tab6_RefreshPB              = Setup_Tabs->findChild<QPushButton *>( "Setup_Tab6_RefreshPB");
    Setup_Tab6_BackPB                 = Setup_Tabs->findChild<QPushButton *>( "Setup_Tab6_BackPB");
    Setup_Tab6_RunValidityCheckPB     = Setup_Tabs->findChild<QPushButton *>( "Setup_Tab6_RunValidityCheckPB");
    Setup_Tab6_SaveToTablePB          = Setup_Tabs->findChild<QPushButton *>( "Setup_Tab6_SaveToTablePB");
    Setup_Tab6_RunAllValidityChecksPB = Setup_Tabs->findChild<QPushButton *>( "Setup_Tab6_RunAllValidityChecksPB");
    Setup_Tab6_TableToCSVPB           = Setup_Tabs->findChild<QPushButton *>( "Setup_Tab6_TableToCSVPB");

    Setup_Tab6_CSVFileLE->clear();


    // RSK - just temporarily
    Setup_Tab6_RunValidityCheckPB->setEnabled(false);
    Setup_Tab6_RunAllValidityChecksPB->setEnabled(false);



/*
    Forecast_Tab1_NextPB->setText("--\u25B7");
*/
    connect(Setup_Tab6_TableListCMB, SIGNAL(currentTextChanged(QString)),
            this,                    SLOT(callback_Setup_Tab6_TableChanged(QString)));
    connect(Setup_Tab6_RefreshPB,    SIGNAL(pressed()),
            this,                    SLOT(callback_Setup_Tab6_Refresh()));

    connect(Setup_Tab6_RunValidityCheckPB, SIGNAL(pressed()),
            this,                          SLOT(callback_Setup_Tab6_RunValidityCheck()));
    connect(Setup_Tab6_SaveToTablePB, SIGNAL(pressed()),
            this,                     SLOT(callback_Setup_Tab6_SaveToTable()));
    connect(Setup_Tab6_RunAllValidityChecksPB, SIGNAL(pressed()),
            this,                              SLOT(callback_Setup_Tab6_RunAllValidityChecks()));
    connect(Setup_Tab6_BackPB, SIGNAL(pressed()),
            this,              SLOT(callback_Setup_Tab6_Back()));
    connect(Setup_Tab6_TableToCSVPB, SIGNAL(pressed()),
            this,              SLOT(callback_Setup_Tab6_TableToCSVPB()));

    Setup_Tab6_TableToCSVPB->hide(); // just until I figure out best way to export table as csv file


    nmfUtils::readTableNames(TableNames);
    nmfUtils::readTableDescriptions(
                nmfConstants::TableDescForecastFile,
                TableDescriptions);
}


nmfSetupTab6::~nmfSetupTab6()
{
}

void
nmfSetupTab6::setReturnData(std::string section, int tab,
                            std::string MSVPAName, std::string ForecastName)
{
    returnToSection = section;
    returnToTab = tab;
    returnMSVPAName = MSVPAName;
    returnForecastName = ForecastName;
}


void
nmfSetupTab6::callback_Setup_Tab6_TableToCSVPB()
{

  std::string table   = Setup_Tab6_TableListCMB->currentText().toStdString();
  std::string csvFile = Setup_Tab6_CSVFileLE->text().toStdString();

  emit ExportTableToCSVFile(table,csvFile);

} // end callback_Setup_Tab6_TableToCSVPB

void
nmfSetupTab6::callback_Setup_Tab6_Back()
{
std::cout << "SetupTab6: Back" << std::endl;
    emit SelectTab(returnToSection,returnToTab,
                   returnMSVPAName,returnForecastName);
}

void
nmfSetupTab6::callback_Setup_Tab6_RunValidityCheck()
{

}

void
nmfSetupTab6::callback_Setup_Tab6_RunAllValidityChecks()
{

}

/*
 * Creates a database table from the loaded CSV file
 */
void
nmfSetupTab6::callback_Setup_Tab6_SaveToTable()
{
    std::vector<std::vector<std::string> > descData;
    std::string line;
    int col = -1;
    std::vector<std::string> parts;
    QString errorMsg;
    std::string errorMsgStr;
    QString msg;
    QString createCmd;
    QString tableName = Setup_Tab6_TableListCMB->currentText();
    QString databaseName = ProjectDatabase;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;

    // Re-read data. Code doesn't like making a class variable data structure that holds the data from
    // the previous read.
    descData.clear();
    std::vector<std::string> aRow;
    QString dscFilename = tableName+".dsc";
    QString dscFile = QDir(QString::fromStdString(std::string(nmfConstants::ProgramDataDir))).filePath(dscFilename);
    std::ifstream fptr(dscFile.toLatin1());
    if (fptr) {
        while (std::getline(fptr,line))
        {
            boost::algorithm::trim(line); // trims any spaces
            if (line[0] != '#') {
                ++col;
                boost::split(parts,line,boost::is_any_of("\t "),boost::token_compress_on);
                if (parts.size() == 5) {
                    aRow.clear();
                    for (int row=0; row<5; ++row) {
                        aRow.push_back(parts[row]);
                    }
                    descData.push_back(aRow);
                } else {
                    QMessageBox::critical(0,tr("Error"),
                                          tr(errorMsg.toLatin1()),QMessageBox::Ok);
                    break;
                }
            }
        }
        fptr.close();
    } else {
        msg = "\nCouldn't open file: " + dscFile;
        //qDebug() << msg;
        QMessageBox::warning(0,tr("File Open Error"),tr(msg.toLatin1()),QMessageBox::Ok);
    }

    // 0,0 MSVPAName      1,0 ForeName
    // 0,1 varchar(255)
    createCmd = "CREATE TABLE " + databaseName + "." + tableName + " (";
    int numRows = descData[0].size();
    int numCols = descData.size();
    std::string keyStr = "";
    for (int col=0; col<numCols; ++col) {
        createCmd +=   QString::fromStdString(descData[col][0]);
        createCmd +=   " " + QString::fromStdString(descData[col][1]);
        if (descData[col][2] == "NO") {
            createCmd += " NOT NULL";
        }
        if (descData[col][3] == "PRI") {
            if (keyStr != "") {
                keyStr += ",";
            }
            keyStr += descData[col][0];
        }
        if (descData[col][4] != "NULL") {
            createCmd += " DEFAULT " + QString::fromStdString(descData[col][4]);
        }
        createCmd += ", ";
    }
    createCmd += " PRIMARY KEY (" + QString::fromStdString(keyStr) + ") ) ENGINE=InnoDB; ";

    //
    // Check if table exists, if it doesn't, add it.
    //
    fields = { "count(*)" };
    queryStr = "SELECT count(*) FROM information_schema.TABLES WHERE (TABLE_SCHEMA='" + databaseName.toStdString() +
            "') AND (TABLE_NAME = '" + tableName.toStdString() + "'); ";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    int tableExists = std::stoi(dataMap["count(*)"][0]);
    if (! tableExists) {
        errorMsgStr = databasePtr->nmfUpdateDatabase(createCmd.toStdString());
        if (errorMsgStr != " ") {
            std::cout << "cmd: " << createCmd.toStdString() << std::endl;
            nmfUtils::printError("Function:  callback_Setup_Tab6_SaveToTable ", errorMsgStr);
        }
    } else {
        std::cout << "Info: Table " + tableName.toStdString() + " exists. Updating..." << std::endl;
    }

    //
    // Write out table data to csv file in case the user has changed any data.
    //
    std::string replaceCmd;
    QString textData = "";
    int numFields;
    std::string value;
    QString csvFilename = tableName+".csv";
    QString path = QDir(ProjectDir).filePath(QString::fromStdString(nmfConstants::InputDataDir));
    QString csvFile = QDir(path).filePath(csvFilename);
    numRows = Setup_Tab6_CSVFileTW->rowCount();
    numCols = Setup_Tab6_CSVFileTW->columnCount();
    for (int j = 0; j < numCols; j++) {
        if (j > 0) {
            textData += ",";
        }
        textData += Setup_Tab6_CSVFileTW->horizontalHeaderItem(j)->text();
    }
    textData += "\n";
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            if (j > 0) {
                textData += ",";
            }
            textData += Setup_Tab6_CSVFileTW->item(i,j)->text().toStdString().c_str();
        }
        textData += "\n";
    }
    QFile csvFileHandler(csvFile);
    if (csvFileHandler.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QTextStream out(&csvFileHandler);
        out << textData;
        csvFileHandler.close();
    }


    //
    // Read in data from csv file and load into MySQL REPLACE statement
    //

    // Read data from csv file
    QFile csvTableFile(csvFile);
    QString content;
    QStringList lines;
    std::vector<std::string> dataFields;
    lines.clear();
    QTableWidgetItem *item;
    if (! csvTableFile.open(QFile::ReadOnly)) {
        msg = "\nCouldn't open file: " + csvFile;
        QMessageBox::warning(0,tr("File Open Error"),tr(msg.toLatin1()),QMessageBox::Ok);
        return;
    }
    content = csvTableFile.readAll();
    lines   = content.split("\n");

    for (int i=1; i<lines.size(); ++i) {

        line = lines[i].toStdString();
        if (line.size() == 0)
            continue;

        boost::split(dataFields,line,boost::is_any_of(","));

        // Add/update data to table
        replaceCmd = "REPLACE INTO " + databaseName.toStdString() + "." + tableName.toStdString() + " (";
        for (int col=0; col<numCols; ++col) {
            if (col > 0) {
                replaceCmd += ",";
            }
            replaceCmd += descData[col][0];
        }
        replaceCmd += ") values (";

        for (int j=0; j<numCols; ++j) {
            value = dataFields[j];
            boost::algorithm::trim(value); // trims any spaces
            if (j > 0) {
                replaceCmd += ",";
            }
            if (value == "")
                replaceCmd += "NULL";
            else {if (descData[j][1].find("varchar") != std::string::npos)
                    replaceCmd += "'" + value + "'";
                else {
                    replaceCmd += value;
                }
            }
        }

        replaceCmd += "); ";
        errorMsgStr = databasePtr->nmfUpdateDatabase(replaceCmd);
        if (errorMsgStr != " ") {
            nmfUtils::printError("Function:  callback_Setup_Tab6_SaveToTable-replaceCmd ", errorMsgStr);
            qDebug() << QString::fromStdString(replaceCmd);
        }

    }

} // end callback_Setup_Tab6_SaveToTable


void
nmfSetupTab6::callback_Setup_Tab6_Refresh()
{

    callback_Setup_Tab6_TableChanged(Setup_Tab6_TableListCMB->currentText());

} // end callback_Setup_Tab6_Refresh



int
nmfSetupTab6::numColumnsTable(QString table)
{
    return TableNames[2][table.toStdString()].size();
}


void
nmfSetupTab6::callback_Setup_Tab6_TableChanged(QString table)
{
    std::string line;
    std::vector<std::string> parts;
    QString msg;
    int col = -1;
    std::string newType;
    QStringList rowHeaders;
    QStringList colHeaders;
    int NumRowsDescTable = 2; // was 5, but really don't need the other 3 (Null,Key, and Default)

    if (table.isEmpty())
        return;
    rowHeaders << "Field" << "Type" << "Null" << "Key" << "Default";


    // Populate CSV File line edit
    QString csvFilename = table+".csv";
    QString path    = QDir(ProjectDir).filePath(QString::fromStdString(nmfConstants::InputDataDir));
    QString csvFile = QDir(path).filePath(csvFilename);
    QString dscFilename = table+".dsc";
    QString dscFile = QDir(QString::fromStdString(std::string(nmfConstants::ProgramDataDir))).filePath(dscFilename);
    QString errorMsg = "Error in: " + dscFile;

    Setup_Tab6_CSVFileLE->setText(csvFile);

    //
    // Load description table
    //
    Setup_Tab6_TableDescTW->clear();
    Setup_Tab6_TableDescTW->setRowCount(NumRowsDescTable);
    Setup_Tab6_TableDescTW->setColumnCount(numColumnsTable(table));

    std::ifstream fptr(dscFile.toLatin1());
    if (fptr) {
        while (std::getline(fptr,line))
        {

            boost::algorithm::trim(line); // trims any spaces
            if (line[0] != '#') {
                ++col;
                boost::split(parts,line,boost::is_any_of("\t "),boost::token_compress_on);

                if (parts.size() == 5) {
                    //aRow.clear();
                    for (int row=0; row<NumRowsDescTable; ++row) {
                        if (row == 1) {
                            newType = moreIntuitiveType(parts[1]);
                            Setup_Tab6_TableDescTW->setItem(row,col,
                                new QTableWidgetItem(QString::fromStdString(newType)));
                        } else {
                            Setup_Tab6_TableDescTW->setItem(row,col,
                                new QTableWidgetItem(QString::fromStdString(parts[row])));
                        }
                        //aRow.push_back(parts[row]);
                    }
                    //descData.push_back(aRow);
                } else {
                    QMessageBox::critical(0,tr("Error"),
                        tr(errorMsg.toLatin1()),QMessageBox::Ok);
                    break;
                }
            }
        }
        fptr.close();
    } else {
        msg = "\nCouldn't open file: " + dscFile;
        QMessageBox::warning(0,tr("File Open Error"),tr(msg.toLatin1()),QMessageBox::Ok);
    }
    Setup_Tab6_TableDescTW->horizontalHeader()->hide();
    Setup_Tab6_TableDescTW->setVerticalHeaderLabels(rowHeaders);

    //
    // Load csv file into qtable
    //
    int numLines;
    Setup_Tab6_CSVFileTW->clear();
    QFile csvTableFile(csvFile);
    QString content;
    std::vector<std::string> lines;
    std::vector<std::string> fields;
    std::vector<std::string> colHeads;
    std::string tempStr;

    lines.clear();
    QTableWidgetItem *item;
    if (csvTableFile.open(QFile::ReadOnly)) {
        content = csvTableFile.readAll();
        //lines   = content.split("\n");
        tempStr = content.toStdString();
        boost::split(lines,tempStr,boost::is_any_of("\n"),boost::token_compress_on);
        //lines   = content.toStdString() .split("\n");
        numLines = 0;
        foreach (std::string str, lines) {
            if (str.length() > 0)
                ++numLines;
        }
        Setup_Tab6_CSVFileTW->setRowCount(numLines-1); // -1 since first row is header
        Setup_Tab6_CSVFileTW->setColumnCount(numColumnsTable(table)); //TableNumFields[table]);

        boost::split(colHeads,lines[0],boost::is_any_of(",\t "),boost::token_compress_on);
        for (std::string colHead : colHeads) {
            colHeaders << QString::fromStdString(colHead);
        }
        for (int i=1; i<numLines; ++i) {
            boost::split(fields,lines[i],boost::is_any_of(","),boost::token_compress_on);
            for (int j=0; j<fields.size(); ++j) {
                boost::algorithm::trim(fields[j]);
                item = new QTableWidgetItem(QString::fromStdString(fields[j]));
                item->setTextAlignment(Qt::AlignCenter);
                Setup_Tab6_CSVFileTW->setItem(i-1,j,item);
            }
        }

    } else {
        msg = "\nCouldn't open file: " + csvFile;
        QMessageBox::warning(0,tr("File Open Error"),tr(msg.toLatin1()),QMessageBox::Ok);
    }
    Setup_Tab6_CSVFileTW->verticalHeader()->hide();
    Setup_Tab6_CSVFileTW->setHorizontalHeaderLabels(colHeaders);
    Setup_Tab6_CSVFileTW->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

} // end callback_Setup_Tab6_TableChanged


std::string
nmfSetupTab6::moreIntuitiveType(std::string oldType)
{
    if ((oldType == "int(11)") || (oldType == "tinyint(1)"))
        return "integer";
    else if ((oldType == "varchar(75)")  ||
             (oldType == "varchar(100)") ||
             (oldType == "varchar(255)"))
        return "text";
    else if ((oldType == "float") || (oldType == "double"))
        return "real number";

    return oldType;

} // end moreIntuitiveType



void
nmfSetupTab6::readSettings() {
    // Read the settings and load into class variables.
    QSettings settings("NOAA", "MSVPA_X2");
    settings.beginGroup("SetupTab");
    ProjectDir      = settings.value("ProjectDir","").toString();
    ProjectDatabase = settings.value("ProjectDatabase","").toString();
    settings.endGroup();

} // end readSettings


void
nmfSetupTab6::loadModelTables(int modelType)
{
    int index = 0;

    Setup_Tab6_TableListCMB->clear();
    for (auto const& tableName : TableNames[modelType]) {
        Setup_Tab6_TableListCMB->addItem(QString::fromStdString(tableName.first));
        Setup_Tab6_TableListCMB->setItemData(index,QString::fromStdString(TableDescriptions[tableName.first]),Qt::ToolTipRole);
        ++index;
    }
    Setup_Tab6_TableListCMB->setSizeAdjustPolicy(QComboBox::AdjustToContents);

} // end loadModelTables

void
nmfSetupTab6::loadWidgets(nmfDatabase *theDatabasePtr)
{
    databasePtr = theDatabasePtr;

    nmfUtils::readTableNames(TableNames);
    loadModelTables(2);

} // end loadWidgets


void
nmfSetupTab6::clearWidgets()
{

}
