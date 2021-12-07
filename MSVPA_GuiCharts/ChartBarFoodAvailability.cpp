
#include "ChartBarFoodAvailability.h"

#include "nmfConstants.h"

ChartBarFoodAvailability::ChartBarFoodAvailability(
         QTableWidget* theDataTable,
         nmfLogger*    theLogger)
{
    setObjectName("Food Availability");

    DataTable = theDataTable;
    logger    = theLogger;

    // Set up function map so don't have to use series of if...else... statements.
    GetDataFunctionMap["Relative Food Availability"]     = getAndLoadRelativeFoodAvailabilityData;
    GetDataFunctionMap["Per Capita Food Availability"]   = getAndLoadPerCapitaFoodAvailabilityData;
    GetDataFunctionMap["Stomach Content Weight"]         = getAndLoadStomachContentWeightData;

    // This one's a bit different since it's a stacked bar chart.
    GetDataFunctionMap["Food Availability by Prey Type"] = getAndLoadFoodAvailabilityByPreyTypeData;

    // This one's a bit different since one of its charts is a stacked bar and the others are regular bar charts.
    GetDataFunctionMap["Food Availability by Prey Age"]  = getAndLoadFoodAvailabilityByPreyAgeData;

}


void
ChartBarFoodAvailability::callback_UpdateChart(nmfStructsQt::UpdateDataStruct data)
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

    QChart*     chart     = getChart();
    QChartView* chartView = getChartView();

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
    int Nage                                 = data.NumAgeSizeClasses;
    std::string  MaxScaleY                   = data.MaxScaleY.toStdString();
    int          Theme                       = data.Theme;

    std::vector<bool> GridLines = { data.HorizontalGridLines,
                                    data.VerticalGridLines };

    logger->logMsg(nmfConstants::Normal,
                      "MSVPA ChartBarFoodAvailability " +
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
    std::string newSelectedPredatorAgeSizeClass = SelectedSpeciesAgeSizeClass;

    newSelectedSeason.erase(0,newSelectedSeason.find(" "));
    SeasonInt = std::stoi(newSelectedSeason)-1;

    if (! newSelectedPredatorAgeSizeClass.empty()) {
        offset = (newSelectedPredatorAgeSizeClass.find("Size") != std::string::npos) ? 1 : 0;
        newSelectedPredatorAgeSizeClass.erase(0,newSelectedPredatorAgeSizeClass.find(" "));
        SpeAge = std::stoi(newSelectedPredatorAgeSizeClass);
    }

    // Load the chart data using the FunctionMap set up in the constructor.
    if (GetDataFunctionMap.find(SelectedVariable) == GetDataFunctionMap.end()) {

        logger->logMsg(nmfConstants::Error,
                       "ChartBarFoodAvailability Function: " + SelectedVariable + " not found in GetDataFunctionMap.");

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
   populateDataTable(DataTable,GridData,RowLabels,ColumnLabels,7,3);
}



void
ChartBarFoodAvailability::getAndLoadRelativeFoodAvailabilityData(
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
    std::string fieldToSum = "AnnBiomass";
    std::string predAgeStr;

    ColumnLabels << "Rel. FA";

    if (SelectedByVariables == "Annual") {
        title = "Relative Food Availability for";
    } else if (SelectedByVariables == "Seasonal") {
        title = "Relative Food Availability for";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }
    predAgeStr = " and PredAge = " + std::to_string(SpeAge-offset);

    getDataAndLoadChart("Food Availability",
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
                        "Relative Food Availability", 1.0, 1.0,MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData, MSVPA_NYears, 1);
    GridData = ChartData;

}


void
ChartBarFoodAvailability::getAndLoadPerCapitaFoodAvailabilityData(
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
    std::string fieldToSum = "AnnBiomass";
    std::string predAgeStr;

    ColumnLabels << "Rel. FA";

    if (SelectedByVariables == "Annual") {
        title = "Per Capita Relative Food Availability for";
    } else if (SelectedByVariables == "Seasonal") {
        title = "Per Capita Relative Food Availability for";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }
    predAgeStr = " and PredAge = " + std::to_string(SpeAge-offset);

    getDataAndLoadChart("Food Availability",
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
                        MSVPA_FirstYear, MSVPA_LastYear,
                        SpeAge,
                        chart,
                        title, titleSuffix, nmfConstants::RearrangeTitle,
                        "Relative Food Availability", 1.0, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData, MSVPA_NYears, 1);
    GridData = ChartData;

}


void
ChartBarFoodAvailability::getAndLoadFoodAvailabilityByPreyAgeData(
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
    int seasonVal=0;
    //double scaleFactor = 1000.0; // to display on 1000's metric tons
    std::string ageStr = "";
    std::string predAgeStr = "";
    std::string seasonStr = "";
    std::string fieldToSum = "AnnBiomass";
    std::string title;
    std::string titleSuffix = "";
    std::string queryStr;
    std::string valStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string yLabelConversion="";
    std::vector<std::string> yLabels = {"Hundreds of Fish",
                                        "Thousands of Fish",
                                        "Millions of Fish",
                                        "Billions of Fish"};

    chart->removeAllSeries();

    if (SelectedByVariables == "Annual") {
        title = "Availability of";
    } else if (SelectedByVariables == "Seasonal") {
        title = "Availability of: ";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }
    predAgeStr = " and PredAge = " + std::to_string(SpeAge-offset);

    getDataAndLoadChart("Food Availability",
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
                        "Relative Food Availability", 1.0, 1.0,MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

}



void
ChartBarFoodAvailability::getAndLoadFoodAvailabilityByPreyTypeData(
        nmfDatabase *databasePtr,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int &MSVPA_NYears,
        int &MSVPA_FirstYear,
        int &MSVPA_LastYear,
        int &Nage,
        std::string &selectedSpecies,
        std::string &selectedSpeciesAgeSizeClass,
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
    int NPrey;
    std::string title;
    std::string seasonStr = "";
    int seasonVal=0;
    int firstYear=0;
    int nYears = 0;
    int Forecast_FirstYear;
    int Forecast_NYears;
    //int Forecast_LastYear;
    std::string titleSuffix = "";
    std::string PredAgeStr;
    std::string queryStr,queryStr2;
    std::vector<std::string> fields,fields2;
    std::map<std::string, std::vector<std::string> > dataMap,dataMap2;
    double AvgFA = 0.0;
    boost::numeric::ublas::vector<double> tempData;
    QStackedBarSeries *series = NULL;

    RowLabels.clear();
    ColumnLabels.clear();
    chart->removeAllSeries();

    // Find number of Forecast years
    fields    = {"InitYear","NYears"};
    queryStr  = "SELECT InitYear,NYears FROM " +
                 nmfConstantsMSVPA::TableForecasts +
                " WHERE MSVPAName = '" +
                MSVPAName + "' AND ForeName = '" + ForecastName + "'";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["NYears"].size() > 0) {
        Forecast_FirstYear = std::stoi(dataMap["InitYear"][0]);
        Forecast_NYears    = std::stoi(dataMap["NYears"][0]) + 1;
        //Forecast_LastYear  = Forecast_FirstYear + Forecast_NYears;
    } else {
        //logger->logMsg(nmfConstants::Error,"No NYears data for Forecast: "+ForecastName);
        return;
    }

    if (SelectedByVariables == "Annual") {
        title = "Relative Food Availability for";
    } else if (SelectedByVariables == "Seasonal") {
        title = "Relative Food Availability for";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }
    PredAgeStr = " and PredAge = " + std::to_string(SpeAge-offset);

    if (ModelName == "MSVPA") {

        firstYear = MSVPA_FirstYear;
        nYears    = MSVPA_NYears;

        // First need to get a list of prey names and a count of prey for the predator.
        databasePtr->nmfQueryMsvpaPreyList(selectedSpecies,
                                           MSVPAName,
                                           false,
                                           selectedSpeciesAgeSizeClass,
                                           PredAgeStr,
                                           NPrey,
                                           ColumnLabels);
        queryStr = "SELECT Year, Sum(SuitPreyBiomass) AS TotFA FROM " +
                    nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
                   " WHERE MSVPAname = '" + MSVPAName + "'" +
                   " AND PredName = '" + selectedSpecies + "'" +
                    PredAgeStr + seasonStr +
                   " GROUP By Year";
        queryStr2 = "SELECT Year, PreyName, Sum(SuitPreyBiomass) AS SBM FROM " +
                     nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
                    " WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND PredName = '" + selectedSpecies + "'" +
                     PredAgeStr + seasonStr +
                    " GROUP By Year, PreyName";

    } else if (ModelName == "Forecast") {

        firstYear = Forecast_FirstYear;
        nYears    = Forecast_NYears;

        // First need to get a list of prey names and a count of prey for the predator.
        databasePtr->nmfQueryForecastPreyList(MSVPAName, ForecastName, ScenarioName,
                                              selectedSpecies, true, PredAgeStr, seasonStr,
                                              ColumnLabels);
        NPrey = ColumnLabels.size();

        queryStr  = "SELECT Year, Sum(SuitPreyBiomass) AS TotFA FROM " +
                     nmfConstantsMSVPA::TableForeSuitPreyBiomass +
                    " WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND ForeName = '" + ForecastName + "'" +
                    " AND Scenario = '" + ScenarioName + "'" +
                    " AND PredName = '" + selectedSpecies + "'" +
                      PredAgeStr + seasonStr +
                    " GROUP By Year";
        queryStr2 = "SELECT Year, PreyName, Sum(SuitPreyBiomass) AS SBM FROM " +
                     nmfConstantsMSVPA::TableForeSuitPreyBiomass +
                    " WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND ForeName = '" + ForecastName + "'" +
                    " AND Scenario = '" + ScenarioName + "'" +
                    " AND PredName = '" + selectedSpecies + "'" +
                      PredAgeStr + seasonStr +
                    " GROUP By Year, PreyName";

    }

    nmfUtils::initialize(tempData,  nYears);
    nmfUtils::initialize(ChartData, nYears, NPrey);
    nmfUtils::initialize(GridData,  nYears, NPrey);

    // Then want to get total suitable biomass and calculate average as previously
    fields = {"Year","TotFA"};
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

    for (int i = 0; i < nYears; ++i) {
        tempData(i) = std::stod(dataMap["TotFA"][i]);
        AvgFA      += tempData(i);
    }
    AvgFA /= (nYears + 1.0);

    // Then want to get suitable prey biomass by prey species
    fields2  = {"Year","PreyName","SBM"};
    dataMap2 = databasePtr->nmfQueryDatabase(queryStr2, fields2);
    int m = 0;
    for (int i = 0; i < nYears; ++i) {
        RowLabels << QString::number(firstYear + i);
        for (int j = 0; j < NPrey; ++j) {
            if (dataMap2["PreyName"][m] == ColumnLabels[j].toStdString()) {
                ChartData(i,j) = std::stod(dataMap2["SBM"][m++]) / AvgFA;
            } else {
                ChartData(i,j) = 0.0;
            }
        }
    }
    GridData = ChartData;

    //
    // Draw the chart now that you have all the data.
    //
    loadChartWithData(chart,series,MaxScaleY,ChartData,ColumnLabels);
    setTitles(chart, series, RowLabels, selectedSpecies,
              "Food Availability by Prey Type for ",
              nmfConstants::RearrangeTitle,
              selectedSpeciesAgeSizeClass,
              "Year", "Relative Food Availability","",Theme);

}


void
ChartBarFoodAvailability::getAndLoadStomachContentWeightData(
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
    std::string fieldToSum = "AnnBiomass";
    std::string predAgeStr;

    // Average stomach contants at age
    ColumnLabels << "Energy Density";

    if (SelectedByVariables == "Annual") {
        title = "Average Stomach Contents";
    } else if (SelectedByVariables == "Seasonal") {
        title = "Average Stomach Contents for";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }
    predAgeStr = " and Age = " + std::to_string(SpeAge-offset);

    getDataAndLoadChart("Food Availability",
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
                        "Stomach Content", 1.0, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData, MSVPA_NYears, 1);
    GridData = ChartData;
}



void
ChartBarFoodAvailability::loadChartWithData(
        QChart *chart,
        QStackedBarSeries *series,
        std::string &maxValueStr,
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


    QCategoryAxis *axisY = new QCategoryAxis();

    //
    // RSK - bug here...setRange doesn't change axis numbers for Stacked Bar chart
    //
    if (! maxValueStr.empty()) {
        double maxValue = std::stod(maxValueStr);
        if (maxValue > 0.0) {
            axisY->setRange(0, maxValue);
        }
    } else {
        axisY->applyNiceNumbers();
    }

    //chart->setAxisY(axisY, series);
    nmfUtilsQt::setAxisY(chart,axisY,series);

} // end loadChartWithData




void
ChartBarFoodAvailability::setTitles(
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

