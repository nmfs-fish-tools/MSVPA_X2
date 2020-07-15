#include "GuiControlsConsumptionRates.h"

GuiControlsConsumptionRates::GuiControlsConsumptionRates()
{
    databasePtr = NULL;
    logger      = NULL;
    MSVPAName   = "";
    ForecastName = "";
    ScenarioName = "";
    ModelName    = "";

    // Load up all other widgets with Population Size data
    SelectVariableCMB->addItem("Total Biomass Consumed");
    SelectVariableCMB->addItem("Consumption by Predator Age");
    SelectVariableCMB->addItem("Consumption by Prey Type");
    SelectVariableCMB->addItem("Cons. by Pred. Age and Prey Type");
    SelectVariableCMB->addItem("Consumption by Prey Age");
    SelectVariableCMB->addItem("Wt. Specific Consumption");

    SelectByVariablesCMB->addItem("Annual");
    SelectByVariablesCMB->addItem("Seasonal");

    // Enable the appropriate widgets
    // Set initial enabled states
    SelectSeasonLBL->setEnabled(false);
    SelectSeasonCMB->setEnabled(false);
    SeasonScaleCB->setEnabled(false);
    SeasonScaleLE->setEnabled(false);
    SelectPredatorAgeSizeClassLBL->setEnabled(false);
    SelectPredatorAgeSizeClassCMB->setEnabled(false);
    AgeSizeScaleCB->setEnabled(false);
    AgeSizeScaleLE->setEnabled(false);
    SelectPreyLBL->setEnabled(false);
    SelectPreyCMB->setEnabled(false);
    SelectionModeLBL->setEnabled(false);
    SelectionModeCMB->setEnabled(false);

    // Select initial check states
    HorizontalLinesCB->setChecked(true);
    VerticalLinesCB->setChecked(true);

    // Set up connections for widgets
    connect(SelectPredatorCMB,    SIGNAL(activated(QString)),
            this,                 SLOT(callback_SelectPredatorChanged(QString)));
    connect(SelectVariableCMB,    SIGNAL(activated(QString)),
            this,                 SLOT(callback_SelectVariableChanged(QString)));
    connect(SelectByVariablesCMB, SIGNAL(activated(QString)),
            this, SLOT(callback_SelectByVariablesChanged(QString)));
    connect(SelectSeasonCMB,      SIGNAL(activated(QString)),
            this,                 SLOT(callback_SelectSeasonChanged(QString)));
    connect(SelectPreyCMB,        SIGNAL(activated(QString)),
            this,                 SLOT(callback_SelectPreyChanged(QString)));
    connect(SelectPredatorAgeSizeClassCMB, SIGNAL(activated(QString)),
            this,                 SLOT(callback_SelectPredatorAgeSizeClassChanged(QString)));
    connect(HorizontalLinesCB,    SIGNAL(toggled(bool)),
            this,                 SLOT(callback_HorizontalGridLinesChanged(bool)));
    connect(VerticalLinesCB,      SIGNAL(toggled(bool)),
            this,                 SLOT(callback_VerticalGridLinesChanged(bool)));
    connect(SeasonScaleLE,        SIGNAL(textEdited(QString)),
            this,                 SLOT(callback_SeasonScaleChanged(QString)));
    connect(AgeSizeScaleLE,       SIGNAL(textEdited(QString)),
            this,                 SLOT(callback_AgeSizeScaleChanged(QString)));
    connect(SeasonScaleCB,        SIGNAL(stateChanged(int)),
            this,                 SLOT(callback_SeasonScaleCBChanged(int)));
    connect(AgeSizeScaleCB,       SIGNAL(stateChanged(int)),
            this,                 SLOT(callback_AgeSizeScaleCBChanged(int)));
    connect(ThemesCMB,      SIGNAL(currentIndexChanged(int)),
            this,                 SLOT(callback_chartThemeChanged(int)));

    // Set initial states
    resetWidgets();
}

GuiControlsConsumptionRates::~GuiControlsConsumptionRates()
{

}


