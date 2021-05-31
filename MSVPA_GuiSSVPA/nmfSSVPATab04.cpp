
#include "nmfConstants.h"
#include "nmfUtilsQt.h"
#include "nmfUtils.h"

#include "nmfSSVPATab04.h"


nmfSSVPATab4::nmfSSVPATab4(QTabWidget  *tabs,
                           nmfLogger   *theLogger,
                           std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::nmfSSVPATab4");

    SSVPA_Tabs = tabs;
    databasePtr = NULL;
    mortality_model = NULL;
    for (int i=0;i<nmfConstants::MaxNumberSpecies;++i)
        maturityModel[i] = nullptr;
    for (int i=0;i<nmfConstants::MaxNumberFleets;++i)
        fleetModel[i] = nullptr;
    for (int i=0;i<nmfConstants::MaxNumberIndices;++i)
        xsaModel[i] = nullptr;
    okToRun = true;
    Alpha.clear();
    Beta.clear();
    CurrentTable.clear();
    mortalitySliderScale = 0;
    selectivitySliderScale = 0;
    AllSpecies.clear();
    maturitySliderScale = 0.0;
    ProjectDir = theProjectDir;
    SpeciesIndex = 0;

    // Load ui as a widget from disk
    QFile file(":/forms/SSVPA/SSVPA_Tab04.ui");
    file.open(QFile::ReadOnly);
    SSVPA_Tab4_Widget = loader.load(&file,SSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    SSVPA_Tabs->addTab(SSVPA_Tab4_Widget, tr("4. Config Data"));


    //
    //
    // Move some callbacks from MainWindow to here

    SSVPAMaturityTV                = SSVPA_Tabs->findChild<QTableView *>("SSVPAMaturityTV");
    SSVPAMortalityTV               = SSVPA_Tabs->findChild<QTableView *>("SSVPAMortalityTV");
    SSVPAInitialSelectivityTV      = SSVPA_Tabs->findChild<QTableView *>("SSVPAInitialSelectivityTV");
    SSVPAFleetDataTV               = SSVPA_Tabs->findChild<QTableView *>("SSVPAFleetDataTV");
    SSVPAIndicesTV                 = SSVPA_Tabs->findChild<QTableView *>("SSVPAIndicesTV");
    ResidualNaturalMortalityDSB    = SSVPA_Tabs->findChild<QDoubleSpinBox *>("ResidualNaturalMortalityDSB");
    ResidualNaturalMortalityLBL    = SSVPA_Tabs->findChild<QLabel *>("ResidualNaturalMortalityLBL");
    PredationMortalityDSB          = SSVPA_Tabs->findChild<QDoubleSpinBox *>("PredationMortalityDSB");
    PredationMortalityLBL          = SSVPA_Tabs->findChild<QLabel *>("PredationMortalityLBL");
    LastYearMortalityRateDSB       = SSVPA_Tabs->findChild<QDoubleSpinBox *>("LastYearMortalityRateDSB");
    ReferenceAgeSB                 = SSVPA_Tabs->findChild<QSpinBox *>("ReferenceAgeSB");
    SelectConfigurationCMB         = SSVPA_Tabs->findChild<QComboBox *>("SelectConfigurationCMB");
    Configure_Cohort_SB1           = SSVPA_Tabs->findChild<QSpinBox *>("Configure_Cohort_SB1");
    Configure_Cohort_SB2           = SSVPA_Tabs->findChild<QSpinBox *>("Configure_Cohort_SB2");
    Configure_Cohort_SB3           = SSVPA_Tabs->findChild<QSpinBox *>("Configure_Cohort_SB3");
    Configure_Cohort_SB4           = SSVPA_Tabs->findChild<QSpinBox *>("Configure_Cohort_SB4");
    Configure_Cohort_LB1           = SSVPA_Tabs->findChild<QLabel *>("Configure_Cohort_LB1");
    Configure_Cohort_LB2           = SSVPA_Tabs->findChild<QLabel *>("Configure_Cohort_LB2");
    Configure_Cohort_LB3           = SSVPA_Tabs->findChild<QLabel *>("Configure_Cohort_LB3");
    Configure_Cohort_LB4           = SSVPA_Tabs->findChild<QLabel *>("Configure_Cohort_LB4");
    Configure_EffortTuned_SetNumFleetsCMB = SSVPA_Tabs->findChild<QComboBox *>("Configure_EffortTuned_SetNumFleetsCMB");
    Configure_EffortTuned_SelectAFleetCMB = SSVPA_Tabs->findChild<QComboBox *>("Configure_EffortTuned_SelectAFleetCMB");
    Configure_EffortTuned_LB3      = SSVPA_Tabs->findChild<QLabel *>("Configure_EffortTuned_LB3");
    Configure_EffortTuned_SB1      = SSVPA_Tabs->findChild<QSpinBox *>("Configure_EffortTuned_SB1");
    Configure_EffortTuned_LB1      = SSVPA_Tabs->findChild<QLabel *>("Configure_EffortTuned_LB1");
    SSVPAExtendedIndicesCMB        = SSVPA_Tabs->findChild<QComboBox *>("SSVPAExtendedIndicesCMB");
    SSVPAExtendedStartTimeLE       = SSVPA_Tabs->findChild<QLineEdit *>("SSVPAExtendedStartTimeLE");
    SSVPAExtendedEndTimeLE         = SSVPA_Tabs->findChild<QLineEdit *>("SSVPAExtendedEndTimeLE");
    Configure_Shrinkage_GB         = SSVPA_Tabs->findChild<QGroupBox *>("Configure_XSA_GB");
    Configure_Downweight_GB        = SSVPA_Tabs->findChild<QGroupBox *>("Configure_Downweight_GB");
    Configure_XSA_LE1              = SSVPA_Tabs->findChild<QLineEdit *>("Configure_XSA_LE1");
    Configure_XSA_NYears_SB        = SSVPA_Tabs->findChild<QSpinBox *>("Configure_XSA_NYears_SB");
    Configure_XSA_NAges_SB         = SSVPA_Tabs->findChild<QSpinBox *>("Configure_XSA_NAges_SB");
    Configure_XSA_CMB4             = SSVPA_Tabs->findChild<QComboBox *>("Configure_XSA_CMB4");
    Configure_XSA_CMB5             = SSVPA_Tabs->findChild<QComboBox *>("Configure_XSA_CMB5");
    SSVPA_Tab4_PrevPB              = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_PrevPB");
    SSVPA_Tab4_CheckAllPB          = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_CheckAllPB");
    SSVPA_Tab4_LoadMortalityPB     = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_LoadMortalityPB");
    SSVPA_Tab4_SaveMortalityPB     = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_SaveMortalityPB");
    //SSVPA_Tab4_LoadConfigPB      = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_LoadConfigPB");
    SSVPA_Tab4_SavePB              = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_SavePB");
    SSVPA_Tab4_LoadFleetPB         = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_LoadFleetPB");
    SSVPA_Tab4_SaveFleetPB         = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_SaveFleetPB");
    SSVPA_Tab4_LoadIndicesPB       = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_LoadIndicesPB");
    SSVPA_Tab4_SaveIndicesPB       = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_SaveIndicesPB");
    SSVPA_Tab4_LoadMaturityPB      = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_LoadMaturityPB");
    SSVPA_Tab4_SaveMaturityPB      = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_SaveMaturityPB");
    SSVPAMortalityRatesGB          = SSVPA_Tabs->findChild<QGroupBox *>("SSVPAMortalityRatesGB");
    SSVPASelectConfigurationAndTypeGB = SSVPA_Tabs->findChild<QGroupBox *>("SSVPASelectConfigurationAndTypeGB");
    SSVPAConfigure1GB              = SSVPA_Tabs->findChild<QGroupBox *>("SSVPAConfigure1GB");
    SSVPAConfigure2GB              = SSVPA_Tabs->findChild<QGroupBox *>("SSVPAConfigure2GB");
    SSVPAConfigure3GB              = SSVPA_Tabs->findChild<QGroupBox *>("SSVPAConfigure3GB");
    SSVPAConfigure4GB              = SSVPA_Tabs->findChild<QGroupBox *>("SSVPAConfigure4GB");
    SelectVPATypeCMB               = SSVPA_Tabs->findChild<QComboBox *>("SelectVPATypeCMB");
    Configure_XSA_AddIndexPB       = SSVPA_Tabs->findChild<QPushButton *>("Configure_XSA_AddIndexPB");
    Configure_XSA_DelIndexPB       = SSVPA_Tabs->findChild<QPushButton *>("Configure_XSA_DelIndexPB");
    SSVPAMaturityRatesGB           = SSVPA_Tabs->findChild<QGroupBox *>("SSVPAMaturityRatesGB");
    SSVPAMaturitySL                = SSVPA_Tabs->findChild<QSlider     *>("SSVPAMaturitySL");
    SSVPAMaturityMinLE             = SSVPA_Tabs->findChild<QLineEdit   *>("SSVPAMaturityMinLE");
    SSVPAMaturityMaxLE             = SSVPA_Tabs->findChild<QLineEdit   *>("SSVPAMaturityMaxLE");
    NaturalMortalitySL             = SSVPA_Tabs->findChild<QSlider     *>("NaturalMortalitySL");
    MortalityMinLE                 = SSVPA_Tabs->findChild<QLineEdit   *>("MortalityMinLE");
    MortalityMaxLE                 = SSVPA_Tabs->findChild<QLineEdit   *>("MortalityMaxLE");
    NaturalMortalityCB             = SSVPA_Tabs->findChild<QCheckBox   *>("NaturalMortalityCB");
    SelectivityMinLE               = SSVPA_Tabs->findChild<QLineEdit   *>("SelectivityMinLE");
    SelectivityMaxLE               = SSVPA_Tabs->findChild<QLineEdit   *>("SelectivityMaxLE");
    AddConfiguration               = SSVPA_Tabs->findChild<QPushButton *>("AddConfiguration");
    DelConfiguration               = SSVPA_Tabs->findChild<QPushButton *>("DelConfiguration");
    SSVPA_Tab4_RunPB               = SSVPA_Tabs->findChild<QPushButton *>("SSVPA_Tab4_RunPB");
    SelectivitySL                  = SSVPA_Tabs->findChild<QSlider     *>("SelectivitySL");

    // Load SSVPA Configuration and Type widgets
    for (auto vpaType : nmfConstantsMSVPA::DefaultVPATypes) {
        SelectVPATypeCMB->addItem(QString::fromStdString(vpaType));
    }

    SSVPAMortalityTV->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    MortalityMaxLE->setText(QString::number(nmfConstants::MortalityInit, 'f', 1));
    MortalityMinLE->setText(QString::number(nmfConstants::MortalityMin,  'f', 1));

    // Set default widget values
    ResidualNaturalMortalityDSB->setMaximum(nmfConstants::MortalityMax);
    PredationMortalityDSB->setMaximum(nmfConstants::MortalityMax);
    ResidualNaturalMortalityDSB->setValue(
            nmfConstants::DefaultResidualNaturalMortality);
    PredationMortalityDSB->setValue(nmfConstants::DefaultPredationMortality);


//    smodelXSA    = new QStandardItemModel(nmfConstants::MaxNumberYears,
//                                          nmfConstants::MaxNumberAges);

    //SSVPAExtendedIndicesCMB->blockSignals(true);

    connect(AddConfiguration,           SIGNAL(clicked(bool)),
            this,                       SLOT(callback_SSVPA_Tab4_AddConfigPB(bool)));
    connect(DelConfiguration,           SIGNAL(clicked(bool)),
            this,                       SLOT(callback_SSVPA_Tab4_DelConfigPB(bool)));

    connect(SSVPAExtendedIndicesCMB,    SIGNAL(currentIndexChanged(int)),
            this,                       SLOT(callback_XSA_IndexCMB(int)));
    connect(SelectVPATypeCMB,           SIGNAL(currentIndexChanged(int)),
            this,                       SLOT(callback_SelectVPATypeCMB(int)));

    connect(Configure_Cohort_SB1,       SIGNAL(valueChanged(int)),
            this,                       SLOT(callback_Cohort_SB1(int)));
    connect(Configure_Cohort_SB2,       SIGNAL(valueChanged(int)),
            this,                       SLOT(callback_Cohort_SB2(int)));
    connect(Configure_Cohort_SB3,       SIGNAL(valueChanged(int)),
            this,                       SLOT(callback_Cohort_SB3(int)));
    connect(Configure_Cohort_SB4,       SIGNAL(valueChanged(int)),
            this,                       SLOT(callback_Cohort_SB4(int)));

    connect(Configure_EffortTuned_SB1,  SIGNAL(valueChanged(int)),
            this,                       SLOT(callback_EffortTuned_SB1(int)));
    connect(Configure_EffortTuned_SetNumFleetsCMB, SIGNAL(currentIndexChanged(int)),
            this,                       SLOT(callback_EffortTuned_SetNumFleetsCMB(int)));
    connect(Configure_EffortTuned_SelectAFleetCMB, SIGNAL(currentIndexChanged(int)),
            this,                       SLOT(callback_EffortTuned_SelectAFleetCMB(int)));
    connect(SSVPAExtendedStartTimeLE,   SIGNAL(editingFinished()),
            this,                       SLOT(callback_XSA_StartTimeLE()));
    connect(SSVPAExtendedEndTimeLE,     SIGNAL(editingFinished()),
            this,                       SLOT(callback_XSA_EndTimeLE()));
    connect(Configure_XSA_LE1,          SIGNAL(editingFinished()),
            this,                       SLOT(callback_Configure_XSA_LE1()));
    connect(Configure_XSA_NYears_SB,    SIGNAL(valueChanged(int)),
            this,                       SLOT(callback_Configure_XSA_NYears_SB(int)));
    connect(Configure_XSA_NAges_SB,     SIGNAL(valueChanged(int)),
            this,                       SLOT(callback_Configure_XSA_NAges_SB(int)));

    connect(Configure_XSA_CMB4,         SIGNAL(currentIndexChanged(int)),
            this,                       SLOT(callback_Configure_XSA_CMB4(int)));
    connect(Configure_XSA_CMB5,         SIGNAL(currentIndexChanged(QString)),
            this,                       SLOT(callback_Configure_XSA_CMB5(QString)));
    connect(Configure_Downweight_GB,    SIGNAL(clicked(bool)),
            this,                       SLOT(callback_Configure_Downweight_GB(bool)));
    connect(Configure_Shrinkage_GB,     SIGNAL(clicked(bool)),
            this,                       SLOT(callback_Configure_Shrinkage_GB(bool)));

    connect(LastYearMortalityRateDSB,   SIGNAL(valueChanged(double)),
            this,                       SLOT(callback_TermFChanged(double)));
    connect(ReferenceAgeSB,             SIGNAL(valueChanged(int)),
            this,                       SLOT(callback_RefAgeChanged(int)));

    connect(NaturalMortalitySL,         SIGNAL(valueChanged(int)),
            this,                       SLOT(callback_MortalitySliderMoved(int)));
    connect(NaturalMortalitySL,         SIGNAL(sliderPressed()),
            this,                       SLOT(callback_MortalitySliderPressed()));
    connect(NaturalMortalitySL,         SIGNAL(sliderReleased()),
            this,                       SLOT(callback_MortalitySliderReleased()));
    connect(MortalityMaxLE,             SIGNAL(editingFinished()),
            this,                       SLOT(callback_MortalityMaxEdited()));
    connect(MortalityMinLE,             SIGNAL(editingFinished()),
            this,                       SLOT(callback_MortalityMinEdited()));
    connect(NaturalMortalityCB,         SIGNAL(stateChanged(int)),
            this,                       SLOT(callback_MortalityChecked(int)));
    connect(SelectConfigurationCMB,     SIGNAL(currentIndexChanged(QString)),
            this,                       SLOT(callback_SelectConfigurationCMB(QString)));
    connect(SSVPAMortalityTV,           SIGNAL(clicked(QModelIndex)),
             this,                      SLOT(callback_SSVPAMortalityTV(QModelIndex)));

    connect(SelectivitySL,              SIGNAL(valueChanged(int)),
            this,                       SLOT(callback_SelectivitySliderMoved(int)));
    connect(SelectivitySL,              SIGNAL(sliderPressed()),
            this,                       SLOT(callback_SelectivitySliderPressed()));
    connect(SelectivitySL,              SIGNAL(sliderReleased()),
            this,                       SLOT(callback_SelectivitySliderReleased()));

//    connect(SSVPAInitialSelectivityTV->model(), SIGNAL(dataChanged(QModelIndex, QModelIndex)),
//            this,                               SLOT(callback_InitialSelectivityChanged(QModelIndex,QModelIndex)));

//    connect(SSVPAMortalityTV->model(),  SIGNAL(dataChanged(QModelIndex, QModelIndex)),
//            this,                       SLOT(callback_updateModel(QModelIndex, QModelIndex)));
    connect(ResidualNaturalMortalityDSB, SIGNAL(valueChanged(double)),
            this,                       SLOT(callback_ResidualMortalityChanged(double)));
    connect(PredationMortalityDSB,       SIGNAL(valueChanged(double)),
            this,                       SLOT(callback_PredationMortalityChanged(double)));

    connect(SSVPA_Tab4_LoadFleetPB,     SIGNAL(clicked(bool)),
            this,                       SLOT(callback_SSVPA_Tab4_LoadFleetPB(bool)));
    connect(SSVPA_Tab4_LoadIndicesPB,   SIGNAL(clicked(bool)),
            this,                       SLOT(callback_SSVPA_Tab4_LoadIndicesPB(bool)));
    connect(SSVPA_Tab4_LoadMaturityPB,  SIGNAL(clicked(bool)),
            this,                       SLOT(callback_SSVPA_Tab4_LoadMaturityPB(bool)));
    connect(SSVPA_Tab4_LoadMortalityPB, SIGNAL(clicked(bool)),
            this,                       SLOT(callback_SSVPA_Tab4_LoadMortalityPB(bool)));
    connect(SSVPA_Tab4_SavePB,          SIGNAL(clicked(bool)),
            this,                       SLOT(callback_SSVPA_Tab4_SavePB(bool)));
    connect(SSVPA_Tab4_PrevPB,          SIGNAL(clicked(bool)),
            this,                       SLOT(callback_SSVPA_Tab4_PrevPB(bool)));
    connect(SSVPA_Tab4_CheckAllPB,      SIGNAL(clicked(bool)),
            this,                       SLOT(callback_SSVPA_Tab4_CheckAllPB(bool)));
    connect(SSVPA_Tab4_RunPB,           SIGNAL(clicked(bool)),
            this,                       SLOT(callback_SSVPA_Tab4_RunPB(bool)));


    connect(Configure_XSA_AddIndexPB,   SIGNAL(clicked(bool)),
            this,                       SLOT(callback_SSVPA_Tab4_AddIndexPB(bool)));
    connect(Configure_XSA_DelIndexPB,   SIGNAL(clicked(bool)),
            this,                       SLOT(callback_SSVPA_Tab4_DelIndexPB(bool)));


    connect(SSVPAMaturitySL,      SIGNAL(valueChanged(int)),
            this,                 SLOT(callback_SSVPAMaturitySliderMoved(int)));
    connect(SSVPAMaturitySL,      SIGNAL(sliderPressed()),
            this,                 SLOT(callback_SSVPAMaturitySliderPressed()));
    connect(SSVPAMaturitySL,      SIGNAL(sliderReleased()),
            this,                 SLOT(callback_SSVPAMaturitySliderReleased()));


    // Hide all unused Save buttons
    SSVPA_Tab4_SaveIndicesPB->hide();
    SSVPA_Tab4_SaveMaturityPB->hide();
    SSVPA_Tab4_SaveFleetPB->hide();
    SSVPA_Tab4_SaveMortalityPB->hide();

    //SSVPAExtendedIndicesCMB->blockSignals(false);

    SSVPA_Tab4_LoadMortalityPB->setEnabled(true);
    SSVPA_Tab4_SaveMortalityPB->setEnabled(true);
    //SSVPA_Tab4_LoadConfigPB->setEnabled(true);
    SSVPA_Tab4_SavePB->setEnabled(true);

    SSVPA_Tab4_LoadFleetPB->hide();
    //SSVPA_Tab4_SaveFleetPB->hide();
    SSVPA_Tab4_LoadIndicesPB->hide();
    //SSVPA_Tab4_SaveIndicesPB->hide();
    //SSVPA_Tab4_LoadConfigPB->hide();

    // Disable until user enters at least one index
    SSVPAExtendedStartTimeLE->setEnabled(false);
    SSVPAExtendedEndTimeLE->setEnabled(false);

    showVPAGroupBox(0);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::nmfSSVPATab4 Complete");


} // end nmfSSVPATab4


nmfSSVPATab4::~nmfSSVPATab4()
{
    // Free model memory
    //smodelEffort->clear();
    //smodelXSA->clear();
    for (int i=0; i<nmfConstantsMSVPA::MaxNumSmodels; ++i) {
        if (maturityModel[i])
            maturityModel[i]->clear();
    }
}



void
nmfSSVPATab4::callback_SelectivitySliderMoved(int value) {

    QModelIndexList selection =
            SSVPAInitialSelectivityTV->selectionModel()->selectedIndexes();

    float normalizedValue = SelectivityMinLE->text().toDouble()
            + selectivitySliderScale * (value / 100.0);
    foreach (QModelIndex index, selection) {
        //qDebug() << index.row();
        SSVPAInitialSelectivityTV->model()->setData(index,normalizedValue);
    }

    emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());

}

/*
 * Disconnect the model's signal here so that there won't be a double dataChanged
 * event when the user uses the slider.
 */
void
nmfSSVPATab4::callback_SelectivitySliderPressed() {
    selectivitySliderScale = SelectivityMaxLE->text().toDouble()
            - SelectivityMinLE->text().toDouble();

    QObject::disconnect(SSVPAInitialSelectivityTV->model(),0,0,0);
            //SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
            //SLOT(updateModel(QModelIndex, QModelIndex)));
}

/*
 * Reconnect the model's signal here so that the user will be able to edit by double
 * clicking in a cell.
 */
void
nmfSSVPATab4::callback_SelectivitySliderReleased() {



    // RSK fix this...



    //    QObject::connect(SSVPAInitialSelectivityTV->model(),
//            SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
//            SLOT(updateModel(QModelIndex, QModelIndex)));
}





void
nmfSSVPATab4::callback_Configure_XSA_CMB4(int Type)
{

    if (SelectVPATypeCMB->count() != 0) {
        emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());
    }

    MarkAsDirty("SpeSSVPA");

} // end callback_Configure_XSA_CMB4

void
nmfSSVPATab4::callback_Configure_XSA_CMB5(QString Year)
{

    if ((! Year.isEmpty()) && (SelectVPATypeCMB->count() != 0)) {
        emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());
    }

    MarkAsDirty("SpeSSVPA");

} // end callback_Configure_XSA_CMB5



void
nmfSSVPATab4::callback_Configure_Downweight_GB(bool checked)
{
    if (SelectVPATypeCMB->count() != 0) {
        emit RunSSVPA(""); //SelectVPATypeCMB->currentText().toStdString());
    }

    MarkAsDirty("SpeSSVPA");
} // end callback_Configure_Downweight_GB


void
nmfSSVPATab4::callback_Configure_Shrinkage_GB(bool checked)
{

    if (SelectVPATypeCMB->count() != 0) {
        emit RunSSVPA(""); // SelectVPATypeCMB->currentText().toStdString());
    }
    MarkAsDirty("SpeSSVPA");

} // end callback_Configure_XSA_GB


void
nmfSSVPATab4::callback_SSVPA_Tab4_LoadMaturityPB(bool unused)
{
    std::string Species;
    QString line;
    QModelIndex index;
    QStringList fields;
    QString csvSpecies;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(SSVPA_Tabs);
    QStringList NameFilters;
    std::string SpeciesName;
    int SpeciesIndex;

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::callback_SSVPA_Tab4_LoadMaturityPB");

    //Species = SSVPAMaturityRatesGB->title().split(":")[1].trimmed().toStdString();

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile("SpeMaturity.csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load Species Maturity CSV File");
    if (fileDlg.exec()) {

        // Open the file here....
        SpeMaturityCSVFile = fileDlg.selectedFiles()[0];
        QFile file(SpeMaturityCSVFile);
        if (! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(SSVPA_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);
        int modelRow = 0;
        int csvFileRow = 0;
        int row = 0;
        int col = 0;
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header
                fields = line.split(",");
                csvSpecies = fields[0].trimmed();
                if (csvSpecies == QString::fromStdString(SpeciesName)) {
                    if (isFixedMaturity) {
                        index = maturityModel[SpeciesIndex]->index(modelRow,0);
                        maturityModel[SpeciesIndex]->setData(index,fields[3].trimmed());
                        ++modelRow;
                    } else {
                        index = maturityModel[SpeciesIndex]->index(row,col);
                        maturityModel[SpeciesIndex]->setData(index,fields[3].trimmed());
                        ++col;
                        if (col > MaxCatchAge) {
                            col = 0;
                            ++row;
                        }
                    }
                }
            }
            ++csvFileRow;
        }
        file.close();
        //SSVPA_Tab4_SaveAllPB->setEnabled(true);
        //SSVPA_Tab4_NextPB->setEnabled(false);
    }

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::callback_SSVPA_Tab4_LoadMaturityPB Complete");

} // end callback_SSVPA_Tab4_LoadMaturityPB


QString
nmfSSVPATab4::Save_SpeMaturity()
{
    bool SpeciesIsFixedMaturity;
    int numCols;
    std::string cmd;
    std::string errorMsg;
    std::string SpeciesName;
    QString TableName = "SpeMaturity";
    QString pmature;
    QModelIndex index;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_SpeMaturity");

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    // Remove any current records that have the same Species name, number, etc. Can't remove
    // all the records since that will remove other Species' records as well.
    cmd = "DELETE FROM " + TableName.toStdString() +
            " WHERE SpeIndex = "  + std::to_string(SpeciesIndex) +
            " AND SpeName  = '" + SpeciesName + "'";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_SpeMaturity: Delete error: ", errorMsg);
        return "\nMaturity Data:     Failed. Check data entered.";
    }

    // Get some basic info that you'll need later on
    fields   = {"SpeIndex", "SpeName","MaxAge","MinCatAge","MaxCatAge","FirstYear","LastYear"};
    queryStr = "SELECT SpeIndex,SpeName,MaxAge,MinCatAge,MaxCatAge,FirstYear,lastYear FROM Species WHERE SpeIndex = " +
            std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    int MaxAge    = std::stoi(dataMap["MaxAge"][0]);
    int FirstYear = std::stoi(dataMap["FirstYear"][0]);
    int LastYear  = std::stoi(dataMap["LastYear"][0]);

    // Write the contents from the model into the MySQL table
    cmd  = "REPLACE INTO " + TableName.toStdString();
    cmd += " (SpeIndex,SpeName,Year,Age,PMature) VALUES ";

    //int numRows = maturityModel[SpeciesIndex]->rowCount();
    numCols = maturityModel[SpeciesIndex]->columnCount();
    SpeciesIsFixedMaturity = (numCols == 1);
    for (int year=FirstYear; year<=LastYear; ++year) {
        for (int age=0; age<=MaxAge; ++age) {
            if (SpeciesIsFixedMaturity) {
                index = maturityModel[SpeciesIndex]->index(age,0);
            } else {
                index = maturityModel[SpeciesIndex]->index(year-FirstYear,age);
            }
            pmature = index.data().toString().trimmed();
            cmd += "(" + std::to_string(SpeciesIndex) + ",";
            cmd += "\"" + SpeciesName                 + "\",";
            cmd +=  std::to_string(year)              + ",";
            cmd +=  std::to_string(age)               + ",";
            cmd +=  pmature.toStdString()             + "), ";
        }
    }

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_SpeMaturity: Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        return "\nMaturity Data:     Failed. Check data entered.";
    }

    // Last Restore CSV file
    fields = {"SpeIndex","SpeName","Year","Age","PMature"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_SpeMaturity Complete");

    return "\nMaturity Data:     OK";

} // end Save_SpeMaturity


