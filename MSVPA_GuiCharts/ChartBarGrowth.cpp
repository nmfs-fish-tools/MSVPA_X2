
#include "ChartBarGrowth.h"

#include "nmfConstants.h"

ChartBarGrowth::ChartBarGrowth(
        QTableWidget* theDataTable,
        nmfLogger*    theLogger)
{
    setObjectName("Growth");

    DataTable = theDataTable;
    logger    = theLogger;

    // Set up function map so don't have to use series of if...else... statements.
    GetDataFunctionMap["Average Weight"] = getAndLoadAverageWeightData;
    GetDataFunctionMap["Average Size"]   = getAndLoadAverageSizeData;
    GetDataFunctionMap["Weight at Age"]  = getAndLoadWeightAtAgeData;
    GetDataFunctionMap["Size at Age"]    = getAndLoadSizeAtAgeData;
}


void
ChartBarGrowth::callback_UpdateChart(nmfStructsQt::UpdateDataStruct data)
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

    nmfDatabase* databasePtr                  = data.databasePtr;
    nmfLogger*   logger                       = data.logger;
    int          Nage                         = data.NumAgeSizeClasses;
    std::string  MaxScaleY                    = data.MaxScaleY.toStdString();
    std::string  MSVPAName                    = data.MSVPAName;
    std::string  ModelName                    = data.ModelName;
    std::string  ForecastName                 = data.ForecastName;
    std::string  ScenarioName                 = data.ScenarioName;
    std::string  SelectedByVariables          = data.SelectByVariables.toStdString();
    std::string  SelectedPredator             = data.SelectPredator.toStdString();
    std::string  SelectedVariable             = data.SelectVariable.toStdString();
    std::string  SelectedPredatorAgeSizeClass = data.SelectPredatorAgeSizeClass.toStdString();
    std::string  SelectedSeason               = data.SelectSeason.toStdString();
    std::string  TitlePrefix                  = data.TitlePrefix;
    std::string  TitleSuffix                  = data.TitleSuffix;
    std::string  XLabel                       = data.XLabel;
    std::string  YLabel                       = data.YLabel;
    std::string  DataTypeLabel                = data.DataTypeLabel;
    int          Theme                        = data.Theme;

    std::vector<bool> GridLines = { data.HorizontalGridLines,
                                    data.VerticalGridLines };

    logger->logMsg(nmfConstants::Normal,
                      "MSVPA ChartBarGrowth " +
                      DataTypeLabel + ", " +
                      SelectedPredator + ", " +
                      SelectedVariable + ", " +
                      SelectedByVariables + ", " +
                      SelectedSeason + ", " +
                      SelectedPredatorAgeSizeClass);

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
    std::string newSelectedPredatorAgeSizeClass = SelectedPredatorAgeSizeClass;

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
                       "ChartBarGrowth Function: " + SelectedVariable + " not found in GetDataFunctionMap.");

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
                SelectedPredator,
                SelectedVariable,
                SelectedByVariables,
                SelectedSeason,
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
ChartBarGrowth::getAndLoadAverageWeightData(
        nmfDatabase *databasePtr,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int &MSVPA_NYears,
        int &MSVPA_FirstYear,
        int &MSVPA_LastYear,
        int &Nage,
        std::string &SelectedPredator,
        std::string &SelectedVariable,
        std::string &SelectedByVariables,
        std::string &SelectedSeason,
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

    ColumnLabels << "Weight";

    if (SelectedByVariables == "Annual") {
        title = "Average Weight at Age";
    } else if (SelectedByVariables == "Seasonal") {
        title = "Average Mid-Season Weight at Age: ";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }

    getDataAndLoadChart("Growth", databasePtr,
                        ModelName, ForecastName, ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedPredator, SelectedVariable, SelectedByVariables,
                        "", "", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear,
                        SpeAge,
                        chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        "Age", "WtUnitString", 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

}



void
ChartBarGrowth::getAndLoadAverageSizeData(
        nmfDatabase *databasePtr,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int &MSVPA_NYears,
        int &MSVPA_FirstYear,
        int &MSVPA_LastYear,
        int &Nage,
        std::string &SelectedPredator,
        std::string &SelectedVariable,
        std::string &SelectedByVariables,
        std::string &SelectedSeason,
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

    ColumnLabels << "Size";

    if (SelectedByVariables == "Annual") {
        title = "Average Size at Age";
    } else if (SelectedByVariables == "Seasonal") {
        title = "Average Mid-Season Size at Age: ";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }

    getDataAndLoadChart("Growth", databasePtr,
                        ModelName, ForecastName, ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedPredator, SelectedVariable, SelectedByVariables,
                        "", "", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear,
                        SpeAge,
                        chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        "Age","WtUnitString", 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData,ChartData.size1(),ChartData.size2());
    GridData = ChartData;
}


void
ChartBarGrowth::getAndLoadWeightAtAgeData(
        nmfDatabase *databasePtr,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int &MSVPA_NYears,
        int &MSVPA_FirstYear,
        int &MSVPA_LastYear,
        int &Nage,
        std::string &SelectedPredator,
        std::string &SelectedVariable,
        std::string &SelectedByVariables,
        std::string &SelectedSeason,
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
    std::string predAgeStr;
    std::string seasonStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum = "AnnBiomass";

    ColumnLabels << "Weight";

    if (SelectedByVariables == "Annual") {
        title = "Weight at Age";
    } else if (SelectedByVariables == "Seasonal") {
        title = "Mid-Season Weight at Age: ";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }
    predAgeStr = " and Age = " + std::to_string(SpeAge-offset);

    getDataAndLoadChart("Growth", databasePtr,
                        ModelName, ForecastName, ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum, MSVPAName,
                        SelectedPredator, SelectedVariable, SelectedByVariables,
                        predAgeStr, "", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear, SpeAge, chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        "Year","WtUnitString", 1.0,
                        MaxScaleY, RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData,ChartData.size1(),ChartData.size2());
    GridData = ChartData;
}




void
ChartBarGrowth::getAndLoadSizeAtAgeData(
        nmfDatabase *databasePtr,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int &MSVPA_NYears,
        int &MSVPA_FirstYear,
        int &MSVPA_LastYear,
        int &Nage,
        std::string &SelectedPredator,
        std::string &SelectedVariable,
        std::string &SelectedByVariables,
        std::string &SelectedSeason,
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
    std::string predAgeStr;
    std::string seasonStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum = "AnnBiomass";

    ColumnLabels << "Size";

    if (SelectedByVariables == "Annual") {
        title = "Size at Age";
    } else if (SelectedByVariables == "Seasonal") {
        title = "Mid-Season Size at Age: ";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }
    predAgeStr = " and Age = " + std::to_string(SpeAge-offset);

    getDataAndLoadChart("Growth", databasePtr,
                        ModelName, ForecastName, ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum, MSVPAName,
                        SelectedPredator, SelectedVariable, SelectedByVariables,
                        predAgeStr, "", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear, SpeAge, chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        "Year","SizeUnitString", 1.0,
                        MaxScaleY, RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData,ChartData.size1(),ChartData.size2());
    GridData = ChartData;
}



void
ChartBarGrowth::getDataAndLoadChart(
        std::string caller,
        nmfDatabase *databasePtr,
        std::string &ModelName,
        std::string &ForecastName,
        std::string &ScenarioName,
        boost::numeric::ublas::matrix<double> &ChartData,
        boost::numeric::ublas::matrix<double> &GridData,
        const int &NYears,
        const int &Nage,
        const std::string &fieldToSum,
        const std::string &MSVPAName,
        const std::string &selectedSpecies,
        const std::string &selectedVariable,
        const std::string &selectedByVariables,
        const std::string &ageStr,
        const std::string &recruitChar,
        const std::string &seasonStr,
        const int &season,
        const std::string &selectedPreyName,
        const int &FirstYear,
        const int &LastYear,
        const int &SpeAge,
        QChart *chart,
        const std::string &title,
        const std::string &titleSuffix,
        const bool &rearrangeTitle,
        const std::string &xLabel,
        const std::string &yLabel,
        const double &scaleFactor,
        const std::string &MaxScaleY,
        QStringList &rowLabels,
        QStringList &LegendNames,
        int &Theme)
{
    int ageVal=0;
    QStackedBarSeries *series = NULL;
    QStringList categories;
    std::string newAgeStr = ageStr;
    std::string newYLabel="";
    std::string SizeUnitString;
    std::string WtUnitString;

    bool scaleFactorBool = isOfTypeAbundance(selectedVariable);


    if (newAgeStr != "") {
        ageVal = std::stoi(newAgeStr.erase(0,newAgeStr.find("=")+1));
    }

    getChartData(databasePtr,
                 ModelName,
                 MSVPAName,
                 ForecastName,
                 ScenarioName,
                 NYears,
                 Nage,
                 FirstYear,
                 LastYear,
                 SpeAge,
                 fieldToSum,
                 selectedSpecies,
                 selectedVariable,
                 selectedByVariables,
                 ageStr,
                 seasonStr,
                 selectedPreyName,
                 scaleFactor,
                 ChartData,
                 GridData,
                 categories,
                 LegendNames,
                 SizeUnitString,
                 WtUnitString);

    rowLabels = categories;

    if (ChartData.size1() == 0) {
std::cout << "Error: ChartBar::getDataAndLoadChart ChartData.size1 = 0" << std::endl;
        return;
    }


    loadChartWithData(chart,series,ChartData, LegendNames,
                      false, MaxScaleY, scaleFactorBool);

    if (newAgeStr != "") {
        newAgeStr = "Age " + std::to_string(ageVal) + recruitChar;
    }

    newYLabel = yLabel;
    if (yLabel == "SizeUnitString") {
        newYLabel = SizeUnitString;
    } else if (yLabel == "WtUnitString") {
        newYLabel = WtUnitString;
    }

    setTitles(chart,
              series,
              categories,
              newAgeStr,
              selectedSpecies,
              title,
              titleSuffix,
              xLabel, // "Age",
              newYLabel,
              rearrangeTitle,
              false,
              Theme);

} // end getDataAndLoadChart


void
ChartBarGrowth::getChartData(
        nmfDatabase *databasePtr,
        const std::string &ModelName,
        const std::string &MSVPAName,
        const std::string &ForecastName,
        const std::string &ScenarioName,
        const int &NYears,
        const int &Nage,
        const int &FirstYear,
        const int &LastYear,
        const int &SpeAge,
        const std::string &fieldToSum,
        const std::string &selectedSpecies,
        const std::string &selectedVariable,
        const std::string &selectedByVariables,
        const std::string &ageStr,
        const std::string &seasonStr,
        const std::string &selectedPreyName,
        const double &scaleFactor,
        boost::numeric::ublas::matrix<double> &ChartData,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList &categories,
        QStringList &LegendNames,
        std::string &SizeUnitString,
        std::string &WtUnitString)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    int firstYear=0;
    int nYears;
    int Forecast_FirstYear=0;
    int Forecast_NYears=0;
    //int Forecast_LastYear=0;

    // Find number of Forecast years
    if (! ForecastName.empty()) {
        fields    = {"InitYear","NYears"};
        queryStr  = "SELECT InitYear,NYears FROM " +
                     nmfConstantsMSVPA::TableForecasts +
                    " WHERE MSVPAName = '" + MSVPAName +
                    "' AND ForeName = '" + ForecastName + "'";
        dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["NYears"].size() > 0) {
            Forecast_FirstYear = std::stoi(dataMap["InitYear"][0]);
            Forecast_NYears    = std::stoi(dataMap["NYears"][0]) + 1;
            //Forecast_LastYear  = Forecast_FirstYear + Forecast_NYears;
        } else {
            //logger->logMsg(nmfConstants::Error,"No NYears data for Forecast: "+ForecastName);
            return;
        }
    }

    // Get conversion factor
    double WtConversion = 1.0;
    double SizeConversion = 1.0;
    fields = {"WtUnits","SizeUnits"};
    queryStr = "SELECT WtUnits,SizeUnits FROM " + nmfConstantsMSVPA::TableSpecies +
               " WHERE SpeName = '" + selectedSpecies + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["WtUnits"].size() != 0) {
        WtConversion   = std::stod(dataMap["WtUnits"][0]);
        SizeConversion = std::stod(dataMap["SizeUnits"][0]);
    } else {
        fields = {"WtUnits","SizeUnits"};
        queryStr = "SELECT WtUnits,SizeUnits FROM " + nmfConstantsMSVPA::TableOtherPredSpecies +
                   " WHERE SpeName = '" + selectedSpecies + "'";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["WtUnits"].size() != 0) {
            WtConversion   = std::stod(dataMap["WtUnits"][0]);
            SizeConversion = std::stod(dataMap["SizeUnits"][0]);
        }
    }

    // RSK - use constants here...
    SizeUnitString = "";
    if (SizeConversion == 0.1)
        SizeUnitString = "Millimeters";
    else if (SizeConversion == 1.0)
        SizeUnitString = "Centimeters";
    else if (SizeConversion == 2.54)
        SizeUnitString = "Inches";
    WtUnitString = "";
    if (WtConversion == 0.001) // weights in grams..so resuls in 000 grams = kg
        WtUnitString = "Grams";
    else if (WtConversion == 1.0) // weight in kg so metric tons
        WtUnitString = "Kilograms";
    else if (WtConversion == (1.0/2.2)) // want weight in pounds so 000 pounds
        WtUnitString = "Pounds";

    if (ModelName == "MSVPA") {
        firstYear = FirstYear;
        nYears    = NYears;
    } else if (ModelName == "Forecast") {
        firstYear = Forecast_FirstYear;
        nYears    = Forecast_NYears;
    }

    if (selectedVariable == "Average Weight") {
        nmfUtils::initialize(ChartData, Nage, 1);
        nmfUtils::initialize(GridData,  Nage, 1);

        fields = {"Age","Weight"};
        if (ModelName == "MSVPA") {
            queryStr = "SELECT Age, Avg(SeasWt) as Weight FROM " +
                        nmfConstantsMSVPA::TableMSVPASeasBiomass +
                       " WHERE MSVPAname = '" + MSVPAName + "'" +
                       " AND SpeName = '"  + selectedSpecies + "'" +
                       seasonStr + " Group By Age";
        } else if (ModelName == "Forecast") {
            queryStr = "SELECT Age, Avg(InitWt) as Weight FROM " +
                        nmfConstantsMSVPA::TableForeOutput +
                       " WHERE MSVPAname = '" + MSVPAName + "'" +
                       " AND ForeName = '" + ForecastName + "'" +
                       " AND Scenario = '" + ScenarioName + "'" +
                       " AND SpeName  = '" + selectedSpecies + "'" +
                       seasonStr + " Group By Age";
        }
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i < Nage; ++i) {
            categories << QString::fromStdString("Age " + std::to_string(i));
            ChartData(i,0) = std::stod(dataMap["Weight"][i]) / WtConversion;
        }
    } else if (selectedVariable == "Average Size") {
        nmfUtils::initialize(ChartData, Nage, 1);
        nmfUtils::initialize(GridData,  Nage, 1);
        fields = {"Age","ASize"};
        if (ModelName == "MSVPA") {
            queryStr = "SELECT Age, Avg(SeasSize) as ASize FROM " +
                        nmfConstantsMSVPA::TableMSVPASeasBiomass +
                       " WHERE MSVPAname = '" + MSVPAName + "'" +
                       " AND SpeName = '"  + selectedSpecies + "'" +
                       seasonStr + " Group By Age";
        } else if (ModelName == "Forecast") {
            queryStr = "SELECT Age, Avg(AvgSize) as ASize FROM " +
                        nmfConstantsMSVPA::TableForeOutput +
                       " WHERE MSVPAname = '" + MSVPAName + "'" +
                       " AND ForeName = '" + ForecastName + "'" +
                       " AND Scenario = '" + ScenarioName + "'" +
                       " AND SpeName  = '" + selectedSpecies + "'" +
                       seasonStr + " Group By Age";
        }
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i < Nage; ++i) {
           categories << QString::fromStdString("Age " + std::to_string(i));
           ChartData(i,0) = std::stod(dataMap["ASize"][i]) / WtConversion;
        }
    } else if (selectedVariable == "Weight at Age") {
        fields = {"Year","AvgWeight"};
        nmfUtils::initialize(ChartData, nYears, 1);
        nmfUtils::initialize(GridData,  nYears, 1);
        if (ModelName == "MSVPA") {
            queryStr = "SELECT Year, SeasWt FROM " + nmfConstantsMSVPA::TableMSVPASeasBiomass +
                       " WHERE MSVPAname = '" + MSVPAName + "'" +
                       " AND SpeName = '"  + selectedSpecies + "'" +
                       ageStr + seasonStr +
                       " Order By Year";
        } else if (ModelName == "Forecast") {
            queryStr = "SELECT Year, AvgWeight FROM " + nmfConstantsMSVPA::TableForeOutput +
                       " WHERE MSVPAname = '" + MSVPAName + "'" +
                       " AND ForeName = '" + ForecastName + "'" +
                       " AND Scenario = '" + ScenarioName + "'" +
                       " AND SpeName = '"  + selectedSpecies + "'" +
                       ageStr + seasonStr +
                       " Order By Year";
        }
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i < nYears; ++i) {
            categories << QString::number(firstYear + i);
            ChartData(i,0) = std::stod(dataMap["AvgWeight"][i]) / WtConversion;
        }
    } else if (selectedVariable == "Size at Age") {
        fields = {"Year","AvgSize"};
        nmfUtils::initialize(ChartData, nYears, 1);
        nmfUtils::initialize(GridData,  nYears, 1);

        if (ModelName == "MSVPA") {
            queryStr = "SELECT Year, SeasSize FROM " + nmfConstantsMSVPA::TableMSVPASeasBiomass +
                       " WHERE MSVPAname = '" + MSVPAName + "'" +
                       " AND SpeName = '"  + selectedSpecies + "'" +
                       ageStr + seasonStr +
                       " Order By Year";
        } else if (ModelName == "Forecast") {
            queryStr = "SELECT Year, AvgSize FROM " + nmfConstantsMSVPA::TableForeOutput +
                       " WHERE MSVPAname = '" + MSVPAName + "'" +
                       " AND ForeName = '" + ForecastName + "'" +
                       " AND Scenario = '" + ScenarioName + "'" +
                       " AND SpeName = '"  + selectedSpecies + "'" +
                       ageStr + seasonStr +
                       " Order By Year";
        }
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i < nYears; ++i) {
            categories << QString::number(firstYear + i);
            ChartData(i,0) = std::stod(dataMap["AvgSize"][i]) / SizeConversion;
        }
    }

    GridData = ChartData;
}


