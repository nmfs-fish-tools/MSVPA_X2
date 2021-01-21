/**
 @file MSVPA_X2_GUI.h
 @author rklasky
 @copyright 2017 NOAA - National Marine Fisheries Service
 @brief This contains the class definition for the main Qt window.  If this file changes, moc must be re-run.
 @date Dec 8, 2016
 */

#ifndef NMF_MAIN_WINDOW_H
#define NMF_MAIN_WINDOW_H


#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/filesystem.hpp>
#include <boost/version.hpp>

#include <QObject>

#include "nmfDatabase.h"
#include "nmfDatabaseConnectDialog.h"
#include "nmfTestDialog.h"
#include "nmfEntityModel.h"
#include "nmfMaturityModel.h"
#include "nmfData.h"
#include "nmfSSVPA.h"
#include "nmfMSVPA.h"
#include "nmfForecast.h"
#include "nmfOutputChart3D.h"
#include "nmfMSVPAProgressWidget.h"
#include "nmfLogWidget.h"
#include "nmfOutputChart3DBarModifier.h"
#include "nmfSetupTab01.h"
#include "nmfSetupTab02.h"
#include "nmfSetupTab03.h"
#include "nmfSSVPATab01.h"
#include "nmfSSVPATab02.h"
#include "nmfSSVPATab03.h"
#include "nmfSSVPATab04.h"
#include "nmfMSVPATab01.h"
#include "nmfMSVPATab02.h"
#include "nmfMSVPATab03.h"
#include "nmfMSVPATab04.h"
#include "nmfMSVPATab05.h"
#include "nmfMSVPATab06.h"
#include "nmfMSVPATab07.h"
#include "nmfMSVPATab08.h"
#include "nmfMSVPATab09.h"
#include "nmfMSVPATab10.h"
#include "nmfMSVPATab11.h"
#include "nmfMSVPATab12.h"
#include "nmfForecastTab01.h"
#include "nmfForecastTab02.h"
#include "nmfForecastTab03.h"
#include "nmfForecastTab04.h"
#include "nmfForecastTab05.h"
#include "nmfOutputChartBase.h"
#include "nmfOutputChartBar.h"
#include "nmfOutputChartStackedBar.h"
#include "nmfOutputChartLine.h"
#include "nmfLogger.h"
#include "nmfUtils.h"
#include "nmfUtilsQt.h"
#include "MSVPAVisualizationNode.h"
#include "PreferencesDialog.h"

//#include "ui_MSVPA_X2.h"

#include <QThreadPool>

typedef boost::multi_array<double, 3> Boost3DArrayDouble;
typedef boost::multi_array<double, 4> Boost4DArrayDouble;

QT_CHARTS_USE_NAMESPACE


struct MSVPASpeciesStruct {
    std::string Species;
    int         FirstYear;
    int         LastYear;
    int         EarliestAge;
    int         LastAge;
    bool        LastAgeIsPlus;
    std::string CatchUnits;
    bool        FixedMaturityAtAge;
    bool        VariableMaturityAtAge;
    int         WeightAtAge;
    std::string WeightUnits;
    int         SizeAtAge;
    std::string SizeUnits;
};


struct BiomassPredatorStruct {
    std::string Species;
    int         FirstYear;
    int         LastYear;
    std::string WeightUnits;
    int         MinimumLength;
    int         MaximumLength;
    std::string LengthUnits;
    bool        IncludeSizeStruct;
    int         NumSizeCategories;
    std::string WeightIndUnits;
};

struct EffortTunedStruct {
    int FullRecAge;
    int NFleets;
};



class nmfConfigData {
    std::string databaseName;
};

namespace Ui {
    class nmfMainWindow;
}

class nmfMainWindow: public QMainWindow
{

    Q_OBJECT

private:

//    nmfSSVPA     ssvpaApi;
//    nmfMSVPA    *msvpaApi;
//    nmfForecast *forecastApi;

