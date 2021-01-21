#include "GuiControlsMortalityRates.h"

GuiControlsMortalityRates::GuiControlsMortalityRates()
{
    databasePtr = NULL;
    logger      = NULL;
    MSVPAName   = "";

    // Load up all other widgets with Population Size data
    SelectVariableCMB->addItem("Fishing Mortality");
    SelectVariableCMB->addItem("Average Recruited F");
    SelectVariableCMB->addItem("Predation Mortality");
    SelectVariableCMB->addItem("Predation Mortality - 3D");
    SelectVariableCMB->addItem("Predation Mortality by Predator");

    SelectionModeCMB->addItem("No Selection");
    SelectionModeCMB->addItem("Item");
    SelectionModeCMB->addItem("Row Slice");
    SelectionModeCMB->addItem("Column Slice");

    SelectByVariablesCMB->addItem("Annual");
    SelectByVariablesCMB->addItem("Seasonal");

    // Enable the appropriate widgets
    // Set initial enabled states
    SelectSeasonLBL->setEnabled(false);
    SelectSeasonCMB->setEnabled(false);
    SeasonScaleCB->setEnabled(false);
    SeasonScaleLE->setEnabled(false);
    SelectPredatorAgeSizeClassLBL->setEnabled(true);
    SelectPredatorAgeSizeClassCMB->setEnabled(true);
    AgeSizeScaleCB->setEnabled(true);
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
    connect(SeasonScaleLE,        SIGNAL(textEdited(QString)),
            this,                 SLOT(callback_SeasonScaleChanged(QString)));
    connect(AgeSizeScaleLE,       SIGNAL(textEdited(QString)),
            this,                 SLOT(callback_AgeSizeScaleChanged(QString)));
    connect(SeasonScaleCB,        SIGNAL(stateChanged(int)),
            this,                 SLOT(callback_SeasonScaleCBChanged(int)));
    connect(AgeSizeScaleCB,       SIGNAL(stateChanged(int)),
            this,                 SLOT(callback_AgeSizeScaleCBChanged(int)));
    connect(HorizontalLinesCB,    SIGNAL(toggled(bool)),
            this,                 SLOT(callback_HorizontalGridLinesChanged(bool)));
    connect(VerticalLinesCB,      SIGNAL(toggled(bool)),
            this,                 SLOT(callback_VerticalGridLinesChanged(bool)));
    connect(ThemesCMB,            SIGNAL(currentIndexChanged(int)),
            this,                 SLOT(callback_chartThemeChanged(int)));
    connect(SelectionModeCMB,     SIGNAL(currentIndexChanged(int)),
            this,                 SLOT(callback_SelectionModeChanged(int)));

    // Set initial states
    resetWidgets();


}

GuiControlsMortalityRates::~GuiControlsMortalityRates()
{

}


void
GuiControlsMortalityRates::resetWidgets()
{
    SelectPredatorCMB->setCurrentIndex(0);
    SelectVariableCMB->setCurrentIndex(0);
    SelectByVariablesCMB->setCurrentIndex(0);
    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);
    SelectPreyCMB->setCurrentIndex(0);
    SelectionModeCMB->setCurrentIndex(1);
}


nmfStructsQt::UpdateDataStruct
GuiControlsMortalityRates::getUpdateDataStruct()
{
    nmfStructsQt::UpdateDataStruct retv;
    QString seasonScale  = SeasonScaleLE->text();
    QString ageSizeScale = AgeSizeScaleLE->text();

    retv.databasePtr         = databasePtr;
    retv.logger              = logger;
    retv.MSVPAName           = MSVPAName;
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
    retv.ModelName           = "MSVPA";
    retv.Theme               = ThemesCMB->currentIndex();
    retv.SelectMode          = SelectionModeCMB->currentText();

    return retv;
}


void
GuiControlsMortalityRates::loadWidgets(nmfDatabase* theDatabasePtr,
                                       nmfLogger*   theLogger,
                                       std::string  theMSVPAName,                                         std::string  theModelName,
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
GuiControlsMortalityRates::loadSelectPredatorCMB(nmfDatabase* databasePtr,
                                                   std::string MSVPAName)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    QStringList speciesList;

    SelectPredatorCMB->blockSignals(true);
    SelectPredatorCMB->clear();

    fields = {"SpeName"};
    //  queryStr = "SELECT SpeName FROM Species";
        queryStr = "SELECT SpeName FROM MSVPAspecies where MSVPAName='" + MSVPAName +
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
GuiControlsMortalityRates::callback_SelectPredatorChanged(QString value)
{
    // Need to reload this combo box since it's a function of species
    loadSelectPredatorAgeSizeClassCMB(databasePtr,MSVPAName);

    if (SelectVariableCMB->currentText() != "Predation Mortality - 3D") {
        SelectVariableCMB->setCurrentIndex(0);
        SelectByVariablesCMB->setCurrentIndex(0);
        SelectSeasonCMB->setCurrentIndex(0);
        SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);
    }

    emit UpdateChart(getUpdateDataStruct());
}


void
GuiControlsMortalityRates::callback_SelectVariableChanged(QString value)
{
    bool toggle = (value == "Predation Mortality - 3D");

    SelectPredatorAgeSizeClassLBL->setEnabled(! toggle);
    SelectPredatorAgeSizeClassCMB->setEnabled(! toggle);
    AgeSizeScaleCB->setEnabled(! toggle);
    AgeSizeScaleLE->setEnabled(! toggle);
    SelectionModeLBL->setEnabled(toggle);
    SelectionModeCMB->setEnabled(toggle);
    SelectSeasonCMB->setEnabled(toggle);
    SeasonScaleCB->setEnabled(toggle);
    SeasonScaleLE->setEnabled(toggle);

    SelectByVariablesCMB->setCurrentIndex(0);
    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsMortalityRates::callback_SelectByVariablesChanged(QString value)
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
GuiControlsMortalityRates::callback_SelectSeasonChanged(QString value)
{
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsMortalityRates::callback_SelectPredatorAgeSizeClassChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}


void
GuiControlsMortalityRates::callback_SeasonScaleChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsMortalityRates::callback_AgeSizeScaleChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsMortalityRates::callback_SeasonScaleCBChanged(int toggle)
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
GuiControlsMortalityRates::callback_AgeSizeScaleCBChanged(int toggle)
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
GuiControlsMortalityRates::callback_HorizontalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsMortalityRates::callback_VerticalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}


void
GuiControlsMortalityRates::callback_chartThemeChanged(int newTheme)
{
    emit UpdateTheme(newTheme);
}

void
GuiControlsMortalityRates::callback_SelectionModeChanged(int mode)
{
    emit UpdateChart(getUpdateDataStruct());
}

