
/**
 *
 * @file nmfMainWindow.cpp
 * @date Nov 28, 2016
 * @author Ronald Klasky
 *
 * Public Domain Notice
 * National Oceanic And Atmospheric Administration
 *
 * This software is a "United States Government Work" under the terms of the
 * United States Copyright Act.  It was written as part of the author's official
 * duties as a United States Government employee/contractor and thus cannot be copyrighted.
 * This software is freely available to the public for use. The National Oceanic
 * And Atmospheric Administration and the U.S. Government have not placed any
 * restriction on its use or reproduction.  Although all reasonable efforts have
 * been taken to ensure the accuracy and reliability of the software and data,
 * the National Oceanic And Atmospheric Administration and the U.S. Government
 * do not and cannot warrant the performance or results that may be obtained
 * by using this software or data. The National Oceanic And Atmospheric
 * Administration and the U.S. Government disclaim all warranties, express
 * or implied, including warranties of performance, merchantability or fitness
 * for any particular purpose.
 *
 * Please cite the author(s) in any work or product based on this material.
 *
 * @brief This file contains the MSVPA_X2 Main Window implementation.
 */

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <thread>

#include <QtCore/QString>
#include <QtCore/QModelIndexList>
#include <QtCore/QModelIndex>
#include <QtCore/QSize>
#include <QtCore/QTimer>

#include <QtGui/QStandardItem>
#include <QtGui/QCloseEvent>
#include <QtGui/QPalette>
#include <QtGui/QColor>
#include <QtGui/QDoubleValidator>
#include <QtGui/QScreen>

#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QShortcut>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QDoubleSpinBox>

#include <QtDataVisualization/qcategory3daxis.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/qbardataproxy.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qbar3dseries.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtDataVisualization/q3dbars.h>

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtCharts/QValueAxis>

#include <string>
#include <iostream>
#include <fstream>

#include "nmfMainWindow.h"
#include "ui_nmfMainWindow.h"
#include "nmfData.h"

// Flip these two when going from Linux to Windows....just temporarily
#include "nmfModelDelegates.h"   // linux
//#include "nmfTableItemDelegate.h"  // windows

#include "nmfMortalityModel.h"
#include "nmfMaturityModel.h"
#include "nmfVonBertModel.h"
#include "nmfInitialSelectivityModel.h"
#include "nmfConstants.h"
#include "nmfUtils.h"
#include "nmfMSVPAProgressWidget.h"

#include "nmfOutputChart3DBarModifier.h"


nmfMainWindow::nmfMainWindow(QWidget *parent) :
        QMainWindow(parent),
        m_UI(new Ui::nmfMainWindow)
{
    bool loadLastProject = false;

    m_UI->setupUi(this);

    QString whatsThis;
    m_MSVPAProgressDlg    = nullptr;
    m_ForecastProgressDlg = nullptr;

    skipSingleClick = false;
    YMaxFullyRecruitedAge = 0.0;
    YMaxAgeSizeClass = 0.0;
    YMaxSeason = 0.0;
    //MSVPALoopsBeforeChecking = nmfConstants::MaxLoopsBeforeChecking;

    hGridLine2d = true;
    vGridLine2d = true;
    //hGridLine3d = true;
    //vGridLine3d = true;
    m_CatchDataInitialized  = false;
    SSVPAWidgetsLoaded    = false;
    MSVPAWidgetsLoaded    = false;
    ForecastWidgetsLoaded = false;
    DatabaseName.clear();
    DirtyTables.clear();
    SpeciesIndex = 0;
    SpeciesName.clear();
    Hostname.clear();
    m_Username.clear();
    m_Password.clear();
    Session.clear();
    m_isStartUpOK = true;
    m_TableNamesWidget = nullptr;
    m_TableNamesDlg    = new QDialog(this);

    m_ProjectDir.clear();
    ProjectDatabase.clear();
    ProjectName.clear();

    // Hide all dock widgets and show NOAA logo in ui->CentralWidget
    showDockWidgets(false);
    initLogo();

    // Set up vector to keep track of state of each MSVPA tab and MSVPA Navigator Tree sub-item
    for (int i=0; i<nmfConstantsMSVPA::NumberOfMSVPAPages; ++i) // ui->MSVPAInputTabWidget->count(); ++i)
        MSVPAPageEnabled.push_back(false);

    // Arrange Progress and Log dock widgets.
    QMainWindow::tabifyDockWidget(m_UI->ProgressDockWidget,m_UI->LogDockWidget);
    m_UI->ProgressDockWidget->raise();


    // Set up progress timers
//  progressDlgMSVPA = NULL;
    progressSetupTimerMSVPA = new QTimer(this);
    connect(progressSetupTimerMSVPA, SIGNAL(timeout()),
            this, SLOT(callback_readMSVPAProgressSetupDataFile()));
    progressChartTimerMSVPA = new QTimer(this);

    // Set up progress timers
//  progressDlgForecast = NULL;
    progressSetupTimerForecast = new QTimer(this);
    connect(progressSetupTimerForecast, SIGNAL(timeout()),
            this, SLOT(callback_readForecastProgressSetupDataFile()));
    progressChartTimerForecast = new QTimer(this);

    // Initialize the logger
    //nmfLogger::initLogger();
    m_logger = new nmfLogger();
    m_logger->initLogger("MSVPA_X2");

    // Setup Log Widget
    setupLogWidget();

    // Setup Output Widget
    setupOutputWidget();

    // Check for and make if necessary hidden dirs for program usage: logs, data
    nmfUtilsQt::checkForAndCreateDirectories(nmfConstantsMSVPA::HiddenDir,
                                             nmfConstantsMSVPA::HiddenDataDir,
                                             nmfConstantsMSVPA::HiddenLogDir);

    // Check for log files
    nmfUtilsQt::checkForAndDeleteLogFiles("MSVPA_X2",
             nmfConstantsMSVPA::HiddenLogDir,
             nmfConstantsMSVPA::LogFilter);

    setupOutputDir();

    outputChart = NULL;
    chartView3DContainer = NULL;
    chartView = NULL;
    m_modifier = NULL;
    graph3D = NULL;

    // Set up function map
    FunctionMap["MSVPA_Diet Composition"]            = MSVPA_DietComposition;
//    FunctionMap["MSVPA_Population Size"]             = MSVPA_PopulationSize;
//    FunctionMap["MSVPA_Mortality Rates"]             = MSVPA_MortalityRates;
//    FunctionMap["MSVPA_Yield Per Recruit"]           = MSVPA_YieldPerRecruit;
//    FunctionMap["MSVPA_Food Availability"]           = MSVPA_FoodAvailability;
//    FunctionMap["MSVPA_Multispecies Populations"]    = MSVPA_MultispeciesPopulations;
//    FunctionMap["MSVPA_Growth"]                      = MSVPA_Growth;

    FunctionMap["Forecast_Diet Composition"]         = Forecast_DietComposition;
//    FunctionMap["Forecast_Population Size"]          = Forecast_PopulationSize;
//    FunctionMap["Forecast_Mortality Rates"]          = Forecast_MortalityRates;
//    FunctionMap["Forecast_Yield Per Recruit"]        = Forecast_YieldPerRecruit;
//    FunctionMap["Forecast_Food Availability"]        = Forecast_FoodAvailability;
//    FunctionMap["Forecast_Multispecies Populations"] = Forecast_MultispeciesPopulations;
//    FunctionMap["Forecast_Growth"]                   = Forecast_Growth;

    // Setup APIs
//    msvpaApi    = new nmfMSVPA(logger);
//    forecastApi = new nmfForecast(databasePtr,logger);

    // Read the saved QSettings
    ReadSettings();

    EntityListLV     = new QListView(this);
    ForecastListLV   = new QListView(0);
    ScenarioListLV   = new QListView(0);

    whatsThis  = "<strong><center>Species List</center></strong>";
    whatsThis += "<p>This list shows the age-structured species for ";
    whatsThis += "this database. To interact with the list either: ";
    whatsThis += "<br><br>Click: Select Species and show relevant data ";
    whatsThis += "<br>Ctrl+Click: Select Species and Run SSVPA</p> ";
    EntityListLV->setWhatsThis(whatsThis);
    EntityListLV->setToolTip("Choose a Species with which to load the center tables.");
    EntityListLV->setStatusTip("Choose a Species with which to load the center tables.");

    ForecastListLBL  = new QLabel("Forecast List");
    ScenarioListLBL  = new QLabel("Scenario List");
    vLayout1         = new QVBoxLayout();
    entityDockWidget = new QWidget();
    vLayout1->addWidget(EntityListLV);
    vLayout1->addWidget(ForecastListLBL);
    vLayout1->addWidget(ForecastListLV);
    vLayout1->addWidget(ScenarioListLBL);
    vLayout1->addWidget(ScenarioListLV);
    entityDockWidget->setLayout(vLayout1);
    m_UI->EntityDockWidget->setWidget(entityDockWidget);

    initGuiPages();

    // Hook models to views
    EntityListLV->setModel(&entity_model);
    ForecastListLV->setModel(&forecast_model);
    ScenarioListLV->setModel(&scenario_model);

    QModelIndex index = NavigatorTree->model()->index(0,0);
    NavigatorTree->setCurrentIndex(index);

    // Create connections for input widgets
    connect(m_UI->actionNew,               SIGNAL(triggered()),            this,       SLOT(menu_new()));
    connect(m_UI->actionClear,             SIGNAL(triggered()),            this,       SLOT(menu_clear()));
    connect(m_UI->actionCopy,              SIGNAL(triggered()),            this,       SLOT(menu_copy()));
    connect(m_UI->actionPaste,             SIGNAL(triggered()),            this,       SLOT(menu_paste()));
    connect(m_UI->actionPasteAll,          SIGNAL(triggered()),            this,       SLOT(menu_pasteAll()));
    connect(m_UI->actionSelectAll,         SIGNAL(triggered()),            this,       SLOT(menu_selectAll()));
    connect(m_UI->actionDeselectAll,       SIGNAL(triggered()),            this,       SLOT(menu_deselectAll()));
    connect(m_UI->actionImportDatabase,    SIGNAL(triggered()),            this,       SLOT(menu_importDatabase()));
    connect(m_UI->actionExportDatabase,    SIGNAL(triggered()),            this,       SLOT(menu_exportDatabase()));
    connect(m_UI->actionExportAllDatabases,SIGNAL(triggered()),            this,       SLOT(menu_exportAllDatabases()));
    connect(m_UI->actionReloadCSVFiles,    SIGNAL(triggered()),            this,       SLOT(menu_reloadCSVFiles()));
    // connect(actionCreateForecast,       SIGNAL(triggered()),            this,       SLOT(menu_newForecast()));
    // connect(actionMSVPASpecies,         SIGNAL(triggered()),            this,       SLOT(menu_newMSVPASpecies()));
    // connect(actionBiomassPredator,      SIGNAL(triggered()),            this,       SLOT(menu_newBiomassPredator()));
    // connect(ui->actionDeleteSpecies,    SIGNAL(triggered()),            this,       SLOT(menu_deleteSpecies()));
    // connect(ui->actionDeleteMSVPA,      SIGNAL(triggered()),            this,       SLOT(menu_deleteMSVPA()));
    // connect(ui->actionDeleteForecast,   SIGNAL(triggered()),            this,       SLOT(menu_deleteForecast()));
    connect(m_UI->actionDelete,            SIGNAL(triggered()),            this,       SLOT(menu_delete()));
    connect(m_UI->actionOpenProject,       SIGNAL(triggered()),            this,       SLOT(menu_openProject()));
    // connect(ui->actionConnectToDatabase,SIGNAL(triggered()),            this,       SLOT(menu_connectToDatabase()));
    connect(m_UI->actionSaveToDatabase,    SIGNAL(triggered()),            this,       SLOT(menu_saveToDatabase()));
    connect(m_UI->actionSaveOutputChart,   SIGNAL(triggered()),            this,       SLOT(menu_saveOutputChart()));
    connect(m_UI->actionSaveOutputData,    SIGNAL(triggered()),            this,       SLOT(menu_saveOutputData()));
    connect(m_UI->actionPreferences,       SIGNAL(triggered()),            this,       SLOT(menu_preferences()));
    connect(m_UI->actionShowTableNames,             SIGNAL(triggered()),   this,       SLOT(menu_showTableNames()));
    connect(m_UI->actionClearSSVPANonSpeciesTables, SIGNAL(triggered()),   this,       SLOT(menu_clearSSVPANonSpeciesTables()));
    connect(m_UI->actionClearMSVPATables,           SIGNAL(triggered()),   this,       SLOT(menu_clearMSVPATables()));
    connect(m_UI->actionClearForecastTables,        SIGNAL(triggered()),   this,       SLOT(menu_clearForecastTables()));
    connect(m_UI->actionClearAllNonSpeciesTables,   SIGNAL(triggered()),   this,       SLOT(menu_clearAllNonSpeciesTables()));
    connect(m_UI->actionClearAllTables,             SIGNAL(triggered()),   this,       SLOT(menu_clearAllTables()));
    connect(m_UI->actionCreateTables,               SIGNAL(triggered()),   this,       SLOT(menu_createTables()));
    connect(m_UI->actionAbout,             SIGNAL(triggered()),            this,       SLOT(menu_about()));
    connect(m_UI->actionQuit,              SIGNAL(triggered()),            this,       SLOT(menu_quit()));
    connect(m_UI->actionWhatsThis,         SIGNAL(triggered(bool)),        this,       SLOT(toolbar_whatsThis(bool)));
    connect(m_UI->actionScrGrb,            SIGNAL(triggered(bool)),        this,       SLOT(toolbar_screenGrab(bool)));
    connect(m_UI->SetupInputTabWidget,     SIGNAL(tabBarClicked(int)),     this,       SLOT(callback_updateNavigatorSelection(int)));
    connect(m_UI->MSVPAInputTabWidget,     SIGNAL(currentChanged(int)),    this,       SLOT(callback_updateNavigatorSelection(int)));
    connect(m_UI->MSVPAInputTabWidget,     SIGNAL(currentChanged(int)),    this,       SLOT(callback_MSVPALoadWidgets(int)));
    connect(m_UI->ForecastInputTabWidget,  SIGNAL(tabBarClicked(int)),     this,       SLOT(callback_updateNavigatorSelection(int)));

    connect(NavigatorTree,  SIGNAL(itemSelectionChanged()),
            this,           SLOT(callback_NavigatorSelectionChanged()));
    connect(NavigatorTree,  SIGNAL(itemExpanded(QTreeWidgetItem *)),
            this,           SLOT(callback_NavigatorItemExpanded(QTreeWidgetItem *)));


    // RSK - put this in if need to support wheel action for tab changes.
    //connect(ui->MSVPAInputTabWidget,     SIGNAL(currentChanged(int)),     this,       SLOT(callback_MSVPATabChanged(int)));


    m_UI->ForecastInputTabWidget->findChild<QTableView *>("Forecast_Tab2_GrowthParametersTV")->setModel(&vonBert_model);

    // Setup the Output window
    setupOutputWindows(SSVPAOutputTE);

    //
    // Draw the MSVPA Chart Tab
    //
    chart = new QChart();
    chart->setObjectName("The Chart");
    chart->setAnimationOptions(QChart::NoAnimation);

    QWidget* MSVPAOutputChartTab = m_UI->OutputWidget->findChild<QWidget*>("Chart");
    hMainLayout  = new QHBoxLayout(MSVPAOutputChartTab);

    QVBoxLayout *vLayout               = new QVBoxLayout();
    QHBoxLayout *hFlipLayout           = new QHBoxLayout();
    QHBoxLayout *hSeasonLayout         = new QHBoxLayout();
    QHBoxLayout *hAgeSizeLayout        = new QHBoxLayout();
    QHBoxLayout *hFullyRecruitedLayout = new QHBoxLayout();
    QHBoxLayout *hGrid2dLayout         = new QHBoxLayout();
    QVBoxLayout *controlsLayout        = new QVBoxLayout();

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QWidget *controlsWidget1 = new QWidget();
    controlsWidget1->setLayout(vLayout);
    vLayout->setAlignment(Qt::AlignTop);
    hMainLayout->addWidget(chartView);
    controlsLayout->addWidget(controlsWidget1);

    selectDataTypeLBL = new QLabel(QStringLiteral("Select Data Type:"));
    selectDataTypeCMB = new QComboBox();
    selectDataTypeCMB->addItem(QStringLiteral("Diet Composition"),         0);
    selectDataTypeCMB->addItem(QStringLiteral("Population Size"),          1);
    selectDataTypeCMB->addItem(QStringLiteral("Mortality Rates"),          2);
    selectDataTypeCMB->addItem(QStringLiteral("Consumption Rates"),        3);
    selectDataTypeCMB->addItem(QStringLiteral("Yield Per Recruit"),        4);
    selectDataTypeCMB->addItem(QStringLiteral("Food Availability"),        5);
    selectDataTypeCMB->addItem(QStringLiteral("Multispecies Populations"), 6);
    selectDataTypeCMB->addItem(QStringLiteral("Growth"),                   7);

    selectSpeciesLBL             = new QLabel(QStringLiteral("Select Species:"));
    selectSpeciesCMB             = new QComboBox();
    selectVariableLBL            = new QLabel(QStringLiteral("Select Variable:"));
    selectVariableCMB            = new QComboBox();
    selectByVariablesLBL         = new QLabel(QStringLiteral("Select By Variables:"));
    selectByVariablesCMB         = new QComboBox();
    selectSeasonLBL              = new QLabel(QStringLiteral("Select Season:"));
    selectSeasonCMB              = new QComboBox();
    selectSeasonCB               = new QCheckBox();
    selectSeasonLE               = new QLineEdit();
    selectSpeciesAgeSizeClassLBL = new QLabel(QStringLiteral("Select Predator Age/Size Class:"));
    selectSpeciesAgeSizeClassCMB = new QComboBox();
    selectSpeciesAgeSizeClassCB  = new QCheckBox();
    selectSpeciesAgeSizeClassLE  = new QLineEdit();
    selectPreySpeciesLBL         = new QLabel(QStringLiteral("Select Prey:"));
    selectPreySpeciesCMB         = new QComboBox();

    // This group of widgets is only for Data Type == "Yield Per Recruit"
    selectYPRAnalysisTypeLBL     = new QLabel(QStringLiteral("Select YPR Analysis Type:"));
    selectYPRAnalysisTypeCMB     = new QComboBox();
    selectYearsLBL               = new QLabel(QStringLiteral("Select Year(s):"));
    selectYearsLW                = new QListWidget();
    selectFullyRecruitedAgeLBL   = new QLabel(QStringLiteral("Select Fully Recruited Age:"));
    selectFullyRecruitedAgeCMB   = new QComboBox();
    selectFullyRecruitedAgeCB    = new QCheckBox();
    selectFullyRecruitedAgeLE    = new QLineEdit();

    // Add grid line check boxes
    gridLine2dLBL    = new QLabel(QStringLiteral("Grid Lines"));
    horzGridLine2dCB = new QCheckBox("Horizontal");
    vertGridLine2dCB = new QCheckBox("Vertical");
    horzGridLine2dCB->setChecked(true);
    vertGridLine2dCB->setChecked(true);
    horzGridLine2dCB->setToolTip(  "Enable/disable horizontal chart grid lines");
    horzGridLine2dCB->setStatusTip("Enable/disable horizontal chart grid lines");
    vertGridLine2dCB->setToolTip(  "Enable/disable vertical chart grid lines");
    vertGridLine2dCB->setStatusTip("Enable/disable vertical chart grid lines");
    hGrid2dLayout->addWidget(horzGridLine2dCB);
    hGrid2dLayout->addWidget(vertGridLine2dCB);

    // Add theme combo box
    themeLBL = new QLabel(QStringLiteral("Color schemes:"));
    themeLBL->setToolTip("Change the color theme for the Stacked Bar Chart.");
    themeLBL->setStatusTip("Change the color theme for the Stacked Bar Chart.");
    themeCMB = new QComboBox();
    themeCMB->setObjectName("QT_CHARTS_USE_NAMESPACEthemeCMB");
    themeCMB->addItem("Light", QChart::ChartThemeLight);
    themeCMB->addItem("Blue Cerulean", QChart::ChartThemeBlueCerulean);
    themeCMB->addItem("Dark", QChart::ChartThemeDark);
    themeCMB->addItem("Brown Sand", QChart::ChartThemeBrownSand);
    themeCMB->addItem("Blue NCS", QChart::ChartThemeBlueNcs);
    themeCMB->addItem("High Contrast", QChart::ChartThemeHighContrast);
    themeCMB->addItem("Blue Icy", QChart::ChartThemeBlueIcy);
    themeCMB->addItem("Qt", QChart::ChartThemeQt);
    themeCMB->setToolTip("Change the color theme for the Stacked Bar Chart.");
    themeCMB->setStatusTip("Change the color theme for the Stacked Bar Chart.");

    flipRightPB = new QPushButton("->");
    flipLeftPB  = new QPushButton("<-");
    hFlipLayout->addWidget(flipLeftPB);
    hFlipLayout->addWidget(flipRightPB);
    flipRightPB->hide(); // Hide for now....may put them back in later.
    flipLeftPB->hide();

    selectionModeLBL = new QLabel(QStringLiteral("Selection Mode:"));
    selectionModeCMB = new QComboBox();
    selectionModeCMB->addItem(QStringLiteral("None"),
                              int(QAbstract3DGraph::SelectionNone));
    selectionModeCMB->addItem(QStringLiteral("Bar"),
                              int(QAbstract3DGraph::SelectionItem));
    selectionModeCMB->addItem(QStringLiteral("Row"),
                              int(QAbstract3DGraph::SelectionRow));
    selectionModeCMB->addItem(QStringLiteral("Bar and Row"),
                              int(QAbstract3DGraph::SelectionItemAndRow));
    selectionModeCMB->addItem(QStringLiteral("Column"),
                              int(QAbstract3DGraph::SelectionColumn));
    selectionModeCMB->addItem(QStringLiteral("Bar and Column"),
                              int(QAbstract3DGraph::SelectionItemAndColumn));
    selectionModeCMB->addItem(QStringLiteral("Row and Column"),
                              int(QAbstract3DGraph::SelectionRowAndColumn));
    selectionModeCMB->addItem(QStringLiteral("Bar, Row and Column"),
                              int(QAbstract3DGraph::SelectionItemRowAndColumn));
    selectionModeCMB->addItem(QStringLiteral("Slice into Row"),
                              int(QAbstract3DGraph::SelectionSlice | QAbstract3DGraph::SelectionRow));
    selectionModeCMB->addItem(QStringLiteral("Slice into Row and Item"),
                              int(QAbstract3DGraph::SelectionSlice | QAbstract3DGraph::SelectionItemAndRow));
    selectionModeCMB->addItem(QStringLiteral("Slice into Column"),
                              int(QAbstract3DGraph::SelectionSlice | QAbstract3DGraph::SelectionColumn));
    selectionModeCMB->addItem(QStringLiteral("Slice into Column and Item"),
                              int(QAbstract3DGraph::SelectionSlice | QAbstract3DGraph::SelectionItemAndColumn));
    selectionModeCMB->addItem(QStringLiteral("Multi: Bar, Row, Col"),
                              int(QAbstract3DGraph::SelectionItemRowAndColumn
                                  | QAbstract3DGraph::SelectionMultiSeries));
    selectionModeCMB->addItem(QStringLiteral("Multi, Slice: Row, Item"),
                              int(QAbstract3DGraph::SelectionSlice | QAbstract3DGraph::SelectionItemAndRow
                                  | QAbstract3DGraph::SelectionMultiSeries));
    selectionModeCMB->addItem(QStringLiteral("Multi, Slice: Col, Item"),
                              int(QAbstract3DGraph::SelectionSlice | QAbstract3DGraph::SelectionItemAndColumn
                                  | QAbstract3DGraph::SelectionMultiSeries));
    selectionModeCMB->setCurrentIndex(1);

    connect(themeCMB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(callback_chartThemeChanged(int)));
    connect(selectDataTypeCMB, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(callback_selectDataTypeChanged(QString)));
    connect(selectSpeciesCMB, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(callback_selectSpeciesChanged(QString)));
    connect(selectVariableCMB, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(callback_selectVariableChanged(QString)));
    connect(selectByVariablesCMB, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(callback_selectByVariablesChanged(QString)));
    connect(selectSeasonCMB, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(callback_selectSeasonCMBChanged(QString)));
    connect(selectSpeciesAgeSizeClassCMB, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(callback_selectSpeciesAgeSizeCMBChanged(QString)));
    connect(selectYPRAnalysisTypeCMB, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(callback_selectYPRAnalysisTypeChanged(QString)));
    connect(selectYearsLW, SIGNAL(itemSelectionChanged()),
            this, SLOT(callback_selectYearsChanged()));
    connect(selectFullyRecruitedAgeCMB, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(callback_selectFullyRecruitedAgeCMBChanged(QString)));
    connect(selectPreySpeciesCMB, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(callback_selectPreySpeciesCMBChanged(QString)));
    connect(selectSeasonCB, SIGNAL(stateChanged(int)),
            this, SLOT(callback_selectSeasonCBChanged(int)));
    connect(selectSpeciesAgeSizeClassCB, SIGNAL(stateChanged(int)),
            this, SLOT(callback_selectSpeciesAgeSizeCBChanged(int)));
    connect(selectFullyRecruitedAgeCB, SIGNAL(stateChanged(int)),
            this, SLOT(callback_selectFullyRecruitedAgeCheckboxChanged(int)));
    connect(selectFullyRecruitedAgeLE, SIGNAL(textChanged(QString)),
            this, SLOT(callback_selectFullyRecruitedAgeLEChanged(QString)));
    connect(selectSpeciesAgeSizeClassLE, SIGNAL(textChanged(QString)),
            this, SLOT(callback_selectSpeciesAgeSizeLEChanged(QString)));
    connect(selectSeasonLE, SIGNAL(textChanged(QString)),
            this, SLOT(callback_selectSeasonLEChanged(QString)));
    connect(horzGridLine2dCB, SIGNAL(stateChanged(int)),
            this, SLOT(callback_horzGridLine2dCheckboxChanged(int)));
    connect(vertGridLine2dCB, SIGNAL(stateChanged(int)),
            this, SLOT(callback_vertGridLine2dCheckboxChanged(int)));

    //QObject::connect(selectionModeCMB, SIGNAL(currentIndexChanged(int)), m_modifier,
    //                 SLOT(changeSelectionMode(int)));

    connect(flipRightPB, SIGNAL(clicked()),
            this,        SLOT(callback_flipAgesUp()));
    connect(flipLeftPB,  SIGNAL(clicked()),
            this,        SLOT(callback_flipAgesDown()));

    vLayout->addWidget(selectDataTypeLBL);
    vLayout->addWidget(selectDataTypeCMB);
    vLayout->addSpacerItem(new QSpacerItem(1, 20));
    vLayout->addWidget(selectSpeciesLBL);
    vLayout->addWidget(selectSpeciesCMB);
    vLayout->addWidget(selectVariableLBL);
    vLayout->addWidget(selectVariableCMB);
    vLayout->addWidget(selectByVariablesLBL);
    vLayout->addWidget(selectByVariablesCMB);
    vLayout->addWidget(selectSeasonLBL);
    hSeasonLayout->addWidget(selectSeasonCMB);
    hSeasonLayout->addWidget(selectSeasonCB);
    hSeasonLayout->addWidget(selectSeasonLE);
    vLayout->addLayout(hSeasonLayout);
    vLayout->addWidget(selectSpeciesAgeSizeClassLBL);
    hAgeSizeLayout->addWidget(selectSpeciesAgeSizeClassCMB);
    hAgeSizeLayout->addWidget(selectSpeciesAgeSizeClassCB);
    hAgeSizeLayout->addWidget(selectSpeciesAgeSizeClassLE);
    vLayout->addLayout(hAgeSizeLayout);
    vLayout->addLayout(hFlipLayout);
    vLayout->addWidget(selectPreySpeciesLBL);
    vLayout->addWidget(selectPreySpeciesCMB);
    vLayout->addWidget(selectYPRAnalysisTypeLBL);
    vLayout->addWidget(selectYPRAnalysisTypeCMB);
    vLayout->addWidget(selectYearsLBL);
    vLayout->addWidget(selectYearsLW);
    vLayout->addWidget(selectFullyRecruitedAgeLBL);

    selectPreySpeciesLBL->setEnabled(false);
    selectPreySpeciesCMB->setEnabled(false);

    hFullyRecruitedLayout->addWidget(selectFullyRecruitedAgeCMB);
    hFullyRecruitedLayout->addWidget(selectFullyRecruitedAgeCB);
    hFullyRecruitedLayout->addWidget(selectFullyRecruitedAgeLE);
    vLayout->addLayout(hFullyRecruitedLayout);

    // Add selection mode widgets used just for 3d charts
    vLayout->addWidget(selectionModeLBL);
    vLayout->addWidget(selectionModeCMB);
    selectionModeLBL->setEnabled(false);
    selectionModeCMB->setEnabled(false);

    vLayout->addWidget(gridLine2dLBL);
    vLayout->addLayout(hGrid2dLayout);
    vLayout->addWidget(themeLBL);
    vLayout->addWidget(themeCMB);

    selectYPRAnalysisTypeLBL->hide();
    selectYPRAnalysisTypeCMB->hide();
    selectYearsLBL->hide();
    selectYearsLW->hide();
    selectFullyRecruitedAgeLBL->hide();
    selectFullyRecruitedAgeCMB->hide();
    selectFullyRecruitedAgeCB->hide();
    selectFullyRecruitedAgeLE->hide();

    selectSeasonCMB->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    selectSeasonCB->setToolTip("Set vertical scale to same maximum value for all seasons.");
    selectSeasonCB->setStatusTip("Set vertical scale to same maximum value for all seasons.");
    selectSeasonLE->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    selectSeasonLE->setFixedWidth(50);
    selectSeasonLE->setToolTip("The maximum scale value for all seasons for the current age/size class.");
    selectSeasonLE->setStatusTip("The maximum scale value for all seasons for the current age/size class.");

    selectSpeciesAgeSizeClassCMB->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    selectSpeciesAgeSizeClassCB->setToolTip("Set vertical scale to same maximum value for all Age/Size classes.");
    selectSpeciesAgeSizeClassCB->setStatusTip("Set vertical scale to same maximum value for all Age/Size classes.");
    //selectSpeciesAgeSizeClassCB->setChecked(true);
    selectSpeciesAgeSizeClassLE->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    selectSpeciesAgeSizeClassLE->setFixedWidth(50);
    selectSpeciesAgeSizeClassLE->setToolTip("The maximum scale value for this species' age/size class.");
    selectSpeciesAgeSizeClassLE->setStatusTip("The maximum scale value for this species' age/size class.");
    selectYearsLW->setSelectionMode(QAbstractItemView::ExtendedSelection);
    selectFullyRecruitedAgeCMB->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    selectFullyRecruitedAgeLE->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    selectFullyRecruitedAgeCB->setToolTip("Set vertical scale to same maximum value for all Fully Recruited Age classes.");
    selectFullyRecruitedAgeCB->setStatusTip("Set vertical scale to same maximum value for all Fully Recruited Age classes.");
    selectFullyRecruitedAgeLE->setToolTip("The maximum scale value for all Fully Recruited Age classes.");
    selectFullyRecruitedAgeLE->setStatusTip("The maximum scale value for all Fully Recruited Age classes.");
    selectFullyRecruitedAgeLE->setFixedWidth(50);
    //selectFullyRecruitedAgeLE->setAlignment(Qt::AlignRight);
    selectFullyRecruitedAgeLE->setEnabled(false);

    //
    // Setup the SSVPA Chart Tab
    //
    // Draw 3d surface
    Q3DSurface *graph = new Q3DSurface();
    QWidget *container = QWidget::createWindowContainer(graph);
    if (!graph->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
        return;
    }
    //graph->setLocale(QLocale::English); // inserts "," for every 3 digits


    QWidget       *controlsWidget      = new QWidget();
    QHBoxLayout   *hLayout             = new QHBoxLayout(SSVPAOutputChartTab);
    QVBoxLayout   *vLayout2            = new QVBoxLayout();
    QVBoxLayout   *ssvpaControlsLayout = new QVBoxLayout();
    QHBoxLayout   *buttonLayout        = new QHBoxLayout();
    QHBoxLayout   *presetLayout        = new QHBoxLayout();
    QHBoxLayout   *windowButtonLayout  = new QHBoxLayout();
    QHBoxLayout   *maxScaleLayout1     = new QHBoxLayout();
    QHBoxLayout   *maxScaleLayout2     = new QHBoxLayout();
    QHBoxLayout   *themeLayout         = new QHBoxLayout();
    nmfOutputChart3D *outputChart3D    = new nmfOutputChart3D(graph,m_logger,m_ProjectDir);

    hLayout->addWidget(container);

    ssvpaControlsLayout->addWidget(controlsWidget);
    SSVPAOutputControlsGB->setLayout(ssvpaControlsLayout);

    vLayout2->setAlignment(Qt::AlignTop);

    outputCharts3D.push_back(outputChart3D);

    QLabel    *chartTypeLBL = new QLabel("Select 3D Chart Type:");
    chartTypeCMB = new QComboBox();
    chartTypeCMB->addItem("Abundance");
    chartTypeCMB->addItem("Fishing Mortality");
    chartTypeCMB->addItem("Natural Mortality");

    // Set size of controls by putting their layout into a qwidget and resizing it.
    controlsWidget->setLayout(vLayout2);
    controlsWidget->setMaximumWidth(235);

    // Selection Mode
    QGroupBox    *selectionGroupBox = new QGroupBox(QStringLiteral("Selection Mode"));
    QRadioButton *modeNoneRB        = new QRadioButton();
    QRadioButton *modeItemRB        = new QRadioButton();
    QRadioButton *modeSliceRowRB    = new QRadioButton();
    QRadioButton *modeSliceColumnRB = new QRadioButton();
    QVBoxLayout  *selectionVBox     = new QVBoxLayout;
    modeNoneRB->setText(QStringLiteral("No selection"));
    modeNoneRB->setChecked(false);
    modeItemRB->setObjectName("modeItemRB");
    modeItemRB->setText(QStringLiteral("Item"));
    modeItemRB->setChecked(true);
    modeSliceRowRB->setText(QStringLiteral("Years for Selected Age"));
    modeSliceRowRB->setChecked(false);
    modeSliceColumnRB->setText(QStringLiteral("Ages for Selected Year"));
    modeSliceColumnRB->setChecked(false);

    selectionVBox->addWidget(modeNoneRB);
    selectionVBox->addWidget(modeItemRB);
    selectionVBox->addWidget(modeSliceRowRB);
    selectionVBox->addWidget(modeSliceColumnRB);
    selectionGroupBox->setLayout(selectionVBox);

    // Setup Year slider control widgets
    QSlider *axisMinSliderX = new QSlider(Qt::Horizontal);
    axisMinSliderX->setMinimum(0);
    axisMinSliderX->setTickInterval(1);
    axisMinSliderX->setEnabled(true);
    QSlider *axisMaxSliderX = new QSlider(Qt::Horizontal);
    axisMaxSliderX->setMinimum(1);
    axisMaxSliderX->setTickInterval(1);
    axisMaxSliderX->setEnabled(true);
    axisMaxSliderX->setValue(nmfConstants::MaxSliderValue);
    //QLineEdit *sliderYMinValueLE = new QLineEdit();
    sliderYMaxValueLE = new QLineEdit();
    //sliderYMaxValueLE->setReadOnly(true);
    QPalette palette;
    //palette.setColor(QPalette::Base, QColor(237, 237, 237)); //Qt::lightGray);
    //sliderYMaxValueLE->setPalette(palette);
    sliderYMaxValueLE->setAlignment(Qt::AlignRight);
    sliderYMaxValueLE->setToolTip(
                "The maximum scale value on the y-axis, not the largest y data value.");
    sliderYMaxValueLE->setStatusTip(
                "The maximum scale value on the y-axis, not the largest y data value.");
    //QSlider *axisMinSliderYScale = new QSlider(Qt::Horizontal);
    //axisMinSliderYScale->setMinimum(0);
    //axisMinSliderYScale->setTickInterval(1);
    //axisMinSliderYScale->setEnabled(true);
    QSlider *axisMaxSliderYScale = new QSlider(Qt::Horizontal);
    axisMaxSliderYScale->setMinimum(1);
    axisMaxSliderYScale->setTickInterval(1);
    axisMaxSliderYScale->setEnabled(true);
    axisMaxSliderYScale->setValue(nmfConstants::MaxSliderValue);
    //minScaleLayout->addWidget(sliderYMinValueLE);
    //minScaleLayout->addWidget(axisMinSliderYScale);
    maxScaleLayout2->addWidget(axisMaxSliderYScale);
    maxScaleLayout2->addWidget(sliderYMaxValueLE);
    //sliderYMinValueLE->setFixedWidth(100);
    sliderYMaxValueLE->setFixedWidth(100);
    //sliderYMinValueLE->setObjectName("sliderYMinValueLE");
    sliderYMaxValueLE->setObjectName("sliderYMaxValueLE");

    QLabel     *scaleLBL = new QLabel(QStringLiteral("Refine Abundance Max Scale Value"));
    QCheckBox  *scaleCB  = new QCheckBox("auto");
    scaleCB->setChecked(true);
    scaleCB->setToolTip("Automatically re-scale the chart's y-axis as user changes inputs.");
    scaleCB->setStatusTip("Automatically re-scale the chart's y-axis as user changes inputs.");

    //QCheckBox *lockSliderX = new QCheckBox("Sync Year Range Sliders");
    //lockSliderX->hide();

    // Setup Age slider control widgets
    QSlider *axisMinSliderZ = new QSlider(Qt::Horizontal);
    axisMinSliderZ->setMinimum(0);
    axisMinSliderZ->setTickInterval(1);
    axisMinSliderZ->setEnabled(true);
    QSlider *axisMaxSliderZ = new QSlider(Qt::Horizontal);
    axisMaxSliderZ->setMinimum(1);
    axisMaxSliderZ->setTickInterval(1);
    axisMaxSliderZ->setEnabled(true);
    axisMaxSliderZ->setValue(nmfConstants::MaxSliderValue);

    // Setup rotation preset comboboxes
    QLabel *hRotLB = new QLabel(QStringLiteral("Horz:"));
    hRotLB->setToolTip("Rotate the graph horizontally by a preset amount.");
    hRotLB->setStatusTip(
                "Rotate the graph horizontally by a preset amount.");
    QComboBox *hRotateCMB = new QComboBox();
    QStringList hRotateValues = { "H Rot", "0", "30", "45", "90", "135",
                                  "180", "225", "270", "315", "330" };
    hRotateCMB->setObjectName("hRotateCMB");
    hRotateCMB->addItems(hRotateValues);
    hRotateCMB->setToolTip(
                "Rotate the graph horizontally by a preset amount.");
    hRotateCMB->setStatusTip(
                "Rotate the graph horizontally by a preset amount.");

    QLabel *vRotLB = new QLabel(QStringLiteral("Vert:"));
    vRotLB->setToolTip("Rotate the graph vertically by a preset amount.");
    vRotLB->setStatusTip("Rotate the graph vertically by a preset amount.");
    QComboBox *vRotateCMB = new QComboBox();
    QStringList vRotateValues =
    { "V Rot", "0", "15", "30", "45", "60", "90", };
    vRotateCMB->setObjectName("vRotateCMB");
    vRotateCMB->addItems(vRotateValues);
    vRotateCMB->setToolTip(
                "Rotate the graph vertically by a preset amount.");
    vRotateCMB->setStatusTip(
                "Rotate the graph vertically by a preset amount.");
    //presetLayout->addSpacerItem(new QSpacerItem(50, 1));
    presetLayout->addWidget(hRotLB);
    presetLayout->addWidget(hRotateCMB);
    //presetLayout->addSpacerItem(new QSpacerItem(50, 1));
    presetLayout->addWidget(vRotLB);
    presetLayout->addWidget(vRotateCMB);
    //presetLayout->addSpacerItem(new QSpacerItem(50, 1));

    // Setup control buttons
    QPushButton *orbitLeft = new QPushButton("\u21BB");
    QPushButton *orbitPaus = new QPushButton("||");
    QPushButton *orbitRght = new QPushButton("\u21BA");
    orbitLeft->setToolTip("Rotate the data in a clockwise fashion.");
    orbitLeft->setStatusTip("Rotate the data in a clockwise fashion.");
    orbitPaus->setToolTip("Stop the data rotation.");
    orbitPaus->setStatusTip("Stop the data rotation.");
    orbitRght->setToolTip(
                "Rotate the data in a counter-clockwise fashion.");
    orbitRght->setStatusTip(
                "Rotate the data in a counter-clockwise fashion.");
    QFont font = orbitRght->font();
    font.setPointSize(14);
    orbitLeft->setFont(font);
    orbitRght->setFont(font);
    orbitLeft->setFixedHeight(27);
    orbitPaus->setFixedHeight(27);
    orbitRght->setFixedHeight(27);
    font.setBold(true);
    font.setPointSize(10);
    orbitPaus->setFont(font);
    buttonLayout->addWidget(orbitLeft);
    buttonLayout->addWidget(orbitPaus);
    buttonLayout->addWidget(orbitRght);
    QPushButton *snapshot = new QPushButton("Snapshot");
    QPushButton *reset = new QPushButton("Reset");
    snapshot->setToolTip(
                "Take snapshot image of application and save to current directory.");
    snapshot->setStatusTip(
                "Take snapshot image of application and save to current directory.");
    reset->setToolTip("Reset chart to default position.");
    reset->setStatusTip("Reset chart to default position.");
    windowButtonLayout->addWidget(snapshot);
    windowButtonLayout->addWidget(reset);

//    // Add grid line check boxes
//    gridLine3dLBL    = new QLabel(QStringLiteral("Grid Lines"));
//    horzGridLine3dCB = new QCheckBox("Horizontal");
//    vertGridLine3dCB = new QCheckBox("Vertical");
//    horzGridLine3dCB->setChecked(true);
//    vertGridLine3dCB->setChecked(true);
//    horzGridLine3dCB->setToolTip(  "Enable/disable horizontal 3d chart grid lines");
//    horzGridLine3dCB->setStatusTip("Enable/disable horizontal 3d chart grid lines");
//    vertGridLine3dCB->setToolTip(  "Enable/disable vertical 3d chart grid lines");
//    vertGridLine3dCB->setStatusTip("Enable/disable vertical 3d chart grid lines");
//    grid3dLayout->addWidget(horzGridLine3dCB);
//    grid3dLayout->addWidget(vertGridLine3dCB);


    // Add theme combo box
    themeLBL = new QLabel(QStringLiteral("Color Schemes:"));
    themeLBL->setToolTip(  "Change the color theme for the 3D surface charts.");
    themeLBL->setStatusTip("Change the color theme for the 3D surface charts.");
    themeCMB = new QComboBox();
    themeCMB->setObjectName("themeCMB");
    for (auto theme : nmfConstants::SurfaceThemes) {
        themeCMB->addItem(QString::fromStdString(theme));
    }
    themeCMB->setToolTip(
                "Change the color theme for the 3D surface charts.");
    themeCMB->setStatusTip(
                "Change the color theme for the 3D surface charts.");

    // Add Control widgets to the side vertical layout
    vLayout2->addWidget(chartTypeLBL);
    vLayout2->addWidget(chartTypeCMB);
    vLayout2->addWidget(selectionGroupBox);
    vLayout2->addWidget(new QLabel(QStringLiteral("Trim Year Range")));
    vLayout2->addWidget(axisMinSliderX);
    vLayout2->addWidget(axisMaxSliderX);
    maxScaleLayout1->addWidget(scaleLBL);
    maxScaleLayout1->addItem(
                new QSpacerItem(10, 0, QSizePolicy::Expanding,
                                QSizePolicy::Fixed));
    maxScaleLayout1->addWidget(scaleCB);
    vLayout2->addLayout(maxScaleLayout1);
    vLayout2->addLayout(maxScaleLayout2);
    //vLayout2->addWidget(lockSliderX);
    vLayout2->addWidget(new QLabel(QStringLiteral("Trim Age Range")));
    vLayout2->addWidget(axisMinSliderZ);
    vLayout2->addWidget(axisMaxSliderZ);
    vLayout2->addWidget(
                new QLabel(QStringLiteral("Rotation Presets (deg)")));
    vLayout2->addLayout(presetLayout);
    vLayout2->addWidget(new QLabel(QStringLiteral("Turntable Controls")));
    vLayout2->addLayout(buttonLayout);
    vLayout2->addWidget(new QLabel(QStringLiteral("Window Controls")));
    vLayout2->addLayout(windowButtonLayout);
    vLayout2->addLayout(themeLayout);

//    vLayout2->addWidget(gridLine3dLBL);
//    vLayout2->addLayout(grid3dLayout);

    vLayout2->addWidget(themeLBL);
    vLayout2->addWidget(themeCMB);
    vLayout2->addItem(new QSpacerItem(0, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));

    QObject::connect(chartTypeCMB, SIGNAL(currentIndexChanged(QString)),
                     this, SLOT(callback_SSVPAChartTypeChanged(QString)));

    // Add the slots for the control widgets
    connect(axisMinSliderX, &QSlider::valueChanged,
            outputChart3D,  &nmfOutputChart3D::adjustXMin);
    connect(axisMaxSliderX, &QSlider::valueChanged,
            outputChart3D,  &nmfOutputChart3D::adjustXMax);
    connect(axisMaxSliderYScale, &QSlider::valueChanged,
            outputChart3D,       &nmfOutputChart3D::adjustYMax);
    connect(axisMinSliderZ, &QSlider::valueChanged,
            outputChart3D,  &nmfOutputChart3D::adjustZMin);
    connect(axisMaxSliderZ, &QSlider::valueChanged,
            outputChart3D,  &nmfOutputChart3D::adjustZMax);
    connect(modeNoneRB,     &QRadioButton::toggled,
            outputChart3D,  &nmfOutputChart3D::toggleModeNone);
    connect(modeItemRB,     &QRadioButton::toggled,
            outputChart3D,  &nmfOutputChart3D::toggleModeItem);
    connect(modeSliceRowRB, &QRadioButton::toggled,
            outputChart3D,  &nmfOutputChart3D::toggleModeSliceRow);
    connect(modeSliceColumnRB, &QRadioButton::toggled,
            outputChart3D,     &nmfOutputChart3D::toggleModeSliceColumn);
    connect(orbitLeft,      &QPushButton::clicked,
            outputChart3D,  &nmfOutputChart3D::orbitLeftClicked);
    connect(orbitPaus,      &QPushButton::clicked,
            outputChart3D,  &nmfOutputChart3D::orbitPauseClicked);
    connect(orbitRght,      &QPushButton::clicked,
            outputChart3D,  &nmfOutputChart3D::orbitRightClicked);
    connect(snapshot,       &QPushButton::clicked,
            outputChart3D,  &nmfOutputChart3D::snapshotClicked);
    connect(reset,          &QPushButton::clicked,
            outputChart3D,  &nmfOutputChart3D::resetClicked);
    connect(hRotateCMB,     SIGNAL(currentIndexChanged(QString)),
            outputChart3D,  SLOT(hRotateClicked(QString)));
    connect(vRotateCMB,     SIGNAL(currentIndexChanged(QString)),
            outputChart3D,  SLOT(vRotateClicked(QString)));
    connect(scaleCB,        SIGNAL(stateChanged(int)),
            this,           SLOT(updateModel(int)));
    connect(themeCMB,       SIGNAL(currentIndexChanged(int)),
            outputChart3D,  SLOT(themeChanged(int)));

    //QObject::connect(axisMinSliderYScale, &QSlider::valueChanged,
    //		matrixPlot, &nmfMatrixPlot::adjustYMin);
    //QObject::connect(lockSliderX, &QCheckBox::stateChanged,
    //		matrixPlot, &nmfMatrixPlot::lockXSliders);

    outputChart3D->setAxisMinSliderX(axisMinSliderX);
    outputChart3D->setAxisMaxSliderX(axisMaxSliderX);
    outputChart3D->setAxisMaxSliderYScale(axisMaxSliderYScale);
    outputChart3D->setAxisMinSliderZ(axisMinSliderZ);
    outputChart3D->setAxisMaxSliderZ(axisMaxSliderZ);
    outputChart3D->setMaxValueLE(sliderYMaxValueLE);
    outputChart3D->setScaleCB(scaleCB);

    // Populate Forecast widgets - do it here....just need to do it once.
    QComboBox *Forecast_Tab3_RecruitCurveCMB;
    Forecast_Tab3_RecruitCurveCMB = m_UI->ForecastInputTabWidget->findChild<QComboBox *>("Forecast_Tab3_RecruitCurveCMB");
    Forecast_Tab3_RecruitCurveCMB->clear();
    Forecast_Tab3_RecruitCurveCMB->addItem("Ricker");
    Forecast_Tab3_RecruitCurveCMB->addItem("Beverton Holt");
    Forecast_Tab3_RecruitCurveCMB->addItem("Random from Quartiles");
    Forecast_Tab3_RecruitCurveCMB->addItem("Shepherd Flexible");

    //setModelSlots();
    hideWidgets();

    // On Windows, the following Sql code must be done in main .exe file or else
    // the program can't find the libmysql.dll driver.  Not sure why, but moving
    // the following logic from nmfDatabase.dll to here fixes the issue.
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    m_databasePtr = new nmfDatabase();
    m_databasePtr->nmfSetConnectionByName(db.connectionName());

    // Hide Progress Chart and Log widgets. Show them once user does their first MSVPA run.
    setDefaultDockWidgetsVisibility();

    // Prompt user for database login and password
    if (nmfDatabaseUtils::menu_connectToDatabase(
                this,nmfConstantsMSVPA::SettingsDirWindows,m_databasePtr,
                m_Username,m_Password))
    {
        loadLastProject = queryUserPreviousDatabase();
    } else {
        m_isStartUpOK = false;
        return;
    }
    if (loadLastProject) {
        loadDatabase();
    } else {
        Setup_Tab2_ptr->clearProjectData();
        enableApplicationFeatures(false);
    }
    initializeTableNamesDlg();

    setInitialNavigatorState(false);


} // end nmfMainWindow constructor

bool
nmfMainWindow::isStartUpOK()
{
    return m_isStartUpOK;
}

void
nmfMainWindow::enableApplicationFeatures(bool enable)
{
    QTreeWidgetItem* item;

    // Enable last tab in Setup group
    item = NavigatorTree->topLevelItem(0);
    for (int i=2; i<item->childCount(); ++i) {
        if (enable) {
            for (int j=2; j<item->childCount(); ++j) {
                item->child(j)->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
                Setup_Tab2_ptr->enableSetupTabs(true);
            }
        } else {
            for (int j=2; j<item->childCount(); ++j) {
                item->child(j)->setFlags(Qt::NoItemFlags);
                Setup_Tab2_ptr->enableSetupTabs(false);
            }
        }
    }

    for (int i=1; i<NavigatorTree->topLevelItemCount(); ++i) {
        item = NavigatorTree->topLevelItem(i);
        if (enable) {
            item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        } else {
            item->setFlags(Qt::NoItemFlags);
        }
    }
}
void
nmfMainWindow::loadDatabase()
{
    QString msg = QString::fromStdString("-----------Loading database: "+ProjectDatabase);
    m_logger->logMsg(nmfConstants::Normal,msg.toStdString());
    m_databasePtr->nmfSetDatabase(ProjectDatabase);
}

bool
nmfMainWindow::queryUserPreviousDatabase()
{
    QMessageBox::StandardButton reply;
    std::string msg  = "\nLast Project worked on:  " + ProjectName + "\n\nContinue working with this Project?\n";
    reply = QMessageBox::question(this, tr("Open"), tr(msg.c_str()),
                                  QMessageBox::No|QMessageBox::Yes,
                                  QMessageBox::Yes);

    nmfMainWindowComplete();

    showLogo(false);

    if (reply == QMessageBox::Yes) {
        QString filename = QDir(QString::fromStdString(m_ProjectDir)).filePath(QString::fromStdString(ProjectName));
        Setup_Tab2_ptr->loadProject(filename);
        setNewDatabaseName(ProjectDatabase);
        callback_createTables();
    } else {
        updateMainWindowTitle("");
        callback_createTables();
        callback_NavigatorSelectionChanged();
    }

    showDockWidgets(true);

    return (reply == QMessageBox::Yes);
}


bool
nmfMainWindow::isStoppedAndComplete(std::string &runName,
                                    std::string &elapsedTime,
                                    std::string &numIterationsOrYears,
                                    std::string stopRunFile)
{
    std::string cmd;
    std::ifstream inputFile(stopRunFile);
    if (inputFile) {
        std::getline(inputFile,cmd);
        std::getline(inputFile,runName);
        std::getline(inputFile,elapsedTime);
        std::getline(inputFile,numIterationsOrYears);
    }
    inputFile.close();

    return (cmd == "StopAllOk");

} // end isStoppedAndComplete


void
nmfMainWindow::readMSVPAProgressSetupDataFile(std::string progressDataFile)
{
    int value;

    std::ifstream inputFile(progressDataFile);
    if (inputFile) {
        // Read line of data from file
        std::string str;
        std::getline(inputFile,str);
        inputFile.close();
        // Update widgets with line read
        std::istringstream iss(str);
        std::string token;
        std::string msg;
        std::vector<std::string> parts;
        while (std::getline(iss, token, ';')) {
            parts.push_back(token);
        }

        value = std::stoi(parts[0]);
        if (value == -1) {
//            if (progressDlgMSVPA)
//                progressDlgMSVPA->setValue(progressDlgMSVPA->maximum());
            callback_stopMSVPAProgressSetupTimerOnly();
        } else {
//            if (progressDlgMSVPA)
//                progressDlgMSVPA->setValue(value);
        }

        if (parts.size() == 2) {
//            if (progressDlgMSVPA) {
//                msg = "<p align=\"left\"><br>Status:&nbsp;&nbsp;";
//                msg += parts[1];
//                msg += "</p>";
//                progressDlgMSVPA->setLabelText(QString::fromStdString(msg));
//            }

        }
    }
} // end readMSVPAProgressSetupDataFile


void
nmfMainWindow::readForecastProgressSetupDataFile(std::string progressDataFile)
{
    //int value;

    std::ifstream inputFile(progressDataFile);
    if (inputFile) {
        // Read line of data from file
        std::string str;
        std::getline(inputFile,str);
        inputFile.close();
        // Update widgets with line read
        std::istringstream iss(str);
        std::string token;
        std::string msg;
        std::vector<std::string> parts;
        while (std::getline(iss, token, ';')) {
            parts.push_back(token);
        }

//        value = std::stoi(parts[0]);
//        if (value == -1) {
//            if (progressDlgForecast)
//                progressDlgForecast->setValue(progressDlgForecast->maximum());
//            callback_stopForecastProgressSetupTimerOnly();
//        } else {
//            if (progressDlgForecast)
//                progressDlgForecast->setValue(value);
//        }

//        if (parts.size() == 2) {
//            if (progressDlgForecast) {
//                msg = "<p align=\"left\"><br>Status:&nbsp;&nbsp;";
//                msg += parts[1];
//                msg += "</p>";
//                progressDlgForecast->setLabelText(QString::fromStdString(msg));
//            }

//        }
    }
} // end readForecastProgressSetupDataFile


void
nmfMainWindow::readMSVPAProgressChartDataFile()
{
    progressWidgetMSVPA->readChartDataFile("MSVPA",
                                           nmfConstantsMSVPA::MSVPAProgressChartFile,
                                           nmfConstantsMSVPA::MSVPAProgressChartLabelFile,
                                           "<b>Convergence Values as a Function of Iteration</b>",
                                           "Iterations (i)",
                                           "Convergence Value");
    progressWidgetMSVPA->updateChart();

    std::string runName="";
    std::string msg = "";
    std::string elapsedTime="";
    std::string numIterations="";
    if (isStoppedAndComplete(runName,elapsedTime,numIterations,
                             nmfConstantsMSVPA::MSVPAStopRunFile))
    {
        boost::algorithm::trim(elapsedTime);
        msg  = "\nSuccess!!";
        msg += "\n\nMSVPA completed for: " + runName;
        msg += "\n\nElapsed runtime:  "  + elapsedTime;
        msg += "\nNum iterations:    "   + numIterations + "\n";
        QMessageBox::information(this,
                                 tr("MSVPA Run Complete"),
                                 tr(msg.c_str()),
                                 QMessageBox::Ok);
        progressWidgetMSVPA->callback_stopPB(true);
    }
} // end readMSVPAProgressChartDataFile


void
nmfMainWindow::readForecastProgressChartDataFile()
{
//std::cout << "readFoarecastProgressChartDataFile" << std::endl;

    progressWidgetForecast->readChartDataFile("Forecast",
                                              nmfConstantsMSVPA::ForecastProgressChartFile,
                                              nmfConstantsMSVPA::ForecastProgressChartLabelFile,
                                              "<b>Total Biomass per Forecast Year for Species:&nbsp;&nbsp;</b>",
                                              "Forecast Year",
                                              "000 Metric Tons");

    std::string runName="";
    std::string msg = "";
    std::string elapsedTime="";
    std::string numYears="";
    if (isStoppedAndComplete(runName,elapsedTime,numYears,
                             nmfConstantsMSVPA::ForecastStopRunFile))
    {
        progressWidgetForecast->updateChart();
        boost::algorithm::trim(elapsedTime);
        msg  = "\nSuccess!!";
        msg += "\n\nForecast completed for: "   + runName;
        msg += "\n\nElapsed runtime:  "    + elapsedTime;
        msg += "\nNum Forecast years:    " + numYears + "\n";
        QMessageBox::information(this,
                                 tr("Forecast Run Complete"),
                                 tr(msg.c_str()),
                                 QMessageBox::Ok);
        progressWidgetForecast->callback_stopPB(true);
    }

} // end readForecastProgressChartDataFile




void nmfMainWindow::callback_readMSVPAProgressSetupDataFile()
{

    readMSVPAProgressSetupDataFile(nmfConstantsMSVPA::MSVPAProgressBarFile);

} // end callback_readMSVPAProgressSetupDataFile

void nmfMainWindow::callback_readForecastProgressSetupDataFile()
{

    readForecastProgressSetupDataFile(nmfConstantsMSVPA::ForecastProgressBarFile);

} // end callback_readForecastProgressSetupDataFile





//void nmfMainWindow::readForecastProgressBarDataFile()
//{
//    //readMSVPAProgressSetupDataFile(nmfConstants::ForecastProgressBarFile);
//}


// Store working files in hidden directories
void
nmfMainWindow::setupOutputDir() {
    // Make directories if don't already exist
    boost::filesystem::path mainDir(".MSVPA_X2");
    if (boost::filesystem::create_directory(mainDir)) {
        std::cout << "Created .MSVPA_X2 dir for MSVPA_X2 data files." << std::endl;
    }
    boost::filesystem::path logDir(".MSVPA_X2//logs");
    if (boost::filesystem::create_directory(logDir)) {
        std::cout << "Created .MSVPA_X2/logs dir for MSVPA_X2 log files." << std::endl;
    }
}


void
nmfMainWindow::startForecastProgressBarTimer()
{
    m_logger->logMsg(nmfConstants::Normal,"startForecastProgressBarTimer");
}

void
nmfMainWindow::stopForecastProgressBarTimer()
{
    m_logger->logMsg(nmfConstants::Normal,"stopForecastProgressBarTimer");
    m_logger->logMsg(nmfConstants::Bold,"Forecast Run - End");
    m_logger->logMsg(nmfConstants::Section,"================================================================================");

}



void
nmfMainWindow::startMSVPAProgressSetupTimer()
{
std::cout << "*** *** startMSVPAProgressSetupTimer" << std::endl;
    if (! progressSetupTimerMSVPA)
        return;

    // Start Progress Setup's timer here
    progressSetupTimerMSVPA->start(1000);

    progressWidgetMSVPA->SetupConnections();

    // Alternate progress implementation using QProgressDialog
//    if (progressDlgMSVPA != NULL) {
//        progressDlgMSVPA->deleteLater();
//    }

//    progressDlgMSVPA = new QProgressDialog("\nInitializing data for MSVPA Model Run",
//                                      "Cancel",0,nmfConstants::NumMSVPASetupSteps,
//                                      ui->MSVPAInputTabWidget,Qt::WindowStaysOnTopHint);
//    progressDlgMSVPA->setWindowTitle("MSVPA Run Setup");
//    progressDlgMSVPA->setFixedWidth(550);

//    //progressDlg->setWindowModality(Qt::WindowModal);
//    progressDlgMSVPA->show();

//    connect(progressDlgMSVPA, SIGNAL(canceled()),
//            this, SLOT(callback_stopMSVPAProgressSetupTimer()));
} // end startMSVPAProgressSetupTimer


void
nmfMainWindow::startForecastProgressSetupTimer()
{
    if (! progressSetupTimerForecast)
        return;

    // Start Progress Setup's timer here
    progressSetupTimerForecast->start(1000);

    progressWidgetForecast->SetupConnections();

    // Alternate progress implementation using QProgressDialog
//    if (! progressDlgForecast)
//        progressDlgForecast->deleteLater();
//    progressDlgForecast = new QProgressDialog("\nInitializing data for Forecast Model Run",
//                                      "Cancel",0,nmfConstants::NumForecastSetupSteps,
//                                      ui->ForecastInputTabWidget,Qt::WindowStaysOnTopHint);
//    progressDlgForecast->setWindowTitle("Forecast Run Setup");
//    progressDlgForecast->setFixedWidth(550);
//    //progressDlg->setWindowModality(Qt::WindowModal);
//    progressDlgForecast->show();
//    connect(progressDlgForecast, SIGNAL(canceled()),
//            this, SLOT(callback_stopForecastProgressSetupTimer()));

} // end startForecastProgressSetupTimer


void
nmfMainWindow::callback_stopMSVPAProgressSetupTimer()
{
std::cout << "\n---> callback_stopMSVPAProgressSetupTimer\n" << std::endl;
        m_logger->logMsg(nmfConstants::Normal,"Stop MSVPA Progress Setup Timer and Model Run");
        progressSetupTimerMSVPA->stop();
        progressWidgetMSVPA->StopRun();

} // end callback_stopMSVPAProgressSetupTimer

void
nmfMainWindow::callback_stopMSVPAProgressSetupTimerOnly()
{
std::cout << "\n---> callback_stopMSVPAProgressSetupTimerOnly\n" << std::endl;
        m_logger->logMsg(nmfConstants::Normal,"Stop only MSVPA Progress Setup Timer");

        progressSetupTimerMSVPA->stop();

        std::string msg;
        msg  = "\nPlease note:\n\nMSVPA setup has completed and processing of the first MSVPA iterative loop has begun. Progress ";
        msg += "may be viewed on the Progress Chart window.  Please wait a few moments ";
        msg += "for the first iteration to complete and its respective convergence value ";
        msg += "to be plotted.";
        QMessageBox::information(this,
                                 tr("Information"),
                                 tr(msg.c_str()),
                                 QMessageBox::Ok);

        // Start the progress chart timer here since at this point
        // the setup has been completed and the chart view title will
        // display the current iteration accurately.
        connect(progressChartTimerMSVPA, SIGNAL(timeout()),
                this, SLOT(readMSVPAProgressChartDataFile()));

} // end callback_stopMSVPAProgressSetupTimerOnly


void
nmfMainWindow::callback_stopForecastProgressSetupTimer()
{
std::cout << "\n---> callback_stopForecastProgressSetupTimer\n" << std::endl;
        m_logger->logMsg(nmfConstants::Normal,"Stop Forecast Progress Setup Timer and Model Run");

        progressSetupTimerForecast->stop();

        progressWidgetForecast->StopRun();

} // end callback_stopForecastProgressSetupTimer

void
nmfMainWindow::callback_stopForecastProgressSetupTimerOnly()
{
std::cout << "\n---> callback_stopForecastProgressSetupTimerOnly\n" << std::endl;
        m_logger->logMsg(nmfConstants::Normal,"Stop only Forecast Progress Setup Timer");

        progressSetupTimerForecast->stop();

        std::string msg;
        msg  = "\nPlease note:\n\nForecast setup has completed and processing of the first Forecast year has begun. Progress ";
        msg += "may be viewed on the Progress Chart window.  Please wait a few moments ";
        msg += "for the first year to complete and its respective total biomass value ";
        msg += "to be plotted.";
        QMessageBox::information(this,
                                 tr("Information"),
                                 tr(msg.c_str()),
                                 QMessageBox::Ok);

        // Start the progress chart timer here since at this point
        // the setup has been completed and the chart view title will
        // display the current iteration accurately.
        connect(progressChartTimerForecast, SIGNAL(timeout()),
                this, SLOT(readForecastProgressChartDataFile()));

} // end callback_stopForecastProgressSetupTimerOnly

void
nmfMainWindow::toolbar_screenGrab(bool triggered)
{
    QTabWidget* MSVPAOutputTabWidget = m_UI->OutputWidget->findChild<QTabWidget *>("MSVPAOutputTabWidget");

    if (MSVPAOutputTabWidget->isVisible()) {
        if (MSVPAOutputTabWidget->currentIndex() == 0)
            menu_saveOutputChart();
        else
            menu_saveOutputData();
    } else if (SSVPAOutputTabWidget->isVisible()) {
        if (SSVPAOutputTabWidget->currentIndex() == 0)
            menu_saveOutputChart();
        else
            menu_saveOutputData();
    } else if (SetupOutputTE->isVisible()) {
        menu_saveOutputData();
    }

}

void
nmfMainWindow::toolbar_whatsThis(bool triggered)
{
    QWhatsThis::enterWhatsThisMode();
}


void
nmfMainWindow::showDockWidgets(bool show)
{
    m_UI->NavigatorDockWidget->setVisible(show);
    m_UI->EntityDockWidget->setVisible(show);
}

void
nmfMainWindow::initLogo()
{
    QPixmap logoImage(":/icons/NOAA.png");
    QPixmap logoImageScaled = logoImage.scaled(m_UI->CentralWidget->width()+100,
                                               m_UI->CentralWidget->width()+100,
                                               Qt::KeepAspectRatio);
    QLabel *logoLBL = new QLabel();
    logoLBL->setObjectName("Logo");
    logoLBL->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    logoLBL->setPixmap(logoImageScaled);
    m_UI->CentralWidget->layout()->addWidget(logoLBL);

} // end initLogo


void
nmfMainWindow::showLogo(bool show)
{
    m_UI->CentralWidget->findChild<QLabel *>("Logo")->setVisible(show);

} // end showLogo


void
nmfMainWindow::initGuiPages()
{
    QUiLoader loader;

    // Load ui as a widget from disk
    QFile file(":/forms/Main/Main_NavigatorTreeWidget.ui");
    file.open(QFile::ReadOnly);
    NavigatorTreeWidget = loader.load(&file,this);
    file.close();

    m_UI->NavigatorDockWidget->setWidget(NavigatorTreeWidget);
    NavigatorTree = m_UI->NavigatorDockWidget->findChild<QTreeWidget *>("NavigatorTree");

    Setup_Tab1_ptr    = new nmfSetup_Tab1(m_UI->SetupInputTabWidget);
    Setup_Tab2_ptr    = new nmfSetup_Tab2(m_UI->SetupInputTabWidget,m_logger,SetupOutputTE);
    Setup_Tab3_ptr    = new nmfSetup_Tab3(m_UI->SetupInputTabWidget,m_logger,SetupOutputTE,m_ProjectDir);

    SSVPA_Tab1_ptr    = new nmfSSVPATab1(m_UI->SSVPAInputTabWidget,m_logger,m_ProjectDir);
    SSVPA_Tab2_ptr    = new nmfSSVPATab2(m_UI->SSVPAInputTabWidget,m_logger,m_ProjectDir);
    SSVPA_Tab3_ptr    = new nmfSSVPATab3(m_UI->SSVPAInputTabWidget,m_logger,m_ProjectDir);
    SSVPA_Tab4_ptr    = new nmfSSVPATab4(m_UI->SSVPAInputTabWidget,m_logger,m_ProjectDir);

    MSVPA_Tab1_ptr    = new nmfMSVPATab1(m_UI->MSVPAInputTabWidget,m_logger,m_ProjectDir);
    MSVPA_Tab2_ptr    = new nmfMSVPATab2(m_UI->MSVPAInputTabWidget,m_logger,m_ProjectDir);
    MSVPA_Tab3_ptr    = new nmfMSVPATab3(m_UI->MSVPAInputTabWidget,m_logger,m_ProjectDir);
    MSVPA_Tab4_ptr    = new nmfMSVPATab4(m_UI->MSVPAInputTabWidget,m_logger,m_ProjectDir);
    MSVPA_Tab5_ptr    = new nmfMSVPATab5(m_UI->MSVPAInputTabWidget,m_logger,m_ProjectDir);
    MSVPA_Tab6_ptr    = new nmfMSVPATab6(m_UI->MSVPAInputTabWidget,m_logger,m_ProjectDir);
    MSVPA_Tab7_ptr    = new nmfMSVPATab7(m_UI->MSVPAInputTabWidget,m_logger,m_ProjectDir);
    MSVPA_Tab8_ptr    = new nmfMSVPATab8(m_UI->MSVPAInputTabWidget,m_logger,m_ProjectDir);
    MSVPA_Tab9_ptr    = new nmfMSVPATab9(m_UI->MSVPAInputTabWidget,m_logger);
    MSVPA_Tab10_ptr   = new nmfMSVPATab10(m_UI->MSVPAInputTabWidget,m_logger);
    MSVPA_Tab11_ptr   = new nmfMSVPATab11(m_UI->MSVPAInputTabWidget,m_logger,m_ProjectDir);
    MSVPA_Tab12_ptr   = new nmfMSVPATab12(m_UI->MSVPAInputTabWidget,m_logger);

    Forecast_Tab1_ptr = new nmfForecastTab1(m_UI->ForecastInputTabWidget,m_logger,m_ProjectDir);
    Forecast_Tab2_ptr = new nmfForecastTab2(m_UI->ForecastInputTabWidget,m_logger,m_ProjectDir);
    Forecast_Tab3_ptr = new nmfForecastTab3(m_UI->ForecastInputTabWidget,m_logger,m_ProjectDir);
    Forecast_Tab4_ptr = new nmfForecastTab4(m_UI->ForecastInputTabWidget,m_logger,ScenarioListLV,m_ProjectDir);
    Forecast_Tab5_ptr = new nmfForecastTab5(m_UI->ForecastInputTabWidget,m_logger);

} // end initGuiPages


void
nmfMainWindow::nmfMainWindowComplete()
{
    if (! Session.empty())
        Session = " - " + Session;
    m_logger->logMsg(nmfConstants::Bold,"MSVPA_X2 Start"+Session);

    // Set some SSVPA_Tab4 class variables here
    SelectConfigurationCMB    = m_UI->SSVPAInputTabWidget->findChild<QComboBox *>("SelectConfigurationCMB");
    SelectVPATypeCMB          = m_UI->SSVPAInputTabWidget->findChild<QComboBox *>("SelectVPATypeCMB");
    Configure_Cohort_SB1      = m_UI->SSVPAInputTabWidget->findChild<QSpinBox *>("Configure_Cohort_SB1");
    Configure_Cohort_SB2      = m_UI->SSVPAInputTabWidget->findChild<QSpinBox *>("Configure_Cohort_SB2");
    Configure_Cohort_SB3      = m_UI->SSVPAInputTabWidget->findChild<QSpinBox *>("Configure_Cohort_SB3");
    Configure_Cohort_SB4      = m_UI->SSVPAInputTabWidget->findChild<QSpinBox *>("Configure_Cohort_SB4");
    SSVPAInitialSelectivityTV = m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAInitialSelectivityTV");
    SSVPAMaturityTV           = m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAMaturityTV");
    SSVPAMortalityTV          = m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAMortalityTV");
    SSVPAFleetDataTV          = m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAFleetDataTV");
    SSVPAIndicesTV            = m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAIndicesTV");
    NaturalMortalitySL        = m_UI->SSVPAInputTabWidget->findChild<QSlider *>("NaturalMortalitySL");
    MortalityMinLE            = m_UI->SSVPAInputTabWidget->findChild<QLineEdit *>("MortalityMinLE");
    MortalityMaxLE            = m_UI->SSVPAInputTabWidget->findChild<QLineEdit *>("MortalityMaxLE");
    LastYearMortalityRateDSB  = m_UI->SSVPAInputTabWidget->findChild<QDoubleSpinBox *>("LastYearMortalityRateDSB");
    ResidualNaturalMortalityDSB = m_UI->SSVPAInputTabWidget->findChild<QDoubleSpinBox *>("ResidualNaturalMortalityDSB");
    PredationMortalityDSB     = m_UI->SSVPAInputTabWidget->findChild<QDoubleSpinBox *>("PredationMortalityDSB");
    ReferenceAgeSB            = m_UI->SSVPAInputTabWidget->findChild<QSpinBox *>("ReferenceAgeSB");
    SelectivityMinLE          = m_UI->SSVPAInputTabWidget->findChild<QLineEdit *>("SelectivityMinLE");
    SelectivityMaxLE          = m_UI->SSVPAInputTabWidget->findChild<QLineEdit *>("SelectivityMaxLE");
    SelectivitySL             = m_UI->SSVPAInputTabWidget->findChild<QSlider *>("SelectivitySL");
    NaturalMortalityCB        = m_UI->SSVPAInputTabWidget->findChild<QCheckBox *>("NaturalMortalityCB");
    SSVPASelectConfigurationAndTypeGB = m_UI->SSVPAInputTabWidget->findChild<QGroupBox *>("SSVPASelectConfigurationAndTypeGB");
    Configure_EffortTuned_SB1 = m_UI->SSVPAInputTabWidget->findChild<QSpinBox *>("Configure_EffortTuned_SB1");
    Configure_XSA_NYears_SB   = m_UI->SSVPAInputTabWidget->findChild<QSpinBox *>("Configure_XSA_NYears_SB");
    Configure_XSA_NAges_SB    = m_UI->SSVPAInputTabWidget->findChild<QSpinBox *>("Configure_XSA_NAges_SB");
    Configure_XSA_CMB4        = m_UI->SSVPAInputTabWidget->findChild<QComboBox *>("Configure_XSA_CMB4");
    Configure_XSA_CMB5        = m_UI->SSVPAInputTabWidget->findChild<QComboBox *>("Configure_XSA_CMB5");
    Configure_Downweight_GB   = m_UI->SSVPAInputTabWidget->findChild<QGroupBox *>("Configure_Downweight_GB");
    Configure_XSA_GB          = m_UI->SSVPAInputTabWidget->findChild<QGroupBox *>("Configure_XSA_GB");
    Configure_XSA_LE1         = m_UI->SSVPAInputTabWidget->findChild<QLineEdit *>("Configure_XSA_LE1");
    SSVPAExtendedIndicesCMB   = m_UI->SSVPAInputTabWidget->findChild<QComboBox *>("SSVPAExtendedIndicesCMB");


    // Catch the emit alls from all the input tabs.
    // Set up some listeners for Tab'd panels.  These signals tell the main
    // class that the user has changed some data so that widgets can be enabled,
    // disabled, or modified as necessary to reflect that.

    connect(m_UI->SSVPAInputTabWidget,    SIGNAL(tabBarClicked(int)),
            this,                   SLOT(callback_SSVPAInputTabChanged(int)));

    connect(Setup_Tab2_ptr,         SIGNAL(LoadDatabase(QString)),
            this,                   SLOT(callback_LoadDatabase(QString)));
    connect(Setup_Tab2_ptr,         SIGNAL(LoadDatabase(QString)),
            Setup_Tab3_ptr,         SLOT(callback_LoadDatabase(QString)));
    connect(Setup_Tab2_ptr,         SIGNAL(WaitCursor()),
            this,                   SLOT(callback_WaitCursor()));
    connect(Setup_Tab2_ptr,         SIGNAL(CreateTables()),
            this,                   SLOT(callback_createTables()));
    connect(Setup_Tab2_ptr,         SIGNAL(ProjectSet()),
            this,                   SLOT(callback_ProjectSet()));
    connect(Setup_Tab3_ptr,         SIGNAL(UpdateNavigator(int)),
            this,                   SLOT(callback_updateNavigatorSelection(int)));
    connect(Setup_Tab3_ptr,         SIGNAL(EnableNavigator()),
            this,                   SLOT(callback_enableNavigator()));
//  connect(Setup_Tab3_ptr,         SIGNAL(DeactivateRunButtons()),
//          this,                   SLOT(callback_deactivateRunButtons()));
    connect(Setup_Tab3_ptr,         SIGNAL(CreateAllDatabaseTables()),
            this,                   SLOT(callback_createTables()));

    connect(SSVPA_Tab1_ptr,         SIGNAL(ResetSpeciesList()),
            this,                   SLOT(callback_ResetSpeciesList()));
    connect(SSVPA_Tab1_ptr,         SIGNAL(InitializePage(std::string,int,bool)),
            this,                   SLOT(callback_InitializePage(std::string,int,bool)));
    connect(SSVPA_Tab1_ptr,         SIGNAL(ReselectSpecies(std::string,bool)),
            this,                   SLOT(callback_ReselectSpecies(std::string,bool)));
    connect(SSVPA_Tab1_ptr,         SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(SSVPA_Tab1_ptr,         SIGNAL(MarkSSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(SSVPA_Tab2_ptr,         SIGNAL(ResetSpeciesList()),
            this,                   SLOT(callback_ResetSpeciesList()));
    connect(SSVPA_Tab2_ptr,         SIGNAL(InitializePage(std::string,int,bool)),
            this,                   SLOT(callback_InitializePage(std::string,int,bool)));
    connect(SSVPA_Tab2_ptr,         SIGNAL(ReselectSpecies(std::string,bool)),
            this,                   SLOT(callback_ReselectSpecies(std::string,bool)));
    connect(SSVPA_Tab2_ptr,         SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(SSVPA_Tab2_ptr,         SIGNAL(MarkSSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(SSVPA_Tab3_ptr,         SIGNAL(ResetSpeciesList()),
            this,                   SLOT(callback_ResetSpeciesList()));
    connect(SSVPA_Tab3_ptr,         SIGNAL(InitializePage(std::string,int,bool)),
            this,                   SLOT(callback_InitializePage(std::string,int,bool)));
    connect(SSVPA_Tab3_ptr,         SIGNAL(ReselectSpecies(std::string,bool)),
            this,                   SLOT(callback_ReselectSpecies(std::string,bool)));
    connect(SSVPA_Tab3_ptr,         SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(SSVPA_Tab3_ptr,         SIGNAL(MarkSSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(SSVPA_Tab4_ptr,         SIGNAL(InitializePage(std::string,int,bool)),
            this,                   SLOT(callback_InitializePage(std::string,int,bool)));
    connect(SSVPA_Tab4_ptr,         SIGNAL(RunSSVPA(std::string)),
            this,                   SLOT(callback_RunSSVPA(std::string)));
    connect(SSVPA_Tab4_ptr,         SIGNAL(ReselectSpecies(std::string,bool)),
            this,                   SLOT(callback_ReselectSpecies(std::string,bool)));
//    connect(SSVPA_Tab4_ptr,         SIGNAL(EnableRunSSVPAPB(bool)),
//            this,                   SLOT(callback_EnableRunSSVPAPB(bool)));
    connect(SSVPA_Tab4_ptr,         SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(SSVPA_Tab4_ptr,         SIGNAL(MarkSSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));

    connect(MSVPA_Tab1_ptr,         SIGNAL(MSVPALoadWidgets(int)),
            this,                   SLOT(callback_MSVPALoadWidgets(int)));
    connect(MSVPA_Tab1_ptr,         SIGNAL(UpdateNavigator(std::string,int)),
            this,                   SLOT(callback_updateNavigatorSelection2(std::string,int)));
    connect(MSVPA_Tab1_ptr,         SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(MSVPA_Tab1_ptr,         SIGNAL(MarkMSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(MSVPA_Tab2_ptr,         SIGNAL(MSVPALoadWidgets(int)),
            this,                   SLOT(callback_MSVPALoadWidgets(int)));
    connect(MSVPA_Tab2_ptr,         SIGNAL(UpdateNavigator(std::string,int)),
            this,                   SLOT(callback_updateNavigatorSelection2(std::string,int)));
    connect(MSVPA_Tab2_ptr,         SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(MSVPA_Tab2_ptr,         SIGNAL(MarkMSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(MSVPA_Tab3_ptr,         SIGNAL(MSVPALoadWidgets(int)),
            this,                   SLOT(callback_MSVPALoadWidgets(int)));
    connect(MSVPA_Tab3_ptr,         SIGNAL(UpdateNavigator(std::string,int)),
            this,                   SLOT(callback_updateNavigatorSelection2(std::string,int)));
    connect(MSVPA_Tab3_ptr,         SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(MSVPA_Tab3_ptr,         SIGNAL(MarkMSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(MSVPA_Tab4_ptr,         SIGNAL(MSVPALoadWidgets(int)),
            this,                   SLOT(callback_MSVPALoadWidgets(int)));
    connect(MSVPA_Tab4_ptr,         SIGNAL(UpdateNavigator(std::string,int)),
            this,                   SLOT(callback_updateNavigatorSelection2(std::string,int)));
    connect(MSVPA_Tab4_ptr,         SIGNAL(MSVPATab4(std::string,int,int)),
            MSVPA_Tab5_ptr,         SLOT(callback_RemoveAPreySpecies(std::string,int,int)));
    connect(MSVPA_Tab4_ptr,         SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(MSVPA_Tab4_ptr,         SIGNAL(MarkMSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(MSVPA_Tab5_ptr,         SIGNAL(MSVPALoadWidgets(int)),
            this,                   SLOT(callback_MSVPALoadWidgets(int)));
    connect(MSVPA_Tab5_ptr,         SIGNAL(UpdateNavigator(std::string,int)),
            this,                   SLOT(callback_updateNavigatorSelection2(std::string,int)));
    connect(MSVPA_Tab5_ptr,         SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(MSVPA_Tab5_ptr,         SIGNAL(MarkMSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(MSVPA_Tab6_ptr,         SIGNAL(MSVPALoadWidgets(int)),
            this,                   SLOT(callback_MSVPALoadWidgets(int)));
    connect(MSVPA_Tab6_ptr,         SIGNAL(UpdateNavigator(std::string,int)),
            this,                   SLOT(callback_updateNavigatorSelection2(std::string,int)));
    connect(MSVPA_Tab6_ptr,         SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(MSVPA_Tab6_ptr,         SIGNAL(MarkMSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(MSVPA_Tab7_ptr,         SIGNAL(MSVPALoadWidgets(int)),
            this,                   SLOT(callback_MSVPALoadWidgets(int)));
    connect(MSVPA_Tab7_ptr,         SIGNAL(UpdateNavigator(std::string,int)),
            this,                   SLOT(callback_updateNavigatorSelection2(std::string,int)));
    connect(MSVPA_Tab7_ptr,         SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(MSVPA_Tab7_ptr,         SIGNAL(MarkMSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(MSVPA_Tab8_ptr,         SIGNAL(MSVPALoadWidgets(int)),
            this,                   SLOT(callback_MSVPALoadWidgets(int)));
    connect(MSVPA_Tab8_ptr,         SIGNAL(UpdateNavigator(std::string,int)),
            this,                   SLOT(callback_updateNavigatorSelection2(std::string,int)));
    connect(MSVPA_Tab8_ptr,         SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(MSVPA_Tab8_ptr,         SIGNAL(MarkMSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(MSVPA_Tab11_ptr,        SIGNAL(MSVPALoadWidgets(int)),
            this,                   SLOT(callback_MSVPALoadWidgets(int)));
    connect(MSVPA_Tab11_ptr,        SIGNAL(UpdateNavigator(std::string,int)),
            this,                   SLOT(callback_updateNavigatorSelection2(std::string,int)));
    connect(MSVPA_Tab11_ptr,        SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(MSVPA_Tab11_ptr,        SIGNAL(MarkMSVPAAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(MSVPA_Tab11_ptr,        SIGNAL(CheckAndEnableMSVPAExecutePage()),
            this,                   SLOT(callback_CheckAndEnableMSVPAExecutePage()));
    connect(MSVPA_Tab11_ptr,        SIGNAL(UpdateNavigator(std::string,int)),
            this,                   SLOT(callback_updateNavigatorSelection2(std::string,int)));
    connect(MSVPA_Tab12_ptr,        SIGNAL(MSVPALoadWidgets(int)),
            this,                   SLOT(callback_MSVPALoadWidgets(int)));
    connect(MSVPA_Tab12_ptr,        SIGNAL(UpdateNavigator(std::string,int)),
            this,                   SLOT(callback_updateNavigatorSelection2(std::string,int)));
    connect(MSVPA_Tab12_ptr,        SIGNAL(RunMSVPA()),
            this,                   SLOT(callback_RunMSVPA()));

    connect(Forecast_Tab1_ptr,      SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(Forecast_Tab1_ptr,      SIGNAL(MarkForecastAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(Forecast_Tab1_ptr,      SIGNAL(ReloadForecast(std::string)),
            this,                   SLOT(callback_ReloadForecast(std::string)));
    connect(Forecast_Tab2_ptr,      SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(Forecast_Tab2_ptr,      SIGNAL(MarkForecastAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(Forecast_Tab1_ptr,      SIGNAL(UpdateScenarioList(std::string)),
            this,                   SLOT(callback_UpdateScenarioList(std::string)));
    connect(Forecast_Tab2_ptr,      SIGNAL(LoadDataTable(std::string, std::string,
                                                         std::string, std::string,
                                                         std::string, int)),
            this,                   SLOT(callback_LoadDataTable(std::string,std::string,
                                                         std::string,std::string,
                                                         std::string,int)));
    connect(Forecast_Tab4_ptr,      SIGNAL(LoadDataTable(std::string,std::string,
                                                         std::string,std::string,
                                                         std::string,int)),
            this,                   SLOT(callback_LoadDataTable(std::string,std::string,
                                                         std::string,std::string,
                                                         std::string,int)));
    connect(Forecast_Tab4_ptr,      SIGNAL(ReloadForecast(std::string)),
            this,                   SLOT(callback_ReloadForecast(std::string)));
    connect(Forecast_Tab4_ptr,      SIGNAL(TableDataChanged(std::string)),
            this,                   SLOT(callback_TableDataChanged(std::string)));
    connect(Forecast_Tab4_ptr,      SIGNAL(MarkForecastAsClean()),
            this,                   SLOT(callback_MarkAsClean()));
    connect(Forecast_Tab5_ptr,      SIGNAL(RunForecast()),
            this,                   SLOT(callback_RunForecast()));

    connect(this,                   SIGNAL(LoadDatabase(QString)),
            Setup_Tab3_ptr,         SLOT(callback_LoadDatabase(QString)));

    // Load map used for setting enabled states of comboboxes as users
    // interact with those comboboxes as they're viewing the output charts.
    AllComboBoxes["selectDataTypeCMB"]            = selectDataTypeCMB;
    AllComboBoxes["selectSpeciesCMB"]             = selectSpeciesCMB;
    AllComboBoxes["selectVariableCMB"]            = selectVariableCMB;
    AllComboBoxes["selectByVariablesCMB"]         = selectByVariablesCMB;
    AllComboBoxes["selectSeasonCMB"]              = selectSeasonCMB;
    AllComboBoxes["selectSpeciesAgeSizeClassCMB"] = selectSpeciesAgeSizeClassCMB;
    AllComboBoxes["selectYPRAnalysisTypeCMB"]     = selectYPRAnalysisTypeCMB;
    AllComboBoxes["selectFullyRecruitedAgeCMB"]   = selectFullyRecruitedAgeCMB;
    AllComboBoxes["selectPreySpeciesCMB"]         = selectPreySpeciesCMB;

    AllLabels["selectByVariablesLBL"]             = selectByVariablesLBL;
    AllLabels["selectSeasonLBL"]                  = selectSeasonLBL;
    AllLabels["selectSpeciesAgeSizeClassLBL"]     = selectSpeciesAgeSizeClassLBL;
    AllLabels["selectPreySpeciesLBL"]             = selectPreySpeciesLBL;

    AllButtons["flipLeftPB"]  = flipLeftPB;
    AllButtons["flipRightPB"] = flipRightPB;

    AllCheckBoxes["selectSeasonCB"] = selectSeasonCB;
    AllCheckBoxes["selectSpeciesAgeSizeClassCB"] = selectSpeciesAgeSizeClassCB;
    AllCheckBoxes["selectFullyRecruitedAgeCB"]   = selectFullyRecruitedAgeCB;

    // Show SSVPA current input tab as the 5th one.
    m_UI->SSVPAInputTabWidget->setCurrentIndex(0); //4);

    SaveDlg = new QDialog(this);

    // Set enable off for Navigator items until user load a project.
    enableNavigatorTopLevelItems(false);

    // Initialize the Preferences dialog with default settings.
//    initializePrefDlg();

    disableTabs(m_UI->MSVPAInputTabWidget);

    //callback_EnableRunSSVPAPB(false);

    lastPauseCmd = "None";

    // Setup Progress Widget charts
    setupMSVPAProgressChart();
    setupForecastProgressChart();
    progressMainLayt       = new QVBoxLayout();
    MSVPAProgressWidget    = new QWidget();
    ForecastProgressWidget = new QWidget();
    MSVPAProgressWidget->setLayout(progressWidgetMSVPA->hMainLayt);
    ForecastProgressWidget->setLayout(progressWidgetForecast->hMainLayt);
    progressMainLayt->addWidget(MSVPAProgressWidget);
    progressMainLayt->addWidget(ForecastProgressWidget);
    m_UI->ProgressWidget->setLayout(progressMainLayt);
    MSVPAProgressWidget->hide();
    ForecastProgressWidget->hide();

    // Initialize progress output files
    std::ofstream outputFileMSVPA(nmfConstantsMSVPA::MSVPAProgressChartFile);
    outputFileMSVPA.close();
    std::ofstream outputFileForecast(nmfConstantsMSVPA::ForecastProgressChartFile);
    outputFileForecast.close();


    SetupOutputTE->show();

    setInitialNavigatorState(false);

} // end nmfMainWindowComplete


void
nmfMainWindow::setInitialNavigatorState(bool initialState)
{
    std::vector<std::string> species;
    m_databasePtr->getAllSpecies(m_logger,species);
    if (species.size() == 0) {
        enableNavigatorTopLevelItems(initialState);
    }
}

void
nmfMainWindow::setupLogWidget()
{
    logWidget = new nmfLogWidget(m_logger,nmfConstantsMSVPA::LogDir);
    m_UI->LogWidget->setLayout(logWidget->vMainLayt);
}

void
nmfMainWindow::setupOutputWidget()
{
    outputWidget = new MSVPAVisualizationNode(m_logger);
    m_UI->OutputWidget->setLayout(outputWidget->mainLayout());
    mainGUILayt = qobject_cast<QHBoxLayout *>(m_UI->OutputWidget->layout());

    // Set up Setup panel
    SetupOutputTE = new QTextEdit();
    SetupOutputTE->setToolTip("Shows the output from the Setup procedure.");
    SetupOutputTE->setStatusTip("Shows the output from the Setup procedure.");

    // Setup SSVPA panel
    SSVPAOutputW          = new QWidget();
    SSVPASplitter         = new QSplitter(Qt::Horizontal);
    SSVPAOutputControlsGB = new QGroupBox();
    SSVPAOutputTabWidget  = new QTabWidget();
    SSVPAOutputChartTab   = new QWidget();
    SSVPAOutputDataTab    = new QWidget();
    SSVPAOutputTE         = new QTextEdit();
    QVBoxLayout* laytMain = new QVBoxLayout();
    QVBoxLayout* laytData = new QVBoxLayout();

    SSVPAOutputW->setLayout(laytMain);
    laytMain->addWidget(SSVPASplitter);
    SSVPASplitter->addWidget(SSVPAOutputTabWidget);
    SSVPASplitter->addWidget(SSVPAOutputControlsGB);
    SSVPAOutputTabWidget->addTab(SSVPAOutputChartTab,"Chart");
    SSVPAOutputTabWidget->addTab(SSVPAOutputDataTab, "Data");
    SSVPAOutputDataTab->setLayout(laytData);
    SSVPAOutputControlsGB->setFixedWidth(200);
    laytData->addWidget(SSVPAOutputTE);

    // Add Setup and SSVPA widgets
    mainGUILayt->addWidget(SetupOutputTE);
    mainGUILayt->addWidget(SSVPAOutputW);
    mainGUILayt->setStretch(1,100);

    outputWidget->tabWidget()->hide();
    outputWidget->controlsWidget()->hide();
}



void
nmfMainWindow::setupMSVPAProgressChart()
{
    // The main progress chart view was created in designer.  Get it and the
    // chart to be customized.
    progressWidgetMSVPA = new nmfMSVPAProgressWidget(progressChartTimerMSVPA,
                                           m_logger,
                                           "MSVPA",
                                           "<b>Convergence Values as a Function of Iteration</b>",
                                           "Iterations",
                                           "Convergence Value");
    progressChartMSVPA  = progressWidgetMSVPA->chart;

} // end setupMSVPAProgressChart

void
nmfMainWindow::setupForecastProgressChart()
{
    // The main progress chart view was created in designer.  Get it and the
    // chart to be customized.
    progressWidgetForecast = new nmfMSVPAProgressWidget(progressChartTimerForecast,
                                            m_logger,
                                           "Forecast",
                                           "<b>Total Biomass per Forecast Year</b>",
                                           "Forecast Year",
                                           "000 Metric Tons");
    progressChartForecast = progressWidgetForecast->chart;

} // end setupForecastProgressChart



nmfMainWindow::~nmfMainWindow() {

    //std::cout << "calling destructor" << std::endl;
    delete Setup_Tab1_ptr;
    delete Setup_Tab2_ptr;
    delete Setup_Tab3_ptr;
    delete SSVPA_Tab1_ptr;
    delete SSVPA_Tab2_ptr;
    delete SSVPA_Tab3_ptr;
    delete SSVPA_Tab4_ptr;
    delete MSVPA_Tab1_ptr;
    delete MSVPA_Tab2_ptr;
    delete MSVPA_Tab3_ptr;
    delete MSVPA_Tab4_ptr;
    delete MSVPA_Tab5_ptr;
    delete MSVPA_Tab6_ptr;
    delete MSVPA_Tab7_ptr;
    delete MSVPA_Tab8_ptr;
    delete MSVPA_Tab9_ptr;
    delete MSVPA_Tab10_ptr;
    delete MSVPA_Tab11_ptr;
    delete MSVPA_Tab12_ptr;
    delete Forecast_Tab1_ptr;
    delete Forecast_Tab2_ptr;
    delete Forecast_Tab3_ptr;
    delete Forecast_Tab4_ptr;
    delete Forecast_Tab5_ptr;

//    progressDlgMSVPA->deleteLater();

    //progressTimer->stop();
    //delete progressTimer;
    m_logger->logMsg(nmfConstants::Normal,"MSVPA_X2 Exited Normally.");
}

void
nmfMainWindow::callback_ProjectSet()
{
    enableApplicationFeatures(true);
}

void
nmfMainWindow::callback_createTables()
{
    int NumTables = nmfConstantsMSVPA::AllTables.size();
    std::string tableName = "Application";

    QProgressDialog progress("\nInitializing database tables...", "Cancel", 0, NumTables, this);
    progress.setWindowModality(Qt::WindowModal);

    for (int i = 0; i < NumTables; ++i) {
        progress.setValue(i);

        m_databasePtr->checkForTableAndCreate(QString::fromStdString(nmfConstantsMSVPA::AllTables[i]));

        if (progress.wasCanceled())
            break;
    }
    progress.setValue(NumTables);

    if (! m_databasePtr->nmfGetCurrentDatabase().empty()) {
        // Populate the Application database
        m_databasePtr->saveApplicationTable(this,m_logger,tableName);
    }

}


void
nmfMainWindow::enableNavigatorTopLevelItems(bool enable)
{
    NavigatorTree->topLevelItem(1)->setDisabled(! enable);
    NavigatorTree->topLevelItem(2)->setDisabled(! enable);
    NavigatorTree->topLevelItem(3)->setDisabled(! enable);
} // enableNavigatorTopLevelItems


// Load the species index and name into a vector of pairs for
// all other pred species.
void
nmfMainWindow::loadOtherPredSpecies()
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::pair<int,std::string> item;

    fields = {"SpeIndex","SpeName"};
    queryStr = "SELECT SpeIndex,SpeName FROM OtherPredSpecies;";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);

    for (unsigned int i=0; i<dataMap["SpeName"].size(); i++) {
        item = std::make_pair(
            std::stoi(dataMap["SpeIndex"][i]), dataMap["SpeName"][i]);
        OtherPredSpecies.push_back(item);
    }

} // end getOtherPredSpecies



void
nmfMainWindow::callback_MSVPATab4(std::string tabAction)
{

    if (tabAction == "SavePB") {
        std::cout << "Calling MSVPA_Tab5_SavePB..." << std::endl;
        MSVPA_Tab5_ptr->callback_MSVPA_Tab5_SavePB(true);
    }

} // end callback_MSVPATab4



void
nmfMainWindow::callback_LoadDatabase(QString database)
{

    ProjectDatabase = findChild<QComboBox *>("Setup_Tab2_ProjectDatabaseCMB")->currentText().toStdString();
    ProjectName     = findChild<QLineEdit *>("Setup_Tab2_ProjectNameLE")->text().toStdString();
    m_ProjectDir      = findChild<QLineEdit *>("Setup_Tab2_ProjectDirLE")->text().toStdString();
    //ProjectDatabase = Setup_Tab2_ProjectDatabaseCMB->currentText().toStdString();
    //ProjectName     = Setup_Tab2_ProjectNameLE->text().toStdString();
    //ProjectDir      = Setup_Tab2_ProjectDirLE->text().toStdString();
std::cout << "*** new project dir: " << m_ProjectDir << std::endl;

    // RSK - create base classes for these tabs and then iterate

    // Update ProjectDir
    SSVPA_Tab1_ptr->updateProjectDir(m_ProjectDir);
    SSVPA_Tab2_ptr->updateProjectDir(m_ProjectDir);
    SSVPA_Tab3_ptr->updateProjectDir(m_ProjectDir);
    SSVPA_Tab4_ptr->updateProjectDir(m_ProjectDir);

    MSVPA_Tab1_ptr->updateProjectDir(m_ProjectDir);
    MSVPA_Tab2_ptr->updateProjectDir(m_ProjectDir);
    MSVPA_Tab3_ptr->updateProjectDir(m_ProjectDir);
    MSVPA_Tab4_ptr->updateProjectDir(m_ProjectDir);
    MSVPA_Tab5_ptr->updateProjectDir(m_ProjectDir);
    MSVPA_Tab6_ptr->updateProjectDir(m_ProjectDir);
    MSVPA_Tab7_ptr->updateProjectDir(m_ProjectDir);
    MSVPA_Tab8_ptr->updateProjectDir(m_ProjectDir);
    //MSVPA_Tab9_ptr->updateProjectDir(ProjectDir);
    //MSVPA_Tab10_ptr->updateProjectDir(ProjectDir);
    MSVPA_Tab11_ptr->updateProjectDir(m_ProjectDir);

    Forecast_Tab1_ptr->updateProjectDir(m_ProjectDir);
    Forecast_Tab2_ptr->updateProjectDir(m_ProjectDir);
    Forecast_Tab3_ptr->updateProjectDir(m_ProjectDir);
    Forecast_Tab4_ptr->updateProjectDir(m_ProjectDir);

    setNewDatabaseName(database.toStdString());

} // end callback_LoadDatabase


void
nmfMainWindow::callback_WaitCursor()
{
    this->setCursor(Qt::WaitCursor);
}

void
nmfMainWindow::callback_ExportTableToCSVFile(std::string table, std::string csvFile)
{
    std::string cmd = "mysql ";

    cmd += "-u" + m_Username + " -p" + m_Password + " -e " +
            "' select * from " + DatabaseName + "." + table + "' " +
            "| sed 's/\\t/,/g' > " + csvFile;
std::cout << "CMD: " << cmd << std::endl;
    m_logger->logMsg(nmfConstants::Error,"Export Table To CSV File is TBD");
    //system(cmd);
    // mysql -uroot -p -e 'select * from Sept_10.Scenarios' |
    //        sed 's/\t/,/g' > ronScenarios.csv;

} // end callback_ExportTableToCSVFile



//void
//nmfMainWindow::callback_MSVPATabChanged(int tab)
//{
//    std::cout << "tab: " << tab << std::endl;
//    callback_MSVPALoadWidgets(tab);


//} // end callback_MSVPATabChanged


void
nmfMainWindow::callback_SelectTab(std::string returnToSection, int returnToTab,
                                  std::string MSVPAName, std::string ForecastName)
{
    QModelIndex index;

  if (returnToSection == "Forecast") {

      callback_NavigatorSelectionChanged();

      activateForecastWidgets();

      for (int i=0;i<EntityListLV->model()->rowCount();++i) {
          index = EntityListLV->model()->index(i,0);
          if (MSVPAName == EntityListLV->model()->data(index).toString().toStdString()) {
              EntityListLV->setCurrentIndex(index);
          }
      } // end for

      // Reload forecast widgets
      reloadForecastWidgets(ForecastName);

      loadForecastListWidget();

      m_UI->ForecastInputTabWidget->setCurrentIndex(returnToTab);
      if (returnToTab == 3) {
          Forecast_Tab4_ptr->refresh();
      }

      reloadForecastWidgets(ForecastName);

      for (int i=0;i<ForecastListLV->model()->rowCount();++i) {
          index = ForecastListLV->model()->index(i,0);
          if (ForecastName == ForecastListLV->model()->data(index).toString().toStdString()) {
              ForecastListLV->setCurrentIndex(index);
          }
      } // end for

  }
} // end callback_SelectTab


void
nmfMainWindow::callback_ReloadForecast(std::string tab)
{
    std::string configData;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;

    if (tab == "Tab4") {
        Forecast_Tab4_ptr->load(m_databasePtr,
                                MSVPAName, forecastName(),
                                forecastFirstYear(), forecastNYears(),
                                FirstYear, LastYear);
    } else if (tab == "Tab5") {

        configData = Forecast_Tab4_ptr->getConfigData();

        fields = {"MSVPAName","ForeName","InitYear","NYears","Growth"};
        queryStr = "SELECT MSVPAName,ForeName,InitYear,NYears,Growth FROM Forecasts WHERE MSVPAName='" + MSVPAName + "'" +
                " AND ForeName='" + forecastName() + "'";
        dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["MSVPAName"].size() > 0) {
            Forecast_Tab5_ptr->loadWidgets(MSVPAName,
                                           forecastName(),
                                           scenarioName(),
                                           std::stoi(dataMap["InitYear"][0]),
                                           std::stoi(dataMap["NYears"][0]),
                                           std::stoi(dataMap["Growth"][0]),
                                           configData);
        }
    }


    //loadForecastInputWidgets();
} // end callback_ReloadForecast


void
nmfMainWindow::callback_LoadDataTable(std::string MSVPAName, std::string ForecastName,
                                      std::string model, std::string table,
                                      std::string originSection, int originTab)
{

    if (model == "Forecast") {

        activateSetupWidgets();
        //ui->SetupInputTabWidget->setCurrentIndex(5);

//        Setup_Tab6_TableListCMB->setCurrentText(QString::fromStdString(table));
//        Setup_Tab6_ptr->setReturnData(originSection,originTab,
//                                      MSVPAName, ForecastName);

    }


} // end callback_LoadDataTable


void
nmfMainWindow::callback_UpdateScenarioList(std::string scenarioToSelect)
{
    int row = -1;
    QModelIndex indexToSelect;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;

    loadForecastListWidget();

    // Load Scenario list
    scenario_model.removeRows(0, scenario_model.count(), QModelIndex());
    fields = {"Scenario"};
    queryStr = "SELECT Scenario FROM Scenarios WHERE MSVPAName='" + entityName() + "'" +
            " AND ForeName='" + forecastName() + "'";

    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    for (std::string name : dataMap[fields[0]]) {
        ++row;
        scenario_model.append( QString::fromStdString(name));
        if (scenarioToSelect == name) {
            indexToSelect = scenario_model.index(row,0);
        }
    }
    ScenarioListLV->scrollToTop();

    ScenarioListLV->selectionModel()->setCurrentIndex(indexToSelect,QItemSelectionModel::Select);

    loadForecastInputWidgets();

    if (scenarioToSelect.empty()) {
        ScenarioListLV->clearFocus();
        ScenarioListLV->selectionModel()->clearSelection();
    }
//    Forecast_Tab2_ptr->loadWidgets(databasePtr,
//                                   vonBert_model,
//                                   entityName(),
//                                   forecastName());


} // end callback_UpdateScenarioList


//void
//nmfMainWindow::callback_EnableRunSSVPAPB(bool toggle)
//{
//    RunSSVPAPB->setEnabled(toggle);
//}

void
nmfMainWindow::callback_TableDataChanged(std::string tableName)
{
    DirtyTables.insert(tableName);

    // Mark window title as being modified (i.e. with asterisk)
    // Reset window title (i.e., remove asterisk from title)
    QString winTitle = windowTitle();
    winTitle.replace("MSVPA_X2:","MSVPA_X2*:");
    setWindowTitle(winTitle);
}

void nmfMainWindow::draw3DCharts()
{
    enableMSVPAWidgets(false);
    selectionModeLBL->setEnabled(true);
    selectionModeCMB->setEnabled(true);

    if ((graph3D != NULL) && (m_modifier != NULL)) {
        m_modifier->resetData(M2Matrix);
        chartView3DContainer->show();
        return;
    }

    graph3D = new Q3DBars();

    Q3DTheme *myTheme;
    myTheme = graph3D->activeTheme();
    myTheme->setLabelBorderEnabled(false);

    chartView3DContainer = QWidget::createWindowContainer(graph3D);
    if (!graph3D->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
        return;
    }

    chartView3DContainer->setMinimumWidth(800);
    chartView3DContainer->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    //splitterMSVPA->insertWidget(0,chartView3DContainer);
    hMainLayout->insertWidget(0,chartView3DContainer);

    m_modifier = new nmfOutputChart3DBarModifier(graph3D);
    m_modifier->resetData(M2Matrix);

    QObject::connect(selectionModeCMB, SIGNAL(currentIndexChanged(int)),
                     m_modifier,       SLOT(changeSelectionMode(int)));

}



void nmfMainWindow::restoreTitleFonts()
{
    QFont mainTitleFont = chart->titleFont();
    mainTitleFont.setPointSize(14);
    mainTitleFont.setWeight(QFont::Bold);
    chart->setTitleFont(mainTitleFont);

    QAbstractAxis *axisX = chart->axes(Qt::Horizontal).back();
    QFont titleFont = axisX->titleFont();
    titleFont.setPointSize(12);
    titleFont.setWeight(QFont::Bold);
    axisX->setTitleFont(titleFont);

    QAbstractAxis *axisY = chart->axes(Qt::Vertical).back();
    titleFont = axisY->titleFont();
    titleFont.setPointSize(12);
    titleFont.setWeight(QFont::Bold);
    axisY->setTitleFont(titleFont);
}



void nmfMainWindow::callback_chartThemeChanged(int newTheme) {
   chart->setTheme((QChart::ChartTheme)newTheme);
   restoreTitleFonts();
}

void nmfMainWindow::callback_flipAgesDown() {
    //int maxNumAges = selectSpeciesAgeSizeClassCMB->count();
    int numAges = selectSpeciesAgeSizeClassCMB->currentIndex();
    if (--numAges < 0)
        numAges = 0;
    selectSpeciesAgeSizeClassCMB->setCurrentIndex(numAges);
}

void nmfMainWindow::callback_flipAgesUp() {
    int maxNumAges = selectSpeciesAgeSizeClassCMB->count();
    int numAges = selectSpeciesAgeSizeClassCMB->currentIndex();
    if (++numAges >= maxNumAges)
        numAges = maxNumAges-1;
    selectSpeciesAgeSizeClassCMB->setCurrentIndex(numAges);
}

void nmfMainWindow::resetScaleWidgets(QCheckBox *checkbox, QLineEdit *lineedit)
{
    checkbox->setChecked(Qt::Unchecked);
    lineedit->blockSignals(true);
    lineedit->setText("");
    lineedit->blockSignals(false);
}


void nmfMainWindow::callback_selectDataTypeChanged(QString dataType)
{
    enable3DControls(false);
    if (chartView3DContainer != NULL) {
        chartView3DContainer->hide();
    }
//    chartView->show(); // RSK - test this more...don't think it's necessary
    QTableWidget* ChartDataTbW = m_UI->OutputWidget->findChild<QTableWidget*>("Data");

    std::vector<std::string> selectedYears = {};

    resetScaleWidgets(selectSeasonCB,selectSeasonLE);
    resetScaleWidgets(selectSpeciesAgeSizeClassCB,selectSpeciesAgeSizeClassLE);
    resetScaleWidgets(selectFullyRecruitedAgeCB,selectFullyRecruitedAgeLE);
    YMaxFullyRecruitedAge = 0;
    std::string theModelName = modelName();
    QStringList categories;
    QStringList rowLabels;
    QStringList PreyList;
    QStringList colLabels;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    QTabWidget* MSVPAOutputTabWidget = m_UI->OutputWidget->findChild<QTabWidget *>("MSVPAOutputTabWidget");


    // Update widgets
    if (dataType == "Yield Per Recruit" ) {
        selectVariableLBL->hide();
        selectVariableCMB->hide();
        selectByVariablesLBL->hide();
        selectByVariablesCMB->hide();
        selectSeasonLBL->hide();
        selectSeasonCMB->hide();
        selectSpeciesAgeSizeClassLBL->hide();
        selectSpeciesAgeSizeClassCMB->hide();
        selectYPRAnalysisTypeLBL->show();
        selectYPRAnalysisTypeCMB->show();
        selectYearsLBL->show();
        selectYearsLW->show();
        selectYearsLW->setEnabled(true);
        selectFullyRecruitedAgeLBL->show();
        selectFullyRecruitedAgeCMB->show();
        selectFullyRecruitedAgeCB->show();
        selectFullyRecruitedAgeLE->show();
        selectSeasonCB->hide();
        selectSeasonLE->hide();
        selectSpeciesAgeSizeClassCB->hide();
        selectSpeciesAgeSizeClassLE->hide();
        flipLeftPB->hide();
        flipRightPB->hide();
        MSVPAOutputTabWidget->setTabEnabled(2,true);
    } else {
        selectVariableLBL->show();
        selectVariableCMB->show();
        selectByVariablesLBL->show();
        selectByVariablesCMB->show();
        selectSeasonLBL->show();
        selectSeasonCMB->show();
        selectSpeciesAgeSizeClassLBL->show();
        selectSpeciesAgeSizeClassCMB->show();
        selectYPRAnalysisTypeLBL->hide();
        selectYPRAnalysisTypeCMB->hide();
        selectYearsLBL->hide();
        selectYearsLW->hide();
        selectFullyRecruitedAgeLBL->hide();
        selectFullyRecruitedAgeCMB->hide();
        selectFullyRecruitedAgeCB->hide();
        selectFullyRecruitedAgeLE->hide();
        selectSeasonCB->show();
        selectSeasonLE->show();
        selectSpeciesAgeSizeClassCB->show();
        selectSpeciesAgeSizeClassLE->show();
        //flipLeftPB->show(); // RSK - put these back in later.  Not sure if they're useful.
        //flipRightPB->show();
        MSVPAOutputTabWidget->setTabEnabled(2,false);
    }

    if (theModelName == "MSVPA") {
        loadMSVPAChartWidgets();
    } else if (theModelName == "Forecast") {
        loadForecastChartWidgets();
    }

    if (dataType == "Diet Composition" ) {

        if (NULL != outputChart) {
            delete outputChart;
            outputChart = NULL;
        }

        outputChart = new nmfOutputChartStackedBar(m_logger);

        outputChart->redrawChart("dataType",
                    m_databasePtr, theModelName,
                    chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                    MSVPAName, forecastName(), scenarioName(),
                    forecastFirstYear(), forecastNYears(),
                    FirstYear,
                    dataType.toStdString(),
                    selectSpeciesCMB->currentText().toStdString(),
                    selectSpeciesCMB->currentIndex(),
                    selectVariableCMB->currentText().toStdString(),
                    selectByVariablesCMB->currentText().toStdString(),
                    selectSeasonCMB->currentText().toStdString(),
                    selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                    "", "", selectedYears, "",0.0,
                    hGridLine2d, vGridLine2d,
                    categories, PreyList, ChartData, GridData);

        int numDigits   = 7;
        int numDecimals = 3;
        sendToOutputTable(ChartDataTbW, categories, PreyList, GridData, numDigits, numDecimals);

    } else if (dataType == "Population Size" ) {

        if (NULL != outputChart) {
            delete outputChart;
            outputChart = NULL;
        }
        outputChart = new nmfOutputChartBar(m_logger);
        outputChart->redrawChart("dataType",
                    m_databasePtr, theModelName,
                    chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                    MSVPAName, forecastName(), scenarioName(),
                    forecastFirstYear(), forecastNYears(),
                    FirstYear,
                    dataType.toStdString(),
                    selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
                    selectVariableCMB->currentText().toStdString(),
                    selectByVariablesCMB->currentText().toStdString(),
                    selectSeasonCMB->currentText().toStdString(),
                    selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                    selectPreySpeciesCMB->currentText().toStdString(),
                    "", selectedYears, "",0.0,
                    hGridLine2d, vGridLine2d,
                    rowLabels, colLabels, ChartData, GridData);
//std::cout << "row size: " << rowLabels.size() << std::endl;
//std::cout << "col size: " << colLabels.size() << std::endl;
//std::cout << "griddata size: " << GridData.size1() << "," << GridData.size2() << std::endl;
//std::cout << "griddata 0,0: " << GridData(0,0) << std::endl;

        int numDigits   = 7;
        int numDecimals = 3;
        sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);


    } else if ((dataType == "Mortality Rates" ) &&
               (selectVariableCMB->currentText() != "Predation Mortality by Predator")) {

        if (NULL != outputChart) {
            delete outputChart;
            outputChart = NULL;
        }
        outputChart = new nmfOutputChartBar(m_logger);
        outputChart->redrawChart("dataType",
                    m_databasePtr, theModelName,
                    chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                    MSVPAName, forecastName(), scenarioName(),
                     forecastFirstYear(), forecastNYears(),
                     FirstYear,
                    dataType.toStdString(),
                    selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
                    selectVariableCMB->currentText().toStdString(),
                    selectByVariablesCMB->currentText().toStdString(),
                    selectSeasonCMB->currentText().toStdString(),
                    selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                    selectPreySpeciesCMB->currentText().toStdString(),
                    "", selectedYears, "",0.0,
                    hGridLine2d, vGridLine2d,
                    rowLabels, colLabels, ChartData, GridData);

//std::cout << "row size: " << rowLabels.size() << std::endl;
//std::cout << "col size: " << colLabels.size() << std::endl;
//std::cout << "griddata size: " << GridData.size1() << "," << GridData.size2() << std::endl;
//std::cout << "griddata 0,0: " << GridData(0,0) << std::endl;

        int numDigits   = 7;
        int numDecimals = 3;
        sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);

    } else if ((dataType == "Consumption Rates") ||
               (dataType == "Food Availability")) {

        if (NULL != outputChart) {
            delete outputChart;
            outputChart = NULL;
        }

        std::string selVar = selectVariableCMB->currentText().toStdString();
        if (selVar == "Food Availability by Prey Type") {
            outputChart = new nmfOutputChartStackedBar(m_logger);
        } else {
            outputChart = new nmfOutputChartBar(m_logger);
        }
        outputChart->redrawChart("dataType",
                    m_databasePtr, theModelName,
                    chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                    MSVPAName, forecastName(), scenarioName(),
                     forecastFirstYear(), forecastNYears(),
                     FirstYear,
                    dataType.toStdString(),
                    selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
                    selectVariableCMB->currentText().toStdString(),
                    selVar,
                    selectSeasonCMB->currentText().toStdString(),
                    selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                    selectPreySpeciesCMB->currentText().toStdString(),
                    "", selectedYears, "",0.0,
                    hGridLine2d, vGridLine2d,
                    rowLabels, colLabels, ChartData, GridData);

        int numDigits   = 7;
        int numDecimals = 2;
        sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);

    } else if (dataType == "Yield Per Recruit" ) {

        getSelectedListWidgetItems(selectYearsLW, selectedYears);
        if (NULL != outputChart) {
            delete outputChart;
            outputChart = NULL;
        }
        outputChart = new nmfOutputChartLine(m_logger);
        outputChart->redrawChart("dataType",
                    m_databasePtr, theModelName,
                    chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                    MSVPAName, forecastName(), scenarioName(),
                                 forecastFirstYear(), forecastNYears(),
                                 FirstYear,
                    dataType.toStdString(),
                    selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
                    selectVariableCMB->currentText().toStdString(),
                    selectByVariablesCMB->currentText().toStdString(),
                    selectSeasonCMB->currentText().toStdString(),
                    selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                    "",
                    selectYPRAnalysisTypeCMB->currentText().toStdString(),
                    selectedYears,
                    selectFullyRecruitedAgeCMB->currentText().toStdString(),
                    YMaxFullyRecruitedAge,
                    hGridLine2d, vGridLine2d,
                    rowLabels, colLabels, ChartData, GridData);

        int numDigits   = 7;
        int numDecimals = 2;
        sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);

    } else if (dataType == "Multispecies Populations" ) {

        if (NULL != outputChart) {
            delete outputChart;
            outputChart = NULL;
        }
        outputChart = new nmfOutputChartLine(m_logger);
        outputChart->redrawChart("dataType",
                    m_databasePtr, theModelName,
                    chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                    MSVPAName, forecastName(), scenarioName(),
                                 forecastFirstYear(), forecastNYears(),
                                 FirstYear,
                    dataType.toStdString(),
                    selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
                    selectVariableCMB->currentText().toStdString(),
                    selectByVariablesCMB->currentText().toStdString(),
                    selectSeasonCMB->currentText().toStdString(),
                    selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                    "",
                    selectYPRAnalysisTypeCMB->currentText().toStdString(),
                    selectedYears,
                    selectFullyRecruitedAgeCMB->currentText().toStdString(),
                    YMaxFullyRecruitedAge,
                    hGridLine2d, vGridLine2d,
                    rowLabels, colLabels, ChartData, GridData);

        int numDigits   = 7;
        int numDecimals = 3;
        sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);

    } else if (dataType == "Growth" ) {

         if (NULL != outputChart) {
             delete outputChart;
             outputChart = NULL;
         }

        outputChart = new nmfOutputChartBar(m_logger);
        outputChart->redrawChart("dataType",
                    m_databasePtr, theModelName,
                    chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                    MSVPAName, forecastName(), scenarioName(),
                    forecastFirstYear(), forecastNYears(),
                    FirstYear,
                    dataType.toStdString(),
                    selectSpeciesCMB->currentText().toStdString(),
                    selectSpeciesCMB->currentIndex(),
                    selectVariableCMB->currentText().toStdString(),
                    selectByVariablesCMB->currentText().toStdString(),
                    selectSeasonCMB->currentText().toStdString(),
                    selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                    "",
                    selectYPRAnalysisTypeCMB->currentText().toStdString(),
                    selectedYears,
                    selectFullyRecruitedAgeCMB->currentText().toStdString(),
                    YMaxFullyRecruitedAge,
                    hGridLine2d, vGridLine2d,
                    rowLabels, colLabels, ChartData, GridData);

        int numDigits   = 7;
        int numDecimals = 3;
        sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);

    }

} // end callback_selectDataTypeChanged


void
nmfMainWindow::MSVPA_DietComposition(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string MSVPAName,
        std::string unused1,
        std::string unused2,
        int unusedInt1,
        int unusedInt2,
        std::string selectedSpecies,
        int unusedInt3,
        std::string selectedSpeciesAgeSizeClass,
        QTableWidget *chartDataTbW)
{
    int m;
    int NPrey;
    int Nage = 0;
    int numRecords;
    int MSVPA_FirstYear=0;
    int MSVPA_LastYear=0;
    int MSVPA_NYears=0;
    int MSVPA_NSeasons=0;
    std::string queryStr;
    std::string PredAge;
    std::string xLabel = "";
    std::string yLabel = "";
    std::string mainTitle = "";
    std::vector<std::string> fields;
    QStringList PreyList;
    std::map<std::string, std::vector<std::string> > dataMap;
    boost::numeric::ublas::matrix<double> ChartData;
    QStringList categories;
    QStackedBarSeries *series = NULL;

    chart->removeAllSeries();


//       First get ChartData and GridData
    databasePtr->nmfGetMSVPAInitData(MSVPAName,
                                     MSVPA_FirstYear,
                                     MSVPA_LastYear,
                                     MSVPA_NYears,
                                     MSVPA_NSeasons);

    // Find number of Age groups
    fields = {"NumAges"};
    queryStr = "SELECT COUNT(DISTINCT(Age)) as NumAges from MSVPAprefs WHERE MSVPAname='" + MSVPAName +
            "' and SpeName='" + selectedSpecies + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    Nage = std::stoi(dataMap["NumAges"][0]);
    databasePtr->nmfQueryMsvpaPreyList(selectedSpeciesAgeSizeClass, selectedSpecies, MSVPAName,
                                       PredAge, NPrey, PreyList, false);

    nmfUtils::initialize(ChartData, Nage, NPrey);

    // Calculate ChartData based upon passed in arguments
    fields = {"PredName","PredAge","PreyName","Year","Season","Diet"};
    for (int i=0; i<Nage; ++i) {
        for (int j=0; j<=NPrey-1; ++j) {
            queryStr = "SELECT PredName,PredAge,PreyName,Year,Season,Sum(PropDiet) as Diet FROM MSVPASuitPreyBiomass WHERE MSVPAName ='" + MSVPAName + "'" +
                    " AND PredName = '" + selectedSpecies + "'" +
                    " AND PredAge = " + std::to_string(i) +
                    " AND PreyName = '" + PreyList[j].toStdString() + "'" +
                    " GROUP BY PredName,PredAge,PreyName,Year,Season";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            numRecords = dataMap["PredName"].size();
            if (numRecords > 0) {
                m = 0;
                for (int k=0; k<=MSVPA_NYears; ++k) {
                    for (int l=0; l<MSVPA_NSeasons; ++l) {
                        if (m < numRecords) {
                            ChartData(i,j) += std::stod(dataMap["Diet"][m++]);
                        }
                    } // end for l
                } // end for k
                ChartData(i,j) /= (MSVPA_NSeasons*(MSVPA_NYears+1));
            } // end if
        } // end for j
    } // end for i

    // if current species is of Type=3 then the x axis is Size Class, else it's Age Class
    std::string ageSizePrefix = "Age ";
    int NageOrSizeCategories = Nage;
    int sizeOffset = 0;
    fields = {"SpeName"};
    queryStr = "SELECT SpeName from MSVPAspecies WHERE MSVPAName='" + MSVPAName +
            "' and SpeName='" + selectedSpecies + "' and Type=3";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["SpeName"].size() > 0) {
        if (dataMap["SpeName"][0] == selectedSpecies) {
            ageSizePrefix = "Size ";
        }
    }
    fields = {"SpeName","NumSizeCats"};
    queryStr = "SELECT SpeName,NumSizeCats from OtherPredSpecies WHERE SpeName='" + selectedSpecies + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["SpeName"].size() > 0) {
        if (dataMap["SpeName"][0] == selectedSpecies) {
            sizeOffset = 1;
            NageOrSizeCategories = std::stoi(dataMap["NumSizeCats"][0]);
        }
    }
    for (int ageOrSize=0; ageOrSize<NageOrSizeCategories; ++ageOrSize) {
        categories << QString::fromStdString(ageSizePrefix) + QString::number(ageOrSize+sizeOffset);
    }




    // Set the titles
    mainTitle = "Average " + selectedSpecies + "Diet By " + xLabel;
    xLabel = ageSizePrefix+"Class";
    yLabel = "Prop. Diet";

    // Draw the chart and set its titles.
    loadChartWithData(chart,series,PreyList,ChartData);
    setTitles(chart, series, categories, selectedSpecies, mainTitle, xLabel, yLabel);

    // Display Grid (i.e. numeric) data
    //int numDigits = 7;
    //int numDecimals = 3;
    //sendToOutputTable(chartDataTbW, categories, PreyList, ChartData, numDigits, numDecimals);

} // end MSVPA_DietComposition

void
nmfMainWindow::Forecast_DietComposition(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string MSVPAName,
        std::string ForecastName,
        std::string ScenarioName,
        int Forecast_FirstYear,
        int Forecast_NYears,
        std::string selectedSpecies,
        int PredNum,
        std::string selectedSpeciesAgeSizeClass,
        QTableWidget *chartDataTbW)
{
    int m;
    int NPrey;
    int Nage = 0;
    int NPreds;
    int NOthPreds;
    int NumRecords;
    int MSVPA_FirstYear=0;
    int MSVPA_LastYear=0;
    int MSVPA_NYears=0;
    int MSVPA_NSeasons=0;
    int Forecast_NSeasons;
    std::string queryStr,queryStr2;
    std::string xLabel = "";
    std::string yLabel = "";
    std::string mainTitle = "";
    std::vector<std::string> fields,fields2;
    QStringList PreyList;
    std::map<std::string, std::vector<std::string> > dataMap,dataMap2;
    boost::numeric::ublas::matrix<double> ChartData;
    QStringList categories;
    QStackedBarSeries *series = NULL;

    chart->removeAllSeries();
    boost::numeric::ublas::vector<std::string> OthPredList;
    boost::numeric::ublas::vector<int>         NPredAge;
    boost::numeric::ublas::vector<int>         NOthPredAge;
    boost::numeric::ublas::vector<std::string> PredList;
    boost::numeric::ublas::vector<int>         PredType;
    nmfUtils::initialize(PredType,    nmfConstants::MaxNumberSpecies);
    nmfUtils::initialize(PredList,    nmfConstants::MaxNumberSpecies);
    nmfUtils::initialize(OthPredList, nmfConstants::MaxNumberSpecies);
    nmfUtils::initialize(NPredAge,    nmfConstants::MaxNumberSpecies);
    nmfUtils::initialize(NOthPredAge, nmfConstants::MaxNumberSpecies);

    databasePtr->nmfGetMSVPAInitData(MSVPAName, MSVPA_FirstYear, MSVPA_LastYear,
                                     MSVPA_NYears, MSVPA_NSeasons);
    Forecast_NSeasons = MSVPA_NSeasons;

    // Load predator name and age lists for use later
    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "' AND Type = 0";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    NPreds = dataMap["SpeName"].size();

    for (int i = 0; i < NPreds; ++i) {
        PredList(i) = dataMap["SpeName"][i];
        fields2 = {"MaxAge"};
        queryStr2 = "SELECT MaxAge FROM Species WHERE SpeName = '" + PredList(i) + "'";
        dataMap2 = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        NPredAge(i) = std::stoi(dataMap2["MaxAge"][0]);
        PredType(i) = 0;
    } // end for i

    // Load Other Predator Names and Ages
    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "' AND Type = 3";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    NOthPreds = dataMap["SpeName"].size();

    for (int i = 0; i < NOthPreds; ++i) {
        OthPredList(i) = dataMap["SpeName"][i];
        PredType(i+NPreds) = 1;
        fields2 = {"NumSizeCats"};
        queryStr2 = "SELECT NumSizeCats FROM OtherPredSpecies WHERE SpeName = '" + OthPredList(i) + "'";
        dataMap2 = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        NOthPredAge(i) = std::stoi(dataMap2["NumSizeCats"][0]);
    } // end for i

    if (PredType(PredNum) == 0) {
        Nage = NPredAge(PredNum);
    } else {
        Nage = (NOthPredAge(PredNum - NPreds)) - 1;
    }

    std::string dummyStr = "";
    databasePtr->nmfQueryForecastPreyList(MSVPAName, ForecastName, ScenarioName,
                                          selectedSpecies, false, dummyStr, "", PreyList);
    NPrey = PreyList.size();
    nmfUtils::initialize(ChartData, Nage+1, NPrey);

    for (int i = 0; i <= Nage; ++i) {
        for (int j = 0; j < NPrey; ++j) {
            fields = {"PredName","PredAge","PreyName","Year","Season","Diet"};
            queryStr = "SELECT PredName, PredAge, PreyName, Year, Season, Sum(PropDiet) as Diet FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND ForeName = '" + ForecastName + "'" +
                    " AND Scenario = '" + ScenarioName + "'" +
                    " AND PredName = '" + selectedSpecies + "'" +
                    " AND PredAge = "   + std::to_string(i) +
                    " AND PreyName = '" + PreyList[j].toStdString() + "'" +
                    " GROUP By PredName, PredAge, PreyName, Year, Season ";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            NumRecords = dataMap["Diet"].size();
            if (NumRecords > 0) {
                m = 0;
                for (int k = 0; k<Forecast_NYears; ++k) { // RSK improve these types of loops, very inefficient
                    for (int l = 0; l < Forecast_NSeasons; ++l) {
                        if (m < NumRecords) {
                            ChartData(i,j) += std::stod(dataMap["Diet"][m++]);
                        } else
                            break; // end if
                    } // end for l
                } // end for k
                ChartData(i,j) /= (Forecast_NSeasons * (Forecast_NYears + 1));
            } // end if
        } // end j
    } // end i

    xLabel = (PredType(PredNum) == 0) ? "Age Class" : "Size Class";
    if (xLabel == "Age Class") {
        for (int ageOrSize=0; ageOrSize<=Nage; ++ageOrSize) {
            categories << "Age " + QString::number(ageOrSize);
        }
    } else {
        for (int ageOrSize=0; ageOrSize<=Nage; ++ageOrSize) {
            categories << "Size " + QString::number(ageOrSize);
        }
    }


    // Set the titles
    mainTitle = "Average " + selectedSpecies + "Diet By " + xLabel;
    yLabel = "Prop. Diet";


    // Draw the chart and set its titles.
    loadChartWithData(chart,series,PreyList,ChartData);
    setTitles(chart, series, categories, selectedSpecies, mainTitle, xLabel, yLabel);

    // Display Grid (i.e. numeric) data
    //int numDigits = 7;
    //int numDecimals = 3;
    //sendToOutputTable(chartDataTbW, categories, PreyList, ChartData, numDigits, numDecimals);

} // end Forecast_DietComposition

void nmfMainWindow::loadChartWithData(
        QChart *chart,
        QStackedBarSeries *series,
        QStringList &PreyList,
        boost::numeric::ublas::matrix<double> &ChartData)
{
    QBarSet *newSet;

    series = new QStackedBarSeries();
    for (unsigned int pred=0; pred<ChartData.size2(); ++pred) {
        newSet = new QBarSet((PreyList[pred]));
        for (unsigned int val=0; val<ChartData.size1(); ++val) {
            *newSet << ChartData(val,pred);
        }
        series->append(newSet);
    }
    chart->addSeries(series);
} // end loadChartWithData

void nmfMainWindow::setTitles(
        QChart *chart,
        QStackedBarSeries *series,
        QStringList categories,
        std::string species,
        std::string mainTitle,
        std::string xLabel,
        std::string yLabel)
{
    std::stringstream ss;
    QFont mainTitleFont = chart->titleFont();
    mainTitleFont.setPointSize(14);
    mainTitleFont.setWeight(QFont::Bold);
    chart->setTitleFont(mainTitleFont);
    chart->setTitle(mainTitle.c_str());

    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(categories);
    chart->createDefaultAxes();
    //chart->setAxisX(axis, series);
    nmfUtilsQt::setAxisX(chart,axis,series);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    QAbstractAxis *axisX = chart->axes(Qt::Horizontal).back();
    QFont titleFont = axisX->titleFont();
    titleFont.setPointSize(12);
    titleFont.setWeight(QFont::Bold);
    axisX->setTitleFont(titleFont);
    axisX->setTitleText(xLabel.c_str());
    if (categories.count() > nmfConstants::NumCategoriesForVerticalNotation)
        axis->setLabelsAngle(-90);
    else
        axis->setLabelsAngle(0);

    // Rescale vertical axis....0 to 1 in increments of 0.2
    QValueAxis *newAxisY = new QValueAxis();
    newAxisY->setTitleFont(titleFont);
    newAxisY->setTitleText(yLabel.c_str());
    newAxisY->setRange(0,1.0);
    newAxisY->setTickCount(6);
    //chart->setAxisY(newAxisY,series);
    nmfUtilsQt::setAxisY(chart,newAxisY,series);
}



void nmfMainWindow::getSelectedListWidgetItems(QListWidget *selectYearsLW,
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


void nmfMainWindow::callback_selectSpeciesChanged(QString species)
{
    QTableWidget* ChartDataTbW = m_UI->OutputWidget->findChild<QTableWidget*>("Data");

    if (selectVariableCMB->currentText().toStdString() == "Predation Mortality - 3D") {
        chartView->hide();
        loadMsvpaCharts(species.toStdString());
        draw3DCharts();
        return;
    }

    std::vector<std::string> selectedYears;
    std::string variable = selectVariableCMB->currentText().toStdString();
    std::string ageStr = selectSpeciesAgeSizeClassCMB->currentText().toStdString();
    std::string str1,str2;
    QStringList categories;
    QStringList rowLabels;
    std::vector<std::string> PreyList;
    QStringList colLabels;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;

    nmfUtils::split(ageStr," ",str1,str2);
    int PredAgeVal = std::stoi(str2);

    resetScaleWidgets(selectSeasonCB,selectSeasonLE);
    resetScaleWidgets(selectSpeciesAgeSizeClassCB,selectSpeciesAgeSizeClassLE);
    resetScaleWidgets(selectFullyRecruitedAgeCB,selectFullyRecruitedAgeLE);

    bool enablePreySpecies = ((variable == "Consumption by Prey Age")||
                              (variable == "Food Availability by Prey Age"));
    selectPreySpeciesLBL->setEnabled(enablePreySpecies);
    selectPreySpeciesCMB->setEnabled(enablePreySpecies);
    if (enablePreySpecies) {
        reloadPreySpecies(species.toStdString(),PredAgeVal);
    } else {
        selectPreySpeciesCMB->clear();
    }


    if (MSVPAName != "") {
        if (selectDataTypeCMB->currentText() == "Yield Per Recruit") {
            selectYearsLW->blockSignals(true);
            selectFullyRecruitedAgeCMB->blockSignals(true);
            selectFullyRecruitedAgeCB->blockSignals(true);
            selectFullyRecruitedAgeLE->blockSignals(true);
            reloadYearsLW(MSVPAName);
            reloadFullyRecruitedAgeComboBox(MSVPAName);
            selectYearsLW->blockSignals(false);
            selectFullyRecruitedAgeCMB->blockSignals(false);
            selectFullyRecruitedAgeCB->blockSignals(false);
            selectFullyRecruitedAgeLE->blockSignals(false);

            QString type = selectYPRAnalysisTypeCMB->currentText();

            if ((type.toStdString() == "Historical YPR") ||
                (type.toStdString() == "Historical F Benchmarks") ||
                (type.toStdString() == "Historical SSB Benchmarks"))
            {
                for (int year=FirstYear; year <= LastYear; ++year)
                    selectedYears.push_back(std::to_string(year));
            } else {
                getSelectedListWidgetItems(selectYearsLW, selectedYears);
            }

        } else {
            getSelectedListWidgetItems(selectYearsLW, selectedYears);
            reloadSpeciesAgeSizeComboBox(MSVPAName);
        }
    }

    outputChart->redrawChart("species",
                m_databasePtr, modelName(),
                chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                entityName(),  forecastName(), scenarioName(),
                             forecastFirstYear(), forecastNYears(),
                             FirstYear,
                selectDataTypeCMB->currentText().toStdString(),
                species.toStdString(), selectSpeciesCMB->currentIndex(),
                variable,
                selectByVariablesCMB->currentText().toStdString(),
                selectSeasonCMB->currentText().toStdString(),
                ageStr,
                selectPreySpeciesCMB->currentText().toStdString(),
                selectYPRAnalysisTypeCMB->currentText().toStdString(),
                selectedYears,
                selectFullyRecruitedAgeCMB->currentText().toStdString(),0.0,
                hGridLine2d, vGridLine2d,
                rowLabels, colLabels, ChartData, GridData);

    int numDigits   = 7;
    int numDecimals = 3;
    sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);

} // end callback_selectSpeciesChanged


void nmfMainWindow::reloadPreySpecies(std::string species, int PredAgeVal)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::string theModelName = modelName();

    selectPreySpeciesCMB->clear();
    fields = {"PreyName"};

    if (theModelName == "MSVPA") {
        queryStr = "SELECT DISTINCT PreyName FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND PredName = '" + species + "'" +
                " AND PredAge = "   + std::to_string(PredAgeVal) +
                " ORDER BY PreyName";
    } else if (theModelName == "Forecast") {
        queryStr = "SELECT DISTINCT PreyName FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + forecastName() + "'" +
                " AND Scenario = '" + scenarioName() + "'" +
                " AND PredName = '" + species + "'" +
                " AND PredAge = "   + std::to_string(PredAgeVal) +
                " ORDER BY PreyName";
    }

    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["PreyName"].size(); ++i) {
        selectPreySpeciesCMB->addItem(QString::fromStdString(dataMap["PreyName"][i]));
    }

} // end reloadPreySpecies



void nmfMainWindow::callback_selectVariableChanged(QString variable)
{
    enable3DControls(false);
    if (chartView3DContainer != NULL) {
        chartView3DContainer->hide();
    }
    chartView->show();

    QTableWidget* ChartDataTbW = m_UI->OutputWidget->findChild<QTableWidget*>("Data");

    std::string selectedSpecies = selectSpeciesCMB->currentText().toStdString();
    std::string PredAgeStr = selectSpeciesAgeSizeClassCMB->currentText().toStdString();
    std::string ageStr = PredAgeStr;
    int PredAgeVal = (! ageStr.empty()) ? std::stoi(ageStr.erase(0,ageStr.find(" ")+1)) : 0;
    QStringList rowLabels;
    QStringList colLabels;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    std::vector<std::string> selectedYears = {};

    resetScaleWidgets(selectSeasonCB,selectSeasonLE);
    resetScaleWidgets(selectSpeciesAgeSizeClassCB,selectSpeciesAgeSizeClassLE);

    bool enablePreySpecies = ((variable == "Consumption by Prey Age")||
                              (variable == "Food Availability by Prey Age"));

    bool isWtSpecCons = (variable == "Wt. Specific Consumption");
    selectByVariablesCMB->setEnabled(! isWtSpecCons);

    selectPreySpeciesLBL->setEnabled(enablePreySpecies);
    selectPreySpeciesCMB->setEnabled(enablePreySpecies);

    if (enablePreySpecies) {
        selectPreySpeciesCMB->blockSignals(true);
        reloadPreySpecies(selectedSpecies,PredAgeVal);
        selectPreySpeciesCMB->blockSignals(false);
    } else {
        selectPreySpeciesCMB->clear();
    }

    if (selectVariableCMB->currentText().toStdString() == "Predation Mortality - 3D") {
        chartView->hide();
        loadMsvpaCharts(selectedSpecies);
        draw3DCharts();
        return;
    }

    std::string selDataType = selectDataTypeCMB->currentText().toStdString();
    std::string selVar      = variable.toStdString();
    if (selDataType == "Multispecies Populations") {
        outputChart = new nmfOutputChartLine(m_logger);
    } else if ((selVar == "Food Availability by Prey Type") ||
               (selVar == "Food Availability by Prey Age")) {
        outputChart = new nmfOutputChartStackedBar(m_logger);
    } else {
        outputChart = new nmfOutputChartBar(m_logger);
    }
    outputChart->redrawChart("variable",
                m_databasePtr,  modelName(),
                chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                MSVPAName, forecastName(), scenarioName(),
                             forecastFirstYear(), forecastNYears(),
                             FirstYear,
                selDataType,
                selectedSpecies,
                selectSpeciesCMB->currentIndex(),
                selVar,
                selectByVariablesCMB->currentText().toStdString(),
                selectSeasonCMB->currentText().toStdString(),
                PredAgeStr,
                selectPreySpeciesCMB->currentText().toStdString(),
                "", selectedYears, "",0.0,
                hGridLine2d, vGridLine2d,
                rowLabels, colLabels, ChartData, GridData);

    int numDigits   = 7;
    int numDecimals = 3;
    sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);

} // end callback_selectVariableChanged

void nmfMainWindow::callback_selectByVariablesChanged(QString byVariables)
{
    enable3DControls(false);

    QStringList rowLabels;
    QStringList colLabels;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    QTableWidget* ChartDataTbW = m_UI->OutputWidget->findChild<QTableWidget*>("Data");

    std::vector<std::string> selectedYears = {};

    resetScaleWidgets(selectSeasonCB,selectSeasonLE);
    resetScaleWidgets(selectSpeciesAgeSizeClassCB,selectSpeciesAgeSizeClassLE);

    std::string selVar = selectVariableCMB->currentText().toStdString();

    if (selectVariableCMB->currentText() == "Consumption by Prey Age") {
        selectSpeciesAgeSizeClassCMB->setCurrentIndex(0);
    }

    outputChart->redrawChart("byVariables",
                m_databasePtr,  modelName(),
                chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                MSVPAName,  forecastName(), scenarioName(),
                             forecastFirstYear(), forecastNYears(),
                             FirstYear,
                selectDataTypeCMB->currentText().toStdString(),
                selectSpeciesCMB->currentText().toStdString(),
                             selectSpeciesCMB->currentIndex(),
                selVar,
                byVariables.toStdString(),
                selectSeasonCMB->currentText().toStdString(),
                selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                selectPreySpeciesCMB->currentText().toStdString(),
                "", selectedYears, "",0.0,
                hGridLine2d, vGridLine2d,
                rowLabels, colLabels, ChartData, GridData);

    int numDigits   = 7;
    int numDecimals = 3;
    sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);

} // end callback_selectByVariablesChanged


void nmfMainWindow::callback_selectSeasonCMBChanged(QString season)
{
    QStringList rowLabels;
    QStringList colLabels;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    QTableWidget* ChartDataTbW = m_UI->OutputWidget->findChild<QTableWidget*>("Data");

    std::vector<std::string> selectedYears = {};

    YMaxSeason = (selectSeasonCB->isChecked()) ?
                  selectSeasonLE->text().toDouble() : 0.0;

    outputChart->redrawChart("season",
                m_databasePtr,  modelName(),
                chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                MSVPAName,  forecastName(), scenarioName(),
                             forecastFirstYear(), forecastNYears(),
                             FirstYear,
                selectDataTypeCMB->currentText().toStdString(),
                selectSpeciesCMB->currentText().toStdString(),
                             selectSpeciesCMB->currentIndex(),
                selectVariableCMB->currentText().toStdString(),
                selectByVariablesCMB->currentText().toStdString(),
                season.toStdString(),
                selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                selectPreySpeciesCMB->currentText().toStdString(),
                "", selectedYears, "",YMaxSeason,
                hGridLine2d, vGridLine2d,
                rowLabels, colLabels, ChartData, GridData);

    int numDigits   = 7;
    int numDecimals = 3;
    sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);

} // end callback_selectSeasonCMBChanged

void nmfMainWindow::callback_selectSeasonLEChanged(QString text)
{
    QStringList categories;
    QStringList PreyList;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;

    std::vector<std::string> selectedYears = {};
    double validatedInput=0.0;

    // Test validity of entered value.
    if (! inputIsValid(selectSeasonLE,text,validatedInput))
        return;
    YMaxSeason = validatedInput;

    if (outputChart) {
        outputChart->redrawChart("season",
             m_databasePtr,  modelName(),
             chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
             MSVPAName, forecastName(), scenarioName(),
                                 forecastFirstYear(), forecastNYears(),
                                 FirstYear,
             selectDataTypeCMB->currentText().toStdString(),
             selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
             selectVariableCMB->currentText().toStdString(),
             selectByVariablesCMB->currentText().toStdString(),
             selectSeasonCMB->currentText().toStdString(),
             selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
             selectPreySpeciesCMB->currentText().toStdString(),
             "", selectedYears, "", YMaxSeason,
             hGridLine2d, vGridLine2d,
             categories, PreyList, ChartData, GridData);
    }
}

void nmfMainWindow::callback_selectSeasonCBChanged(int state)
{
    QStringList categories;
    QStringList PreyList;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    std::vector<std::string> selectedYears = {};

    selectSeasonLE->setEnabled(state==Qt::Checked);

    double YMax = (state==Qt::Checked) ? selectSeasonLE->text().toDouble() : 0.0;

    if (outputChart) {
        outputChart->redrawChart("season",
                                 m_databasePtr,  modelName(),
                                 chart, AllLabels,  AllComboBoxes, AllButtons, AllCheckBoxes,
                                 MSVPAName,  forecastName(), scenarioName(),
                                 forecastFirstYear(), forecastNYears(),
                                 FirstYear,
                                 selectDataTypeCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
                                 selectVariableCMB->currentText().toStdString(),
                                 selectByVariablesCMB->currentText().toStdString(),
                                 selectSeasonCMB->currentText().toStdString(),
                                 selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                                 selectPreySpeciesCMB->currentText().toStdString(),
                                 "", selectedYears, "",YMax,
                                 hGridLine2d, vGridLine2d,
                                 categories, PreyList, ChartData, GridData);
    }
}


void
nmfMainWindow::callback_horzGridLine2dCheckboxChanged(int state)
{
    if (! outputChart)
        return;
    hGridLine2d = state==Qt::Checked;
    chart->axes(Qt::Vertical).back()->setGridLineVisible(hGridLine2d);

    //outputChart->setGridLines(chart,"horizontal",(state==Qt::Checked));

} // end callback_horzGridLine2dCheckboxChanged

void
nmfMainWindow::callback_vertGridLine2dCheckboxChanged(int state)
{
    if (! outputChart)
        return;

    vGridLine2d = state==Qt::Checked;
    chart->axes(Qt::Horizontal).back()->setGridLineVisible(vGridLine2d);

    //outputChart->setGridLines(chart,"vertical",(state==Qt::Checked));

} // end callback_vertGridLine2dCheckboxChanged


//void
//nmfMainWindow::callback_horzGridLine3dCheckboxChanged(int state)
//{
//} // end callback_horzGridLine3dCheckboxChanged

//void
//nmfMainWindow::callback_vertGridLine3dCheckboxChanged(int state)
//{
//} // end callback_vertGridLine3dCheckboxChanged


void
nmfMainWindow::callback_selectFullyRecruitedAgeCheckboxChanged(int state)
{
    QStringList categories;
    QStringList PreyList;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    std::vector<std::string> selectedYears = {};
    QString type = selectYPRAnalysisTypeCMB->currentText();

    if ((type.toStdString() == "Historical YPR") ||
            (type.toStdString() == "Historical F Benchmarks") ||
            (type.toStdString() == "Historical SSB Benchmarks"))
    {
        for (int year=FirstYear; year <= LastYear; ++year)
            selectedYears.push_back(std::to_string(year));
    } else {
        getSelectedListWidgetItems(selectYearsLW, selectedYears);
    }

    selectFullyRecruitedAgeLE->setEnabled(state==Qt::Checked);

    double YMax = (state==Qt::Checked) ? selectFullyRecruitedAgeLE->text().toDouble() : 0.0;

    if (outputChart) {
        outputChart->redrawChart("fullyRecruitedAge",
             m_databasePtr,  modelName(),
             chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
             MSVPAName,  forecastName(), scenarioName(),
                                 forecastFirstYear(), forecastNYears(),
                                 FirstYear,
             selectDataTypeCMB->currentText().toStdString(),
             selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
             selectVariableCMB->currentText().toStdString(),
             selectByVariablesCMB->currentText().toStdString(),
             selectSeasonCMB->currentText().toStdString(),
             selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
             selectPreySpeciesCMB->currentText().toStdString(),
             type.toStdString(),
             selectedYears,
             selectFullyRecruitedAgeCMB->currentText().toStdString(),
             YMax,
             hGridLine2d, vGridLine2d,
             categories, PreyList, ChartData, GridData);
    }
}


bool nmfMainWindow::inputIsValid(QLineEdit *lineedit, QString text, double &validatedInput)
{
    bool validate;
    validatedInput = text.toDouble(&validate);
    if (! validate) {
        QErrorMessage validateError;
        validateError.showMessage("Input is Invalid. Please enter a valid number.");
        validateError.exec();
        lineedit->blockSignals(true);
        lineedit->clear();
        lineedit->blockSignals(false);
        return false;
    }
    return true;
}


void nmfMainWindow::callback_selectSpeciesAgeSizeLEChanged(QString text)
{
    QStringList categories;
    QStringList PreyList;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    std::vector<std::string> selectedYears = {};
    double validatedInput=0.0;
    QString type = selectYPRAnalysisTypeCMB->currentText();

    if ((type.toStdString() == "Historical YPR") ||
            (type.toStdString() == "Historical F Benchmarks") ||
            (type.toStdString() == "Historical SSB Benchmarks"))
    {
        for (int year=FirstYear; year <= LastYear; ++year)
            selectedYears.push_back(std::to_string(year));
    } else {
        getSelectedListWidgetItems(selectYearsLW, selectedYears);
    }

    // Test validity of entered value.
    if (! inputIsValid(selectSpeciesAgeSizeClassLE,text,validatedInput))
        return;
    YMaxAgeSizeClass = validatedInput;

    if (outputChart) {
        outputChart->redrawChart("speciesAgeSizeClass",
             m_databasePtr,  modelName(),
             chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
             MSVPAName,  forecastName(), scenarioName(),
                                 forecastFirstYear(), forecastNYears(),
                                 FirstYear,
             selectDataTypeCMB->currentText().toStdString(),
             selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
             selectVariableCMB->currentText().toStdString(),
             selectByVariablesCMB->currentText().toStdString(),
             selectSeasonCMB->currentText().toStdString(),
             selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
             selectPreySpeciesCMB->currentText().toStdString(),
             type.toStdString(),
             selectedYears,
             selectFullyRecruitedAgeCMB->currentText().toStdString(),
             YMaxAgeSizeClass,
             hGridLine2d, vGridLine2d,
             categories, PreyList, ChartData, GridData);
    }
}



void nmfMainWindow::callback_selectFullyRecruitedAgeLEChanged(QString text)
{
    QStringList categories;
    QStringList PreyList;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    double validatedInput = 0.0;
    std::vector<std::string> selectedYears = {};
    QString type = selectYPRAnalysisTypeCMB->currentText();

    if ((type.toStdString() == "Historical YPR") ||
            (type.toStdString() == "Historical F Benchmarks") ||
            (type.toStdString() == "Historical SSB Benchmarks"))
    {
        for (int year=FirstYear; year <= LastYear; ++year)
            selectedYears.push_back(std::to_string(year));
    } else {
        getSelectedListWidgetItems(selectYearsLW, selectedYears);
    }

    // Test validity of entered value.
    if (! inputIsValid(selectFullyRecruitedAgeLE,text,validatedInput))
        return;
    YMaxFullyRecruitedAge = validatedInput;

    if (outputChart) {
        outputChart->redrawChart("fullyRecruitedAge",
             m_databasePtr,  modelName(),
             chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
             MSVPAName, forecastName(), scenarioName(),
                                 forecastFirstYear(), forecastNYears(),
                                 FirstYear,
             selectDataTypeCMB->currentText().toStdString(),
             selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
             selectVariableCMB->currentText().toStdString(),
             selectByVariablesCMB->currentText().toStdString(),
             selectSeasonCMB->currentText().toStdString(),
             selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
             selectPreySpeciesCMB->currentText().toStdString(),
             type.toStdString(),
             selectedYears,
             selectFullyRecruitedAgeCMB->currentText().toStdString(),
             YMaxFullyRecruitedAge,
             hGridLine2d, vGridLine2d,
             categories, PreyList, ChartData, GridData);
    }
}


void nmfMainWindow::callback_selectSpeciesAgeSizeCBChanged(int state)
{
    QStringList categories;
    QStringList PreyList;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    std::vector<std::string> selectedYears = {};

    selectSpeciesAgeSizeClassLE->setEnabled(state==Qt::Checked);

    double YMax = (state==Qt::Checked) ? selectSpeciesAgeSizeClassLE->text().toDouble() : 0.0;

    if (outputChart) {
        outputChart->redrawChart("speciesAgeSizeClass",
             m_databasePtr,  modelName(),
             chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
             MSVPAName,  forecastName(), scenarioName(),
                                 forecastFirstYear(), forecastNYears(),
                                 FirstYear,
             selectDataTypeCMB->currentText().toStdString(),
             selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
             selectVariableCMB->currentText().toStdString(),
             selectByVariablesCMB->currentText().toStdString(),
             selectSeasonCMB->currentText().toStdString(),
             selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
             selectPreySpeciesCMB->currentText().toStdString(),
             "", selectedYears, "",YMax,
             hGridLine2d, vGridLine2d,
             categories, PreyList, ChartData, GridData);
    }
}

void nmfMainWindow::callback_selectPreySpeciesCMBChanged(QString preyName)
{
    QStringList rowLabels;
    QStringList colLabels;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    std::vector<std::string> selectedYears = {};
    QTableWidget* ChartDataTbW = m_UI->OutputWidget->findChild<QTableWidget*>("Data");

    YMaxAgeSizeClass = (selectSpeciesAgeSizeClassCB->isChecked()) ?
                        selectSpeciesAgeSizeClassLE->text().toDouble() : 0.0;

    outputChart->redrawChart("speciesAgeSizeClass",
                m_databasePtr, modelName(),
                chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                MSVPAName, forecastName(), scenarioName(),
                forecastFirstYear(), forecastNYears(), FirstYear,
                selectDataTypeCMB->currentText().toStdString(),
                selectSpeciesCMB->currentText().toStdString(),
                selectSpeciesCMB->currentIndex(),
                selectVariableCMB->currentText().toStdString(),
                selectByVariablesCMB->currentText().toStdString(),
                selectSeasonCMB->currentText().toStdString(),
                selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                preyName.toStdString(),
                "", selectedYears, "", YMaxAgeSizeClass,
                hGridLine2d, vGridLine2d,
                rowLabels, colLabels, ChartData, GridData);

    int numDigits   = 7;
    int numDecimals = 3;
    sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);

}

void nmfMainWindow::callback_selectSpeciesAgeSizeCMBChanged(QString speciesAgeSizeClass)
{
    QStringList rowLabels;
    QStringList colLabels;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    std::vector<std::string> selectedYears = {};
    QTableWidget* ChartDataTbW = m_UI->OutputWidget->findChild<QTableWidget*>("Data");

    YMaxAgeSizeClass = (selectSpeciesAgeSizeClassCB->isChecked()) ?
                        selectSpeciesAgeSizeClassLE->text().toDouble() : 0.0;
    std::string ageStr = speciesAgeSizeClass.toStdString();
    int PredAgeVal = std::stoi(ageStr.erase(0,ageStr.find(" ")+1));
    selectPreySpeciesCMB->blockSignals(true);
    reloadPreySpecies(selectSpeciesCMB->currentText().toStdString(),PredAgeVal);
    selectPreySpeciesCMB->blockSignals(false);
    outputChart->redrawChart("speciesAgeSizeClass",
                m_databasePtr, modelName(),
                chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                MSVPAName, forecastName(), scenarioName(),
                forecastFirstYear(), forecastNYears(), FirstYear,
                selectDataTypeCMB->currentText().toStdString(),
                selectSpeciesCMB->currentText().toStdString(),
                selectSpeciesCMB->currentIndex(),
                selectVariableCMB->currentText().toStdString(),
                selectByVariablesCMB->currentText().toStdString(),
                selectSeasonCMB->currentText().toStdString(),
                selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                selectPreySpeciesCMB->currentText().toStdString(),
                "", selectedYears, "", YMaxAgeSizeClass,
                hGridLine2d, vGridLine2d,
                rowLabels, colLabels, ChartData, GridData);

    int numDigits   = 7;
    int numDecimals = 3;
    sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);

} // end callback_selectSpeciesAgeSizeCMBChanged

void nmfMainWindow::callback_selectYPRAnalysisTypeChanged(QString type)
{
    QStringList rowLabels;
    QStringList colLabels;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    resetScaleWidgets(selectFullyRecruitedAgeCB,selectFullyRecruitedAgeLE);
    QTableWidget* ChartDataTbW = m_UI->OutputWidget->findChild<QTableWidget*>("Data");
    QTabWidget* MSVPAOutputTabWidget = m_UI->OutputWidget->findChild<QTabWidget *>("MSVPAOutputTabWidget");

    std::vector<std::string> selectedYears;

    if ((type.toStdString() == "Historical YPR") ||
        (type.toStdString() == "Historical F Benchmarks") ||
        (type.toStdString() == "Historical SSB Benchmarks"))
    {
        MSVPAOutputTabWidget->setTabEnabled(2,false);
        selectYearsLW->blockSignals(true);
        selectYearsLW->clearSelection();
        selectYearsLW->setEnabled(false);
        for (int year=FirstYear; year <= LastYear; ++year)
            selectedYears.push_back(std::to_string(year));
        selectYearsLW->blockSignals(false);
    } else {
        MSVPAOutputTabWidget->setTabEnabled(2,true);
        //selectYearsLW->setEnabled(true);
        selectYearsLW->blockSignals(true);
        selectYearsLW->setCurrentRow(0);
        selectYearsLW->setEnabled(true);
        selectYearsLW->blockSignals(false);
        getSelectedListWidgetItems(selectYearsLW, selectedYears);
    }

    if (outputChart) {
        outputChart->redrawChart("YPRAnalysisType",
             m_databasePtr, modelName(),
             chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
             MSVPAName, forecastName(), scenarioName(),
                                 forecastFirstYear(), forecastNYears(),
                                 FirstYear,
             selectDataTypeCMB->currentText().toStdString(),
             selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
             selectVariableCMB->currentText().toStdString(),
             selectByVariablesCMB->currentText().toStdString(),
             selectSeasonCMB->currentText().toStdString(),
             selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
             selectPreySpeciesCMB->currentText().toStdString(),
             type.toStdString(),
             selectedYears,
             selectFullyRecruitedAgeCMB->currentText().toStdString(),0.0,
             hGridLine2d, vGridLine2d,
             rowLabels, colLabels, ChartData, GridData);

        int numDigits   = 7;
        int numDecimals = 2;
        sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);
    }
}



void nmfMainWindow::callback_selectYearsChanged()
{
    QStringList rowLabels;
    QStringList colLabels;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    std::vector<std::string> selectedYears;
    QTableWidget* ChartDataTbW = m_UI->OutputWidget->findChild<QTableWidget*>("Data");

    getSelectedListWidgetItems(selectYearsLW, selectedYears);

    if (outputChart) {
        outputChart->redrawChart("years",
             m_databasePtr, modelName(),
             chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
             MSVPAName,  forecastName(), scenarioName(),
                                 forecastFirstYear(), forecastNYears(),
                                 FirstYear,
             selectDataTypeCMB->currentText().toStdString(),
             selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
             selectVariableCMB->currentText().toStdString(),
             selectByVariablesCMB->currentText().toStdString(),
             selectSeasonCMB->currentText().toStdString(),
             selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
             selectPreySpeciesCMB->currentText().toStdString(),
             selectYPRAnalysisTypeCMB->currentText().toStdString(),
             selectedYears,
             selectFullyRecruitedAgeCMB->currentText().toStdString(),0.0,
             hGridLine2d, vGridLine2d,
             rowLabels, colLabels, ChartData, GridData);

        int numDigits   = 7;
        int numDecimals = 2;
        sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);

    }
}

void nmfMainWindow::callback_SSVPAChartTypeChanged(QString chartType)
{
    int xmin;
    int xmax;
    int ymin;
    long ymax;
    int zmin;
    int zmax;
    int chartNum = 0;
    int MaxAge, FirstCatchAge, LastCatchAge, isPlusClass;
    bool autoScale;
    bool updateRange = false;
    double maxMatrixValue;
    int FirstCatchYear;
    int LastCatchYear;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;

    if (chartType == "Abundance") {
        chartNum = 0;
    } else if (chartType == "Fishing Mortality") {
        chartNum = 1;
    } else if (chartType == "Natural Mortality") {
        chartNum = 2;
    } else {
        return;
    }

    //  Update SSVPA charts with correct species data

    std::vector<std::string> chartNames = { "Abundance (millions) ", "Fishing Mortality", "Natural Mortality" };

    std::string SpeciesName = entityName();
    int SpeciesIndex = m_databasePtr->getSpeciesIndex(SpeciesName);
    if (SpeciesIndex < 0) {
        return;
    }

    // Get data needed for SVPA
    std::tie(MaxAge, FirstCatchAge, LastCatchAge, isPlusClass) =
            m_databasePtr->nmfQueryAgeFields("Species",SpeciesIndex);

    // Find max value in matrix for vertical scaling
    maxMatrixValue = nmfUtils::getMatrixMax(SSVPATables[chartNum],nmfConstants::RoundOff);
    outputCharts3D[0]->maxValueLE()->setText(QLocale(QLocale::English).toString(qlonglong(maxMatrixValue)));
    outputCharts3D[0]->minSliderX()->setValue(nmfConstants::MinSliderValue);
    outputCharts3D[0]->maxSliderX()->setValue(nmfConstants::MaxSliderValue);
    outputCharts3D[0]->maxSliderYScale()->setValue(nmfConstants::MaxSliderValue);
    outputCharts3D[0]->minSliderZ()->setValue(nmfConstants::MinSliderValue);
    outputCharts3D[0]->maxSliderZ()->setValue(nmfConstants::MaxSliderValue);
    autoScale = outputCharts3D[0]->scaleCB()->isChecked();

    // Load Catch matrix from table view
    // model = SSVPACatchAtAgeTV->model();
    // int nrows = model->rowCount();
    // int ncols = model->columnCount();
    // FirstCatchYear = model->data(model->index(0, 0), Qt::DisplayRole).toDouble();
    // LastCatchYear  = model->data(model->index(nrows-1, 0), Qt::DisplayRole).toDouble();
    fields   = {"SpeName","FirstYear","LastYear"};
    queryStr = "SELECT SpeName,FirstYear,LastYear FROM Species WHERE SpeIndex = " + std::to_string(SpeciesIndex);
    dataMap  = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    int NumRecords = dataMap["SpeName"].size();
    if (NumRecords == 0) {
        m_logger->logMsg(nmfConstants::Error,"Error nmfMainWindow::callback_SSVPAChartTypeChanged: Couldn't find SpeIndex: "+std::to_string(SpeciesIndex));
    }
    FirstCatchYear = std::stoi(dataMap["FirstYear"][0]);
    LastCatchYear  = std::stoi(dataMap["LastYear"][0]);

    // Update the charts with new species data
    xmin = FirstCatchYear; // year
    xmax = LastCatchYear;
    ymin = 0; // the data
    // Plot the Mortalities between 0 and 1 since they're a percentage.
    //ymax = (chartNames[chartNum] != "Abundance") ? 1 : maxMatrixValue;
    ymax = maxMatrixValue;
    zmin = 0; // age
    zmax = LastCatchAge;
    outputCharts3D[0]->setMaxValue(maxMatrixValue);
    outputCharts3D[0]->enableSSVPAModel(true, xmin, xmax, ymin, ymax,
                                            zmin, zmax, "Year", chartNames[chartNum],
                                            "Cohort Age", autoScale, updateRange);
    outputCharts3D[0]->fillSSVPAProxy(FirstCatchYear, 0, // FirstCatchAge,
                                      SSVPATables[chartNum]);

} // end callback_SSVPAChartTypeChanged


void nmfMainWindow::callback_selectFullyRecruitedAgeCMBChanged(QString age)
{
    QStringList rowLabels;
    QStringList colLabels;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    QString type = selectYPRAnalysisTypeCMB->currentText();
    std::vector<std::string> selectedYears;
    QTableWidget* ChartDataTbW = m_UI->OutputWidget->findChild<QTableWidget*>("Data");

    if ((type.toStdString() == "Historical YPR") ||
        (type.toStdString() == "Historical F Benchmarks") ||
        (type.toStdString() == "Historical SSB Benchmarks"))
    {
        for (int year=FirstYear; year <= LastYear; ++year)
            selectedYears.push_back(std::to_string(year));
    } else {
        getSelectedListWidgetItems(selectYearsLW, selectedYears);
    }

    YMaxFullyRecruitedAge = (selectFullyRecruitedAgeCB->isChecked()) ?
                             selectFullyRecruitedAgeLE->text().toDouble() : 0.0;

    if (outputChart) {
        outputChart->redrawChart("fullyRecruitedAge",
             m_databasePtr, modelName(),
             chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
             MSVPAName,  forecastName(), scenarioName(),
                                 forecastFirstYear(), forecastNYears(),
                                 FirstYear,
             selectDataTypeCMB->currentText().toStdString(),
             selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
             selectVariableCMB->currentText().toStdString(),
             selectByVariablesCMB->currentText().toStdString(),
             selectSeasonCMB->currentText().toStdString(),
             selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
             selectPreySpeciesCMB->currentText().toStdString(),
             type.toStdString(),
             selectedYears,
             age.toStdString(),
             YMaxFullyRecruitedAge,
             hGridLine2d, vGridLine2d,
             rowLabels, colLabels, ChartData, GridData);

        int numDigits   = 7;
        int numDecimals = 2;
        sendToOutputTable(ChartDataTbW, rowLabels, colLabels, GridData, numDigits, numDecimals);
    }
}


void nmfMainWindow::reloadYearsLW(std::string MSVPAName)
{
//    // Find FirstYear, LastYear, and NumSeasons
//    std::map<std::string,int> initMap =
//            databasePtr->nmfQueryInitFields("MSVPAlist", MSVPAName);
//    int FirstYear  = initMap["FirstYear"];
//    int LastYear   = initMap["LastYear"];

    std::string theModelName = modelName();
    selectYearsLW->clear();
    if (theModelName == "MSVPA") {
        for (int year=FirstYear; year<=LastYear; ++year) {
            selectYearsLW->addItem(QString::number(year));
        }
    } else if (theModelName == "Forecast") {
        int firstYear = forecastFirstYear();
        int numYears  = forecastNYears();
        for (int year=firstYear; year<=firstYear+numYears-1; ++year) {
            selectYearsLW->addItem(QString::number(year));
        }
    }
    selectYearsLW->setCurrentRow(0);

}


void nmfMainWindow::reloadFullyRecruitedAgeComboBox(std::string MSVPAName)
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    int NumAges = 0;
    std::string ageStr;

    fields = {"Age"};
    queryStr = "SELECT DISTINCT(Age) FROM MSVPASeasBiomass WHERE MSVPAName = '" + MSVPAName +
            "' and SpeName='" + selectSpeciesCMB->currentText().toStdString() + "'";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    NumAges = dataMap["Age"].size();
    selectFullyRecruitedAgeCMB->clear();
    //selectFullyRecruitedAgeLE->clear();

    for (int i=0; i<NumAges; ++i) {
        ageStr = "Age " + std::string(dataMap["Age"][i]);
        selectFullyRecruitedAgeCMB->addItem(ageStr.c_str());
    }

}


void nmfMainWindow::reloadSpeciesAgeSizeComboBox(std::string MSVPAName)
{
    int sizeOffset = 0;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::string selectedSpecies = selectSpeciesCMB->currentText().toStdString();
    std::string ageSizePrefix;
    int NageOrSizeCategories = 0;
    selectSpeciesAgeSizeClassCMB->blockSignals(true);

    selectSpeciesAgeSizeClassCMB->clear();

    ageSizePrefix = "Age";
    fields = {"SpeName"};
    queryStr = "SELECT SpeName from MSVPAspecies WHERE MSVPAName='" + MSVPAName +
               "' and SpeName='" + selectedSpecies + "' and Type=3";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["SpeName"].size() > 0) {
        if (dataMap["SpeName"][0] == selectedSpecies) {
            ageSizePrefix = "Size";
        }
    }

    if (ageSizePrefix == "Age" ) {
        fields = {"NumAges"};
        queryStr = "SELECT count(DISTINCT(Age)) as NumAges FROM MSVPASeasBiomass WHERE MSVPAName = '" + MSVPAName +
                "' and SpeName='" + selectSpeciesCMB->currentText().toStdString() + "'";
        dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
        NageOrSizeCategories  = std::stoi(dataMap["NumAges"][0]);

    } else {
        fields = {"SpeName","NumSizeCats"};
        queryStr = "SELECT SpeName,NumSizeCats from OtherPredSpecies WHERE SpeName='" + selectedSpecies + "'";
        dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["SpeName"].size() > 0) {
            if (dataMap["SpeName"][0] == selectedSpecies) {
                sizeOffset = 1;
                NageOrSizeCategories = std::stoi(dataMap["NumSizeCats"][0]);
            }
        }
    }

    std::string ageSizeText;
    for (int i=0; i<NageOrSizeCategories; ++i) {
        ageSizeText = ageSizePrefix + " " + std::to_string(i+sizeOffset);
        selectSpeciesAgeSizeClassCMB->addItem(ageSizeText.c_str());
    }

    selectSpeciesAgeSizeClassCMB->blockSignals(false);
}


void
nmfMainWindow::loadForecastInputWidgets()
{
    std::string configData="";

    if (ForecastWidgetsLoaded)
        return;

    std::string queryStr = "";
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

//    // Load Von Bert data from table
//    std::string ForecastName = forecastName();
//    fields = {"PredName","Linf","GrowthK","TZero","LWAlpha","LWBeta"};
//    queryStr = "SELECT PredName,Linf,GrowthK,TZero,LWAlpha,LWBeta FROM ForePredVonB WHERE MSVPAname = '" + MSVPAName + "'" +
//               " AND ForeName = '" + ForecastName + "'" +
//               " ORDER BY PredIndex";
//    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
//    int NumRecords = dataMap["Linf"].size();
//    // Load Von Bert model
//    vonBert_model.removeRows(0, vonBert_model.count(), QModelIndex());
//    for (unsigned int i=0; i<NumRecords; ++i) {
//        vonBert_model.append(nmfVonBert {
//             QString::fromStdString(dataMap["PredName"][i]),
//             std::stod(dataMap["Linf"][i]),
//             std::stod(dataMap["GrowthK"][i]),
//             std::stod(dataMap["TZero"][i]),
//             std::stod(dataMap["LWAlpha"][i]),
//             std::stod(dataMap["LWBeta"][i]) });
//    }
//    Forecast_Tab2_GrowthParametersTV->resizeColumnsToContents();
//    Forecast_Tab2_GrowthParametersTV->scrollToTop();

    Forecast_Tab2_ptr->loadWidgets(
                m_databasePtr,
                vonBert_model,
                MSVPAName,
                forecastName());


    // Get MSVPA info
    fields = {"FirstYear","LastYear"};
    queryStr = "SELECT FirstYear,LastYear FROM MSVPAlist WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    int FirstYear = std::stod(dataMap["FirstYear"][0]);
    int LastYear  = std::stod(dataMap["LastYear"][0]);
    int NYears = LastYear - FirstYear;
    QStringList headerNames;



//      Eventually move this into Forecast_Tab3_ptr->


    // Load Forecast_Tab3_DataTbW
    QTableWidget *Forecast_Tab3_DataTbW;
    Forecast_Tab3_DataTbW = m_UI->ForecastInputTabWidget->findChild<QTableWidget *>("Forecast_Tab3_DataTbW");
    Forecast_Tab3_DataTbW->setRowCount(NYears);
    headerNames << tr("Year") << tr("SSB (000 mt)") << tr("Recruits (000 000)")
                << tr("Predicted R") << tr("Residual");
    Forecast_Tab3_DataTbW->setColumnCount(headerNames.size());
    Forecast_Tab3_DataTbW->setHorizontalHeaderLabels(headerNames);
    Forecast_Tab3_DataTbW->verticalHeader()->hide();
    for (int i=0; i<NYears; ++i) {
        Forecast_Tab3_DataTbW->setItem(i,0,new QTableWidgetItem(QString::number(FirstYear+i)));
    }
    Forecast_Tab3_DataTbW->resizeColumnsToContents();
    Forecast_Tab3_ptr->loadWidgets(m_databasePtr,
                                   MSVPAName,
                                   forecastName(),
                                   NYears,
                                   FirstYear);
    Forecast_Tab4_ptr->load(m_databasePtr,
                            MSVPAName,
                            forecastName(),
                            forecastFirstYear(),
                            forecastNYears(),
                            FirstYear,
                            LastYear);

    configData = Forecast_Tab4_ptr->getConfigData();

    fields = {"MSVPAName","ForeName","InitYear","NYears","Growth"};
    queryStr = "SELECT MSVPAName,ForeName,InitYear,NYears,Growth FROM Forecasts WHERE MSVPAName='" + MSVPAName + "'" +
            " AND ForeName='" + forecastName() + "'";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["MSVPAName"].size() > 0) {
        Forecast_Tab5_ptr->loadWidgets(MSVPAName,
                                       forecastName(),
                                       scenarioName(),
                                       std::stoi(dataMap["InitYear"][0]),
                                       std::stoi(dataMap["NYears"][0]),
                                       std::stoi(dataMap["Growth"][0]),
                                       configData);
    }

    ForecastWidgetsLoaded = true;
} // end loadForecastInputWidgets


void nmfMainWindow::loadForecastChartWidgets() {
    std::string dataType;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::string MSVPAName = entityName();
    QStringList speciesList;
    if (MSVPAName == "")
        return;

    selectByVariablesCMB->setEnabled(true);
    selectSpeciesCMB->blockSignals(true);
    selectVariableCMB->blockSignals(true);
    selectByVariablesCMB->blockSignals(true);
    selectSeasonCMB->blockSignals(true);
    selectSpeciesAgeSizeClassCMB->blockSignals(true);
    selectYPRAnalysisTypeCMB->blockSignals(true);
    selectYearsLW->blockSignals(true);
    selectFullyRecruitedAgeCMB->blockSignals(true);
    selectFullyRecruitedAgeCB->blockSignals(true);
    selectFullyRecruitedAgeLE->blockSignals(true);

    // Load "Select Species" combo box
    selectSpeciesCMB->clear();
    dataType = selectDataTypeCMB->currentText().toStdString();
    selectSpeciesCMB->setInsertPolicy(QComboBox::InsertAlphabetically);
    if ((dataType == "Diet Composition") ||
        (dataType == "Consumption Rates") ||
        (dataType == "Food Availability") ||
        (dataType == "Growth"))
    {
        // Load Type=0 species from MSVPAspecies table
        fields = {"SpeName"};
        queryStr = "SELECT SpeName FROM MSVPAspecies where MSVPAName='" + MSVPAName +
                   "' and Type = 0";
        dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
        selectSpeciesCMB->setInsertPolicy(QComboBox::InsertAlphabetically);
        for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
            speciesList << QString::fromStdString(dataMap["SpeName"][i]);
        }

        // Then also load species from OtherPredSpecies
        queryStr = "SELECT SpeName FROM OtherPredSpecies";
        dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
        for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
            speciesList << QString::fromStdString(dataMap["SpeName"][i]);
        }
        speciesList.sort();
        selectSpeciesCMB->addItems(speciesList);
    } else if ((dataType == "Population Size")  ||
               (dataType == "Mortality Rates")  ||
               (dataType == "Yield Per Recruit"))
    {
        fields = {"SpeName"};
        queryStr = "SELECT SpeName FROM Species";
        dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
        for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
            selectSpeciesCMB->addItem(QString::fromStdString((dataMap["SpeName"][i])));
        }
    }

    // Load Select Variable combo box
    selectVariableCMB->clear();
    if (dataType == "Diet Composition") {
        selectVariableCMB->addItem("Prey Type");
    } else if (dataType == "Population Size") {
        selectVariableCMB->addItem("Total Biomass");
        selectVariableCMB->addItem("Biomass by Age");
        selectVariableCMB->addItem("Spawning Stock Biomass");
        selectVariableCMB->addItem("Total Abundance");
        selectVariableCMB->addItem("Abundance by Age");
        selectVariableCMB->addItem("Recruit Abundance");
        selectVariableCMB->addItem("Recruit Biomass");
    } else if (dataType == "Mortality Rates") {
        selectVariableCMB->addItem("Total Fishery Catch (Numbers)");
        selectVariableCMB->addItem("Fishery Catch at Age (Numbers)");
        selectVariableCMB->addItem("Total Fishery Yield (Biomass)");
        selectVariableCMB->addItem("Fishery Yield at Age (Biomass)");
        selectVariableCMB->addItem("Fishery Mortality Rate at Age");
        selectVariableCMB->addItem("Average Recruited F");
        selectVariableCMB->addItem("Predation Mortality Rate");
        selectVariableCMB->addItem("Predation Mortality by Predator");
    } else if (dataType == "Consumption Rates") {
        selectVariableCMB->addItem("Total Biomass Consumed");
        selectVariableCMB->addItem("Consumption by Predator Age");
        selectVariableCMB->addItem("Consumption by Prey Type");
        selectVariableCMB->addItem("Cons. by Pred. Age and Prey Type");
        selectVariableCMB->addItem("Consumption by Prey Age");
        selectVariableCMB->addItem("Wt. Specific Consumption");
    } else if (dataType == "Yield Per Recruit") {
        selectYPRAnalysisTypeCMB->clear();
        selectYPRAnalysisTypeCMB->addItem("YPR vs. F");
        selectYPRAnalysisTypeCMB->addItem("SSB vs. F");
        selectYPRAnalysisTypeCMB->addItem("Projected YPR");
        selectYPRAnalysisTypeCMB->addItem("Projected F Benchmarks");
        selectYPRAnalysisTypeCMB->addItem("Projected SSB Benchmarks");
        reloadYearsLW(MSVPAName);
        reloadFullyRecruitedAgeComboBox(MSVPAName);
    } else if (dataType == "Food Availability") {
        selectVariableCMB->addItem("Relative Food Availability");
        selectVariableCMB->addItem("Per Capita Food Availability");
        selectVariableCMB->addItem("Food Availability by Prey Type");
        selectVariableCMB->addItem("Food Availability by Prey Age");
        selectVariableCMB->addItem("Stomach Content Weight");
        if (forecastIsGrowthModel()) {
            selectVariableCMB->addItem("Caloric Content of Diet");
            selectVariableCMB->addItem("Caloric Content by Prey Type");
        }
    } else if (dataType == "Multispecies Populations") {
        selectVariableCMB->addItem("Total Biomass");
        selectVariableCMB->addItem("Total Abundance");
        selectVariableCMB->addItem("Age 1+ Biomass");
        selectVariableCMB->addItem("Age 1+ Abundance");
        selectVariableCMB->addItem("Spawning Stock Biomass");
    } else if (dataType == "Growth") {
        selectVariableCMB->addItem("Average Weight");
        selectVariableCMB->addItem("Average Size");
        selectVariableCMB->addItem("Weight at Age");
        selectVariableCMB->addItem("Size at Age");
    }

    // Load Select By Variable combobox
    selectByVariablesCMB->clear();
    if (dataType == "Diet Composition") {
        selectByVariablesCMB->addItem("Average");
        selectByVariablesCMB->addItem("Average by Year");
        selectByVariablesCMB->addItem("Average by Season");
        selectByVariablesCMB->addItem("By Year and Season");
    } else if ((dataType == "Population Size") ||
               (dataType == "Mortality Rates") ||
               (dataType == "Consumption Rates") ||
               (dataType == "Food Availability") ||
               (dataType == "Multispecies Populations") ||
               (dataType == "Growth")) {
        selectByVariablesCMB->addItem("Annual");
        selectByVariablesCMB->addItem("Seasonal");
    }

    // Load Select Season combo box
    selectSeasonCMB->clear();
    std::map<std::string,int> initMap =
            m_databasePtr->nmfQueryInitFields("MSVPAlist", MSVPAName);
    int NumSeasons = initMap["NSeasons"];
    std::string seasonText;
    for (int i=0; i<NumSeasons; ++i) {
        seasonText = "Season " + std::to_string(i+1);
        selectSeasonCMB->addItem(seasonText.c_str());
    }

    // Load Select Species Age/Size Class combo box
    reloadSpeciesAgeSizeComboBox(MSVPAName);

    // Set initial state of two combo boxes
    selectSeasonCMB->setEnabled(false);
    selectSeasonCB->setEnabled(false);
    selectSeasonLE->setEnabled(false);
    flipLeftPB->setEnabled(false);
    flipRightPB->setEnabled(false);
    selectSpeciesAgeSizeClassLBL->setEnabled(false);
    selectSpeciesAgeSizeClassCMB->setEnabled(false);
    selectSpeciesAgeSizeClassCB->setEnabled(false);
    selectSpeciesAgeSizeClassLE->setEnabled(false);

    selectSpeciesCMB->blockSignals(false);
    selectVariableCMB->blockSignals(false);
    selectByVariablesCMB->blockSignals(false);
    selectSeasonCMB->blockSignals(false);
    selectSpeciesAgeSizeClassCMB->blockSignals(false);

    selectYPRAnalysisTypeCMB->blockSignals(false);
    selectYearsLW->blockSignals(false);
    selectFullyRecruitedAgeCMB->blockSignals(false);
    selectFullyRecruitedAgeCB->blockSignals(false);
    selectFullyRecruitedAgeLE->blockSignals(false);

} // end loadForecastChartWidgets


void nmfMainWindow::loadMSVPAChartWidgets() {
    std::string dataType;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    QStringList speciesList;
    if (MSVPAName == "")
        return;

    selectSpeciesCMB->blockSignals(true);
    selectVariableCMB->blockSignals(true);
    selectByVariablesCMB->blockSignals(true);
    selectSeasonCMB->blockSignals(true);
    selectSpeciesAgeSizeClassCMB->blockSignals(true);
    selectYPRAnalysisTypeCMB->blockSignals(true);
    selectYearsLW->blockSignals(true);
    selectFullyRecruitedAgeCMB->blockSignals(true);
    selectFullyRecruitedAgeCB->blockSignals(true);
    selectFullyRecruitedAgeLE->blockSignals(true);

    // Load "Select Predator" combo box
    selectSpeciesCMB->clear();
    dataType = selectDataTypeCMB->currentText().toStdString();
    if ((dataType == "Diet Composition")  ||
        (dataType == "Consumption Rates") ||
        (dataType == "Food Availability") ||
        (dataType == "Growth"))
    {
        // Load Type=0 species from MSVPAspecies table
        fields = {"SpeName"};
        queryStr = "SELECT SpeName FROM MSVPAspecies where MSVPAName='" + MSVPAName +
                   "' and Type = 0";
        dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
        for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
           speciesList << QString::fromStdString(dataMap["SpeName"][i]);
        }

        // Then also load species from OtherPredSpecies
        queryStr = "SELECT SpeName FROM OtherPredSpecies";
        dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
        for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
          speciesList << QString::fromStdString(dataMap["SpeName"][i]);
        }
        speciesList.sort();
        selectSpeciesCMB->addItems(speciesList);
    } else if ((dataType == "Population Size")  ||
               (dataType == "Mortality Rates")  ||
               (dataType == "Yield Per Recruit"))
    {
        fields = {"SpeName"};
        queryStr = "SELECT SpeName FROM Species";
        dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
        for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
            selectSpeciesCMB->addItem(QString::fromStdString((dataMap["SpeName"][i])));
        }
    }

    // Load Select Variable combo box
    selectVariableCMB->clear();
    if (dataType == "Diet Composition") {
        selectVariableCMB->addItem("Prey Type");
    } else if (dataType == "Population Size") {
        selectVariableCMB->addItem("Total Biomass");
        selectVariableCMB->addItem("Biomass by Age");
        selectVariableCMB->addItem("Spawning Stock Biomass");
        selectVariableCMB->addItem("Total Abundance");
        selectVariableCMB->addItem("Abundance by Age");
        selectVariableCMB->addItem("Recruit Abundance");
        selectVariableCMB->addItem("Recruit Biomass");
    } else if (dataType == "Mortality Rates") {
        selectVariableCMB->addItem("Fishing Mortality");
        selectVariableCMB->addItem("Average Recruited F");
        selectVariableCMB->addItem("Predation Mortality");
        selectVariableCMB->addItem("Predation Mortality - 3D");
        selectVariableCMB->addItem("Predation Mortality by Predator");
    } else if (dataType == "Consumption Rates") {
        selectVariableCMB->addItem("Total Biomass Consumed");
        selectVariableCMB->addItem("Consumption by Predator Age");
        selectVariableCMB->addItem("Consumption by Prey Type");
        selectVariableCMB->addItem("Cons. by Pred. Age and Prey Type");
        selectVariableCMB->addItem("Consumption by Prey Age");
        selectVariableCMB->addItem("Wt. Specific Consumption");
    } else if (dataType == "Yield Per Recruit") {
        selectYPRAnalysisTypeCMB->clear();
        selectYPRAnalysisTypeCMB->addItem("YPR vs. F");
        selectYPRAnalysisTypeCMB->addItem("SSB vs. F");
        selectYPRAnalysisTypeCMB->addItem("Historical YPR");
        selectYPRAnalysisTypeCMB->addItem("Historical F Benchmarks");
        selectYPRAnalysisTypeCMB->addItem("Historical SSB Benchmarks");
        reloadYearsLW(MSVPAName);
        reloadFullyRecruitedAgeComboBox(MSVPAName);
    } else if (dataType == "Food Availability") {
        selectVariableCMB->addItem("Relative Food Availability");
        selectVariableCMB->addItem("Per Capita Food Availability");
        selectVariableCMB->addItem("Food Availability by Prey Type");
        selectVariableCMB->addItem("Food Availability by Prey Age");
        selectVariableCMB->addItem("Stomach Content Weight");
    } else if (dataType == "Multispecies Populations") {
        selectVariableCMB->addItem("Total Biomass");
        selectVariableCMB->addItem("Total Abundance");
        selectVariableCMB->addItem("Age 1+ Biomass");
        selectVariableCMB->addItem("Age 1+ Abundance");
        selectVariableCMB->addItem("Spawning Stock Biomass");
    } else if (dataType == "Growth") {
        selectVariableCMB->addItem("Average Weight");
        selectVariableCMB->addItem("Average Size");
        selectVariableCMB->addItem("Weight at Age");
        selectVariableCMB->addItem("Size at Age");
    }

    // Load Select By Variable combobox
    selectByVariablesCMB->clear();
    if (dataType == "Diet Composition") {
        selectByVariablesCMB->addItem("Average");
        selectByVariablesCMB->addItem("Average by Year");
        selectByVariablesCMB->addItem("Average by Season");
        selectByVariablesCMB->addItem("By Year and Season");
    } else if ((dataType == "Population Size") ||
               (dataType == "Mortality Rates") ||
               (dataType == "Consumption Rates") ||
               (dataType == "Food Availability") ||
               (dataType == "Multispecies Populations") ||
               (dataType == "Growth")) {
        selectByVariablesCMB->addItem("Annual");
        selectByVariablesCMB->addItem("Seasonal");
    }

    // Load Select Season combo box
    selectSeasonCMB->clear();
    std::map<std::string,int> initMap =
            m_databasePtr->nmfQueryInitFields("MSVPAlist", MSVPAName);
    int NumSeasons = initMap["NSeasons"];

    std::string seasonText;
    for (int i=0; i<NumSeasons; ++i) {
        seasonText = "Season " + std::to_string(i+1);
        selectSeasonCMB->addItem(seasonText.c_str());
    }

    // Load Select Species Age/Size Class combo box
    reloadSpeciesAgeSizeComboBox(MSVPAName);

    // Set initial state of two combo boxes
    selectSeasonCMB->setEnabled(false);
    selectSeasonCB->setEnabled(false);
    selectSeasonLE->setEnabled(false);
    flipLeftPB->setEnabled(false);
    flipRightPB->setEnabled(false);
    selectSpeciesAgeSizeClassLBL->setEnabled(false);
    selectSpeciesAgeSizeClassCMB->setEnabled(false);
    selectSpeciesAgeSizeClassCB->setEnabled(false);
    selectSpeciesAgeSizeClassLE->setEnabled(false);

    selectSpeciesCMB->blockSignals(false);
    selectVariableCMB->blockSignals(false);
    selectByVariablesCMB->blockSignals(false);
    selectSeasonCMB->blockSignals(false);
    selectSpeciesAgeSizeClassCMB->blockSignals(false);

    selectYPRAnalysisTypeCMB->blockSignals(false);
    selectYearsLW->blockSignals(false);
    selectFullyRecruitedAgeCMB->blockSignals(false);
    selectFullyRecruitedAgeCB->blockSignals(false);
    selectFullyRecruitedAgeLE->blockSignals(false);

} // end loadMSVPAChartWidgets



bool nmfMainWindow::isFixedPitch(const QFont & font) {
    const QFontInfo fi(font);
    //qDebug() << fi.family() << fi.fixedPitch();
    return fi.fixedPitch();
}

QFont nmfMainWindow::getMonospaceFont()
{
    QFont font("monospace");
    if (isFixedPitch(font)) return font;

    font.setStyleHint(QFont::Monospace);
    if (isFixedPitch(font)) return font;

    font.setStyleHint(QFont::TypeWriter);
    if (isFixedPitch(font)) return font;

    font.setFamily("courier");
    if (isFixedPitch(font)) return font;

    return font;
}


void nmfMainWindow::setupOutputWindows(QTextEdit *textEdit)
{
    QFont monoFont = getMonospaceFont();
    monoFont.setPointSize(10);
    textEdit->setFont(monoFont);
    textEdit->setText("");
    textEdit->setReadOnly(true);

    //textEdit->hide(); // RSK remove
}

void nmfMainWindow::updateModel() {
    callback_RunSSVPA(SelectVPATypeCMB->currentText().toStdString());
}

void nmfMainWindow::updateModel(int state) {
    callback_RunSSVPA(SelectVPATypeCMB->currentText().toStdString());
}

void nmfMainWindow::updateModel(double value) {
    callback_RunSSVPA(SelectVPATypeCMB->currentText().toStdString());
}

void nmfMainWindow::updateModel(const QModelIndex &topLeft,
        const QModelIndex &bottomRight) {
    callback_RunSSVPA(SelectVPATypeCMB->currentText().toStdString());
}


void nmfMainWindow::update() {
    qDebug() << "Update";

    QModelIndexList selection = m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPACatchAtAgeTV")->selectionModel()->selectedColumns();
    if (!selection.isEmpty()) {
        int column = selection[0].column();

        qDebug() << "column:" << column;
        float datum = m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPACatchAtAgeTV")->model()->data(selection[0]).toFloat();
        qDebug() << "data:" << datum;
        datum += 100.0;

//         // This is pretty slow
//         char buffer [1000];
//         int n;
//         n=sprintf (buffer, "update SpeCatch set Catch=%f where SpeIndex=1 and Year=1982 and Age=1;", datum);
//         std::string cmd(buffer);
//         qDebug() << "cmd: " << cmd.c_str();
//         databasePtr->nmfUpdateDatabase(cmd);

        m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPACatchAtAgeTV")->model()->setData(selection[0], 100);

        //SSVPACatchAtAgeTV->model()->item(1,1)->setData("10000",Qt::DisplayRole);

//         SSVPACatchAtAgeTV->model()->setData(selection[0],"10000");
//         QModelIndex startOfRow = SSVPACatchAtAgeTV->model()->index(0, 0);
//         QModelIndex endOfRow   = SSVPACatchAtAgeTV->model()->index(0, 4);
//         emit QAbstractItemModel::dataChanged(startOfRow,endOfRow);
        //QList<QStandardItem*> itemList = SSVPACatchAtAgeTV->model()->takeRow(row);
        //SSVPACatchAtAgeTV->model()->moveRow(row-1,itemList);
    }


}

void
nmfMainWindow::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Delete) ||
        (event->key() == Qt::Key_Backspace))
    {
        menu_clear();

    }

} // end keyPressEvent



double
nmfMainWindow::getValueAsDouble(QLineEdit* widget) {
    return widget->text().toDouble();
}

int
nmfMainWindow::getValueAsInt(QLineEdit* widget) {
    return widget->text().toInt();
}

int
nmfMainWindow::currentEntity() {
    //QModelIndexList speciesList = EntityListLV->selectionModel()->selectedIndexes();
    //return speciesList[0].row()+1;
    QModelIndex mindex = EntityListLV->currentIndex();
    return mindex.row() + 1;
}

std::string
nmfMainWindow::entityName() {
    QModelIndex mindex = EntityListLV->currentIndex();
    return mindex.data().toString().toStdString();
    //QModelIndexList speciesList = EntityListLV->selectionModel()->selectedIndexes();
    //return speciesList[0].data(Qt::DisplayRole).toString().toStdString();
}


std::string
nmfMainWindow::modelName()
{
    QTreeWidgetItem *item;

    // Works only if NavigatorTree is 2 levels deep, which it is.
    if (! NavigatorTree->currentItem()->parent())
        item = NavigatorTree->currentItem();
    else
        item = NavigatorTree->currentItem()->parent();

   return item->text(0).toStdString();

}

int nmfMainWindow::ssvpaConfigurationIndex() {

    return SelectConfigurationCMB->currentIndex() + 1;
}

std::string nmfMainWindow::ssvpaConfigurationName() {

    return SelectConfigurationCMB->currentText().toStdString();
}

//nmfData* nmfMainWindow::appData() {
//    return data;
//}

//void nmfMainWindow::setAppData(nmfData* dataArg) {
//    data = dataArg;
//}


void nmfMainWindow::callback_scenarioSingleClicked(const QModelIndex &curr)
{

    updateMainWindowTitle("Forecast: "+MSVPAName+", "+forecastName()+", "+scenarioName());

    //Forecast_Tab1_ScenarioNameLE->setText(curr.data().toString());

    // Find what the current selected data type is.
    QString currentDataType = selectDataTypeCMB->currentText();

    // Enable button(s)
    //RunForecastPB->setEnabled(true);

    // Load output charts
    loadForecastChartWidgets();

    // Load input widgets
    loadForecastInputWidgets();
    //Forecast_Tab2_ptr->loadWidgets();

    // Force a refresh of the current data type since we've switched to MSVPA mode
    callback_selectDataTypeChanged(currentDataType);

    m_UI->ForecastInputTabWidget->findChild<QLineEdit *>("Forecast_Tab1_ScenarioNameLE")->setText(QString::fromStdString(scenarioName()));
    Forecast_Tab4_ptr->ScenarioName = scenarioName();
    Forecast_Tab1_ptr->refresh(MSVPAName,forecastName(),scenarioName());

    // Initialize the output gui controls and chart with db ptr and MSVPA name
    outputWidget->setDatabaseVars(m_databasePtr,
                                  m_logger, entityName(), "Forecast",
                                  forecastName(), scenarioName());
}


void nmfMainWindow::callback_forecastSingleClicked(const QModelIndex &curr)
{

    //RunForecastPB->setEnabled(false);

    std::string currForecast = curr.data().toString().toStdString();

    reloadForecastWidgets(currForecast);

} // end callback_forecastSingleClicked


void nmfMainWindow::reloadForecastWidgets(std::string Forecast)
{
    std::string queryStr = "";
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string configData="";

    // Load Scenario list
    scenario_model.removeRows(0, scenario_model.count(), QModelIndex());
    fields = {"Scenario"};
    queryStr = "SELECT Scenario FROM Scenarios WHERE MSVPAName='" + entityName() + "'" +
            " AND ForeName='" + Forecast + "'";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    for (auto name : dataMap[fields[0]]) {
        scenario_model.append( QString::fromStdString(name));
    }
    ScenarioListLV->scrollToTop();

    if (! queryAndLoadInitFields())
        return;

    Forecast_Tab1_ptr->loadWidgets(m_databasePtr,
                                   entityName(),
                                   Forecast,
                                   FirstYear,
                                   LastYear);

    Forecast_Tab2_ptr->loadWidgets(
                m_databasePtr,
                vonBert_model,
                MSVPAName,
                Forecast);

    Forecast_Tab4_ptr->load(m_databasePtr,
                            MSVPAName,
                            Forecast,
                            forecastFirstYear(),
                            forecastNYears(),
                            FirstYear, LastYear);
    configData = Forecast_Tab4_ptr->getConfigData();


    Forecast_Tab1_ptr->refresh(MSVPAName,Forecast,scenarioName());

    fields = {"MSVPAName","ForeName","InitYear","NYears","Growth"};
    queryStr = "SELECT MSVPAName,ForeName,InitYear,NYears,Growth FROM Forecasts WHERE MSVPAName='" + MSVPAName + "'" +
            " AND ForeName='" + Forecast + "'";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["MSVPAName"].size() > 0) {
        Forecast_Tab5_ptr->loadWidgets(MSVPAName,
                                       Forecast,
                                       scenarioName(),
                                       std::stoi(dataMap["InitYear"][0]),
                                       std::stoi(dataMap["NYears"][0]),
                                       std::stoi(dataMap["Growth"][0]),
                                       configData);
    }
} // end reloadForecastWidgets


void nmfMainWindow::callback_forecastDoubleClicked(const QModelIndex &curr)
{

std::cout << "Not yet implemented." << std::endl;

}


void nmfMainWindow::callback_MSVPASingleClickedGUI(const QModelIndex &curr)
{
std::cout << "callback_MSVPASingleClickedGUI" << std::endl;

    MSVPAName = curr.data(Qt::DisplayRole).toString().toStdString();
    outputWidget->setDatabaseVars(m_databasePtr,
                                  m_logger, MSVPAName, "MSVPA",
                                  forecastName(), scenarioName());

    updateMainWindowTitle("MSVPA: " + MSVPAName);

    // Find what the current selected data type is.
    QString currentDataType = selectDataTypeCMB->currentText();

    loadMSVPAInputWidgets();
    loadMSVPAChartWidgets();

    // Force a refresh of the current data type since we've switched to MSVPA mode
    callback_selectDataTypeChanged(currentDataType);

    // User just clicked on a new MSVPA; select first MSVPA Nav Tree item and corresponding Tab
    m_UI->MSVPAInputTabWidget->setTabEnabled(0,true);
    m_UI->MSVPAInputTabWidget->setCurrentIndex(0);

    MSVPA_Tab1_ptr->loadWidgets( m_databasePtr,
                                 MSVPAName );

    // Enable the appropriate Tab pages per the MSVPA config just selected.
    MSVPAFindStoreAndSetPageStates();

    // Catch the Ctrl+Click and do a Run
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) == true) {
        std::cout << "Control key held down..." << std::endl;
        QApplication::keyboardModifiers().setFlag(Qt::NoModifier);

        // Run MSVPA
        callback_RunMSVPA();
    }

} // end callback_MSVPASingleClickedGUI


void nmfMainWindow::callback_MSVPASingleClicked(const QModelIndex &curr)
{
    updateMainWindowTitle("MSVPA: " + curr.data(Qt::DisplayRole).toString().toStdString());

    // Find what the current selected data type is.
    QString currentDataType = selectDataTypeCMB->currentText();

    loadMSVPAInputWidgets();
    loadMSVPAChartWidgets();

    // Force a refresh of the current data type since we've switched to MSVPA mode
    callback_selectDataTypeChanged(currentDataType);

    if (EntityListLV->model()->rowCount() == 0) {
        menu_newMSVPA();
    }


    // Find and Store all the MSVPA Tab states (i.e. true for enabled, false for not)
    //MSVPAFindStoreAndSetPageStates();

} // end callback_MSVPASingleClicked


void nmfMainWindow::callback_msvpaForecastSingleClicked(const QModelIndex &curr)
{

    std::string queryStr = "";
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    clearForecastWidgets();

    std::string currMSVPA = curr.data().toString().toStdString();
    m_UI->ForecastInputTabWidget->findChild<QLineEdit *>("Forecast_Tab1_MsvpaNameLE")->setText(QString::fromStdString(currMSVPA));

    // If user is in Forecast mode, load the Forecast widget.
    if (modelName() == "Forecast") {
        // Remove previous model data and read data from database into model
        forecast_model.removeRows(0, forecast_model.count(), QModelIndex());
        fields = {"ForeName"};
        queryStr = "SELECT ForeName FROM Forecasts WHERE MSVPAName='" + currMSVPA + "';";
        dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
        for (auto name : dataMap[fields[0]]) {
            forecast_model.append( QString::fromStdString(name));
        }
        ForecastListLV->scrollToTop();
    }

}

void nmfMainWindow::loadForecastListWidget()
{
    std::string queryStr = "";
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    // Remove previous model data and read data from database into model
    forecast_model.removeRows(0, forecast_model.count(), QModelIndex());
    fields = {"ForeName"};
    queryStr = "SELECT ForeName FROM Forecasts WHERE MSVPAName='" + MSVPAName + "';";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    for (auto name : dataMap[fields[0]]) {
        forecast_model.append( QString::fromStdString(name));
    }
    ForecastListLV->scrollToTop();
    ForecastListLV->setCurrentIndex(forecast_model.index(0,0));
    ForecastListLV->clearFocus();

} // end loadForecastListWidget



bool
nmfMainWindow::queryAndLoadInitFields()
{
    bool ok=true;
    MSVPAName = entityName();

    std::map<std::string,int> initMap =
            m_databasePtr->nmfQueryInitFields("MSVPAlist", MSVPAName);
    FirstYear     = initMap["FirstYear"];
    LastYear      = initMap["LastYear"];
    NumSeasons    = initMap["NSeasons"];
    isGrowthModel = initMap["GrowthModel"];

    ok = ((FirstYear != 0) && (LastYear != 0) && (NumSeasons != 0));
    if (! ok) {
        QMessageBox::information(this,
             tr("Incomplete MSVPA"),
             tr("\nMissing MSVPA Year and/or NumSeasons data.\nPlease complete setup process for the selected MSVPA Configuration on MSVPA Tab #2."),
             QMessageBox::Ok);
    }

    return ok;
} // end queryAndLoadInitFields


//void nmfMainWindow::disableSSVPAMenuAndTabItems()
//{
//    RunSSVPAPB->setEnabled(false);
//} // end disableSSVPAMenuAndTabItems



// This function determines which MSVPA pages have already been completed
//  and sets an array accordingly.  The array is used to set the enabled
//  state of the MSVPA sub items in the Navigational Tree so that the user
//  can see where they left off.

void
nmfMainWindow::MSVPAFindStoreAndSetPageStates()
{
    bool pageEnabled;
    int page=0;

    m_logger->logMsg(nmfConstants::Normal,"nmfMainWindow::MSVPAFindStoreAndSetPageStates");

    std::vector<std::string> fields,fields1,fields2;
    std::map<std::string, std::vector<std::string> > dataMap,dataMap1,dataMap2;
    std::string queryStr,queryStr1,queryStr2;

    for (unsigned int i=0;i<MSVPAPageEnabled.size();++i) {
        MSVPAPageEnabled[i] = false;
    }
    //RunMSVPAPB->setEnabled(false);


    // Tab 1
    fields    = {"MSVPAName", "SpeName"};
    queryStr  = "SELECT MSVPAName,SpeName FROM MSVPAspecies ";
    queryStr += "WHERE MSVPAName = '" + MSVPAName + "'";
    dataMap   = m_databasePtr->nmfQueryDatabase(queryStr,fields);
    MSVPAPageEnabled[page++] = (dataMap["MSVPAName"].size() > 0);

    // Tab 2
    fields     = {"MSVPAName", "NSeasons"};
    queryStr   =  "SELECT MSVPAName,NSeasons FROM MSVPAlist ";
    queryStr  +=  "WHERE MSVPAName = '" + MSVPAName + "'";
    dataMap    = m_databasePtr->nmfQueryDatabase(queryStr,fields);
    fields1    = {"MSVPAName", "Variable"};
    queryStr1  =  "SELECT MSVPAName,Variable FROM MSVPASeasInfo ";
    queryStr1 +=  "WHERE MSVPAName = '" + MSVPAName + "' ";
    queryStr1 +=  "AND Variable = 'SeasLen'";
    dataMap1   = m_databasePtr->nmfQueryDatabase(queryStr1,fields1);
    fields2    = {"MSVPAName", "Variable"};
    queryStr2  =  "SELECT MSVPAName,Variable FROM MSVPASeasInfo ";
    queryStr2 +=  "WHERE MSVPAName = '" + MSVPAName + "' ";
    queryStr2 +=  "AND Variable = 'SeasTemp'";
    dataMap2   = m_databasePtr->nmfQueryDatabase(queryStr2,fields2);
    MSVPAPageEnabled[page++] = ((dataMap["MSVPAName"].size() > 0) &&
                                (dataMap1["MSVPAName"].size() > 0) &&
                                (dataMap2["MSVPAName"].size() > 0));

    // Tab 3
    fields     = {"SpeName", "Biomass"};
    queryStr   = "SELECT SpeName,Biomass FROM OtherPredBM ";
    dataMap    = m_databasePtr->nmfQueryDatabase(queryStr,fields);
    fields1    = {"SpeName", "SizeCat"};
    queryStr1  = "SELECT SpeName,SizeCat FROM OthPredSizeData ";
    dataMap1   = m_databasePtr->nmfQueryDatabase(queryStr1,fields1);
    MSVPAPageEnabled[page++] = ((dataMap["SpeName"].size() > 0) &&
                                (dataMap1["SpeName"].size() > 0));

    // Tab 4
    fields     = {"MSVPAName", "OthPreyName"};
    queryStr   = "SELECT MSVPAName,OthPreyName FROM MSVPAOthPrey ";
    queryStr  += "WHERE MSVPAName = '" + MSVPAName + "' ";
    dataMap    = m_databasePtr->nmfQueryDatabase(queryStr,fields);
    fields1    = {"MSVPAName", "OthPreyName"};
    queryStr1  = "SELECT MSVPAName,OthPreyName FROM MSVPAOthPrey ";
    queryStr1 += "WHERE MSVPAName = '" + MSVPAName + "' ";
    dataMap1   = m_databasePtr->nmfQueryDatabase(queryStr1,fields1);
    MSVPAPageEnabled[page++] = ((dataMap["MSVPAName"].size() > 0) &&
                                (dataMap1["MSVPAName"].size() > 0));

    // Tab 5
    fields     = {"MSVPAName", "PrefVal"};
    queryStr   = "SELECT MSVPAName,PrefVal FROM MSVPAprefs ";
    queryStr  += "WHERE MSVPAName = '" + MSVPAName + "' ";
    dataMap    = m_databasePtr->nmfQueryDatabase(queryStr,fields);
    MSVPAPageEnabled[page++] = (dataMap["MSVPAName"].size() > 0);

    // Tab 6
    fields     = {"MSVPAName", "SpOverlap"};
    queryStr   = "SELECT MSVPAName,SpOverlap FROM MSVPASpaceO ";
    queryStr  += "WHERE MSVPAName = '" + MSVPAName + "' ";
    dataMap    = m_databasePtr->nmfQueryDatabase(queryStr,fields);
    MSVPAPageEnabled[page++] = (dataMap["MSVPAName"].size() > 0);

    // Tab 7
    fields     = {"MSVPAName", "SpeIndex"};
    queryStr   = "SELECT MSVPAName,SpeIndex FROM MSVPASizePref ";
    queryStr  += "WHERE MSVPAName = '" + MSVPAName + "' ";
    dataMap    = m_databasePtr->nmfQueryDatabase(queryStr,fields);
    MSVPAPageEnabled[page++] = (dataMap["MSVPAName"].size() > 0);

    // Tab 8
    fields     = {"MSVPAName", "SpeIndex"};
    queryStr   = "SELECT MSVPAName,SpeIndex FROM MSVPAStomCont ";
    queryStr  += "WHERE MSVPAName = '" + MSVPAName + "' ";
    dataMap    = m_databasePtr->nmfQueryDatabase(queryStr,fields);
    MSVPAPageEnabled[page++] = (dataMap["MSVPAName"].size() > 0);

    // Tab 9 [Energy Density]       TBD
    MSVPAPageEnabled[page++] = false;

    // Tab 10 [Diet Energy Density] TBD
    MSVPAPageEnabled[page++] = false;

    // Tab 11
    MSVPAPageEnabled[page++] = true;
    fields     = {"MSVPAName", "Type", "SSVPAindex"};
    queryStr   = "SELECT MSVPAName,Type,SSVPAindex FROM MSVPAspecies ";
    queryStr  += "WHERE MSVPAName = '" + MSVPAName + "' ";
    queryStr  += "AND (Type = 0 OR Type = 1)";
    dataMap    = m_databasePtr->nmfQueryDatabase(queryStr,fields);
    int NumRecords = dataMap["MSVPAName"].size();
    for (int i=0; i<NumRecords; ++i) {
        if (std::stoi(dataMap["SSVPAindex"][i]) < 1) {
            MSVPAPageEnabled[page-1] = false;
            break;
        }
    }

    // This loop changes when the above TBD's are done.
//    bool allTrueSoFar = MSVPAPageEnabled[page-1];
//    if (allTrueSoFar) {
//        for (int i=0; i<8; ++i) {// TBD
//            if (! MSVPAPageEnabled[i]) {
//                allTrueSoFar = false;
//                break;
//            }
//        }
//    }

    bool allTrueSoFar = true;
    for (unsigned int i=0; i<MSVPAPageEnabled.size()-1; ++i) {
        if ((i != 8) && (i != 9)) {
            if (! MSVPAPageEnabled[i]) {
                allTrueSoFar = false;
                break;
            }
        }
    }



    MSVPAPageEnabled[page++] = allTrueSoFar;
    //RunMSVPAPB->setEnabled(allTrueSoFar);


    // Set the MSVPA sub item states appropriately
    for (unsigned int i=0;i<MSVPAPageEnabled.size();++i) {
        // Enable the tab page and the Navigator Tree sub-item
        pageEnabled = MSVPAPageEnabled[i];
        m_UI->MSVPAInputTabWidget->setTabEnabled(i, pageEnabled);
        NavigatorTree->topLevelItem(2)->child(i)->setDisabled(! pageEnabled);
    }
    MSVPA_Tab11_ptr->enableNextButton(m_UI->MSVPAInputTabWidget->isTabEnabled(10));


    m_logger->logMsg(nmfConstants::Normal,"nmfMainWindow::MSVPAFindAndStorePageStates Complete");

} // end MSVPAFindStoreAndSetPageStates

void
nmfMainWindow::callback_MSVPALoadWidgets(int TabNum)
{
    bool okToProceed = true;

    m_UI->MSVPAInputTabWidget->setTabEnabled(TabNum,true); // RSK

    m_logger->logMsg(nmfConstants::Normal,"Loading MSVPA Tab "+std::to_string(TabNum+1)+"...");

    MSVPAPageEnabled[TabNum] = true;

    switch (TabNum) {
        case 0:
            okToProceed = MSVPA_Tab1_ptr->loadWidgets( m_databasePtr, MSVPAName);
            break;
        case 1:
            okToProceed = MSVPA_Tab2_ptr->loadWidgets( m_databasePtr, MSVPAName);
            break;
        case 2:
            okToProceed = MSVPA_Tab3_ptr->loadWidgets( m_databasePtr, MSVPAName);
            break;
        case 3:
            okToProceed = MSVPA_Tab4_ptr->loadWidgets( m_databasePtr, MSVPAName);
            break;
        case 4:
            okToProceed = MSVPA_Tab5_ptr->loadWidgets( m_databasePtr, MSVPAName);
            break;
        case 5:
            okToProceed = MSVPA_Tab6_ptr->loadWidgets( m_databasePtr, MSVPAName);
            break;
        case 6:
            okToProceed = MSVPA_Tab7_ptr->loadWidgets( m_databasePtr, MSVPAName);
            break;
        case 7:
            okToProceed = MSVPA_Tab8_ptr->loadWidgets( m_databasePtr, MSVPAName);
            break;
        case 10:
            okToProceed = MSVPA_Tab11_ptr->loadWidgets(m_databasePtr, MSVPAName);
            break;
        case 11:
            okToProceed = MSVPA_Tab12_ptr->loadWidgets(m_databasePtr, MSVPAName, m_ProjectDir);
//            if (okToProceed) {
//                RunMSVPAPB->setEnabled(true);
//            }
            break;
        default:
            break;
    }

    if (! okToProceed) {
        QMessageBox::information(this,
                                 tr("Problem with Loading"),
                                 tr("\nPlease check all data fields on page and reload."),
                                 QMessageBox::Ok);
    }
} // end callback_MSVPALoadWidget



void nmfMainWindow::loadMSVPAInputWidgets()
{
    bool okToProceed = false;

    MSVPAName = entityName();
    if (MSVPAWidgetsLoaded)
        return;
    // Disble Navigator tree items
    for (int i=0; i<nmfConstantsMSVPA::NumberOfMSVPAPages; ++i) { // ui->MSVPAInputTabWidget->count(); ++i) {
        NavigatorTree->topLevelItem(2)->child(i)->setDisabled(true);
    }

    m_logger->logMsg(nmfConstants::Normal,"Loading MSVPA configuration: "+MSVPAName);
    NavigatorTree->topLevelItem(2)->child(0)->setDisabled(false);

    okToProceed = MSVPA_Tab1_ptr->loadWidgets(m_databasePtr,
                                              MSVPAName);
    if (! okToProceed)
        return;

    queryAndLoadInitFields();
    MSVPAWidgetsLoaded = true;

return;

    // RSK - Implement 8 and 9 when you do the Growth algorithm
    MSVPA_Tab9_ptr->loadWidgets(m_databasePtr, MSVPAName);
    MSVPA_Tab10_ptr->loadWidgets(m_databasePtr, MSVPAName);

}

void nmfMainWindow::clearMSVPAInputWidgets()
{
    MSVPA_Tab1_ptr->clearWidgets();
    MSVPA_Tab2_ptr->clearWidgets();
    MSVPA_Tab3_ptr->clearWidgets();
    MSVPA_Tab4_ptr->clearWidgets();
    MSVPA_Tab5_ptr->clearWidgets();
    MSVPA_Tab6_ptr->clearWidgets();
    MSVPA_Tab7_ptr->clearWidgets();
    MSVPA_Tab8_ptr->clearWidgets();
    MSVPA_Tab9_ptr->clearWidgets();
    MSVPA_Tab10_ptr->clearWidgets();
    MSVPA_Tab11_ptr->clearWidgets();
    MSVPA_Tab12_ptr->clearWidgets();
    //msvpaApi->clearProgressBar();

} // end clearMSVPAInputWidgets


void nmfMainWindow::getYearsAndAges(const std::string &MSVPAName, const std::string &species,
                                    int &NumYears, int &NumAges) {
    std::string queryStr = "";
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    // Find NumYears and NumAges per species
    fields = {"NumYears","NumAges"};
    queryStr = "SELECT count(DISTINCT(Year)) as NumYears, count(DISTINCT(Age)) as NumAges FROM MSVPASeasBiomass WHERE MSVPAName = '" + MSVPAName +
            "' and SpeName='" + species + "'";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    NumYears = std::stoi(dataMap["NumYears"][0]);
    NumAges  = std::stoi(dataMap["NumAges"][0]);
}

void nmfMainWindow::getMaturityData(
        const int &Nage,
        //const int &NYears, const int &FirstYear, const int &LastYear,
        const std::string &SpeName,
        boost::numeric::ublas::matrix<double> &Maturity)
{
    int m = 0;
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    fields = {"PMature"};
    queryStr = "SELECT PMature from SpeMaturity where SpeName='" + SpeName +
               "' and Year >= " + std::to_string(FirstYear) + " and Year <= " + std::to_string(LastYear) +
               " ORDER By Age,Year";

    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["PMature"].size() > 0) {
        for (int i=0; i<Nage; ++i) {
            for (int j=0; j<NumYears; ++j) {
                Maturity(i,j) = std::stod(dataMap["PMature"][m++]);
            }
        }
    }
} // end getMaturityData

void nmfMainWindow::loadMsvpaCharts(std::string selectedSpecies) {
    std::string queryStr = "";
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::vector<std::string> species;
    int NumAges;
    int NumSpecies;
    std::map<std::string, int> NumYearsMap;
    std::map<std::string, int> NumAgesMap;

    // Find all species
    fields = {"SpeName"};
    queryStr = "SELECT SpeName from MSVPAspecies WHERE MSVPAName='" + MSVPAName + "'";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    NumSpecies = dataMap["SpeName"].size();
    for (int i=0;i<NumSpecies;++i) {
        species.push_back(dataMap["SpeName"][i]);
    }

    // Get NumYears and NumAges for all species and read into maps
    for (int i=0; i<NumSpecies; ++i) {
        getYearsAndAges(MSVPAName,species[i],NumYears,NumAges);
        NumYearsMap[species[i]] = NumYears;
        NumAgesMap[species[i]] = NumAges;
    }

    // Load M2 rates
    fields = {"Year","Age","M2"};
    queryStr = "SELECT Year,Age,Sum(SeasM2) as M2 FROM MSVPASeasBiomass WHERE MSVPAName = '" + MSVPAName +"'" +
               " and SpeName='" + selectedSpecies + "'" +
               " GROUP BY Year, Age";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);  // RSK - error message here if queried table is empty
    if (dataMap["Year"].size() > 0) {
        // Check for data in table (it may be empty if model not run).
        nmfUtils::initialize(M2Matrix, NumYears, NumAges);
        int k = 0;
        for (int i = 0; i < NumYears; ++i) {
            for (int j = 0; j < NumAges; ++j) {
                M2Matrix(i, j) = std::stod(dataMap["M2"][k++]);
            }
        }
    } else {
        std::cout << "Note: No data found in table from previous run to load into Chart area." << std::endl;
    }

} // end loadMsvpaCharts


void
nmfMainWindow::callback_ResetSpeciesList()
{

    QModelIndex index;

    index = EntityListLV->currentIndex();

    callback_SSVPASingleClicked(index);

} // end callback_ResetSpeciesList


void
nmfMainWindow::callback_ReselectSpecies(std::string Species, bool withCallback)
{
    m_logger->logMsg(nmfConstants::Normal,"nmfMainWindow::callback_ReselectSpecies");

    QModelIndex mindex;

    for (int i=0; i<EntityListLV->model()->rowCount(); ++i) {
        mindex = EntityListLV->model()->index(i,0);
        if (Species == EntityListLV->model()->data(mindex).toString().toStdString()) {
            EntityListLV->blockSignals(true);
            EntityListLV->setCurrentIndex(mindex);
            EntityListLV->blockSignals(false);
            if (withCallback)
                callback_SSVPASingleClickedDo(mindex);
            break;
        }

    } // end for

    NavigatorTree->clearFocus(); // Just to clean up Nav Tree

    m_logger->logMsg(nmfConstants::Normal,"nmfMainWindow::callback_ReselectSpecies Complete");

} // end callback_ReselectSpecies



void
nmfMainWindow::callback_SSVPAInputTabChanged(int tab)
{
    callback_InitializePage("SSVPA",tab,true);

} // end callback_SSVPAInputTabChanged


void
nmfMainWindow::callback_InitializePage(std::string model, int page, bool withCallback)
{
    m_logger->logMsg(nmfConstants::Normal,"nmfMainWindow::callback_InitializePage: " + std::to_string(page));

    // Load all Species into CatchAtAge tableView model array.
    std::vector<QString> AllSpecies;
    QModelIndex mindex;

    // Set Species List to first item
    //ResetSpeciesList(false);
    for (int i=0;i<EntityListLV->model()->rowCount();++i) {
        mindex = EntityListLV->model()->index(i,0);
        AllSpecies.push_back(EntityListLV->model()->data(mindex).toString());
    } // end for

    if (model == "SSVPA") {
        //callback_EnableRunSSVPAPB(false);

        NavigatorTree->blockSignals(true);
        NavigatorTree->clearFocus();
        NavigatorTree->clearSelection();
        NavigatorTree->topLevelItem(1)->setExpanded(true);
        NavigatorTree->topLevelItem(1)->child(page)->setSelected(true);
        NavigatorTree->blockSignals(false);

        switch (page) {
            // Loading deselects the currently selected species
            case 0:
                m_CatchDataInitialized = true;
                SSVPA_Tab1_ptr->loadAllSpeciesFromTableOrFile(m_databasePtr,
                                                              SpeciesIndex, SpeciesName,
                                                              AllSpecies, "FromTable");
                break;
            case 1:
                SSVPA_Tab2_ptr->loadAllSpeciesFromTableOrFile(m_databasePtr,
                                                              SpeciesIndex, SpeciesName,
                                                              AllSpecies, "FromTable");
                break;
            case 2:
                SSVPA_Tab3_ptr->loadAllSpeciesFromTableOrFile(m_databasePtr,
                                                              SpeciesIndex, SpeciesName,
                                                              AllSpecies,  "FromTable");
                break;
            case 3:
                SSVPA_Tab1_ptr->loadSpecies(SpeciesIndex);
                //callback_EnableRunSSVPAPB(true);
                SSVPA_Tab4_ptr->loadDefaultWidgets(m_databasePtr,
                                                   AllSpecies,
                                                   SpeciesIndex);
                break;
            default:
                break;
        }
    }

    m_logger->logMsg(nmfConstants::Normal,"nmfMainWindow::callback_InitializePage Complete");

} // end callback_InitializePage



void
nmfMainWindow::clearMSVPAOutputWindow()
{
    std::cout << "clearMSVPAOutputWindow TBD" << std::endl;

}


void
nmfMainWindow::clearSSVPAOutputWindow()
{
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    boost::numeric::ublas::matrix<double> Abundance;
    boost::numeric::ublas::matrix<double> M_NaturalMortality;
    boost::numeric::ublas::matrix<double> F_FishingMortality;
    int MinCatchAge,MaxCatchAge,FirstCatchYear,LastCatchYear; //,NumCatchYears;

    //
    // Clear the Chart Tab
    //
    fields   = {"SpeName","FirstYear","LastYear","MinCatAge","MaxCatAge"};
    queryStr = "SELECT SpeName,FirstYear,LastYear,MinCatAge,MaxCatAge FROM Species WHERE SpeIndex = " +
                std::to_string(SpeciesIndex);
    dataMap  = m_databasePtr->nmfQueryDatabase(queryStr,fields);
    int NumRecords = dataMap["SpeName"].size();
    if (NumRecords == 0) {
        //logger->logMsg(nmfConstants::Normal,"Error(nmfMainWindow::clearSSVPAOutputWindow): Couldn't find SpeIndex: " +
        //                  std::to_string(SpeciesIndex));
        return;
    }
    FirstCatchYear = std::stoi(dataMap["FirstYear"][0]);
    LastCatchYear  = std::stoi(dataMap["LastYear"][0]);
    MinCatchAge    = std::stoi(dataMap["MinCatAge"][0]);
    MaxCatchAge    = std::stoi(dataMap["MaxCatAge"][0]);
    //NumCatchYears  = LastCatchYear - FirstCatchYear + 1;

    nmfUtils::initialize(Abundance,          1, MaxCatchAge + 1);
    nmfUtils::initialize(M_NaturalMortality, 1, MaxCatchAge + 1);
    nmfUtils::initialize(F_FishingMortality, 1, MaxCatchAge + 1);

    // Draw chart
    updateChart(Abundance,
                M_NaturalMortality,
                F_FishingMortality,
                MinCatchAge,
                MaxCatchAge,
                FirstCatchYear,
                LastCatchYear,
                true);

    //
    // Clear the Data tab
    //
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, nmfConstants::Clear);


} // end clearSSVPAOutputWindow


void
nmfMainWindow::callback_SpeciesDoubleClick( QModelIndex m)
{
    std::cout << "DOUBLE CLICK *new*" << std::endl;

    //callback_RunSSVPAPBClicked(false);

} // end callback_SpeciesDoubleClick



void nmfMainWindow::callback_SSVPASingleClicked(const QModelIndex &curr)
{
    callback_SSVPASingleClickedDo(curr);

    QString tabName = nmfUtilsQt::getCurrentTabName(m_UI->SSVPAInputTabWidget);

    if ( (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) == true) &&
         (tabName == "4. Config Data"))
    {
        QApplication::keyboardModifiers().setFlag(Qt::NoModifier);
        // Run
        this->setCursor(Qt::WaitCursor);
        updateModel();
        this->setCursor(Qt::ArrowCursor);
    }

} // end callback_SSVPASingleClicked

void
nmfMainWindow::callback_SSVPASingleClickedDo(const QModelIndex &curr)
{
std::cout << "curr: " << curr.data().toString().toStdString() << std::endl;

    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    int tabIndex = m_UI->SSVPAInputTabWidget->currentIndex();
    std::vector<QString> AllSpecies;
    QModelIndex mindex;
    SpeciesName = curr.data().toString().toStdString();

    m_logger->logMsg(nmfConstants::Normal,"callback_SSVPASingleClickedDo - Species: " + SpeciesName);

    for (int i=0;i<EntityListLV->model()->rowCount();++i) {
        mindex = EntityListLV->model()->index(i,0);
        AllSpecies.push_back(EntityListLV->model()->data(mindex).toString());
    } // end for

    clearSSVPAOutputWindow();

    std::string species = curr.data(Qt::DisplayRole).toString().toStdString();
    updateMainWindowTitle("SSVPA: " + species);

    SSVPASelectConfigurationAndTypeGB->setTitle("Select Configuration and Type for:  " +
                                                QString::fromStdString(species));
    m_UI->SSVPAInputTabWidget->show();

    // Set wait cursor
    this->setCursor(Qt::WaitCursor);

    // Find the actual species index from the Species table.  Don't assume the index is the index
    // from the item in the table (i.e., its position). It may not always be.
    fields   = {"SpeIndex"};
    queryStr = "SELECT SpeIndex FROM Species WHERE SpeName = '" + species + "'";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["SpeIndex"].size() == 0) {
        std::cout << "query: " << queryStr << std::endl;
        QMessageBox::information(this,
                                 tr("No Species Found"),
                                 tr("\nPlease enter at least 1 Species before continuing."),
                                 QMessageBox::Ok);
        this->setCursor(Qt::ArrowCursor);
        return;
    }
    m_UI->SSVPAInputTabWidget->setTabEnabled(0,true);

    SpeciesIndex = std::stoi(dataMap["SpeIndex"][0]);
    SpeciesName  = species;

    // Find out what tab is active and load the appropriate data.
    switch (tabIndex) {
        case 0:
            SSVPA_Tab1_ptr->loadSpecies(SpeciesIndex);
            break;
        case 1:
            SSVPA_Tab2_ptr->loadSpecies(SpeciesIndex);
            break;
        case 2:
            SSVPA_Tab3_ptr->loadSpecies(SpeciesIndex);
            break;
        case 3:
            //callback_EnableRunSSVPAPB(true);
            SSVPA_Tab4_ptr->clearModels();
            SSVPA_Tab4_ptr->loadDefaultWidgets(m_databasePtr,
                                               AllSpecies,
                                               SpeciesIndex);
            break;
        default:
            break;
    }

    this->setCursor(Qt::ArrowCursor);

    m_logger->logMsg(nmfConstants::Normal,"callback_SSVPASingleClickedDo Complete - Species: " + SpeciesName);

} // end callback_SSVPASingleClickedDo


void nmfMainWindow::updateTable(QTableView* view, QSqlQueryModel* model_ptr,
                                std::string cmd)
{
    QSqlQuery query(cmd.c_str());
    model_ptr->setQuery(query);
    view->resizeColumnsToContents();
}


void nmfMainWindow::setCatchAtAgePtr(QSqlQueryModel* ptr) {
    catchAtAge_ptr = ptr;
}
void nmfMainWindow::setWeightAtAgePtr(QSqlQueryModel* ptr) {
    weightAtAge_ptr = ptr;
}
void nmfMainWindow::setSizeAtAgePtr(QSqlQueryModel* ptr) {
    sizeAtAge_ptr = ptr;
}
void nmfMainWindow::setMaturityPtr(QSqlQueryModel* ptr) {
    maturity_ptr = ptr;
}

QSqlQueryModel* nmfMainWindow::catchAtAgePtr() {
    return catchAtAge_ptr;
}
QSqlQueryModel* nmfMainWindow::weightAtAgePtr() {
    return weightAtAge_ptr;
}
QSqlQueryModel* nmfMainWindow::sizeAtAgePtr() {
    return sizeAtAge_ptr;
}
QSqlQueryModel* nmfMainWindow::maturityPtr() {
    return maturity_ptr;
}


void nmfMainWindow::sendToOutputWindow(QTextEdit *textEdit, std::string text) {
    if (text.empty()) {
        textEdit->clear();
    }
    textEdit->append(QString::fromStdString(text));
}

void nmfMainWindow::sendToOutputWindow(QTextEdit *textEdit, std::string name, int value) {
    std::string text = "<html><b>" + name + ": " + "</b></html>"
            + std::to_string(value);
    textEdit->append(QString::fromStdString(text));
}

void nmfMainWindow::sendToOutputWindow(QTextEdit *textEdit, std::string name, double value,
        int numDigits, int numDecimals) {
    std::stringstream stream;
    stream << std::fixed << std::setw(numDigits)
            << std::setprecision(numDecimals) << value;

    std::string text = "<html><b>" + name + ": " + "</b></html>" + stream.str();
    textEdit->append(QString::fromStdString(text));
}

//void nmfMainWindow::sendToOutputWindow(QTextEdit *te,
//    QStringList rowTitles,
//    std::vector<std::string> colTitles,
//    boost::numeric::ublas::matrix<double> &outMatrix,
//    int numDigits, int numDecimals)
//{
//    char buf[100];
//    te->clear();
//    std::string content;

//    std::string format = "%"+std::to_string(numDigits)+"."+std::to_string(numDecimals)+"f";

//    int numRows = outMatrix.size1();
//    int numCols = outMatrix.size2();

//    for (int row=0; row<numRows; ++row) {

//        for (int col=0; col<numCols; ++col) {
//            sprintf(buf,format.c_str(),outMatrix(row,col));
//            std::string valStr(buf);
//            content += valStr;

//        }
//        content += "<br>";
//    }

//} // end sendToOutputTextEdit


void nmfMainWindow::sendToOutputTable(QTableWidget *tw,
    QStringList rowTitles,
    QStringList colTitles,
    boost::numeric::ublas::matrix<double> &outMatrix,
    int numDigits, int numDecimals)
{

    char buf[200];
    tw->clear();
    QTableWidgetItem *item;

    std::string format = "%"+std::to_string(numDigits)+"."+std::to_string(numDecimals)+"f";

    int numRows = outMatrix.size1();
    int numCols = outMatrix.size2();
std::cout << "nRowTitles,nColTitles: " << rowTitles.size() << "," << colTitles.size() << std::endl;
std::cout << "r,c: " << numRows << "," << numCols << std::endl;
    tw->setColumnCount(numCols);
    tw->setRowCount(numRows);

    for (int col=0; col<numCols; ++col) {
        tw->setHorizontalHeaderItem(col,
          new QTableWidgetItem(colTitles[col]));
    }

    for (int row=0; row<numRows; ++row) {
        tw->setVerticalHeaderItem(row,
          new QTableWidgetItem(rowTitles[row]));
std::cout << std::endl;
        for (int col=0; col<numCols; ++col) {
            sprintf(buf,format.c_str(),outMatrix(row,col));
            std::string valStr(buf);
std::cout << valStr;
            item = new QTableWidgetItem(QString::fromStdString(valStr));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            tw->setItem(row,col,item);
        }
    }

    tw->resizeColumnsToContents();

} // end sendToOutputTable

void nmfMainWindow::sendToOutputWindow(QTextEdit *textEdit,
        std::string name, int FirstCatchYear,
        int FirstCatchAge, int LastCatchAge,
        boost::numeric::ublas::matrix<double> &outMatrix,
        int numDigits,
        int numDecimals)
{
    std::string ageStr = "";
    std::string ageStr2 = "";
    std::stringstream stream;

    // Draw header
    for (int age = FirstCatchAge; age <= LastCatchAge; ++age) {
        ageStr = "Age " + std::to_string(age) + "  ";
        stream << std::fixed << std::setw(numDigits)
                << std::setprecision(numDecimals) << ageStr.c_str();
        ageStr2 += stream.str();
        stream.str(std::string());
        stream.clear();
    }
    ageStr2 += "\n";

    // Draw data
    QString matrixStr = "";
    std::string dimensions = "(" + std::to_string(outMatrix.size1()) + " x "
            + std::to_string(outMatrix.size2()) + ")";
    textEdit->append(
            QString::fromStdString(
                    "<html><b><br/>" + name + dimensions + "<br/></b></html>"));
    //OutputTE->append(QString::fromStdString("Year "+ageStr2));
    stream.str(std::string());
    stream.clear();
    for (boost::numeric::ublas::matrix<double>::iterator1 it1 =
            outMatrix.begin1(); it1 != outMatrix.end1(); ++it1) {
        matrixStr = QString::number(FirstCatchYear++) + "  ";

        for (boost::numeric::ublas::matrix<double>::iterator2 it2 = it1.begin();
                it2 != it1.end(); ++it2) {
            stream << std::fixed << std::setw(numDigits)
                    << std::setprecision(numDecimals) << (*it2);
            matrixStr += QString::fromStdString(stream.str());
            stream.str(std::string());
            stream.clear();
        }
        textEdit->append(matrixStr);
    }
}

void nmfMainWindow::load(QListView* view,
        QSqlQueryModel* model_ptr,
        std::string query)
{
    if (query != "") {
        model_ptr->setQuery(query.c_str());
    }
    view->setModel( model_ptr );
}

void nmfMainWindow::hideWidgets()
{
    QTabWidget* MSVPAOutputTabWidget = m_UI->OutputWidget->findChild<QTabWidget *>("MSVPAOutputTabWidget");

    m_UI->SetupInputTabWidget->hide();
    m_UI->SSVPAInputTabWidget->hide();
    m_UI->MSVPAInputTabWidget->hide();
    m_UI->ForecastInputTabWidget->hide();

    ForecastListLBL->hide();
    ForecastListLV->hide();
    ScenarioListLBL->hide();
    ScenarioListLV->hide();

    SSVPAOutputW->hide();

    MSVPAOutputTabWidget->setTabEnabled(2,false);

}

void nmfMainWindow::activateSetupWidgets()
{
    // Set widget parameters
    m_UI->EntityDockWidget->setWindowTitle("Parameters");
    m_UI->EntityDockWidget->setToolTip("");
    m_UI->EntityDockWidget->setStatusTip("");
    EntityListLV->setWhatsThis("");

    // Remove previous model data and read data from database into model
    entity_model.removeRows(0, entity_model.count(), QModelIndex());

    // Hide unneeded widgets
    hideWidgets();

    m_UI->SetupInputTabWidget->show();
//    m_UI->SetupOutputW->show();

    // Load Setup widgets
    Setup_Tab2_ptr->loadWidgets(m_databasePtr);
    Setup_Tab3_ptr->loadWidgets(m_databasePtr);

} // end activateSetupWidgets




//void
//nmfMainWindow::callback_currentTextChanged()
//{
//       Species_CMB->lineEdit()->clear();
//}

void nmfMainWindow::enableSSVPAWidgets(bool enable) {
    NaturalMortalityCB->setEnabled(enable);
    LastYearMortalityRateDSB->setEnabled(enable);
    ReferenceAgeSB->setEnabled(enable);
    SelectVPATypeCMB->setEnabled(enable);
    SelectConfigurationCMB->setEnabled(enable);
    SSVPAInitialSelectivityTV->setEnabled(enable);
    MortalityMinLE->setEnabled(enable);
    MortalityMaxLE->setEnabled(enable);
    SelectivityMinLE->setEnabled(enable);
    SelectivityMaxLE->setEnabled(enable);
    Configure_Cohort_SB1->setEnabled(enable);
    Configure_Cohort_SB2->setEnabled(enable);
    Configure_Cohort_SB3->setEnabled(enable);
    Configure_Cohort_SB4->setEnabled(enable);
}


void nmfMainWindow::enableMSVPAWidgets(bool enable) {
    selectByVariablesCMB->setEnabled(enable);
    selectByVariablesLBL->setEnabled(enable);
    selectSeasonLBL->setEnabled(enable);
    selectSeasonCMB->setEnabled(enable);
    selectSeasonCB->setEnabled(enable);
    selectSpeciesAgeSizeClassLBL->setEnabled(enable);
    selectSpeciesAgeSizeClassCMB->setEnabled(enable);
    selectSpeciesAgeSizeClassCB->setEnabled(enable);
    selectPreySpeciesCMB->setEnabled(enable);
    selectPreySpeciesLBL->setEnabled(enable);
    themeLBL->setEnabled(enable);
    themeCMB->setEnabled(enable);
}

void nmfMainWindow::enable3DControls(bool enable)
{
    selectionModeLBL->setEnabled(enable);
    selectionModeCMB->setEnabled(enable);
}

void nmfMainWindow::activateSSVPAWidgets()
{
    QString whatsThis;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    clearMSVPAInputWidgets();

    // Set widget parameters
    m_UI->EntityDockWidget->setWindowTitle("Species List");

    whatsThis  = "<strong><center>Species List</center></strong>";
    whatsThis += "<p>This list shows the age-structured species for ";
    whatsThis += "this database. To interact with the list either: ";
    whatsThis += "<br><br>Click: Select Species and show relevant data ";
    whatsThis += "<br>Ctrl+Click: Select Species and Run SSVPA</p> ";
    EntityListLV->setWhatsThis(whatsThis);
    EntityListLV->setToolTip("Choose a Species with which to load the center tables.");
    EntityListLV->setStatusTip("Choose a Species with which to load the center tables.");

    // Remove previous model data and read data from database into model
    entity_model.removeRows(0, entity_model.count(), QModelIndex());
    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM Species;";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    for (auto name : dataMap[fields[0]]) {
        entity_model.append( nmfEntity{ QString::fromStdString(name) });
    }
//    queryStr = "SELECT SpeName FROM OtherPredSpecies;";
//    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
//    for (auto name : dataMap[fields[0]]) {
//        entity_model.append( nmfEntity{ QString::fromStdString(name) });
//    }
    EntityListLV->scrollToTop();

    // Disable SSVPA widgets
    //enableSSVPAWidgets(false);  // RSK put this back in???

    // Setup signals
    disconnect(EntityListLV, 0, 0, 0);

    connect(EntityListLV, SIGNAL(clicked(const QModelIndex &)),
            this,         SLOT(callback_SSVPASingleClicked(const QModelIndex &)));

    // Hide unneeded widgets
    hideWidgets();

    // Show needed widgets
    m_UI->SSVPAInputTabWidget->show();
    //SSVPAOutputTabWidget->show();
    //RunSSVPAPB->show();
    //SSVPAOutputControlsGB->show();
    SSVPAOutputW->show();

    m_UI->actionNew->setToolTip("Create a New Species");
    m_UI->actionNew->setStatusTip("Create a New Species");


} // end activateSSVPAWidgets


void nmfMainWindow::activateMSVPAWidgets()
{
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    QString whatsThis;

    whatsThis  = "<strong><center>MSVPA List</center></strong>";
    whatsThis += "<p>This list shows the MSVPA configurations defined for ";
    whatsThis += "the loaded database. To interact with the list either: ";
    whatsThis += "<br><br>Click: Load MSVPA and show relevant data ";
    whatsThis += "<br>Ctrl+Click: Load and Run MSVPA</p> ";

    // Set widget parameters
    m_UI->EntityDockWidget->setWindowTitle("MSVPA List");
    m_UI->EntityDockWidget->setToolTip("Choose an MSVPA configuration to load");
    m_UI->EntityDockWidget->setStatusTip("Choose an MSVPA configuration to load");
    EntityListLV->setToolTip("Choose an MSVPA configuration to load.");
    EntityListLV->setStatusTip("Choose an MSVPA configuration to load.");
    EntityListLV->setWhatsThis(whatsThis);

    // Remove previous model data and read data from database into model
    entity_model.removeRows(0, entity_model.count(), QModelIndex());
    fields = {"MSVPAName"};
    queryStr = "SELECT MSVPAName FROM MSVPAlist;";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    for (auto name : dataMap[fields[0]]) {
        entity_model.append( nmfEntity{ QString::fromStdString(name) });
    }
    EntityListLV->scrollToTop();

    // Setup signals
    disconnect(EntityListLV, 0, 0, 0);
    connect(EntityListLV, SIGNAL(clicked(const QModelIndex &)),
            this,         SLOT(callback_MSVPASingleClickedGUI(const QModelIndex &)));

    // Hide unneeded widgets
    hideWidgets();

    // Show needed widgets
    m_UI->MSVPAInputTabWidget->show();

    m_UI->actionNew->setToolTip("Create a New MSVPA Configuration");
    m_UI->actionNew->setStatusTip("Create a New MSVPA Configuration");

} // end activateMSVPAWidgets


void nmfMainWindow::clearForecastWidgets()
{
    // Clear list widgets
    //EntityListLV->clearSelection();
    ForecastListLV->clearSelection();
    ScenarioListLV->clearSelection();
    forecast_model.removeRows(0, forecast_model.count(), QModelIndex());
    scenario_model.removeRows(0, scenario_model.count(), QModelIndex());
    //RunForecastPB->setEnabled(false);

    // Clear tab widgets
    Forecast_Tab1_ptr->clearWidgets();
    Forecast_Tab2_ptr->clearWidgets();
    Forecast_Tab3_ptr->clearWidgets();
    Forecast_Tab4_ptr->clearWidgets();
    Forecast_Tab5_ptr->clearWidgets();

    //nmfForecast::updateProgress(0,"");

} // end clearForecastWidgets

void nmfMainWindow::activateForecastWidgets()
{
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    EntityListLV->clearSelection();
    clearForecastWidgets();

    // Set widget parameters
    m_UI->EntityDockWidget->setWindowTitle("MSVPA List");
    m_UI->EntityDockWidget->setToolTip("Choose an MSVPA configuration.");
    m_UI->EntityDockWidget->setStatusTip("Choose an MSVPA configuration.");
    m_UI->EntityDockWidget->setToolTip("Choose an MSVPA configuration.");
    EntityListLV->setToolTip("Choose an MSVPA configuration and then choose a Forecast and Scenario.");
    EntityListLV->setStatusTip("Choose an MSVPA configuration and then choose a Forecast and Scenario.");

    // Remove previous model data and read data from database into model
    entity_model.removeRows(0, entity_model.count(), QModelIndex());
    fields = {"MSVPAName"};
    queryStr = "SELECT MSVPAName FROM MSVPAlist;";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    for (auto name : dataMap[fields[0]]) {
        entity_model.append( nmfEntity{ QString::fromStdString(name) });
    }
    EntityListLV->scrollToTop();

    // Setup signals
    disconnect(EntityListLV,   0, 0, 0);
    disconnect(ScenarioListLV, 0, 0, 0);
    connect(EntityListLV,   SIGNAL(clicked(const QModelIndex &)),
            this,           SLOT(callback_msvpaForecastSingleClicked(const QModelIndex &)));
    connect(ForecastListLV, SIGNAL(doubleClicked(const QModelIndex &)),
            this,	        SLOT(callback_forecastDoubleClicked(const QModelIndex &)));
    connect(ForecastListLV, SIGNAL(clicked(const QModelIndex &)),
            this,           SLOT(callback_forecastSingleClicked(const QModelIndex &)));
    connect(ScenarioListLV, SIGNAL(clicked(const QModelIndex &)),
            this,           SLOT(callback_scenarioSingleClicked(const QModelIndex &)));

    // Hide unneeded widgets
    hideWidgets();

    // Show needed widgets
    //RunForecastPB->show();
    m_UI->ForecastInputTabWidget->show();
    //ForecastOutputTabWidget->show();
    ForecastListLBL->show();
    ForecastListLV->show();
    ScenarioListLBL->show();
    ScenarioListLV->show();

    m_UI->actionNew->setToolTip("Create a New Forecast Configuration");
    m_UI->actionNew->setStatusTip("Create a New Forecast Configuration");

} // end activateForecastWidgets


// RSK - problem is when clicking on tab, current species resets to the first in the list.

void
nmfMainWindow::ResetSpeciesList(bool withCallback)
{
    // RSK - continue here...original code
    QModelIndex index = EntityListLV->model()->index(0,0);
std::cout << "ResetSpeciesList index: " << index.data().toString().toStdString() << std::endl;

    EntityListLV->setCurrentIndex(index);

    if (withCallback) {
        callback_SSVPASingleClicked(index);
    }

} // end ResetSpeciesList


void
nmfMainWindow::ResetMSVPAConfigList(bool withCallback)
{
    QModelIndex index;
    QModelIndexList indexes;
    QAbstractItemModel *model;

    // Reselect MSVPAName
    model = EntityListLV->model();
    indexes = model->match(model->index(0,0),
                           Qt::DisplayRole,
                           QString::fromStdString(MSVPAName),
                           1,
                           Qt::MatchExactly);

    // If find MSVPAName in list get its index, otherwise get first item's index.
    if (indexes.count() > 0) {
        index = indexes[0];
    } else {
        index = EntityListLV->model()->index(0,0);
    }
    EntityListLV->setCurrentIndex(index);

    if (withCallback) {
        callback_MSVPASingleClicked(index);
    }

} // end ResetMSVPAConfigList


void
nmfMainWindow::callback_NavigatorItemExpanded(QTreeWidgetItem *item)
{
    NavigatorTree->blockSignals(true);
    NavigatorTree->clearSelection();
    NavigatorTree->blockSignals(false);
    item->setSelected(true);
    callback_NavigatorSelectionChanged();

} // end callback_NavigatorItemExpanded


void nmfMainWindow::callback_NavigatorSelectionChanged()
{
    const int DEFAULT_NAVIGATOR_ITEM = 0; // Setup
    std::string msg;
    QString parentStr;
    QString itemSelected;
    std::string tabNumStr="";
    std::string unused="";
    int index=0;
    int tab = 0;

    std::vector<std::string> species;
    m_databasePtr->getAllSpecies(m_logger,species);
    bool thereAreSpecies = (species.size() > 0);

    updateMainWindowTitle("");

    // Handle the case if the user hasn't selected anything yet.
    QList<QTreeWidgetItem *> selectedItems = NavigatorTree->selectedItems();
    if (selectedItems.count() > 0) {
       // root = NavigatorTree->indexOfTopLevelItem(selectedItems[0]);
        itemSelected = selectedItems[0]->text(0);
        parentStr.clear();
        if (selectedItems[0]->parent()) {
            parentStr = selectedItems[0]->parent()->text(0);
        }
        if (thereAreSpecies && ((itemSelected == "SSVPA") || (parentStr == "SSVPA"))) {
            SetupOutputTE->hide();
            outputWidget->tabWidget()->hide();
            outputWidget->controlsWidget()->hide();
            mainGUILayt->setStretch(2,100);
            activateSSVPAWidgets();
            if (itemSelected != "SSVPA") {
                nmfUtils::split(itemSelected.toStdString(),". ",tabNumStr,unused);
                tab = std::stoi(tabNumStr)-1;
            }
            m_UI->SSVPAInputTabWidget->setCurrentIndex(tab);
            callback_InitializePage("SSVPA",tab,true);
            ResetSpeciesList(true);

        } else if (thereAreSpecies && ((itemSelected == "MSVPA") || (parentStr == "MSVPA"))) {
            SetupOutputTE->hide();
            outputWidget->tabWidget()->show();
            outputWidget->controlsWidget()->show();
            mainGUILayt->setStretch(2,100);
            activateMSVPAWidgets();
            index = 0;
            if (itemSelected != "MSVPA") {
                nmfUtils::split(itemSelected.toStdString(),". ",tabNumStr,unused);
                index = std::stoi(tabNumStr)-1;
            }
            callback_MSVPALoadWidgets(index);
            m_UI->MSVPAInputTabWidget->blockSignals(true);
            m_UI->MSVPAInputTabWidget->setCurrentIndex(index);
            m_UI->MSVPAInputTabWidget->blockSignals(false);

            if (EntityListLV->model()->rowCount() == 0) {
                disableTabs(m_UI->MSVPAInputTabWidget);
                disableNavigatorTreeItems(nmfConstantsMSVPA::MSVPALevel);
                msg  = "\nPlease create at least 1 MSVPA configuration.";
                msg += "\n\nMSVPA configs may be created with the 1st icon in the menu bar or";
                msg += "\nby selecting File->New...";
                QMessageBox::information(this,
                                         tr("No MSVPA Configs"),
                                         tr(msg.c_str()),
                                         QMessageBox::Ok);
                return;
            }
            ResetMSVPAConfigList(true);
            MSVPAFindStoreAndSetPageStates();

            // Initialize the output gui controls and chart with db ptr and MSVPA name
            outputWidget->setDatabaseVars(m_databasePtr,
                                          m_logger, entityName(), "MSVPA",
                                          forecastName(), scenarioName());

        } else if (thereAreSpecies && ((itemSelected == "Forecast") || (parentStr == "Forecast"))) {
            SetupOutputTE->hide();
            outputWidget->tabWidget()->show();
            outputWidget->controlsWidget()->show();
            mainGUILayt->setStretch(2,100);

            activateForecastWidgets();
            if (itemSelected != "Forecast") {
                nmfUtils::split(itemSelected.toStdString(),". ",tabNumStr,unused);
                m_UI->ForecastInputTabWidget->setCurrentIndex(std::stoi(tabNumStr)-1);
            } else {
                m_UI->ForecastInputTabWidget->setCurrentIndex(0);
            }

            // Initialize the output gui controls and chart with db ptr and MSVPA name
            outputWidget->setDatabaseVars(m_databasePtr,
                                          m_logger, entityName(), "Forecast",
                                          forecastName(), scenarioName());
        } else {
            SetupOutputTE->show();
            outputWidget->tabWidget()->hide();
            outputWidget->controlsWidget()->hide();
            mainGUILayt->setStretch(1,100);

            activateSetupWidgets();

            if (itemSelected != "Setup") {
                nmfUtils::split(itemSelected.toStdString(),". ",tabNumStr,unused);
                if (! tabNumStr.empty()) {
                    m_UI->SetupInputTabWidget->setCurrentIndex(std::stoi(tabNumStr)-1);
                }
            } else {
                m_UI->SetupInputTabWidget->setCurrentIndex(0);
            }
        }

    } else {
        // Nothing selected....so as default select SSVPA
        NavigatorTree->topLevelItem(DEFAULT_NAVIGATOR_ITEM)->setSelected(true);
        if (DEFAULT_NAVIGATOR_ITEM == 1) { // RSK work on this...
            activateSSVPAWidgets();
        }
    }

} // end callback_NavigatorSelectionChanged



void
nmfMainWindow::menu_reloadCSVFiles()
{
    std::string msg;
    QMessageBox::StandardButton reply;

    msg  = "\nWarning: This will create all new CSV files for the currently selected database.";
    msg += " All previous CSV files for this project will be overwritten.\n\nThis cannot";
    msg += " be undone.\n\nOK to Continue?\n";

    reply = QMessageBox::warning(this,
                         tr("Warning"),
                         tr(msg.c_str()),
                         QMessageBox::Yes|QMessageBox::No,
                         QMessageBox::Yes);

    if (reply == QMessageBox::No)
        return;

    m_logger->logMsg(nmfConstants::Normal,"nmfMainWindow::menu_reloadCSVFiles Restoring data from database into CSV files...");

    Setup_Tab3_ptr->createTheTemplates(nmfConstantsMSVPA::AllTables,false);

    this->setCursor(Qt::WaitCursor);
    Setup_Tab3_ptr->restoreCSVFromDatabase(m_databasePtr);
    SSVPA_Tab1_ptr->restoreCSVFromDatabase();
    SSVPA_Tab2_ptr->restoreCSVFromDatabase();
    SSVPA_Tab3_ptr->restoreCSVFromDatabase();
    SSVPA_Tab4_ptr->restoreCSVFromDatabase(m_databasePtr);
    MSVPA_Tab1_ptr->restoreCSVFromDatabase(m_databasePtr);
    MSVPA_Tab2_ptr->restoreCSVFromDatabase(m_databasePtr);
    MSVPA_Tab3_ptr->restoreCSVFromDatabase(m_databasePtr);
    MSVPA_Tab4_ptr->restoreCSVFromDatabase(m_databasePtr);
    MSVPA_Tab5_ptr->restoreCSVFromDatabase(m_databasePtr);
    MSVPA_Tab6_ptr->restoreCSVFromDatabase(m_databasePtr);
    MSVPA_Tab7_ptr->restoreCSVFromDatabase(m_databasePtr);
    MSVPA_Tab8_ptr->restoreCSVFromDatabase(m_databasePtr);
    //MSVPA_Tab9_ptr->restoreCSVFromDatabase(databasePtr);  // tbd
    //MSVPA_Tab10_ptr->restoreCSVFromDatabase(databasePtr); // tbd
    MSVPA_Tab11_ptr->restoreCSVFromDatabase(m_databasePtr);
    Forecast_Tab1_ptr->restoreCSVFromDatabase(m_databasePtr);
    Forecast_Tab2_ptr->restoreCSVFromDatabase(m_databasePtr);
    Forecast_Tab3_ptr->restoreCSVFromDatabase(m_databasePtr);
    Forecast_Tab4_ptr->restoreCSVFromDatabase(m_databasePtr);
    this->setCursor(Qt::ArrowCursor);

    m_logger->logMsg(nmfConstants::Normal,"nmfMainWindow::menu_reloadCSVFiles Restoring data from database into CSV files...complete.");

} // end createNewCSVFilesFromDatabase


//boost::posix_time::seconds interval(1); // 1 second
//boost::asio::deadline_timer timer(io_service, interval);

std::string nmfMainWindow::forecastName()
{
    return ForecastListLV->currentIndex().data().toString().toStdString();
}

int nmfMainWindow::forecastFirstYear()
{
    return m_UI->ForecastInputTabWidget->findChild<QComboBox *>("Forecast_Tab1_InitialYearCMB")->currentText().toInt();
}

int nmfMainWindow::forecastNYears()
{
    return m_UI->ForecastInputTabWidget->findChild<QLineEdit *>("Forecast_Tab1_NumYearsLE")->text().toInt()+1;
}

bool nmfMainWindow::forecastIsGrowthModel()
{
    return m_UI->ForecastInputTabWidget->findChild<QCheckBox *>("Forecast_Tab1_PredatorGrowthCB")->isChecked();
}

std::string nmfMainWindow::scenarioName()
{
    return ScenarioListLV->currentIndex().data().toString().toStdString();
}

std::string
nmfMainWindow::updateCmd(const std::string &currentCmd,
    const std::vector<std::string> &outputFields)
{
    std::string cmd = currentCmd;

    cmd += "(";
    for (auto item : outputFields) {
        cmd += item + ",";
    }
    // Remove last ","
    cmd = cmd.substr(0, cmd.size()-1);
    cmd += "), ";

    return cmd;
}

void
nmfMainWindow::callback_RunForecast()
{

    callback_RunForecastClicked(true);

} // end callback_RunForecast

void
nmfMainWindow::callback_RunForecastClicked(bool checked)
{
    std::vector<std::string> outputFields;
    std::string errorMsg="";
    std::string cmd = "";
    std::string configInfo="";
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string MSVPAName    = entityName();
    std::string ForecastName = forecastName();
    std::string ScenarioName = scenarioName();
    int InitYear             = forecastFirstYear();
    int NYears               = forecastNYears()-1;
    int isGrowthModel        = forecastIsGrowthModel();
    ForecastNoPredatorGrowthStruct argStruct;

    // Show these dock widgets once user is running an MSVPA config.
    m_UI->ProgressDockWidget->show();
    m_UI->LogDockWidget->show();

    m_logger->logMsg(nmfConstants::Section,"================================================================================");
    m_logger->logMsg(nmfConstants::Bold,"Forecast Run - Begin");

    // Update Forecasts table
    cmd  = "REPLACE INTO Forecasts ";
    cmd += "(MSVPAName,ForeName,InitYear,NYears,Growth) values ";
    outputFields.clear();
    outputFields.push_back(std::string("\"")+MSVPAName+"\"");
    outputFields.push_back(std::string("\"")+ForecastName+"\"");
    outputFields.push_back(std::string(std::to_string(InitYear)));
    outputFields.push_back(std::string(std::to_string(NYears)));
    outputFields.push_back(std::string(std::to_string(isGrowthModel)));
    cmd = updateCmd(cmd,outputFields);
    // Remove last comma and space from string
    cmd = cmd.substr(0, cmd.size() - 2);
    errorMsg = m_databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        m_logger->logMsg(nmfConstants::Error,"Function: callback_RunForecastClicked: " + errorMsg);
    }


    // Flip to Tab5
    Forecast_Tab5_ptr->makeCurrent();

    // Update output window
    fields = {"FishAsF","VarF","VarOthPred","VarOthPrey","VarRec"};
    queryStr = "SELECT FishAsF,VarF,VarOthPred,VarOthPrey,VarRec FROM Scenarios WHERE MSVPAName='" + MSVPAName + "'" +
               " AND ForeName='" + ForecastName +"'" +
               " AND Scenario='" + ScenarioName + "'";
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);

    if (std::stoi(dataMap["FishAsF"][0]))
        configInfo += "Fishery removals entered as Fishing Mortality Rates<br>";
    else
        configInfo += "Fishery removals entered as Catch Limits<br>";
    if (std::stoi(dataMap["VarF"][0]))
        configInfo += "Variable Fishery Removals<br>";
    else
        configInfo += "Status Quo Fishery Removals<br>";
    if (std::stoi(dataMap["VarOthPred"][0]))
        configInfo += "Variable Biomass Predator Abundance<br>";
    else
        configInfo += "Status Quo Biomass Predator Abundance<br>";
    if (std::stoi(dataMap["VarOthPrey"][0]))
        configInfo += "Variable Other Prey Biomass<br>";
    else
        configInfo += "Status Quo Other Prey Biomass<br>";
    if (std::stoi(dataMap["VarRec"][0]))
        configInfo += "Variable Recruitment<br>";
    else
        configInfo += "Status Quo Recruitment<br>";

    // Clear Forecast progress chart
//    std::ofstream outputFile(nmfConstants::ForecastProgressChartFile);
//    outputFile.close();


    // Find number of seasons
    //fields = {"NSeasons"};
    //queryStr = "SELECT NSeasons FROM MSVPAlist WHERE MSVPAName='" + MSVPAName + "'";
    //dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    //int NSeasons = std::stoi(dataMap["NSeasons"][0]);

    // Setup 1 second periodic timer to check a file that contains progress data from algorithm.
    // Doing it this way rather than passing the QProgressBar to the algorithms continues to keep
    // separate the GUI from the algorithm.
    startForecastProgressSetupTimer();

    // Initialize the "stop" file. This file is continually checked by the MSVPA loop and once
    // ""stop" is written to the file, execution will stop.
    progressWidgetForecast->startTimer();

    // Show progress widget
    MSVPAProgressWidget->hide();
    ForecastProgressWidget->show();

    argStruct.databasePtr  = m_databasePtr;
    argStruct.MSVPAName    = MSVPAName;
    argStruct.ForecastName = ForecastName;
    argStruct.ScenarioName = ScenarioName;
    argStruct.InitYear     = InitYear;
    argStruct.NYears       = NYears;

    //
    // Can't use QSqlDatabase in a thread. Problematic. So I run in the
    // same thread and the user must wait.
    //    QFuture<void> future = QtConcurrent::run(
    //        forecastApi,
    //        &nmfForecast::Forecast_NoPredatorGrowth,
    //        argStruct);
    // Don't add any code here since you just kicked off a threaded process
    // N.B. If you try to use future, the code will block until run completes.
    //
    //std::unique_ptr<nmfForecast> forecast(new nmfForecast(databasePtr,logger));

    int MaxNum = 200;  // Just an appoximation as to how many progress updates there are for a Forecast
    if (m_ForecastProgressDlg == nullptr) {
        m_ForecastProgressDlg = new QProgressDialog(
                    "Running Forecast...",
                    "Abort Forecast", 0, MaxNum, this);
    }
    m_ForecastProgressDlg->show();

    nmfForecast forecast(m_databasePtr,m_logger);

    QObject::disconnect(&forecast,0,0,0);
    QObject::connect(&forecast, SIGNAL(UpdateForecastProgressDialog(int,QString)),
                     this,      SLOT(callback_UpdateForecastProgressDialog(int,QString)));
    QObject::connect(&forecast, SIGNAL(UpdateForecastProgressWidget()),
                     this,      SLOT(callback_UpdateForecastProgressWidget()));

    forecast.Forecast_NoPredatorGrowth(argStruct);


} // end callback_RunForecastClicked

void
nmfMainWindow::callback_UpdateMSVPAProgressWidget()
{
    readMSVPAProgressChartDataFile();
}
void
nmfMainWindow::callback_UpdateForecastProgressWidget()
{
    readForecastProgressChartDataFile();
}

void
nmfMainWindow::callback_RunMSVPA()
{
    callback_RunMSVPAClicked(true);

} // end callback_RunMSVPA



void nmfMainWindow::testFunc()
{
    std::cout << "testFunc!! "  << std::endl;
}



void nmfMainWindow::callback_RunMSVPAClicked(bool checked)
{
    std::string errorMsg;
    std::map<std::string,int> CohortAnalysisGuiData;
    std::map<std::string,int> EffortTunedGuiData;
    std::map<std::string,std::string> XSAGuiData;
    MSVPANoPredatorGrowthStruct argStruct;

    // Show these dock widgets once user is running an MSVPA config.
    m_UI->ProgressDockWidget->show();
    m_UI->LogDockWidget->show();

    m_logger->logMsg(nmfConstants::Section,"================================================================================");
    m_logger->logMsg(nmfConstants::Bold,"MSVPA Run - Begin");

    // Force current page to set and update widgets
    m_UI->MSVPAInputTabWidget->setCurrentIndex(m_UI->MSVPAInputTabWidget->count()-1);

    // Find vectors of predators and prey
    std::map<std::string,std::vector<std::string> > predPreyMap =
            m_databasePtr->nmfQueryPredatorPreyFields("MSVPAspecies", MSVPAName);

    // Update text window on Tab 12 - Output Page
    MSVPA_Tab12_ptr->outputCurrentConfiguration(
                MSVPAName, FirstYear, LastYear, NumSeasons, isGrowthModel,
                predPreyMap["predators"],
                predPreyMap["prey"],
                predPreyMap["otherPredators"]);

    std::cout << "\n*** Warning: Check for other hardcoded values from original tool.....\n\n" << std::endl;


    // Setup 1 second periodic timer to check a file that contains progress data from algorithm.
    // Doing it this way rather than passing the QProgressBar to the algorithms continues to keep
    // separate the GUI from the algorithm.
    startMSVPAProgressSetupTimer();

    getCohortAnalysisGuiData(CohortAnalysisGuiData);
    getEffortTunedGuiData(EffortTunedGuiData);
    getXSAGuiData(XSAGuiData);

    // Initialize the "stop" file. This file is continually checked by the MSVPA loop and once
    // ""stop" is written to the file, execution will stop.
    progressWidgetMSVPA->startTimer();

    // Show progress widget
    MSVPAProgressWidget->show();
    ForecastProgressWidget->hide();

    // Clear MSVPA output window
    clearMSVPAOutputWindow();

    // Disable Controls panel, since clicking on something here will cause a crash
    // since you'll be reading from and writing to the same database table.
    // RSK...tbd see where to enable this... - continue here....


    errorMsg.clear();
    if (isGrowthModel) {
        // TBD
        //numLoops = msvpaApi->MSVPA_PredatorGrowth(databasePtr, MSVPAName);
    } else {


////qDebug() << 31;
//        msvpaApi    = new nmfMSVPA(logger); // dummy line
//        std::string dummy = "";
//        QFuture<void> f1 = QtConcurrent::run(
//            msvpaApi,
//            &nmfMSVPA::dummyFunc);


        argStruct.Username                 = m_Username;
        argStruct.Hostname                 = Hostname;
        argStruct.Password                 = m_Password;
        argStruct.databasePtr              = m_databasePtr;
        argStruct.MSVPAName                = MSVPAName;
        argStruct.CohortAnalysisGuiData    = CohortAnalysisGuiData;
        argStruct.EffortTunedGuiData       = EffortTunedGuiData;
        argStruct.XSAGuiData               = XSAGuiData;


        //
        // Can't use QSqlDatabase in a thread. Problematic
        // QFuture<void> future = QtConcurrent::run(
        //    msvpaApi,
        //    &nmfMSVPA::MSVPA_NoPredatorGrowth,
        //    argStruct);
        //

        // Kick off a process that updates the GUI
//      QFuture<void> future = QtConcurrent::run(
//                msvpaApi,
//                &nmfTestUtils::ShowDialog,
//                nmfConstants::MSVPAProgressBarFile);
//      Don't do anything more here, since just kicked off a concurrent process.


        int MaxNum = 14+4+4+nmfConstantsMSVPA::MaxMSVPALoops+10;  // This is a worst case guestimate...don't really know NumLoops a prioi
        if (m_MSVPAProgressDlg == nullptr) {
            m_MSVPAProgressDlg = new QProgressDialog(
                        "Running MSVPA...",
                        "Abort MSVPA", 0, MaxNum, this);
        }
        m_MSVPAProgressDlg->show();

        // Run an MSVPA here and wait until it finishes before continuing.
        //std::unique_ptr<nmfMSVPA> msvpa(new nmfMSVPA(logger));
        nmfMSVPA msvpa(m_logger);

        QObject::disconnect(&msvpa,0,0,0);
        QObject::connect(&msvpa, SIGNAL(UpdateMSVPAProgressDialog(int,QString)),
                         this,   SLOT(callback_UpdateMSVPAProgressDialog(int,QString)));
        QObject::connect(&msvpa, SIGNAL(UpdateMSVPAProgressWidget()),
                         this,   SLOT(callback_UpdateMSVPAProgressWidget()));
        bool ok = msvpa.MSVPA_NoPredatorGrowth(argStruct);
        if (! ok) {
            MSVPAProgressWidget->hide();
        }
    }


} // end callback_RunMSVPAClicked



void
nmfMainWindow::callback_UpdateMSVPAProgressDialog(int value,
                                            QString text)
{
    if (m_MSVPAProgressDlg == nullptr) {
        return;
    }

    bool UserHitCancel = m_MSVPAProgressDlg->wasCanceled();

    if (UserHitCancel) {
        StopTheRun("MSVPA");
    }

    if (UserHitCancel || (value == -1)) {
        m_MSVPAProgressDlg->close();
        delete m_MSVPAProgressDlg;
        m_MSVPAProgressDlg = nullptr;
    }

    if (m_MSVPAProgressDlg != nullptr) {
        // Update the dialog's text and percent completed
        text = "\n\n" + text + "\n";
        m_MSVPAProgressDlg->setLabelText(text);
        m_MSVPAProgressDlg->setValue(value);
    }

}

void
nmfMainWindow::callback_UpdateForecastProgressDialog(int value,
                                               QString text)
{
    if (m_ForecastProgressDlg == nullptr) {
        return;
    }

    bool UserHitCancel = m_ForecastProgressDlg->wasCanceled();
    if (UserHitCancel) {
        StopTheRun("Forecast");
    }

    if (UserHitCancel || (value == -1)) {
        m_ForecastProgressDlg->close();
        delete m_ForecastProgressDlg;
        m_ForecastProgressDlg = nullptr;
    }

    if (m_ForecastProgressDlg != nullptr) {
        // Update the dialog's text and percent completed
        text = "\n\n" + text + "\n";
        m_ForecastProgressDlg->setLabelText(text);
        m_ForecastProgressDlg->setValue(value);
    }
}

void
nmfMainWindow::StopTheRun(std::string RunType)
{
    if (RunType == "MSVPA") {
        std::ofstream outputFile(nmfConstantsMSVPA::MSVPAStopRunFile);
        outputFile << "Stop" << std::endl;
        outputFile.close();
    } else if (RunType == "Forecast") {
        std::ofstream outputFile(nmfConstantsMSVPA::ForecastStopRunFile);
        outputFile << "Stop" << std::endl;
        outputFile.close();
    }

    m_logger->logMsg(nmfConstants::Bold,RunType + " Run - End");
    m_logger->logMsg(nmfConstants::Section,"================================================================================");
}

void
nmfMainWindow::showVPAGroupBox(int whichGroupBox)
{
    std::vector<QGroupBox *> groupBoxes =
        {m_UI->SSVPAInputTabWidget->findChild<QGroupBox *>("SSVPAConfigure1GB"),
         m_UI->SSVPAInputTabWidget->findChild<QGroupBox *>("SSVPAConfigure2GB"),
         m_UI->SSVPAInputTabWidget->findChild<QGroupBox *>("SSVPAConfigure3GB"),
         m_UI->SSVPAInputTabWidget->findChild<QGroupBox *>("SSVPAConfigure4GB")};

    for (QGroupBox *aGroupBox : groupBoxes) {
        aGroupBox->hide();
    }
    groupBoxes[whichGroupBox]->show();

} // end showVPAGroupBox


void
nmfMainWindow::getEffortTunedGuiData(std::map<std::string,int> &EffortTunedGuiData)
{
    EffortTunedGuiData["FullRecAge"] = Configure_EffortTuned_SB1->value();
    //EffortTunedGuiData["NFleets"]    = Configure_EffortTuned_SetNumFleetsCMB->currentIndex();

} // end getCohortAnalysisGuiData


void
nmfMainWindow::getCohortAnalysisGuiData(std::map<std::string,int> &CohortAnalysisGuiData)
{
    CohortAnalysisGuiData["FullRecAge"]    = Configure_Cohort_SB1->value();
    CohortAnalysisGuiData["pSVPANCatYrs"]  = Configure_Cohort_SB2->value();
    CohortAnalysisGuiData["pSVPANMortYrs"] = Configure_Cohort_SB3->value();
    CohortAnalysisGuiData["pSVPARefAge"]   = Configure_Cohort_SB4->value();

} // end getCohortAnalysisGuiData



void
nmfMainWindow::getXSAGuiData(std::map<std::string,std::string> &XSAGuiData)
{
    XSAGuiData["DownweightType"] = std::to_string(Configure_XSA_CMB4->currentIndex());
    XSAGuiData["DownweightYear"] = (! Configure_XSA_CMB5->currentText().isEmpty()) ?
                                   Configure_XSA_CMB5->currentText().toStdString() : "0";
    XSAGuiData["Downweight"]     = Configure_Downweight_GB->isChecked() ? "1" : "0";
    XSAGuiData["NXSAIndex"]      = std::to_string(SSVPAExtendedIndicesCMB->currentIndex());

    XSAGuiData["Shrink"]         = Configure_XSA_GB->isChecked() ? "1" : "0";
    XSAGuiData["ShrinkCV"]       = (! Configure_XSA_LE1->text().isEmpty()) ?
                                   Configure_XSA_LE1->text().toStdString() : "0.00";
    XSAGuiData["ShrinkYears"]    = std::to_string(Configure_XSA_NYears_SB->value());
    XSAGuiData["ShrinkAge"]      = std::to_string(Configure_XSA_NAges_SB->value());

} // end getXSAGuiData



void
nmfMainWindow::callback_RunSSVPA(std::string SSVPAType)
{
std::cout << "nmfMainWindow::callback_RunSSVPA START with type: " << SSVPAType <<
             ", Species: " << SpeciesName << ", SpeciesIndex: " << SpeciesIndex <<
             std::endl;

    m_logger->logMsg(nmfConstants::Normal,"nmfMainWindow::callback_RunSSVPA: " + SpeciesName);

    bool ok;
    int LastCatchYear;
    int NumCatchYears;
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    boost::numeric::ublas::matrix<double> Abundance;
    boost::numeric::ublas::matrix<double> M_NaturalMortality;
    boost::numeric::ublas::matrix<double> F_FishingMortality;
    QAbstractItemModel* model;
    double CV = nmfConstants::NoValueDouble;
    double SE = nmfConstants::NoValueDouble;
    int NIt   = nmfConstants::NoValueInt;
    std::vector<std::string> rowTitles;
    std::vector<std::string> colTitles;
    std::string elapsedTimeStr;
    std::map<std::string,int> CohortAnalysisGuiData;
    std::map<std::string,int> EffortTunedGuiData;
    std::map<std::string,std::string> XSAGuiData;

    // Don't allow SpeciesIndex of 0
    if (SpeciesIndex == 0)
    {
        std::string Species = entityName();
        SpeciesIndex = m_databasePtr->getSpeciesIndex(Species);
        if (SpeciesIndex < 0) {
            return;
        }
    }
    if (SSVPAType.empty()) {
        SSVPAType = SelectVPATypeCMB->currentText().toStdString();
    }

    getCohortAnalysisGuiData(CohortAnalysisGuiData);
    getEffortTunedGuiData(EffortTunedGuiData);
    getXSAGuiData(XSAGuiData);

    // Get data needed for SVPA
    int MaxAge,isPlusClass;
    int MinCatchAge, MaxCatchAge;
    std::tie(MaxAge, MinCatchAge, MaxCatchAge, isPlusClass) =
            m_databasePtr->nmfQueryAgeFields("Species", SpeciesIndex);
    //int NumCatchAges = LastCatchAge - FirstCatchAge + 1;
    int NumCatchAges = MaxAge + 1   +1; // RSK - fix this extra +1.

    if (isPlusClass)
        ReferenceAgeSB->setRange(MinCatchAge,
                MinCatchAge + (MaxCatchAge - MinCatchAge) - 2); // -2 if plus class
    else
        ReferenceAgeSB->setRange(MinCatchAge,
                MinCatchAge + (MaxCatchAge - MinCatchAge) - 1); // -1 since starts with 0th element

    // Load Catch matrix from table view
    if (! m_CatchDataInitialized) {
        callback_InitializePage("SSVPA",0,true);
        m_CatchDataInitialized = true;
    }
    model = m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPACatchAtAgeTV")->model();
    //int nrows = model->rowCount();
    //int ncols = model->columnCount();

    fields   = {"SpeName","FirstYear","LastYear"};
    queryStr = "SELECT SpeName,FirstYear,LastYear FROM Species WHERE SpeIndex = " + std::to_string(SpeciesIndex);
    dataMap  = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    int NumRecords = dataMap["SpeName"].size();
    if (NumRecords == 0) {
        m_logger->logMsg(nmfConstants::Error,"Error nmfMainWindow::callback_RunSSVPA: Couldn't find SpeIndex: "+std::to_string(SpeciesIndex));
    }
    FirstCatchYear = std::stoi(dataMap["FirstYear"][0]);
    LastCatchYear  = std::stoi(dataMap["LastYear"][0]);
    NumCatchYears  = LastCatchYear - FirstCatchYear + 1;
    boost::numeric::ublas::matrix<double> CatchMatrix;

    nmfUtils::initialize(CatchMatrix,        NumCatchYears, NumCatchAges);
    nmfUtils::initialize(Abundance,          NumCatchYears, MaxAge + 1);
    nmfUtils::initialize(M_NaturalMortality, NumCatchYears, MaxAge + 1);
    nmfUtils::initialize(F_FishingMortality, NumCatchYears, MaxAge + 1);

    for (int row = 0; row < NumCatchYears; ++row) {
        for (int column = 0; column < NumCatchAges; ++column) {
            CatchMatrix(row, column) = model->data(
                    model->index(row, column), Qt::DisplayRole).toDouble();
        }
    }


    //
    // RSK -- move the following code to Tab4!!!!!
    //
    // Define Initial Selectivity
    //boost::numeric::ublas::vector<double> InitialSelectivity;
    //nmfUtils::initialize(InitialSelectivity, NumCatchAges);


    m_logger->logMsg(nmfConstants::Normal,"nmfMainWindow::callback_RunSSVPA Loading M_NaturalMortality...");


    // Load Natural Mortality table (M)
    if (NaturalMortalityCB->isChecked()) {
        float M1_NaturalMortalityFromTable;
        float M2_PredationMortalityFromTable;
        for (auto c = 0; c <= MaxAge; ++c) {
            for (auto r = 0; r < NumCatchYears; ++r) {
                if (SSVPAMortalityTV->model()) {
                    M1_NaturalMortalityFromTable =
                            SSVPAMortalityTV->model()->index(c, 1).data().toFloat();
                    M2_PredationMortalityFromTable =
                            SSVPAMortalityTV->model()->index(c, 2).data().toFloat();
                    try {
                        M_NaturalMortality(r, c) = M1_NaturalMortalityFromTable +
                                M2_PredationMortalityFromTable;
                    } catch (std::exception &e) {
                        std::cout << "Error: " << e.what() << std::endl;
                        return;
                    }
                }
            }
        }
    } else {
        double M1_NaturalMortality;
        double M2_PredationMortality;
        M1_NaturalMortality   = ResidualNaturalMortalityDSB->value();
        M2_PredationMortality = PredationMortalityDSB->value();
        for (auto c = 0; c <= MaxAge; ++c) {
            for (auto r = 0; r < NumCatchYears; ++r) {
                try {
                    M_NaturalMortality(r, c) = M1_NaturalMortality + M2_PredationMortality;
                } catch (std::exception &e) {
                    std::cout << "Error: " << e.what() << std::endl;
                    return;
                }
            }
        }
    }

   int SSVPAIndex        = ssvpaConfigurationIndex();
   std::string SSVPAName = ssvpaConfigurationName();
   std::string VPAType   = SelectVPATypeCMB->currentText().toStdString();
   //int XSAIndex          = SSVPAExtendedIndicesCMB->currentIndex()+1;

   this->setCursor(Qt::WaitCursor);

   if (SSVPAType == nmfConstantsMSVPA::DefaultVPATypes[0]) { // "Cohort Analysis/SVPA"
       ok = SSVPA_Tab4_ptr->runSSVPA_CohortAnalysis(
                    CohortAnalysisGuiData,
                    SSVPAName,
                    FirstCatchYear,
                    LastCatchYear,
                    NumCatchYears,
                    MinCatchAge,
                    MaxCatchAge,
                    MaxAge,
                    CV, SE, NIt,
                    Abundance,
                    M_NaturalMortality,
                    F_FishingMortality,
                    rowTitles, colTitles,
                    elapsedTimeStr);
       if (! ok) {
           this->setCursor(Qt::ArrowCursor);
           return;
       }

       showSSVPAOutput(SpeciesName, elapsedTimeStr,
                   SSVPAName,
                   VPAType,
                   CV, SE, NIt, 10,
                   Abundance,
                   M_NaturalMortality,
                   F_FishingMortality,
                   rowTitles, colTitles);

   } else if (SSVPAType == nmfConstantsMSVPA::DefaultVPATypes[1]) { // "Separable VPA"

        SSVPA_Tab4_ptr->runSSVPA_SeparableVPA(
                    MinCatchAge,
                    MaxCatchAge,
                    MaxAge,
                    NumCatchYears,
                    NumCatchAges,
                    CatchMatrix,
                    CV, SE, NIt,
                    Abundance,
                    M_NaturalMortality,
                    F_FishingMortality,
                    rowTitles,
                    colTitles,
                    elapsedTimeStr);

        showSSVPAOutput(SpeciesName, elapsedTimeStr,
                   SSVPAName,
                   VPAType,
                   CV, SE, NIt, 10,
                   Abundance, M_NaturalMortality, F_FishingMortality,
                   rowTitles, colTitles);

   } else if (SSVPAType == nmfConstantsMSVPA::DefaultVPATypes[2]) { // "Laurec-Shepard Tuned VPA"

        // If its only one fleet, then the catch is the same as the
        // total catch - so put it in the data grid
        SSVPA_Tab4_ptr->runSSVPA_EffortTuned(
                    EffortTunedGuiData,
                    SSVPAIndex,
                    SSVPAName,
                    MaxAge,
                    NumCatchYears,
                    CV, SE, NIt,
                    Abundance, M_NaturalMortality, F_FishingMortality,
                    rowTitles, colTitles,
                    elapsedTimeStr);

        showSSVPAOutput(SpeciesName, elapsedTimeStr,
                   SSVPAName,
                   VPAType,
                   CV, SE, NIt, 10,
                   Abundance, M_NaturalMortality, F_FishingMortality,
                   rowTitles, colTitles);

   } else if (SSVPAType == nmfConstantsMSVPA::DefaultVPATypes[3]) { // "Extended Survivors Analysis"

       SSVPA_Tab4_ptr->runSSVPA_XSA(
                    XSAGuiData,
                    SSVPAName,
                    CV, SE, NIt,
                    Abundance, M_NaturalMortality, F_FishingMortality,
                    rowTitles, colTitles,
                    elapsedTimeStr);

       showSSVPAOutput(SpeciesName, elapsedTimeStr,
                   SSVPAName,
                   VPAType,
                   CV, SE, NIt, 10,
                   Abundance, M_NaturalMortality, F_FishingMortality,
                   rowTitles, colTitles);

    }

   // Draw chart
   updateChart(Abundance,
               M_NaturalMortality,
               F_FishingMortality,
               MinCatchAge,
               MaxCatchAge,
               FirstCatchYear,
               LastCatchYear,
               true);

   callback_SSVPAChartTypeChanged(chartTypeCMB->currentText());

   this->setCursor(Qt::ArrowCursor);

   m_logger->logMsg(nmfConstants::Normal,"nmfMainWindow::callback_RunSSVPA Complete");

} // end callback_RunSSVPA



void
nmfMainWindow::showSSVPAOutput(
        std::string species, std::string elapsedTimeStr,
        std::string Config, std::string VPAType,
        double CV, double SE, int NIt,
        int numDigits,
        boost::numeric::ublas::matrix<double> &Abundance,
        boost::numeric::ublas::matrix<double> &M_NaturalMortality,
        boost::numeric::ublas::matrix<double> &F_FishingMortality,
        std::vector<std::string> &rowTitles,
        std::vector<std::string> &colTitles)
{
    std::string CVStr  = (CV  == nmfConstants::NoValueDouble) ? "n/a" : std::to_string(CV);
    std::string SEStr  = (SE  == nmfConstants::NoValueDouble) ? "n/a" : std::to_string(SE);
    std::string NItStr = (NIt == nmfConstants::NoValueInt)    ? "n/a" : std::to_string(NIt);

    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, nmfConstants::Clear);
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "~~~~~~~~~~~~~~~~~",   "");
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "SSVPA Information",   "");
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "~~~~~~~~~~~~~~~~~",   "");
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "\nSpecies: ",           species);
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "Elapsed time: ",        elapsedTimeStr);
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "\nConfiguration: ",     Config);
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "VPA Type: ",            VPAType);
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "\nCV_coeffVariation: ", CVStr);
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "SE_standardError: ",    SEStr);
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "TotalNumIterations: ",  NItStr);
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "\nAbundance",
                                   rowTitles,colTitles,
                                   Abundance,numDigits,3);
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "F_FishingMortality",
                                   rowTitles,colTitles,
                                   F_FishingMortality,numDigits,3);
    nmfUtilsQt::sendToOutputWindow(SSVPAOutputTE, "M_NaturalMortality",
                                   rowTitles,colTitles,
                                   M_NaturalMortality,numDigits,3);

} // end showSSVPAOutput

void
nmfMainWindow::updateChart(
        boost::numeric::ublas::matrix<double> Abundance,
        boost::numeric::ublas::matrix<double> M_NaturalMortality,
        boost::numeric::ublas::matrix<double> F_FishingMortality,
        int FirstCatchAge,
        int LastCatchAge,
        int FirstCatchYear,
        int LastCatchYear,
        int updateRange)
{
    bool autoScale;


//   Update SSVPA charts with correct species data


    int tableNum = 0;
    if (chartTypeCMB->currentText() == "Fishing Mortality")
            tableNum = 1;
    else if (chartTypeCMB->currentText() == "Natural Mortality")
            tableNum = 2;

    int xmin,ymin,zmin;
    int xmax,ymax,zmax;
    int chartNum = 0;
    double maxMatrixValue;
    std::vector<std::string> chartNames = { "Abundance (millions) ", "Fishing Mortality", "Natural Mortality" };
    SSVPATables = { Abundance, F_FishingMortality, M_NaturalMortality };

    // Find max value in matrix for vertical scaling
    maxMatrixValue = nmfUtils::getMatrixMax(SSVPATables[tableNum],nmfConstants::RoundOff);
    outputCharts3D[chartNum]->maxValueLE()->setText(QLocale(QLocale::English).toString(qlonglong(maxMatrixValue)));
    outputCharts3D[chartNum]->minSliderX()->setValue(nmfConstants::MinSliderValue);
    outputCharts3D[chartNum]->maxSliderX()->setValue(nmfConstants::MaxSliderValue);
    outputCharts3D[chartNum]->maxSliderYScale()->setValue(nmfConstants::MaxSliderValue);
    outputCharts3D[chartNum]->minSliderZ()->setValue(nmfConstants::MinSliderValue);
    outputCharts3D[chartNum]->maxSliderZ()->setValue(nmfConstants::MaxSliderValue);
    autoScale = outputCharts3D[chartNum]->scaleCB()->isChecked();

    // Update the charts with new species data
    xmin = FirstCatchYear; // year
    xmax = LastCatchYear;
    ymin = 0; // the data
    // Plot the Mortalities between 0 and 1 since they're a percentage.
    //ymax = (chartNames[chartNum] != "Abundance") ? 1 : maxMatrixValue;
    ymax = maxMatrixValue;
    zmin = 0; // age
    zmax = LastCatchAge;
    outputCharts3D[chartNum]->setMaxValue(maxMatrixValue);
    outputCharts3D[chartNum]->enableSSVPAModel(true, xmin, xmax, ymin, ymax,
                                            zmin, zmax, "Year", chartNames[chartNum],
                                            "Cohort Age", autoScale, true);
    outputCharts3D[chartNum]->fillSSVPAProxy(FirstCatchYear, FirstCatchAge, SSVPATables[tableNum]);
}


QPalette
nmfMainWindow::getDarkPalette()
{
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    return darkPalette;
}



void nmfMainWindow::menu_preferences()
{

    PreferencesDialog* PreferencesDlg = new PreferencesDialog(this,m_databasePtr);

    disconnect(PreferencesDlg,0,0,0);
    connect(PreferencesDlg, SIGNAL(SetStyleSheet(QString)),
            this,           SLOT(callback_SetStyleSheet(QString)));

    if (PreferencesDlg->exec()) {  // OK pressed
        saveSettings();
    }
    ReadSettings("style");


} // end menu_preferences


void
nmfMainWindow::menu_showTableNames()
{
//    std::vector<std::string> fields;
//    std::map<std::string, std::vector<std::string> > dataMap;
//    std::string queryStr;
//    std::string msg="";
//    int NumTables=0;
//    fields = {"table_name"};
//    queryStr  = "SELECT table_name FROM information_schema.tables WHERE ";
//    queryStr += "table_schema = '" + ProjectDatabase + "'";
//    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
//    NumTables = dataMap["table_name"].size();
//    if (NumTables <= 0) {
//        msg = "\nNo tables found in database: " + ProjectDatabase;
//        QMessageBox::information(this,
//                                 tr("Database Tables"),
//                                 tr(msg.c_str()),
//                                 QMessageBox::Ok);
//    } else {
//        for (int i=0; i<NumTables; ++i) {
//            msg += std::to_string(i+1) + ". " + dataMap["table_name"][i] + "\n";
//        }
//        msg = "\nTables in database: " + ProjectDatabase + "\n\n" + msg;
//        QMessageBox::information(this,
//                                 tr("Database Tables"),
//                                 tr(msg.c_str()),
//                                 QMessageBox::Ok);
//    }
    QLabel* DatabaseNameLB = m_TableNamesWidget->findChild<QLabel*>("DatabaseNameLB");
    DatabaseNameLB->setText(QString::fromStdString(ProjectDatabase));
    m_TableNamesDlg->show();
} // end menu_showTableNames

void
nmfMainWindow::initializeTableNamesDlg()
{

    QUiLoader loader;
    QFile file(":/forms/Main/TableNamesDlg.ui");
    file.open(QFile::ReadOnly);
    m_TableNamesWidget = loader.load(&file,this);
    file.close();

    QPushButton* TableNamesOkPB = m_TableNamesWidget->findChild<QPushButton*>("TableNamesOkPB");
    QListWidget* TableNamesLW   = m_TableNamesWidget->findChild<QListWidget*>("TableNamesLW");
    QLabel*      DatabaseNameLB = m_TableNamesWidget->findChild<QLabel*>("DatabaseNameLB");

    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    int NumTables=0;

    fields    = {"table_name"};
    queryStr  = "SELECT table_name FROM information_schema.tables WHERE ";
    queryStr += "table_schema = '" + ProjectDatabase + "'";
    dataMap   = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    NumTables = dataMap["table_name"].size();

    if (NumTables <= 0) {
        TableNamesLW->addItem(QString::fromStdString("No tables found in database: " + ProjectDatabase));
    } else {
        DatabaseNameLB->setText(QString::fromStdString(ProjectDatabase));
        for (int i=0; i<NumTables; ++i) {
            TableNamesLW->addItem(QString::fromStdString(std::to_string(i+1) + ". " + dataMap["table_name"][i]));
        }
    }

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(m_TableNamesWidget);

    m_TableNamesDlg->adjustSize();
    m_TableNamesDlg->setMinimumWidth(400);
    m_TableNamesDlg->setMinimumHeight(300);
    m_TableNamesDlg->setLayout(layout);
    m_TableNamesDlg->setWindowTitle("Table Names");

    connect(TableNamesOkPB, SIGNAL(clicked()),
            this,           SLOT(callback_TableNamesOkPB()));
}

void
nmfMainWindow::callback_TableNamesOkPB()
{
    m_TableNamesDlg->hide();
}

void
nmfMainWindow::menu_clearSSVPANonSpeciesTables()
{
    // Clear the database tables
    clearDatabaseTables("SSVPA",nmfConstantsMSVPA::SSVPANonSpeciesTables);

    // Set the Nav Tree back to the origin
    QModelIndex index = NavigatorTree->model()->index(0,0);
    NavigatorTree->setCurrentIndex(index);

    // Re-create the templates for the deleted table CSV files
    Setup_Tab3_ptr->createTheTemplates(nmfConstantsMSVPA::SSVPANonSpeciesTables,false);

    // Clear SSVPA widgets
    SSVPA_Tab1_ptr->clearWidgets();
    SSVPA_Tab2_ptr->clearWidgets();
    SSVPA_Tab3_ptr->clearWidgets();
    SSVPA_Tab4_ptr->clearWidgets();

} // end menu_clearSSVPATables


void
nmfMainWindow::menu_clearMSVPATables()
{
    // Clear the database tables
    clearDatabaseTables("MSVPA",nmfConstantsMSVPA::MSVPATables);

    // Remove the MSVPA config names from the entity list since they've been deleted from the tables.
    EntityListLV->model()->removeRows(0, EntityListLV->model()->rowCount());

    // Re-create the templates for the deleted table CSV files
    Setup_Tab3_ptr->createTheTemplates(nmfConstantsMSVPA::MSVPATables,false);

    // Clear MSVPA widgets
    MSVPA_Tab1_ptr->clearWidgets();
    MSVPA_Tab2_ptr->clearWidgets();
    MSVPA_Tab3_ptr->clearWidgets();
    MSVPA_Tab4_ptr->clearWidgets();
    MSVPA_Tab5_ptr->clearWidgets();
    MSVPA_Tab6_ptr->clearWidgets();
    MSVPA_Tab7_ptr->clearWidgets();
    MSVPA_Tab8_ptr->clearWidgets();
    MSVPA_Tab9_ptr->clearWidgets();
    MSVPA_Tab10_ptr->clearWidgets();
    MSVPA_Tab11_ptr->clearWidgets();
    MSVPA_Tab12_ptr->clearWidgets();

    // Select the 1st MSVPA item in the Navigation Tree
    QModelIndex index = EntityListLV->model()->index(0,0);
    EntityListLV->setCurrentIndex(index);
    callback_MSVPASingleClickedGUI(index);

    // Disable first tab until user creates the first MSVPA config
    disableTabs(m_UI->MSVPAInputTabWidget);
    disableNavigatorTreeItems(nmfConstantsMSVPA::MSVPALevel);

} // end menu_clearMSVPATables

void
nmfMainWindow::disableTabs(QTabWidget *tabWidget)
{
    tabWidget->blockSignals(true);
    for (int i=0;i<tabWidget->count(); ++i)
        tabWidget->setTabEnabled(i,false);
    tabWidget->blockSignals(false);

    //RunMSVPAPB->setEnabled(false);

} // end disableTabs



//  Disable all of the items under the passed in level.
//  level = 1 => SSVPA
//  level = 2 => MSVPA
//  level = 3 => Forecast

void
nmfMainWindow::disableNavigatorTreeItems(int level)
{
    NavigatorTree->blockSignals(true);
    NavigatorTree->topLevelItem(level)->setExpanded(true);
    for (int i=0; i<NavigatorTree->topLevelItem(level)->childCount(); ++i) {
        NavigatorTree->topLevelItem(level)->child(i)->setDisabled(true);
    }
    NavigatorTree->topLevelItem(level)->child(0)->setSelected(true);
    NavigatorTree->topLevelItem(level)->child(0)->setDisabled(false);
    NavigatorTree->blockSignals(false);

} // end disableNavigationItems


void
nmfMainWindow::menu_clearForecastTables()
{
    // Clear the database tables
    clearDatabaseTables("Forecast",nmfConstantsMSVPA::ForecastTables);

    // Re-create the templates for the deleted table CSV files
    Setup_Tab3_ptr->createTheTemplates(nmfConstantsMSVPA::ForecastTables,false);

} // end menu_clearForecastTables


void
nmfMainWindow::menu_clearAllNonSpeciesTables()
{
    // Clear the database tables
    clearDatabaseTables("AllNonSpecies",nmfConstantsMSVPA::AllNonSpeciesTables);

    // Set the Nav Tree back to the origin
    QModelIndex index = NavigatorTree->model()->index(0,0);
    NavigatorTree->setCurrentIndex(index);

    // Re-create the templates for the deleted table CSV files
    Setup_Tab3_ptr->createTheTemplates(nmfConstantsMSVPA::AllNonSpeciesTables,false);

} // end menu_clearAllNonSpeciesTables


void
nmfMainWindow::menu_clearAllTables()
{
    clearDatabaseTables("All",nmfConstantsMSVPA::AllTables);

    QModelIndex index = NavigatorTree->model()->index(0,0);
    NavigatorTree->setCurrentIndex(index);

    // Need to clear the tables like so, since there are no CSV files for them.
    // This may change in the future and so if there are CSV files for these two
    // table widgets, the following lines would be unnecessary.
    findChild<QTableWidget *>("Setup_Tab3_SpeciesTW")->setRowCount(0);
    findChild<QTableWidget *>("Setup_Tab3_OtherPredatorsTW")->setRowCount(0);

    // Can't create the templates because the Species tables have been
    // deleted and the user needs to create the Species before the templates.
    // Re-run the templates for the deleted table CSV files
    //Setup_Tab3_ptr->createTheTemplates(nmfConstants::AllTables,false);

} // end menu_clearAllTables



void
nmfMainWindow::clearDatabaseTables(std::string type,
                                   const std::vector<std::string> &Tables)
{
    int NumToClear = Tables.size();
    int NumTablesCleared = 0;
    int NumCSVFilesCleared = 0;
    int progressInc=0;
    bool clearedOK = true;
    QString qcmd;
    std::string errorMsg;
    std::string msg;
    std::string title = "Clear " + type + " Tables from: " + ProjectDatabase;

    if (type == "All")
        msg  = "\nAre you sure you want to clear all " + std::to_string(NumToClear) +
                " CSV files and database tables?\n";
    else if (type == "AllNonSpecies")
        msg  = "\nAre you sure you want to clear all " + std::to_string(NumToClear) +
                " non-Species CSV files and database tables?\n";
    else
        msg  = "\nAre you sure you want to clear all " + std::to_string(NumToClear) + " " +
                type + " CSV files and database tables?\n";
    msg += "\nThis cannot be undone.\n";
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                             tr(title.c_str()),
                             tr(msg.c_str()),
                             QMessageBox::No|QMessageBox::Yes,
                             QMessageBox::No);

    if (reply == QMessageBox::Yes) {

        QProgressDialog progressDlg("Clearing...","Cancel",0,Tables.size(),this);
        progressDlg.setWindowModality(Qt::WindowModal);
        progressDlg.show();
        for (std::string tableToClear : Tables)
        {
            progressDlg.setValue(progressInc++);
            if (progressDlg.wasCanceled())
                break;
            QApplication::processEvents();

            // Clear database table
            qcmd = "TRUNCATE TABLE " + QString::fromStdString(tableToClear);
            errorMsg = m_databasePtr->nmfUpdateDatabase(qcmd.toStdString());
            if (nmfUtilsQt::isAnError(errorMsg)) {
                nmfUtils::printError("menu_clear"+type+"Tables: Clearing table error: ",
                                     errorMsg+": "+tableToClear);
                continue;
            }
            ++NumTablesCleared;

            // Clear CSV file
            errorMsg.clear();
            clearedOK = nmfUtilsQt::clearCSVFile(tableToClear,m_ProjectDir,errorMsg);
            if (clearedOK) {
                ++NumCSVFilesCleared;
            } else {
                if (! errorMsg.empty()) {
                    nmfUtils::printError("menu_clear"+type+"Tables: Clearing csv file error: ",
                                         errorMsg+": "+tableToClear);
                }
            }
        } // end for

        progressDlg.setValue(Tables.size());

        msg = "\n" + type + " data cleared for database:  " + ProjectDatabase + "\n";
        msg += "\n" + std::to_string(NumCSVFilesCleared) + " CSV file(s) cleared.";
        msg += "\n" + std::to_string(NumTablesCleared) + " table(s) cleared.\n";
        QMessageBox::question(this,
                              tr(title.c_str()),
                              tr(msg.c_str()),
                              QMessageBox::Ok);
    } // end if reply is Yes

} // end clearDatabaseTables



void nmfMainWindow::callback_closePrefDlg()
{

    prefDlg->hide();
    //delete prefDlg;

} // end closePrefDlg


void nmfMainWindow::callback_schemeLight() {

    qApp->setPalette(this->style()->standardPalette());
    qApp->setStyle(QStyleFactory::create("WindowsDefault"));
    qApp->setStyleSheet("");
    qApp->setStyleSheet(QString::fromUtf8("QComboBox:enabled"
    "{ color: gray }"
    ));
    qApp->setStyleSheet(QString::fromUtf8("QComboBox:disabled"
    "{ color: gray }"
    ));
    qApp->setStyleSheet(QString::fromUtf8("QSpinBox:enabled"
    "{ color: lightgray }"
    ));
    qApp->setStyleSheet(QString::fromUtf8("QSpinBox:disabled"
    "{ color: lightgray }"
    ));
    qApp->setStyleSheet(QString::fromUtf8("QLineEdit::enabled"
    "{ color: lightgray }"
    ));
    qApp->setStyleSheet(QString::fromUtf8("QLineEdit::disabled"
    "{ color: lightgray }"
    ));
    qApp->setStyleSheet(QString::fromUtf8("QPushButton:enabled"
    "{ color: gray }"
    ));
    qApp->setStyleSheet(QString::fromUtf8("QPushButton:disabled"
    "{ color: gray }"
    ));
    //NavigatorTree->headerItem()->setBackground(0, navigatorBackgroundBrush);

   // NavigatorTree->headerItem()->setBackground(0, QBrush(QColor(225,225,225)));


//    std::cout << "Not yet fully implemented" << std::endl;
//    return;

//    qApp->setPalette(this->style()->standardPalette());
//    qApp->setStyleSheet("");

//    RunMSVPAPB->setStyleSheet("");
//    RunSSVPAPB->setStyleSheet("");
//    RunForecastPB->setStyleSheet("");
//    SelectVPATypeCMB->setStyleSheet("");
//    SelectConfigurationCMB->setStyleSheet("");
//    ResidualNaturalMortalitySB->setStyleSheet("");
//    PredationMortalitySB->setStyleSheet("");
//    LastYearMortalityRateSB->setStyleSheet("");
//    ReferenceAgeSB->setStyleSheet("");
//    SelectivitySL->setStyleSheet("");
//    NaturalMortalitySL->setStyleSheet("");
//    SSVPAMortalityTV->horizontalHeader()->setStyleSheet("");
//    SSVPAMortalityTV->verticalHeader()->setStyleSheet("");
//    SSVPAInitialSelectivityTV->horizontalHeader()->setStyleSheet("");
//    SSVPAInitialSelectivityTV->verticalHeader()->setStyleSheet("");
//    NavigatorTree->headerItem()->setBackground(0, QBrush(QColor(215,215,215))); // navigatorBackgroundBrush);
//    sliderYMaxValueLE->setStyleSheet("");
}

//void nmfMainWindow::callback_schemeDark() {

//    //navigatorBackgroundBrush = NavigatorTree->headerItem()->background(0);
//    qApp->setStyleSheet(QString::fromUtf8("QSpinBox:enabled"
//    "{ color: darkgray }"
//    ));
//    qApp->setStyleSheet(QString::fromUtf8("QSpinBox:disabled"
//    "{ color: darkgray }"
//    ));

//    // The following came from Jorgen-VikingGod under the MIT license.
//    qApp->setStyle(QStyleFactory::create("Fusion"));
//    QPalette darkPalette;
//    darkPalette.setColor(QPalette::Window,QColor(53,53,53));
//    darkPalette.setColor(QPalette::WindowText,Qt::white);
//    darkPalette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
//    darkPalette.setColor(QPalette::Base,QColor(42,42,42));
//    darkPalette.setColor(QPalette::AlternateBase,QColor(66,66,66));
//    darkPalette.setColor(QPalette::ToolTipBase,Qt::white);
//    darkPalette.setColor(QPalette::ToolTipText,Qt::white);
//    darkPalette.setColor(QPalette::Text,Qt::white);
//    darkPalette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
//    darkPalette.setColor(QPalette::Dark,QColor(35,35,35));
//    darkPalette.setColor(QPalette::NoRole,QColor(35,35,35));
//    darkPalette.setColor(QPalette::Shadow,QColor(20,20,20));
//    darkPalette.setColor(QPalette::Button,QColor(53,53,53));
//    darkPalette.setColor(QPalette::Disabled,QPalette::Button,QColor(53,53,53));
//    darkPalette.setColor(QPalette::Inactive,QPalette::Button,QColor(53,53,53));
//    darkPalette.setColor(QPalette::Active,QPalette::Button,QColor(53,53,53));
//    darkPalette.setColor(QPalette::ButtonText,Qt::white);
//    darkPalette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
//    darkPalette.setColor(QPalette::BrightText,Qt::red);
//    darkPalette.setColor(QPalette::Link,QColor(42,130,218));
//    darkPalette.setColor(QPalette::Highlight,QColor(42,130,218));
//    darkPalette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
//    darkPalette.setColor(QPalette::HighlightedText,Qt::white);
//    darkPalette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));
//    qApp->setPalette(darkPalette);
//    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
//    qApp->setStyleSheet(QString::fromUtf8("QPushButton:disabled"
//    "{ color: gray }"
//    ));
//    qApp->setStyleSheet(QString::fromUtf8("QComboBox:enabled"
//    "{ color: darkgray }"
//    ));

//    NavigatorTree->headerItem()->setBackground(0, QBrush(QColor(25,25,25)));


//    //std::cout << "Not yet fully implemented" << std::endl;
//    return;

//} // end callback_schemeDark



void nmfMainWindow::menu_about()
{
    QString name = "Multi-Species Virtual Population Analysis 2nd Version";
    QString version = QString("MSVPA_X2 v0.9.4 (beta)"); // + "&alpha;";
    QString specialAcknowledgement = "<br><br>This code is a C++ implementation of the Visual Basic code written by Dr. Lance Garrison.";
    QString msg = "";
    QString cppVersion = "C++??";
    QString mysqlVersion = "?";
    QString boostVersion = "?";
    QString qdarkLink;
    QString linuxDeployLink;
    QString boostLink;
    QString mysqlLink;
    QString qtLink;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::string queryStr;
    QString os = QString::fromStdString(nmfUtils::getOS());

    // Define Qt link
    qtLink = QString("<a href='https://www.qt.io'>https://www.qt.io</a>");

    // Find C++ version in case you want it later
    if (__cplusplus == 201103L)
        cppVersion = "C++11";
    else if (__cplusplus == 201402L)
        cppVersion = "C++14";
    else if (__cplusplus == 201703L)
        cppVersion = "C++17";

    // MySQL version and link
    fields   = {"version()"};
    queryStr = "SELECT version()";
    dataMap  = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["version()"].size() > 0) {
        mysqlVersion = QString::fromStdString(dataMap["version()"][0]);
    }
    mysqlLink = QString("<a href='https://www.mysql.com'>https://www.mysql.com</a>");

    // Boost version and link
    boostVersion = QString::number(BOOST_VERSION / 100000) + "." +
                   QString::number(BOOST_VERSION / 100 % 1000) + "." +
                   QString::number(BOOST_VERSION / 100);
    boostLink = QString("<a href='https://www.boost.org'>https://www.boost.org</a>");

    // QDarkStyle link
    qdarkLink = QString("<a href='https://github.com/ColinDuquesnoy/QDarkStyleSheet'>https://github.com/ColinDuquesnoy/QDarkStyleSheet</a>");

    // linuxdeployqt link
    linuxDeployLink = QString("<a href='https://github.com/probonopd/linuxdeployqt'>https://github.com/probonopd/linuxdeployqt</a>");

    // Build About message
  //msg += QString("<li>")+cppVersion+QString("</li>");
    msg += QString("<li>")+QString("Qt ")+QString::fromUtf8(qVersion())+QString("<br>")+qtLink+QString("</li>");
    msg += QString("<li>")+QString("MySQL ")+mysqlVersion+QString("<br>")+mysqlLink+QString("</li>");
    msg += QString("<li>")+QString("Boost ")+boostVersion+QString("<br>")+boostLink+QString("</li>");
    msg += QString("<li>")+QString("QDarkStyleSheet 2.6.5 - Colin Duquesnoy (MIT License)<br>")+qdarkLink+QString("</li>");
    msg += QString("<li>")+QString("linuxdeployqt 6 (January 27, 2019)<br>")+linuxDeployLink+QString("</li>");
    msg += QString("</ul>");

    // Show the message
//   QMessageBox::about(this,tr("About MSVPA_X2"),tr(msg.toLatin1()));

    nmfUtilsQt::showAboutWidget(this,name,os,version,specialAcknowledgement,msg);

}


void
nmfMainWindow::menu_newMSVPA()
{
    std::string errorMsg;
    std::string cmd;
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    bool ok;
    bool MSVPAexists=false;
    int NumRecords;
    int SpeIndex;
    std::string SpeName;
    std::string msg;

    QString NewMSVPAName = QInputDialog::getText(this,
         tr("New MSVPA"),
         tr("Enter new MSVPA configuration name:"),
         QLineEdit::Normal,
         "", &ok);
    if (! ok || NewMSVPAName.isEmpty())
        return;

    // Get list of all MSVPA names and see if configuration already exists.
    fields   = {"MSVPAName"};
    queryStr = "SELECT MSVPAName FROM MSVPAlist";
    dataMap  = m_databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["MSVPAName"].size();
    for (int i=0; i< NumRecords; ++i) {
        if (NewMSVPAName == QString::fromStdString(dataMap["MSVPAName"][i])) {
            MSVPAexists = true;
            break;
        }
    }

    msg  = "\nThe following MSVPA configuration already exists: " + NewMSVPAName.toStdString();
    msg += "\nPlease enter another name.";
    if (MSVPAexists) {
        QMessageBox::question(this,
            tr("MSVPA Exists"),
            tr(msg.c_str()),
            QMessageBox::Ok);
        menu_newMSVPA();
    } else {

        // Add the new MSVPA configuration to: MSVPAlist
        cmd  = "INSERT INTO MSVPAlist ";
        cmd += "(MSVPAName,NSpe,NPreyOnly,NOther,NOtherPred,FirstYear,LastYear,NSeasons,AnnTemps,SeasSpaceO,GrowthModel,Complete) values ";
        cmd += "(\"" + NewMSVPAName.toStdString() + "\", " +
                "0,0,0,0,0,0,0,0,0,0,0)";
        errorMsg = m_databasePtr->nmfUpdateDatabase(cmd);
        if (nmfUtilsQt::isAnError(errorMsg)) {
            std::cout << cmd << std::endl;
            m_logger->logMsg(nmfConstants::Error,"menu_newMSVPA: INSERT INTO MSVPAlist: " + errorMsg);
        }

        // Next add the species from Species,OtherPredSpecies to MSVPAspecies

        fields   = {"SpeIndex","SpeName"};
        for (std::string table : {"Species","OtherPredSpecies"})
        {
            queryStr = "SELECT SpeIndex,SpeName FROM " + table ;
            dataMap  = m_databasePtr->nmfQueryDatabase(queryStr, fields);
            for (unsigned int i=0; i<dataMap["SpeIndex"].size(); ++i) {
                SpeIndex = std::stoi(dataMap["SpeIndex"][i]);
                SpeName  = dataMap["SpeName"][i];

                cmd  = "INSERT INTO MSVPAspecies ";
                cmd += "(MSVPAName,SpeName,SpeIndex) values ";
                cmd += "(\"" + NewMSVPAName.toStdString() + "\", " +
                        "\"" + SpeName + "\", " +
                        std::to_string(SpeIndex) + ") ";
                cmd += "ON DUPLICATE KEY UPDATE ";
                cmd += "SpeIndex=values(SpeIndex); ";
                errorMsg = m_databasePtr->nmfUpdateDatabase(cmd);
                if (nmfUtilsQt::isAnError(errorMsg)) {
                    std::cout << cmd << std::endl;
                    m_logger->logMsg(nmfConstants::Error,"menu_newMSVPA: INSERT INTO MSVPAspecies: "+errorMsg);
                }
            }

        } // end for


        MSVPA_Tab1_ptr->loadWidgets(m_databasePtr,
                                    NewMSVPAName.toStdString());


        // Refresh the MSVPA list view...
        callback_NavigatorSelectionChanged();

        // Select the MSVPA Configuration just created
        int NumMSVPAConfigs = EntityListLV->model()->rowCount();
        QModelIndex index = EntityListLV->model()->index(NumMSVPAConfigs-1,0);
        EntityListLV->setCurrentIndex(index);
        callback_MSVPASingleClickedGUI(index);

        // Enable the first tab
        m_UI->MSVPAInputTabWidget->blockSignals(true);
        m_UI->MSVPAInputTabWidget->setTabEnabled(0,true);
        m_UI->MSVPAInputTabWidget->blockSignals(false);
    }

} // end menu_newMSVPA


void nmfMainWindow::menu_newForecast()
{
    // Make sure user has selected an MSVPA Configuration.
    if (EntityListLV->selectionModel()->selectedIndexes().count() == 0) {
        QMessageBox::information(this,
                                 tr("New Forecast"),
                                 tr("\nPlease select an MSVPA Configuration."),
                                 QMessageBox::Ok);
        return;
    }

    m_UI->ForecastInputTabWidget->setCurrentIndex(0);
    QMessageBox::information(this,
                             tr("New Forecast"),
                             tr("\nEnter data on tab \"1. Forecast Configuration.\""),
                             QMessageBox::Ok);
    ForecastListLV->selectionModel()->clear();
    ForecastListLV->selectionModel()->clearSelection();
    ScenarioListLV->selectionModel()->clear();
    ScenarioListLV->selectionModel()->clearSelection();
    ForecastListLV->clearFocus();
    ScenarioListLV->clearFocus();
    Forecast_Tab1_ptr->setupNewForecast(m_databasePtr,
                                        entityName());

std::cout << "menu_newForecast" << std::endl;

} // end menu_newForecast


void nmfMainWindow::menu_new() {

    std::string itemToCreate = modelName();

//    if (itemToCreate == "Setup") {
//        itemToCreate = "Project";
//    } else if (itemToCreate == "SSVPA") {
//        menu_newSpeciesChooser();
//    } else
    if (itemToCreate == "MSVPA") {
        menu_newMSVPA();
    } else if (itemToCreate == "Forecast") {
        menu_newForecast();
    }

} // end menu_new


bool
nmfMainWindow::validateArgs(MSVPASpeciesStruct args)
{
    if (args.Species.empty())
        return false;
    if ((args.LastAge == 0) || (args.LastYear == 0))
        return false;
    if (args.FirstYear*args.LastYear*args.EarliestAge*args.LastAge < 0)
        return false;
    if (args.FirstYear >= args.LastYear)
        return false;
    if (args.EarliestAge >= args.LastAge)
        return false;

    return true;
} // end validateArgs


void nmfMainWindow::menu_deleteSpecies()
{
    std::string currentSpecies;
    std::string msg;
    std::string cmd;
    std::string errorMsg;
    std::string theModelName = modelName();
    QMessageBox::StandardButton reply;

    if (theModelName == "SSVPA") {
        currentSpecies = entityName();
        if (currentSpecies.empty()) {
            QMessageBox::question(this, tr("Delete"),
                 tr("\nPlease select a species to delete."),
                 QMessageBox::Ok);
        } else {
            msg = "\nOK to delete species: " + currentSpecies + " ?\n";
            reply = QMessageBox::question(this, tr("Delete"),
                                          tr(msg.c_str()),QMessageBox::No|QMessageBox::Yes);
            if (reply == QMessageBox::Yes) {
                cmd = "DELETE FROM Species WHERE SpeName='" + currentSpecies + "'";
                errorMsg = m_databasePtr->nmfUpdateDatabase(cmd);
                if (nmfUtilsQt::isAnError(errorMsg)) {
                    m_logger->logMsg(nmfConstants::Error,"menu_deleteSpecies: DELETE FROM Species: " + errorMsg);
                }
            } else {
                return;
            }
        }
    } else if (theModelName == "MSVPA") {
std::cout << "TBD" << std::endl;
    } else if (theModelName == "Forecast") {
std::cout << "TBD" << std::endl;
    } else {
        msg = "\nPlease pick either a Species, MSVPA, or Forecast to delete.\n";
        QMessageBox::question(this, tr("Delete"),
             tr(msg.c_str()),QMessageBox::Ok);
    }

} // end menu_deleteSpecies


void nmfMainWindow::menu_deleteMSVPA()
{
    std::string theModelName = modelName();

    if (theModelName == "MSVPA") {
        deleteTheMSVPA();
    } else {
        QMessageBox::question(this,
                              tr("Delete MSVPA"),
                              tr("\nPlease select an MSVPA configuration to delete."),
                              QMessageBox::Ok);
    }

} // end menu_deleteMSVPA


void nmfMainWindow::menu_deleteForecast()
{

std::cout << "TBD" << std::endl;

} // end menu_deleteForecast


void nmfMainWindow::deleteTheMSVPA()
{
    std::string cmd;
    std::string errorMsg;
    std::string msg;
    QMessageBox::StandardButton reply;
    int index=0;
    QModelIndex mindex;

    for (int i=0;i<EntityListLV->model()->rowCount();++i) {
        mindex = EntityListLV->model()->index(i,0);
        if (MSVPAName == EntityListLV->model()->data(mindex).toString().toStdString())
            index = i;
    } // end for


    msg  = "\nOK to permanently delete this MSVPA Configuration and all\n";
    msg += "assocated Forecasts and Scenarios?\n\nThis cannot be undone.\n";
    reply = QMessageBox::question(this, tr("Delete MSVPA"),
                                  tr(msg.c_str()),
                                  QMessageBox::No|QMessageBox::Yes);
    if (reply == QMessageBox::Yes) {
        for (std::string table : nmfConstantsMSVPA::MSVPATables) {
            cmd  = "DELETE FROM " + table;
            cmd += " WHERE MSVPAName = '" + MSVPAName + "' ";
            errorMsg = m_databasePtr->nmfUpdateDatabase(cmd);
            if (nmfUtilsQt::isAnError(errorMsg)) {
                msg = "deleteTheMSVPA: DELETE FROM " + table + ": ";
                m_logger->logMsg(nmfConstants::Error,msg+errorMsg);
            }
        }
    } // end if

    callback_NavigatorSelectionChanged();

    // Select the MSVPA config at position: index
    if  (index >= EntityListLV->model()->rowCount())
        index = EntityListLV->model()->rowCount()-1;
    if (index < 0)
        index = 0;
    mindex = EntityListLV->model()->index(index,0);
    EntityListLV->setCurrentIndex(mindex);
    callback_MSVPASingleClickedGUI(mindex);


} // end deleteTheMSVPA


void nmfMainWindow::menu_delete()
{
    QMessageBox::StandardButton reply;
    std::string theModelName = modelName();
    std::string forecast = forecastName();
    std::string scenario = scenarioName();
    int numScenarios = ScenarioListLV->model()->rowCount();

    if (theModelName == "SSVPA") {

std::cout << "TBD" << std::endl;

    } else if (theModelName == "MSVPA") {

        deleteTheMSVPA();

    } else if (theModelName == "Forecast") {
         if (forecast == "") {
             QMessageBox::warning(this,
                     tr("Delete Forecast"),
                     tr("\nPlease select a Forecast to delete."),
                     QMessageBox::Ok);
         } else if ((scenario == "") && (numScenarios > 1)) {
//             reply = QMessageBox::warning(this,
//                     tr("Delete Forecast"),
//                     tr("\nDo you want to delete all Scenarios for this Forecast?"),
//                     QMessageBox::No|QMessageBox::Yes);
//             if (reply == QMessageBox::Yes)
                 deleteTheForecast(forecast);
         } else if ((scenario == "") && (numScenarios == 1)) {
             reply = QMessageBox::warning(this,
                     tr("Delete Forecast"),
                     tr("\nOk to delete the selected Forecast and the unselected Scenario?"),
                     QMessageBox::No|QMessageBox::Yes);
             if (reply == QMessageBox::Yes)
                 deleteTheForecast(forecast,scenario,numScenarios);
         } else if ((scenario != "") && (numScenarios > 1)) {
             reply = QMessageBox::warning(this,
                     tr("Delete Forecast"),
                     tr("\nOk to delete the selected Scenario?"),
                     QMessageBox::No|QMessageBox::Yes);
             if (reply == QMessageBox::Yes)
                 deleteTheForecast(forecast,scenario,numScenarios);
         } else if ((scenario != "") && (numScenarios == 1)) {
             reply = QMessageBox::warning(this,
                     tr("Delete Forecast"),
                     tr("\nOk to delete the selected Forecast and Scenario?"),
                     QMessageBox::No|QMessageBox::Yes);
             if (reply == QMessageBox::Yes)
                 deleteTheForecast(forecast,scenario,numScenarios);
         }
    }

} // end menu_delete

void nmfMainWindow::clearForecastTables(std::string MSVPAName, std::string Forecast)
{
    std::string msg;
    std::string cmd;
    std::string errorMsg;
    QMessageBox::StandardButton reply;

    msg  = "\nOK to permanently delete this Forecast and all\n";
    msg += "assocated Scenario and Output tables?\n\nThis cannot be undone.\n";
    reply = QMessageBox::question(this, tr("Clear Forecast Tables"),
                                  tr(msg.c_str()),
                                  QMessageBox::No|QMessageBox::Yes);
    if (reply == QMessageBox::Yes) {
        for (std::string table : nmfConstantsMSVPA::ForecastTables) {
            cmd  = "DELETE FROM " + table;
            cmd += " WHERE MSVPAName = '" + MSVPAName + "' " +
                    "AND ForeName = '" + Forecast + "' ";
            errorMsg = m_databasePtr->nmfUpdateDatabase(cmd);
            if (nmfUtilsQt::isAnError(errorMsg)) {
                m_logger->logMsg(nmfConstants::Error,"clearForecastTables: DELETE FROM Scenarios: "+errorMsg);
            }
        }
    } // end if

} // end clearForecastTables

void nmfMainWindow::deleteTheForecast(std::string forecast)
{

std::cout << "Deleting all scenarios and the forecast" << std::endl;
    clearForecastTables(MSVPAName,forecast);

    loadForecastListWidget();
    callback_UpdateScenarioList("");

} // end deleteTheForecast

void nmfMainWindow::deleteTheForecast(std::string Forecast,
                                      std::string Scenario,
                                      int numScenarios)
{
    std::cout << "Deleting the scenario and the forecast" << std::endl;

    std::string cmd;
    std::string errorMsg;

    // Delete the scenario
    cmd  = "DELETE FROM Scenarios ";
    cmd += "WHERE MSVPAName = '" + MSVPAName + "' " +
           "AND ForeName = '" + Forecast + "' " +
           "AND Scenario = '" + Scenario + "'";
//std::cout << "cmd1: " << cmd << std::endl;
    errorMsg = m_databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        m_logger->logMsg(nmfConstants::Error,"deleteTheForecast: DELETE FROM Scenarios: "+errorMsg);
    }

    // And if there was only 1 scenario, delete the forecast
    if (numScenarios == 1) {
        cmd  = "DELETE FROM Forecasts ";
        cmd += "WHERE MSVPAName = '" + MSVPAName + "' " +
                "AND ForeName = '" + Forecast + "' ";
//std::cout << "cmd2: " << cmd << std::endl;
        errorMsg = m_databasePtr->nmfUpdateDatabase(cmd);
        if (nmfUtilsQt::isAnError(errorMsg)) {
            m_logger->logMsg(nmfConstants::Error,"deleteTheForecast: DELETE FROM Forecasts..."+errorMsg);
        }
        clearForecastTables(MSVPAName,Forecast);
    }

    loadForecastListWidget();
    callback_UpdateScenarioList("");


} // end deleteTheForecast


void nmfMainWindow::menu_openProject() {

    Setup_Tab2_ptr->callback_Setup_Tab2_BrowseProject();
//    ProjectDatabase = Setup_Tab2_ProjectDatabaseCMB->currentText().toStdString();
//    ProjectName     = Setup_Tab2_ProjectNameLE->text().toStdString();
//    ProjectDir      = Setup_Tab2_ProjectDirLE->text().toStdString();
//    setNewDatabaseName(ProjectDatabase);

    emit LoadDatabase(QString::fromStdString(ProjectDatabase));

    return;
}

void
nmfMainWindow::menu_createTables()
{
    callback_createTables();
    QMessageBox::information(this,
                             tr("Tables Created"),
                             tr("\nChecked for and created any missing tables.\n"),
                             QMessageBox::Ok);
}

/*
void
nmfMainWindow::menu_connectToDatabase()
{

    QDialog *dlg = new QDialog(this);
    dlg->setWindowModality(Qt::ApplicationModal);
    dlg->setWindowTitle("Connect to Database");

    QVBoxLayout *vLayout = new QVBoxLayout();
    QHBoxLayout *hLayout = new QHBoxLayout();

    QLabel      *hostNameLBL      = new QLabel("Host name:");
    QLabel      *userNameLBL      = new QLabel("Database user name:");
    QLabel      *passwordLBL      = new QLabel("Database password:");
    QLabel      *sessionLBL       = new QLabel("Session comment for log file:");
    QLineEdit   *hostNameLE       = new QLineEdit();
    QLineEdit   *userNameLE       = new QLineEdit();
    QLineEdit   *passwordLE       = new QLineEdit();
    QLineEdit   *sessionLE        = new QLineEdit();
    QPushButton *okDatabasePB     = new QPushButton("OK");
    QPushButton *cancelDatabasePB = new QPushButton("Cancel");
    QPushButton *resetGUIPB       = new QPushButton("Reset GUI");
    hostNameLE->setObjectName("hostNameLE");
    userNameLE->setObjectName("userNameLE");
    passwordLE->setObjectName("passwordLE");
    sessionLE->setObjectName("sessionLE");
    okDatabasePB->setObjectName("okDatabasePB");
    resetGUIPB->setObjectName("resetGUIPB");
    okDatabasePB->setDefault(true);
    resetGUIPB->setToolTip("Move main GUI back to (0,0). (Useful if it gets lost.)");
    resetGUIPB->setStatusTip("Move main GUI back to (0,0). (Useful if it gets lost.)");
    sessionLE->setToolTip("This (optional) comment appears at the beginning of this session's log file.");
    sessionLE->setStatusTip("This (optional) comment appears at the beginning of this session's log file.");

    hLayout->addWidget(cancelDatabasePB);
    hLayout->addWidget(resetGUIPB);
    hLayout->addWidget(okDatabasePB);
    vLayout->addWidget(hostNameLBL);
    vLayout->addWidget(hostNameLE);
    vLayout->addWidget(userNameLBL);
    vLayout->addWidget(userNameLE );
    vLayout->addWidget(passwordLBL);
    vLayout->addWidget(passwordLE);
    vLayout->addWidget(sessionLBL); // the comment is available after the first log message has already printed
    vLayout->addWidget(sessionLE);
    vLayout->addItem(new QSpacerItem(0,10,QSizePolicy::Expanding,QSizePolicy::Expanding));
    vLayout->addLayout(hLayout);

    dlg->setLayout(vLayout);
    dlg->setMinimumWidth(250);

    // Load default values
    passwordLE->setEchoMode(QLineEdit::Password);
    hostNameLE->setText("localhost");
    userNameLE ->setText("root");

    // RSK - remove this line eventually.....just for testing purposes to speed things along.....
std::cout << "\n*** *** Remove hardcoded password for debugging! *** *** \n" << std::endl;
    passwordLE->setText("rklasky$$");

    connect(okDatabasePB,     SIGNAL(clicked(bool)),
            this,             SLOT(callback_connectToDatabase()));
    connect(cancelDatabasePB, SIGNAL(clicked(bool)),
            dlg,              SLOT(close()));
    connect(resetGUIPB,       SIGNAL(clicked(bool)),
            this,             SLOT(callback_resetGUI()));

    dlg->show();

    passwordLE->setFocus();
}


void
nmfMainWindow::callback_resetGUI()
{
    move(QPoint(0,0));
} // end callback_resetGUI


// With an item selected, the ok button sets the application to a new database.
void nmfMainWindow::callback_connectToDatabase()
{
    std::string msg;
    std::string errorMsg;
    QPushButton *ok_btn    = qobject_cast<QPushButton *>(QObject::sender());
    QDialog     *dlg       = qobject_cast<QDialog *>(ok_btn->parent());
    QLineEdit *hostNameLE  = dlg->findChild<QLineEdit *>("hostNameLE");
    QLineEdit *userNameLE  = dlg->findChild<QLineEdit *>("userNameLE");
    QLineEdit *passwordLE  = dlg->findChild<QLineEdit *>("passwordLE");
    QLineEdit *sessionLE   = dlg->findChild<QLineEdit *>("sessionLE");

    Hostname = hostNameLE->text().toStdString();
    Username = userNameLE->text().toStdString();
    Password = passwordLE->text().toStdString();
    Session  = sessionLE->text().toStdString();

    // Finish the constructor logic in this function.
    nmfMainWindowComplete();

    bool openOK = databasePtr->nmfOpenDatabase(
            Hostname, Username, Password, errorMsg);
    if (! openOK) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("MSVPA_X2 Error");
        msg  = "\nCouldn't open database.\n";
        msg += "\nError: "+errorMsg+"\n";
        msg += "\nFor Windows: Check that libmysql.dll is present in the directory\n";
        msg += "where the MSVPA_X2 executable is installed.\n";
        msgBox.setText(QString::fromStdString(msg));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    } else {
        dlg->close();
        QMessageBox::StandardButton reply;
        msg  = "\nLast Project worked on:  " + ProjectName + "\n\nContinue working with this Project?\n";
        reply = QMessageBox::question(this, tr("Open"), tr(msg.c_str()),
                                      QMessageBox::No|QMessageBox::Yes,
                                      QMessageBox::Yes);
        showLogo(false);
        if (reply == QMessageBox::Yes) {
            QString filename = QDir(QString::fromStdString(ProjectDir)).filePath(QString::fromStdString(ProjectName));
            Setup_Tab2_ptr->loadProject(filename);
            setNewDatabaseName(ProjectDatabase);
            callback_createTables();

        } else {
            updateMainWindowTitle("");
            callback_createTables();
            callback_NavigatorSelectionChanged();
        }
        showDockWidgets(true);

        // Hide Progress Chart and Log widgets. Show them once user does their first MSVPA run.
        ui->ProgressDockWidget->hide();
        ui->LogDockWidget->hide();

    }
} // end callback_connectToDatabase

*/

//void nmfMainWindow::callback_SSVPATabBarClicked(int index)
//{
//return;
//    if (index == 0) {
//        SSVPAOutputTE->hide();
//        SSVPAOutputTabWidget->repaint();
//    } else {
//        SSVPAOutputTE->show();
//        SSVPAOutputTabWidget->repaint();
//    }
//}

// Changes the ok button to enabled after user chooses a database.
void nmfMainWindow::callback_databaseSelected(QListWidgetItem *item)
{
    QDialog *dlg = qobject_cast<QDialog*>(item->listWidget()->parent());
    QPushButton *ok_btn = dlg->findChild<QPushButton*>("btn_okDB");
    ok_btn->setEnabled(true);
}


// Changes the ok button to enabled after user chooses a database.
void nmfMainWindow::callback_databaseSelectedAndClose(QListWidgetItem *item)
{
    QDialog *dlg = qobject_cast<QDialog*>(item->listWidget()->parent());
    QPushButton *ok_btn = dlg->findChild<QPushButton*>("btn_okDB");
    ok_btn->setEnabled(true);

    setNewDatabaseName(item->text().toStdString());
    dlg->close();
}


void nmfMainWindow::setNewDatabaseName(std::string newDatabaseName)
{
    QStringList categories;
    QStringList PreyList;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;

    DatabaseName = newDatabaseName;

    std::string theModelName = "";
    try {

        std::vector<std::string> selectedYears = {};
        m_databasePtr->nmfSetDatabase(newDatabaseName);
        updateMainWindowTitle("");
        callback_NavigatorSelectionChanged();
        // Update any chart GUIs that require access to database tables
        loadMSVPAChartWidgets();

        m_logger->logMsg(nmfConstants::Normal,"Database: "+newDatabaseName);

        // Start out with Diet Composition.
        if (nullptr != outputChart) {
            delete outputChart;
            outputChart = nullptr;
        }
        outputChart = new nmfOutputChartStackedBar(m_logger);
        outputChart->redrawChart("",
                    m_databasePtr, theModelName,
                    chart, AllLabels, AllComboBoxes, AllButtons, AllCheckBoxes,
                    entityName(),  forecastName(), scenarioName(),
                                 forecastFirstYear(), forecastNYears(),
                                 FirstYear,
                    selectDataTypeCMB->currentText().toStdString(),
                    selectSpeciesCMB->currentText().toStdString(),
                                 selectSpeciesCMB->currentIndex(),
                    selectVariableCMB->currentText().toStdString(),
                    selectByVariablesCMB->currentText().toStdString(),
                    selectSeasonCMB->currentText().toStdString(),
                    selectSpeciesAgeSizeClassCMB->currentText().toStdString(),
                    selectPreySpeciesCMB->currentText().toStdString(),
                    "", selectedYears, "",0.0,
                    hGridLine2d, vGridLine2d,
                    categories, PreyList, ChartData, GridData);

        enableNavigatorTopLevelItems(true);

    } catch (std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
} // end setNewDatabaseName

// With an item selected, the ok button sets the application to a new database.
void nmfMainWindow::callback_okDatabase()
{
    QPushButton *ok_btn = qobject_cast<QPushButton *>(QObject::sender());
    QDialog     *dlg    = qobject_cast<QDialog *>(ok_btn->parent());
    QListWidget *lw     = dlg->findChild<QListWidget *>("lw_names");

    setNewDatabaseName(lw->selectedItems()[0]->text().toStdString());

    dlg->close();

} // end callback_okDatabase


void
nmfMainWindow::callback_enableNavigator()
{
    setInitialNavigatorState(true);
}

//This callback updates the Navigator Tree with the correct item selected
//after the user selects a particular tab in one of the input tab widgets.

void nmfMainWindow::callback_updateNavigatorSelection2(std::string type,
                                                       int tabIndex)
{

    if (tabIndex < 0) // Could be -1 if user clicks on a disabled tab.
        return;


    int topLevelNum = 0;  // Default to Setup

    // RSK continue here with implementation of Setup tab3 Continue button
    if (type == "SSVPA") {
        topLevelNum = 1;
    } else if (type == "MSVPA") {
        topLevelNum = 2;
    }

    disconnect(NavigatorTree, 0,0,0);

    clearNavigationTreeParent(topLevelNum,tabIndex);

    connect(NavigatorTree, SIGNAL(itemSelectionChanged()),
            this,          SLOT(callback_NavigatorSelectionChanged()));
    connect(NavigatorTree, SIGNAL(itemExpanded(QTreeWidgetItem *)),
            this,          SLOT(callback_NavigatorItemExpanded(QTreeWidgetItem *)));

} // end callback_updateNavigatorSelection2


void
nmfMainWindow::clearNavigationTreeParent(int topLevelNum, int tabIndex) {
    NavigatorTree->clearSelection();
    NavigatorTree->clearFocus();
    NavigatorTree->blockSignals(true);
    NavigatorTree->topLevelItem(topLevelNum)->setExpanded(true);
    NavigatorTree->topLevelItem(topLevelNum)->child(tabIndex)->setSelected(true);
    NavigatorTree->topLevelItem(topLevelNum)->child(tabIndex)->setDisabled(false);
    NavigatorTree->blockSignals(false);
} // end clearNavigationTreeParent



// This callback updates the Navigator Tree with the correct item selected
// after the user selects a particular tab in one of the input tab widgets.

void nmfMainWindow::callback_updateNavigatorSelection(int tabIndex)
{
    if (tabIndex < 0) // Could be -1 if user clicks on a disabled tab.
        return;

    int topLevelNum = 0;  // Default to Setup
    if (QObject::sender() == Setup_Tab3_ptr) {

        topLevelNum = 1;
        activateSSVPAWidgets();

        // Load all Species into CatchAtAge tableView model array.
        std::vector<QString> AllSpecies;
        QModelIndex mindex;
        for (int i=0;i<EntityListLV->model()->rowCount();++i) {
            mindex = EntityListLV->model()->index(i,0);
            AllSpecies.push_back(EntityListLV->model()->data(mindex).toString());
        } // end for
        SSVPA_Tab1_ptr->loadAllSpeciesFromTableOrFile(
                    m_databasePtr,
                    SpeciesIndex, SpeciesName,
                    AllSpecies,  "FromTable");
        ResetSpeciesList(false);

    } else if ((QObject::sender() == m_UI->SSVPAInputTabWidget) ||
               (QObject::sender() == SSVPA_Tab1_ptr)      ||
               (QObject::sender() == SSVPA_Tab2_ptr)      ||
               (QObject::sender() == SSVPA_Tab3_ptr)      ||
               (QObject::sender() == SSVPA_Tab4_ptr))
            topLevelNum = 1;
    else if (QObject::sender() == m_UI->MSVPAInputTabWidget)
        topLevelNum = 2;
    else if (QObject::sender() == m_UI->ForecastInputTabWidget)
        topLevelNum = 3;

    disconnect(NavigatorTree, 0,0,0);

    NavigatorTree->clearSelection();
    NavigatorTree->clearFocus();
    NavigatorTree->blockSignals(true);
    //NavigatorTree->collapseAll();
    NavigatorTree->topLevelItem(topLevelNum)->setExpanded(true);
    NavigatorTree->topLevelItem(topLevelNum)->child(tabIndex)->setSelected(true);
    NavigatorTree->blockSignals(false);

    connect(NavigatorTree, SIGNAL(itemSelectionChanged()),
            this,          SLOT(callback_NavigatorSelectionChanged()));
    connect(NavigatorTree, SIGNAL(itemExpanded(QTreeWidgetItem *)),
            this,          SLOT(callback_NavigatorItemExpanded(QTreeWidgetItem *)));

} // end callback_updateNavigatorSelection



void nmfMainWindow::updateMainWindowTitle(std::string modelType)
{
    std::string modStr = (DirtyTables.size() > 0) ? "*" : "";
    std::string title = "MSVPA_X2" + modStr + ":  " + DatabaseName;
    //std::string title = "MSVPA_X2:  " + DatabaseName;
    if (! modelType.empty())
        title += " (" + modelType + ")";
    setWindowTitle(QString::fromStdString(title));

} // end updateMainWindowTitle

void
nmfMainWindow::callback_MarkAsClean()
{

    //RunSSVPAPB->setEnabled(true);
    DirtyTables.clear();

    // Reset window title (i.e., remove asterisk from title)
    QString winTitle = windowTitle();
    winTitle.replace("MSVPA_X2*:","MSVPA_X2:");
    setWindowTitle(winTitle);

} // end callback_MarkAsClean


void nmfMainWindow::menu_saveToDatabase()
{
    std::cout << "TBD menu_saveToDatabase" << std::endl;

//    bool ok = true;
//    for (const std::string &tableName : DirtyTables) {
//        std::cout << "Updating: " << tableName << std::endl;
//        if (! updateTable(tableName))
//            ok = false;
//    }
//    if (ok) {
//        MarkAsClean();
//    }


//    std::cout << "Saving modified tables to database." << std::endl;
//    databasePtr->nmfSaveDatabase();

//    // Start new database transaction after a save!!! - RSK
//    databasePtr->nmfStartTransaction();

} // end menu_saveToDatabase


//void nmfMainWindow::sendToFile(std::string name,
//        boost::numeric::ublas::matrix<double> &outMatrix, std::string species) {
//    std::ofstream ofile;

//    ofile.open(name);
//    ofile.setf(std::ios_base::fixed);
//    ofile << species.c_str() << "\n";
//    char delimeter = ',';
//    // Draw data
//    int i = 0;
//    for (boost::numeric::ublas::matrix<double>::iterator1 it1 =
//            outMatrix.begin1(); it1 != outMatrix.end1(); ++it1) {
//        ofile << FirstCatchYear + i++;
//        for (boost::numeric::ublas::matrix<double>::iterator2 it2 = it1.begin();
//                it2 != it1.end(); ++it2) {
//            //delimeter = (it2 == it1.begin()) ? ' ' : ',';
//            ofile << delimeter << (*it2);
//        }
//        ofile << "\n";
//    }
//    ofile.close();
//}

void nmfMainWindow::sendToFile(std::string filename, QTableWidget *tw)
{
    std::ofstream ofile;

   //filename.erase(remove(filename.begin(), filename.end(), ' '), filename.end());

std::cout << "Writing to: " << filename << std::endl;
    ofile.open(filename);
    ofile.setf(std::ios_base::fixed);
    std::string delim = ", ";

    int numRows = tw->rowCount();
    int numCols = tw->columnCount();

    for (int col=0; col<numCols; ++col) {
        delim = (col < numCols-1) ? ", " : "" ;
        ofile << tw->horizontalHeaderItem(col)->text().toStdString() << delim;
    }
    ofile << "\n";

    for (int row=0; row<numRows; ++row) {
        ofile << tw->verticalHeaderItem(row)->text().toStdString() << ", ";

        for (int col=0; col<numCols; ++col) {
            delim = (col < numCols-1) ? ", " : "" ;
            ofile << tw->item(row,col)->text().toStdString() << delim;
        }
        ofile << "\n";
    }

    ofile.close();

} // end sendToFile




QString nmfMainWindow::getOutputFilename(QString type, QString filenameEntered)
{
    bool ok;
    QString path;
    QString outputFile = "";
    QMessageBox::StandardButton reply;

    // Make images sub dir in the project dir if it's not already there.
    path = QDir(QString::fromStdString(m_ProjectDir)).filePath(type);
    if (! QDir(path).exists()) {
        ok = QDir().mkdir(path);
        if (! ok) {
            m_logger->logMsg(nmfConstants::Error,"getOutputFilename: Couldn't create path: "+path.toStdString());
        } else {
            m_logger->logMsg(nmfConstants::Normal,"getOutputFilename: Created directory: "+ path.toStdString());
        }
    }

    if (filenameEntered.isEmpty()) {
        // Query the user for the output file name.
        QString msg = (type == QString::fromStdString(nmfConstantsMSVPA::OutputImagesDir)) ? "Filename for chart image (jpg or png):" : "Filename for chart data csv file:";
        QString filename = QInputDialog::getText(0, "Filename",
                                                 msg, QLineEdit::Normal,"", &ok);

        if (ok && !filename.isEmpty()) {
            outputFile = QDir(path).filePath(filename);
            if (QFileInfo(outputFile).exists()) {
                reply = QMessageBox::question(this, tr("File Exists"),
                                              tr("\nFile exists. OK to overwrite?\n"),
                                              QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::No)
                    return "";
            }

            // Sleep so any previous popup has time to close and so won't be captured in the image.
            QThread::msleep((unsigned long)(100));
        }
    } else {
        outputFile = QDir(path).filePath(filenameEntered);
        if (QFileInfo(outputFile).exists()) {
            reply = QMessageBox::question(this, tr("File Exists"),
                                          tr("\nFile exists. OK to overwrite?\n"),
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No)
                return "";
        }
    }

    return outputFile;
} // end getOutputFilename


void nmfMainWindow::menu_saveOutputChart()
{
    QPixmap pm;
    QScreen *screen;
    QString path;
    QString outputFile;
    QString outputFileWithPath;
    std::string theModelName;

    nmfStructsQt::ChartSaveDlg *dlg = new nmfStructsQt::ChartSaveDlg(this);
    if (dlg->exec())
    {
        outputFile = dlg->getFilename();
    }
    if (outputFile.isEmpty())
        return;

    theModelName = modelName();
    if (theModelName == "SSVPA") {
        screen = QGuiApplication::primaryScreen();
        pm     = screen->grabWindow(outputCharts3D[0]->graph()->winId());
    } if (theModelName == "MSVPA") {
        if (selectVariableCMB->currentText() == "Predation Mortality - 3D") {
            screen = QGuiApplication::primaryScreen();
            pm     = screen->grabWindow(graph3D->winId());
        } else {
            pm = chartView->grab();
        }
    } else if (theModelName == "Forecast") {
        pm = chartView->grab();
    }

    path = QDir(QString::fromStdString(m_ProjectDir)).filePath(QString::fromStdString(nmfConstantsMSVPA::OutputImagesDir));
    // If path doesn't exist make it
    QDir pathDir(path);
    if (! pathDir.exists()) {
        pathDir.mkpath(path);
    }
    outputFileWithPath = QDir(path).filePath(outputFile);
    pm.save(outputFileWithPath);

    QString msg = "\nCapture image saved to file:\n\n" + outputFileWithPath;
    QMessageBox::information(this,
                             tr("Image Saved"),
                             tr(msg.toLatin1()),
                             QMessageBox::Ok);

    m_logger->logMsg(nmfConstants::Normal,"menu_saveOutputChart: Image saved: "+ outputFile.toStdString());

} // end menu_saveOutputChart


void nmfMainWindow::menu_saveOutputData()
{
    std::string theModelName = modelName();
    QString filename = getOutputFilename(QString::fromStdString(nmfConstantsMSVPA::OutputDataDir),"");
    if (filename.isEmpty())
        return;

    QFileInfo finfo(filename);
    if (finfo.suffix() != "csv")
        filename += ".csv";

    if (theModelName == "Setup") {

        QFile outFile;
        outFile.setFileName(filename);
        outFile.open(QIODevice::Append | QIODevice::Text);
        QTextStream out(&outFile);
        out << SetupOutputTE->toPlainText() << "\n";
        outFile.close();

    } else if (theModelName == "SSVPA") {

        QFile outFile;
        outFile.setFileName(filename);
        outFile.open(QIODevice::Append | QIODevice::Text);
        QTextStream out(&outFile);
        out << SSVPAOutputTE->toPlainText() << "\n";
        outFile.close();

    } else if ((theModelName == "MSVPA") ||
               (theModelName == "Forecast")) {
        QTableWidget* ChartDataTbW = m_UI->OutputWidget->findChild<QTableWidget*>("Data");
        QString textData = "";

        int numRows = ChartDataTbW->rowCount();
        int numCols = ChartDataTbW->columnCount();
        textData += "-"; // place holder for vertical label header (which is blank)
        for (int j = 0; j < numCols; j++) {
            textData += ", ";
            textData += ChartDataTbW->horizontalHeaderItem(j)->text();
        }
        textData += "\n";
        for (int i = 0; i < numRows; i++) {
            textData += ChartDataTbW->verticalHeaderItem(i)->text();
            for (int j = 0; j < numCols; j++) {
                textData += ", ";
                textData += ChartDataTbW->item(i,j)->text().toStdString().c_str();
            }
            textData += "\n";
        }
        QFile csvFile(filename);
        if (csvFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QTextStream out(&csvFile);
            out << textData;
            csvFile.close();
        }
    }

    QString msg = "\nCapture data saved to file:\n\n" + filename;
    QMessageBox::information(this,
                             tr("Data Saved"),
                             tr(msg.toLatin1()),
                             QMessageBox::Ok);
}

void nmfMainWindow::menu_quit() {
    close(); // emits closeEvent
}



//  RSK - Better to subclass QTableView and implement the cut,copy,paste in there.
QTableView*
nmfMainWindow::findTableInFocus()
{
    QTableView *retv=NULL;

    if (m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPACatchAtAgeTV")->hasFocus())
        return m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPACatchAtAgeTV");
    else if (m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAWeightAtAgeTV")->hasFocus())
        return m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAWeightAtAgeTV");
    else if (m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPASizeAtAgeTV")->hasFocus())
        return m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPASizeAtAgeTV");
    else if (m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAMaturityTV")->hasFocus())
        return m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAMaturityTV");
    else if (m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAMortalityTV")->hasFocus())
        return m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAMortalityTV");
    else if (m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAInitialSelectivityTV")->hasFocus())
        return m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAInitialSelectivityTV");
    else if (m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAFleetDataTV")->hasFocus())
        return m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAFleetDataTV");
    else if (m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAIndicesTV")->hasFocus())
        return m_UI->SSVPAInputTabWidget->findChild<QTableView *>("SSVPAIndicesTV");

    else if (m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab2_SeasonalDataTV")->hasFocus())
        return m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab2_SeasonalDataTV");
    else if (m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab3_BiomassTV")->hasFocus())
        return m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab3_BiomassTV");
    else if (m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab3_FeedingDataTV")->hasFocus())
        return m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab3_FeedingDataTV");
    else if (m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab4_BiomassTV")->hasFocus())
        return m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab4_BiomassTV");
    else if (m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab5_PreyPrefsTV")->hasFocus())
        return m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab5_PreyPrefsTV");
    else if (m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab6_SpOverlapTV")->hasFocus())
        return m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab6_SpOverlapTV");
    else if (m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab7_SizePrefsTV")->hasFocus())
        return m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab7_SizePrefsTV");
    else if (m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab8_PreyPrefsTV")->hasFocus())
        return m_UI->MSVPAInputTabWidget->findChild<QTableView *>("MSVPA_Tab8_PreyPrefsTV");

    else if (m_UI->ForecastInputTabWidget->findChild<QTableWidget *>("Forecast_Tab4_DataTW")->hasFocus())
        return m_UI->ForecastInputTabWidget->findChild<QTableWidget *>("Forecast_Tab4_DataTW");

    else {
        std::cout << "Error: No table found to cut, copy, or paste." << std::endl;
        return retv;
    }

} // end findTableInFocus


void
nmfMainWindow::menu_clear()
{
    QString retv = nmfUtilsQt::clear(qApp,findTableInFocus());
    if (! retv.isEmpty()) {
        QMessageBox::question(this,tr("Clear"),retv,QMessageBox::Ok);
    }
}

void
nmfMainWindow::menu_copy()
{
    QString retv = nmfUtilsQt::copy(qApp,findTableInFocus());
    if (! retv.isEmpty()) {
        QMessageBox::question(this,tr("Copy"),retv,QMessageBox::Ok);
    }
}

void
nmfMainWindow::menu_paste()
{
    QString retv = nmfUtilsQt::paste(qApp,findTableInFocus());
    if (! retv.isEmpty()) {
        QMessageBox::question(this,tr("Paste"),retv,QMessageBox::Ok);
    }
}

void
nmfMainWindow::menu_pasteAll()
{
    QString retv = nmfUtilsQt::pasteAll(qApp,findTableInFocus());
    if (! retv.isEmpty()) {
        QMessageBox::question(this,tr("Paste All"),retv,QMessageBox::Ok);
    }

}

void
nmfMainWindow::menu_clearAll()
{
    QString retv = nmfUtilsQt::clearAll(findTableInFocus());
    if (! retv.isEmpty()) {
        QMessageBox::question(this,tr("Clear All"),retv,QMessageBox::Ok);
    }
}

void
nmfMainWindow::menu_selectAll()
{
    QString retv = nmfUtilsQt::selectAll(findTableInFocus());
    if (! retv.isEmpty()) {
        QMessageBox::question(this,tr("Select All"),retv,QMessageBox::Ok);
    }
}

void
nmfMainWindow::menu_deselectAll()
{
    QString retv = nmfUtilsQt::deselectAll(findTableInFocus());
    if (! retv.isEmpty()) {
        QMessageBox::question(this,tr("Deselect All"),retv,QMessageBox::Ok);
    }
}


void
nmfMainWindow::menu_importDatabase()
{
    QString msg;
    QMessageBox::StandardButton reply;

    // Go to project data page
    NavigatorTree->setCurrentIndex(NavigatorTree->model()->index(0,0));
    m_UI->SetupInputTabWidget->setCurrentIndex(1);

    // Ask if user wants to clear the Project meta data
    msg  = "\nDo you want to overwrite current Project data with imported database information?";
    msg += "\n\nYes: Overwrites Project data\nNo: Clears Project data, user enters new Project data\n";
    reply = QMessageBox::question(this, "Import Database", msg,
                                  QMessageBox::No|QMessageBox::Yes|QMessageBox::Cancel,
                                  QMessageBox::Yes);
    if (reply == QMessageBox::Cancel) {
        return;
    }

    // Do the import
    this->setCursor(Qt::WaitCursor);
    QString dbName = m_databasePtr->importDatabase(this,
                                                 m_logger,
                                                 m_ProjectDir,
                                                 m_Username,
                                                 m_Password);
    if (!dbName.isEmpty()) {
        Setup_Tab2_ptr->loadWidgets(m_databasePtr);
        Setup_Tab3_ptr->loadWidgets(m_databasePtr);

        msg  = "\n[] After importing a database, you may want to regenerate new ";
        msg += "CSV files for the database's project.  To regenerate new CSV files:\n\n";
        msg += "File -> Regenerate CSV Files from Database...";

        if (reply == QMessageBox::No) {
            Setup_Tab2_ptr->clearProjectData();
            msg += "\n\n[] Please fill in Project data fields before continuing.";
        }

        QMessageBox::question(this, "Attention", msg.toLatin1(), QMessageBox::Ok);

        if (reply == QMessageBox::No) {
            Setup_Tab2_ptr->enableProjectData();
        }
        Setup_Tab2_ptr->setProjectDatabase(dbName);
        Setup_Tab2_ptr->callback_Setup_Tab2_SaveProject();
    }
    this->setCursor(Qt::ArrowCursor);
}


void
nmfMainWindow::menu_exportDatabase()
{
    this->setCursor(Qt::WaitCursor);
    m_databasePtr->exportDatabase(this,m_ProjectDir,
                                             m_Username,m_Password,
                                             ProjectDatabase);
    this->setCursor(Qt::ArrowCursor);
}

void
nmfMainWindow::menu_exportAllDatabases()
{
    QList<QString> authDBs = {};
    m_databasePtr->getListOfAuthenticatedDatabaseNames(authDBs);

    QList<QString>::iterator authDBsIterator;
    std::string projectDatabase;
    for (authDBsIterator = authDBs.begin(); authDBsIterator != authDBs.end(); authDBsIterator++)
    {
        projectDatabase = authDBsIterator->toStdString();

        m_databasePtr->exportDatabase(this,
                                      m_ProjectDir,
                                      m_Username,
                                      m_Password,
                                      projectDatabase);
    }
}

void
nmfMainWindow::ReadSettings(QString Name)
{
    QSettings* settings = nmfUtilsQt::createSettings(nmfConstantsMSVPA::SettingsDirWindows,"MSVPA_X2");

    settings->beginGroup("MainWindow");
    callback_SetStyleSheet(settings->value(Name,"").toString());
    settings->endGroup();

    delete settings;
}

void nmfMainWindow::ReadSettings() {
    QSettings* settings = nmfUtilsQt::createSettings(nmfConstantsMSVPA::SettingsDirWindows,"MSVPA_X2");

    settings->beginGroup("MainWindow");
    resize(settings->value("size", QSize(400, 400)).toSize());
    move(settings->value("pos", QPoint(200, 200)).toPoint());
    callback_SetStyleSheet(settings->value("style","").toString());
    //MSVPALoopsBeforeChecking = settings->value("MSVPALoopsBeforeChecking",nmfConstants::MaxLoopsBeforeChecking).toInt();
    settings->endGroup();

    settings->beginGroup("SetupTab");
    ProjectName     = settings->value("ProjectName","").toString().toStdString();
    m_ProjectDir      = settings->value("ProjectDir","").toString().toStdString();
    ProjectDatabase = settings->value("ProjectDatabase","").toString().toStdString();
    settings->endGroup();

//    settings->beginGroup("ui->CentralWidget");
//     centralWidget()->setMinimumWidth(settings->value("width",100).toInt());
//     settings->endGroup();

    delete settings;
}

QString
nmfMainWindow::getCurrentStyle()
{
    if (qApp->styleSheet().isEmpty())
        return "Light";
    else
        return "Dark";
}


void
nmfMainWindow::setDefaultDockWidgetsVisibility()
{
    QSettings* settings = nmfUtilsQt::createSettings(nmfConstantsMSVPA::SettingsDirWindows,"MSVPA_X2");

    settings->beginGroup("MainWindow");
    m_UI->OutputDockWidget->setVisible(settings->value("OutputDockWidgetIsVisible",false).toBool());
    m_UI->ProgressDockWidget->setVisible(settings->value("ProgressDockWidgetIsVisible",false).toBool());
    m_UI->LogDockWidget->setVisible(settings->value("LogDockWidgetIsVisible",false).toBool());
    settings->endGroup();

    delete settings;

    // Turn these off
//    SetupOutputTE->setVisible(false);
}

void
nmfMainWindow::saveSettings() {
    QSettings* settings = nmfUtilsQt::createSettings(nmfConstantsMSVPA::SettingsDirWindows,"MSVPA_X2");

    settings->beginGroup("MainWindow");
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->setValue("style",getCurrentStyle());
    settings->setValue("OutputDockWidgetIsVisible",  m_UI->OutputDockWidget->isVisible());
    settings->setValue("ProgressDockWidgetIsVisible",m_UI->ProgressDockWidget->isVisible());
    settings->setValue("LogDockWidgetIsVisible",     m_UI->LogDockWidget->isVisible());
    //settings->setValue("MSVPALoopsBeforeChecking",MSVPALoopsBeforeChecking);
    settings->endGroup();

//   settings->beginGroup("ui->CentralWidget");
//     settings->setValue("width",centralWidget()->width());
//     settings->endGroup();

    delete settings;

}


std::string nmfMainWindow::getDatabaseName() {
    std::vector<std::string> fields = { "database()" };
    std::string queryStr = "SELECT database()";
    std::map<std::string, std::vector<std::string> > dataMap;
    dataMap = m_databasePtr->nmfQueryDatabase(queryStr, fields);

    if (dataMap["database()"].size() > 0)
        return dataMap["database()"][0];
    else
        return "";
}


void
nmfMainWindow::SaveTables()
{
    std::string tableName;
    QListWidget *DirtyTablesLW;
    QListWidgetItem *item;
    DirtyTablesLW  = SaveDlg->findChild<QListWidget *>("DirtyTablesLW");
    int numItems = DirtyTablesLW->count();
//    std::map<std::string, void(*)(bool quiet)> SaveFunctionMap;
//    std::map<std::string, pfunc> SaveFunctionMap;
//    SaveFunctionMap["SpeCatch"]      = SSVPA_Tab1_ptr->callback_SaveAllPB;
//    SaveFunctionMap["SpeWeight"]     = SSVPA_Tab2_ptr->Save_Weight;
//    SaveFunctionMap["SpeSize"]       = SSVPA_Tab3_ptr->Save_Size;
//    SaveFunctionMap["SpeSSVPA"]      = SSVPA_Tab4_ptr->Save_Config;
//    SaveFunctionMap["SSVPAAgeM"]     = SSVPA_Tab4_ptr->Save_Config;
//    SaveFunctionMap["SpeMaturity"]   = SSVPA_Tab4_ptr->Save_SpeMaturity;
//    SaveFunctionMap["SpeTuneEffort"] = SSVPA_Tab4_ptr->Save_Fleet;
//    SaveFunctionMap["SpeTuneCatch"]  = SSVPA_Tab4_ptr->Save_Fleet;
//    SaveFunctionMap["SpeXSAData"]    = SSVPA_Tab4_ptr->Save_XSAIndexData;

    for (int i=0; i<numItems; ++i) {
        item = DirtyTablesLW->item(i);
        if (item->checkState() == Qt::Checked) {
            tableName = item->text().toStdString();
            //SaveFunctionMap[tableName](true);
            if (tableName == "SpeCatch")
                SSVPA_Tab1_ptr->callback_SaveAllPB(true);
            else if (tableName == "SpeWeight")
                SSVPA_Tab2_ptr->callback_SaveAllPB(true);
            else if (tableName == "SpeSize")
                SSVPA_Tab3_ptr->callback_SaveAllPB(true);
            else if ((tableName == "SpeSSVPA") || (tableName == "SSVPAAgeM"))
                SSVPA_Tab4_ptr->Save_Config();
            else if (tableName == "SpeMaturity")
                SSVPA_Tab4_ptr->Save_SpeMaturity();
            else if ((tableName == "SpeTuneEffort") || (tableName == "SpeTuneCatch"))
                SSVPA_Tab4_ptr->Save_Fleet();
            else if (tableName == "SpeXSAData")
                SSVPA_Tab4_ptr->Save_XSAIndexData();
            else if (tableName == "MSVPAspecies")
                MSVPA_Tab1_ptr->callback_MSVPA_Tab1_SavePB(true);
            else if ((tableName == "MSVPASeasInfo") || (tableName == "MSVPAlist"))
                MSVPA_Tab2_ptr->callback_MSVPA_Tab2_SavePB(true);
            else if ((tableName == "OtherPredBM") || (tableName == "OthPredSizeData"))
                MSVPA_Tab3_ptr->callback_MSVPA_Tab3_SavePB(true);
            else if ((tableName == "MSVPAOthPrey") || (tableName == "MSVPAOthPreyAnn"))
                MSVPA_Tab4_ptr->callback_MSVPA_Tab4_SavePB(true);
            else if (tableName == "MSVPAprefs")
                MSVPA_Tab5_ptr->callback_MSVPA_Tab5_SavePB(true);
            else if (tableName == "MSVPASpaceO")
                MSVPA_Tab6_ptr->callback_MSVPA_Tab6_SavePB(true);
            else if (tableName == "MSVPASizePref")
                MSVPA_Tab7_ptr->callback_MSVPA_Tab7_SavePB(true);
            else if (tableName == "MSVPAStomCont")
                MSVPA_Tab8_ptr->callback_MSVPA_Tab8_SavePB(true);
            else if (tableName == "MSVPAspecies")
                MSVPA_Tab11_ptr->callback_MSVPA_Tab11_SavePB(true);
            else if (tableName == "Forecasts")
                Forecast_Tab1_ptr->callback_Forecast_Tab1_SavePB(true);
            else if (tableName == "Scenarios") {
                Forecast_Tab1_ptr->callback_Forecast_Tab1_SavePB(true);
                Forecast_Tab4_ptr->callback_Forecast_Tab4_SavePB(true);
            } else if (tableName == "ForePredVonB")
                Forecast_Tab2_ptr->callback_Forecast_Tab2_SavePB(true);
            else if ((tableName == "ScenarioF")       || (tableName == "ScenarioOthPred") ||
                     (tableName == "ScenarioOthPrey") || (tableName == "ScenarioRec"))
                Forecast_Tab4_ptr->callback_Forecast_Tab4_SavePB(true);
        }
    }
    DirtyTablesLW->clear();
    callback_MarkAsClean();

} // end SaveTables

void
nmfMainWindow::callback_CheckAllItemsSaveDlg(int state)
{
    QListWidget *DirtyTablesLW;
    QListWidgetItem *item;
    DirtyTablesLW  = SaveDlg->findChild<QListWidget *>("DirtyTablesLW");
    int numItems = DirtyTablesLW->count();

    for (int i=0; i<numItems; ++i) {
        item = DirtyTablesLW->item(i);
        item->setCheckState((state==Qt::Checked) ? Qt::Checked : Qt::Unchecked);
    }

} // end CheckAllItemsSaveDlg


void
nmfMainWindow::callback_CheckAndEnableMSVPAExecutePage()
{
    bool enableMSPVPAExecutePage = true;

    for (unsigned int i=0; i<MSVPAPageEnabled.size(); ++i) {
        if ((i != 8) && (i != 9)) { // temp hack until implement Tabs 9 and 10
            if (! MSVPAPageEnabled[i]) {
                enableMSPVPAExecutePage = false;
                break;
            }
        }
    }

    MSVPA_Tab11_ptr->enableNextButton(enableMSPVPAExecutePage);

} // end callback_CheckAndEnableMSVPAExecutePage



void
nmfMainWindow::callback_SetStyleSheet(QString style)
{
    if (style == "Dark") {
        QFile fileStyle(":qdarkstyle/style.qss");
        if (! fileStyle.exists()) {
            std::cout << "Error: Unable to set stylesheet, file not found: qdarkstyle/style.qss\n" << std::endl;;
        } else {
            fileStyle.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&fileStyle);
            qApp->setStyleSheet(ts.readAll());
        }
    } else {
            qApp->setStyleSheet("");
    }

}


void
nmfMainWindow::closeEvent(QCloseEvent *event)
{
    QUiLoader loader;

    this->setCursor(Qt::WaitCursor);

    // Load ui as a widget from disk
    QFile file(":/forms/Main/Main_SaveWidget.ui");
    file.open(QFile::ReadOnly);
    SaveWidget = loader.load(&file, this);
    file.close();

    // Create a QDialog to hold the widget.  Extract information from the widget
    // once the user clicks the OK button.
    QPushButton  *CancelPB;
    QPushButton  *QuitPB;
    QPushButton  *SaveDontQuitPB;
    QPushButton  *SaveQuitPB;
    QListWidget  *DirtyTablesLW;
    QCheckBox    *CheckAllCB;
    QVBoxLayout  *lyt = new QVBoxLayout();

    // Get handles to widgets with data, hook the widget up to the dialog, and
    // show the dialog as modal.  The sub-widget data are available once the user
    // clicks OK.
    lyt->addWidget(SaveWidget);
    if (SaveDlg->layout()) {
        qDeleteAll(SaveDlg->children());
    }
    SaveDlg->setLayout(lyt);

    SaveQuitPB     = SaveWidget->findChild<QPushButton *>("SaveQuitPB");
    SaveDontQuitPB = SaveWidget->findChild<QPushButton *>("SaveDontQuitPB");
    QuitPB         = SaveWidget->findChild<QPushButton *>("QuitPB");
    CancelPB       = SaveWidget->findChild<QPushButton *>("CancelPB");
    DirtyTablesLW  = SaveWidget->findChild<QListWidget *>("DirtyTablesLW");
    CheckAllCB     = SaveWidget->findChild<QCheckBox   *>("CheckAllCB");

    connect(QuitPB,         SIGNAL(clicked(bool)), SaveDlg, SLOT(accept()));
    connect(CancelPB,       SIGNAL(clicked(bool)), SaveDlg, SLOT(accept()));
    connect(SaveQuitPB,     SIGNAL(clicked(bool)), SaveDlg, SLOT(accept()));
    connect(SaveDontQuitPB, SIGNAL(clicked(bool)), SaveDlg, SLOT(accept()));
    connect(CheckAllCB,     SIGNAL(stateChanged(int)),
            this,           SLOT(callback_CheckAllItemsSaveDlg(int)));

    // Load list view with names of tables that have changed.
    std::set<std::string>::iterator it;
    QListWidgetItem *item;
    for (it = DirtyTables.begin(); it != DirtyTables.end(); ++it) {
        item = new QListWidgetItem(QString::fromStdString(*it));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        DirtyTablesLW->addItem(item);
    }
    CheckAllCB->setChecked(true);

    this->setCursor(Qt::ArrowCursor);

    if (SaveDlg->exec()) {
        if (QuitPB->underMouse()) {
            m_logger->logMsg(nmfConstants::Bold,"MSVPA_X2 End");
        }
        if (CancelPB->underMouse()) {
            event->ignore();
        }
        if (SaveQuitPB->underMouse()) {
            SaveTables();
            m_logger->logMsg(nmfConstants::Bold,"MSVPA_X2 End");
        }
        if (SaveDontQuitPB->underMouse()) {
            SaveTables();
            event->ignore();
        }
    }

    saveSettings();


//    if (DirtyTables.size() > 0) {
//        std::string msg = "\nYou have unsaved changes.\n\nWould you like to save any modified tables?\n";
//        reply = QMessageBox::question(0, "Save Database",
//            msg.c_str(),
//            QMessageBox::Cancel|QMessageBox::Yes|QMessageBox::No);
//        if (reply == QMessageBox::Yes) {
//            menu_saveToDatabase();
//        }
//    }

//    if (reply == QMessageBox::Cancel) {
//        event->ignore();
//    } else {
//        saveSettings();
//        databasePtr->nmfCloseDatabase();
//        event->accept();
//    }

} // end closeEvent


