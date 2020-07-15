#include "MSVPAGuiControlsNonYieldPerRecruit.h"


MSVPAGuiControlsNonYieldPerRecruit::MSVPAGuiControlsNonYieldPerRecruit()
{
    // Create GUI widgets
    SelectByVariablesLBL          = new QLabel("Select By Variables:");
    SelectSeasonLBL               = new QLabel("Select Season:");
    SelectPredatorAgeSizeClassLBL = new QLabel("Select Predator Age/Size Class:");
    SelectPreyLBL                 = new QLabel("Select Prey:");
    SelectByVariablesCMB          = new QComboBox();
    SelectSeasonCMB               = new QComboBox();
    SelectPredatorAgeSizeClassCMB = new QComboBox();
    SelectPreyCMB                 = new QComboBox();
    SeasonLayt                    = new QHBoxLayout();
    AgeSizeLayt                   = new QHBoxLayout();
    SeasonScaleCB                 = new QCheckBox();
    AgeSizeScaleCB                = new QCheckBox();
    SeasonScaleLE                 = new QLineEdit();
    AgeSizeScaleLE                = new QLineEdit();

    // Load gui elements into main layout
    MainLayt->addWidget(SelectPredatorLBL);
    MainLayt->addWidget(SelectPredatorCMB);
    MainLayt->addWidget(SelectVariableLBL);
    MainLayt->addWidget(SelectVariableCMB);
    MainLayt->addWidget(SelectByVariablesLBL);
    MainLayt->addWidget(SelectByVariablesCMB);
    MainLayt->addWidget(SelectSeasonLBL);
    SeasonLayt->addWidget(SelectSeasonCMB);
    SeasonLayt->addWidget(SeasonScaleCB);
    SeasonLayt->addWidget(SeasonScaleLE);
    MainLayt->addLayout(SeasonLayt);
    MainLayt->addWidget(SelectPredatorAgeSizeClassLBL);
    AgeSizeLayt->addWidget(SelectPredatorAgeSizeClassCMB);
    AgeSizeLayt->addWidget(AgeSizeScaleCB);
    AgeSizeLayt->addWidget(AgeSizeScaleLE);
    MainLayt->addLayout(AgeSizeLayt);
    MainLayt->addWidget(SelectPreyLBL);
    MainLayt->addWidget(SelectPreyCMB);
    MainLayt->addWidget(SelectionModeLBL);
    MainLayt->addWidget(SelectionModeCMB);
    MainLayt->addWidget(GridLinesLBL);
    MainLayt->addLayout(GridLinesLayt);
    MainLayt->addWidget(ColorSchemesLBL);
    MainLayt->addWidget(ThemesCMB);
    MainLayt->insertStretch(-1);

    // Set up connections
    connect(SeasonScaleCB,  SIGNAL(clicked(bool)),
            this,           SLOT(callback_SeasonScaleChanged(bool)));
    connect(AgeSizeScaleCB, SIGNAL(clicked(bool)),
            this,           SLOT(callback_AgeSizeScaleChanged(bool)));

    // Set up initial states
    SelectSeasonCMB->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    SelectPredatorAgeSizeClassCMB->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    SeasonScaleLE->setFixedWidth(50);
    AgeSizeScaleLE->setFixedWidth(50);
    SeasonScaleCB->setFixedWidth(15);
    AgeSizeScaleCB->setFixedWidth(15);

    // Set tool tips
    SeasonScaleCB->setToolTip("Set vertical scale to same maximum value for all Seasons.");
    SeasonScaleCB->setStatusTip("Set vertical scale to same maximum value for all Seasons.");
    SeasonScaleLE->setToolTip("The maximum scale value for all Seasons.");
    SeasonScaleLE->setStatusTip("The maximum scale value for all Seasons.");
    AgeSizeScaleCB->setToolTip("Set vertical scale to same maximum value for all Age/Size classes.");
    AgeSizeScaleCB->setStatusTip("Set vertical scale to same maximum value for all Age/Size classes.");
    AgeSizeScaleLE->setToolTip("The maximum scale value for all Age/Size classes.");
    AgeSizeScaleLE->setStatusTip("The maximum scale value for all Age/Size classes.");

    // Set any validators
    validMaxSeasonScaleY = new QDoubleValidator(0.0,10000.0,3,SeasonScaleLE);
    validMaxSeasonScaleY->setNotation(QDoubleValidator::StandardNotation);
    SeasonScaleLE->setValidator(validMaxSeasonScaleY);
    validMaxAgeSizeScaleY = new QDoubleValidator(0.0,10000.0,3,AgeSizeScaleLE);
    validMaxAgeSizeScaleY->setNotation(QDoubleValidator::StandardNotation);
    AgeSizeScaleLE->setValidator(validMaxAgeSizeScaleY);

}


