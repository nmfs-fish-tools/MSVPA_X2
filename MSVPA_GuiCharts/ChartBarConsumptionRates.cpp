
#include "ChartBarConsumptionRates.h"

#include "nmfConstants.h"

ChartBarConsumptionRates::ChartBarConsumptionRates(
        QTableWidget* theDataTable,
        nmfLogger*    theLogger)
{
    setObjectName("Consumption Rates");

    DataTable = theDataTable;
    logger    = theLogger;

    // Set up function map so don't have to use series of if...else... statements.
    GetDataFunctionMap["Total Biomass Consumed"]           = getAndLoadTotalBiomassConsumedData;
    GetDataFunctionMap["Consumption by Predator Age"]      = getAndLoadConsumptionByPredatorAgeData;
    GetDataFunctionMap["Consumption by Prey Type"]         = getAndLoadConsumptionByPreyTypeData;
    GetDataFunctionMap["Cons. by Pred. Age and Prey Type"] = getAndLoadConsByPredAgeAndPreyTypeData;
    GetDataFunctionMap["Consumption by Prey Age"]          = getAndLoadConsumptionByPreyAgeData;
    GetDataFunctionMap["Wt. Specific Consumption"]         = getAndLoadWtSpecificConsumptionData;
}


void
ChartBarConsumptionRates::callback_UpdateChart(nmfStructsQt::UpdateDataStruct data)
{
    int MSVPA_FirstYear = 0;
    int MSVPA_LastYear = 0;
    int MSVPA_NYears = 0;
    int MSVPA_NSeasons = 0;
    int SeasonInt=0;
    int offset = 0;
    int SpeAge = 0;
    std::string yLabel="000 Metric Tons";
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    QStringList ColumnLabels;
    QStringList RowLabels;

    QChart*      chart     = getChart();
    QChartView*  chartView = getChartView();

    nmfDatabase* databasePtr                 = data.databasePtr;
    nmfLogger*   logger                      = data.logger;
    std::string  MSVPAName                   = data.MSVPAName;
    std::string  ForecastName                = data.ForecastName;
    std::string  ScenarioName                = data.ScenarioName;
    std::string  ModelName                   = data.ModelName;
    std::string  SelectedByVariables         = data.SelectByVariables.toStdString();
    std::string  SelectedSpecies             = data.SelectPredator.toStdString();
    std::string  SelectedVariable            = data.SelectVariable.toStdString();
    std::string  SelectedSpeciesAgeSizeClass = data.SelectPredatorAgeSizeClass.toStdString();
    std::string  SelectedSeason              = data.SelectSeason.toStdString();
    std::string  SelectedPrey                = data.SelectPreyName.toStdString();
    std::string  TitlePrefix                 = data.TitlePrefix;
    std::string  TitleSuffix                 = data.TitleSuffix;
    std::string  XLabel                      = data.XLabel;
    std::string  YLabel                      = data.YLabel;
    std::string  DataTypeLabel               = data.DataTypeLabel;
    int          Nage                        = data.NumAgeSizeClasses;
    std::string  MaxScaleY                   = data.MaxScaleY.toStdString();
    int          Theme                       = data.Theme;

    std::vector<bool> GridLines = { data.HorizontalGridLines,
                                    data.VerticalGridLines };

    logger->logMsg(nmfConstants::Normal,
                      "MSVPA ChartBarConsumptionRates " +
                      DataTypeLabel + ", " +
                      SelectedSpecies  + ", " +
                      SelectedVariable + ", " +
                      SelectedByVariables + ", " +
                      SelectedSeason + ", " +
                      SelectedSpeciesAgeSizeClass);

    // Get some initial data
    databasePtr->nmfGetMSVPAInitData(MSVPAName,
                                     MSVPA_FirstYear,
                                     MSVPA_LastYear,
                                     MSVPA_NYears,
                                     MSVPA_NSeasons);

    // Clear chart and show chartView
    chart->removeAllSeries();
    chartView->show();

    std::string newSelectedSeason = SelectedSeason;
    std::string newSelectedSpeciesAgeSizeClass = SelectedSpeciesAgeSizeClass;

    newSelectedSeason.erase(0,newSelectedSeason.find(" "));
    SeasonInt = std::stoi(newSelectedSeason)-1;

    if (! newSelectedSpeciesAgeSizeClass.empty()) {
        offset = (newSelectedSpeciesAgeSizeClass.find("Size") != std::string::npos) ? 1 : 0;
        newSelectedSpeciesAgeSizeClass.erase(0,newSelectedSpeciesAgeSizeClass.find(" "));
        SpeAge = std::stoi(newSelectedSpeciesAgeSizeClass);
    }

    // Load the chart data using the FunctionMap set up in the constructor.
    if (GetDataFunctionMap.find(SelectedVariable) == GetDataFunctionMap.end()) {

        logger->logMsg(nmfConstants::Error,
                       "ChartBarConsumptionRates Function: " + SelectedVariable + " not found in GetDataFunctionMap.");

    } else {

        GetDataFunctionMap[SelectedVariable](
                databasePtr,
                ModelName,
                MSVPAName,
                ForecastName,
                ScenarioName,
                MSVPA_NYears,
                MSVPA_FirstYear,
                MSVPA_LastYear,
                Nage,
                SelectedSpecies,
                SelectedSpeciesAgeSizeClass,
                SelectedVariable,
                SelectedByVariables,
                SelectedSeason,
                SelectedPrey,
                SeasonInt,
                SpeAge,
                MaxScaleY,
                offset,
                GridData,
                ChartData,
                RowLabels,
                ColumnLabels,
                chart,
                yLabel,
                Theme);

    }

   callback_UpdateChartGridLines(data);

   // Populate the data table that corresponds with the chart
   populateDataTable(DataTable,
                     GridData,
                     RowLabels,
                     ColumnLabels,
                     7,3);
}



