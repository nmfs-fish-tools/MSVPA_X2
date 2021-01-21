
#include "GuiControlsDietComposition.h"

GuiControlsDietComposition::GuiControlsDietComposition()
{
    databasePtr  = NULL;
    logger       = NULL;
    MSVPAName    = "";
    ForecastName = "";
    ScenarioName = "";
    ModelName    = "";

    // Load up all other widgets with Diet Composition data
    SelectVariableCMB->addItem("Prey Type");

    SelectByVariablesCMB->addItem("Average");
    SelectByVariablesCMB->addItem("Average by Year");
    SelectByVariablesCMB->addItem("Average by Season");
    SelectByVariablesCMB->addItem("By Year and Season");

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
            this,                          SLOT(callback_SelectPredatorAgeSizeClassChanged(QString)));
    connect(HorizontalLinesCB,    SIGNAL(toggled(bool)),
            this,                 SLOT(callback_HorizontalGridLinesChanged(bool)));
    connect(VerticalLinesCB,      SIGNAL(toggled(bool)),
            this,                 SLOT(callback_VerticalGridLinesChanged(bool)));
    connect(ThemesCMB,            SIGNAL(currentIndexChanged(int)),
            this,                 SLOT(callback_chartThemeChanged(int)));

    // Set initial states
    resetWidgets();
}

GuiControlsDietComposition::~GuiControlsDietComposition()
{
}


void
GuiControlsDietComposition::resetWidgets()
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
GuiControlsDietComposition::getUpdateDataStruct()
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
    retv.HorizontalGridLines = HorizontalLinesCB->isChecked();
    retv.VerticalGridLines   = VerticalLinesCB->isChecked();
    retv.DataTypeLabel       = "Diet";
    retv.XLabel              = ""; // is calculated in code
    retv.YLabel              = "Prop. Diet";
    retv.TitlePrefix         = ""; // is calculated in code
    retv.TitleSuffix         = "";
    retv.MaxScaleY           = "";
    retv.Theme               = ThemesCMB->currentIndex();
    if (SeasonScaleCB->isChecked()) {
        retv.MaxScaleY = (seasonScale ==  ".") ? "0" : seasonScale;
    }
    else if (AgeSizeScaleCB->isChecked()) {
        retv.MaxScaleY = (ageSizeScale == ".") ? "0" : ageSizeScale;
    }

    retv.SelectMode          = "";

    return retv;
}

void
GuiControlsDietComposition::loadWidgets(nmfDatabase *theDatabasePtr,
                                        nmfLogger   *theLogger,
                                        std::string  theMSVPAName,
                                        std::string  theModelName,
                                        std::string  theForecastName,
                                        std::string  theScenarioName)
{
    databasePtr  = theDatabasePtr;
    logger       = theLogger;
    MSVPAName    = theMSVPAName;
    ForecastName = theForecastName;
    ScenarioName = theScenarioName;
    ModelName    = theModelName;

    loadSelectPredatorCMB(databasePtr,MSVPAName);
    loadSelectSeasonCMB(databasePtr,MSVPAName);
    loadSelectPredatorAgeSizeClassCMB(databasePtr,MSVPAName);

    emit UpdateChart(getUpdateDataStruct());

}

void
GuiControlsDietComposition::loadSelectPredatorCMB(nmfDatabase* databasePtr,
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
}

void
GuiControlsDietComposition::callback_SelectPredatorChanged(QString value)
{
    // Need to reload this combo box since it's a function of species
    loadSelectPredatorAgeSizeClassCMB(databasePtr,MSVPAName);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsDietComposition::callback_SelectVariableChanged(QString value)
{
    SelectByVariablesCMB->setCurrentIndex(0);
    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsDietComposition::callback_SelectByVariablesChanged(QString value)
{
    SelectPredatorAgeSizeClassLBL->setEnabled(value != "Average");
    SelectPredatorAgeSizeClassCMB->setEnabled(value != "Average");

    SelectSeasonLBL->setEnabled(value == "By Year and Season");
    SelectSeasonCMB->setEnabled(value == "By Year and Season");

    SelectSeasonCMB->setCurrentIndex(0);
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());

}

void
GuiControlsDietComposition::callback_SelectSeasonChanged(QString value)
{
    SelectPredatorAgeSizeClassCMB->setCurrentIndex(0);

    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsDietComposition::callback_SelectPredatorAgeSizeClassChanged(QString value)
{
    emit UpdateChart(getUpdateDataStruct());
}

void
GuiControlsDietComposition::callback_HorizontalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsDietComposition::callback_VerticalGridLinesChanged(bool toggled)
{
    emit UpdateChartGridLines(getUpdateDataStruct());
}

void
GuiControlsDietComposition::callback_chartThemeChanged(int newTheme)
{
    emit UpdateTheme(newTheme);
}