/*
QString
nmfSSVPATab4::Save_SpeMaturityOLD()
{
    bool FirstLine = true;
    QString fileName;
    QString filePath;
    QString fileNameWithPath;
    QString tmpFileNameWithPath;
    QString qcmd;
    QString line;
    std::string Species;
    QStringList qfields;
    std::string cmd;
    std::string errorMsg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString TableName = "SpeMaturity";
    int SpeciesIndex;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_Maturity");

    if (! nmfQtUtils::allCellsArePopulated(SSVPA_Tabs,SSVPAMaturityTV,
                                           nmfConstants::ShowError))
        return "\nMaturity Data:     Failed. Check data entered.";

    // Build a Species name and index map, you'll need this.
    std::map<std::string,int> SpeciesIndexMap;
    std::map<std::string,int> SpeciesMinCatchAgeMap;
    std::map<std::string,int> SpeciesMaxCatchAgeMap;
    std::map<std::string,int> SpeciesFirstYearMap;
    std::map<std::string,int> SpeciesLastYearMap;
    std::map<std::string,int> SpeciesFixedMaturityMap;
    fields   = {"SpeIndex", "SpeName", "MinCatAge","MaxCatAge","FirstYear","LastYear","FixedMaturity"};
    queryStr = "SELECT SpeIndex,SpeName,MinCatAge,MaxCatAge,FirstYear,LastYear,FixedMaturity FROM Species";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    for (int i=0; i<dataMap["SpeIndex"].size(); ++i) {
        SpeciesIndexMap[dataMap["SpeName"][i]]         = std::stoi(dataMap["SpeIndex"][i]);
        SpeciesMinCatchAgeMap[dataMap["SpeName"][i]]   = std::stoi(dataMap["MinCatAge"][i]);
        SpeciesMaxCatchAgeMap[dataMap["SpeName"][i]]   = std::stoi(dataMap["MaxCatAge"][i]);
        SpeciesFirstYearMap[dataMap["SpeName"][i]]     = std::stoi(dataMap["FirstYear"][i]);
        SpeciesLastYearMap[dataMap["SpeName"][i]]      = std::stoi(dataMap["LastYear"][i]);
        SpeciesFixedMaturityMap[dataMap["SpeName"][i]] = std::stoi(dataMap["FixedMaturity"][i]);
    }

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (SpeMaturityCSVFile.isEmpty()) {
        SpeMaturityCSVFile = "SpeMaturity.csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPath    = QDir(filePath).filePath(SpeMaturityCSVFile);
        tmpFileNameWithPath = QDir(filePath).filePath("."+SpeMaturityCSVFile);
    } else {
        QFileInfo finfo(SpeMaturityCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPath    = SpeMaturityCSVFile;
        tmpFileNameWithPath = QDir(filePath).filePath("."+fileName);
    }
    SpeMaturityCSVFile.clear();

    // Read entire file and copy all lines that don't have the current Species into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    QFile fin(fileNameWithPath);
    QFile fout(tmpFileNameWithPath);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error Save_Maturity: Opening Input File", fin.errorString());
        return "\nMaturity Data:     Failed. Check data entered.";
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error Save_Maturity: Opening Output File", fout.errorString());
        return "\nMaturity Data:     Failed. Check data entered.";
    }
    QTextStream inStream(&fin);
    QTextStream outStream(&fout);
    QString tableLine;
    QString vLabel;
    QModelIndex index;
    int Age;
    int SpeciesIsFixedMaturity;
    bool done = false;
    bool isFirstLine = true;
    QString qval;
    bool notifyUser=false;

    // We're going to save a bit differently than in previous tabs, since
    // we're only looking at the first year's worth of data.  The original code uses
    // the first year's data for every year's data.
    while ((! inStream.atEnd()) && (! done)) {
        line = inStream.readLine();
        qfields = line.split(",");
        if (isFirstLine) {
            outStream << line + "\n";
            isFirstLine = false;
        } else {
            for (unsigned int n=0; n<AllSpecies.size(); ++n) {
                Species        = AllSpecies[n].toStdString();
                SpeciesIndex   = SpeciesIndexMap[Species];
                SpeciesIsFixedMaturity = SpeciesFixedMaturityMap[Species];
                for (int Year = SpeciesFirstYearMap[Species]; Year <= SpeciesLastYearMap[Species]; ++Year)
                {
                    for (int i = 0;i <= SpeciesMaxCatchAgeMap[Species]; ++i)
                    {
                        if (maturityModel[SpeciesIndex] != NULL) {
                            if (SpeciesIsFixedMaturity) {
                                vLabel = maturityModel[SpeciesIndex]->verticalHeaderItem(i)->text();
                                Age    = vLabel.split(" ")[1].toInt();
                                index = maturityModel[SpeciesIndex]->index(i,0);
                            } else {
                                Age = i;
                                index = maturityModel[SpeciesIndex]->index(Year-FirstYear,i);
                            }
                            tableLine = QString::fromStdString(Species) + ", " +
                                        QString::number(Year) + ", " +
                                        QString::number(Age) + ", ";
                            qval = index.data().toString().trimmed();
                            if (! qval.isEmpty()) {
                                tableLine += qval;
                                outStream << tableLine << "\n";
                            }
                        } else {
                            notifyUser = true;
                        }
                    }
                }
                done = true;
            } // end for
        }
    } // end while

    if (! done) {
        Species        = AllSpecies[0].toStdString();
        SpeciesIndex   = SpeciesIndexMap[Species];
        SpeciesIsFixedMaturity = SpeciesFixedMaturityMap[Species];

        for (int Year = SpeciesFirstYearMap[Species];
                 Year <= SpeciesLastYearMap[Species]; ++Year) {
            for (int i = 0;
                     i <= SpeciesMaxCatchAgeMap[Species]; ++i) {
                if (maturityModel[SpeciesIndex] != NULL) {
                    if (SpeciesIsFixedMaturity) {
                        vLabel = maturityModel[SpeciesIndex]->verticalHeaderItem(i)->text();
                        Age    = vLabel.split(" ")[1].toInt();
                        index = maturityModel[SpeciesIndex]->index(i,0);
                    } else {
                        Age = i;
                        index = maturityModel[SpeciesIndex]->index(Year-FirstYear,i);
                    }
                    tableLine = QString::fromStdString(Species) + ", " +
                                QString::number(Year) + ", " +
                                QString::number(Age) + ", ";
                    qval = index.data().toString().trimmed();
                    if (! qval.isEmpty()) {
                        tableLine += qval;
                        outStream << tableLine << "\n";
                    }
                }
            }
        }
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    int retv = nmfQtUtils::rename(tmpFileNameWithPath, fileNameWithPath);
    if (retv < 0) {
        std::cout << "Error SSVPA_SaveMaturity: Couldn't rename " <<
                     tmpFileNameWithPath.toStdString() << " to " <<
                     fileNameWithPath.toStdString() <<
                     ". Save aborted." << std::endl;
        return "\nMaturity Data:     Failed. Check data entered.";
    }

    //
    // Save to mysql table
    //

    // Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableName;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_Maturity: Clearing table error: ", errorMsg);
        return "\nMaturity Data:     Failed. Check data entered.";
    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableName.toStdString();
    cmd += " (SpeIndex,SpeName,Year,Age,PMature) VALUES ";

    // Read from csv file (and not from the GUI) since it has all the Species in it.
    QFile fin2(fileNameWithPath);
    if(! fin2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error Save_Maturity: Opening Input File", fin.errorString());
        return "\nMaturity Data:     Failed. Check data entered.";
    }
    QTextStream inStream2(&fin2);

    std::string FileName,FileIndex,FileYear,FileAge,FilePMature; // SpeMaturity table fields
    while (! inStream2.atEnd()) {
        line = inStream2.readLine();
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields  = line.split(",");
        FileName = qfields[0].trimmed().toStdString();
        FileIndex   = std::to_string(SpeciesIndexMap[FileName]);
        FileYear    = qfields[1].trimmed().toStdString();
        FileAge     = qfields[2].trimmed().toStdString();
        FilePMature = qfields[3].trimmed().toStdString();
        cmd += "(" + FileIndex + ",";
        cmd += "\"" + FileName + "\",";
        cmd += FileYear     + ",";
        cmd += FileAge      + ",";
        cmd += FilePMature  + "), ";

    } // end for all rows in csv file

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    fin2.close();

    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_Maturity: Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        std::cout << "csv file: " << fileNameWithPath.toStdString() << std::endl;
        return "\nMaturity Data:     Failed. Check data entered.";
    }

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_Maturity Complete");

    return "\nMaturity Data:     OK";

} // end Save_MaturityOLD
*/

void
nmfSSVPATab4::callback_Maturity_ItemChanged(QStandardItem *item)
{

    MarkAsDirty("SpeMaturity");
    SSVPAMaturityTV->resizeColumnsToContents();

    //SSVPA_Tab4_SaveMaturityPB->setEnabled(true);

} // end callback_Maturity_ItemChanged


/*
 * N.B. Maturity doesn't affect SSVPA...only Forecast.
 */
void
nmfSSVPATab4::callback_SSVPAMaturitySliderMoved(int value)
{
    QModelIndexList selection = SSVPAMaturityTV->selectionModel()->selectedIndexes();

    float normalizedValue = SSVPAMaturityMinLE->text().toDouble()
            + maturitySliderScale * (value / 100.0);
    foreach (QModelIndex index, selection) {

//        currentMaturityModel->setData(index,normalizedValue);
        SSVPAMaturityTV->model()->setData(index,normalizedValue);

    }

    MarkAsDirty("SpeMaturity");
    emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());

} // end callback_SSVPAMaturitySliderMoved


void
nmfSSVPATab4::runSSVPA()
{
    emit RunSSVPA("");
}


void
nmfSSVPATab4::callback_RunSSVPA(QModelIndex a, QModelIndex b)
{
    runSSVPA();
}


/*
 * Disconnect the model's signal here so that there won't be a double dataChanged
 * event when the user uses the slider.
 */
void
nmfSSVPATab4::callback_SSVPAMaturitySliderPressed() {
    std::string SpeciesName;
    int SpeciesIndex;

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    currentMaturityModel = maturityModel[SpeciesIndex];

    maturitySliderScale = SSVPAMaturityMaxLE->text().toDouble() -
                          SSVPAMaturityMinLE->text().toDouble();

    disconnect(currentMaturityModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
               this,                 SLOT(callback_RunSSVPA(QModelIndex, QModelIndex)));

} // end callback_SSVPAMaturitySliderPressed


/*
 * Reconnect the model's signal here so that the user will be able to edit by double
 * clicking in a cell.
 */
void
nmfSSVPATab4::callback_SSVPAMaturitySliderReleased() {

    connect(currentMaturityModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this,                 SLOT(callback_RunSSVPA(QModelIndex, QModelIndex)));

} // end callback_SSVPAMaturitySliderReleased






void
nmfSSVPATab4::callback_updateModel(
        const QModelIndex &topLeft,
        const QModelIndex &bottomRight)
{

    // Only emit if checks have all passed.
    QString BadTables = "";
    bool AllChecksOK = runDBChecks("CheckAll",BadTables);
    if (AllChecksOK) {
        emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());
    }
}

void nmfSSVPATab4::showVPAGroupBox(int whichGroupBox)
{
    std::vector<QGroupBox *> groupBoxes =
        {SSVPAConfigure1GB,SSVPAConfigure2GB,SSVPAConfigure3GB,SSVPAConfigure4GB};

    for (QGroupBox *aGroupBox : groupBoxes) {
        aGroupBox->hide();
    }
    groupBoxes[whichGroupBox]->show();

} // end showVPAGroupBox


void
nmfSSVPATab4::callback_Configure_XSA_LE1()
{
    Configure_XSA_LE1->setText(Configure_XSA_LE1->text().trimmed());

    if (SelectVPATypeCMB->count() != 0) {
        emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());
    }

    MarkAsDirty("SpeSSVPA");

} // end callback_Configure_XSA_LE1

void
nmfSSVPATab4::callback_Configure_XSA_NYears_SB(int value)
{
    //Configure_XSA_LE2->setText(Configure_XSA_LE2->text().trimmed());

    if (SelectVPATypeCMB->count() != 0) {
        emit RunSSVPA(""); //SelectVPATypeCMB->currentText().toStdString());
    }

    MarkAsDirty("SpeSSVPA");

} // end callback_Configure_XSA_NYears_SB

void
nmfSSVPATab4::callback_Configure_XSA_NAges_SB(int value)
{
    //Configure_XSA_LE3->setText(Configure_XSA_LE3->text().trimmed());

    if (SelectVPATypeCMB->count() != 0) {
        emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());
    }

    MarkAsDirty("SpeSSVPA");

} // end callback_Configure_XSA_NAges_SB


void
nmfSSVPATab4::callback_SSVPA_Tab4_AddConfigPB(bool unused)
{
    bool ok;
    int SpeciesIndex;
    std::string SpeciesName;
    QString msg;
    QString NewSSVPAName;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::callback_SSVPA_Tab4_AddConfigPB");

    msg = "Enter new SSVPA Configuration name:";
    NewSSVPAName = QInputDialog::getText(SSVPA_Tabs,
                                         "Add SSVPA Config",
                                         msg, QLineEdit::Normal,"", &ok);

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    NewSSVPAName = NewSSVPAName.trimmed();

    if (ok && !NewSSVPAName.isEmpty() && (SelectConfigurationCMB->findText(NewSSVPAName) < 0))
    {
        SelectConfigurationCMB->blockSignals(true);
        SelectConfigurationCMB->addItem(NewSSVPAName);
        SelectConfigurationCMB->setCurrentText(NewSSVPAName);
        SelectConfigurationCMB->blockSignals(false);
        msg = "Added SSVPA Configuration: " + NewSSVPAName;
        logger->logMsg(nmfConstants::Normal,msg.toStdString());

        // Save the config here...
        Save_SpeSSVPA(SpeciesIndex);

        // Clear Mortality gui
//        int NumRows = mortality_model->count();
//        mortality_model->removeRows(0, NumRows, QModelIndex());
//        for (int i=0; i<=NumRows; ++i) {
//            mortality_model->append(nmfMortality { i, "0", "0" });
//        }
    } else {
        if (ok) {
            QMessageBox::information(SSVPA_Tabs,
                                     tr("Invalid SSVPA Configuration Name"),
                                     tr("\nPlease add a unique and valid SSVPA Configuration name."));
        }
        return;
    }
    QMessageBox::information(SSVPA_Tabs, "Save",
                             tr("\n"+msg.toLatin1()));

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::callback_SSVPA_Tab4_AddConfigPB Complete");


} // end callback_SSVPA_Tab4_AddConfigPB


void
nmfSSVPATab4::callback_SSVPA_Tab4_DelConfigPB(bool unused)
{
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    std::string msg;
    QMessageBox::StandardButton reply;
    QString SSVPANameToRemove    = SelectConfigurationCMB->currentText();
    int SSVPAIndexToRemove       = SelectConfigurationCMB->currentIndex();

    if (SSVPANameToRemove.isEmpty())
        return;

    msg = "\nOK to delete SSVPA Configuration: " + SSVPANameToRemove.toStdString() + " ?";
    reply = QMessageBox::question(SSVPA_Tabs,
                                  tr("Delete SSVPA Configuration"),
                                  tr(msg.c_str()),
                                  QMessageBox::Cancel|QMessageBox::Ok,
                                  QMessageBox::Cancel);

    if (reply == QMessageBox::Ok) {
        // Check if SSVPA has even been saved and if it has, remove it from the
        // database and the csv file.
        fields   = {"SpeIndex", "SSVPAName"};
        queryStr = "SELECT SpeIndex,SSVPAName FROM SpeSSVPA WHERE SSVPAName = '" +
                   SSVPANameToRemove.toStdString() + "'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);

        if (dataMap["SpeIndex"].size() != 0) {
            bool removeOK = removeFromTableAndCSVFile(SSVPANameToRemove,"SpeSSVPA");
            if (removeOK) {
                removeFromTableAndCSVFile(SSVPANameToRemove,"SSVPAAgeM");

                // Remove from combo box
                SelectConfigurationCMB->blockSignals(true);
                SelectConfigurationCMB->removeItem(SSVPAIndexToRemove);
                SelectConfigurationCMB->blockSignals(false);
            }
            /*
            // Remove from SSVPAAgeM and SpeSSVPA csv files and resave to database
            bool removeOk = removeFromCSVFiles(SSVPANameToRemove);
            if (removeOk) {
                //Save_SpeSSVPA_DB_Table();
                Save_SSVPAAgeM_DB_Table();
            }
            */
        }
    }

} // end callback_SSVPA_Tab4_DelConfigPB


bool
nmfSSVPATab4::removeFromTableAndCSVFile(QString SSVPAName, QString TableName)
{
    int SSVPAIndex;
    std::string cmd;
    std::string errorMsg;
    std::string SpeciesName;
    std::vector<std::string> fields;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::removeFromTableAndCSVFile");

    if ((TableName != "SpeSSVPA") && (TableName != "SSVPAAgeM"))
    {
        logger->logMsg(nmfConstants::Normal,"Error removeFromTableAndCSVFile: Please add fields in function for table: " + TableName.toStdString());
        return false;
    }

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    //SSVPAName  = SelectConfigurationCMB->currentText().toStdString();
    SSVPAIndex = SelectConfigurationCMB->currentIndex()+1;

    cmd = "DELETE FROM " + TableName.toStdString() +
          " WHERE SpeIndex = "  + std::to_string(SpeciesIndex) +
          " AND SSVPAName  = '" + SSVPAName.toStdString() + "'" +
          " AND SSVPAIndex = "  + std::to_string(SSVPAIndex);
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
std::cout << "Del Cmd: " << cmd << std::endl;
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error removeFromTableAndCSVFile: Delete error: ", errorMsg);
        return false;
    }

    // Add fields for TableName
    if (TableName == "SpeSSVPA")
        fields = {"SpeIndex","SSVPAName","SSVPAIndex","Type","AgeM","TermZType","TermF","RefAge",
                  "LogCatchAge1","LogCatchAge2","FullRecAge","PartSVPA","pSVPANCatYrs","pSVPANMortYrs",
                  "pSVPARefAge","NFleets","NXSAIndex","Downweight","DownweightType","DownweightYear",
                  "Shrink","ShrinkCV","ShrinkYears","ShrinkAge"};
    else if (TableName == "SSVPAAgeM")
        fields = {"SpeIndex","SSVPAIndex","SSVPAName","Age","AgeM1","AgeM2","PRF"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::removeFromTableAndCSVFile Complete");

    return true;

} // end removeFromTableAndCSVFile


bool
nmfSSVPATab4::removeFromCSVFiles(QString SSVPANameToRemove)
{
    QString line,qSpeIndex,qSSVPAName;
    QStringList qfields;
    std::string SpeciesName;
    int SpeciesIndex;

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    for (QString CSVFile : {"SpeSSVPA.csv","SSVPAAgeM.csv"})
    {
        QString filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        QString fileNameWithPath    = QDir(filePath).filePath(CSVFile);
        QString tmpFileNameWithPath = QDir(filePath).filePath("."+CSVFile);
        QFile   fin(fileNameWithPath);
        QFile   fout(tmpFileNameWithPath);

        // Open the files here....
        if (! fin.open(QIODevice::ReadOnly)) {
            QMessageBox::information(SSVPA_Tabs, "Error(8) Opening Input File", fin.errorString());
            return false;
        }
        if (! fout.open(QIODevice::WriteOnly)) {
            QMessageBox::information(SSVPA_Tabs, "Error(8) Opening Output File", fout.errorString());
            return false;
        }
        QTextStream inStream(&fin);
        QTextStream outStream(&fout);

        // Copy instream to outstream without the lines to be deleted
        while (! inStream.atEnd()) {
            line      = inStream.readLine();
            qfields   = line.split(",");
            qSpeIndex = qfields[0].trimmed();
            if (CSVFile == "SpeSSVPA.csv")
                qSSVPAName = qfields[1].trimmed();
            else if (CSVFile == "SSVPAAgeM.csv")
                qSSVPAName = qfields[2].trimmed();
            if (SpeciesIndex != qSpeIndex.toInt() || SSVPANameToRemove != qSSVPAName)
                outStream << line << "\n";
        }
        fin.close();
        fout.close();

        // Now, mv temp file to actual file.
        int retv = nmfUtilsQt::rename(tmpFileNameWithPath, fileNameWithPath);
        if (retv < 0) {
            std::cout << "Error: Couldn't rename " <<
                         tmpFileNameWithPath.toStdString() << " to " <<
                         fileNameWithPath.toStdString() <<
                         ". Save aborted." << std::endl;
            return false;
        }
    } // end for

    return true;

} // end removeFromCSVFiles


void
nmfSSVPATab4::callback_RefAgeChanged(int val)
{

    MarkAsDirty("SpeSSVPA");
    emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());

} // end callback_RefAgeChanged

void
nmfSSVPATab4::callback_TermFChanged(double val)
{

    MarkAsDirty("SpeSSVPA");
    emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());

} // end callback_TermFChanged


void
nmfSSVPATab4::callback_SSVPAMortalityTV(QModelIndex index)
{

    CurrentTable = "SSVPAAgeM";

} // end callback_SSVPAMortalityTV


/*
 * Disconnect the model's signal here so that there won't be a double dataChanged
 * event when the user uses the slider.
 */
void
nmfSSVPATab4::callback_MortalitySliderPressed()
{
    mortalitySliderScale = MortalityMaxLE->text().toDouble() -
                           MortalityMinLE->text().toDouble();

    MarkAsDirty("SSVPAAgeM");

//    disconnect(SSVPAMortalityTV->model(), SIGNAL(dataChanged(QModelIndex, QModelIndex)),
//               this,                      SLOT(updateModel(QModelIndex, QModelIndex)));

} // end callback_MortalitySliderPressed


/*
 * Reconnect the model's signal here so that the user will be able to edit by double
 * clicking in a cell.
 */
void
nmfSSVPATab4::callback_MortalitySliderReleased()
{
//    connect(SSVPAMortalityTV->model(),  SIGNAL(dataChanged(QModelIndex, QModelIndex)),
//            this,                       SLOT(updateModel(QModelIndex, QModelIndex)));

} // end callback_MortalitySliderReleased


void
nmfSSVPATab4::callback_MortalitySliderMoved(int value)
{
    QModelIndexList selection = SSVPAMortalityTV->selectionModel()->selectedIndexes();

//    disconnect(SSVPAMortalityTV->model(),       SIGNAL(dataChanged(QModelIndex, QModelIndex)),
//            this,                               SLOT(callback_updateModel(QModelIndex, QModelIndex)));

    float normalizedValue = MortalityMinLE->text().toDouble()
            + mortalitySliderScale * (value / 100.0);
    foreach (QModelIndex index, selection) {
        SSVPAMortalityTV->model()->setData(index,normalizedValue);
    }

    MarkAsDirty("SSVPAAgeM");
    emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());

//    connect(SSVPAMortalityTV->model(),          SIGNAL(dataChanged(QModelIndex, QModelIndex)),
//            this,                               SLOT(callback_updateModel(QModelIndex, QModelIndex)));

} // end callback_MortalitySliderMoved


void
nmfSSVPATab4::callback_MortalityMaxEdited()
{
    double mortalityMax = MortalityMaxLE->text().toDouble();
    double mortalityMin = MortalityMinLE->text().toDouble();

    if ((mortalityMax > nmfConstants::MortalityMax) || (mortalityMax <= mortalityMin)) {
        mortalityMax = nmfConstants::MortalityMax;
        MortalityMaxLE->setText(QString::number(mortalityMax, 'f', 1));
    }

} // end callback_MortalityMaxEdited


void
nmfSSVPATab4::callback_MortalityMinEdited()
{
    double mortalityMax = MortalityMaxLE->text().toDouble();
    double mortalityMin = MortalityMinLE->text().toDouble();

    if ((mortalityMin < nmfConstants::MortalityMin) || (mortalityMin >= mortalityMax)) {
        mortalityMin = nmfConstants::MortalityMin;
        MortalityMinLE->setText(QString::number(mortalityMin, 'f', 1));
    }

} // end callback_MortalityMinEdited


void
nmfSSVPATab4::callback_InitialSelectivityChanged(QModelIndex indexA,
                                                 QModelIndex indexB)
{
    MarkAsDirty("SSVPAAgeM");
    SSVPAInitialSelectivityTV->resizeColumnsToContents();

} // end callback_InitialSelectivityChanged

void
nmfSSVPATab4::callback_MortalityChecked(int state, bool okToEmit)
{
    bool checked = (state == Qt::Checked);

    ResidualNaturalMortalityLBL->setEnabled(!checked);
    ResidualNaturalMortalityDSB->setEnabled(!checked);
    PredationMortalityLBL->setEnabled(!checked);
    PredationMortalityDSB->setEnabled(!checked);
    SSVPAMortalityTV->setEnabled(checked);
    NaturalMortalitySL->setEnabled(checked);
    MortalityMaxLE->setEnabled(checked);
    MortalityMinLE->setEnabled(checked);

    if (okToEmit) {
        emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());
    }
    MarkAsDirty("SSVPAAgeM");
}

void
nmfSSVPATab4::callback_MortalityChecked(int state)
{
    callback_MortalityChecked(state, true);

} // end callback_MortalityChecked


void
nmfSSVPATab4::callback_ResidualMortalityChanged(double newM1)
{
    int numRows =  mortality_model->count();

    for (int i=0; i<numRows; ++i) {
        mortality_model->setData(mortality_model->index(i,1),
                                 newM1);
    }
    SSVPAMortalityTV->resizeColumnsToContents();
    emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());

    MarkAsDirty("SSVPAAgeM");

} // end callback_ResidualMortalityChanged


void
nmfSSVPATab4::callback_PredationMortalityChanged(double newM2)
{
    int numRows =  mortality_model->count();

    for (int i=0; i<numRows; ++i) {
        mortality_model->setData(mortality_model->index(i,2),
                                 newM2);
    }
    SSVPAMortalityTV->resizeColumnsToContents();
    emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());

    MarkAsDirty("SSVPAAgeM");

} // end callback_PredationMortalityChanged


void
nmfSSVPATab4::callback_SelectVPATypeCMB(int index)
{

    // Set all Load/Save buttons to their default state.
    SSVPA_Tab4_LoadFleetPB->hide();
    SSVPA_Tab4_LoadIndicesPB->hide();
    if (index == 1) {
        //Load_SeparableVPAWidgets();
    } else if (index == 2) {
        SSVPA_Tab4_LoadFleetPB->show();
    } else if (index == 3) {
        SSVPA_Tab4_LoadIndicesPB->show();
    }

    MarkAsDirty("SpeSSVPA");

    showVPAGroupBox(index);
    emit EnableRunSSVPAPB(false);

    //emit RunSSVPA(SelectVPATypeCMB->currentText().toStdString());

} // end callback_SelectVPATypeCMB