    std::map<std::string, void(*)(
            nmfDatabase* m_databasePtr,
            QChart* chart,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int Forecast_FirstYear,
            int Forecast_NYears,
            std::string selectedSpecies,
            int PredNum,
            std::string selectedSpeciesAgeSizeClass,
            QTableWidget *chartDataTbW
            )> FunctionMap;

    bool               m_CatchDataInitialized;
    bool               m_isStartUpOK;
    Ui::nmfMainWindow* m_UI;
    nmfLogger*         m_logger;
    nmfDatabase*       m_databasePtr;
    QProgressDialog*   m_MSVPAProgressDlg;
    QProgressDialog*   m_ForecastProgressDlg;
    QDialog*           m_TableNamesDlg;
    QWidget*           m_TableNamesWidget;


    int timerVal;
    int MSVPALoopsBeforeChecking;
    double YMaxFullyRecruitedAge;
    double YMaxAgeSizeClass;
    double YMaxSeason;
    std::string m_ProjectDir;
    std::string ProjectDatabase;
    std::string ProjectName;
    std::string Hostname;
    std::string m_Username;
    std::string m_Password;
    std::string Session;
    std::string lastPauseCmd;
    QDialog *prefDlg;

    QChartView *chartView;
    QHBoxLayout *hMainLayout;
    QHBoxLayout *hProgressLayout;
    QTreeWidget *NavigatorTree;
    QWidget *NavigatorTreeWidget;

    nmfConfigData configData;
    nmfMSVPAProgressWidget *progressWidgetMSVPA;
    nmfMSVPAProgressWidget *progressWidgetForecast;
    nmfLogWidget *logWidget;

    QWidget *chartView3DContainer;
    QSqlQueryModel *catchAtAge_ptr;
    QSqlQueryModel *weightAtAge_ptr;
    QSqlQueryModel *sizeAtAge_ptr;
    QSqlQueryModel *maturity_ptr;
    nmfData* data;