void
ChartBarConsumptionRates::getAndLoadTotalBiomassConsumedData(
        nmfDatabase *databasePtr,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int &MSVPA_NYears,
        int &MSVPA_FirstYear,
        int &MSVPA_LastYear,
        int &Nage,
        std::string &SelectedSpecies,
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedVariable,
        std::string &SelectedByVariables,
        std::string &SelectedSeason,
        std::string &SelectedPreyName,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
        int &offset,
        boost::numeric::ublas::matrix<double> &GridData,
        boost::numeric::ublas::matrix<double> &ChartData,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        QChart *chart,
        std::string &yLabel,
        int &Theme)
{
    std::string title;
    std::string seasonStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    std::string predAgeStr;

    ColumnLabels << "BM Consumed";

    if (SelectedByVariables == "Annual") {
        fieldToSum = "BMConsumed";
        title = "Total Prey Biomass Consumed by";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum = "BMConsumed";
        title = "Total Prey Biomass Consumed by";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }

    getDataAndLoadChart("Consumption Rates",
                        databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedSpecies, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        predAgeStr, "", seasonStr, seasonVal, SelectedPreyName,
                        MSVPA_FirstYear, MSVPA_LastYear, SpeAge, chart,
                        title, titleSuffix, nmfConstants::RearrangeTitle,
                        "Biomass (000 Metric Tons)", 1.0, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData,ChartData.size1(),ChartData.size2());
    GridData = ChartData;

}



void
ChartBarConsumptionRates::getAndLoadConsumptionByPredatorAgeData(
        nmfDatabase *databasePtr,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int &MSVPA_NYears,
        int &MSVPA_FirstYear,
        int &MSVPA_LastYear,
        int &Nage,
        std::string &SelectedSpecies,
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedVariable,
        std::string &SelectedByVariables,
        std::string &SelectedSeason,
        std::string &SelectedPreyName,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
        int &offset,
        boost::numeric::ublas::matrix<double> &GridData,
        boost::numeric::ublas::matrix<double> &ChartData,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        QChart *chart,
        std::string &yLabel,
        int &Theme)
{
    std::string title;
    std::string seasonStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    std::string predAgeStr;

    ColumnLabels << "BM Consumed";

    if (SelectedByVariables == "Annual") {
        fieldToSum = "BMConsumed";
        title = "Total Prey Biomass Consumed by";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum = "BMConsumed";
        title = "Total Prey Biomass Consumed by";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }
    predAgeStr = " and PredAge = " + std::to_string(SpeAge-offset);

    getDataAndLoadChart("Consumption Rates",
                        databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedSpecies, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        predAgeStr, "", seasonStr, seasonVal, SelectedPreyName,
                        MSVPA_FirstYear, MSVPA_LastYear, SpeAge, chart,
                        title, titleSuffix, nmfConstants::RearrangeTitle,
                        "Biomass (000 Metric Tons)", 1.0, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);


}