void
nmfSSVPATab4::Load_SSVPAAgeM()
{
    // Load SSVPAAgeM and then ask user about loading others....
    QString line;
    QStringList fields;
    QString csvSpecies;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QFileDialog fileDlg(SSVPA_Tabs);
    QStringList NameFilters;
    std::string SpeciesName;
    int SpeciesIndex;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Load_SSVPAAgeM");

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    // Setup Load dialog
    fileDlg.setDirectory(path);
    fileDlg.selectFile("SSVPAAgeM.csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg.setNameFilters(NameFilters);
    fileDlg.setWindowTitle("Load Species Mortality CSV File");
    if (fileDlg.exec()) {

        // Open the file here....
        SpeMortalityCSVFile = fileDlg.selectedFiles()[0];
        QFile file(SpeMortalityCSVFile);
        if(! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(SSVPA_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);

        int csvFileRow = 0;
        mortality_model->removeRows(0, mortality_model->count(), QModelIndex());
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (csvFileRow > 0) { // skip row=0 as it's the header
                fields = line.split(",");
                csvSpecies = fields[0];
                if (csvSpecies == QString::number(SpeciesIndex)) {
                    mortality_model->append(nmfMortality { fields[3].toInt(), fields[4], fields[5] });
                }
            }
            ++csvFileRow;
        }
        file.close();
        SSVPAMortalityTV->resizeColumnsToContents();
    }

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Load_SSVPAAgeM Complete");

} // end Load_SSVPAAgeM


//void
//nmfSSVPATab4::Load_SpeSSVPA()
//{
//    QMessageBox::information(SSVPA_Tabs, tr("Load"),
//         tr("\nThis table does not display visually.\nPopulate its CSV file (SpeSSVPA.csv) and then Save.\n"));

//} // end Load_SpeSSVPA


void
nmfSSVPATab4::callback_SSVPA_Tab4_LoadMortalityPB(bool unused)
{

    Load_SSVPAAgeM();

} // end callback_SSVPA_Tab4_LoadMortalityPB

/*
void
nmfSSVPATab4::callback_SSVPA_Tab4_SaveMortalityPB()
{
    std::string SpeciesName;
    int SpeciesIndex;

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    Save_SSVPAAgeM(SpeciesIndex, false);

} // end callback_SSVPA_Tab4_SaveMortalityPB
*/
//void
//nmfSSVPATab4::callback_SSVPA_Tab4_LoadConfigPB()
//{
//    QMessageBox::StandardButton reply;
//    std::string msg;

//    msg  = "\nPlease edit the SpeSSVPA.csv file before\n";
//    msg += "clicking the OK button below.\n";
//    reply = QMessageBox::information(SSVPA_Tabs,
//                         tr("Load SpeSSVPA"),
//                         tr(msg.c_str()),
//                         QMessageBox::Cancel|QMessageBox::Ok,
//                         QMessageBox::Ok);
//    if (reply == QMessageBox::Ok) {
//        Load_SpeSSVPA();
//    }

//} // end callback_SSVPA_Tab4_LoadConfigPB


QString
nmfSSVPATab4::Save_Config()
{
    std::string SpeciesName;
    int SpeciesIndex;
    bool okCSVFile;
    bool okConfig;
    bool okMortality;
    QString retv;

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    okConfig    = Save_SpeSSVPA(SpeciesIndex);
    okMortality = Save_InitialSelectivityAndMortality(SpeciesIndex);

//    emit EnableRunSSVPAPB(true);

    retv  = (okConfig)    ? "\nConfig Data:       OK" : "\nConfig Data:       Failed. Please check SSVPA config data entered.";
    retv += (okMortality) ? "\nMortality Data:    OK" : "\nMortality Data:    Failed. Check Mortality data entered.";

    return retv;

} // end Save_Config


void
nmfSSVPATab4::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    // 1. Restore Config
    TableName = "SpeSSVPA";
    fields    = {"SpeIndex","SSVPAName","SSVPAIndex","Type","AgeM","TermZType",
                 "TermF","RefAge","LogCatchAge1","LogCatchAge2","FullRecAge",
                 "PartSVPA","pSVPANCatYrs","pSVPANMortYrs","pSVPARefAge","NFleets",
                 "NXSAIndex","Downweight","DownweightType","DownweightYear",
                 "Shrink","ShrinkCV","ShrinkYears","ShrinkAge"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    // 2. Restore Mortality
    TableName = "SSVPAAgeM";
    fields    = {"SpeIndex","SSVPAIndex","SSVPAName","Age","AgeM1","AgeM2","PRF"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    // 3. Restore XSA Indices and Data
    TableName = "SpeXSAIndices";
    fields    = {"SpeIndex","SSVPAName","`Index`","IndexName","Alpha","Beta"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = "SpeXSAData";
    fields    = {"SpeIndex","SSVPAName","`Index`","Year","Age","Value"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    // 4. Restore Fleet
    TableName = "SpeTuneCatch";
    fields    = {"SpeName","SpeIndex","SSVPAName","Fleet","Year","Age","Catch"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = "SpeTuneEffort";
    fields    = {"SpeName","SpeIndex","SSVPAName","Fleet","Year","Effort"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    // 5. Restore Maturity
    TableName = "SpeMaturity";
    fields    = {"SpeIndex","SpeName","Year","Age","PMature"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase


void
nmfSSVPATab4::callback_SSVPA_Tab4_SavePB(bool unused)
{
    QString fleetDataStr = "";
    QString indicesDataStr = "";
    int currentIndex = SelectConfigurationCMB->currentIndex();
    QString currentVPAType = SelectVPATypeCMB->currentText();

    // 1. Save Config and Mortality Info
    QString configDataStr   = Save_Config();

    // 2. Save InitialSelectivity
    QString initialSelectivityDataStr = Save_InitialSelectivity();

    // 3. Save Index data
    if (currentVPAType.toStdString() == nmfConstantsMSVPA::DefaultVPATypes[3]) {
        indicesDataStr = Save_XSAIndexData();
    }

    // 4. Save Maturity
    QString maturityDataStr = Save_SpeMaturity();

    // 5. Save Fleet
    if (currentVPAType.toStdString() == nmfConstantsMSVPA::DefaultVPATypes[2]) {
        fleetDataStr = Save_Fleet();
    }

    // Display GUI with Save feedback....
    QString msg = "\nSave results: \n";
    msg += configDataStr;
    msg += initialSelectivityDataStr;
    msg += maturityDataStr;
    msg += fleetDataStr;
    msg += indicesDataStr;
    QMessageBox *mbox = new QMessageBox(SSVPA_Tabs);
    mbox->setWindowTitle(tr("Save"));
    mbox->setText(tr(msg.toLatin1()));
    mbox->setStandardButtons(QMessageBox::Ok);
    //QFont font = QFont("Courier");
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    //font.setBold(true);
    mbox->setFont(fixedFont);
    mbox->exec();

    QString BadTables = "";
    bool AllChecksOK = runDBChecks("CheckAll",BadTables);
    if (AllChecksOK) {
        emit EnableRunSSVPAPB(true);
    }

    SelectConfigurationCMB->setCurrentIndex(currentIndex);

    MarkAsClean();

} // end callback_SavePB


void
nmfSSVPATab4::clearModels()
{

    mortality_model->removeRows(0, mortality_model->count(), QModelIndex());
    selectivity_model->removeRows(0, selectivity_model->count(), QModelIndex());

} // end clearModels

/*
bool
nmfSSVPATab4::Save_SSVPAAgeM_DB_Table()
{
    QString qcmd;
    QString line;
    QString TableName = "SSVPAAgeM";
    QStringList qfields;
    std::string cmd;
    std::string errorMsg;
    bool missingMortalityData = false;
    bool FirstLine = true;

    //
    // Save to mysql table
    //

    // Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableName;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("SSVPA Save(5c): Clearing table error: ", errorMsg);
        return false;
    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableName.toStdString();
    if (missingMortalityData)
        cmd += " (SpeIndex, SSVPAIndex, SSVPAName, Age, PRF) VALUES ";
    else
        cmd += " (SpeIndex, SSVPAIndex, SSVPAName, Age, AgeM1, AgeM2, PRF) VALUES ";

    // Read from csv file (and not from the GUI) since it has all the Species in it.
    QString SpeMortalityCSVFile = TableName+".csv";
    QString filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QString fileNameWithPath    = QDir(filePath).filePath(SpeMortalityCSVFile);
    QFile fin2(fileNameWithPath);
    if (! fin2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(5a) Opening Input File", fin2.errorString());
        return false;
    }
    QTextStream inStream2(&fin2);

    std::string FileSpeIndex, FileSSVPAIndex, FileSSVPAName, FileAge;
    std::string FileAgeM1, FileAgeM2, FilePRF; // SSVPAAgeM table fields
    while (! inStream2.atEnd()) {
        line = inStream2.readLine();
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields   = line.split(",");

        FileSpeIndex   = qfields[0].trimmed().toStdString();
        FileSSVPAIndex = qfields[1].trimmed().toStdString();
        FileSSVPAName  = qfields[2].trimmed().toStdString();
        FileAge        = qfields[3].trimmed().toStdString();
        FileAgeM1      = qfields[4].trimmed().toStdString();
        FileAgeM2      = qfields[5].trimmed().toStdString();
        FilePRF        = qfields[6].trimmed().toStdString();
        cmd += "(" + FileSpeIndex + ",";
        cmd += FileSSVPAIndex     + ",";
        cmd += "\"" + FileSSVPAName + "\",";
        cmd += FileAge     + ",";
        if (! missingMortalityData) {
            cmd += FileAgeM1      + ",";
            cmd += FileAgeM2      + ",";
        }
        cmd += FilePRF  + "), ";
//std::cout << FileSpeIndex << "," << FileSSVPAIndex << "," << FileAge << std::endl;

    } // end for all rows in csv file

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    fin2.close();
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("SSVPA Save(5a): Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        return false;
    }

//    QMessageBox::information(SSVPA_Tabs, "Save",
//                             tr("\nMortality data saved.\n"));

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_SSVPAAgeM Complete");

    return true;

} // end Save_SSVPAAgeM
*/

bool
nmfSSVPATab4::displayMsg(std::string msg)
{

    QMessageBox::information(SSVPA_Tabs,
                             tr("Warning"),
                             tr(msg.c_str()),
                             QMessageBox::Ok);

    return true;

} // end displayMsg


void
nmfSSVPATab4::callback_SSVPA_Tab4_LoadFleetPB(bool unused)
{
    int NFleets=0;
    int NumRecords = 0;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::callback_SSVPA_Tab4_LoadFleetPB");

    // Set correct Number of Fleets
    fields   = {"SSVPAName", "SSVPAIndex", "NFleets"};
    queryStr = "SELECT SSVPAName,SSVPAIndex,NFleets FROM SpeSSVPA WHERE SpeIndex = " +
               std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    NumRecords = dataMap["NFleets"].size();
    if (NumRecords > 0) {
        NFleets = std::stoi(dataMap["NFleets"][0]);
        if (NFleets == -1) // Means that user hasn't saved yet, so read gui
        {
            NFleets = Configure_EffortTuned_SetNumFleetsCMB->currentIndex();
        }
        std::cout << "PB NFleets: " << NFleets << std::endl;

        Configure_EffortTuned_SetNumFleetsCMB->setCurrentIndex(NFleets);
        Load_FleetEffortData("");
        if (NFleets == 1)
            Load_CatchData("");
        else
            Load_FleetCatchData("");
    }

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::callback_SSVPA_Tab4_LoadFleetPB Complete");

} // callback_SSVPA_Tab4_LoadFleetPB


void
nmfSSVPATab4::Load_FleetEffortData(QString filename)
{
    QString line;
    QStringList fields;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QStringList NameFilters;
    QString SpeFleetEffortCSVFile;
    QString FileSpeName,FileSSVPAName;
    //int FileSpeIndex;
    int FileFleet,FileYear;
    double FileEffort;
    bool readFirstLine = false;
    QFileDialog *fileDlg;
    QModelIndex index;
    std::string SpeciesName;

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Load_FleetEffortData with: "+filename.toStdString());

    if (filename.isEmpty()) {
        // Setup Load dialog for SpeTuneEffort (ie, just the Effort data)
        fileDlg = new QFileDialog(SSVPA_Tabs);
        fileDlg->setDirectory(path);
        fileDlg->selectFile("SpeTuneEffort.csv");
        NameFilters << "*.csv" << "*.*";
        fileDlg->setNameFilters(NameFilters);
        fileDlg->setWindowTitle("Load Fleet Effort CSV File");
        if (fileDlg->exec()) {
            // Open the file here....
            SpeFleetEffortCSVFile = fileDlg->selectedFiles()[0];
        }
        delete fileDlg;
    } else {
        SpeFleetEffortCSVFile = filename;
    }

    QFile file(SpeFleetEffortCSVFile);
    if (! file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "File Read Error", file.errorString());
        return;
    }
    QTextStream inStream(&file);

    // Load the model from the data in the csv file
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        if (readFirstLine) { // skip row=0 as it's the header
            fields = line.split(",");
            FileSpeName   = fields[0];
            //FileSpeIndex  = fields[1].toInt();
            FileSSVPAName = fields[2];
            FileFleet     = fields[3].toInt();
            FileYear      = fields[4].toInt();
            FileEffort    = fields[5].toDouble();
            if (FileSpeName.toStdString() == SpeciesName) {
                index = fleetModel[FileFleet]->index(FileYear-FirstYear,0);
                fleetModel[FileFleet]->setData(index,FileEffort);
            }
        }
        readFirstLine = true;
    }
    file.close();

    callback_EffortTuned_SelectAFleetCMB(0);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Load_FleetEffortData Complete");

} // end Load_FleetEffortData


void
nmfSSVPATab4::Load_CatchData(QString filename)
{
    //int numAgeCols;
    int SpeciesIndex;
    std::string SpeciesName;
    QString line;
    QStringList fields;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QStringList NameFilters;
    QString SpeCatchCSVFile;
    QString FileSpeName,FileSSVPAName;
    int FileYear;
    double FileCatch;
    bool readFirstLine = false;
    QFileDialog *fileDlg;
    QModelIndex index;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Load_CatchData: " + filename.toStdString());

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    if (filename.isEmpty()) {
        // Setup Load dialog for SpeCatch (ie, just the Catch data)
        fileDlg = new QFileDialog();
        fileDlg->setParent(SSVPA_Tabs);
        fileDlg->setDirectory(path);
        fileDlg->selectFile("SpeCatch.csv");
        NameFilters << "*.csv" << "*.*";
        fileDlg->setNameFilters(NameFilters);
        fileDlg->setWindowTitle("Load Catch CSV File");
        if (fileDlg->exec()) {
            // Open the file here....
            SpeCatchCSVFile = fileDlg->selectedFiles()[0];
        }
        delete fileDlg;
    } else {
        SpeCatchCSVFile = filename;
    }
std::cout << "Opening: " << SpeCatchCSVFile.toStdString() << std::endl;
    QFile file(SpeCatchCSVFile);
    if (! file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "File Read Error", file.errorString());
        return;
    }
    QTextStream inStream(&file);

    // Load the model from the data in the csv file
    //numAgeCols = MaxCatchAge-MinCatchAge+1;
    int i=0;
    int m;
    QString theField;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        if (readFirstLine) { // skip row=0 as it's the header
            fields = line.split(",");
            FileSpeName   = fields[0];
            if (FileSpeName.toStdString() == SpeciesName) {
                //FileSpeIndex  = fields[1].toInt();
                FileYear      = fields[1].toInt();
                m = 0;
                for (int j=2; j<=MaxCatchAge+2; ++j) {
                    theField = fields[j];
                    if (! theField.trimmed().isEmpty()) {
                        FileCatch = theField.trimmed().toDouble();
                        index = fleetModel[0]->index(FileYear-FirstYear,m+1);
                        fleetModel[0]->setData(index,FileCatch);
                        m++;
                    }
                }
            }
        }
        readFirstLine = true;
    }
    file.close();

    callback_EffortTuned_SelectAFleetCMB(0);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Load_CatchData Complete");

} // end Load_CatchData


void
nmfSSVPATab4::Load_FleetCatchData(QString filename)
{
    int numAgeCols;
    QString line;
    QStringList fields;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QStringList NameFilters;
    QString SpeFleetCatchCSVFile;
    QString FileSpeName,FileSSVPAName;
    //int FileSpeIndex;
    int FileFleet,FileYear,FileAge;
    double FileCatch;
    bool readFirstLine = false;
    QFileDialog *fileDlg;
    QModelIndex index;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Load_FleetCatchData: " + filename.toStdString());

    if (filename.isEmpty()) {
        // Setup Load dialog for SpeTuneCatch (ie, just the Catch data)
        fileDlg = new QFileDialog();
        fileDlg->setParent(SSVPA_Tabs);
        fileDlg->setDirectory(path);
        fileDlg->selectFile("SpeTuneCatch.csv");
        NameFilters << "*.csv" << "*.*";
        fileDlg->setNameFilters(NameFilters);
        fileDlg->setWindowTitle("Load Fleet Catch CSV File");
        if (fileDlg->exec()) {
            // Open the file here....
            SpeFleetCatchCSVFile = fileDlg->selectedFiles()[0];
        }
        delete fileDlg;
    } else {
        SpeFleetCatchCSVFile = filename;
    }
    QFile file(SpeFleetCatchCSVFile);
    if (! file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "File Read Error", file.errorString());
        return;
    }
    QTextStream inStream(&file);

    // Load the model from the data in the csv file
    numAgeCols = MaxCatchAge-MinCatchAge+1;
    int i=0;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        if (readFirstLine) { // skip row=0 as it's the header
            fields = line.split(",");
            FileSpeName   = fields[0];
            //FileSpeIndex  = fields[1].toInt();
            FileSSVPAName = fields[2];
            FileFleet     = fields[3].toInt();
            FileYear      = fields[4].toInt();
            FileAge       = fields[5].toInt();
            FileCatch     = fields[6].toDouble();
            index = fleetModel[FileFleet]->index(FileYear-FirstYear,i++%numAgeCols+1);
            fleetModel[FileFleet]->setData(index,FileCatch);
        }
        readFirstLine = true;
    }
    file.close();

    callback_EffortTuned_SelectAFleetCMB(0);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Load_FleetCatchData Complete");

} // end Load_FleetCatchData


QString
nmfSSVPATab4::Save_Fleet()
{
    QString retv;
    std::string SpeciesName;
    int SpeciesIndex;
    bool okFleetEffort=true;
    bool okFleetData=true;

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::SSVPA_SaveFleet");

    int NumFleets = Configure_EffortTuned_SetNumFleetsCMB->currentText().toInt();
    if (NumFleets == 0) {
        okFleetData = true;
        okFleetEffort = true;
    } else {
        if (Save_FleetNumData(NumFleets,SpeciesIndex)) {
            okFleetEffort = Save_FleetEffortData(NumFleets, SpeciesName, SpeciesIndex);
            if (NumFleets > 0) {
                okFleetData = Save_FleetCatchData(NumFleets, SpeciesName, SpeciesIndex);
            }
        }
    }

    retv  = (okFleetEffort) ? "\nFleet Effort Data: OK" : "\nFleet Effort Data: Failed. Check data entered.";
    retv += (okFleetData)   ? "\nFleet Catch Data:  OK" : "\nFleet Catch Data:  Failed. Check data entered.";

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::SSVPA_SaveFleet Complete");

    return retv;

} // end SSVPA_SaveFleet


bool
nmfSSVPATab4::Save_FleetNumData(int NumFleets, int SpeciesIndex)
{
    std::string cmd;
    std::string errorMsg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;

    fields   = {"SSVPAName", "SSVPAIndex", "NFleets"};
    queryStr = "SELECT SSVPAName,SSVPAIndex,NFleets FROM SpeSSVPA WHERE SpeIndex = " +
            std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    std::string SSVPAName  = dataMap["SSVPAName"][0];
    std::string SSVPAIndex = dataMap["SSVPAIndex"][0];

    cmd  = "INSERT INTO SpeSSVPA ";
    cmd += "(SpeIndex,SSVPAName,SSVPAIndex,NFleets) values ";
    cmd +=  "(" + std::to_string(SpeciesIndex) +
            ",\"" + SSVPAName + "\"" +
            "," + SSVPAIndex +
            "," + std::to_string(NumFleets)+ ") ";
    cmd += " ON DUPLICATE KEY UPDATE ";
    cmd += " NFleets=values(NFleets); ";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        logger->logMsg(nmfConstants::Normal,"Error Save_FleetNumData: INSERT INTO SpeSSVPA: " + errorMsg);
        return false;
    }

    return true;

} // end Save_FleetNumData

/*
bool
nmfSSVPATab4::Save_FleetCatchData(int NumFleets, std::string SpeciesName, int SpeciesIndex)
{
std::cout << "Save_FleetCatchData" << std::endl;

    bool saveOK = true;
    bool FirstLine = true;
    QString fileName;
    QString filePath;
    QString fileNameWithPath;
    QString tmpFileNameWithPath;
    QString qcmd;
    QString line;
    QStringList qfields;
    std::string cmd;
    std::string errorMsg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString TableName = "SpeTuneCatch";

    if (! nmfQtUtils::allCellsArePopulated(SSVPA_Tabs,SSVPAFleetDataTV,
                                           nmfConstants::ShowError))
        return false;



    // Get some basic info that you'll need later on
    fields   = {"SpeIndex", "SpeName", "MinCatAge","MaxCatAge","FirstYear","LastYear"};
    queryStr = "SELECT SpeIndex,SpeName,MinCatAge,MaxCatAge,FirstYear,lastYear FROM Species WHERE SpeIndex = " +
            std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    int MinCatchAge = std::stoi(dataMap["MinCatAge"][0]);
    int MaxCatchAge = std::stoi(dataMap["MaxCatAge"][0]);
    int FirstYear   = std::stoi(dataMap["FirstYear"][0]);
    int LastYear    = std::stoi(dataMap["LastYear"][0]);
    fields   = {"SpeIndex", "SSVPAName"};
    queryStr = "SELECT SpeIndex,SSVPAName FROM SpeSSVPA WHERE SpeIndex = " +
            std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    std::string SSVPAName = dataMap["SSVPAName"][0];

std::cout << "MaxCatchAge: " << MaxCatchAge << std::endl;

    QString SpeFleetCatchCSVFile = "";

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (SpeFleetCatchCSVFile.isEmpty()) {
        SpeFleetCatchCSVFile = "SpeTuneCatch.csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPath    = QDir(filePath).filePath(SpeFleetCatchCSVFile);
        tmpFileNameWithPath = QDir(filePath).filePath("."+SpeFleetCatchCSVFile);
    } else {
        QFileInfo finfo(SpeFleetCatchCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPath    = SpeFleetCatchCSVFile;
        tmpFileNameWithPath = QDir(filePath).filePath("."+fileName);
    }
std::cout << fileNameWithPath.toStdString() << std::endl;
std::cout << tmpFileNameWithPath.toStdString() << std::endl;

    // Read entire file and copy all lines that don't have the current Species Index into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.

    QFile fin(fileNameWithPath);
    QFile fout(tmpFileNameWithPath);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(2) Opening Input File", fin.errorString());
        return false;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(2) Opening Output File", fout.errorString());
        return false;
    }
    QTextStream inStream(&fin);
    QTextStream outStream(&fout);

    QString tableLine;
    bool ignoreSpecies = false;

    QString FileSpeIndex;
    double FileCatch;

// RSK - continue refactoring this...read from model and to a REPLACE INTO...

    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");

        FileSpeIndex  = qfields[1].trimmed();
        if (FileSpeIndex == QString::number(SpeciesIndex)) {
            // Skip over all other Species Index rows, since going through this once copies the qtableview data
            if (! ignoreSpecies)
            {
                for (int Fleet=0; Fleet < NumFleets; ++Fleet) {
                    for (int Year  = FirstYear; Year <= LastYear; ++Year) {
                        for (int Age = 0; Age <= MaxCatchAge; ++Age) {
                            FileCatch = FleetData[Fleet][SpeciesIndex][Year-FirstYear][Age+1];
                            tableLine = QString::fromStdString(SpeciesName)   + ", " +
                                    QString::number(SpeciesIndex)         + ", " +
                                    QString::fromStdString(SSVPAName) + ", " +
                                    QString::number(Fleet)            + ", " +
                                    QString::number(Year-FirstYear)   + ", " +
                                    QString::number(Age)              + ", " +
                                    QString::number(FileCatch);
                            outStream << tableLine << "\n";
                        }
                    }
                }
                ignoreSpecies = true;
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }
    }
    if (! ignoreSpecies) // Means that there was no Species data found...so add it
    {
        for (int Fleet=0; Fleet < NumFleets; ++Fleet) {
            for (int Year  = FirstYear; Year <= LastYear; ++Year) {
                for (int Age = 0; Age <= MaxCatchAge; ++Age) {
                    FileCatch = FleetData[Fleet][SpeciesIndex][Year-FirstYear][Age+1];
                    tableLine = QString::fromStdString(SpeciesName)   + ", " +
                            QString::number(SpeciesIndex)         + ", " +
                            QString::fromStdString(SSVPAName) + ", " +
                            QString::number(Fleet)            + ", " +
                            QString::number(Year-FirstYear)   + ", " +
                            QString::number(Age)              + ", " +
                            QString::number(FileCatch);
                    outStream << tableLine << "\n";
                }
            }
        }
    }
    fin.close();
    fout.close();

    int retv = nmfQtUtils::rename(tmpFileNameWithPath, fileNameWithPath);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPath.toStdString() << " to " <<
                     fileNameWithPath.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return false ;
    }

    //
    // Save to mysql table
    //

    // Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableName;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
        saveOK = false;
        nmfUtils::printError("SSVPA Save(5C): Clearing table error: ", errorMsg);
    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableName.toStdString();
    cmd += "(SpeName, SpeIndex, SSVPAName, Fleet, Year, Age, Catch) VALUES ";

    // Read from csv file (and not from the GUI) since it has all the Species in it.
    QFile fin2(fileNameWithPath);
    if (! fin2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(5C) Opening Input File", fin.errorString());
        return false;
    }
    QTextStream inStream2(&fin2);

    while (! inStream2.atEnd()) {
        line = inStream2.readLine();
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields   = line.split(",");
        cmd += "(\"" + qfields[0].trimmed().toStdString() + "\",";
        cmd +=         qfields[1].trimmed().toStdString() + ",";
        cmd += "\""  + qfields[2].trimmed().toStdString() + "\",";
        cmd +=         qfields[3].trimmed().toStdString() + ",";
        cmd +=         qfields[4].trimmed().toStdString() + ",";
        cmd +=         qfields[5].trimmed().toStdString() + ",";
        cmd +=         qfields[6].trimmed().toStdString() + "), ";
    } // end for all rows in csv file

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    fin2.close();
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("SSVPA Save(5C): Write table error: ", errorMsg);
        return false;
    }

    //SSVPA_Tab4_CheckAllPB->setEnabled(true);
//    if (saveOK) {
//        QMessageBox::information(SSVPA_Tabs, "Save",
//                                 tr("\nFleet Catch data saved.\n"));
//    }

    return true;

} // end Save_FleetCatchData
*/


bool
nmfSSVPATab4::Save_FleetEffortData(int NumFleets, std::string SpeciesName, int SpeciesIndex)
{
    int numRows;
    std::string cmd;
    std::string errorMsg;
    std::string SSVPAName;
    QString TableName = "SpeTuneEffort";
    std::vector<std::string> fields;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_Tab4_SaveFleetEffortData");

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    SSVPAName = SelectConfigurationCMB->currentText().toStdString();

    if (! nmfUtilsQt::allCellsArePopulated(SSVPA_Tabs,SSVPAFleetDataTV,nmfConstants::ShowError))
        return false;

    // Remove any current records that have the same Species name, number, etc. Can't remove
    // all the records since that will remove other Species' records as well.
    cmd = "DELETE FROM " + TableName.toStdString() +
          " WHERE SpeName = '" + SpeciesName + "'" +
          " AND SpeIndex = "   + std::to_string(SpeciesIndex) +
          " AND SSVPAName = '" + SSVPAName + "'";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_Tab4_SaveFleetCatchData: Delete error: ", errorMsg);
        return false;
    }

    // Write the contents from the model into the MySQL table
    cmd  = "REPLACE INTO " + TableName.toStdString();
    cmd += " (SpeName, SpeIndex, SSVPAName, Fleet, Year, Effort) VALUES ";
    for (int fleet=0; fleet<Configure_EffortTuned_SetNumFleetsCMB->currentIndex(); ++fleet) {
        numRows = fleetModel[fleet]->rowCount();
        for (int j=0; j<numRows; ++j) {
            cmd += "(\"" + SpeciesName + "\",";
            cmd +=         std::to_string(SpeciesIndex) + ",";
            cmd += "\""  + SSVPAName + "\",";
            cmd +=         std::to_string(fleet) + ",";
            cmd +=         std::to_string(FirstYear+j) + ",";
            cmd +=         fleetModel[fleet]->index(j,0).data().toString().toStdString() + "), ";
        }
    }
    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_Tab4_SaveFleetEffortData: Write table error: ", errorMsg);
        return false;
    }

    // Last Restore CSV file
    fields = {"SpeName","SpeIndex","SSVPAName","Fleet","Year","Effort"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_Tab4_SaveFleetEffortData Complete");

    return true;

} // end Save_Tab4_SaveFleetEffortData