    nmfMortalityModel mortality_model;
    nmfVonBertModel   vonBert_model;
    nmfEntityModel    entity_model;
    nmfEntityModel    forecast_model;
    nmfEntityModel    scenario_model;
    nmfMaturityModel  maturity_model;
    nmfInitialSelectivityModel selectivity_model;
    std::vector<nmfOutputChart3D *> outputCharts3D;
    nmfOutputChart3DBarModifier *m_modifier;
    QLabel* selectDataTypeLBL;
    QLabel* selectSpeciesLBL;
    QLabel* selectVariableLBL;
    QLabel* selectByVariablesLBL;
    QLabel* selectSeasonLBL;
    QLabel* selectSpeciesAgeSizeClassLBL;
    QLabel* gridLine2dLBL;
    //QLabel* gridLine3dLBL;
    QComboBox *selectDataTypeCMB;
    QComboBox *selectSpeciesCMB;
    QComboBox *selectVariableCMB;
    QComboBox *selectByVariablesCMB;
    QComboBox *selectSeasonCMB;
    QCheckBox *selectSeasonCB;
    QCheckBox *horzGridLine2dCB;
    QCheckBox *vertGridLine2dCB;
    QCheckBox *horzGridLine3dCB;
    QCheckBox *vertGridLine3dCB;
    QLineEdit *selectSeasonLE;
    QComboBox *selectSpeciesAgeSizeClassCMB;
    QCheckBox *selectSpeciesAgeSizeClassCB;
    QLineEdit *selectSpeciesAgeSizeClassLE;
    QLabel *selectYPRAnalysisTypeLBL;
    QLabel *selectYearsLBL;
    QLabel *selectFullyRecruitedAgeLBL;
    QComboBox *selectYPRAnalysisTypeCMB;
    QListWidget *selectYearsLW;
    QComboBox *selectFullyRecruitedAgeCMB;
    QCheckBox *selectFullyRecruitedAgeCB;
    QLineEdit *selectFullyRecruitedAgeLE;
    QLabel *selectPreySpeciesLBL;
    QComboBox *selectPreySpeciesCMB;
    QVBoxLayout *controlsLayout;
    QChart *chart;
    QChart *progressChartMSVPA;
    QChart *progressChartForecast;
    QBrush navigatorBackgroundBrush;
    QTimer* timer;
    QTimer* progressSetupTimerMSVPA;
    QTimer* progressChartTimerMSVPA;
    QTimer* progressSetupTimerForecast;
    QTimer* progressChartTimerForecast;
    QLineEdit   *sliderYMaxValueLE; // Needed to put this here so have access to it when changing stylesheets.
    QPushButton *flipLeftPB;
    QPushButton *flipRightPB;
//    nmfEntityListLV *EntityListLV;
//    nmfEntityListLV *ForecastListLV;
//    nmfEntityListLV *ScenarioListLV;
    QComboBox  *SelectVPATypeCMB;
    QComboBox  *SelectConfigurationCMB;
    QSpinBox   *Configure_Cohort_SB1;
    QSpinBox   *Configure_Cohort_SB2;
    QSpinBox   *Configure_Cohort_SB3;
    QSpinBox   *Configure_Cohort_SB4;
    QTableView *SSVPAInitialSelectivityTV;
    QTableView *SSVPAMaturityTV;
    QTableView *SSVPAMortalityTV;
    QTableView *SSVPAFleetDataTV;
    QTableView *SSVPAIndicesTV;
    QSlider    *NaturalMortalitySL;
    QSlider    *SelectivitySL;
    QLineEdit  *MortalityMaxLE;
    QLineEdit  *MortalityMinLE;
    QDoubleSpinBox *LastYearMortalityRateDSB;
    QDoubleSpinBox *ResidualNaturalMortalityDSB;
    QDoubleSpinBox *PredationMortalityDSB;
    QLineEdit  *SelectivityMaxLE;
    QLineEdit  *SelectivityMinLE;
    QSpinBox   *ReferenceAgeSB;
    QCheckBox  *NaturalMortalityCB;
    QGroupBox  *SSVPASelectConfigurationAndTypeGB;
    QSpinBox   *Configure_EffortTuned_SB1;
    QSpinBox   *Configure_XSA_NYears_SB;
    QSpinBox   *Configure_XSA_NAges_SB;
    QComboBox  *Configure_XSA_CMB4;
    QComboBox  *Configure_XSA_CMB5;
    QGroupBox  *Configure_Downweight_GB;
    QGroupBox  *Configure_XSA_GB;
    QLineEdit  *Configure_XSA_LE1;
    QComboBox  *SSVPAExtendedIndicesCMB;
    QListView *EntityListLV;
    QListView *ForecastListLV;
    QListView *ScenarioListLV;
    QWidget *entityDockWidget;
    QVBoxLayout *vLayout1;
    QLabel *ForecastListLBL;
    QLabel *ScenarioListLBL;
    //nmfEntityListObject *EntityListObject;
    //nmfSSVPATabOtherPred *SSVPA_TabOtherPred_ptr;
    nmfSetup_Tab1*          Setup_Tab1_ptr;
    nmfSetup_Tab2*          Setup_Tab2_ptr;
    nmfSetup_Tab3*          Setup_Tab3_ptr;
    nmfSSVPATab1*           SSVPA_Tab1_ptr;
    nmfSSVPATab2*           SSVPA_Tab2_ptr;
    nmfSSVPATab3*           SSVPA_Tab3_ptr;
    nmfSSVPATab4*           SSVPA_Tab4_ptr;
    nmfMSVPATab1*           MSVPA_Tab1_ptr;
    nmfMSVPATab2*           MSVPA_Tab2_ptr;
    nmfMSVPATab3*           MSVPA_Tab3_ptr;
    nmfMSVPATab4*           MSVPA_Tab4_ptr;
    nmfMSVPATab5*           MSVPA_Tab5_ptr;
    nmfMSVPATab6*           MSVPA_Tab6_ptr;
    nmfMSVPATab7*           MSVPA_Tab7_ptr;
    nmfMSVPATab8*           MSVPA_Tab8_ptr;
    nmfMSVPATab9*           MSVPA_Tab9_ptr;
    nmfMSVPATab10*          MSVPA_Tab10_ptr;
    nmfMSVPATab11*          MSVPA_Tab11_ptr;
    nmfMSVPATab12*          MSVPA_Tab12_ptr;
    nmfForecastTab1*        Forecast_Tab1_ptr;
    nmfForecastTab2*        Forecast_Tab2_ptr;
    nmfForecastTab3*        Forecast_Tab3_ptr;
    nmfForecastTab4*        Forecast_Tab4_ptr;
    nmfForecastTab5*        Forecast_Tab5_ptr;
    MSVPAVisualizationNode* outputWidget;

