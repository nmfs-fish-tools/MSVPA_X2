#include "GuiControlsPopulationSize.h"

GuiControlsPopulationSize::GuiControlsPopulationSize()
{
    databasePtr  = NULL;
    logger       = NULL;
    MSVPAName    = "";
    ForecastName = "";
    ScenarioName = "";
    ModelName    = "";

    // Load up all other widgets with Population Size data
    SelectVariableCMB->addItem("Total Biomass");
    SelectVariableCMB->addItem("Biomass by Age");
    SelectVariableCMB->addItem("Spawning Stock Biomass");
    SelectVariableCMB->addItem("Total Abundance");
    SelectVariableCMB->addItem("Abundance by Age");
    SelectVariableCMB->addItem("Recruit Abundance");
    SelectVariableCMB->addItem("Recruit Biomass");

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
            this,                 SLOT(callback_SelectByVariablesChanged(QString)));
    connect(SelectSeasonCMB,      SIGNAL(activated(QString)),
            this,                 SLOT(callback_SelectSeasonChanged(QString)));
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

GuiControlsPopulationSize::~GuiControlsPopulationSize()
{

}


nmfStructsQt::UpdateDataStruct
GuiControlsPopulationSize::getUpdateDataStruct()
{
    nmfStructsQt::UpdateDataStruct retv;
    QString seasonScale  = SeasonScaleLE->text();
    QString ageSizeScale = AgeSizeScaleLE->text();

    retv.databasePtr         = databasePtr;
    retv.logger              = logger;
    retv.MSVPAName           = MSVPAName;
    retv.ForecastName        = ForecastName;
    retv.ScenarioName        = ScenarioName;
    retv.ModelName           = ModelName;
    retv.SelectDataType      = "Diet Composition";
    retv.SelectPredator      = SelectPredatorCMB->currentText();
    retv.SelectVariable      = SelectVariableCMB->currentText();
    retv.SelectByVariables   = SelectByVariablesCMB->currentText();
    retv.SelectSeason        = SelectSeasonCMB->currentText();
    retv.SelectPredatorAgeSizeClass = SelectPredatorAgeSizeClassCMB->currentText();
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
    retv.Theme               = ThemesCMB->currentIndex();

    return retv;
}


void
GuiControlsPopulationSize::resetWidgets()
{
    SelectPredatorCMB->setCurrentIndex(0);
    SelectVariableCMB->setCurrentIndex(0);
    SelectByVariablesCMB->setCurrentIndex(0);
    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);
    SelectPreyCMB->setCurrentIndex(0);
    SelectionModeCMB->setCurrentIndex(0);
}

void
GuiControlsPopulationSize::loadWidgets(nmfDatabase* theDatabasePtr,
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

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsPopulationSize::loadSelectPredatorCMB(nmfDatabase* databasePtr,
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
GuiControlsPopulationSize::callback_SelectPredatorChanged(QString value)
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
GuiControlsPopulationSize::callback_SelectVariableChanged(QString value)
{
    bool ageSizeToggle = (value == "Biomass by Age")    ||
                         (value == "Abundance by Age")  ||
                         (value == "Recruit Abundance") ||
                         (value == "Recruit Biomass");

    SelectPredatorAgeSizeClassLBL->setEnabled(ageSizeToggle);
    SelectPredatorAgeSizeClassCMB->setEnabled(ageSizeToggle);
    AgeSizeScaleCB->setEnabled(ageSizeToggle);
    AgeSizeScaleLE->setEnabled(ageSizeToggle);

    SelectByVariablesCMB->setCurrentIndex(0);
    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsPopulationSize::callback_SelectByVariablesChanged(QString value)
{
    SelectSeasonLBL->setEnabled(value == "Seasonal");
    SelectSeasonCMB->setEnabled(value == "Seasonal");
    SeasonScaleCB->setEnabled(value == "Seasonal");
    SeasonScaleLE->setEnabled(value == "Seasonal");

    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsPopulationSize::callback_SelectSeasonChanged(QString value)
{
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsPopulationSize::callback_SelectPredatorAgeSizeClassChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsPopulationSize::callback_SeasonScaleChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsPopulationSize::callback_AgeSizeScaleChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsPopulationSize::callback_SeasonScaleCBChanged(int toggle)
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
GuiControlsPopulationSize::callback_AgeSizeScaleCBChanged(int toggle)
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
GuiControlsPopulationSize::callback_HorizontalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsPopulationSize::callback_VerticalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsPopulationSize::callback_chartThemeChanged(int newTheme)
{
    emit UpdateTheme(newTheme);
}