bool
nmfSSVPATab4::Save_SpeSSVPA(int SpeciesIndex)
{
    int SSVPAIndex;
    std::string cmd;
    std::string errorMsg;
    std::string SSVPAName;
    std::string SpeciesName;
    QString TableName = "SpeSSVPA";
    std::vector<std::string> fields;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_SpeSSVPA");

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    SSVPAName  = SelectConfigurationCMB->currentText().toStdString();
    SSVPAIndex = SelectConfigurationCMB->currentIndex()+1;


    // Remove any current records that have the same Species name, number, etc. Can't remove
    // all the records since that will remove other Species' records as well.
    cmd = "DELETE FROM " + TableName.toStdString() +
          " WHERE SpeIndex = "  + std::to_string(SpeciesIndex) +
          " AND SSVPAName  = '" + SSVPAName + "'" +
          " AND SSVPAIndex = "  + std::to_string(SSVPAIndex);
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_SpeSSVPA: Delete error: ", errorMsg);
        return false;
    }

    int Type           = SelectVPATypeCMB->currentIndex();
    int AgeM           = NaturalMortalityCB->isChecked();
    int TermZType      = 0; // RSK - not sure when this is filled out
    double TermF       = LastYearMortalityRateDSB->value();
    int RefAge         = ReferenceAgeSB->value();
    int LogCatchAge1   = 0; // RSK - not sure when this is filled out
    int LogCatchAge2   = 0; // RSK - not sure when this is filled out
    int FullRecAge     = Configure_Cohort_SB1->value();
    int PartSVPA       = 0; // RSK - not sure when this is filled out
    int pSVPANCatYrs   = Configure_Cohort_SB2->value();
    int pSVPANMortYrs  = Configure_Cohort_SB3->value();
    int pSVPARefAge    = Configure_Cohort_SB4->value();
    int NFleets        = Configure_EffortTuned_SetNumFleetsCMB->currentIndex();
    int NXSAIndex      = SSVPAExtendedIndicesCMB->currentIndex() +1;
    int Downweight     = Configure_Downweight_GB->isChecked();
    int DownweightType = Configure_XSA_CMB4->currentIndex();
    int DownweightYear = Configure_XSA_CMB5->currentText().toInt();
    int Shrink         = Configure_Shrinkage_GB->isChecked();
    double ShrinkCV    = Configure_XSA_LE1->text().toDouble();
    int ShrinkYears    = Configure_XSA_NYears_SB->value();
    int ShrinkAge      = Configure_XSA_NAges_SB->value();

    // Only save NXSAIndex if Type != Extended Survivors Analysis
std::cout << "Save_SpeSSVPA: NXSAIndex: " << NXSAIndex << std::endl;
    if ((NXSAIndex < 1 ) || (Type != 3))
        NXSAIndex = 1;


    // Write the contents from the model into the MySQL table
    cmd  = "REPLACE INTO " + TableName.toStdString();
    cmd += " (SpeIndex,SSVPAName,SSVPAIndex,Type,AgeM,TermZType,TermF,RefAge,";
    cmd += "LogCatchAge1,LogCatchAge2,FullRecAge,PartSVPA,pSVPANCatYrs,pSVPANMortYrs,";
    cmd += "pSVPARefAge,NFleets,NXSAIndex,Downweight,DownweightType,DownweightYear,";
    cmd += "Shrink,ShrinkCV,ShrinkYears,ShrinkAge) VALUES ";

    cmd += "(" + std::to_string(SpeciesIndex) + ",";
    cmd += "\"" + SSVPAName                   + "\",";
    cmd +=  std::to_string(SSVPAIndex)        + ",";
    cmd +=  std::to_string(Type)              + ",";
    cmd +=  std::to_string(AgeM)              + ",";
    cmd +=  std::to_string(TermZType)         + ",";
    cmd +=  std::to_string(TermF)             + ",";
    cmd +=  std::to_string(RefAge)            + ",";
    cmd +=  std::to_string(LogCatchAge1)      + ",";
    cmd +=  std::to_string(LogCatchAge2)      + ",";
    cmd +=  std::to_string(FullRecAge)        + ",";
    cmd +=  std::to_string(PartSVPA)          + ",";
    cmd +=  std::to_string(pSVPANCatYrs)      + ",";
    cmd +=  std::to_string(pSVPANMortYrs)     + ",";
    cmd +=  std::to_string(pSVPARefAge)       + ",";
    cmd +=  std::to_string(NFleets)           + ",";
    cmd +=  std::to_string(NXSAIndex)         + ",";
    cmd +=  std::to_string(Downweight)        + ",";
    cmd +=  std::to_string(DownweightType)    + ",";
    cmd +=  std::to_string(DownweightYear)    + ",";
    cmd +=  std::to_string(Shrink)            + ",";
    cmd +=  std::to_string(ShrinkCV)          + ",";
    cmd +=  std::to_string(ShrinkYears)       + ",";
    cmd +=  std::to_string(ShrinkAge)         + ")";

    // Remove last two characters of cmd string since we have an extra ", "
    //cmd = cmd.substr(0,cmd.size()-2);
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_SpeSSVPA: Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        return false;
    }

    // Last Restore CSV file
    fields = {"SpeIndex","SSVPAName","SSVPAIndex","Type","AgeM","TermZType","TermF","RefAge",
              "LogCatchAge1","LogCatchAge2","FullRecAge","PartSVPA","pSVPANCatYrs","pSVPANMortYrs",
              "pSVPARefAge","NFleets","NXSAIndex","Downweight","DownweightType","DownweightYear",
              "Shrink","ShrinkCV","ShrinkYears","ShrinkAge"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_SpeSSVPA Complete");

    return true;

} // end Save_SpeSSVPA


bool
nmfSSVPATab4::Save_InitialSelectivityAndMortality(int SpeciesIndex)
{
    int SSVPAIndex;
    std::string cmd;
    std::string errorMsg;
    std::string SSVPAName;
    std::string SpeciesName;
    QString TableName = "SSVPAAgeM";
    std::vector<std::string> fields;
    nmfMortalityModel *mortalityModel;
    nmfInitialSelectivityModel *selectivityModel;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_InitialSelectivityAndMortality");

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    SSVPAName  = SelectConfigurationCMB->currentText().toStdString();
    SSVPAIndex = SelectConfigurationCMB->currentIndex()+1;

    if (! nmfUtilsQt::allCellsArePopulated(SSVPA_Tabs,
                                           SSVPAMortalityTV,
                                           nmfConstants::ShowError))
        return false;

    // Remove any current records that have the same SpeIndex
    cmd.clear();
    for (int age=0; age <= MaxCatchAge; ++age) {
        cmd += "DELETE FROM " + TableName.toStdString() +
                " WHERE SpeIndex = " + std::to_string(SpeciesIndex) +
                " AND SSVPAIndex = " + std::to_string(SSVPAIndex)   +  "; ";
                //" AND Age = "        + std::to_string(age) + "; ";
    }
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_InitialSelectivityAndMortality: Delete error: ", errorMsg);
        std::cout << cmd << std::endl;
        return false;
    }

    bool ageSpecificMortalityEnabled = NaturalMortalityCB->isChecked();
    double M1 = ResidualNaturalMortalityDSB->value();
    double M2 = PredationMortalityDSB->value();

    // Write the contents from the model into the MySQL table
    cmd  = "REPLACE INTO " + TableName.toStdString();
    cmd += " (SpeIndex,SSVPAIndex,SSVPAName,Age,AgeM1,AgeM2,PRF) VALUES ";

    mortalityModel  = dynamic_cast<nmfMortalityModel *>(SSVPAMortalityTV->model());
    if (mortalityModel == nullptr) {
        logger->logMsg(nmfConstants::Error,"nmfSSVPATab4::Save_InitialSelectivityAndMortality Couldn't cast SSVPAMortalityTV->model()");
        return false;
    }
    selectivityModel = dynamic_cast<nmfInitialSelectivityModel *>(SSVPAInitialSelectivityTV->model());
    if (selectivityModel == nullptr) {
        logger->logMsg(nmfConstants::Error,"nmfSSVPATab4::Save_InitialSelectivityAndMortality Couldn't cast SSVPAInitialSelectivityTV->model()");
        return false;
    }

    int numRows = mortalityModel->count();
    for (int age=0; age<numRows; ++age) {
        cmd += "(" + std::to_string(SpeciesIndex) + ",";
        cmd +=  std::to_string(SSVPAIndex)    + ",";
        cmd += "\"" + SSVPAName               + "\",";
        cmd +=  std::to_string(age)           + ",";
        if (ageSpecificMortalityEnabled) {
            cmd +=  mortalityModel->index(age,1).data().toString().toStdString()  + ",";
            cmd +=  mortalityModel->index(age,2).data().toString().toStdString()  + ",";
        } else {
            cmd += std::to_string(M1) + ",";
            cmd += std::to_string(M2) + ",";
        }
        cmd +=  selectivityModel->index(age,1).data().toString().toStdString() + "), ";
    }

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_InitialSelectivityAndMortality: Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        return false;
    }

    // Last Restore CSV file
    fields = {"SpeIndex","SSVPAIndex","SSVPAName","Age","AgeM1","AgeM2","PRF"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_InitialSelectivityAndMortality Complete");

    return true;

} // end Save_InitialSelectivityAndMortality


bool
nmfSSVPATab4::Save_XSAIndex(int Index)
{
    std::string cmd;
    std::string Alpha;
    std::string Beta;
    std::string errorMsg;
    std::string SSVPAName;
    std::string IndexName;
    std::string SpeciesName;
    QString TableName = "SpeXSAIndices";
    std::vector<std::string> fields;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_XSAIndex");

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    SSVPAName = SelectConfigurationCMB->currentText().toStdString();

    Alpha     = SSVPAExtendedStartTimeLE->text().toStdString();
    Beta      = SSVPAExtendedEndTimeLE->text().toStdString();
    if (Alpha.empty() || Beta.empty()) {
        QMessageBox::warning(SSVPA_Tabs,
                             tr("Warning"),
                             tr("\nPlease enter values for both Start and End times before saving."),
                             QMessageBox::Ok);
        return false;
    }

    // Remove any current records that have the same Species index, .... Can't remove
    // all the records since that will remove other Species' records as well.
    cmd = "DELETE FROM " + TableName.toStdString() +
          " WHERE SpeIndex = " + std::to_string(SpeciesIndex) +
          " AND SSVPAName = '" + SSVPAName + "'" +
          " AND `Index` = " + std::to_string(Index);
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_XSAIndex: Delete error: ", errorMsg);
        return false;
    }

    IndexName = SSVPAExtendedIndicesCMB->currentText().toStdString();



    // Write the contents from the model into the MySQL table
    cmd  = "REPLACE INTO " + TableName.toStdString();
    cmd += " (SpeIndex, SSVPAName, `Index`, IndexName, Alpha, Beta) VALUES ";
    cmd +=  "("  + std::to_string(SpeciesIndex) + ",";
    cmd += "\""  + SSVPAName + "\",";
    cmd +=         std::to_string(Index) + ",";
    cmd += "\""  + IndexName + "\",";
    cmd +=         Alpha + ",";
    cmd +=         Beta + ")";

    // Remove last two characters of cmd string since we have an extra ", "
    //cmd = cmd.substr(0,cmd.size()-2);
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_XSAIndex: Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        return false;
    }

    // Restore CSV file
    fields = {"SpeIndex", "SSVPAName", "`Index`", "IndexName", "Alpha", "Beta"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_XSAIndex Complete");

    return true;

} // end Save_XSAIndex


bool
nmfSSVPATab4::Save_FleetCatchData(int NumFleets, std::string SpeciesName, int SpeciesIndex)
{
    int numRows;
    int numCols;
    std::string cmd;
    std::string errorMsg;
    std::string SSVPAName;
    QString TableName = "SpeTuneCatch";
    std::vector<std::string> fields;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_Tab4_SaveFleetCatchData");

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    SSVPAName = SelectConfigurationCMB->currentText().toStdString();

    if (! nmfUtilsQt::allCellsArePopulated(SSVPA_Tabs,SSVPAFleetDataTV,nmfConstants::ShowError))
        return false;

    // Remove any current records that have the same Species name, number, etc. Can't remove
    // all the records since that will remove other Species' records as well.
    cmd = "DELETE FROM " + TableName.toStdString() +
          " WHERE SpeName = '" + SpeciesName + "'" +
          " AND SpeIndex = "   + std::to_string(SpeciesIndex) +
          " AND SSVPAName = '" + SSVPAName + "'";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_Tab4_SaveFleetCatchData: Delete error: ", errorMsg);
        return false;
    }

    // Write the contents from the model into the MySQL table
    cmd  = "REPLACE INTO " + TableName.toStdString();
    cmd += " (SpeName, SpeIndex, SSVPAName, Fleet, Year, Age, Catch) VALUES ";
    for (int fleet=0; fleet<Configure_EffortTuned_SetNumFleetsCMB->currentIndex(); ++fleet) {
        numRows = fleetModel[fleet]->rowCount();
        numCols = fleetModel[fleet]->columnCount();
        for (int j=0; j<numRows; ++j) {
            for (int k=0; k<numCols-1; ++k) {
                cmd += "(\"" + SpeciesName + "\",";
                cmd +=         std::to_string(SpeciesIndex) + ",";
                cmd += "\""  + SSVPAName + "\",";
                cmd +=         std::to_string(fleet) + ",";
                cmd +=         std::to_string(FirstYear+j) + ",";
                cmd +=         std::to_string(MinCatchAge+k) + ",";
                cmd +=         fleetModel[fleet]->index(j,k+1).data().toString().toStdString() + "), ";
            }
        }
    }
    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_Tab4_SaveFleetCatchData: Write table error: ", errorMsg);
        return false;
    }

    // Last Restore CSV file
    fields = {"SpeName","SpeIndex","SSVPAName","Fleet","Year","Age","Catch"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_Tab4_SaveFleetCatchData Complete");

    return true;

} // end Save_Tab4_SaveFleetCatchData


/*
bool
nmfSSVPATab4::Save_FleetEffortDataOLD(int NumFleets, std::string SpeciesName, int SpeciesIndex)
{
    bool saveOK = true;
    bool FirstLine = true;
    QString fileName;
    QString filePath;
    QString dir;
    QString fileNameWithPath;
    QString tmpFileNameWithPath;
    QString qcmd;
    QString line;
    QStringList qfields;
    std::string cmd;
    std::string errorMsg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString TableName = "SpeTuneEffort";
std::cout << "Save_FleetEffortData" << std::endl;

    if (! nmfQtUtils::allCellsArePopulated(SSVPA_Tabs,SSVPAFleetDataTV,
                                           nmfConstants::ShowError)) {
        return false;
    }

    // Get some basic info that you'll need later on
    fields   = {"SpeIndex", "SpeName","FirstYear","LastYear"};
    queryStr = "SELECT SpeIndex,SpeName,FirstYear,lastYear FROM Species WHERE SpeIndex = " +
            std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    int FirstYear = std::stoi(dataMap["FirstYear"][0]);
    int LastYear  = std::stoi(dataMap["LastYear"][0]);
    fields   = {"SpeIndex", "SSVPAName"};
    queryStr = "SELECT SpeIndex,SSVPAName FROM SpeSSVPA WHERE SpeIndex = " + std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    std::string SSVPAName = dataMap["SSVPAName"][0];

    QString SpeFleetEffortCSVFile = "";

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (SpeFleetEffortCSVFile.isEmpty()) {
        SpeFleetEffortCSVFile = "SpeTuneEffort.csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPath    = QDir(filePath).filePath(SpeFleetEffortCSVFile);
        tmpFileNameWithPath = QDir(filePath).filePath("."+SpeFleetEffortCSVFile);
    } else {
        QFileInfo finfo(SpeFleetEffortCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPath    = SpeFleetEffortCSVFile;
        tmpFileNameWithPath = QDir(filePath).filePath("."+fileName);
    }

    // Read entire file and copy all lines that don't have the current Species Index into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.

    QFile fin(fileNameWithPath);
    QFile fout(tmpFileNameWithPath);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(3) Opening Input File", fin.errorString());
        return false;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(3) Opening Output File", fout.errorString());
        return false;
    }
    QTextStream inStream(&fin);
    QTextStream outStream(&fout);

    QString tableLine;
    bool ignoreSpecies = false;

    QString FileSpeIndex;
    double FileEffort;

    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");

        FileSpeIndex  = qfields[1].trimmed();
        if (FileSpeIndex == QString::number(SpeciesIndex))
        {
            // Skip over all other Species Index rows, since going through this once copies the qtableview data
            if (! ignoreSpecies)
            {
                for (int Fleet=0; Fleet < NumFleets; ++Fleet) {
                    for (int Year  = FirstYear; Year <= LastYear; ++Year) {
                        FileEffort = FleetData[Fleet][SpeciesIndex][Year-FirstYear][0];
                        tableLine = QString::fromStdString(SpeciesName)   + ", " +
                                    QString::number(SpeciesIndex)         + ", " +
                                    QString::fromStdString(SSVPAName) + ", " +
                                    QString::number(Fleet)            + ", " +
                                    QString::number(Year-FirstYear)   + ", " +
                                    QString::number(FileEffort);
                        outStream << tableLine << "\n";
std::cout << "tableLine1: " << tableLine.toStdString() << std::endl;

                    }
                }
                ignoreSpecies = true;
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
std::cout << "line1: " << line.toStdString() << std::endl;
        }
    }

    if (! ignoreSpecies) // Means that there was no Species data found...so add it
    {
std::cout << 1234 << std::endl;
std::cout << "NumFleets: " << NumFleets << std::endl;
std::cout << FirstYear << "," << LastYear << std::endl;

        for (int Fleet=0; Fleet < NumFleets; ++Fleet) {
            for (int Year  = FirstYear; Year <= LastYear; ++Year) {
std::cout << Fleet << "," << Year << std::endl;
                FileEffort = FleetData[Fleet][SpeciesIndex][Year-FirstYear][0];
                tableLine = QString::fromStdString(SpeciesName)   + ", " +
                            QString::number(SpeciesIndex)         + ", " +
                            QString::fromStdString(SSVPAName) + ", " +
                            QString::number(Fleet)            + ", " +
                            QString::number(Year-FirstYear)   + ", " +
                            QString::number(FileEffort);
                outStream << tableLine << "\n";
std::cout << "tableLine2: " << tableLine.toStdString() << std::endl;
            }
        }
    }
    fin.close();
    fout.close();

    int retv = nmfQtUtils::rename(tmpFileNameWithPath, fileNameWithPath);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPath.toStdString() << " to " <<
                     fileNameWithPath.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return false ;
    }


    //
    // Save to mysql table
    //

    // Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableName;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
        saveOK = false;
        nmfUtils::printError("SSVPA Save(5F): Clearing table error: ", errorMsg);
        return false;
    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableName.toStdString();
    cmd += "(SpeName, SpeIndex, SSVPAName, Fleet, Year, Effort) VALUES ";

    // Read from csv file (and not from the GUI) since it has all the Species in it.
    QFile fin2(fileNameWithPath);
    if(! fin2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(5F) Opening Input File", fin.errorString());
        return false;
    }
    QTextStream inStream2(&fin2);

    while (! inStream2.atEnd()) {
        line = inStream2.readLine();
std::cout << "line: " << line.toStdString() << std::endl;

        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields   = line.split(",");
        cmd += "(\"" + qfields[0].trimmed().toStdString() + "\",";
        cmd +=         qfields[1].trimmed().toStdString() + ",";
        cmd += "\""  + qfields[2].trimmed().toStdString() + "\",";
        cmd +=         qfields[3].trimmed().toStdString() + ",";
        cmd +=         qfields[4].trimmed().toStdString() + ",";
        cmd +=         qfields[5].trimmed().toStdString() + "), ";
    } // end for all rows in csv file

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    fin2.close();
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("SSVPA Save(5F): Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        return false;
    }

    return true;

} // end Save_FleetEffortData
*/

/*
bool
nmfSSVPATab4::Save_SpeXSAIndices(std::string SpeciesName, int SpeciesIndex)
{
    QString fileName;
    QString filePath;
    QString fileNameWithPath;
    QString tmpFileNameWithPath;
    QString line;
    QStringList qfields;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString TableName = "SpeXSAIndices";
    QString SpeXSAIndicesCSVFile = "";

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_SpeXSAIndices");

    if (! nmfQtUtils::allCellsArePopulated(SSVPA_Tabs,SSVPAIndicesTV,
                                           nmfConstants::ShowError))
        return false;

    // Build a Species name and index map, you'll need this.
    fields   = {"SpeIndex", "SSVPAName"};
    queryStr = "SELECT SpeIndex,SSVPAName FROM SpeSSVPA WHERE SpeIndex = " + std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    if (dataMap["SpeIndex"].size() == 0) {
        QMessageBox::information(SSVPA_Tabs,
                                 tr("Missing Data"),
                                 tr("\nPlease Load and Save SpeSSVPA.csv before continuing."),
                                 QMessageBox::Ok);
        CurrentTable = "SpeSSVPA";
        return false;
    }
    std::string SSVPAName = dataMap["SSVPAName"][0];

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (SpeXSAIndicesCSVFile.isEmpty()) {
        SpeXSAIndicesCSVFile = TableName + ".csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPath    = QDir(filePath).filePath(SpeXSAIndicesCSVFile);
        tmpFileNameWithPath = QDir(filePath).filePath("."+SpeXSAIndicesCSVFile);
    } else {
        QFileInfo finfo(SpeXSAIndicesCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPath    = SpeXSAIndicesCSVFile;
        tmpFileNameWithPath = QDir(filePath).filePath("."+fileName);
    }

    // Read entire file and copy all lines that don't have the current Species Index into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.
    QFile fin(fileNameWithPath);
    QFile fout(tmpFileNameWithPath);

    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(4) Opening Input File", fin.errorString());
        return false;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(4) Opening Output File", fout.errorString());
        return false;
    }
    QTextStream inStream(&fin);
    QTextStream outStream(&fout);

    QString tableLine;
    bool ignoreSpecies = false;

    QString csvSpeciesIndex,csvSSVPAName,csvIndex,csvIndexName,csvAlpha,csvBeta;
    int NumIndices = SSVPAExtendedIndicesCMB->count();
    while (! inStream.atEnd()) {
        line = inStream.readLine().trimmed();
        if (line.isEmpty())
            break;
        qfields = line.split(",");
        csvSpeciesIndex = qfields[0].trimmed();
        csvSSVPAName    = qfields[1].trimmed();
        csvIndex        = qfields[2].trimmed();
        csvIndexName    = qfields[3].trimmed();
        csvAlpha        = qfields[4].trimmed();
        csvBeta         = qfields[5].trimmed();

        if (csvSpeciesIndex == QString::number(SpeciesIndex)) { // dump qtableview data into output file
            // Skip over all other Species Index rows, since going through this once copies the qtableview data
            if (! ignoreSpecies) {
                for (int i  = 0; i < NumIndices; ++i)
                {
                    tableLine = QString::number(SpeciesIndex) + ", " +
                                QString::fromStdString(SSVPAName) + ", " +
                                QString::number(i) + ", " +
                                SSVPAExtendedIndicesCMB->itemText(i) + ", " +
                                QString::number(Alpha[i]) + ", " +
                                QString::number(Beta[i]);
                    outStream << tableLine << "\n";
                }
                ignoreSpecies = true;
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }

    } // end while

    if (! ignoreSpecies) {
        for (int i  = 0; i < NumIndices; ++i)
        {
            tableLine = QString::number(SpeciesIndex) + ", " +
                    QString::fromStdString(SSVPAName) + ", " +
                    QString::number(i) + ", " +
                    SSVPAExtendedIndicesCMB->itemText(i) + ", " +
                    QString::number(Alpha[i]) + ", " +
                    QString::number(Beta[i]);
            outStream << tableLine << "\n";
        }
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    int retv = nmfQtUtils::rename(tmpFileNameWithPath, fileNameWithPath);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPath.toStdString() << " to " <<
                     fileNameWithPath.toStdString() <<
                     ". Save aborted." << std::endl;
        return false;
    }


    //
    // Save to mysql table
    //
    SaveToSpeXSAIndicesDatabaseTable(TableName,fileNameWithPath);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_SpeXSAIndices Complete");

    return true;

} // end Save_SpeXSAIndices


bool
nmfSSVPATab4::SaveToSpeXSAIndicesDatabaseTable(QString TableName,
                                               QString fileNameWithPath)
{
    QString qcmd;
    QString line;
    QStringList qfields;
    std::string cmd;
    std::string errorMsg;

    // Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableName;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("SSVPA Save(5E1): Clearing table error: ", errorMsg);
        return false;
    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableName.toStdString();
    cmd += " (SpeIndex, SSVPAName, `Index`, IndexName, Alpha, Beta) VALUES ";

    // Read from csv file (and not from the GUI) since it has all the Species in it.
    QFile fin(fileNameWithPath);
    if(! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(5E1) Opening Input File", fin.errorString());
        return false;
    }
    QTextStream inStream(&fin);
    bool FirstLine = true;
    while (! inStream.atEnd()) {
        line = inStream.readLine().trimmed();
        if (line.isEmpty())
            continue;
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields = line.split(",");
        cmd += "("  + qfields[0].trimmed().toStdString() + ",";
        cmd += "\"" + qfields[1].trimmed().toStdString() + "\",";
        cmd +=        qfields[2].trimmed().toStdString() + ",";
        cmd += "\"" + qfields[3].trimmed().toStdString() + "\",";
        cmd +=        qfields[4].trimmed().toStdString() + ",";
        cmd +=        qfields[5].trimmed().toStdString() + "), ";
    } // end for all rows in csv file

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    fin.close();
//std::cout << "cmd: \n" << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("SSVPA Save(5E1): Write table error: ", errorMsg);
        return false;
    }

//    QMessageBox::information(SSVPA_Tabs, "Save",
//                             tr("\nXSA Indices data saved.\n"));

    return true;

} // end SaveToSpeXSAIndicesDatabaseTable
*/