    boost::numeric::ublas::matrix<double> ChartData;
    std::vector<std::string> PreyList;
    nmfOutputChartBase *outputChart;
    //nmfOutputChartBase2 *outputChart2;
    std::map<std::string, QLabel*>      AllLabels;
    std::map<std::string, QComboBox*>   AllComboBoxes;
    std::map<std::string, QPushButton*> AllButtons;
    std::map<std::string, QCheckBox*>   AllCheckBoxes;
    std::map<std::string, std::map<std::string, std::map<int,double> > > MaxAgesPerSeasonMap;
    std::map<std::string, std::map<std::string, std::map<int,double> > > MaxSeasonsPerAgeMap;
    std::map<std::string, std::map<std::string, double > > MaxAllSeasonsAndAgesMap;
    boost::numeric::ublas::matrix<double> M2Matrix;
    QLabel* selectionModeLBL;
    QComboBox *selectionModeCMB;
    QLabel *themeLBL;
    QComboBox *themeCMB;
    Q3DBars *graph3D;
    std::vector<boost::numeric::ublas::matrix<double> > SSVPATables;
    QSplitter *splitterMSVPA;
    QSplitter *splitterMSVPA2;
    QComboBox *chartTypeCMB;
    std::set<std::string> DirtyTables; // std::set so don't have to worry about duplicate names
    QDialog *SaveDlg;
    QDialog *PauseDlg;
    QVBoxLayout *PauseVLayout;
    QHBoxLayout *PauseHLayout;
    QWidget *MSVPAProgressWidget;
    QWidget *ForecastProgressWidget;
    QVBoxLayout *progressMainLayt;
    QWidget *SaveWidget;
    QTextEdit *SetupOutputTE;
    QWidget* SSVPAOutputW;
    QSplitter* SSVPASplitter;
    QGroupBox* SSVPAOutputControlsGB;
    QTabWidget* SSVPAOutputTabWidget;
    QWidget* SSVPAOutputChartTab;
    QWidget* SSVPAOutputDataTab;
    QTextEdit* SSVPAOutputTE;
    QHBoxLayout* mainGUILayt;

    QLabel      *lbl;
    QPushButton *YesPB;
    QPushButton *NoPB;

    bool hGridLine2d;
    bool vGridLine2d;

    int timerId;
    static nmfMainWindow nmfMainWindowInstance;

    void loadDatabase();
    bool queryUserPreviousDatabase();
    void setDefaultDockWidgetsVisibility();

public:
    bool SSVPAWidgetsLoaded;
    bool MSVPAWidgetsLoaded;
    bool ForecastWidgetsLoaded;
    bool skipSingleClick;
    int SpeciesIndex;
    int FirstYear;
    int LastYear;
    int NumYears;
    int NumSeasons;
    int isGrowthModel;
    int FirstCatchYear;
    std::string SpeciesName;
    std::string MSVPAName;
    std::string DatabaseName;
    double maturitySliderScale;
    double mortalitySliderScale;
    double selectivitySliderScale;
    boost::numeric::ublas::matrix<double> Abundance;
    boost::numeric::ublas::matrix<double> M_NaturalMortality;
    boost::numeric::ublas::matrix<double> F_FishingMortality;
    std::vector<bool> MSVPAPageEnabled;

    explicit nmfMainWindow(QWidget *parent = 0);
    ~nmfMainWindow();

