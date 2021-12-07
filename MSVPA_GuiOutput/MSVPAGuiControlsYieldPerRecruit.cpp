#include "MSVPAGuiControlsYieldPerRecruit.h"

MSVPAGuiControlsYieldPerRecruit::MSVPAGuiControlsYieldPerRecruit()
{
    // Create GUI widgets
    SelectYPRAnalysisTypeLBL   = new QLabel("Select YPR Analaysis Type:");
    SelectYearsLBL             = new QLabel("Select Year(s):");
    SelectFullyRecruitedAgeLBL = new QLabel("Select Fully Recruited Age:");
    SelectYPRAnalysisTypeCMB   = new QComboBox();
    SelectFullyRecruitedAgeCMB = new QComboBox();
    SelectFullyRecruitedAgeCB  = new QCheckBox();
    SelectFullyRecruitedAgeLE  = new QLineEdit();
    SelectYearsLW              = new QListWidget();
    FullyRecruitedLayt         = new QHBoxLayout();

    // Load GUI elements into main layout
    MainLayt->addWidget(SelectPredatorLBL);
    MainLayt->addWidget(SelectPredatorCMB);
    MainLayt->addWidget(SelectYPRAnalysisTypeLBL);
    MainLayt->addWidget(SelectYPRAnalysisTypeCMB);
    MainLayt->addWidget(SelectYearsLBL);
    MainLayt->addWidget(SelectYearsLW);

    MainLayt->addWidget(SelectFullyRecruitedAgeLBL);
    FullyRecruitedLayt->addWidget(SelectFullyRecruitedAgeCMB);
    FullyRecruitedLayt->addWidget(SelectFullyRecruitedAgeCB);
    FullyRecruitedLayt->addWidget(SelectFullyRecruitedAgeLE);
    MainLayt->addLayout(FullyRecruitedLayt);
    MainLayt->addWidget(GridLinesLBL);
    MainLayt->addLayout(GridLinesLayt);
    MainLayt->addWidget(ColorSchemesLBL);
    MainLayt->addWidget(ThemesCMB);
    MainLayt->insertStretch(-1);

    // Set default states
    SelectFullyRecruitedAgeLE->setFixedWidth(50);
    SelectFullyRecruitedAgeLE->setEnabled(false);
    SelectFullyRecruitedAgeCB->setFixedWidth(18);
    SelectYearsLW->setSelectionMode(QAbstractItemView::ExtendedSelection);
    SelectFullyRecruitedAgeCB->setToolTip("Set vertical scale to same maximum value for all Fully Recruited Age classes.");
    SelectFullyRecruitedAgeCB->setStatusTip("Set vertical scale to same maximum value for all Fully Recruited Age classes.");
    SelectFullyRecruitedAgeLE->setToolTip("The maximum scale value for all Fully Recruited Age classes.");
    SelectFullyRecruitedAgeLE->setStatusTip("The maximum scale value for all Fully Recruited Age classes.");

    // Set up connections
    connect(SelectFullyRecruitedAgeCB, SIGNAL(clicked(bool)),
            this,                      SLOT(callback_FullyRecruitedScaleChanged(bool)));

}



void
MSVPAGuiControlsYieldPerRecruit::loadSelectFullyRecruitedAgeCMB(nmfDatabase* databasePtr,
                                                                std::string MSVPAName)
{
    int sizeOffset = 0;
    int NageOrSizeCategories = 0;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::string SelectedSpecies = SelectPredatorCMB->currentText().toStdString();
    std::string ageSizePrefix;

    SelectFullyRecruitedAgeCMB->blockSignals(true);
    SelectFullyRecruitedAgeCMB->clear();

    ageSizePrefix = "Age";
    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAName='" + MSVPAName +
               "' and SpeName='" + SelectedSpecies + "' and Type=3";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["SpeName"].size() > 0) {
        if (dataMap["SpeName"][0] == SelectedSpecies) {
            ageSizePrefix = "Size";
        }
    }

    if (ageSizePrefix == "Age" ) {
        fields = {"NumAges"};
        queryStr = "SELECT count(DISTINCT(Age)) as NumAges FROM " +
                    nmfConstantsMSVPA::TableMSVPASeasBiomass +
                   " WHERE MSVPAName = '" + MSVPAName +
                   "' and SpeName='" + SelectPredatorCMB->currentText().toStdString() + "'";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        NageOrSizeCategories  = std::stoi(dataMap["NumAges"][0]);

    } else {
        fields = {"SpeName","NumSizeCats"};
        queryStr = "SELECT SpeName,NumSizeCats FROM " + nmfConstantsMSVPA::TableOtherPredSpecies +
                   " WHERE SpeName='" + SelectedSpecies + "'";
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
        SelectFullyRecruitedAgeCMB->addItem(ageSizeText.c_str());
    }

    SelectFullyRecruitedAgeCMB->blockSignals(false);
}




void
MSVPAGuiControlsYieldPerRecruit::callback_FullyRecruitedScaleChanged(bool value)
{
    SelectFullyRecruitedAgeLE->setEnabled(value);
}



void
MSVPAGuiControlsYieldPerRecruit::getSelectedYears(
        QListWidget *selectYearsLW,
        std::vector<std::string> &selectedYears)
{
    QList<QListWidgetItem *> list = selectYearsLW->selectedItems();
    QList<QListWidgetItem *>::iterator it;

    for (it = list.begin(); it != list.end(); ++it)
    {
        selectedYears.push_back( (*it)->text().toStdString()  );
    }
    std::sort(selectedYears.begin(),selectedYears.end());
}