QString
nmfSSVPATab4::Save_XSAIndexData()
{
    int SpeciesIndex;
    int numRows;
    int numCols;
    int NumIndices;
    int dbIndex;
    int currIndex;
    std::string cmd;
    std::string errorMsg;
    std::string SSVPAName;
    std::string SpeciesName;
    QString TableName = "SpeXSAData";
    QString retv = "\nXSA Index Data:    Failed. Check data entered.";
    std::vector<std::string> fields;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_XSAIndexData");

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    SSVPAName  = SelectConfigurationCMB->currentText().toStdString();
    NumIndices = SSVPAExtendedIndicesCMB->count();
    if (NumIndices == 0) {
        QMessageBox::warning(SSVPA_Tabs,
                    tr("Error"),
                    tr("\nPlease create at least 1 index prior to saving this XSA configuration."),
                    QMessageBox::Ok);
        return retv;
    }

    currIndex = SSVPAExtendedIndicesCMB->currentIndex();

    // Check all index models for missing data.
    for (int i=0; i<NumIndices; ++i) {
        SSVPAExtendedIndicesCMB->setCurrentIndex(i);
        if (! nmfUtilsQt::allCellsArePopulated(SSVPA_Tabs,SSVPAIndicesTV,nmfConstants::ShowError)) {
            return retv;
        }
    }
    SSVPAExtendedIndicesCMB->setCurrentIndex(currIndex);

    // Remove any current records that have the same Species number.
    // all the records since that will remove other Species' records as well.
    for (int index=0; index<NumIndices; ++index) {
        cmd = "DELETE FROM " + TableName.toStdString() +
                " WHERE SpeIndex = "  + std::to_string(SpeciesIndex) +
                " AND `Index` = "     + std::to_string(index);
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if (nmfUtilsQt::isAnError(errorMsg)) {
            nmfUtils::printError("Error Save_XSAIndexData: Delete error: ", errorMsg);
            return retv;
        }
    }

    // Write the contents from the model into the MySQL table
    cmd  = "REPLACE INTO " + TableName.toStdString();
    cmd += " (SpeIndex,SSVPAName,`Index`,Year,Age,Value) VALUES ";
    for (int index=0; index<NumIndices; ++index) {
        numRows = xsaModel[index]->rowCount();
        numCols = xsaModel[index]->columnCount();
        for (int j=0; j<numRows; ++j) {
            for (int k=0; k<numCols; ++k) {
                cmd += "(" + std::to_string(SpeciesIndex) + ",";
                cmd += "\"" + SSVPAName            + "\",";
                cmd +=  std::to_string(index)    + ",";
                cmd +=  std::to_string(j)       + ",";
                cmd +=  std::to_string(k)        + ",";
                cmd +=  xsaModel[index]->index(j,k).data().toString().toStdString() + "), ";
            }
        }
    }

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_XSAIndexData: Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        return retv;
    }

    // Last Restore CSV file
    fields = {"SpeIndex","SSVPAName","`Index`","Year","Age","Value"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    // -------------------------------------------------

    //
    // Update SpeXSAIndices with Alpha and Beta values
    //
    // Write the contents from the model into the MySQL table
    TableName = "SpeXSAIndices";
    cmd  = "REPLACE INTO " + TableName.toStdString();
    cmd += " (SpeIndex,SSVPAName,`Index`,IndexName,Alpha,Beta) VALUES ";
    for (int i=0; i<NumIndices; ++i) {
        cmd += "(" + std::to_string(SpeciesIndex) + ",";
        cmd += "\"" + SSVPAName           + "\",";
        cmd +=  std::to_string(i)       + ",";  // Index starts with 0.
        cmd += "\"" + SSVPAExtendedIndicesCMB->itemText(i).toStdString() + "\",";
        cmd +=  std::to_string(Alpha[i])  + ",";
        cmd +=  std::to_string(Beta[i])   + "), ";
    }
    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Save_XSAIndexData Alpha/Beta: Write table error: ", errorMsg);
        std::cout << cmd << std::endl;
        return retv;
    }

    // Last Restore CSV file
    fields = {"SpeIndex","SSVPAName","`Index`","IndexName","Alpha","Beta"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);


    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_XSAIndexData Complete");

    return "\nXSA Index Data:    OK";

} // end Save_XSAIndexData


/*
bool
nmfSSVPATab4::Save_SpeXSAData(std::string SpeciesName, int SpeciesIndex)
{
    bool saveOK = true;
    bool FirstLine = true;
    QString fileName;
    QString filePath;
    QString dir;
    QString fileNameWithPath;
    QString tmpFileNameWithPath;
    QString qcmd;
    QString line;
    QStringList qfields;
    std::string cmd;
    std::string errorMsg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString TableName = "SpeXSAData";
    QString SpeSpeXSADataCSVFile = "";

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_SpeXSAData");

    if (! nmfQtUtils::allCellsArePopulated(SSVPA_Tabs,SSVPAIndicesTV,
                                           nmfConstants::ShowError))
        return false;

    // Load basic species data that you'll need later
    fields   = {"SpeIndex", "SSVPAName"};
    queryStr = "SELECT SpeIndex,SSVPAName FROM SpeSSVPA WHERE SpeIndex = " + std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    std::string SSVPAName = dataMap["SSVPAName"][0];

    fields   = {"SpeIndex", "SpeName","MinCatAge","MaxCatAge","FirstYear","LastYear"};
    queryStr = "SELECT SpeIndex,SpeName,MinCatAge,MaxCatAge,FirstYear,LastYear FROM Species WHERE SpeIndex = " +
            std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    //int MinCatchAge = std::stoi(dataMap["MinCatAge"][0]);
    int MaxCatchAge = std::stoi(dataMap["MaxCatAge"][0]);
    int FirstYear   = std::stoi(dataMap["FirstYear"][0]);
    int LastYear    = std::stoi(dataMap["LastYear"][0]);

    // Save back to csv file in case user changed anything inline.
    // Find filename for .csv file and for the temp file you'll write to for updating.
    if (SpeSpeXSADataCSVFile.isEmpty()) {
        SpeSpeXSADataCSVFile = TableName + ".csv";
        filePath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
        fileNameWithPath    = QDir(filePath).filePath(SpeSpeXSADataCSVFile);
        tmpFileNameWithPath = QDir(filePath).filePath("."+SpeSpeXSADataCSVFile);
    } else {
        QFileInfo finfo(SpeSpeXSADataCSVFile);
        fileName = finfo.fileName();
        filePath = finfo.path();
        fileNameWithPath    = SpeSpeXSADataCSVFile;
        tmpFileNameWithPath = QDir(filePath).filePath("."+fileName);
    }

    // Read entire file and copy all lines that don't have the current Species Index into temp output file which begins with ".".
    // For the lines that need updating, read the data from the qtableview, create a line out output, and write it to
    // the output file.

    QFile fin(fileNameWithPath);
    QFile fout(tmpFileNameWithPath);
    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(1a) Opening Input File", fin.errorString());
        return false;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(1a) Opening Output File", fout.errorString());
        return false;
    }
    QTextStream inStream(&fin);
    QTextStream outStream(&fout);

    QString tableLine;
    bool ignoreSpecies = false;

    QString csvSpeciesIndex,csvSSVPAName,csvIndex,csvYear,csvAge,csvValue;
    int NumIndices = SSVPAExtendedIndicesCMB->count();
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvSpeciesIndex = qfields[0].trimmed();
        csvSSVPAName    = qfields[1].trimmed();
        csvIndex        = qfields[2].trimmed();
        csvYear         = qfields[3].trimmed();
        csvAge          = qfields[4].trimmed();
        csvValue        = qfields[5].trimmed();

        if (csvSpeciesIndex == QString::number(SpeciesIndex)) { // dump qtableview data into output file
            // Skip over all other Species Index rows, since going through this once copies the qtableview data
            if (! ignoreSpecies) {
                for (int i  = 0; i < NumIndices; ++i) {
                    for (int Year=0; Year <= LastYear-FirstYear; ++Year) {
                        for (int Age=0; Age<=MaxCatchAge; ++Age) {
                            tableLine = QString::number(SpeciesIndex)            + ", " +
                                        QString::fromStdString(SSVPAName)    + ", " +
                                        QString::number(i)                   + ", " +
                                        QString::number(Year)                + ", " +
                                        QString::number(Age)                 + ", " +
                                        QString::number(IndexData[SpeciesIndex][i][Year][Age]);
                            outStream << tableLine << "\n";
                        }
                    }
                }
                ignoreSpecies = true;
            }
        } else { // if it's not data for the current Species, copy it to the new "." file
            outStream << line << "\n";
        }
    }
    if (! ignoreSpecies) {
        for (int i  = 0; i < NumIndices; ++i) {
            for (int Year=0; Year <= LastYear-FirstYear; ++Year) {
                for (int Age=0; Age<=MaxCatchAge; ++Age) {
                    tableLine = QString::number(SpeciesIndex)            + ", " +
                                QString::fromStdString(SSVPAName)    + ", " +
                                QString::number(i)                   + ", " +
                                QString::number(Year)                + ", " +
                                QString::number(Age)                 + ", " +
                                QString::number(IndexData[SpeciesIndex][i][Year][Age]);
                    outStream << tableLine << "\n";
                }
            }
        }
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    int retv = nmfQtUtils::rename(tmpFileNameWithPath, fileNameWithPath);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPath.toStdString() << " to " <<
                     fileNameWithPath.toStdString() <<
                     ". Save aborted." << std::endl;
        saveOK = false;
        return false ;
    }


    //
    // Save to mysql table
    //

    // Clear the current table contents
    qcmd = "TRUNCATE TABLE " + TableName;
    errorMsg = databasePtr->nmfUpdateDatabase(qcmd.toStdString());
    if (nmfUtilsQt::isAnError(errorMsg)) {
        saveOK = false;
        nmfUtils::printError("SSVPA Save(5E2): Clearing table error: ", errorMsg);
        return false;
    }

    // Write the contents from the CSV file into the MySQL table
    cmd  = "INSERT INTO " + TableName.toStdString();
    cmd += "(SpeIndex, SSVPAName, `Index`, Year, Age, Value) VALUES ";

    // Read from csv file (and not from the GUI) since it has all the Species in it.
    QFile fin2(fileNameWithPath);
    if(! fin2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(5E2) Opening Input File", fin.errorString());
        return false;
    }
    QTextStream inStream2(&fin2);

    while (! inStream2.atEnd()) {
        line = inStream2.readLine();
        if (FirstLine) { // skip first line since it's a header
            FirstLine = false;
            continue;
        }
        qfields = line.split(",");
        cmd += "("  + qfields[0].trimmed().toStdString() + ",";
        cmd += "\"" + qfields[1].trimmed().toStdString() + "\",";
        cmd +=        qfields[2].trimmed().toStdString() + ",";
        cmd +=        qfields[3].trimmed().toStdString() + ",";
        cmd +=        qfields[4].trimmed().toStdString() + ",";
        cmd +=        qfields[5].trimmed().toStdString() + "), ";
    } // end for all rows in csv file

    // Remove last two characters of cmd string since we have an extra ", "
    cmd = cmd.substr(0,cmd.size()-2);
    fin2.close();
//std::cout << cmd << std::endl;
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("SSVPA Save(5E2): Write table error: ", errorMsg);
        return false;
    }

    //SSVPA_Tab4_CheckAllPB->setEnabled(true);
//    if (saveOK) {
//        QMessageBox::information(SSVPA_Tabs, "Save",
//                                 tr("\nXSA Table Data saved.\n"));
//    }

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Save_SpeXSAData Complete");

    return true;

} // end Save_SpeXSAData
*/


bool
nmfSSVPATab4::Load_XSAData(QString filename)
{
    bool readFirstLine = false;
    int NumIndices;
    int idx;
    int FileSpeIndex,FileIndex,FileYear,FileAge;
    int SpeciesIndex;
    double FileValue;
    QString line;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QString SpeXSADataCSVFile;
    QString FileSSVPAName;
    QStringList qfields;
    QStringList NameFilters;
    QFileDialog *fileDlg;
    QModelIndex index;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    std::string SpeciesName;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Load_XSAData: " + filename.toStdString());

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    if (filename.isEmpty()) {
        // Setup Load dialog for SpeXSAData
        fileDlg = new QFileDialog(SSVPA_Tabs);
        fileDlg->setDirectory(path);
        fileDlg->selectFile("SpeXSAData.csv");
        NameFilters << "*.csv" << "*.*";
        fileDlg->setNameFilters(NameFilters);
        fileDlg->setWindowTitle("Load XSA Data CSV File");
        if (fileDlg->exec()) {
            // Open the file here....
            SpeXSADataCSVFile = fileDlg->selectedFiles()[0];
        } else {
            SpeXSADataCSVFile = "";
        }
        delete fileDlg;
    } else {
        SpeXSADataCSVFile = filename;
    }
    if (SpeXSADataCSVFile.isEmpty())
        return false;

    QFile file(SpeXSADataCSVFile);
    if (! file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "File Read Error", file.errorString());
        return false;
    }
    QTextStream inStream(&file);
    // Load the model from the data in the csv file
    NumIndices = SSVPAExtendedIndicesCMB->count();
    for (int i=0; i<NumIndices; ++i) {
        InitializeXSAIndex(i);
    }

    while (! inStream.atEnd()) {
        line = inStream.readLine();
        if (readFirstLine) { // skip row=0 as it's the header
            qfields = line.split(",");
            FileSpeIndex  = qfields[0].trimmed().toInt();
            FileSSVPAName = qfields[1].trimmed();
            FileIndex     = qfields[2].trimmed().toInt(); // Starts with 0 in table
            FileYear      = qfields[3].trimmed().toInt();
            FileAge       = qfields[4].trimmed().toInt();
            FileValue     = qfields[5].trimmed().toDouble();
            if (FileSpeIndex == SpeciesIndex) {
                if (xsaModel[FileIndex] != NULL) {
                    index = xsaModel[FileIndex]->index(FileYear,FileAge);
                    xsaModel[FileIndex]->setData(index,QString::number(FileValue));
                }
            }
        }
        readFirstLine = true;
    }
    file.close();

    // Find which index should be selected
    fields     = {"SpeIndex","NXSAIndex"};
    queryStr   = "SELECT SpeIndex,NXSAIndex from SpeSSVPA";
    queryStr  += " WHERE SpeIndex = " + std::to_string(SpeciesIndex);
    dataMap    = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap.size() > 0) {
        idx = std::stoi(dataMap["NXSAIndex"][0]);
        if (idx < 1)
            idx = 1;
        SSVPAExtendedIndicesCMB->setCurrentIndex(idx-1);
        SSVPAIndicesTV->setModel(xsaModel[idx-1]);
    }

    return true;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Load_XSAData Complete");

} // end Load_XSAData

/*
void
nmfSSVPATab4::Load_SpeXSAData()
{
    QString line;
    QStringList fields;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QStringList NameFilters;
    QString SpeXSADataCSVFile;
    QString FileSSVPAName;
    int FileSpeIndex,FileIndex;
    double FileYear,FileAge,FileValue;
    bool readFirstLine = false;
    QFileDialog *fileDlg;
    fileDlg = new QFileDialog(SSVPA_Tabs);

    // Setup Load dialog for SpeXSAData
    fileDlg->setDirectory(path);
    fileDlg->selectFile("SpeXSAData.csv");
    NameFilters << "*.csv" << "*.*";
    fileDlg->setNameFilters(NameFilters);
    fileDlg->setWindowTitle("Load XSA Data CSV File");
    if (fileDlg->exec()) {
        // Open the file here....
        SpeXSADataCSVFile = fileDlg->selectedFiles()[0];
        QFile file(SpeXSADataCSVFile);
        if (! file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(SSVPA_Tabs, "File Read Error", file.errorString());
            return;
        }
        QTextStream inStream(&file);

        // Load the model from the data in the csv file
        while (! inStream.atEnd()) {
            line = inStream.readLine();
            if (readFirstLine) { // skip row=0 as it's the header
                fields = line.split(",");
                FileSpeIndex  = fields[0].toInt();
                FileSSVPAName = fields[1];
                FileIndex     = fields[2].toInt();
                FileYear      = fields[3].toInt();
                FileAge       = fields[4].toInt();
                FileValue     = fields[5].toDouble();
                IndexData[FileSpeIndex][FileIndex][FileYear][FileAge] = FileValue;
            }
            readFirstLine = true;
        }
        file.close();

        callback_XSA_IndexCMB(0);

    } // end if fileDlg

    delete fileDlg;

} // end Load_SpeXSAData
*/

bool
nmfSSVPATab4::Load_XSAIndices(QString filename)
{
//    int index;
    int SpeciesIndex;
    QString line;
    QStringList qfields;
    QString path = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QStringList NameFilters;
    QString SpeXSAIndicesCSVFile;
    QString FileSSVPAName,FileIndexName;
    int FileSpeIndex; // ,FileIndex;
    double FileAlpha,FileBeta;
    bool readFirstLine = false;
    QFileDialog *fileDlg;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    std::string SpeciesName;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Load_XSAIndices: " + filename.toStdString());

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    Alpha.clear();
    Beta.clear();

    if (filename.isEmpty()) {
        // Setup Load dialog for SpeXSAIndices
        fileDlg = new QFileDialog(SSVPA_Tabs);
        fileDlg->setDirectory(path);
        fileDlg->selectFile("SpeXSAIndices.csv");
        NameFilters << "*.csv" << "*.*";
        fileDlg->setNameFilters(NameFilters);
        fileDlg->setWindowTitle("Load XSA Indices CSV File");
        if (fileDlg->exec()) {
            // Open the file here....
            SpeXSAIndicesCSVFile = fileDlg->selectedFiles()[0];
        } else {
            SpeXSAIndicesCSVFile = "";
        }
        delete fileDlg;
    } else {
        SpeXSAIndicesCSVFile = filename;
    }
    if (SpeXSAIndicesCSVFile.isEmpty())
        return false;
    QFile file(SpeXSAIndicesCSVFile);
    if (! file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "File Read Error", file.errorString());
        return false;
    }
    QTextStream inStream(&file);

    // Load the model from the data in the csv file
//    index = 0;
    SSVPAExtendedIndicesCMB->clear();
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        if (readFirstLine) { // skip row=0 as it's the header
            qfields = line.split(",");
            FileSpeIndex  = qfields[0].trimmed().toInt();
            FileSSVPAName = qfields[1].trimmed();
//            FileIndex     = qfields[2].trimmed().toInt();
            FileIndexName = qfields[3].trimmed();
            FileAlpha     = qfields[4].trimmed().toDouble();
            FileBeta      = qfields[5].trimmed().toDouble();
            if (FileSpeIndex == SpeciesIndex) {
                Alpha.push_back(FileAlpha);
                Beta.push_back(FileBeta);
                SSVPAExtendedIndicesCMB->blockSignals(true);
                SSVPAExtendedIndicesCMB->addItem(FileIndexName);
                SSVPAExtendedIndicesCMB->blockSignals(false);
                //InitializeXSAIndex(index++);
            }
        }
        readFirstLine = true;
    }
    file.close();

    return true;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Load_XSAIndices Complete");

} // end Load_XSAIndices



void
nmfSSVPATab4::callback_SSVPA_Tab4_LoadIndicesPB(bool unused)
{
    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::callback_SSVPA_Tab4_LoadIndicesPB");

    bool loadedIndicesOK;
    //bool loadedDataOK;

    loadedIndicesOK = Load_XSAIndices("");
    if (loadedIndicesOK) {
        Load_XSAData("");
    }

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::callback_SSVPA_Tab4_LoadIndicesPB Complete");

} // callback_SSVPA_Tab4_LoadIndicesPB


QString
nmfSSVPATab4::Save_InitialSelectivity()
{
    std::string SpeciesName;
    int SpeciesIndex;
    QString retv = "\nInit Sel Data:     Failed. Check data entered.";
    bool dataSavedOK;

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    dataSavedOK = Save_InitialSelectivityAndMortality(SpeciesIndex);

    retv = (dataSavedOK) ? "\nInit Sel Data:     OK" : "\nInit Sel Data:     Failed. Check data entered.";

    return retv;

} // end Save_InitialSelectivity


/*
QString
nmfSSVPATab4::SSVPA_Tab4_SaveIndices()
{    
    std::string SpeciesName;
    int SpeciesIndex;
    bool indicesSavedOK;
    bool dataSavedOK;
    QString retv;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::SSVPA_Tab4_SaveIndicesPB");

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    if (SSVPAExtendedIndicesCMB->count() == 0) {
        QMessageBox::information(SSVPA_Tabs,
                                 tr("Save"),
                                 tr("\nThere must be at least one Index to save."),
                                 QMessageBox::Ok);
        return "\nXSA Indices:       Failed. There must be at least one Index to save.";
    }

    //indicesSavedOK = SSVPA_Tab4_SaveXSAIndex();

    // First save to the CSV file and table: SpeXSAIndices.csv and SpeXSAIndices
    //indicesSavedOK = Save_SpeXSAIndices(SpeciesName, SpeciesIndex);
    // Next save to the CSV file and table: SpeXSAData.csv and SpeXSAData
//    if (indicesSavedOK)
//        dataSavedOK = Save_SpeXSAData(SpeciesName, SpeciesIndex);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::SSVPA_Tab4_SaveIndicesPB Complete");

    retv  = (indicesSavedOK) ? "\nXSA Indices:       OK" : "\nXSA Indices:       Failed. Check data entered.";
    retv += (dataSavedOK)    ? "\nXSA Data:          OK" : "\nXSA Data:          Failed. Check data entered.";

    return retv;

} // end SSVPA_Tab4_SaveIndices
*/

bool
nmfSSVPATab4::runDBChecks(QString type,
                          QString &BadTables)
{
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    bool AllChecksOK = true;
    std::string SpeciesName;
    int SpeciesIndex;

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    // 1. Check SSVPAAgeM: SpeIndex AgeM1.size()
    fields   = {"SpeIndex","AgeM1","AgeM2"};
    queryStr = "SELECT SpeIndex,AgeM1,AgeM2 FROM SSVPAAgeM WHERE SpeIndex=" + std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if ((dataMap["AgeM1"].size() == 0) || (dataMap["AgeM2"].size() == 0)) {
        AllChecksOK = false;
        BadTables += "SSVPAAgeM.csv\n";
    }

    // 2. Check SpeCatch: SpeIndex Catch
    fields   = {"SpeIndex","Catch"};
    queryStr = "SELECT SpeIndex,Catch FROM SpeCatch WHERE SpeIndex=" + std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["Catch"].size() == 0) {
        AllChecksOK = false;
        BadTables += "SpeCatch.csv\n";
    }

    // 3. Check SpeMaturity: SpeIndex PMature
    fields   = {"SpeIndex","PMature"};
    queryStr = "SELECT SpeIndex,PMature FROM SpeMaturity WHERE SpeIndex=" + std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["PMature"].size() == 0) {
        AllChecksOK = false;
        BadTables += "SpeMaturity.csv\n";
    }

    // 4. Check SpeSSVPA: SpeIndex SSVPAName
    fields   = {"SpeIndex","SSVPAName"};
    queryStr = "SELECT SpeIndex,SSVPAName FROM SpeSSVPA WHERE SpeIndex=" + std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["SSVPAName"].size() == 0) {
        AllChecksOK = false;
        BadTables += "SpeSSVPA.csv\n";
    }

    // 5. Check SpeSize: SpeIndex Value
    fields   = {"SpeIndex","Value"};
    queryStr = "SELECT SpeIndex,Value FROM SpeSize WHERE SpeIndex=" + std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["Value"].size() == 0) {
        AllChecksOK = false;
        BadTables += "SpeSize.csv\n";
    }

    // 6. Check SpeWeight: SpeIndex Value
    fields   = {"SpeIndex","Value"};
    queryStr = "SELECT SpeIndex,Value FROM SpeWeight WHERE SpeIndex=" + std::to_string(SpeciesIndex);
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["Value"].size() == 0) {
        AllChecksOK = false;
        BadTables += "SpeWeight.csv\n";
    }

    if ((type == "EffortTuned") ||
        (SelectVPATypeCMB->currentText().toStdString() == nmfConstantsMSVPA::DefaultVPATypes[2]))

    {
        // 7. Check SpeTuneEffort: SpeIndex Effort
        fields     = {"SpeName","SpeIndex","SSVPAName","Fleet","Year","Effort"};
        queryStr   = "SELECT SpeName,SpeIndex,SSVPAName,Fleet,Year,Effort FROM SpeTuneEffort";
        queryStr  += " WHERE SpeIndex = " + std::to_string(SpeciesIndex);
        dataMap    = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["Effort"].size() == 0) {
            AllChecksOK = false;
            BadTables += "SpeTuneEffort.csv\n";
        }
    }

    return AllChecksOK;
} // end runDBChecks()


void
nmfSSVPATab4::callback_SSVPA_Tab4_RunPB(bool unused)
{

    emit RunSSVPA("");

} // end callback_SSVPA_Tab4_RunPB


void
nmfSSVPATab4::callback_SSVPA_Tab4_CheckAllPB(bool unused)
{
    QString BadTables = "";
    bool AllChecksOK = runDBChecks("CheckAll",BadTables);

    if (AllChecksOK) {
        QMessageBox::information(SSVPA_Tabs,
                                 tr("Check"),
                                 tr("\nSSVPA data checks: All Passed"),
                                 QMessageBox::Ok);
        emit EnableRunSSVPAPB(true);

    } else {
        QString msg = "\nSSVPA Data checks: Failed\n\n";
        msg += "Please check/resave the following csv file(s):\n\n"+BadTables;
        QMessageBox::information(SSVPA_Tabs,
                                 tr("Check"),
                                 tr(msg.toLatin1()),
                                 QMessageBox::Ok);
    }

} // end callback_SSVPA_Tab4_CheckAllPB


void
nmfSSVPATab4::enableAllWidgets(bool enable)
{

    NaturalMortalityCB->setEnabled(enable);
    //ResidualNaturalMortalitySB->setEnabled(enable);
    //PredationMortalitySB->setEnabled(enable);
    LastYearMortalityRateDSB->setEnabled(enable);
    ReferenceAgeSB->setEnabled(enable);
    SelectVPATypeCMB->setEnabled(enable);
    SelectConfigurationCMB->setEnabled(enable);
    SSVPAInitialSelectivityTV->setEnabled(enable);
    //SSVPAMortalityTV->setEnabled(enable);
    MortalityMinLE->setEnabled(enable);
    MortalityMaxLE->setEnabled(enable);
    SelectivityMinLE->setEnabled(enable);
    SelectivityMaxLE->setEnabled(enable);
    Configure_Cohort_SB1->setEnabled(enable);
    Configure_Cohort_SB2->setEnabled(enable);
    Configure_Cohort_SB3->setEnabled(enable);
    Configure_Cohort_SB4->setEnabled(enable);

} // end enableAllWidgets