    /**
     * @brief Notifies main routine if MainWindow has been started correctly with MySQL active
     * @return true/false signifying the state of the application start
     */
    bool isStartUpOK();
    static void testFunc();
    void nmfMainWindowComplete();
//    nmfData* appData();
    void closeEvent(QCloseEvent *event);
    void setCatchAtAgePtr(QSqlQueryModel* catchAtAge_Ptr);
    void setWeightAtAgePtr(QSqlQueryModel* weightAtAge_Ptr);
    void setSizeAtAgePtr(QSqlQueryModel* sizeAtAge_Ptr);
    void setMaturityPtr(QSqlQueryModel* maturity_Ptr);
    QSqlQueryModel* catchAtAgePtr();
    QSqlQueryModel* weightAtAgePtr();
    QSqlQueryModel* sizeAtAgePtr();
    QSqlQueryModel* maturityPtr();
    //void setModelSlots();
    void updateTable(QTableView* view, QSqlQueryModel* model_ptr, std::string cmd);
//    void setAppData(nmfData* data);
    int currentEntity();
    std::string entityName();
    int ssvpaConfigurationIndex();
    std::string ssvpaConfigurationName();
    double getValueAsDouble(QLineEdit* widget);
    int getValueAsInt(QLineEdit* widget);
    void sendToOutputWindow(QTextEdit *textEdit, std::string name, int FirstCatchYear,
            int FirstCatchAge, int LastCatchAge,
            boost::numeric::ublas::matrix<double> &matrix,
            int numDigits, int numDecimals);
    void sendToOutputWindow(QTextEdit *textEdit, std::string name, double value,
            int numDigits, int numDecimals);
    void sendToOutputWindow(QTextEdit *textEdit, std::string name, int value);
    void sendToOutputWindow(QTextEdit *textEdit, std::string text);
//    void sendToOutputWindow(QTextEdit *textEdit,
//                            QStringList rowTitles,
//                            std::vector<std::string> colTitles,
//                            boost::numeric::ublas::matrix<double> &outMatrix,
//                            int numDigits,
//                            int numDecimals);
    static void sendToOutputTable(QTableWidget *tw,
             QStringList rowTitles,
             QStringList colTitles,
             boost::numeric::ublas::matrix<double> &outMatrix,
             int numDigits, int numDecimals);
    void keyPressEvent(QKeyEvent* event);
//    void sendToFile(std::string name,
//            boost::numeric::ublas::matrix<double> &outMatrix, std::string species);
    void sendToFile(std::string filename, QTableWidget *tw);
    void activateSetupWidgets();
    void activateSSVPAWidgets();
    void activateMSVPAWidgets();
    void activateForecastWidgets();
    void load(QListView* view, QSqlQueryModel* model_ptr, std::string query);
    void hideWidgets();
    void setupOutputWindows(QTextEdit *textEdit);
    void loadMsvpaCharts(std::string selectedSpecies);
    //void disableSSVPAMenuAndTabItems();
    void loadMSVPAInputWidgets();
    void updateMainWindowTitle(std::string modelType);
    void setNewDatabaseName(std::string newDatabaseName);
    std::string getDatabaseName();
    QFont getMonospaceFont();
    bool isFixedPitch(const QFont &font);
    void loadMSVPAChartWidgets();
    void loadForecastChartWidgets();
    void restoreTitleFonts();
    void enableApplicationFeatures(bool enable);
    void setInitialNavigatorState(bool initialState);

