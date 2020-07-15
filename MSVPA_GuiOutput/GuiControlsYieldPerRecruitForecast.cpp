#include "GuiControlsYieldPerRecruitForecast.h"

GuiControlsYieldPerRecruitForecast::GuiControlsYieldPerRecruitForecast()
{
    databasePtr = NULL;
    logger      = NULL;
    MSVPAName   = "";
    ForecastName = "";
    ScenarioName = "";
    ModelName    = "";

    // Load main GUI elements
    SelectYPRAnalysisTypeCMB->addItem("YPR vs. F");
    SelectYPRAnalysisTypeCMB->addItem("SSB vs. F");
    SelectYPRAnalysisTypeCMB->addItem("Projected YPR");
    SelectYPRAnalysisTypeCMB->addItem("Projected F Benchmarks");
    SelectYPRAnalysisTypeCMB->addItem("Projected SSB Benchmarks");

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
    connect(ThemesCMB,          SIGNAL(currentIndexChanged(int)),
            this,                     SLOT(callback_chartThemeChanged(int)));

    // Set initial states
    resetWidgets();

}

GuiControlsYieldPerRecruitForecast::~GuiControlsYieldPerRecruitForecast()
{

}


nmfStructsQt::UpdateDataStruct
GuiControlsYieldPerRecruitForecast::getUpdateDataStruct()
{
    nmfStructsQt::UpdateDataStruct retv;
    std::vector<std::string> SelectedYears;
    QString fullyRecruitedAge = SelectFullyRecruitedAgeLE->text();

    getSelectedYears(SelectYearsLW,SelectedYears);

    retv.databasePtr             = databasePtr;
    retv.logger                  = logger;
    retv.MSVPAName               = MSVPAName;
    retv.ModelName               = ModelName;
    retv.ForecastName            = ForecastName;
    retv.ScenarioName            = ScenarioName;
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
//    retv.MaxScaleY               = (SelectFullyRecruitedAgeCB->isChecked()) ?
//                                    SelectFullyRecruitedAgeLE->text() : "";
    if (SelectFullyRecruitedAgeCB->isChecked()) {
        retv.MaxScaleY = (fullyRecruitedAge == ".") ? "" : fullyRecruitedAge;
    } else {
        retv.MaxScaleY = "";
    }
    retv.Theme                   = ThemesCMB->currentIndex();

    return retv;
}


void
GuiControlsYieldPerRecruitForecast::resetWidgets()
{
    SelectYPRAnalysisTypeCMB->setCurrentIndex(0);
    SelectFullyRecruitedAgeCMB->setCurrentIndex(0);
}


void
GuiControlsYieldPerRecruitForecast::loadWidgets(nmfDatabase* theDatabasePtr,
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
GuiControlsYieldPerRecruitForecast::loadSelectPredatorCMB(nmfDatabase* databasePtr,
                                                  std::string MSVPAName)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    QStringList speciesList;

    SelectPredatorCMB->blockSignals(true);
    SelectPredatorCMB->clear();

    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM Species";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        speciesList << QString::fromStdString(dataMap["SpeName"][i]);
    }
    speciesList.sort();
    SelectPredatorCMB->addItems(speciesList);
    SelectPredatorCMB->blockSignals(false);
}


void
GuiControlsYieldPerRecruitForecast::loadSelectYearsLW(nmfDatabase* databasePtr,
                                              std::string MSVPAName)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    int FirstYear;
    int LastYear;

    SelectYearsLW->blockSignals(true);
    SelectYearsLW->clear();

    fields = {"InitYear", "NYears"};
    queryStr = "SELECT InitYear,NYears FROM Forecasts WHERE MSVPAName ='" + MSVPAName +
            "' and ForeName = '" + ForecastName + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["InitYear"].size() == 1) {
        FirstYear = std::stoi(dataMap["InitYear"][0]);
        LastYear  = FirstYear + std::stoi(dataMap["NYears"][0]);
    } else {
        std::cout << "Error: GuiControlsYieldPerRecruitForecast::loadSelectYearsLW" << std::endl;
        return;
    }

    SelectYearsLW->blockSignals(true);
    SelectYearsLW->clear();
    for (int year=FirstYear; year<=LastYear; ++year) {
        SelectYearsLW->addItem(QString::number(year));
    }
    SelectYearsLW->setCurrentRow(0);
    SelectYearsLW->blockSignals(false);
}

void
GuiControlsYieldPerRecruitForecast::checkForProjectedType()
{
return; // RSK check this against original code
    if (SelectYPRAnalysisTypeCMB->currentIndex() > 1) {
        SelectYearsLW->setEnabled(false);
        SelectYearsLW->selectionModel()->clear();
    }
}

void
GuiControlsYieldPerRecruitForecast::callback_SelectPredatorChanged(QString value)
{
    // Need to reload this combo box since it's a function of species
    loadSelectFullyRecruitedAgeCMB(databasePtr,MSVPAName);
    SelectYearsLW->setCurrentRow(0);
    checkForProjectedType();

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruitForecast::callback_SelectYPRAnalysisTypeChanged(QString value)
{
    bool toggle = (value == "YPR vs. F") ||
                  (value == "SSB vs. F");
    toggle = true;  // RSK - check the above logic against original code


    SelectYearsLBL->setEnabled(toggle);
    SelectYearsLW->setCurrentRow(0);
    SelectYearsLW->setEnabled(toggle);

    checkForProjectedType();

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruitForecast::callback_SelectYearsChanged()
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruitForecast::callback_SelectFullyRecruitedAgeChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruitForecast::callback_FullyRecruitedAgeCBChanged(int toggle)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruitForecast::callback_FullyRecruitedAgeLEChanged(QString toggle)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruitForecast::callback_HorizontalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruitForecast::callback_VerticalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsYieldPerRecruitForecast::callback_chartThemeChanged(int newTheme)
{
    emit UpdateTheme(newTheme);
}