void
nmfSSVPATab4::callback_XSA_StartTimeLE()
{

    int currIndex  = SSVPAExtendedIndicesCMB->currentIndex();
    int numIndices = SSVPAExtendedIndicesCMB->count();
    double value;

    SSVPAExtendedStartTimeLE->setText(SSVPAExtendedStartTimeLE->text().trimmed());
    if (! SSVPAExtendedStartTimeLE->text().isEmpty()) {
        value = SSVPAExtendedStartTimeLE->text().toDouble();
        if (currIndex < numIndices)
            Alpha[currIndex] = value;
        else
            Alpha.push_back(value);

        //MarkAsDirty("SpeXSAIndices");
        Save_XSAIndex(currIndex);
    }


} // end callback_XSA_StartTimeLE


void
nmfSSVPATab4::callback_XSA_EndTimeLE()
{
    int currIndex  = SSVPAExtendedIndicesCMB->currentIndex();
    int numIndices = SSVPAExtendedIndicesCMB->count();
    double value;

    SSVPAExtendedEndTimeLE->setText(SSVPAExtendedEndTimeLE->text().trimmed());
    if (! SSVPAExtendedEndTimeLE->text().isEmpty()) {
        value = SSVPAExtendedEndTimeLE->text().toDouble();
        if (currIndex < numIndices) {
            Beta[currIndex] = value;
        } else {
            Beta.push_back(value);
        }

        //MarkAsDirty("SpeXSAIndices");
        Save_XSAIndex(currIndex);
    }
} // end callback_XSA_EndTimeLE


void
nmfSSVPATab4::InitializeXSAIndex(int index)
{
    QStringList strList;

    if (xsaModel[index] != NULL) {
        xsaModel[index]->deleteLater();
    }
    xsaModel[index] = new QStandardItemModel(
                LastYear-FirstYear+1,
                MaxCatchAge+1);

    connect(xsaModel[index], SIGNAL(itemChanged(QStandardItem *)),
            this, SLOT(callback_XSA_ItemChanged(QStandardItem *)));

    SSVPAIndicesTV->setModel(xsaModel[index]);

    // Make row headers
    strList.clear();
    for (int k=FirstYear; k<=LastYear; ++k) {
        strList << QString::number(k);
    }
    xsaModel[index]->setVerticalHeaderLabels(strList);

    // Make column headers
    strList.clear();
    for (int k=0; k<=MaxCatchAge; ++k) {
        strList << "Age " + QString::number(k);
    }
    xsaModel[index]->setHorizontalHeaderLabels(strList);

} // end InitializeXSAIndex


void
nmfSSVPATab4::callback_XSA_IndexCMB(int index)
{
    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::callback_XSA_IndexCMB: index = "+std::to_string(index));

    if (index < 0) {
        std::cout << "Note callback_XSA_IndexCMB: Found index = " << index << std::endl;
        return;
    }

    // Update Alpha and Beta boxes
    if (! Alpha.empty()) {
        SSVPAExtendedStartTimeLE->setText(QString::number(Alpha[index]));
    }
    if (! Beta.empty()) {
        SSVPAExtendedEndTimeLE->setText(QString::number(Beta[index]));
    }

    if (xsaModel[index] != NULL)
        SSVPAIndicesTV->setModel(xsaModel[index]);

    // Run the SSVPA with the newly loaded data
    MarkAsDirty("SpeSSVPA");

    // Works, but not sure it's necessary.
    //emit RunSSVPA(nmfConstants::DefaultVPATypes[3]);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::callback_XSA_IndexCMB Complete");

} // end callback_XSA_IndexCMB




/*
bool
nmfSSVPATab4::SaveToSpeXSAIndicesCSVFile(QString indexNameToDelete,
                                         QString fileNameWithPath,
                                         QString tmpFileNameWithPath)
{
    QString line;
    QStringList qfields;

    QFile fin(fileNameWithPath);
    QFile fout(tmpFileNameWithPath);

    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(2) Opening Input File", fin.errorString());
        return false;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(2) Opening Output File", fout.errorString());
        return false;
    }
    QTextStream inStream(&fin);
    QTextStream outStream(&fout);

    QString csvIndexName;
    while (! inStream.atEnd()) {
        line = inStream.readLine();
        qfields = line.split(",");
        csvIndexName = qfields[3].trimmed();
        if (csvIndexName != indexNameToDelete) {
            outStream << line << "\n";
        }
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    int retv = nmfQtUtils::rename(tmpFileNameWithPath, fileNameWithPath);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPath.toStdString() << " to " <<
                     fileNameWithPath.toStdString() <<
                     ". Save aborted." << std::endl;
        return false;
    }

    return true;

} // end SaveToSpeXSAIndicesCSVFile
*/
/*
bool
nmfSSVPATab4::SaveToSpeXSADataCSVFile(QString indexToDelete,
                                      QString fileNameWithPath,
                                      QString tmpFileNameWithPath)
{
    QString line;
    QStringList qfields;

    QFile fin(fileNameWithPath);
    QFile fout(tmpFileNameWithPath);

    // Open the files here....
    if (! fin.open(QIODevice::ReadOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(5) Opening Input File", fin.errorString());
        return false;
    }
    if (! fout.open(QIODevice::WriteOnly)) {
        QMessageBox::information(SSVPA_Tabs, "Error(5) Opening Output File", fout.errorString());
        return false;
    }
    QTextStream inStream(&fin);
    QTextStream outStream(&fout);

    QString csvIndex;
    while (! inStream.atEnd()) {
        line = inStream.readLine().trimmed();
        if (line.isEmpty())
            continue;
        qfields = line.split(",");
        csvIndex = qfields[2].trimmed();
        if (csvIndex != indexToDelete) {
            outStream << line << "\n";
        }
    }
    fin.close();
    fout.close();

    // Now, mv temp file to actual file.
    int retv = nmfQtUtils::rename(tmpFileNameWithPath, fileNameWithPath);
    if (retv < 0) {
        std::cout << "Error: Couldn't rename " <<
                     tmpFileNameWithPath.toStdString() << " to " <<
                     fileNameWithPath.toStdString() <<
                     ". Save aborted." << std::endl;
        return false;
    }

    return true;

} // end SaveToSpeXSADataCSVFile

*/


void
nmfSSVPATab4::callback_SSVPA_Tab4_AddIndexPB(bool unused)
{
    bool ok;
    int currentIndex;

    QString NewIndexName = QInputDialog::getText(SSVPA_Tabs,
         tr("New Index Name"),
         tr("Enter new XSA Index Name:"),
         QLineEdit::Normal,
         "", &ok);
    NewIndexName = NewIndexName.trimmed();
    if (ok and ! NewIndexName.isEmpty()) {
        if (SSVPAExtendedIndicesCMB->findText(NewIndexName) == -1) {
            SSVPAExtendedIndicesCMB->blockSignals(true);
            SSVPAExtendedIndicesCMB->addItem(NewIndexName);
            SSVPAExtendedIndicesCMB->setCurrentText(NewIndexName);
            //SSVPAExtendedIndicesCMB->model()->sort(0);  // Sorting messes up the Index values in the table...you'll need to code a bit more to get this working correctly
            SSVPAExtendedIndicesCMB->blockSignals(false);
            currentIndex = SSVPAExtendedIndicesCMB->currentIndex();
            SSVPAExtendedStartTimeLE->clear();
            SSVPAExtendedEndTimeLE->clear();
            Alpha.push_back(0);
            Beta.push_back(1);
            SSVPAExtendedStartTimeLE->setEnabled(true);
            SSVPAExtendedEndTimeLE->setEnabled(true);

            InitializeXSAIndex(currentIndex);

            // Automatically save indices. If we don't, and the user selects another config and
            // then reselects this config, get a crash.
            Save_XSAIndex(currentIndex);

        } else {
            QMessageBox::information(SSVPA_Tabs,
                                     tr("Index Already Exists"),
                                     tr("\nThat Index name already exists. Please try again."),
                                     QMessageBox::Ok);
        }
    }

} // end callback_SSVPA_Tab4_AddIndexPB


void
nmfSSVPATab4::callback_SSVPA_Tab4_DelIndexPB(bool unused)
{
    int indexToDelete=0;

    if (Delete_SpeXSAIndices(indexToDelete)) {
        Delete_SpeXSAData(indexToDelete);
    }

} // end callback_SSVPA_Tab4_DelIndexPB


bool
nmfSSVPATab4::Delete_SpeXSAIndices(int &indexInGUI)
{
    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Delete_SpeXSAIndices");

    int indexToDelete;
    int SpeciesIndex;
    int NumIndices;
    std::string cmd;
    std::string errorMsg;
    std::string SpeciesName;
    QString SSVPAName;
    QString TableName = "SpeXSAIndices";
    QString indexNameToDelete;
    QString msg;
    std::vector<std::string> fields;
    QMessageBox::StandardButton reply;
    QStandardItemModel* theXSAModel;

    indexToDelete = SSVPAExtendedIndicesCMB->currentIndex();
    indexInGUI    = indexToDelete;

    theXSAModel = xsaModel[indexInGUI];

    disconnect(theXSAModel,0,0,0);

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    SSVPAName = SelectConfigurationCMB->currentText();
    indexNameToDelete = SSVPAExtendedIndicesCMB->currentText();
    msg = "\nOK to delete index: " + indexNameToDelete + " ?\n";

    reply = QMessageBox::question(SSVPA_Tabs,
                                  tr("Delete Index"),
                                  tr(msg.toLatin1()),
                                  QMessageBox::No|QMessageBox::Yes);
    if (reply == QMessageBox::No)
        return false;

    // Delete from the SpeXSAIndices table and then update the CSV file
    cmd = "DELETE FROM " + TableName.toStdString() +
          " WHERE SpeIndex = "  + std::to_string(SpeciesIndex) +
          " AND SSVPAName  = '" + SSVPAName.toStdString() + "'";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Delete_SpeXSAIndices: Delete error: ", errorMsg);
        return false;
    }

    // Remove from combo box
    SSVPAExtendedIndicesCMB->blockSignals(true);
    SSVPAExtendedIndicesCMB->removeItem(indexInGUI);
    SSVPAExtendedIndicesCMB->blockSignals(false);

    // Need to now re-index the items in the table since the indices have changed
    // Write the contents from the model into the MySQL table
    NumIndices = SSVPAExtendedIndicesCMB->count();
    if (NumIndices > 0) {
        cmd.clear();
        for (int i=0; i<NumIndices; ++i) {
            cmd += "REPLACE INTO " + TableName.toStdString();
            cmd += " (SpeIndex, SSVPAName, `Index`, IndexName, Alpha, Beta) VALUES ";
            cmd +=  "("  + std::to_string(SpeciesIndex) + ",";
            cmd += "\""  + SSVPAName.toStdString() + "\",";
            cmd +=         std::to_string(i+1) + ",";
            cmd += "\""  + SSVPAExtendedIndicesCMB->itemText(i).toStdString() + "\",";
            cmd +=         std::to_string(Alpha[i]) + ",";
            cmd +=         std::to_string(Beta[i]) + ");";
        }
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if (nmfUtilsQt::isAnError(errorMsg)) {
            nmfUtils::printError("Error Delete_SpeXSAIndices: Re-index error: ", errorMsg);
            return false;
        }
    }

    // Restore CSV file
    fields = {"SpeIndex", "SSVPAName", "`Index`", "IndexName", "Alpha", "Beta"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    // Update model pointers
    for (int i=indexInGUI;i<SSVPAExtendedIndicesCMB->count();++i) {
        xsaModel[i] = xsaModel[i+1];
    }
    if (SSVPAExtendedIndicesCMB->currentIndex() > -1) {
        SSVPAIndicesTV->setModel(xsaModel[SSVPAExtendedIndicesCMB->currentIndex()]);
    } else {
        xsaModel[0]->clear();
        SSVPAIndicesTV->setModel(xsaModel[0]);
    }

    Alpha.erase(Alpha.begin() + indexInGUI);
    Beta.erase( Beta.begin()  + indexInGUI);
    Save_XSAIndexData();

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Delete_SpeXSAIndices Complete");

    return true;

} // end Delete_SpeXSAIndices


void
nmfSSVPATab4::Delete_SpeXSAData(int indexToDelete)
{
    int numRows;
    int numCols;
    int dbIndex;
    int NumIndices;
    int SpeciesIndex;
    std::string cmd;
    std::string errorMsg;
    std::string SpeciesName;
    QString SSVPAName;
    QString TableName = "SpeXSAData";
    std::vector<std::string> fields;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Delete_SpeXSAData");

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    SSVPAName = SelectConfigurationCMB->currentText();

    // Delete from the SpeXSAData table and then update the CSV file
    cmd = "DELETE FROM " + TableName.toStdString() +
          " WHERE SpeIndex = "  + std::to_string(SpeciesIndex) +
          " AND SSVPAName  = '" + SSVPAName.toStdString() + "'";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Error Delete_SpeXSAData: Delete error: ", errorMsg);
        return;
    }

    // Need to now re-index the items in the table since the indices have changed
    // Write the contents from the model into the MySQL table
    NumIndices = SSVPAExtendedIndicesCMB->count();
    if (NumIndices > 0) {
        cmd.clear();
        cmd  = "REPLACE INTO " + TableName.toStdString();
        cmd += " (SpeIndex, SSVPAName, `Index`, Year, Age, Value) VALUES ";
        for (int i=0; i<NumIndices; ++i) {
            numRows = xsaModel[i]->rowCount();
            numCols = xsaModel[i]->columnCount();
            dbIndex = i + 1; // since it starts from 1 and not 0
            for (int j=0; j<numRows; ++j) {
                for (int k=0; k<numCols; ++k) {
                    cmd += "(" + std::to_string(SpeciesIndex) + ",";
                    cmd += "\"" + SSVPAName.toStdString()     + "\",";
                    cmd +=  std::to_string(dbIndex)           + ",";
                    cmd +=  std::to_string(j)                 + ",";
                    cmd +=  std::to_string(k)                 + ",";
                    cmd +=  xsaModel[i]->index(j,k).data().toString().toStdString() + "), ";
                }
            }
        }
        // Remove last two characters of cmd string since we have an extra ", "
        cmd = cmd.substr(0,cmd.size()-2);
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if (nmfUtilsQt::isAnError(errorMsg)) {
            nmfUtils::printError("Error Delete_SpeXSAData: Re-index error: ", errorMsg);
            return;
        }
    }

    // Restore CSV file
    fields = {"SpeIndex", "SSVPAName", "`Index`", "Year", "Age", "Value"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    if (NumIndices == 0) {
        SSVPAExtendedStartTimeLE->setEnabled(false);
        SSVPAExtendedEndTimeLE->setEnabled(false);
    }

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::Delete_SpeXSAData Complete");

} // end Delete_SpeXSAData


void
nmfSSVPATab4::callback_SSVPA_Tab4_PrevPB(bool unused)
{
    QString Species = SSVPASelectConfigurationAndTypeGB->title().split(":")[1].trimmed();

    int prevPage = SSVPA_Tabs->currentIndex()-1;
    SSVPA_Tabs->setCurrentIndex(prevPage);

    emit InitializePage("SSVPA",prevPage,true);
    emit ReselectSpecies(Species.toStdString(),true);

} // end callback_SSVPA_Tab4_PrevPB


/*
 * Partial SVPA/ Cohort Analysis
 */
bool
nmfSSVPATab4::runSSVPA_CohortAnalysis(
        std::map<std::string,int> &CohortAnalysisGuiData,
        std::string SSVPAName,
        int FirstCatchYear,
        int LastCatchYear,
        int NCatchYears,
        int FirstCatchAge,
        int LastCatchAge,
        int MaxAge,
        double &CV,
        double &SE,
        int &NIt,
        boost::numeric::ublas::matrix<double> &Abundance,
        boost::numeric::ublas::matrix<double> &M_NaturalMortality,
        boost::numeric::ublas::matrix<double> &F_FishingMortality,
        std::vector<std::string> &rowTitles,
        std::vector<std::string> &colTitles,
        std::string &elapsedTimeStr)
{
    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::runSSVPA_CohortAnalysis");

    std::string SpeciesName;
    int SpeciesIndex;
    QString BadTables;
    CandSVPAInputArgs  argsIn;
    CandSVPAOutputArgs argsOut;

    if (! runDBChecks("CohortAnalysis",BadTables)) {
//        QMessageBox::warning(SSVPA_Tabs,
//                             tr("Run Error: \"Cohort Analysis\""),
//                             tr("\nMissing data. Click \"Check All\" for more information."),
//                             QMessageBox::Ok);
//        return false;
        // RSK - too restrictive...since code should run even without weight data
        logger->logMsg(nmfConstants::Warning,"Missing data. Click \"Check All\" for more information");

    }

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    // Set cursor and timer

    //QApplication::setOverrideCursor(Qt::WaitCursor);
    QDateTime startTime = nmfUtilsQt::getCurrentTime();

    // Pack input and output arg structs
    argsIn.databasePtr           = databasePtr;
    argsIn.CohortAnalysisGuiData = CohortAnalysisGuiData;
    argsIn.FirstYear             = FirstCatchYear;
    argsIn.LastYear              = LastCatchYear;
    argsIn.FirstCatchAge         = FirstCatchAge;
    argsIn.LastCatchAge          = LastCatchAge;
    argsIn.Plus                  = isPlusClass;
    argsIn.MaxAge                = MaxAge;
    argsIn.SpeIndex              = SpeciesIndex;
    argsIn.SSVPAName             = SSVPAName;
    argsIn.M_NaturalMortality    = M_NaturalMortality;
    argsOut.CV_CoeffVariation    = CV;
    argsOut.SE_StandardError     = SE;
    argsOut.NumIterations        = NIt;
    argsOut.Abundance            = Abundance;
    argsOut.F_FishingMortality   = F_FishingMortality;

    //
    // Run the CandSVPA model
    //
    ssvpaApi.CandSVPA( argsIn, argsOut );

    // Unpack the output arg struct
    CV                 = argsOut.CV_CoeffVariation;
    SE                 = argsOut.SE_StandardError;
    NIt                = argsOut.NumIterations;
    Abundance          = argsOut.Abundance;
    F_FishingMortality = argsOut.F_FishingMortality;


    // Reset cursor and stop timer
    //QApplication::setOverrideCursor(Qt::ArrowCursor);
    elapsedTimeStr = nmfUtilsQt::elapsedTime(startTime);

    // Create the column and row headings
    for (int year=FirstCatchYear; year<=LastCatchYear; ++year) {
        rowTitles.push_back(std::to_string(year));
    }
    colTitles.push_back("Year ");
    for (int age=0; age<=MaxAge; ++age) {
        colTitles.push_back("Age "+std::to_string(age));
    }

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::runSSVPA_CohortAnalysis Complete");

    return true;
} // end runSSVPA_CohortAnalysis



/*
 * Full SVPA/ Cohort Analysis
 */
void
nmfSSVPATab4::runSSVPA_SeparableVPA(
        int MinCatchAge,
        int MaxCatchAge,
        int MaxAge,
        int NumCatchYears,
        int NumCatchAges,
        boost::numeric::ublas::matrix<double> &CatchMatrix,
        double &CV_CoeffVariation,
        double &SE_StandardError,
        int &TotalNumIterations,
        boost::numeric::ublas::matrix<double> &Abundance,
        boost::numeric::ublas::matrix<double> &M_NaturalMortality,
        boost::numeric::ublas::matrix<double> &F_FishingMortality,
        std::vector<std::string> &rowTitles,
        std::vector<std::string> &colTitles,
        std::string &elapsedTimeStr)
{
    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::runSSVPA_SeparableVPA");

    FullSVPAInputArgs  argsIn;
    FullSVPAOutputArgs argsOut;
    boost::numeric::ublas::vector<double> InitialSelectivity;
    std::string SpeciesName;
    int SpeciesIndex;
    int ReferenceAge;
    double TerminalMortality;
    QString BadTables;

    nmfUtils::initialize(InitialSelectivity, NumCatchAges);

    if (! runDBChecks("SeparableVPA",BadTables)) {
        QMessageBox::warning(SSVPA_Tabs,
                             tr("Run Error: \"Separable VPA\""),
                             tr("\nMissing data. Click \"Check All\" for more information."),
                             QMessageBox::Ok);
        return;
    }

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    QAbstractItemModel* selectivityModel;

    // Set cursor and timer
    //QApplication::setOverrideCursor(Qt::WaitCursor);
    QDateTime startTime = nmfUtilsQt::getCurrentTime();

    selectivityModel = SSVPAInitialSelectivityTV->model();
    for (int row = 0; row < selectivityModel->rowCount(); ++row) {
        InitialSelectivity(row) =
            selectivityModel->data(selectivityModel->index(row, 1),Qt::DisplayRole).toDouble();

    }

    TerminalMortality = LastYearMortalityRateDSB->value();
    ReferenceAge      = ReferenceAgeSB->value();

    // Load input and output args struct
    argsIn.InitialSelectivity  = InitialSelectivity;
    argsIn.MaxAge              = MaxAge;
    argsIn.FirstCatchAge       = MinCatchAge;
    argsIn.NumCatchAges        = MaxCatchAge-MinCatchAge+1;
    argsIn.isPlusClass         = isPlusClass;
    argsIn.FullF               = TerminalMortality;
    argsIn.ReferenceAge        = ReferenceAge;
    argsIn.NumCatchYears       = NumCatchYears;
    argsOut.CatchMatrix        = CatchMatrix;
    argsOut.CV_CoeffVariation  = CV_CoeffVariation;
    argsOut.SE_StandardError   = SE_StandardError;
    argsOut.NumIterations      = TotalNumIterations;
    argsOut.Abundance          = Abundance;
    argsOut.M_NaturalMortality = M_NaturalMortality;
    argsOut.F_FishingMortality = F_FishingMortality;

    //
    // Run the Full_SVPA model
    //
    ssvpaApi.Full_SVPA( argsIn, argsOut );

    // Unpack the output args struct
    CV_CoeffVariation  = argsOut.CV_CoeffVariation;
    SE_StandardError   = argsOut.SE_StandardError;
    TotalNumIterations = argsOut.NumIterations;
    Abundance          = argsOut.Abundance;
    M_NaturalMortality = argsOut.M_NaturalMortality;
    F_FishingMortality = argsOut.F_FishingMortality;

    // Reset cursor and stop timer
    //QApplication::setOverrideCursor(Qt::ArrowCursor);
    elapsedTimeStr = nmfUtilsQt::elapsedTime(startTime);

    // Create row and column headers for output table.
    for (int year=FirstYear; year<=LastYear; ++year) {
        rowTitles.push_back(std::to_string(year));
    }
    colTitles.push_back("Year ");
    for (int age=0; age<=MaxCatchAge; ++age) {
        colTitles.push_back("Age "+std::to_string(age));
    }

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::runSSVPA_SeparableVPA Complete");

} // end runSSVPA_SeparableVPA


void
nmfSSVPATab4::runSSVPA_EffortTuned(
        std::map<std::string,int> &EffortTunedGuiData,
        int &SSVPAIndex,
        std::string &SSVPAName,
        int &MaxAge,
        int &NCatchYears,
        double &CV,
        double &SE,
        int &NIt,
        MatrixDouble &Abundance,
        MatrixDouble &M_NaturalMortality,
        MatrixDouble &F_FishingMortality,
        std::vector<std::string> &rowTitles,
        std::vector<std::string> &colTitles,
        std::string &elapsedTimeStr)
{

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::runSSVPA_EffortTuned");

    EffortTuneInputArgs  argsIn;
    EffortTuneOutputArgs argsOut;

    std::string SpeciesName;
    int SpeciesIndex;
    QString BadTables;
    Boost4DArrayDouble FleetData;
    nmfUtils::initialize(FleetData);
    FleetData.resize(boost::extents[nmfConstants::MaxNumberFleets]
                                   [nmfConstants::MaxNumberSpecies]
                                   [nmfConstants::MaxNumberYears]
                                   [nmfConstants::MaxNumberAges+1]);

    argsIn.FleetData.resize(boost::extents[nmfConstants::MaxNumberFleets]
            [nmfConstants::MaxNumberSpecies]
            [nmfConstants::MaxNumberYears]
            [nmfConstants::MaxNumberAges+1]);

//    if (! nmfUtilsQt::allCellsArePopulated(SSVPA_Tabs,SSVPAFleetDataTV,nmfConstants::ShowError)) {
//        logger->logMsg(nmfConstants::Error,"nmfSSVPATab4::runSSVPA_EffortTuned Incomplete Data");
//        return;
//    }

    if (! runDBChecks("EffortTuned",BadTables)) {
        QMessageBox::warning(SSVPA_Tabs,
                             tr("Run Error: \"Effort Tuned\""),
                             tr("\nMissing data. Click \"Check All\" for more information."),
                             QMessageBox::Ok);
        return;
    }

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    // Set cursor and timer
    //QApplication::setOverrideCursor(Qt::WaitCursor);
    QDateTime startTime = nmfUtilsQt::getCurrentTime();

    // Load FleetData struct for the Effort_Tune_VPA API call
    int numRows = LastYear-FirstYear+1;
    int numCols = MaxAge-MinCatchAge+1 +1; // The extra +1 is for the Effort (i.e., first) column
    int numFleets = Configure_EffortTuned_SetNumFleetsCMB->currentIndex();
    for (int fleet=0; fleet<numFleets; ++fleet)
        for (int i=0; i<numRows; ++i)
            for (int j=0; j<numCols; ++j) {
                FleetData[fleet][SpeciesIndex][i][j] = fleetModel[fleet]->index(i,j).data().toDouble();
            }

    // Pack arg structures
    argsIn.databasePtr         = databasePtr;
    argsIn.EffortTunedGuiData  = EffortTunedGuiData;
    argsIn.FleetData           = FleetData;
    argsIn.FirstYear           = FirstYear;
    argsIn.LastYear            = LastYear;
    argsIn.FirstCatchAge       = MinCatchAge;
    argsIn.LastCatchAge        = MaxAge;
    argsIn.Plus                = isPlusClass;
    argsIn.MaxAge              = MaxAge;
    argsIn.SpeIndex            = SpeciesIndex;
    argsIn.SSVPAName           = SSVPAName;
    argsOut.Abundance          = Abundance;
    argsOut.M_NaturalMortality = M_NaturalMortality;
    argsOut.F_FishingMortality = F_FishingMortality;

    ssvpaApi.Effort_Tune_VPA( argsIn, argsOut );

    // Unpack return arguments from Effort_Tune run
    Abundance          = argsOut.Abundance;
    M_NaturalMortality = argsOut.M_NaturalMortality;
    F_FishingMortality = argsOut.F_FishingMortality;

    // Reset cursor and stop timer
    //QApplication::setOverrideCursor(Qt::ArrowCursor);
    elapsedTimeStr = nmfUtilsQt::elapsedTime(startTime);

    // Create row and column headers for output table.
    for (int year=FirstYear; year<=LastYear; ++year) {
        rowTitles.push_back(std::to_string(year));
    }
    colTitles.push_back("Year ");
    for (int age=0; age<=MaxAge; ++age) {
        colTitles.push_back("Age "+std::to_string(age));
    }

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::runSSVPA_EffortTuned Complete");

} // end runSSVPA_EffortTuned