    QPalette getDarkPalette();
    void enableSSVPAWidgets(bool enable);
    void clearMSVPAInputWidgets();
    void setupOutputDir();
    void stopForecastProgressBarTimer();
    void startForecastProgressBarTimer();
    void startForecastProgressSetupTimer();
    void startMSVPAProgressSetupTimer();
    void startMSVPAProgressChartTimer();
    void reloadYearsLW(std::string MSVPAName);
    void reloadSpeciesAgeSizeComboBox(std::string MSVPAName);
    void reloadFullyRecruitedAgeComboBox(std::string MSVPAName);
    void createMaxValueMap();
    void getYearsAndAges(const std::string &MSVPAName, const std::string &species,
                         int &NumYears, int &NumAges);
    void getMaturityData(
            const int &Nage,
            //const int &NYears, const int &FirstYear, const int &LastYear,
            const std::string &SpeName,
            boost::numeric::ublas::matrix<double> &Maturity);
    void getSelectedListWidgetItems(QListWidget *selectYearsLW,
            std::vector<std::string> &selectedYears);
    bool queryAndLoadInitFields();
    bool inputIsValid(QLineEdit *lineedit, QString text, double &validatedInput);
    void resetScaleWidgets(QCheckBox *cb, QLineEdit *le);
    void clearForecastWidgets();
    std::string modelName();
    std::string forecastName();
    int forecastFirstYear();
    int forecastNYears();
    bool forecastIsGrowthModel();
    std::vector<std::pair<int,std::string> > OtherPredSpecies;
    std::string scenarioName();
    std::string updateCmd(const std::string &currentCmd,
                          const std::vector<std::string> &outputFields);
    void reloadPreySpecies(std::string species, int PredAgeVal);
    void draw3DCharts();
    void enableMSVPAWidgets(bool enable);
    void enable3DControls(bool enable);
    void loadForecastInputWidgets();
    void showVPAGroupBox(int whichGroupBox);
    QString getOutputFilename(QString type, QString filename);

    static void setTitles(
            QChart *chart,
            QStackedBarSeries *series,
            QStringList categories,
            std::string species,
            std::string mainTitle,
            std::string xLabel,
            std::string yLabel);

    static void loadChartWithData(
            QChart *chart,
            QStackedBarSeries *series,
            QStringList &PreyList,
            boost::numeric::ublas::matrix<double> &ChartData);

    static void MSVPA_DietComposition(
            nmfDatabase* m_databasePtr,
            QChart* chart,
            std::string MSVPAName,
            std::string unused1,
            std::string unused2,
            int unusedInt1,
            int unusedInt2,
            std::string selectedSpecies,
            int unusedInt3,
            std::string selectedSpeciesAgeSizeClass,
            QTableWidget *chartDataTbW);

