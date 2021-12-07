#include "GuiControlsYieldPerRecruit.h"

GuiControlsYieldPerRecruit::GuiControlsYieldPerRecruit()
{
    databasePtr = NULL;
    logger      = NULL;
    MSVPAName   = "";

    // Load main GUI elements
    SelectYPRAnalysisTypeCMB->addItem("YPR vs. F");
    SelectYPRAnalysisTypeCMB->addItem("SSB vs. F");
    SelectYPRAnalysisTypeCMB->addItem("Historical YPR");
    SelectYPRAnalysisTypeCMB->addItem("Historical F Benchmarks");
    SelectYPRAnalysisTypeCMB->addItem("Historical SSB Benchmarks");

    SelectionModeLBL->hide();
    SelectionModeCMB->hide();

    // Select initial check states
    HorizontalLinesCB->setChecked(true);
    VerticalLinesCB->setChecked(true);

    // Set up connections for widgets
    connect(SelectPredatorCMB,        SIGNAL(activated(QString)),
            this,                     SLOT(callback_SelectPredatorChanged(QString)));
    connect(SelectYPRAnalysisTypeCMB, SIGNAL(activated(QString)),
            this,                     SLOT(callback_SelectYPRAnalysisTypeChanged(QString)));
    connect(SelectYearsLW,            SIGNAL(itemSelectionChanged()),
            this,                     SLOT(callback_SelectYearsChanged()));
    connect(SelectFullyRecruitedAgeCMB, SIGNAL(activated(QString)),
            this,                     SLOT(callback_SelectFullyRecruitedAgeChanged(QString)));
    connect(SelectFullyRecruitedAgeCB,SIGNAL(stateChanged(int)),
            this,                     SLOT(callback_FullyRecruitedAgeCBChanged(int)));
    connect(SelectFullyRecruitedAgeLE,SIGNAL(textEdited(QString)),
            this,                     SLOT(callback_FullyRecruitedAgeLEChanged(QString)));
    connect(HorizontalLinesCB,        SIGNAL(toggled(bool)),
            this,                     SLOT(callback_HorizontalGridLinesChanged(bool)));
    connect(VerticalLinesCB,          SIGNAL(toggled(bool)),
            this,                     SLOT(callback_VerticalGridLinesChanged(bool)));
    connect(ThemesCMB,      SIGNAL(currentIndexChanged(int)),
            this,                 SLOT(callback_chartThemeChanged(int)));

    // Set initial states
    resetWidgets();

}

GuiControlsYieldPerRecruit::~GuiControlsYieldPerRecruit()
{

}


nmfStructsQt::UpdateDataStruct
GuiControlsYieldPerRecruit::getUpdateDataStruct()
{
    nmfStructsQt::UpdateDataStruct retv;
    std::vector<std::string> SelectedYears;
    QString fullyRecruitedAge = SelectFullyRecruitedAgeLE->text();

    getSelectedYears(SelectYearsLW,SelectedYears);

    retv.databasePtr             = databasePtr;
    retv.logger                  = logger;
    retv.MSVPAName               = MSVPAName;
    retv.ModelName               = "MSVPA";
    retv.SelectDataType          = "Diet Composition";
    retv.SelectPredator          = SelectPredatorCMB->currentText();
    retv.HorizontalGridLines     = HorizontalLinesCB->isChecked();
    retv.VerticalGridLines       = VerticalLinesCB->isChecked();
    retv.DataTypeLabel           = "Diet";
    retv.XLabel                  = ""; // is calculated in code
    retv.YLabel                  = "Prop. Diet";
    retv.TitlePrefix             = ""; // is calculated in code
    retv.TitleSuffix             = "";
    retv.NumAgeSizeClasses       = SelectFullyRecruitedAgeCMB->count();
    retv.SelectFullyRecruitedAge = SelectFullyRecruitedAgeCMB->currentText();
    retv.SelectYPRAnalysisType   = SelectYPRAnalysisTypeCMB->currentText();
    retv.SelectedYears           = SelectedYears;
    if (SelectFullyRecruitedAgeCB->isChecked()) {
        retv.MaxScaleY = (fullyRecruitedAge == ".") ? "" : fullyRecruitedAge;
    } else {
        retv.MaxScaleY = "";
    }
//    retv.MaxScaleY               = (SelectFullyRecruitedAgeCB->isChecked()) ?
//                                    SelectFullyRecruitedAgeLE->text() : "";
    retv.Theme                   = ThemesCMB->currentIndex();

    return retv;
}