void
nmfSSVPATab4::runSSVPA_XSA(
        std::map<std::string,std::string> &XSAGuiData,
        std::string SSVPAName,
        //int XSAIndex,
        double &CV,
        double &SE,
        int &NIt,
        boost::numeric::ublas::matrix<double> &Abundance,
        boost::numeric::ublas::matrix<double> &M_NaturalMortality,
        boost::numeric::ublas::matrix<double> &F_FishingMortality,
        std::vector<std::string> &rowTitles,
        std::vector<std::string> &colTitles,

        std::string &elapsedTimeStr)
{
    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::runSSVPA_XSA");

    XSAInputArgs  inputArgs;
    XSAOutputArgs outputArgs;

    int XSAWarn=0;
    std::string SpeciesName;
    int SpeciesIndex;
    QString BadTables;

    if (! runDBChecks("XSA",BadTables)) {
        QMessageBox::warning(SSVPA_Tabs,
                             tr("Run Error: \"XSA\""),
                             tr("\nMissing data. Click \"Check All\" for more information."),
                             QMessageBox::Ok);
        return;
    }

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    // Set cursor and timer
    //QApplication::setOverrideCursor(Qt::WaitCursor);
    QDateTime startTime = nmfUtilsQt::getCurrentTime();

    // pack input and output arguments for passing to XSA run
    inputArgs.databasePtr         = databasePtr;
    inputArgs.XSAGuiData          = XSAGuiData;
    inputArgs.FirstYear           = FirstYear;
    inputArgs.LastYear            = LastYear;
    inputArgs.SpeIndex            = SpeciesIndex;
    inputArgs.SSVPAName           = SSVPAName;
    inputArgs.M_NaturalMortality  = M_NaturalMortality;
    outputArgs.Abundance          = Abundance;
    outputArgs.F_FishingMortality = F_FishingMortality;
    outputArgs.XSAWarn            = XSAWarn;

    //
    // Run XSA
    //
    ssvpaApi.XSA(inputArgs, outputArgs);

    // Unpack return arguments from XSA run
    Abundance          = outputArgs.Abundance;
    F_FishingMortality = outputArgs.F_FishingMortality;

    // Reset cursor and stop timer
    //QApplication::setOverrideCursor(Qt::ArrowCursor);
    elapsedTimeStr = nmfUtilsQt::elapsedTime(startTime);

    for (int year=FirstYear; year<=LastYear; ++year) {
        rowTitles.push_back(std::to_string(year));
    }
    colTitles.push_back("Year ");
    for (int age=0; age<=MaxCatchAge; ++age) {
        colTitles.push_back("Age "+std::to_string(age));
    }

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::runSSVPA_XSA Complete");

} // end runSSVPA_XSA


void
nmfSSVPATab4::callback_Cohort_SB1(int fullyRecruitedAge)
{
    Configure_Cohort_SB2->blockSignals(true);
    Configure_Cohort_SB2->setMinimum(fullyRecruitedAge+1);
    Configure_Cohort_SB2->setValue(fullyRecruitedAge+1);
    Configure_Cohort_SB2->blockSignals(false);

    MarkAsDirty("SpeSSVPA");
    emit RunSSVPA(nmfConstantsMSVPA::DefaultVPATypes[0]);

}

void
nmfSSVPATab4::callback_Cohort_SB1(int fullyRecruitedAge, bool sendDirtySignal)
{
    Configure_Cohort_SB2->setMinimum(fullyRecruitedAge+1);
    Configure_Cohort_SB2->setValue(fullyRecruitedAge+1);
}

void
nmfSSVPATab4::callback_Cohort_SB2(int pSVPANCatYrs)
{
std::cout << "nmfSSVPATab4::callback_Cohort_SB2" << std::endl;

    MarkAsDirty("SpeSSVPA");    
    emit RunSSVPA(nmfConstantsMSVPA::DefaultVPATypes[0]);
}

void
nmfSSVPATab4::callback_Cohort_SB3(int pSVPANMortYrs)
{
    MarkAsDirty("SpeSSVPA");
    emit RunSSVPA(nmfConstantsMSVPA::DefaultVPATypes[0]);
}
void
nmfSSVPATab4::callback_Cohort_SB4(int pSVPARefAge)
{
    MarkAsDirty("SpeSSVPA");
    emit RunSSVPA(nmfConstantsMSVPA::DefaultVPATypes[0]);

}



void
nmfSSVPATab4::callback_EffortTuned_SB1(int fullyRecruitedAge)
{
    MarkAsDirty("SpeSSVPA");
    emit RunSSVPA(nmfConstantsMSVPA::DefaultVPATypes[2]);
}


void
nmfSSVPATab4::enableWidgets(bool enable)
{
    Configure_EffortTuned_LB3->setEnabled(enable);
    Configure_EffortTuned_SelectAFleetCMB->setEnabled(enable);
    SSVPAFleetDataTV->setEnabled(enable);
} // end enableWidgets



void
nmfSSVPATab4::SetNumFleets(int numFleets)
{
    QString fleet;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::SetNumFleets");

    Configure_EffortTuned_SelectAFleetCMB->blockSignals(true);

    enableWidgets(numFleets>0);

    Configure_EffortTuned_SelectAFleetCMB->clear();
    for (int i=0; i<numFleets; ++i) {
        fleet = "Fleet " + QString::number(i+1);
        Configure_EffortTuned_SelectAFleetCMB->addItem(fleet);
    }
    Configure_EffortTuned_SelectAFleetCMB->setCurrentIndex(0);
    Configure_EffortTuned_SelectAFleetCMB->blockSignals(false);

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::SetNumFleets Complete");

} // end SetNumFleets

void
nmfSSVPATab4::callback_EffortTuned_SetNumFleetsCMB(int numFleets)
{
    int SpeciesIndex;
    std::string SpeciesName;

    QString csvPath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));
    QString catchCSVFile = QDir(csvPath).filePath("SpeCatch.csv");

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    SetNumFleets(numFleets);

    InitializeNumFleets(numFleets);
    if (numFleets == 1) {
        Load_CatchData(catchCSVFile);
    }
    Save_SpeSSVPA(SpeciesIndex);

    MarkAsDirty("SpeTuneEffort");

} // end callback_EffortTuned_SetNumFleetsCMB


void
nmfSSVPATab4::callback_EffortTuned_SetNumFleetsCMB(int numFleets, bool noSignal)
{
    SetNumFleets(numFleets);

} // end callback_EffortTuned_SetNumFleetsCMB


void
nmfSSVPATab4::callback_EffortTuned_SelectAFleetCMB(int fleet)
{
    QStringList strList;
    std::string suffix;
//    QStandardItem *item;
    std::string SpeciesName;
    int SpeciesIndex;

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    //disconnect(fleetModel[fleet],0,0,0);

    SSVPAFleetDataTV->setModel(fleetModel[fleet]);


/*
    if (smodelEffort != NULL)
        smodelEffort->clear();
    else
        smodelEffort = new QStandardItemModel(nmfConstants::MaxNumberYears,nmfConstants::MaxNumberAges+1);

    disconnect(smodelEffort, SIGNAL(itemChanged(QStandardItem *)),
               this,   SLOT(callback_EffortTuned_ItemChanged(QStandardItem *)));

    // Load the model
    for (int i=FirstYear; i<=LastYear; ++i) {
        // First column is Effort value
        item = new QStandardItem(QString::number(FleetData[fleet][SpeciesIndex][i-FirstYear][0]));
        item->setTextAlignment(Qt::AlignCenter);
        smodelEffort->setItem(i-FirstYear, 0, item);
        // Subsequent columns are Catch values for each Age class
        for (int j=0;j<=MaxCatchAge;++j){
            item = new QStandardItem(QString::number(FleetData[fleet][SpeciesIndex][i-FirstYear][j-MinCatchAge+1]));
            item->setTextAlignment(Qt::AlignCenter);
            smodelEffort->setItem(i-FirstYear, j+1, item);
        }
    }
    SSVPAFleetDataTV->setModel(smodelEffort);

    connect(smodelEffort, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_EffortTuned_ItemChanged(QStandardItem *)));
*/
} // end callback_EffortTuned_SelectAFleetCMB


bool
nmfSSVPATab4::checkFleetDataValue(QStandardItem *item)
{
    bool retv = true;
    int col = item->column();

    double value = item->index().data().toDouble();
    if (col == 0) {
        if (value <= 0) {
            std::cout << "Error: Effort value must be > 0." << std::endl;
            retv = false;
        }
    } else {
        if (value < 0) {
            std::cout << "Error: Catch value must be >= 0." << std::endl;
            retv = false;
        }
    }

    return retv;
} // end checkFleetDataValue


void
nmfSSVPATab4::callback_EffortTuned_ItemChanged(QStandardItem *item)
{
    //int fleet;
    bool valueOK = checkFleetDataValue(item);
    double val;

    Configure_EffortTuned_SelectAFleetCMB->blockSignals(true);
    //fleet = Configure_EffortTuned_SelectAFleetCMB->currentIndex();
    Configure_EffortTuned_SelectAFleetCMB->blockSignals(false);

    val = (valueOK) ? item->index().data().toDouble() : 0.0;

    SSVPAFleetDataTV->model()->blockSignals(true);
    item->setData(val);
    SSVPAFleetDataTV->model()->blockSignals(false);

    if (item->column() == 0)
        MarkAsDirty("SpeTuneEffort");
    else
        MarkAsDirty("SpeTuneCatch");

} // end callback_EffortTuned_ItemChanged


void
nmfSSVPATab4::callback_XSA_ItemChanged(QStandardItem *item)
{
    double val;

    val = item->index().data().toDouble();

    SSVPAIndicesTV->model()->blockSignals(true);
    item->setData(val);
    SSVPAIndicesTV->model()->blockSignals(false);

    MarkAsDirty("SpeXSAData");

} // end callback_XSA_ItemChanged


void
nmfSSVPATab4::MarkAsClean()
{

    emit MarkSSVPAAsClean();

} // end MarkAsClean


void
nmfSSVPATab4::MarkAsDirty(std::string tableName)
{
    // Emit a custom signal signifying the user has entered (or caused to enter)
    // data that may need to be saved prior to quitting.
    emit TableDataChanged(tableName);

} // end MarkAsDirty


void
nmfSSVPATab4::AdjustButtonsForMissingData()
{
//    SSVPA_Tab4_LoadPB->setEnabled(true);
//    SSVPA_Tab4_SavePB->setEnabled(false);
//    SSVPA_Tab4_CheckAllPB->setEnabled(false);

} // end AdjustButtonsForMissingData


void
nmfSSVPATab4::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir

void
nmfSSVPATab4::loadDefaultWidgets(nmfDatabase *theDatabasePtr,
                                 std::vector<QString> &theAllSpecies,
                                 int SelectedSpeciesIndex)
{
    int m;
    int numRows;
    int numCols;
    int offset = 1;
    int MaxAge=0;
    int NCatchYears = 0;
    int NumRecords = 0;
    int SSVPAIndex;
    int FullRecAge=0,RefAge=0,val2,val3,val4;
    int NFleets=0;
    int Downweight=0;
    int DownweightType=0;
    int DownweightYear=0;
    int Shrink=0;
    int ShrinkYears=0;
    int ShrinkAge=0;
    int AgeM=0;
    int NXSAIndex=0;
    double TermF = nmfConstants::TerminalMortalityMin;
    double ShrinkCV=0.0;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QStandardItem *item;
    QString csvPath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));

    AllSpecies = theAllSpecies;
    SpeciesIndex = SelectedSpeciesIndex;

    databasePtr = theDatabasePtr;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::loadDefaultWidgets");

    //
    // loadSSVPA()
    //
    // Find Species data
    fields   = {"SpeIndex","SpeName","MaxAge","FirstYear","LastYear","PlusClass","MinCatAge","MaxCatAge","FixedMaturity"};
    queryStr = "SELECT SpeIndex,SpeName,MaxAge,FirstYear,LastYear,PlusClass,MinCatAge,MaxCatAge,FixedMaturity FROM Species WHERE SpeIndex = '" + std::to_string(SelectedSpeciesIndex) + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    MaxAge          = std::stoi(dataMap["MaxAge"][0]);
    MinCatchAge     = std::stoi(dataMap["MinCatAge"][0]);
    MaxCatchAge     = std::stoi(dataMap["MaxCatAge"][0]);
    isFixedMaturity = std::stoi(dataMap["FixedMaturity"][0]);
    FirstYear       = std::stoi(dataMap["FirstYear"][0]);
    LastYear        = std::stoi(dataMap["LastYear"][0]);
    isPlusClass     = std::stoi(dataMap["PlusClass"][0]);
    NCatchYears     = LastYear-FirstYear+1;

    // Prompt user for SSVPA config and set default SSVPA configuration and type
    SelectConfigurationCMB->blockSignals(true);
    SelectConfigurationCMB->clear();
    SelectConfigurationCMB->blockSignals(false);
    fields     = {"SpeIndex","SSVPAName","AgeM","SSVPAIndex","Type"};
    queryStr   = "SELECT SpeIndex,SSVPAName,AgeM,SSVPAIndex,Type FROM SpeSSVPA WHERE SpeIndex = '" + std::to_string(SelectedSpeciesIndex) + "'";
    dataMap    = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SSVPAName"].size();
    if (NumRecords == 0) {  // Prompt the user to add a SSVPA configuration
        callback_SSVPA_Tab4_AddConfigPB(true);
        SelectVPATypeCMB->blockSignals(true);
        SelectVPATypeCMB->setCurrentIndex(0);
        SelectVPATypeCMB->blockSignals(false);
    } else {                // Read the currently saved SSVPA configuration
        SelectConfigurationCMB->blockSignals(true);
        SelectConfigurationCMB->clear();
        for (int i=0; i<NumRecords; ++i) {
            SelectConfigurationCMB->addItem(QString::fromStdString(dataMap["SSVPAName"][i]));
        }
        SelectConfigurationCMB->blockSignals(false);
        SelectConfigurationCMB->setCurrentIndex(std::stoi(dataMap["SSVPAIndex"][0])-1);
        SelectVPATypeCMB->setCurrentIndex(std::stoi(dataMap["Type"][0]));
        AgeM = std::stoi(dataMap["AgeM"][0]);
    }

    logger->logMsg(nmfConstants::Normal,"loading Mortality...");

    //
    // load Mortality And Selectivity
    //
    // See if mortality data exists for SpeciesIndex
    SSVPAIndex = SelectConfigurationCMB->currentIndex()+1;
    fields     = {"SpeIndex","SSVPAIndex","SSVPAName","Age","AgeM1","AgeM2","PRF"};
    queryStr   = "SELECT SpeIndex,SSVPAIndex,SSVPAName,Age,AgeM1,AgeM2,PRF FROM SSVPAAgeM";
    queryStr  += " WHERE SpeIndex = " + std::to_string(SelectedSpeciesIndex);
    queryStr  += " AND SSVPAIndex = " + std::to_string(SSVPAIndex);
    dataMap    = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpeIndex"].size();
    mortality_model = new nmfMortalityModel();

//    if (mortality_model == NULL) {
//        mortality_model = (nmfMortalityModel *)SSVPAMortalityTV->model();
//    }
    //numRows = mortality_model->count();
    //mortality_model->removeRows(0, numRows, QModelIndex());

    if (NumRecords == 0) {
        for (int i=0; i<=MaxCatchAge; ++i) {
            mortality_model->append(nmfMortality {i,"0","0"});
        }
    } else {
        for (int i=0; i<=MaxCatchAge; ++i) {
            mortality_model->append(nmfMortality {i,
                                                  QString::fromStdString(dataMap["AgeM1"][i]),
                                                  QString::fromStdString(dataMap["AgeM2"][i]) });
        }
    }

    NaturalMortalityCB->setChecked(AgeM);
    SSVPAMortalityTV->setModel(mortality_model);
    SSVPAMortalityTV->resizeColumnsToContents();
    SSVPAMortalityTV->scrollToTop();
    ResidualNaturalMortalityDSB->blockSignals(true);
    PredationMortalityDSB->blockSignals(true);
    if (NumRecords == 0) {
        ResidualNaturalMortalityDSB->setValue(0);
        PredationMortalityDSB->setValue(0);
    } else {
        ResidualNaturalMortalityDSB->setValue(std::stod(dataMap["AgeM1"][0]));
        PredationMortalityDSB->setValue(std::stod(dataMap["AgeM2"][0]));
    }
    ResidualNaturalMortalityDSB->blockSignals(false);
    PredationMortalityDSB->blockSignals(false);

    logger->logMsg(nmfConstants::Normal,"loading Selectivity...");

    //
    // Set SSVPAInitialSelectivityTV table (data come from same table as Mortality data)
    selectivity_model = new nmfInitialSelectivityModel();
    selectivity_model->blockSignals(true);
    if (NumRecords == 0) {
        for (int i=0; i<=MaxCatchAge; ++i) {
            selectivity_model->append(nmfSelectivity { i, -9 });
        }
    } else {
        for (int i=0; i<=MaxCatchAge; ++i) {
            selectivity_model->append(nmfSelectivity { i, std::stod(dataMap["PRF"][i]) });
        }
    }
    selectivity_model->blockSignals(false);
    SSVPAInitialSelectivityTV->setModel(selectivity_model);
    SSVPAInitialSelectivityTV->blockSignals(true);
    SSVPAInitialSelectivityTV->resizeColumnsToContents();
    SSVPAInitialSelectivityTV->blockSignals(false);

    disconnect(selectivity_model,0,0,0);
    connect(selectivity_model, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this, SLOT(callback_InitialSelectivityChanged(QModelIndex, QModelIndex)));

    logger->logMsg(nmfConstants::Normal,"loading Maturity...");

    //
    // load SpeMaturity
    //
    fields     = {"SpeIndex","SpeName","Year","Age","PMature"};
    queryStr   = "SELECT SpeIndex,SpeName,Year,Age,PMature FROM SpeMaturity";
    queryStr  += " WHERE SpeIndex = " + std::to_string(SelectedSpeciesIndex);
    dataMap    = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpeIndex"].size();
    if (isFixedMaturity) {
        maturityModel[SelectedSpeciesIndex] = new QStandardItemModel(nmfConstants::MaxNumberAges, 1 );
        numRows = MaxCatchAge+1;
        numCols = 1;
    } else {
        maturityModel[SelectedSpeciesIndex] = new QStandardItemModel(nmfConstants::MaxNumberYears,
                                                                     nmfConstants::MaxNumberAges);
        numRows = NCatchYears;
        numCols = MaxCatchAge+1;
    }
    maturityModel[SelectedSpeciesIndex]->setRowCount(numRows);
    maturityModel[SelectedSpeciesIndex]->setColumnCount(numCols);
    SSVPAMaturityTV->setModel(maturityModel[SelectedSpeciesIndex]);

    if (NumRecords == 0) {
        for (int i=0;i<numRows;++i) {
            for (int j=0;j<numCols;++j) {
                item = new QStandardItem("0");
                item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                maturityModel[SelectedSpeciesIndex]->setItem(i,j,item);
            }
        }
    } else {
        m = 0;
        for (int i=0;i<numRows;++i) {
            for (int j=0;j<numCols;++j) {
                item = new QStandardItem(QString::fromStdString(dataMap["PMature"][m++]));
                item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                maturityModel[SelectedSpeciesIndex]->setItem(i,j,item);
            }
        }
    }

    // Setup callback in case user edits the Maturity matrix by hand.
    disconnect(maturityModel[SelectedSpeciesIndex],0,0,0);
    connect(maturityModel[SelectedSpeciesIndex], SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_Maturity_ItemChanged(QStandardItem *)));

    makeMaturityHeaders(isFixedMaturity,FirstYear,
                        numRows,numCols,
                        SelectedSpeciesIndex);
    SSVPAMaturityTV->resizeColumnsToContents();

    logger->logMsg(nmfConstants::Normal,"loading SpeSSVPA...");

    //
    // load SpeSSVPA
    //
    // Set default Cohort Analysis widgets
    fields     = {"SpeIndex","TermF","RefAge","FullRecAge","pSVPANCatYrs","pSVPANMortYrs",
                  "pSVPARefAge","NFleets","NXSAIndex","Downweight","DownweightType",
                  "DownweightYear","Shrink","ShrinkCV","ShrinkYears","ShrinkAge"};
    queryStr   = "SELECT SpeIndex,TermF,RefAge,FullRecAge,pSVPANCatYrs,pSVPANMortYrs,pSVPARefAge,";
    queryStr  += "NFleets,NXSAIndex,Downweight,DownweightType,DownweightYear,Shrink,ShrinkCV,";
    queryStr  += "ShrinkYears,ShrinkAge FROM SpeSSVPA";
    queryStr  += " WHERE SpeIndex = " + std::to_string(SelectedSpeciesIndex);
    dataMap    = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpeIndex"].size();
    Configure_Cohort_SB1->blockSignals(true);
    Configure_Cohort_SB2->blockSignals(true);
    Configure_Cohort_SB3->blockSignals(true);
    Configure_Cohort_SB4->blockSignals(true);
    loadDefaultCohortWidgets(MaxCatchAge,NCatchYears);
    Configure_Cohort_SB1->setValue(Configure_Cohort_SB1->minimum());
    Configure_Cohort_SB2->setValue(Configure_Cohort_SB1->minimum()+1);
    Configure_Cohort_SB3->setValue(Configure_Cohort_SB3->minimum());
    Configure_Cohort_SB4->setValue(Configure_Cohort_SB4->minimum());

    if (NumRecords != 0) {
        FullRecAge = std::stoi(dataMap["FullRecAge"][0]);
        if (FullRecAge != 0) {
            Configure_Cohort_SB1->setValue(FullRecAge);
        }
        val2 = std::stoi(dataMap["pSVPANCatYrs"][0]);
        if (val2 != 0)
            Configure_Cohort_SB2->setValue(val2);
        val3 = std::stoi(dataMap["pSVPANMortYrs"][0]);
        if (val3 != 0)
            Configure_Cohort_SB3->setValue(val3);
        val4 = std::stoi(dataMap["pSVPARefAge"][0]);
        if (val4 != 0)
            Configure_Cohort_SB4->setValue(val4);
        NFleets        = std::stoi(dataMap["NFleets"][0]);
        if (NFleets == 0)
            NFleets = 1; // Because 0 Fleets is error prone here and in the original code
        Downweight     = std::stoi(dataMap["Downweight"][0]);
        DownweightType = std::stoi(dataMap["DownweightType"][0]);
        DownweightYear = std::stoi(dataMap["DownweightYear"][0]);
        Shrink         = std::stoi(dataMap["Shrink"][0]);
        ShrinkCV       = std::stod(dataMap["ShrinkCV"][0]);
        ShrinkYears    = std::stoi(dataMap["ShrinkYears"][0]);
        ShrinkAge      = std::stoi(dataMap["ShrinkAge"][0]);
        TermF          = std::stod(dataMap["TermF"][0]);
        RefAge         = std::stoi(dataMap["RefAge"][0]);
        NXSAIndex      = std::stoi(dataMap["NXSAIndex"][0]) -1; // Starts with 1 in table
    }
    Configure_Cohort_SB4->blockSignals(false);
    Configure_Cohort_SB3->blockSignals(false);
    Configure_Cohort_SB2->blockSignals(false);
    Configure_Cohort_SB1->blockSignals(false);

    //
    // Set default Separable VPA widgets: TermF and RefAge values
    //
    LastYearMortalityRateDSB->blockSignals(true);
    LastYearMortalityRateDSB->setValue(TermF);
    LastYearMortalityRateDSB->blockSignals(false);
    //
    ReferenceAgeSB->blockSignals(true);
    if (isPlusClass)
        offset = 2;
    ReferenceAgeSB->setRange(0, MaxCatchAge - offset);
    ReferenceAgeSB->setValue(nmfConstants::DefaultReferenceAge);
    if (RefAge != 0) {
        ReferenceAgeSB->setValue(RefAge);
    }
    ReferenceAgeSB->blockSignals(false);

    logger->logMsg(nmfConstants::Normal,"loading XSA Data...");

    bool loadedIndicesOK;
    bool loadedDataOK;
    //
    // load SpeXSAIndices
    // load SpeXSAData
    //
    // Check SpeXSAIndices: If there's data, means there's data also in SpeXSAData.
    fields     = {"SpeIndex","SSVPAName","`Index`","IndexName","Alpha","Beta"};
    queryStr   = "SELECT SpeIndex,SSVPAName,`Index`,IndexName,Alpha,Beta FROM SpeXSAIndices";
    queryStr  += " WHERE SpeIndex = " + std::to_string(SelectedSpeciesIndex);
    dataMap    = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpeIndex"].size();
    loadDefaultXSAWidgets(FirstYear,LastYear,MaxAge,
                          Downweight,DownweightType,DownweightYear,
                          Shrink,ShrinkCV,ShrinkYears,ShrinkAge);
    if (NumRecords > 0) {
        SSVPAExtendedStartTimeLE->setEnabled(true);
        SSVPAExtendedEndTimeLE->setEnabled(true);
        loadedIndicesOK = Load_XSAIndices(QDir(csvPath).filePath("SpeXSAIndices.csv")); // Load from table
        if (loadedIndicesOK) {
            InitializeXSAIndex(NXSAIndex);
            loadedDataOK = Load_XSAData(QDir(csvPath).filePath("SpeXSAData.csv"));
            if (loadedDataOK) {
                SSVPAExtendedIndicesCMB->setCurrentIndex(NXSAIndex);
                callback_XSA_IndexCMB(NXSAIndex);
            }
        }
    }

    logger->logMsg(nmfConstants::Normal,"loading Fleet Data...");

    //
    // load SpeTuneCatch
    // load SpetuneEffort
    //
    // Set default Laurec Shepard Tuned VPA widgets
    loadDefaultLaurecShepardWidgets(MaxCatchAge,FullRecAge,NFleets);

    // If there's Fleet Effort/Catch data load it...
    // Check SpeTuneEffort; If there's data, means there's data also in SpeTuneCatch.
    fields     = {"SpeName","SpeIndex","SSVPAName","Fleet","Year","Effort"};
    queryStr   = "SELECT SpeName,SpeIndex,SSVPAName,Fleet,Year,Effort FROM SpeTuneEffort";
    queryStr  += " WHERE SpeIndex = " + std::to_string(SelectedSpeciesIndex);
    dataMap    = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpeIndex"].size();
    if (NumRecords > 0) {
        Load_FleetEffortData(QDir(csvPath).filePath("SpeTuneEffort.csv"));
        if (NFleets == 1)
            Load_CatchData(QDir(csvPath).filePath("SpeCatch.csv"));
        else
            Load_FleetCatchData(QDir(csvPath).filePath("SpeTuneCatch.csv"));
    }


    // Just read in everything so make sure no table is shown as edited.
    MarkAsClean();

    if (AgeM == 0)
        callback_MortalityChecked(Qt::Unchecked,false);
    else
        callback_MortalityChecked(Qt::Checked,false);


    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::loadDefaultWidgets End");

} // end loadDefaultWidgets

