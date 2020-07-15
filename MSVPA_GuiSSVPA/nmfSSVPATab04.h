#ifndef NMFSSVPATAB4_H
#define NMFSSVPATAB4_H

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QFontDatabase>
#include <QGroupBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTableView>
#include <QTextEdit>
#include <QTextStream>
#include <QUiLoader>
#include <QVector>

#include "nmfMortalityModel.h"
#include "nmfInitialSelectivityModel.h"
#include "nmfDatabase.h"
#include "nmfSSVPA.h"
#include "nmfLogger.h"
#include "nmfConstantsMSVPA.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/filesystem.hpp>
#include <boost/multi_array.hpp>

typedef boost::multi_array<double, 3> Boost3DArrayDouble;
typedef boost::multi_array<double, 4> Boost4DArrayDouble;
typedef Boost3DArrayDouble::index index3D;
typedef Boost4DArrayDouble::index index4D;

class nmfSSVPATab4: public QObject
{
    Q_OBJECT

    nmfLogger      *logger;

    QTabWidget     *SSVPA_Tabs;
    QWidget        *SSVPA_Tab4_Widget;
    QTableView     *SSVPAMortalityTV;
    QTableView     *SSVPAMaturityTV;
    QTableView     *SSVPAInitialSelectivityTV;
    QTableView     *SSVPAFleetDataTV;
    QTableView     *SSVPAIndicesTV;
    QDoubleSpinBox *ResidualNaturalMortalityDSB;
    QDoubleSpinBox *PredationMortalityDSB;
    QDoubleSpinBox *LastYearMortalityRateDSB;
    QSpinBox       *ReferenceAgeSB;
    QComboBox      *SelectConfigurationCMB;
    QComboBox      *SSVPAExtendedIndicesCMB;
    QLineEdit      *SSVPAExtendedStartTimeLE;
    QLineEdit      *SSVPAExtendedEndTimeLE;
    QComboBox      *Configure_EffortTuned_SetNumFleetsCMB;
    QComboBox      *Configure_EffortTuned_SelectAFleetCMB;
    QLabel         *Configure_EffortTuned_LB3;
    QSpinBox       *Configure_Cohort_SB1;
    QSpinBox       *Configure_Cohort_SB2;
    QSpinBox       *Configure_Cohort_SB3;
    QSpinBox       *Configure_Cohort_SB4;
    QLabel         *Configure_Cohort_LB1;
    QLabel         *Configure_Cohort_LB2;
    QLabel         *Configure_Cohort_LB3;
    QLabel         *Configure_Cohort_LB4;
    QSpinBox       *Configure_EffortTuned_SB1;
    QLabel         *Configure_EffortTuned_LB1;
    QLineEdit      *Configure_XSA_LE1;
    QSpinBox       *Configure_XSA_NYears_SB;
    QSpinBox       *Configure_XSA_NAges_SB;
    QComboBox      *Configure_XSA_CMB4;
    QComboBox      *Configure_XSA_CMB5;
    QPushButton    *SSVPA_Tab4_PrevPB;
    QPushButton    *SSVPA_Tab4_CheckAllPB;
    QPushButton    *SSVPA_Tab4_LoadMortalityPB;
    QPushButton    *SSVPA_Tab4_SaveMortalityPB;
    QPushButton    *SSVPA_Tab4_SavePB;
    QPushButton    *SSVPA_Tab4_LoadFleetPB;
    QPushButton    *SSVPA_Tab4_SaveFleetPB;
    QPushButton    *SSVPA_Tab4_LoadIndicesPB;
    QPushButton    *SSVPA_Tab4_SaveIndicesPB;
    QPushButton    *SSVPA_Tab4_LoadMaturityPB;
    QPushButton    *SSVPA_Tab4_SaveMaturityPB;
    QGroupBox      *Configure_Shrinkage_GB;
    QGroupBox      *Configure_Downweight_GB;
    QGroupBox      *SSVPAMortalityRatesGB;
    QGroupBox      *SSVPASelectConfigurationAndTypeGB;
    QGroupBox      *SSVPAConfigure1GB;
    QGroupBox      *SSVPAConfigure2GB;
    QGroupBox      *SSVPAConfigure3GB;
    QGroupBox      *SSVPAConfigure4GB;
    QComboBox      *SelectVPATypeCMB;
    QPushButton    *Configure_XSA_AddIndexPB;
    QPushButton    *Configure_XSA_DelIndexPB;
    QGroupBox      *SSVPAMaturityRatesGB;
    QSlider        *SSVPAMaturitySL;
    QLineEdit      *SSVPAMaturityMinLE;
    QLineEdit      *SSVPAMaturityMaxLE;
    QSlider        *NaturalMortalitySL;
    QLineEdit      *MortalityMinLE;
    QLineEdit      *MortalityMaxLE;
    QCheckBox      *NaturalMortalityCB;
    QLabel         *ResidualNaturalMortalityLBL;
    QLabel         *PredationMortalityLBL;
    QLineEdit      *SelectivityMinLE;
    QLineEdit      *SelectivityMaxLE;
    QPushButton    *AddConfiguration;
    QPushButton    *DelConfiguration;
    QString         SpeMaturityCSVFile;
    QPushButton    *SSVPA_Tab4_RunPB;
    QSlider        *SelectivitySL;