    static void Forecast_DietComposition(
            nmfDatabase* m_databasePtr,
            QChart* chart,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int Forecast_FirstYear,
            int Forecast_NYears,
            std::string selectedSpecies,
            int PredNum,
            std::string selectedSpeciesAgeSizeClass,
            QTableWidget *chartDataTbW);
    //bool updateTable(std::string tableName);
    void updateChart(
            boost::numeric::ublas::matrix<double> Abundance,
            boost::numeric::ublas::matrix<double> M_NaturalMortality,
            boost::numeric::ublas::matrix<double> F_FishingMortality,
            int FirstCatchAge,
            int LastCatchAge,
            int FirstCatchYear,
            int LastCatchYear,
            int updateRange);
    void loadOtherPredSpecies();
    void showSSVPAOutput(
            std::string SpeciesName, std::string elapsedTimeStr,
            std::string Config, std::string VPAType,
            double CV, double SE, int NIt,
            int numDigits,
            boost::numeric::ublas::matrix<double> &Abundance,
            boost::numeric::ublas::matrix<double> &M,
            boost::numeric::ublas::matrix<double> &F,
            std::vector<std::string> &rowTitles,
            std::vector<std::string> &colTitles);
    bool validateArgs(MSVPASpeciesStruct args);
    void SaveTables();
    void deleteTheForecast(std::string forecast,
                           std::string scenario,
                           int numScenarios);
    void deleteTheForecast(std::string forecast);
    void deleteTheMSVPA();
    void initializeTableNamesDlg();
    void loadForecastListWidget();
    void loadScenarioListWidget(std::string scenarioToSelect);
    void enableNavigatorTopLevelItems(bool enable);
    void clearForecastTables(std::string MSVPAName, std::string Forecast);
    void reloadForecastWidgets(std::string currForecast);
    QTableView* findTableInFocus();
//    void initializePrefDlg();
    void clearNavigationTreeParent(int topLevelItem, int tabIndex);
    void ResetSpeciesList(bool withCallback);
    void clearDatabaseTables(std::string type,
                             const std::vector<std::string> &Tables);
    void disableTabs(QTabWidget *tabWidget);
    void disableNavigatorTreeItems(int level);
    void getCohortAnalysisGuiData(std::map<std::string,int> &CohortAnalysisGuiData);
    void getEffortTunedGuiData(std::map<std::string,int> &EffortTunedGuiData);
    void getXSAGuiData(std::map<std::string,std::string> &XSAGuiData);
    void clearSSVPAOutputWindow();
    void clearMSVPAOutputWindow();
    void setupMSVPAProgressChart();
    void setupForecastProgressChart();
    void setupLogWidget();
    void setupOutputWidget();
    bool isStoppedAndComplete(std::string &runName,
                              std::string &elapsedTime,
                              std::string &numIterationsOrYears,
                              std::string stopRunFile);
    void MSVPAFindStoreAndSetPageStates();
    void showDockWidgets(bool show);
    void initLogo();
    void showLogo(bool show);
    void initGuiPages();
    QString getCurrentStyle();
    void StopTheRun(std::string RunType);

signals:
    void LoadDatabase(QString database);

public slots:
//  void callback_ShowPauseDlg();
//  void callback_connectToDatabase();
    void callback_createTables();
//  void callback_resetGUI();
    void callback_chartThemeChanged(int newTheme);
    void callback_databaseSelected(QListWidgetItem *item);
    void callback_databaseSelectedAndClose(QListWidgetItem *item);
    void callback_enableNavigator();
    void callback_flipAgesDown();
    void callback_flipAgesUp();
    void callback_forecastSingleClicked(const QModelIndex &curr);
    void callback_forecastDoubleClicked(const QModelIndex &curr);
    void callback_MSVPASingleClicked(const QModelIndex &curr);
    void callback_MSVPASingleClickedGUI(const QModelIndex &curr);
//  void callback_MSVPADoubleClicked(const QModelIndex &index);
    void callback_msvpaForecastSingleClicked(const QModelIndex &curr);
    void callback_NavigatorSelectionChanged();
    void callback_okDatabase();
    void callback_ProjectSet();
    void callback_RunForecastClicked(bool checked);
    void callback_RunMSVPAClicked(bool checked);
//  void callback_RunSSVPAPBClicked(bool checked);
    void callback_scenarioSingleClicked(const QModelIndex &curr);
//  void callback_schemeDark();
    void callback_schemeLight();
    void callback_selectByVariablesChanged(QString byVariables);
    void callback_selectDataTypeChanged(QString dataType);
    void callback_selectFullyRecruitedAgeCMBChanged(QString age);
    void callback_selectFullyRecruitedAgeCheckboxChanged(int state);
    void callback_selectFullyRecruitedAgeLEChanged(QString text);
    void callback_selectPreySpeciesCMBChanged(QString preyName);
    void callback_selectSpeciesAgeSizeLEChanged(QString text);
    void callback_selectSeasonCBChanged(int state);
    void callback_selectSeasonCMBChanged(QString season);
    void callback_selectSeasonLEChanged(QString text);
    void callback_selectSpeciesAgeSizeCBChanged(int state);
    void callback_selectSpeciesAgeSizeCMBChanged(QString speciesAgeSizeClass);
    void callback_selectSpeciesChanged(QString species);
    void callback_selectVariableChanged(QString variable);
    void callback_selectYPRAnalysisTypeChanged(QString type);
    void callback_selectYearsChanged();
    void callback_SSVPAChartTypeChanged(QString chartType);
    void callback_SSVPASingleClicked(const QModelIndex &curr);
    void callback_SetStyleSheet(QString style);
    void callback_TableNamesOkPB();
    void callback_updateNavigatorSelection(int tabIndex);
    void callback_updateNavigatorSelection2(std::string type,int tabIndex);