void
GuiControlsConsumptionRates::resetWidgets()
{
    SelectPredatorCMB->setCurrentIndex(0);
    SelectVariableCMB->setCurrentIndex(0);
    SelectByVariablesCMB->setCurrentIndex(0);
    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);
    SelectPreyCMB->setCurrentIndex(0);
    SelectionModeCMB->setCurrentIndex(0);
}


nmfStructsQt::UpdateDataStruct
GuiControlsConsumptionRates::getUpdateDataStruct()
{
    nmfStructsQt::UpdateDataStruct retv;
    QString seasonScale  = SeasonScaleLE->text();
    QString ageSizeScale = AgeSizeScaleLE->text();

    retv.databasePtr         = databasePtr;
    retv.logger              = logger;
    retv.MSVPAName           = MSVPAName;
    retv.ModelName           = ModelName;
    retv.ForecastName        = ForecastName;
    retv.ScenarioName        = ScenarioName;
    retv.SelectDataType      = "Diet Composition";
    retv.SelectPredator      = SelectPredatorCMB->currentText();
    retv.SelectVariable      = SelectVariableCMB->currentText();
    retv.SelectByVariables   = SelectByVariablesCMB->currentText();
    retv.SelectSeason        = SelectSeasonCMB->currentText();
    retv.SelectPredatorAgeSizeClass = SelectPredatorAgeSizeClassCMB->currentText();
    retv.SelectPreyName      = SelectPreyCMB->currentText();
    retv.HorizontalGridLines = HorizontalLinesCB->isChecked();
    retv.VerticalGridLines   = VerticalLinesCB->isChecked();
    retv.DataTypeLabel       = "Diet";
    retv.XLabel              = ""; // is calculated in code
    retv.YLabel              = "Prop. Diet";
    retv.TitlePrefix         = ""; // is calculated in code
    retv.TitleSuffix         = "";
    retv.NumAgeSizeClasses   = SelectPredatorAgeSizeClassCMB->count();
    retv.MaxScaleY           = "";
    if (SeasonScaleCB->isChecked()) {
        retv.MaxScaleY = (seasonScale ==  ".") ? "0" : seasonScale;
    }
    else if (AgeSizeScaleCB->isChecked()) {
        retv.MaxScaleY = (ageSizeScale == ".") ? "0" : ageSizeScale;
    }


    return retv;
}


void
GuiControlsConsumptionRates::loadWidgets(nmfDatabase* theDatabasePtr,
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
    loadSelectSeasonCMB(databasePtr,MSVPAName);
    loadSelectPredatorAgeSizeClassCMB(databasePtr,MSVPAName);
    loadSelectPreyCMB(databasePtr,MSVPAName);

    emit UpdateChart(getUpdateDataStruct());
}


void
GuiControlsConsumptionRates::loadSelectPredatorCMB(nmfDatabase* databasePtr,
                                                   std::string MSVPAName)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    QStringList speciesList;

    SelectPredatorCMB->blockSignals(true);
    SelectPredatorCMB->clear();
    //
    // Load Type=0 species from MSVPAspecies table
    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM MSVPAspecies where MSVPAName='" + MSVPAName +
            "' and Type = 0";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        speciesList << QString::fromStdString(dataMap["SpeName"][i]);
    }
    //
    // Then also load species from OtherPredSpecies
    queryStr = "SELECT SpeName FROM OtherPredSpecies";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        speciesList << QString::fromStdString(dataMap["SpeName"][i]);
    }
    speciesList.sort();
    SelectPredatorCMB->addItems(speciesList);

    SelectPredatorCMB->blockSignals(false);

    // Update the prey for the new species
    loadSelectPreyCMB(databasePtr,MSVPAName);
}