    QStandardItemModel *maturityModel[nmfConstantsMSVPA::MaxNumSmodels];
    QStandardItemModel *fleetModel[10];
    QStandardItemModel *xsaModel[nmfConstantsMSVPA::MaxNumSmodels];
    QStandardItemModel *currentMaturityModel;

    nmfMortalityModel          *mortality_model;
    nmfInitialSelectivityModel *selectivity_model;

    int MinCatchAge;
    int MaxCatchAge;
    int FirstYear;
    int LastYear;
    int SpeciesIndex;
    int NCatchYears;
    int isPlusClass;
    bool isFixedMaturity;
    bool okToRun;
    double maturitySliderScale;
    double mortalitySliderScale;
    double selectivitySliderScale;
    std::string ProjectDir;
    std::vector<double> Alpha;
    std::vector<double> Beta;
    QString SpeMortalityCSVFile;
    QString SpeSSVPACSVFile;
    QString CurrentTable;
    std::vector<QString> AllSpecies;

    nmfSSVPA ssvpaApi;

    nmfDatabase *databasePtr;
    QTextEdit *SSVPAOutputTE;
    //std::map<std::string, int> SpeciesNametoIndexMap;


public:

    nmfSSVPATab4(QTabWidget *tabs,
                 nmfLogger  *logger,
                 std::string &ProjectDir);
    virtual ~nmfSSVPATab4();
    void clearWidgets();
    void enableWidgets(bool enable);
    bool checkFleetDataValue(QStandardItem *item);
    void AdjustButtonsForMissingData();
    void Load_SSVPAAgeM();
    bool removeFromTableAndCSVFile(QString SSVPAName,
                                   QString TableName);
    void getSpeciesFromGUI(std::string &SpeciesName,
                           int &SpeciesIndex);

    bool runSSVPA_CohortAnalysis(
            std::map<std::string,int> &CohortAnalysisGuiData,
            std::string SSVPAName,
            int FirstCatchYear,
            int LastCatchYear,
            int NCatchYears,
            int MinCatchAge,
            int MaxCatchAge,
            int MaxAge,
            double &CV,
            double &SE,
            int &NIt,
            boost::numeric::ublas::matrix<double> &Abundance,
            boost::numeric::ublas::matrix<double> &M_NaturalMortality,
            boost::numeric::ublas::matrix<double> &F_FishingMortality,
            std::vector<std::string> &rowTitles,
            std::vector<std::string> &colTitles,
            std::string &elapsedTimeStr);

