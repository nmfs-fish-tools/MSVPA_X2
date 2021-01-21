#include "GuiControlsMultispeciesPopulations.h"

GuiControlsMultispeciesPopulations::GuiControlsMultispeciesPopulations()
{
    databasePtr = NULL;
    logger      = NULL;
    MSVPAName   = "";
    ModelName.clear();
    ForecastName.clear();
    ScenarioName.clear();

    // Load up all other widgets with Population Size data
    SelectVariableCMB->addItem("Total Biomass");
    SelectVariableCMB->addItem("Total Abundance");
    SelectVariableCMB->addItem("Age 1+ Biomass");
    SelectVariableCMB->addItem("Age 1+ Abundance");
    SelectVariableCMB->addItem("Spawning Stock Biomass");

    SelectByVariablesCMB->addItem("Annual");
    SelectByVariablesCMB->addItem("Seasonal");

    // Enable the appropriate widgets
    // Set initial enabled states
    SelectPredatorLBL->setEnabled(false);
    SelectPredatorCMB->setEnabled(false);
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
    connect(SelectVariableCMB, SIGNAL(activated(QString)),
            this, SLOT(callback_SelectVariableChanged(QString)));
    connect(SelectByVariablesCMB, SIGNAL(activated(QString)),
            this, SLOT(callback_SelectByVariablesChanged(QString)));
    connect(SelectSeasonCMB,  SIGNAL(activated(QString)),
            this, SLOT(callback_SelectSeasonChanged(QString)));
    connect(SelectPredatorAgeSizeClassCMB, SIGNAL(activated(QString)),
            this, SLOT(callback_SelectPredatorAgeSizeClassChanged(QString)));
    connect(SeasonScaleLE,        SIGNAL(textEdited(QString)),
            this,                 SLOT(callback_SeasonScaleChanged(QString)));
    connect(SeasonScaleCB,        SIGNAL(stateChanged(int)),
            this,                 SLOT(callback_SeasonScaleCBChanged(int)));
    connect(HorizontalLinesCB,    SIGNAL(toggled(bool)),
            this,                 SLOT(callback_HorizontalGridLinesChanged(bool)));
    connect(VerticalLinesCB,      SIGNAL(toggled(bool)),
            this,                 SLOT(callback_VerticalGridLinesChanged(bool)));
    connect(ThemesCMB,      SIGNAL(currentIndexChanged(int)),
            this,                 SLOT(callback_chartThemeChanged(int)));
    connect(ThemesCMB,      SIGNAL(currentIndexChanged(int)),
            this,                 SLOT(callback_chartThemeChanged(int)));

    // Set initial states
    resetWidgets();


}

GuiControlsMultispeciesPopulations::~GuiControlsMultispeciesPopulations()
{

}


void
GuiControlsMultispeciesPopulations::resetWidgets()
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
GuiControlsMultispeciesPopulations::getUpdateDataStruct()
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
    retv.SelectDataType      = "";
    retv.SelectPredator      = SelectPredatorCMB->currentText();
    retv.SelectVariable      = SelectVariableCMB->currentText();
    retv.SelectByVariables   = SelectByVariablesCMB->currentText();
    retv.SelectSeason        = SelectSeasonCMB->currentText();
    retv.SelectPredatorAgeSizeClass = SelectPredatorAgeSizeClassCMB->currentText();
    retv.HorizontalGridLines = HorizontalLinesCB->isChecked();
    retv.VerticalGridLines   = VerticalLinesCB->isChecked();
    retv.DataTypeLabel       = "";
    retv.XLabel              = ""; // is calculated in code
    retv.YLabel              = "";
    retv.TitlePrefix         = ""; // is calculated in code
    retv.TitleSuffix         = "";
    retv.NumAgeSizeClasses   = SelectPredatorAgeSizeClassCMB->count();
    retv.MaxScaleY           = "";
    if (SeasonScaleCB->isChecked()) {
        retv.MaxScaleY = (seasonScale ==  "") ? "0" : seasonScale;
    }
    else if (AgeSizeScaleCB->isChecked()) {
        retv.MaxScaleY = (ageSizeScale == "") ? "0" : ageSizeScale;
    }

    retv.Theme               = ThemesCMB->currentIndex();

    return retv;
}

void
GuiControlsMultispeciesPopulations::loadWidgets(nmfDatabase* theDatabasePtr,
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

    //loadSelectPredatorCMB(databasePtr,MSVPAName);
    loadSelectSeasonCMB(databasePtr,MSVPAName);
    loadSelectPredatorAgeSizeClassCMB(databasePtr,MSVPAName);

    emit UpdateChart(getUpdateDataStruct());
}


void
GuiControlsMultispeciesPopulations::callback_SelectVariableChanged(QString value)
{
    SelectByVariablesCMB->setCurrentIndex(0);
    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);
    SelectPreyCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}


void
GuiControlsMultispeciesPopulations::callback_SelectByVariablesChanged(QString value)
{
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
GuiControlsMultispeciesPopulations::callback_SelectSeasonChanged(QString value)
{
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);
    SelectPreyCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsMultispeciesPopulations::callback_SelectPredatorAgeSizeClassChanged(QString value)
{
    SelectPreyCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsMultispeciesPopulations::callback_SeasonScaleChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsMultispeciesPopulations::callback_SeasonScaleCBChanged(int toggle)
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
GuiControlsMultispeciesPopulations::callback_HorizontalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsMultispeciesPopulations::callback_VerticalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsMultispeciesPopulations::callback_chartThemeChanged(int newTheme)
{
    emit UpdateTheme(newTheme);
}


