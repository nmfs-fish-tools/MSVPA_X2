#include "GuiControlsFoodAvailability.h"

GuiControlsFoodAvailability::GuiControlsFoodAvailability()
{
    databasePtr = NULL;
    logger      = NULL;
    MSVPAName.clear();
    ModelName.clear();
    ForecastName.clear();
    ScenarioName.clear();

    // Load up all other widgets with Population Size data
    SelectVariableCMB->addItem("Relative Food Availability");
    SelectVariableCMB->addItem("Per Capita Food Availability");
    SelectVariableCMB->addItem("Food Availability by Prey Type");
    SelectVariableCMB->addItem("Food Availability by Prey Age");
    SelectVariableCMB->addItem("Stomach Content Weight");

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
            this, SLOT(callback_SelectByVariablesChanged(QString)));
    connect(SelectSeasonCMB,      SIGNAL(activated(QString)),
            this,                 SLOT(callback_SelectSeasonChanged(QString)));
    connect(SelectPreyCMB,        SIGNAL(activated(QString)),
            this,                 SLOT(callback_SelectPreyChanged(QString)));
    connect(SelectPredatorAgeSizeClassCMB, SIGNAL(activated(QString)),
            this,                          SLOT(callback_SelectPredatorAgeSizeClassChanged(QString)));
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
    connect(ThemesCMB,      SIGNAL(currentIndexChanged(int)),
            this,                 SLOT(callback_chartThemeChanged(int)));

    // Set initial states
    resetWidgets();
}

GuiControlsFoodAvailability::~GuiControlsFoodAvailability()
{

}

void
GuiControlsFoodAvailability::resetWidgets()
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
GuiControlsFoodAvailability::getUpdateDataStruct()
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
GuiControlsFoodAvailability::loadWidgets(nmfDatabase* theDatabasePtr,
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
GuiControlsFoodAvailability::loadSelectPredatorCMB(nmfDatabase* databasePtr,
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
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAName='" + MSVPAName +
                "' and Type = 0";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        speciesList << QString::fromStdString(dataMap["SpeName"][i]);
    }
    //
    // Then also load species from OtherPredSpecies
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableOtherPredSpecies;
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        speciesList << QString::fromStdString(dataMap["SpeName"][i]);
    }
    speciesList.sort();
    SelectPredatorCMB->addItems(speciesList);

    SelectPredatorCMB->blockSignals(false);
}


void
GuiControlsFoodAvailability::loadSelectPreyCMB(nmfDatabase* databasePtr,
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
    queryStr = "SELECT DISTINCT PreyName FROM " + nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
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
GuiControlsFoodAvailability::callback_SelectPredatorChanged(QString value)
{
    // Need to reload this combo box since it's a function of species
    loadSelectPredatorAgeSizeClassCMB(databasePtr,MSVPAName);

    SelectVariableCMB->setCurrentIndex(0);
    SelectByVariablesCMB->setCurrentIndex(0);
    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);
    SelectPreyCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}


void
GuiControlsFoodAvailability::callback_SelectVariableChanged(QString value)
{
    bool toggle = (value == "Food Availability by Prey Age");
    SelectPreyLBL->setEnabled(toggle);
    SelectPreyCMB->setEnabled(toggle);

    SelectByVariablesCMB->setCurrentIndex(0);
    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);
    SelectPreyCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsFoodAvailability::callback_SelectByVariablesChanged(QString value)
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
GuiControlsFoodAvailability::callback_SelectSeasonChanged(QString value)
{   
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);
    SelectPreyCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsFoodAvailability::callback_SelectPredatorAgeSizeClassChanged(QString value)
{
    loadSelectPreyCMB(databasePtr,MSVPAName);

    SelectPreyCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsFoodAvailability::callback_SelectPreyChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsFoodAvailability::callback_SeasonScaleChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsFoodAvailability::callback_AgeSizeScaleChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsFoodAvailability::callback_SeasonScaleCBChanged(int toggle)
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
GuiControlsFoodAvailability::callback_AgeSizeScaleCBChanged(int toggle)
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
GuiControlsFoodAvailability::callback_HorizontalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsFoodAvailability::callback_VerticalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsFoodAvailability::callback_chartThemeChanged(int newTheme)
{
    emit UpdateTheme(newTheme);
}