    void runSSVPA_SeparableVPA(
            int MinCatchAge,
            int MaxCatchAge,
            int MaxAge,
            int NumCatchYears,
            int NumCatchAges,
            boost::numeric::ublas::matrix<double> &CatchMatrix,
            double &CV, double &SE, int &NIt,
            boost::numeric::ublas::matrix<double> &Abundance,
            boost::numeric::ublas::matrix<double> &M,
            boost::numeric::ublas::matrix<double> &F,
            std::vector<std::string> &rowTitles,
            std::vector<std::string> &colTitles,
            std::string &elapsedTimeStr);

    void runSSVPA_EffortTuned(
            std::map<std::string,int> &EffortTunedGuiData,
            int &SSVPAIndex,
            std::string &SSVPAName,
            int &MaxCatchAge,
            int &NCatchYears,
            double &CV,
            double &SE,
            int &NIt,
            boost::numeric::ublas::matrix<double> &Abundance,
            boost::numeric::ublas::matrix<double> &M_NaturalMortality,
            boost::numeric::ublas::matrix<double> &F_FishingMortality,
            std::vector<std::string> &rowTitles,
            std::vector<std::string> &colTitles,
            std::string &elapsedTimeStr);

    void runSSVPA_XSA(
            std::map<std::string,std::string> &XSAGuiData,
            std::string SSVPAName,
            double &CV,
            double &SE,
            int &NIt,
            boost::numeric::ublas::matrix<double> &Abundance,
            boost::numeric::ublas::matrix<double> &M_NaturalMortality,
            boost::numeric::ublas::matrix<double> &F_FishingMortality,
            std::vector<std::string> &rowTitles,
            std::vector<std::string> &colTitles,
            std::string &elapsedTimeStr);

    void MarkAsDirty(std::string table);
    void MarkAsClean();
    void SetNumFleets(int numFleets);    
    void Load_FleetEffortData(QString filename);
    void Load_CatchData(QString filename);
    void Load_FleetCatchData(QString filename);
    bool Load_XSAIndices(QString filename);
    bool Load_XSAData(QString filename);
    void enableAllWidgets(bool enable);
    bool displayMsg(std::string msg);
    bool loadSSVPAIndexFields(int SpeIndex, QString SSVPAName);
    bool removeFromCSVFiles(QString SSVPANameToRemove);
    void clearModels();
    bool runDBChecks(QString type, QString &BadTables);
    void showVPAGroupBox(int whichGroupBox);
    void makeMaturityHeaders(bool isFixedMaturity, int FirstYear,
                             int nRows, int nCols, int SpeciesIndex);
    void runSSVPA();
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void loadDefaultWidgets(nmfDatabase *theDatabasePtr,
                            std::vector<QString> &theAllSpecies,
                            int SelectedSpeciesIndex);
    void loadDefaultCohortWidgets(int MaxCatchAge, int NCatchYears);
    void loadDefaultLaurecShepardWidgets(int MaxCatchAge, int FullRecAge, int NFleets);
    void loadDefaultXSAWidgets(
            int FirstYear,
            int LastYear,
            int MaxAge,
            int Downweight,
            int DownweightType,
            int DownweightYear,
            int Shrink,
            double ShrinkCV,
            int ShrinkYears,
            int ShrinkAge);
    void InitializeNumFleets(int NumFleets);
    void InitializeXSAIndex(int index);
    bool Delete_SpeXSAIndices(int &indexToDelete);
    void Delete_SpeXSAData(int indexToDelete);

