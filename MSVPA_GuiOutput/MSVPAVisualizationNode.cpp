

#include "MSVPAVisualizationNode.h"

#include "nmfConstants.h"


MSVPAVisualizationNode::MSVPAVisualizationNode(nmfLogger*  theLogger)
{
    QStringList ChartTypes = {"Diet Composition",  "Population Size",
                              "Mortality Rates",   "Consumption Rates",
                              "Yield Per Recruit", "Food Availability",
                              "Multispecies Populations","Growth"};
    QFrame *frame;
    Q3DBars *graph3D = new Q3DBars();
    QWidget *chartView3DContainer = QWidget::createWindowContainer(graph3D);
    Q3DTheme *myTheme = graph3D->activeTheme();
    nmfOutputChart3DBarModifier *m_modifier;

    myTheme->setLabelBorderEnabled(false);
    if (! graph3D->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
        return;
    }
    chartView3DContainer->setMinimumHeight(200);
    chartView3DContainer->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    m_modifier = new nmfOutputChart3DBarModifier(graph3D);

    QVBoxLayout *vlayt = new QVBoxLayout();
    vlayt->setObjectName("VLAYT");
    vlayt->insertWidget(0,chartView3DContainer);

    databasePtr = NULL;
    logger      = theLogger;
    MSVPAName   = "";

    // Create widgets and layouts
    //OutputChart       = new ChartStackedBar();
    OutputData        = new MSVPAX2OutputData();
    OutputGuiControls = new MSVPAGuiControlsNonYieldPerRecruit();
    mainLayt          = new QHBoxLayout();
    mainSplitter      = new QSplitter(Qt::Horizontal);
    outputTW          = new QTabWidget();
    guiControlsGB     = new QGroupBox("Controls");

    // Setup main horizontal layout
    mainSplitter->addWidget(outputTW);
    mainSplitter->addWidget(guiControlsGB);
    mainLayt->addWidget(mainSplitter);

    // Create pointer map and use SelectDataTypeCMB index to select the appropriate one
    GuiControls["Diet Composition"]          = std::shared_ptr<MSVPAGuiControls>(new GuiControlsDietComposition());
    GuiControls["Population Size"]           = std::shared_ptr<MSVPAGuiControls>(new GuiControlsPopulationSize());
    GuiControls["Mortality Rates"]           = std::shared_ptr<MSVPAGuiControls>(new GuiControlsMortalityRates());
    GuiControls["Consumption Rates"]         = std::shared_ptr<MSVPAGuiControls>(new GuiControlsConsumptionRates());
    GuiControls["Yield Per Recruit"]         = std::shared_ptr<MSVPAGuiControls>(new GuiControlsYieldPerRecruit());
    GuiControls["Food Availability"]         = std::shared_ptr<MSVPAGuiControls>(new GuiControlsFoodAvailability());
    GuiControls["Multispecies Populations"]  = std::shared_ptr<MSVPAGuiControls>(new GuiControlsMultispeciesPopulations());
    GuiControls["Growth"]                    = std::shared_ptr<MSVPAGuiControls>(new GuiControlsGrowth());

    GuiControls["Forecast Diet Composition"]          = std::shared_ptr<MSVPAGuiControls>(new GuiControlsDietComposition());
    GuiControls["Forecast Population Size"]           = std::shared_ptr<MSVPAGuiControls>(new GuiControlsPopulationSize());
    GuiControls["Forecast Mortality Rates"]           = std::shared_ptr<MSVPAGuiControls>(new GuiControlsMortalityRatesForecast());
    GuiControls["Forecast Consumption Rates"]         = std::shared_ptr<MSVPAGuiControls>(new GuiControlsConsumptionRates());
    GuiControls["Forecast Yield Per Recruit"]         = std::shared_ptr<MSVPAGuiControls>(new GuiControlsYieldPerRecruitForecast());
    GuiControls["Forecast Food Availability"]         = std::shared_ptr<MSVPAGuiControls>(new GuiControlsFoodAvailability());
    GuiControls["Forecast Multispecies Populations"]  = std::shared_ptr<MSVPAGuiControls>(new GuiControlsMultispeciesPopulations());
    GuiControls["Forecast Growth"]                    = std::shared_ptr<MSVPAGuiControls>(new GuiControlsGrowth());

    OutputCharts["Diet Composition"]         = std::shared_ptr<MSVPAX2OutputChart>(new ChartStackedBarDietComposition(OutputData->widget(),logger));
    OutputCharts["Population Size"]          = std::shared_ptr<MSVPAX2OutputChart>(new ChartBarPopulationSize(OutputData->widget(),logger));
    OutputCharts["Mortality Rates"]          = std::shared_ptr<MSVPAX2OutputChart>(new ChartBarMortalityRates(vlayt,m_modifier,chartView3DContainer,OutputData->widget(),logger));
    OutputCharts["Consumption Rates"]        = std::shared_ptr<MSVPAX2OutputChart>(new ChartBarConsumptionRates(OutputData->widget(),logger));
    OutputCharts["Yield Per Recruit"]        = std::shared_ptr<MSVPAX2OutputChart>(new ChartLineYieldPerRecruit(OutputData->widget(),logger));
    OutputCharts["Food Availability"]        = std::shared_ptr<MSVPAX2OutputChart>(new ChartBarFoodAvailability(OutputData->widget(),logger));
    OutputCharts["Multispecies Populations"] = std::shared_ptr<MSVPAX2OutputChart>(new ChartLineMultispeciesPopulations(OutputData->widget(),logger));
    OutputCharts["Growth"]                   = std::shared_ptr<MSVPAX2OutputChart>(new ChartBarGrowth(OutputData->widget(),logger));

    // Add each output chart to the layout.  Will hide them all
    // and only show the one we're currently interested in.
    vlayt->addWidget(OutputCharts["Diet Composition"].get()->getChartView());
    vlayt->addWidget(OutputCharts["Population Size"].get()->getChartView());
    vlayt->addWidget(OutputCharts["Mortality Rates"].get()->getChartView());
    vlayt->addWidget(OutputCharts["Consumption Rates"].get()->getChartView());
    vlayt->addWidget(OutputCharts["Yield Per Recruit"].get()->getChartView());
    vlayt->addWidget(OutputCharts["Food Availability"].get()->getChartView());
    vlayt->addWidget(OutputCharts["Multispecies Populations"].get()->getChartView());
    vlayt->addWidget(OutputCharts["Growth"].get()->getChartView());

    frame = new QFrame();
    frame->setLayout(vlayt);

    // Setup tab widget
    outputTW->insertTab(0, frame,                "Chart");
    outputTW->insertTab(1, OutputData->widget(), "Data");

    // Set names for the chart widget page and the widget that will hold the data
    OutputData->widget()->setObjectName("Data");
    outputTW->widget(0)->setObjectName("Chart");
    outputTW->setObjectName("MSVPAOutputTabWidget");

    // Setup gui controls
    mainGuiLayt = new QVBoxLayout();
    mainGuiLayt->addWidget( SelectDataTypeLBL );
    mainGuiLayt->addWidget( SelectDataTypeCMB );
    guiControlsGB->setLayout(mainGuiLayt);
    guiControlsGB->setFixedWidth(240);

    // Setup connections between GUI controls and the Chart type object
    QString key;
    for (QString ChartType : ChartTypes) {
        for (QString item : {"", "Forecast "}) {
            key = item + ChartType;
            connect(GuiControls [key].get(),       SIGNAL(       UpdateChart(nmfStructsQt::UpdateDataStruct)),
                    OutputCharts[ChartType].get(), SLOT(callback_UpdateChart(nmfStructsQt::UpdateDataStruct)));
            connect(GuiControls [key].get(),       SIGNAL(       UpdateChartGridLines(nmfStructsQt::UpdateDataStruct)),
                    OutputCharts[ChartType].get(), SLOT(callback_UpdateChartGridLines(nmfStructsQt::UpdateDataStruct)));
            connect(GuiControls [key].get(),       SIGNAL(       UpdateTheme(int)),
                    OutputCharts[ChartType].get(), SLOT(callback_UpdateTheme(int)));
        }
    }

    // Setup connection for data type combobox
    connect(SelectDataTypeCMB, SIGNAL(currentIndexChanged(QString)),
            this,              SLOT(callback_SelectDataTypeChanged(QString)));

    DataTypes = {
        "Diet Composition",
        "Population Size",
        "Mortality Rates",
        "Consumption Rates",
        "Yield Per Recruit",
        "Food Availability",
        "Multispecies Populations",
        "Growth",
        "Forecast Diet Composition",
        "Forecast Population Size",
        "Forecast Mortality Rates",
        "Forecast Consumption Rates",
        "Forecast Yield Per Recruit",
        "Forecast Food Availability",
        "Forecast Multispecies Populations",
        "Forecast Growth"
    };

    // Load up a map of data type names and frames containing layouts
    // of all the different gui control widgets per data type.
    for (unsigned int i=0; i<DataTypes.size(); ++i) {
        OutputGuiControls = GuiControls[DataTypes[i]].get();
        frame = new QFrame();
        frame->setObjectName(DataTypes[i]);
        frame->setLayout(OutputGuiControls->mainLayout());
        mainGuiLayt->addWidget(frame);
        DataTypeMap[DataTypes[i]] = frame;
    }

    // Set initial set of widgets to be Diet Composition
    SelectDataTypeCMB->setCurrentIndex(0);
    callback_SelectDataTypeChanged("Diet Composition");
}

