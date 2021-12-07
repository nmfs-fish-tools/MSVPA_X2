
#include "nmfConstants.h"
#include "nmfForecastTab02.h"
#include "nmfUtils.h"


nmfForecastTab2::nmfForecastTab2(QTabWidget  *tabs,
                                 nmfLogger   *theLogger,
                                 std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfForecastTab2::nmfForecastTab2");

    Forecast_Tabs = tabs;
    ProjectDir = theProjectDir;
    MSVPAName.clear();
    ForecastName.clear();
    databasePtr = NULL;

    // Load ui as a widget from disk
    QFile file(":/forms/Forecast/Forecast_Tab02.ui");
    file.open(QFile::ReadOnly);
    Forecast_Tab2_Widget = loader.load(&file,Forecast_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    Forecast_Tabs->addTab(Forecast_Tab2_Widget, tr("2. Von Bert. Parameters"));

    Forecast_Tab2_PrevPB = Forecast_Tabs->findChild<QPushButton *>("Forecast_Tab2_PrevPB");
    Forecast_Tab2_NextPB = Forecast_Tabs->findChild<QPushButton *>("Forecast_Tab2_NextPB");
    Forecast_Tab2_LoadPB = Forecast_Tabs->findChild<QPushButton *>("Forecast_Tab2_LoadPB");
    Forecast_Tab2_SavePB = Forecast_Tabs->findChild<QPushButton *>("Forecast_Tab2_SavePB");
    Forecast_Tab2_GrowthParametersTV = Forecast_Tabs->findChild<QTableView *>("Forecast_Tab2_GrowthParametersTV");

    Forecast_Tab2_PrevPB->setText("\u25C1--");
    Forecast_Tab2_NextPB->setText("--\u25B7");

    connect(Forecast_Tab2_PrevPB,  SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab2_PrevPB(bool)));
    connect(Forecast_Tab2_NextPB,  SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab2_NextPB(bool)));
    connect(Forecast_Tab2_LoadPB,  SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab2_LoadPB(bool)));
    connect(Forecast_Tab2_SavePB,  SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab2_SavePB(bool)));
    //connect(vonBert_model, SIGNAL(dataChanged(QModelIndex &, QModelIndex &)),
    //       this,          SLOT(callback_TableDataChanged(QModelIndex &, QModelIndex &)));
    connect(Forecast_Tab2_GrowthParametersTV, SIGNAL(pressed(QModelIndex)),
            this,          SLOT(callback_TableDataChanged(QModelIndex)));

    Forecast_Tab2_SavePB->setEnabled(false);
    Forecast_Tab2_SavePB->setToolTip("Update table: ForePredVonB");
    Forecast_Tab2_SavePB->setStatusTip("Update table: ForePredVonB");

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab2::nmfForecastTab2 Complete");

}


nmfForecastTab2::~nmfForecastTab2()
{
}


void
nmfForecastTab2::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir



void
nmfForecastTab2::callback_Forecast_Tab2_SavePB(bool unused)
{
    std::string cmd;
    std::string errorMsg;

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab2::callback_Forecast_Tab2_SavePB");

    int nrows = vonBert_model->count();
    for (int i=0;i<nrows; ++i) {
        cmd  = "INSERT INTO " + nmfConstantsMSVPA::TableForePredVonB;
        cmd += " (MSVPAName,ForeName,PredName,PredIndex,PredType,Linf,GrowthK,TZero,LWAlpha,LWBeta) values ";
        cmd += "(\"" + MSVPAName + "\", \"" + ForecastName + "\", \"" +
                vonBert_model->index(i,0).data().toString().toStdString() + "\", " +
                vonBert_model->index(i,1).data().toString().toStdString() + "," +
                vonBert_model->index(i,2).data().toString().toStdString() + "," +
                vonBert_model->index(i,3).data().toString().toStdString() + "," +
                vonBert_model->index(i,4).data().toString().toStdString() + "," +
                vonBert_model->index(i,5).data().toString().toStdString() + "," +
                vonBert_model->index(i,6).data().toString().toStdString() + "," +
                vonBert_model->index(i,7).data().toString().toStdString() + ") ";
        cmd += "ON DUPLICATE KEY UPDATE ";
        cmd += "Linf=values(Linf),GrowthK=values(GrowthK),TZero=values(TZero),";
        cmd += "LWAlpha=values(LWAlpha),LWBeta=values(LWBeta);";
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if (nmfUtilsQt::isAnError(errorMsg)) {
            std::cout << cmd << std::endl;
            nmfUtils::printError("callback_Forecast_Tab2_SavePB: INSERT INTO ForePredVonB...", errorMsg);
        }
    }

    restoreCSVFromDatabase(databasePtr);

    QMessageBox::information(Forecast_Tabs, "Save",
                             tr("\nForePredVonB data saved.\n"));

    Forecast_Tab2_SavePB->setEnabled(false);
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab2::callback_Forecast_Tab2_SavePB Complete");

} // end callback_Forecast_Tab2_SavePB