void
ChartBarConsumptionRates::getAndLoadConsumptionByPreyTypeData(
        nmfDatabase *databasePtr,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int &MSVPA_NYears,
        int &MSVPA_FirstYear,
        int &MSVPA_LastYear,
        int &Nage,
        std::string &SelectedSpecies,
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedVariable,
        std::string &SelectedByVariables,
        std::string &SelectedSeason,
        std::string &SelectedPreyName,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
        int &offset,
        boost::numeric::ublas::matrix<double> &GridData,
        boost::numeric::ublas::matrix<double> &ChartData,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        QChart *chart,
        std::string &yLabel,
        int &Theme)
{
    std::string title;
    std::string seasonStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    std::string predAgeStr="";

    ColumnLabels.clear();

    if (SelectedByVariables == "Annual") {
        fieldToSum  = "BMConsumed";
        title       = "Total Prey Biomass Consumed by";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum  = "BMConsumed";
        title       = "Total Prey Biomass Consumed by";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    getDataAndLoadChart("Consumption Rates",
                        databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedSpecies, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        predAgeStr, "", seasonStr, seasonVal, SelectedPreyName,
                        MSVPA_FirstYear, MSVPA_LastYear, SpeAge, chart,
                        title, titleSuffix, nmfConstants::RearrangeTitle,
                        "Biomass (000 Metric Tons)", 1.0, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData,ChartData.size1(),ChartData.size2());
    GridData = ChartData;

}


void
ChartBarConsumptionRates::getAndLoadConsByPredAgeAndPreyTypeData(
        nmfDatabase *databasePtr,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int &MSVPA_NYears,
        int &MSVPA_FirstYear,
        int &MSVPA_LastYear,
        int &Nage,
        std::string &SelectedSpecies,
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedVariable,
        std::string &SelectedByVariables,
        std::string &SelectedSeason,
        std::string &SelectedPreyName,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
        int &offset,
        boost::numeric::ublas::matrix<double> &GridData,
        boost::numeric::ublas::matrix<double> &ChartData,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        QChart *chart,
        std::string &yLabel,
        int &Theme)
{
    std::string title;
    std::string seasonStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    std::string predAgeStr;

    ColumnLabels.clear();

    if (SelectedByVariables == "Annual") {
        fieldToSum = "BMConsumed";
        title = "Total Prey Biomass Consumed by";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum = "BMConsumed";
        title = "Total Prey Biomass Consumed by";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }
    predAgeStr = " and PredAge = " + std::to_string(SpeAge-offset);

    getDataAndLoadChart("Consumption Rates",
                        databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedSpecies, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        predAgeStr, "", seasonStr, seasonVal, SelectedPreyName,
                        MSVPA_FirstYear, MSVPA_LastYear, SpeAge, chart,
                        title, titleSuffix, nmfConstants::RearrangeTitle,
                        "Biomass (000 Metric Tons)", 1.0, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData,ChartData.size1(),ChartData.size2());
    GridData = ChartData;

}


void
ChartBarConsumptionRates::getAndLoadConsumptionByPreyAgeData(
        nmfDatabase *databasePtr,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int &MSVPA_NYears,
        int &MSVPA_FirstYear,
        int &MSVPA_LastYear,
        int &Nage,
        std::string &SelectedSpecies,
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedVariable,
        std::string &SelectedByVariables,
        std::string &SelectedSeason,
        std::string &SelectedPreyName,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
        int &offset,
        boost::numeric::ublas::matrix<double> &GridData,
        boost::numeric::ublas::matrix<double> &ChartData,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        QChart *chart,
        std::string &yLabel,
        int &Theme)
{
    std::string title;
    std::string seasonStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    std::string predAgeStr;

    ColumnLabels.clear();

    if (SelectedByVariables == "Annual") {
        fieldToSum = "BMConsumed";
        title = SelectedPreyName + " Biomass Consumed by";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum = "BMConsumed";
        title = SelectedPreyName + " Biomass Consumed by";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }
    predAgeStr = " and PredAge = " + std::to_string(SpeAge-offset);

    getDataAndLoadChart("Consumption Rates",
                        databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedSpecies, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        predAgeStr, "", seasonStr, seasonVal, SelectedPreyName,
                        MSVPA_FirstYear, MSVPA_LastYear, SpeAge, chart,
                        title, titleSuffix, nmfConstants::RearrangeTitle,
                        "Biomass (000 Metric Tons)", 1.0, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData,ChartData.size1(),ChartData.size2());
    GridData = ChartData;

}


void
ChartBarConsumptionRates::getAndLoadWtSpecificConsumptionData(
        nmfDatabase *databasePtr,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int &MSVPA_NYears,
        int &MSVPA_FirstYear,
        int &MSVPA_LastYear,
        int &Nage,
        std::string &SelectedSpecies,
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedVariable,
        std::string &SelectedByVariables,
        std::string &SelectedSeason,
        std::string &SelectedPreyName,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
        int &offset,
        boost::numeric::ublas::matrix<double> &GridData,
        boost::numeric::ublas::matrix<double> &ChartData,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        QChart *chart,
        std::string &yLabel,
        int &Theme)
{
    std::string title;
    std::string seasonStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum = "";
    std::string predAgeStr;

    ColumnLabels.clear();

    title       = "Weight specific daily consumption by";
    titleSuffix = SelectedSeason;
    seasonVal   = SeasonInt;
    seasonStr   = " and Season = " + std::to_string(seasonVal);
    predAgeStr  = " and Age = " + std::to_string(SpeAge-offset);

    getDataAndLoadChart("Consumption Rates",
                        databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedSpecies, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        predAgeStr, "", seasonStr, seasonVal, SelectedPreyName,
                        MSVPA_FirstYear, MSVPA_LastYear, SpeAge, chart,
                        title, titleSuffix, nmfConstants::RearrangeTitle,
                        "gPrey per gBW per Day", 1.0, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

}



void
ChartBarConsumptionRates::loadChartWithData(
        QChart *chart,
        QStackedBarSeries *series,
        boost::numeric::ublas::matrix<double> &ChartData,
        QStringList &LegendNames)
{
    std::string msg;
    QBarSet *newSet;

    series = new QStackedBarSeries();

    if ((int)LegendNames.size() < (int)ChartData.size2()) {
        msg = "Error: nmfOutputChartBar::loadChartWithData array size problem. ";
        msg += "\n       LegendNames size of " + std::to_string(LegendNames.size()) + " < ";
        msg += "ChartData size of " + std::to_string(ChartData.size2());
        std::cout << msg << std::endl;
        return;
    }

    bool showLegend = (ChartData.size2() > 1);

    // There may be bar charts with multiple barsets so ChartData is 2 dimensional.
    for (unsigned int i=0; i<ChartData.size2(); ++i) {
        newSet = new QBarSet(LegendNames[i]);
        for (unsigned int val=0; val<ChartData.size1(); ++val) {
            *newSet << ChartData(val,i); //*scaleFactor;
        }
        series->append(newSet);
    }
    chart->addSeries(series);

    chart->legend()->setVisible(showLegend);

//    QValueAxis *axisY = new QValueAxis();
//    chart->setAxisY(axisY, series);
//    if (maxValue > 0.0) {
//        axisY->setRange(0, maxValue);
//    } else {
//        axisY->applyNiceNumbers();
//    }

} // end loadChartWithData




void
ChartBarConsumptionRates::setTitles(
        QChart *chart,
        QStackedBarSeries *series, QStringList categories,
        std::string species, std::string byVariables,
        bool rearrangeTitle, std::string dataType,
        std::string xLabel, std::string yLabel,
        std::string suffix,
        int Theme)
{
    std::stringstream ss;

    chart->setTheme(static_cast<QChart::ChartTheme>(Theme));

    QFont mainTitleFont = chart->titleFont();
    mainTitleFont.setPointSize(14);
    mainTitleFont.setWeight(QFont::Bold);
    chart->setTitleFont(mainTitleFont);
    if (rearrangeTitle) {
        ss << byVariables << " " << dataType << " " << species;
    } else {
        ss << byVariables << " " << species << " " << dataType << " By " << xLabel << suffix;
    }
    chart->setTitle(ss.str().c_str());

    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(categories);
    chart->createDefaultAxes();
    chart->setAxisX(axis, series);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    QAbstractAxis *axisX = chart->axisX();
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
    chart->setAxisY(newAxisY,series);

}