void
GuiControlsConsumptionRates::loadSelectPreyCMB(nmfDatabase* databasePtr,
                                               std::string  MSVPAName)
{
    int PredAgeVal;
    std::string Species;
    std::string PredAgeStr;
    std::string ageStr;
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    Species    = SelectPredatorCMB->currentText().toStdString();
    PredAgeStr = SelectPredatorAgeSizeClassCMB->currentText().toStdString();
    ageStr     = PredAgeStr;
    PredAgeVal = (! ageStr.empty()) ? std::stoi(ageStr.erase(0,ageStr.find(" ")+1)) : 0;

    SelectPreyCMB->blockSignals(true);
    SelectPreyCMB->clear();

    fields = {"PreyName"};

    // For MSVPA
    queryStr = "SELECT DISTINCT PreyName FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
            " AND PredName = '" + Species + "'" +
            " AND PredAge = "   + std::to_string(PredAgeVal) +
            " ORDER BY PreyName";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["PreyName"].size(); ++i) {
        SelectPreyCMB->addItem(QString::fromStdString(dataMap["PreyName"][i]));
    }

    SelectPreyCMB->blockSignals(false);
}


void
GuiControlsConsumptionRates::callback_SelectPredatorChanged(QString value)
{
    // Need to reload this combo box since it's a function of species
    loadSelectPredatorAgeSizeClassCMB(databasePtr,MSVPAName);

    SelectVariableCMB->setCurrentIndex(0);
    SelectByVariablesCMB->setCurrentIndex(0);
    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}


void
GuiControlsConsumptionRates::callback_SelectVariableChanged(QString value)
{
    bool toggle = (value == "Consumption by Predator Age") ||
                  (value == "Cons. by Pred. Age and Prey Type") ||
                  (value == "Consumption by Prey Age") ||
                  (value == "Wt. Specific Consumption");

    loadSelectPreyCMB(databasePtr,MSVPAName);

    SelectPredatorAgeSizeClassLBL->setEnabled(toggle);
    SelectPredatorAgeSizeClassCMB->setEnabled(toggle);
    AgeSizeScaleCB->setEnabled(toggle);
    AgeSizeScaleLE->setEnabled(toggle);

    toggle = (value == "Consumption by Prey Age");
    SelectPreyLBL->setEnabled(toggle);
    SelectPreyCMB->setEnabled(toggle);

    toggle = (value == "Wt. Specific Consumption");
    SelectSeasonLBL->setEnabled(toggle);
    SelectSeasonCMB->setEnabled(toggle);
    SeasonScaleCB->setEnabled(toggle);
    SeasonScaleLE->setEnabled(toggle);

    SelectByVariablesCMB->setCurrentIndex(0);
    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);
    SelectPreyCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsConsumptionRates::callback_SelectByVariablesChanged(QString value)
{
    loadSelectPreyCMB(databasePtr,MSVPAName);

    SelectSeasonLBL->setEnabled(value == "Seasonal");
    SelectSeasonCMB->setEnabled(value == "Seasonal");
    SeasonScaleCB->setEnabled(value == "Seasonal");
    SeasonScaleLE->setEnabled(value == "Seasonal");

    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);
    SelectPreyCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}


void
GuiControlsConsumptionRates::callback_SelectSeasonChanged(QString value)
{
    loadSelectPreyCMB(databasePtr,MSVPAName);

    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);
    SelectPreyCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}


void
GuiControlsConsumptionRates::callback_SelectPreyChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}


void
GuiControlsConsumptionRates::callback_SelectPredatorAgeSizeClassChanged(QString value)
{
    loadSelectPreyCMB(databasePtr,MSVPAName);

    SelectPreyCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}


void
GuiControlsConsumptionRates::callback_SeasonScaleChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsConsumptionRates::callback_AgeSizeScaleChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsConsumptionRates::callback_SeasonScaleCBChanged(int toggle)
{

    if (toggle == Qt::Checked) {
        if (AgeSizeScaleCB->isChecked()) {
            AgeSizeScaleCB->setChecked(false);
            AgeSizeScaleLE->setEnabled(false);
        }
    }

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsConsumptionRates::callback_AgeSizeScaleCBChanged(int toggle)
{
    if (toggle == Qt::Checked) {
        if (SeasonScaleCB->isChecked()) {
            SeasonScaleCB->setChecked(false);
            SeasonScaleLE->setEnabled(false);
        }
    }

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsConsumptionRates::callback_HorizontalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsConsumptionRates::callback_VerticalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsConsumptionRates::callback_chartThemeChanged(int newTheme)
{
    emit UpdateTheme(newTheme);
}