void
MSVPAGuiControlsNonYieldPerRecruit::callback_SeasonScaleChanged(bool value)
{
    SeasonScaleLE->setEnabled(value);
}

void
MSVPAGuiControlsNonYieldPerRecruit::callback_AgeSizeScaleChanged(bool value)
{
    AgeSizeScaleLE->setEnabled(value);
}


void
MSVPAGuiControlsNonYieldPerRecruit::loadSelectSeasonCMB(nmfDatabase* databasePtr,
                                                std::string MSVPAName)
{
    std::string SeasonText;
    int NumSeasons;

    SelectSeasonCMB->blockSignals(true);
    SelectSeasonCMB->clear();

    std::map<std::string,int> initMap = databasePtr->nmfQueryInitFields("MSVPAlist", MSVPAName);
    NumSeasons = initMap["NSeasons"];
    for (int i=0; i<NumSeasons; ++i) {
        SeasonText = "Season " + std::to_string(i+1);
        SelectSeasonCMB->addItem(SeasonText.c_str());
    }

    SelectSeasonCMB->blockSignals(false);
}

void
MSVPAGuiControlsNonYieldPerRecruit::loadSelectPredatorAgeSizeClassCMB(nmfDatabase* databasePtr,
                                                              std::string MSVPAName)
{
    int sizeOffset = 0;
    int NageOrSizeCategories = 0;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::string SelectedSpecies = SelectPredatorCMB->currentText().toStdString();
    std::string ageSizePrefix;

    SelectPredatorAgeSizeClassCMB->blockSignals(true);
    SelectPredatorAgeSizeClassCMB->clear();

    ageSizePrefix = "Age";
    fields = {"SpeName"};
    queryStr = "SELECT SpeName from MSVPAspecies WHERE MSVPAName='" + MSVPAName +
               "' and SpeName='" + SelectedSpecies + "' and Type=3";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["SpeName"].size() > 0) {
        if (dataMap["SpeName"][0] == SelectedSpecies) {
            ageSizePrefix = "Size";
        }
    }

    if (ageSizePrefix == "Age" ) {
        fields = {"NumAges"};
        queryStr = "SELECT count(DISTINCT(Age)) as NumAges FROM MSVPASeasBiomass WHERE MSVPAName = '" + MSVPAName +
                "' and SpeName='" + SelectPredatorCMB->currentText().toStdString() + "'";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        NageOrSizeCategories  = std::stoi(dataMap["NumAges"][0]);

    } else {
        fields = {"SpeName","NumSizeCats"};
        queryStr = "SELECT SpeName,NumSizeCats from OtherPredSpecies WHERE SpeName='" + SelectedSpecies + "'";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["SpeName"].size() > 0) {
            if (dataMap["SpeName"][0] == SelectedSpecies) {
                sizeOffset = 1;
                NageOrSizeCategories = std::stoi(dataMap["NumSizeCats"][0]);
            }
        }
    }

    std::string ageSizeText;
    for (int i=0; i<NageOrSizeCategories; ++i) {
        ageSizeText = ageSizePrefix + " " + std::to_string(i+sizeOffset);
        SelectPredatorAgeSizeClassCMB->addItem(QString::fromStdString(ageSizeText));
    }

    SelectPredatorAgeSizeClassCMB->blockSignals(false);
}