void
nmfForecastTab2::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    TableName = QString::fromStdString(nmfConstantsMSVPA::TableForePredVonB);
    fields    = {"MSVPAName","ForeName","PredName","PredIndex","PredType",
                 "Linf","GrowthK","TZero","LWAlpha","LWBeta"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase



void
nmfForecastTab2::callback_Forecast_Tab2_LoadPB(bool unused)
{
    logger->logMsg(nmfConstants::Normal,"nmfForecastTab2::callback_Forecast_Tab2_Load");

    int csvFileRow = 0;
    QString line;
    QStringList fields;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QStringList NameFilters = QStringList() << "*.csv" << "*.*";
    QString ForePredVonBCSVFile;
    QFileDialog *fileDlg;
    nmfVonBertModel *theVonBert_model;

    fileDlg = new QFileDialog(Forecast_Tabs);

    theVonBert_model = qobject_cast<nmfVonBertModel *>(Forecast_Tab2_GrowthParametersTV->model());

    // Setup Load dialog
    fileDlg->setDirectory(path);
    fileDlg->selectFile(QString::fromStdString(nmfConstantsMSVPA::TableForePredVonB)+".csv");
    fileDlg->setNameFilters(NameFilters);
    fileDlg->setWindowTitle("Load Forecast Predator Von B CSV File");
    if (fileDlg->exec()) {

        // Open the file here....
        ForePredVonBCSVFile = fileDlg->selectedFiles()[0];
        QFile file(ForePredVonBCSVFile);
        if (! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(Forecast_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);

        // Store data to load in next step
        Forecast_Tab2_GrowthParametersTV->blockSignals(true);
        theVonBert_model->removeRows(0, theVonBert_model->count(), QModelIndex());
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header
                fields = line.split(",");
                if ((MSVPAName    == fields[0].trimmed().toStdString()) &&
                    (ForecastName == fields[1].trimmed().toStdString()))
                {
                    theVonBert_model->append(nmfVonBert {
                         fields[2].trimmed(),
                         fields[3].trimmed().toInt(),
                         fields[4].trimmed().toInt(),
                         fields[5].trimmed().toDouble(),
                         fields[6].trimmed().toDouble(),
                         fields[7].trimmed().toDouble(),
                         fields[8].trimmed().toDouble(),
                         fields[9].trimmed().toDouble() });
                }
            }
            ++csvFileRow;
        } // end while
        file.close();
        Forecast_Tab2_GrowthParametersTV->blockSignals(false);
        Forecast_Tab2_GrowthParametersTV->resizeColumnsToContents();
        Forecast_Tab2_GrowthParametersTV->scrollToTop();
        Forecast_Tab2_SavePB->setEnabled(true);

    } // end if
    fileDlg->close();
    fileDlg->deleteLater();

    Forecast_Tab2_LoadPB->clearFocus();
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab2::callback_Forecast_Tab2_Load Complete");


//    emit LoadDataTable(MSVPAName,ForecastName,
//                       "Forecast","ForePredVonB",
//                       "Forecast",1);

} // end callback_Forecast_Tab2_Load


void
nmfForecastTab2::callback_TableDataChanged(QModelIndex unused)
{
    Forecast_Tab2_SavePB->setEnabled(true);

    MarkAsDirty(nmfConstantsMSVPA::TableForePredVonB);

} // end callback_TableDataChanged

//void
//nmfForecastTab2::callback_TableDataChanged(QModelIndex &unused1, QModelIndex &unused2)
//{
//    MarkAsDirty(nmfConstantsMSVPA::TableForePredVonB);
//} // end callback_TableDataChanged

void
nmfForecastTab2::callback_Forecast_Tab2_PrevPB(bool unused)
{
    Forecast_Tabs->setCurrentIndex(Forecast_Tabs->currentIndex()-1);
}