void
GuiControlsYieldPerRecruit::resetWidgets()
{
    SelectYPRAnalysisTypeCMB->setCurrentIndex(0);
    SelectFullyRecruitedAgeCMB->setCurrentIndex(0);
}


void
GuiControlsYieldPerRecruit::loadWidgets(nmfDatabase* theDatabasePtr,
                                        nmfLogger*   theLogger,
                                        std::string  theMSVPAName,
                                        std::string  theModelName,
                                        std::string  theForecastName,
                                        std::string  theScenarioName)
{
    databasePtr  = theDatabasePtr;
    logger       = theLogger;
    MSVPAName    = theMSVPAName;
    ModelName    = theModelName;
    ForecastName = theForecastName;
    ScenarioName = theScenarioName;

    loadSelectPredatorCMB(databasePtr,MSVPAName);
    loadSelectFullyRecruitedAgeCMB(databasePtr,MSVPAName);
    loadSelectYearsLW(databasePtr,MSVPAName);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruit::loadSelectPredatorCMB(nmfDatabase* databasePtr,
                                                  std::string MSVPAName)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    QStringList speciesList;

    SelectPredatorCMB->blockSignals(true);
    SelectPredatorCMB->clear();

    fields = {"SpeName"};
    //  queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableSpecies;
        queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
                   " WHERE MSVPAName='" + MSVPAName +
                   "' and (Type = 0 or Type = 1)";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        speciesList << QString::fromStdString(dataMap["SpeName"][i]);
    }
    speciesList.sort();
    SelectPredatorCMB->addItems(speciesList);
    SelectPredatorCMB->blockSignals(false);
}


void
GuiControlsYieldPerRecruit::loadSelectYearsLW(nmfDatabase* databasePtr,
                                              std::string MSVPAName)
{
    // Find FirstYear, LastYear, and NumSeasons
    std::map<std::string,int> initMap = databasePtr->nmfQueryInitFields(
                nmfConstantsMSVPA::TableMSVPAlist, MSVPAName);
    int FirstYear  = initMap["FirstYear"];
    int LastYear   = initMap["LastYear"];

    SelectYearsLW->blockSignals(true);

//    std::string theModelName = modelName();
    SelectYearsLW->clear();
//    if (theModelName == "MSVPA") {
        for (int year=FirstYear; year<=LastYear; ++year) {
            SelectYearsLW->addItem(QString::number(year));
        }
//    } else if (theModelName == "Forecast") {
//        int firstYear = forecastFirstYear();
//        int numYears  = forecastNYears();
//        for (int year=firstYear; year<=firstYear+numYears-1; ++year) {
//            selectYearsLW->addItem(QString::number(year));
//        }
//    }
    SelectYearsLW->setCurrentRow(0);
    SelectYearsLW->blockSignals(false);
}

void
GuiControlsYieldPerRecruit::checkForHistoricalType()
{
    if (SelectYPRAnalysisTypeCMB->currentIndex() > 1) {
        SelectYearsLW->setEnabled(false);
        SelectYearsLW->selectionModel()->clear();
    }
}

void
GuiControlsYieldPerRecruit::callback_SelectPredatorChanged(QString value)
{
    // Need to reload this combo box since it's a function of species
    loadSelectFullyRecruitedAgeCMB(databasePtr,MSVPAName);
    SelectYearsLW->setCurrentRow(0);
    checkForHistoricalType();

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruit::callback_SelectYPRAnalysisTypeChanged(QString value)
{
    bool toggle = (value == "YPR vs. F") ||
                  (value == "SSB vs. F");

    SelectYearsLBL->setEnabled(toggle);
    SelectYearsLW->setCurrentRow(0);
    SelectYearsLW->setEnabled(toggle);

    checkForHistoricalType();

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruit::callback_SelectYearsChanged()
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruit::callback_SelectFullyRecruitedAgeChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruit::callback_FullyRecruitedAgeCBChanged(int toggle)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruit::callback_FullyRecruitedAgeLEChanged(QString toggle)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruit::callback_HorizontalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruit::callback_VerticalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruit::callback_chartThemeChanged(int newTheme)
{
    emit UpdateTheme(newTheme);
}