    void menu_about();
    void menu_new();
    void menu_clear();
    void menu_clearAll();
//  void menu_clearSelection(QModelIndexList &indexes);
    void menu_copy();
    void menu_paste();
    void menu_pasteAll();
    void menu_selectAll();
    void menu_deselectAll();
    void menu_importDatabase();
    void menu_exportDatabase();
    void menu_exportAllDatabases();
    void menu_reloadCSVFiles();
//  void menu_newMSVPASpecies();
    void menu_newForecast();
    void menu_deleteSpecies();
    void menu_deleteMSVPA();
    void menu_deleteForecast();
    void menu_delete();
    void menu_openProject();
//  void menu_connectToDatabase();
    void menu_createTables();
    void menu_preferences();
    void menu_saveToDatabase();
    void menu_saveOutputChart();
    void menu_saveOutputData();
    void menu_quit();
    void menu_showTableNames();
    void menu_clearSSVPANonSpeciesTables();
    void menu_clearMSVPATables();
    void menu_clearForecastTables();
    void menu_clearAllNonSpeciesTables();
    void menu_clearAllTables();
    void menu_newMSVPA();

    void readMSVPAProgressChartDataFile();
    void readForecastProgressChartDataFile();
    void readMSVPAProgressSetupDataFile(std::string progressDataFile);
    void readForecastProgressSetupDataFile(std::string progressDataFile);
    //void readForecastProgressBarDataFile();
    void ReadSettings(QString name);
    void ReadSettings();
    void saveSettings();
    void update();
    void updateModel();
    void updateModel(int value);
    void updateModel(double value);
    void updateModel(
            const QModelIndex &topLeft,
            const QModelIndex &bottomRight);
    void callback_TableDataChanged(std::string tableName);
    void callback_RunSSVPA(std::string SSVPAType);
    void toolbar_whatsThis(bool triggered);
    void toolbar_screenGrab(bool triggered);
    void callback_CheckAllItemsSaveDlg(int state);
    void callback_UpdateScenarioList(std::string scenarioToSelect);
    void callback_LoadDataTable(std::string MSVPAName, std::string ForecastName,
                                std::string model, std::string table,
                                std::string section, int tabNum);
    void callback_SelectTab(std::string returnToSection, int returnToTab,
                            std::string MSVPAName, std::string ForecastName);
    void callback_ExportTableToCSVFile(std::string table,
                                       std::string csvFile);
    void callback_LoadDatabase(QString database);
    void callback_ReloadForecast(std::string tab);
//  void callback_deactivateRunButtons();
    void callback_closePrefDlg();
    void callback_MSVPALoadWidgets(int TabNum);
    void callback_WaitCursor();
    void callback_MSVPATab4(std::string tabAction);
    void callback_ResetSpeciesList();
    void callback_InitializePage(std::string model, int page, bool withCallback);
    void callback_SSVPAInputTabChanged(int tab);
    void ResetMSVPAConfigList(bool withCallback);
    void callback_ReselectSpecies(std::string Species, bool withCallback);
//  void callback_EnableRunSSVPAPB(bool toggle);
    void callback_horzGridLine2dCheckboxChanged(int state);
    void callback_vertGridLine2dCheckboxChanged(int state);
    void callback_stopMSVPAProgressSetupTimer();
    void callback_stopMSVPAProgressSetupTimerOnly();
    void callback_stopForecastProgressSetupTimer();
    void callback_stopForecastProgressSetupTimerOnly();
    void callback_readMSVPAProgressSetupDataFile();
    void callback_readForecastProgressSetupDataFile();
    void callback_SpeciesDoubleClick(QModelIndex m);
    void callback_SSVPASingleClickedDo(const QModelIndex &curr);
    void callback_CheckAndEnableMSVPAExecutePage();
    void callback_MarkAsClean();
    void callback_RunMSVPA();
    void callback_RunForecast();
    void callback_NavigatorItemExpanded(QTreeWidgetItem *item);
    void callback_UpdateMSVPAProgressDialog(int value, QString msg);
    void callback_UpdateMSVPAProgressWidget();
    void callback_UpdateForecastProgressDialog(int value, QString msg);
    void callback_UpdateForecastProgressWidget();
};



#endif /* NMF_MAIN_WINDOW_H */