void
nmfForecastTab2::callback_Forecast_Tab2_NextPB(bool unused)
{
    Forecast_Tabs->setCurrentIndex(Forecast_Tabs->currentIndex()+1);
}


void
nmfForecastTab2::MarkAsDirty(std::string tableName)
{
    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty

void
nmfForecastTab2::MarkAsClean()
{
    emit MarkForecastAsClean();
} // end MarkAsClean


void
nmfForecastTab2::loadWidgets(
        nmfDatabase *theDatabasePtr,
        nmfVonBertModel &theVonBert_model,
        std::string theMSVPAName,
        std::string theForecastName)
{
    std::string queryStr = "";
    std::map<std::string, std::vector<std::string> > dataMap;
    std::map<std::string, std::vector<std::string> > dataMap2;
    std::vector<std::string> fields;
    MSVPAName    = theMSVPAName;
    ForecastName = theForecastName;
    databasePtr  = theDatabasePtr;

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab2::loadWidgets");

    vonBert_model = &theVonBert_model;

    // Load Von Bert data from table
    fields = {"PredName","PredIndex","PredType","Linf","GrowthK","TZero","LWAlpha","LWBeta"};
    queryStr = "SELECT PredName,PredIndex,PredType,Linf,GrowthK,TZero,LWAlpha,LWBeta FROM " +
                nmfConstantsMSVPA::TableForePredVonB +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND ForeName = '" + ForecastName + "'" +
               " ORDER BY PredIndex";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    int NumRecords = dataMap["Linf"].size();

    if (NumRecords == 0) {
        theVonBert_model.removeRows(0, theVonBert_model.count(), QModelIndex());
        fields = {"SpeName"};
        queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
                   " WHERE MSVPAName = '" + MSVPAName + "'" +
                   " AND (Type = 0 or Type = 1)";
        dataMap2 = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (int i=0; i<int(dataMap2["SpeName"].size());++i) {
            theVonBert_model.append(nmfVonBert {
                QString::fromStdString(dataMap2["SpeName"][i]),0,0,0,0,0,0,0});
        }

    } else {
        // Load Von Bert model
        theVonBert_model.removeRows(0, theVonBert_model.count(), QModelIndex());
        for (int i=0; i<NumRecords; ++i) {
            theVonBert_model.append(nmfVonBert {
                                        QString::fromStdString(dataMap["PredName"][i]),
                                        std::stoi(dataMap["PredIndex"][i]),
                                        std::stoi(dataMap["PredType"][i]),
                                        std::stod(dataMap["Linf"][i]),
                                        std::stod(dataMap["GrowthK"][i]),
                                        std::stod(dataMap["TZero"][i]),
                                        std::stod(dataMap["LWAlpha"][i]),
                                        std::stod(dataMap["LWBeta"][i]) });
        }
    }
    Forecast_Tab2_GrowthParametersTV->resizeColumnsToContents();
    Forecast_Tab2_GrowthParametersTV->scrollToTop();

    Forecast_Tab2_SavePB->setEnabled(false);
    MarkAsClean();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab2::loadWidgets Complete");

} // end loadWidgets







//void
//nmfForecastTab2::loadWidgets(QString ForecastName, int NYears, int FirstYear, int LastYear)
//{
//    Forecast_Tab2_ScenarioNameLE->clear();
//    Forecast_Tab2_ForecastNameLE->setText(ForecastName);
//    Forecast_Tab2_NumYearsLE->setText(QString::number(NYears));
//    Forecast_Tab2_InitialYearCMB->clear();
//    for (int year=FirstYear; year<=LastYear; ++year) {
//        Forecast_Tab2_InitialYearCMB->addItem(QString::number(year));
//    }
//    Forecast_Tab2_InitialYearCMB->setCurrentIndex(Forecast_Tab2_InitialYearCMB->count()-1);
//}

void
nmfForecastTab2::clearWidgets()
{
//    Forecast_Tab2_MsvpaNameLE->clear();
//    Forecast_Tab2_ForecastNameLE->clear();
//    Forecast_Tab2_ScenarioNameLE->clear();
//    Forecast_Tab2_InitialYearCMB->clear();
//    Forecast_Tab2_NumYearsLE->clear();
//    Forecast_Tab2_PredatorGrowthCB->setChecked(false);
}