MSVPAVisualizationNode::~MSVPAVisualizationNode()
{
}

QTabWidget*
MSVPAVisualizationNode::tabWidget()
{
    return outputTW;
}

QGroupBox*
MSVPAVisualizationNode::controlsWidget()
{
    return guiControlsGB;
}

QHBoxLayout*
MSVPAVisualizationNode::mainLayout()
{
    return mainLayt;
}

void
MSVPAVisualizationNode::setDatabaseVars(nmfDatabase* theDatabasePtr,
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

    SelectDataTypeCMB->setCurrentIndex(0);
    callback_SelectDataTypeChanged("Diet Composition");

}


void
MSVPAVisualizationNode::callback_SelectDataTypeChanged(QString value)
{
    if (ModelName == "Forecast") {
        value = "Forecast " + value;
    }

    // Hide all charts
    for (int i=0; i<outputTW->widget(0)->layout()->count(); ++i) {
        QWidget *widget = outputTW->widget(0)->layout()->itemAt(i)->widget();
        if (widget != NULL)
            widget->hide();
    }

    // Hide all the data type group boxes
    for (unsigned int i=0; i<DataTypes.size(); ++i) {
        DataTypeMap[DataTypes[i]]->hide();
    }

    // Show the right one
    QChartView *chartView = outputTW->widget(0)->layout()->findChild<QChartView*>(value);
    if (chartView)
        chartView->show();

    // Show the appropriate data type group box
    DataTypeMap[value]->show();

    // Load data into widgets
    OutputGuiControls = GuiControls[value].get();
    if (! MSVPAName.empty()) {

        OutputGuiControls->loadWidgets(databasePtr,
                                       logger,
                                       MSVPAName,
                                       ModelName,
                                       ForecastName,
                                       ScenarioName);

    } else {
        if (logger)
            logger->logMsg(nmfConstants::Error,
                       "MSVPAVisualizationNode::callback_SelectDataTypeChanged No MSVPAName");
    }
}