    bool    Save_XSAIndex(int index);
    bool    Save_SpeSSVPA(int SpeciesIndex);
    bool    Save_InitialSelectivityAndMortality(int SpeciesIndex);
    //bool    Save_SSVPAAgeM_DB_Table();
    bool    Save_FleetNumData(int NumFleets, int SpeciesIndex);
    bool    Save_FleetEffortData(int NumFleets, std::string SpeciesName, int SpeciesIndex);
    bool    Save_FleetCatchData(int NumFleets, std::string SpeciesName, int SpeciesIndex);
    QString Save_Config();
    QString Save_Fleet();
    QString Save_SpeMaturity();
    QString Save_InitialSelectivity();
    QString Save_XSAIndexData();
    void updateProjectDir(std::string newProjectDir);


signals:
    void InitializePage(std::string type, int index, bool withCallback);
    void TableDataChanged(std::string tableName);
    void RunSSVPA(std::string SSVPAType);
    void ReselectSpecies(std::string currentSpecies, bool withCallback);
    void RunSSVPAPB(bool unused);
    void EnableRunSSVPAPB(bool toggle);
    void MarkSSVPAAsClean();

public Q_SLOTS:
    void callback_Cohort_SB1(int fullyRecruitedAge);
    void callback_Cohort_SB1(int fullyRecruitedAge, bool sendSignal);
    void callback_Cohort_SB2(int pSVPANCatYrs);
    void callback_Cohort_SB3(int pSVPANMortYrs);
    void callback_Cohort_SB4(int pSVPARefAge);
    void callback_EffortTuned_SB1(int fullyRecruitedAge);
    void callback_EffortTuned_SetNumFleetsCMB(int value);
    void callback_EffortTuned_SetNumFleetsCMB(int value, bool noSignal);
    void callback_EffortTuned_SelectAFleetCMB(int value);
    void callback_EffortTuned_ItemChanged(QStandardItem *item);
    void callback_XSA_ItemChanged(QStandardItem *item);
    void callback_XSA_IndexCMB(int val);
    void callback_XSA_StartTimeLE();
    void callback_XSA_EndTimeLE();
    void callback_SSVPA_Tab4_PrevPB(bool unused);
    void callback_SSVPA_Tab4_CheckAllPB(bool unused);
    void callback_SSVPA_Tab4_LoadFleetPB(bool unused);
    void callback_SelectVPATypeCMB(int item);
    void callback_SSVPA_Tab4_AddIndexPB(bool unused);
    void callback_SSVPA_Tab4_DelIndexPB(bool unused);
    void callback_SSVPA_Tab4_LoadIndicesPB(bool unused);
    void callback_SSVPAMortalityTV(QModelIndex index);
    void callback_MortalitySliderPressed();
    void callback_MortalitySliderReleased();
    void callback_MortalitySliderMoved(int value);
    void callback_MortalityMinEdited();
    void callback_MortalityMaxEdited();
    void callback_MortalityChecked(int state);
    void callback_MortalityChecked(int state, bool okToEmit);
    void callback_SSVPA_Tab4_LoadMortalityPB(bool unused);
    void callback_SSVPA_Tab4_SavePB(bool unused);
    void callback_SSVPA_Tab4_AddConfigPB(bool unused);
    void callback_SSVPA_Tab4_DelConfigPB(bool unused);
    void callback_SelectConfigurationCMB(QString SSVPAName);
    void callback_Configure_XSA_LE1();
    void callback_Configure_XSA_NYears_SB(int value);
    void callback_Configure_XSA_NAges_SB(int value);
    void callback_updateModel(
            const QModelIndex &topLeft,
            const QModelIndex &bottomRight);
    void callback_SSVPA_Tab4_LoadMaturityPB(bool unused);
    void callback_Maturity_ItemChanged(QStandardItem *item);
    void callback_SSVPAMaturitySliderMoved(int value);
    void callback_SSVPAMaturitySliderPressed();
    void callback_SSVPAMaturitySliderReleased();
    void callback_RunSSVPA(QModelIndex a, QModelIndex b);
    void callback_Configure_XSA_CMB4(int Type);
    void callback_Configure_XSA_CMB5(QString year);
    void callback_Configure_Downweight_GB(bool checked);
    void callback_Configure_Shrinkage_GB(bool checked);
    void callback_ResidualMortalityChanged(double value);
    void callback_PredationMortalityChanged(double value);
    void callback_InitialSelectivityChanged(QModelIndex indexA,
                                            QModelIndex indexB);
    void callback_RefAgeChanged(int val);
    void callback_TermFChanged(double val);
    void callback_SSVPA_Tab4_RunPB(bool unused);
    void callback_SelectivitySliderMoved(int value);
    void callback_SelectivitySliderPressed();
    void callback_SelectivitySliderReleased();
};

#endif // NMFSSVPATAB4_H