/*
bool
nmfSSVPATab4::loadAllSpeciesFromTableOrFile(nmfDatabase *theDatabasePtr,
                                      std::vector<QString> theAllSpecies,
                                      std::string FromTableOrFile,
                                      int SelectedSpeciesIndex,
                                      std::string SelectedSpeciesName,
                                      QTextEdit *outputTE)
{
std::cout << "\nnmfSSVPATab4::loadAllSpeciesFromTableOrFile" << std::endl;


    bool ok=true;
    QString Species;
    std::string fileName="";
    std::string currSpeciesIndex;

    //logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::loadAllSpeciesFromTableOrFile");

    AllSpecies = theAllSpecies;

    //loadDefaultWidgets(theDatabasePtr,AllSpecies,SelectedSpeciesIndex);



//    for (int i=AllSpecies.size()-1; i>=0; --i) {

//        Species = AllSpecies[i];

//        fields   = {"SpeIndex"};
//        queryStr = "SELECT SpeIndex FROM Species WHERE SpeName = '" + Species.toStdString() + "'";
//        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
//        if (dataMap["SpeIndex"].size() != 0) {
//            currSpeciesIndex = dataMap["SpeIndex"][0];
//            ok = loadWidgets(theDatabasePtr,
//                        AllSpecies,
//                        currSpeciesIndex,
//                        Species.toStdString(),
//                        FromTableOrFile,
//                        fileName,
//                        outputTE);
//        }

//    }

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::loadAllSpeciesFromTableOrFile Complete");

    return true;

} // end loadAllSpeciesFromTableOrFile
*/
/*
bool
nmfSSVPATab4::loadSpecies(int theSpeciesIndex,
                          std::string theSpecies,
                          int theFirstYear,
                          QTextEdit *outputTE)
{
std::cout << "*** loadSpecies" << std::endl;

    std::string SpeciesName = theSpecies;
    int SpeciesIndex = theSpeciesIndex;

std::cout << "Loading: " << SpeciesName << ", " << SpeciesIndex << std::endl;

    FirstYear    = theFirstYear;

    loadWidgets(databasePtr,
                AllSpecies,
                std::to_string(SpeciesIndex),
                SpeciesName,
                "FromTable",
                "",
                outputTE);

} // end loadSpecies
*/

/*
   Returns the current Species name and SpeciesIndex from Tab4's GUI.
*/

// RSK - Consider hiding the SpeciesIndexStr in the GUI as well....quicker to get it out
// than having to hit the database everytime.
void
nmfSSVPATab4::getSpeciesFromGUI(std::string &SpeciesName, int &SpeciesIndex)
{
//std::cout << "getSpeciesFromGUI" << std::endl;

    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    std::string msg;

    if (SSVPASelectConfigurationAndTypeGB->title().split(":").size() <= 1) {
        std::cout << "getSpeciesFromGUI end - no title with \":\" found" << std::endl;
        return;
    }

    SpeciesName  = SSVPASelectConfigurationAndTypeGB->title().split(":")[1].trimmed().toStdString();
    SpeciesIndex = -1;

    fields   = {"SpeIndex"};
    queryStr = "SELECT SpeIndex FROM Species WHERE SpeName = '" + SpeciesName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["SpeIndex"].size() > 0) {
        SpeciesIndex = std::stoi(dataMap["SpeIndex"][0]);
    } else {
        msg = "\nCouldn't find species: " + SpeciesName + ", in the table: Species.";
        QMessageBox::warning(SSVPA_Tabs,
                             tr("Warning"),
                             tr(msg.c_str()),
                             QMessageBox::Ok);
    }
//std::cout << "getSpeciesFromGUI end" << std::endl;

} // end nmfSSVPATab4::getSpeciesFromGUI






void
nmfSSVPATab4::loadDefaultCohortWidgets(int MaxCatchAge, int NCatchYears)
{
    Configure_Cohort_SB1->setMinimum(1); //MinCatchAge);
    Configure_Cohort_SB1->setMaximum(MaxCatchAge-2);
    Configure_Cohort_SB1->setToolTip(  "The age class for the Fully Recruited Age must be between the minimum Catch Age and the maximum Catch Age - 2, inclusive.");
    Configure_Cohort_SB1->setStatusTip("The age class for the Fully Recruited Age must be between the minimum Catch Age and the maximum Catch Age - 2, inclusive.");
    Configure_Cohort_LB1->setToolTip(  "The age class for the Fully Recruited Age must be between the minimum Catch Age and the maximum Catch Age - 2, inclusive.");
    Configure_Cohort_LB1->setStatusTip("The age class for the Fully Recruited Age must be between the minimum Catch Age and the maximum Catch Age - 2, inclusive.");
    Configure_Cohort_SB2->setMaximum(NCatchYears);
    Configure_Cohort_SB2->setToolTip(  "The number of years for the partial SVPA must be > the Fully Recruited Age and < the number of Catch years.");
    Configure_Cohort_SB2->setStatusTip("The number of years for the partial SVPA must be > the Fully Recruited Age and < the number of Catch years.");
    Configure_Cohort_LB2->setToolTip(  "The number of years for the partial SVPA must be > the Fully Recruited Age and < the number of Catch years.");
    Configure_Cohort_LB2->setStatusTip("The number of years for the partial SVPA must be > the Fully Recruited Age and < the number of Catch years.");
    Configure_Cohort_SB3->setMinimum(1);
    Configure_Cohort_SB3->setMaximum(NCatchYears-1);
    Configure_Cohort_SB3->setToolTip(  "The number of years to calculate initial selectivity for the partial SVPA must be >= 1 and < the number of Catch years.");
    Configure_Cohort_SB3->setStatusTip("The number of years to calculate initial selectivity for the partial SVPA must be >= 1 and < the number of Catch years.");
    Configure_Cohort_LB3->setToolTip(  "The number of years to calculate initial selectivity for the partial SVPA must be >= 1 and < the number of Catch years.");
    Configure_Cohort_LB3->setStatusTip("The number of years to calculate initial selectivity for the partial SVPA must be >= 1 and < the number of Catch years.");
    Configure_Cohort_SB4->setMinimum(0);
    Configure_Cohort_SB4->setMaximum(MaxCatchAge-1);
    Configure_Cohort_SB4->setToolTip(  "Reference age must be >= 0 and < the maximum Catch Age.");
    Configure_Cohort_SB4->setStatusTip("Reference age must be >= 0 and < the maximum Catch Age.");
    Configure_Cohort_LB4->setToolTip(  "Reference age must be >= 0 and < the maximum Catch Age.");
    Configure_Cohort_LB4->setStatusTip("Reference age must be >= 0 and < the maximum Catch Age.");
} // end SetupDefaultCohortSpinBoxes




void
nmfSSVPATab4::loadDefaultLaurecShepardWidgets(int MaxCatchAge,int FullRecAge, int NFleets)
{
    QString fleet;

    Configure_EffortTuned_SB1->blockSignals(true);
    Configure_EffortTuned_LB1->blockSignals(true);
    Configure_EffortTuned_SelectAFleetCMB->blockSignals(true);
    Configure_EffortTuned_SB1->setMinimum(MinCatchAge);
    Configure_EffortTuned_SB1->setMaximum(MaxCatchAge-2);
    Configure_EffortTuned_SB1->setToolTip(  "The age class for the Fully Recruited Age must be between the minimum Catch Age and the maximum Catch Age - 2, inclusive.");
    Configure_EffortTuned_SB1->setStatusTip("The age class for the Fully Recruited Age must be between the minimum Catch Age and the maximum Catch Age - 2, inclusive.");
    Configure_EffortTuned_LB1->setToolTip(  "The age class for the Fully Recruited Age must be between the minimum Catch Age and the maximum Catch Age - 2, inclusive.");
    Configure_EffortTuned_LB1->setStatusTip("The age class for the Fully Recruited Age must be between the minimum Catch Age and the maximum Catch Age - 2, inclusive.");
    if (Configure_EffortTuned_SelectAFleetCMB->count() > 0) {
        Configure_EffortTuned_SelectAFleetCMB->setCurrentIndex(0);
    }
    if (FullRecAge <= Configure_EffortTuned_SB1->maximum())
        Configure_EffortTuned_SB1->setValue(FullRecAge);
    Configure_EffortTuned_SB1->blockSignals(false);
    Configure_EffortTuned_LB1->blockSignals(false);
    Configure_EffortTuned_SelectAFleetCMB->blockSignals(false);

    enableWidgets(Configure_EffortTuned_SetNumFleetsCMB->currentIndex()>0);

    // Set the fleets comboboxes
    Configure_EffortTuned_SetNumFleetsCMB->blockSignals(true);
    Configure_EffortTuned_SelectAFleetCMB->blockSignals(true);
    Configure_EffortTuned_SetNumFleetsCMB->clear();
    Configure_EffortTuned_SelectAFleetCMB->clear();
    for (int i=0;i<=nmfConstants::MaxNumberFleets;++i) {
        Configure_EffortTuned_SetNumFleetsCMB->addItem(QString::number(i));
    }
    for (int i=0; i<nmfConstants::MaxNumberFleets; ++i) {
        fleet = "Fleet " + QString::number(i+1);
        Configure_EffortTuned_SelectAFleetCMB->addItem(fleet);
    }
    //Configure_EffortTuned_SelectAFleetCMB->setCurrentIndex(0);
    Configure_EffortTuned_SelectAFleetCMB->blockSignals(false);
    Configure_EffortTuned_SetNumFleetsCMB->blockSignals(false);
    Configure_EffortTuned_SetNumFleetsCMB->setCurrentIndex(NFleets);

} // end loadDefaultLaurecShepardWidgets






void
nmfSSVPATab4::loadDefaultXSAWidgets(
        int FirstYear,
        int LastYear,
        int MaxAge,
        int Downweight,
        int DownweightType,
        int DownweightYear,
        int Shrink,
        double ShrinkCV,
        int ShrinkYears,
        int ShrinkAge)
{
    int NCatchYears = LastYear-FirstYear+1;

    Configure_Shrinkage_GB->setChecked(Shrink);
    Configure_Downweight_GB->setChecked(Downweight);

    Configure_XSA_LE1->blockSignals(true);
    Configure_XSA_NYears_SB->blockSignals(true);
    Configure_XSA_NAges_SB->blockSignals(true);
    Configure_XSA_CMB4->blockSignals(true);
    Configure_XSA_CMB5->blockSignals(true);

    Configure_XSA_LE1->setText(QString::number(ShrinkCV));
    Configure_XSA_NYears_SB->setMaximum(NCatchYears-1);
    Configure_XSA_NAges_SB->setMaximum(MaxAge-2);
    Configure_XSA_NYears_SB->setValue(ShrinkYears);
    Configure_XSA_NAges_SB->setValue(ShrinkAge);
    Configure_XSA_CMB4->setCurrentIndex(DownweightType);
    Configure_XSA_CMB5->clear();
    for (int i=FirstYear; i<=LastYear; ++i) {
        Configure_XSA_CMB5->addItem(QString::number(i));
    }
    Configure_XSA_CMB5->setCurrentIndex(0);

    Configure_XSA_CMB4->blockSignals(false);
    Configure_XSA_CMB5->blockSignals(false);
    Configure_XSA_NYears_SB->blockSignals(false);
    Configure_XSA_NAges_SB->blockSignals(false);
    Configure_XSA_LE1->blockSignals(false);

} // end loadDefaultXSAWidgets


void
nmfSSVPATab4::makeMaturityHeaders(bool isFixedMaturity,
                                  int FirstYear,
                                  int nRows,
                                  int nCols,
                                  int SpeciesIndex)
{
    QStringList strList;

    if (isFixedMaturity) {

        // Make row headers
        strList.clear();
        for (int k=0; k<nRows; ++k) {
            strList << "Age " + QString::number(k);
        }
        maturityModel[SpeciesIndex]->setVerticalHeaderLabels(strList);

        // Write column label
        strList.clear();
        strList << "PMature";
        maturityModel[SpeciesIndex]->setHorizontalHeaderLabels(strList);

    } else {

        // Make row headers
        strList.clear();
        for (int k=0; k<nRows; ++k) {
            strList << QString::number(FirstYear+k);
        }
        maturityModel[SpeciesIndex]->setVerticalHeaderLabels(strList);

        // Write column labels
        strList.clear();
        for (int k=0; k<nCols; ++k) {
            strList << "Age " + QString::number(k);
        }
        maturityModel[SpeciesIndex]->setHorizontalHeaderLabels(strList);
    }


} // end makeMaturityHeaders



void
nmfSSVPATab4::callback_SelectConfigurationCMB(QString SSVPAName)
{
    std::string SpeciesName;
    int SpeciesIndex;

    getSpeciesFromGUI(SpeciesName, SpeciesIndex);

    loadSSVPAIndexFields(SpeciesIndex, SSVPAName);

} // end callback_SelectConfigurationCMB


bool
nmfSSVPATab4::loadSSVPAIndexFields(int SpeciesIndex, QString SSVPAName)
{
    int defaultVal;
    std::string SpeciesIndexStr = std::to_string(SpeciesIndex);
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    int NumRecords;
    QString csvPath = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::InputDataDir));

    if (SSVPAName.isEmpty())
        return false;

    logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::loadSSVPAIndexFields");

    // Load Mortality rates
    fields    = {"SpeIndex","SSVPAIndex","SSVPAName","Age","AgeM1","AgeM2","PRF"};
    queryStr  = "SELECT SpeIndex,SSVPAIndex,SSVPAName,Age,AgeM1,AgeM2,PRF FROM SSVPAAgeM ";
    queryStr += " WHERE SpeIndex = " + SpeciesIndexStr + " AND SSVPAName = '" +
                  SSVPAName.toStdString() + "'";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpeIndex"].size();
    if (NumRecords == 0) {
        // If didn't find, then try without SSVPAName, in case it's NULL
        fields    = {"SpeIndex","SSVPAIndex","SSVPAName","Age","AgeM1","AgeM2","PRF"};
        queryStr  = "SELECT SpeIndex,SSVPAIndex,SSVPAName,Age,AgeM1,AgeM2,PRF FROM SSVPAAgeM ";
        queryStr += " WHERE SpeIndex = " + SpeciesIndexStr;
        dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
        NumRecords = dataMap["SpeIndex"].size();
        if (NumRecords == 0) {
            std::cout << "Error1b nmfSSVPATab4::loadSSVPAIndexFields: query returned nothing. \n" <<
                         queryStr << std::endl;
            return false;
        }
    } // end if

    // Load Mortality
    mortality_model->removeRows(0, mortality_model->count(), QModelIndex());
    for (int i=0; i<NumRecords; ++i) {
        mortality_model->append(nmfMortality {
                                    std::stoi(dataMap["Age"][i]),
                                    QString::fromStdString(dataMap["AgeM1"][i]),
                                    QString::fromStdString(dataMap["AgeM2"][i])});
    }
    SSVPAMortalityTV->resizeColumnsToContents();

    // Load Initial Selectivity
    double selVal;
    selectivity_model->removeRows(0, selectivity_model->count(), QModelIndex());
    for (int i=MinCatchAge; i<=MaxCatchAge; ++i) {
        if (i >= MinCatchAge) {
            selVal = std::stod(dataMap["PRF"][i]);
        } else {
            selVal = -9;
        }
        selectivity_model->append(nmfSelectivity { i, selVal });
    }
    SSVPAInitialSelectivityTV->resizeColumnsToContents();
    SSVPAInitialSelectivityTV->scrollToTop();


    // Read from table to set saved values for the spin boxes and other widgets
    fields    = {"SpeIndex","SSVPAName","SSVPAIndex","Type","FullRecAge","pSVPANCatYrs",
                 "pSVPANMortYrs","pSVPARefAge","Shrink","ShrinkCV","ShrinkYears","ShrinkAge",
                 "Downweight","DownweightType","DownweightYear","NFleets"};
    queryStr  = "SELECT SpeIndex,SSVPAName,SSVPAIndex,Type,FullRecAge,pSVPANCatYrs,";
    queryStr += "pSVPANMortYrs,pSVPARefAge,Shrink,ShrinkCV,ShrinkYears,ShrinkAge,";
    queryStr += "Downweight,DownweightType,DownweightYear,NFleets FROM SpeSSVPA ";
    queryStr += "WHERE SpeIndex = " + SpeciesIndexStr +
                " AND SSVPAName = '" + SSVPAName.toStdString() + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["SpeIndex"].size() == 0) {
        std::cout << "Error2 nmfSSVPATab4::loadSSVPAIndexFields: query returned nothing: " <<
                     queryStr << std::endl;
        return false;
    }

    int Type           = std::stoi(dataMap["Type"][0]);
    int FullRecAge     = std::stoi(dataMap["FullRecAge"][0]);
    int pSVPANCatYrs   = std::stoi(dataMap["pSVPANCatYrs"][0]);
    int pSVPANMortYrs  = std::stoi(dataMap["pSVPANMortYrs"][0]);
    int pSVPARefAge    = std::stoi(dataMap["pSVPARefAge"][0]);
    int NFleets        = std::stoi(dataMap["NFleets"][0]);
    //int Shrink         = std::stoi(dataMap["Shrink"][0]);
    double ShrinkCV    = std::stod(dataMap["ShrinkCV"][0]);
    int ShrinkYears    = std::stoi(dataMap["ShrinkYears"][0]);
    int ShrinkAge      = std::stoi(dataMap["ShrinkAge"][0]);
    //int Downweight     = std::stoi(dataMap["Downweight"][0]);
    int DownweightType = std::stoi(dataMap["DownweightType"][0]);
    int DownweightYear = std::stoi(dataMap["DownweightYear"][0]);

    //SelectVPATypeCMB->blockSignals(true);
    SelectVPATypeCMB->setCurrentIndex(Type);
    //SelectVPATypeCMB->blockSignals(false);

    defaultVal = (FullRecAge == 0)    ? Configure_Cohort_SB1->minimum() : FullRecAge;
    Configure_Cohort_SB1->setValue(defaultVal);
    defaultVal = (pSVPANCatYrs == 0)  ? Configure_Cohort_SB2->minimum() : pSVPANCatYrs;
    Configure_Cohort_SB2->setValue(defaultVal);
    defaultVal = (pSVPANMortYrs == 0) ? Configure_Cohort_SB3->minimum() : pSVPANMortYrs;
    Configure_Cohort_SB3->setValue(defaultVal);
    defaultVal = (pSVPARefAge == 0)   ? Configure_Cohort_SB4->minimum() : pSVPARefAge;
    Configure_Cohort_SB4->setValue(defaultVal);

    if ( FullRecAge * pSVPANCatYrs * pSVPANMortYrs == 0 ) {
        MarkAsDirty("SpeSSVPA");
    }

//    loadDefaultXSAWidgets(FirstYear,LastYear,MaxAge,
//                           ShrinkCV,ShrinkYears,ShrinkAge,
//                           DownWeightType);
//    loadDefaultXSAWidgets(FirstYear,LastYear,MaxAge,
//                          Downweight,DownweightType,DownweightYear,
//                          Shrink,ShrinkCV,ShrinkYears,ShrinkAge);

    Configure_XSA_CMB4->blockSignals(true);
    Configure_XSA_CMB5->blockSignals(true);
    Configure_XSA_LE1->blockSignals(true);
    Configure_XSA_NYears_SB->blockSignals(true);
    Configure_XSA_NAges_SB->blockSignals(true);

    Configure_XSA_LE1->setText(QString::number(ShrinkCV));
    Configure_XSA_NYears_SB->setValue(ShrinkYears);
    Configure_XSA_NAges_SB->setValue(ShrinkAge);
    Configure_XSA_CMB4->setCurrentIndex(DownweightType);
    Configure_XSA_CMB5->clear();
    for (int i=FirstYear; i<=LastYear; ++i) {
        Configure_XSA_CMB5->addItem(QString::number(i));
    }
    Configure_XSA_CMB5->setCurrentIndex(DownweightYear-FirstYear);

    Configure_XSA_CMB4->blockSignals(false);
    Configure_XSA_CMB5->blockSignals(false);
    Configure_XSA_LE1->blockSignals(false);
    Configure_XSA_NYears_SB->blockSignals(false);
    Configure_XSA_NAges_SB->blockSignals(false);

    //
    // Panel 3. Populate Effort Tuned widgets.
    //
    // Set up widgets for Effort Tuned VPA
    int NumFleets = std::stoi(dataMap["NFleets"][0]);
    if (NumFleets == 0) {
        logger->logMsg(nmfConstants::Normal,"NFleets is 0. Using current Catch data as default Fleet.");
        return false;
    }


//    int NumYears = LastYear-FirstYear+1;
//    int NumAges  = MaxCatchAge-MinCatchAge+1;

    //loadDefaultFleetWidgets(NumFleets,NumYears,NumAges,SpeciesIndex);

    loadDefaultLaurecShepardWidgets(MaxCatchAge,FullRecAge,NumFleets);

    // If there's Fleet Effort/Catchdata load it...
    // Check SpeTuneEffort; If there's data, means there's data also in SpeTuneCatch.
    fields     = {"SpeName","SpeIndex","SSVPAName","Fleet","Year","Effort"};
    queryStr   = "SELECT SpeName,SpeIndex,SSVPAName,Fleet,Year,Effort FROM SpeTuneEffort";
    queryStr  += " WHERE SpeIndex = " + std::to_string(SpeciesIndex);
    dataMap    = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["SpeIndex"].size();
    if (NumRecords > 0) {
        Load_FleetEffortData(QDir(csvPath).filePath("SpeTuneEffort.csv"));
        if (NFleets == 1)
            Load_CatchData(QDir(csvPath).filePath("SpeCatch.csv"));
        else
            Load_FleetCatchData(QDir(csvPath).filePath("SpeTuneCatch.csv"));
    }




    // Reset XSA combo box
    SSVPAExtendedIndicesCMB->blockSignals(false);

    Configure_Cohort_SB1->blockSignals(false);
    Configure_Cohort_SB2->blockSignals(false);
    Configure_Cohort_SB3->blockSignals(false);
    Configure_Cohort_SB4->blockSignals(false);

    Configure_EffortTuned_SB1->blockSignals(true);
    Configure_EffortTuned_SelectAFleetCMB->blockSignals(false);
    SSVPAFleetDataTV->blockSignals(true);

logger->logMsg(nmfConstants::Normal,"nmfSSVPATab4::loadSSVPAIndexFields Complete");

    return true;

} // end loadSSVPAIndexFields

/*
void
nmfSSVPATab4::loadDefaultFleetWidgets(int NumFleets,
                                      int NumYears,
                                      int NumAges,
                                      int SpeciesIndex)
{
    int m=0;
    double val=0;
    double checksum=0;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    // RSK - todo
    // Update this logic.... Only take from SpeCatch if there's no data
    // saved in SpeTuneCatch and SpeTuneEffort for this 1 fleet.
    m = 0;
    if (NumFleets == 1) {
        fields   = {"Catch"};
        queryStr = "SELECT Catch FROM SpeCatch WHERE SpeIndex = " + std::to_string(SpeciesIndex);
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (int i = 0; i < NumYears; ++i) {
            for (int j = 0; j < NumAges; ++j) {
                FleetData[0][SpeciesIndex][i][j+1] = std::stod(dataMap["Catch"][m++]);
std::cout << "Loading 0: " << 0 << "," << SpeciesIndex << "," << i << ", " << j+1 << std::endl;
                checksum += FleetData[0][SpeciesIndex][i][j+1];
            }
        }
std::cout << "NumYears: " << NumYears << std::endl;
std::cout << "NumAges:  " << NumAges  << std::endl;
std::cout << "checksum 00: " << checksum << std::endl;
    } else {
        // Load Fleet data into FleetData multiarray
        m = 0;
        fields   = {"SpeIndex","Year","Fleet","Effort"};
        queryStr = "SELECT SpeIndex,Year,Fleet,Effort FROM SpeTuneEffort WHERE SpeIndex = " +
                std::to_string(SpeciesIndex) +
                " ORDER BY SpeIndex,Fleet,Year,Effort";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (int i=0; i<NumFleets; ++i) {
            for (int j=0; j<LastYear-FirstYear+1; ++j) {
                val= std::stod(dataMap["Effort"][m++]);
                FleetData[i][SpeciesIndex][j][0] = val;
std::cout << "Loading 1: " << i << "," << SpeciesIndex << "," << j << ", 0" << std::endl;
            }
        }

        // Load Catch data into FleetData multiarray
        m = 0;
        fields   = {"SpeIndex","Year","Fleet","Age","Catch"};
        queryStr = "SELECT SpeIndex,Year,Fleet,Age,Catch FROM SpeTuneCatch WHERE SpeIndex = "  +
                std::to_string(SpeciesIndex) +
                " ORDER BY SpeIndex,Fleet,Year,Age,Catch";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["Catch"].size() == NumFleets*NumYears*NumAges) {
            for (int i=0; i<NumFleets; ++i) {
                for (int j=0; j<NumYears; ++j) {
                    for (int age=0; age<NumAges; ++age) {
                        val= std::stod(dataMap["Catch"][m++]);
                        FleetData[i][SpeciesIndex][j][age+1] = val;
std::cout << "Loading 2: " << i << "," << SpeciesIndex << "," << j << "," << age+1 << std::endl;
                    } // end for age
                } // end for j
            } // end for i
        } else {
            std::cout << "Warning: SpeTuneCatch size of " << dataMap["Catch"].size() <<
                         " didn't match computed size of " << NumFleets*NumYears*NumAges << std::endl;
        }
    } // end if NumFleets

    Configure_EffortTuned_SetNumFleetsCMB->setCurrentText(QString::number(NumFleets));
    if (NumFleets > 0) {
        callback_EffortTuned_SetNumFleetsCMB(NumFleets,nmfConstants::NoSignal);
        callback_EffortTuned_SelectAFleetCMB(0);
    }

} // end loadDefaultFleetWidgets
*/


// Use array of model ptrs instead of 4 dim array
void
nmfSSVPATab4::InitializeNumFleets(int NumFleets)
{
    QStringList strList;
    std::string suffix;
    QStandardItem *item;
    int numRows = LastYear-FirstYear+1;
    int numCols = (MaxCatchAge-MinCatchAge+1)+1;

    for (int i=0; i<NumFleets; ++i) {

        if (fleetModel[i] != NULL)
            disconnect(fleetModel[i],0,0,0);

        fleetModel[i] = new QStandardItemModel(numRows,numCols);

        for (int j=0; j<numRows; ++j)
            for (int k=0; k<numCols; ++k) {
                item = new QStandardItem("");
                item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                fleetModel[i]->setItem(j, k, item);
            }

        connect(fleetModel[i], SIGNAL(itemChanged(QStandardItem *)),
                this,   SLOT(callback_EffortTuned_ItemChanged(QStandardItem *)));

        // Make row headers
        strList.clear();
        for (int k=FirstYear; k<=LastYear; ++k) {
            strList << QString::number(k);
        }
        fleetModel[i]->setVerticalHeaderLabels(strList);

        // Make column headers
        strList.clear();
        strList << "Effort";
        for (int k=MinCatchAge; k<=MaxCatchAge; ++k) {
            suffix = (k == MaxCatchAge) ? "+" : "";
            strList << "Age = " + QString::number(k) + suffix.c_str();
        }
        fleetModel[i]->setHorizontalHeaderLabels(strList);
    }

    SSVPAFleetDataTV->setModel(fleetModel[0]);

} // end InitializeNumFleets



void
nmfSSVPATab4::clearWidgets()
{
//    if (smodelEffort)
//        smodelEffort->clear();
//    if (smodelXSA)
//        smodelXSA->clear();
}



