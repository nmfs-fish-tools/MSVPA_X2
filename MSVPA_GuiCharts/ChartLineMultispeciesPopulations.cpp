
#include "ChartLineMultispeciesPopulations.h"

#include "nmfConstants.h"

static double static_yMax = 0;

ChartLineMultispeciesPopulations::ChartLineMultispeciesPopulations(
        QTableWidget* theDataTable,
        nmfLogger*    theLogger)
{
    setObjectName("Multispecies Populations");

    m_dataTable = theDataTable;
    m_logger    = theLogger;
    static_yMax      = 0;

    // Set up function map so don't have to use series of if...else... statements.
    GetDataFunctionMap["MSVPA Total Biomass"]          = getAndLoadTotalBiomassDataMSVPA;
    GetDataFunctionMap["MSVPA Total Abundance"]        = getAndLoadTotalAbundanceDataMSVPA;
    GetDataFunctionMap["MSVPA Age 1+ Biomass"]         = getAndLoadAge1PlusBiomassDataMSVPA;
    GetDataFunctionMap["MSVPA Age 1+ Abundance"]       = getAndLoadAge1PlusAbundanceDataMSVPA;
    GetDataFunctionMap["MSVPA Spawning Stock Biomass"] = getAndLoadSpawningStockBiomassDataMSVPA;

    GetDataFunctionMap["Forecast Total Biomass"]          = getAndLoadTotalBiomassDataForecast;
    GetDataFunctionMap["Forecast Total Abundance"]        = getAndLoadTotalAbundanceDataForecast;
    GetDataFunctionMap["Forecast Age 1+ Biomass"]         = getAndLoadAge1PlusBiomassDataForecast;
    GetDataFunctionMap["Forecast Age 1+ Abundance"]       = getAndLoadAge1PlusAbundanceDataForecast;
    GetDataFunctionMap["Forecast Spawning Stock Biomass"] = getAndLoadSpawningStockBiomassDataForecast;

}


void
ChartLineMultispeciesPopulations::callback_UpdateChart(nmfStructsQt::UpdateDataStruct data)
{
    int MSVPA_FirstYear = 0;
    int MSVPA_LastYear = 0;
    int MSVPA_NYears = 0;
    int MSVPA_NSeasons = 0;
    static_yMax = 0;

    QChart*     chart     = getChart();
    QChartView* chartView = getChartView();

    nmfDatabase* databasePtr                  = data.databasePtr;
    nmfLogger*   logger                       = data.logger;
    //int          Nage                         = data.NumAgeSizeClasses;
    double       MaxScaleY                    = data.MaxScaleY.toDouble();
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


    boost::numeric::ublas::vector<std::string> LegendNames;
    boost::numeric::ublas::vector<int> Years;
    boost::numeric::ublas::matrix<double> GridData;
    QStringList RowLabels;
    QStringList ColumnLabels;
    std::string seasonStr;
    std::string str1,str2;

    std::vector<bool> GridLines = { data.HorizontalGridLines,
                                    data.VerticalGridLines };

    logger->logMsg(nmfConstants::Normal,
                      "MSVPA ChartLineMultispeciesPopulations " +
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

    nmfUtils::split(SelectedSeason," ",str1,str2);
    seasonStr = " AND Season = " + std::to_string(std::stoi(str2)-1);

    nmfUtils::initialize(Years, MSVPA_NYears);
    nmfUtils::initialize(LegendNames, MSVPA_NYears);
    for (int i=0; i<MSVPA_NYears; ++i) {
        Years(i) = MSVPA_FirstYear + i;
        LegendNames(i) = std::to_string(Years(i));
    }

    std::string FcnIndex = ModelName + " " + SelectedVariable;

    if (GetDataFunctionMap.find(FcnIndex) == GetDataFunctionMap.end()) {
        logger->logMsg(nmfConstants::Error,
                       "ChartLineMultispeciesPopulations Function: " +
                       FcnIndex + " not found in GetDataFunctionMap.");
    } else {
        GetDataFunctionMap[FcnIndex](
                databasePtr,
                chart,
                ModelName,
                MSVPAName,
                ForecastName,
                ScenarioName,
                MSVPA_FirstYear,
                MSVPA_NYears,
                seasonStr,
                RowLabels,
                ColumnLabels,
                MaxScaleY,
                GridData);
    }

    // Populate the data table that corresponds with the chart
    populateDataTable(m_dataTable,GridData,RowLabels,ColumnLabels,7,3);

}


void
ChartLineMultispeciesPopulations::getAndLoadTotalBiomassDataMSVPA(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string ModelName,
        std::string MSVPAName,
        std::string ForecastName,
        std::string ScenarioName,
        int FirstYear,
        int NumYears, // These are years selected from the gui
        std::string seasonStr,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        double &MaxScaleY,
        boost::numeric::ublas::matrix<double> &GridData)
{
    int NMSVPASpe=0;
    int season;
    std::string str1,str2;
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string mainTitle = "Total Biomass (Jan 1.) for MSVPA Species";
    std::vector<std::string> XLabelNames;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::vector<std::string> MSVPASpeList;
    std::vector<int> MSVPASpeAge;

    RowLabels.clear();
    ColumnLabels.clear();

    // Select sum database field
    nmfUtils::split(seasonStr,"=",str1,str2);
    season = stoi(str2);
    std::string abundVariable = (season == 0) ? "AnnBiomass" : "Biomass";

    getMSVPASpecies(databasePtr,MSVPAName,NMSVPASpe,MSVPASpeList);
    getMSVPASpeciesNamesAges(databasePtr,NMSVPASpe,MSVPASpeList,MSVPASpeAge);

    nmfUtils::initialize(ChartData, NumYears, NMSVPASpe);
    nmfUtils::initialize(GridData,  NumYears, NMSVPASpe);
std::cout << "Data: " << std::endl;
    for (int i = 0; i < NMSVPASpe; ++i) {
        ColumnLabels << QString::fromStdString(MSVPASpeList(i));
        fields = {"Year","Biomass"};
        queryStr = "SELECT Year, Sum(" + abundVariable + ") AS Biomass FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND SpeName = '" + MSVPASpeList(i) + "'" +
                  seasonStr + " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (int j = 0; j < NumYears; ++j) {
            if (i == 0) {
                RowLabels << QString::number(FirstYear+j); // RSK improve on this, both RowLabels and X... aren't necessary
                XLabelNames.push_back(std::to_string(FirstYear + j));
            }
            ChartData(j,i) = std::stod(dataMap["Biomass"][j]) / 1000.0; // RSK replace with constant
std::cout << ChartData(j,i) << std::endl;
        }
std::cout << std::endl;
    }
    GridData = ChartData;

    loadChartWithData(chart,
                      ChartData,
                      MSVPASpeList,
                      mainTitle,
                      "Year",              // x title
                      "000 Metric Tons",   // y title
                      XLabelNames,
                      MaxScaleY,
                      nmfConstants::RotateLabels);
}

void
ChartLineMultispeciesPopulations::getAndLoadTotalAbundanceDataMSVPA(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string ModelName,
        std::string MSVPAName,
        std::string ForecastName,
        std::string ScenarioName,
        int FirstYear,
        int NumYears, // These are years selected from the gui
        std::string seasonStr,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        double &MaxScaleY,
        boost::numeric::ublas::matrix<double> &GridData)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    int NMSVPASpe=0;
    std::vector<std::string> XLabelNames;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::vector<std::string> MSVPASpeList;
    std::vector<int> MSVPASpeAge;
    std::string mainTitle = "Total Abundance (Jan 1.) of MSVPA Species";

    RowLabels.clear();
    ColumnLabels.clear();

    // Select sum database field
    std::string str1,str2;
    nmfUtils::split(seasonStr,"=",str1,str2);
    int season = stoi(str2);
    std::string abundVariable = (season == 0) ? "AnnAbund" : "SeasAbund";

    getMSVPASpecies(databasePtr,MSVPAName,NMSVPASpe,MSVPASpeList);
    getMSVPASpeciesNamesAges(databasePtr,NMSVPASpe,MSVPASpeList,MSVPASpeAge);

    nmfUtils::initialize(ChartData, NumYears, NMSVPASpe);
    nmfUtils::initialize(GridData,  NumYears, NMSVPASpe);

    for (int i = 0; i < NMSVPASpe; ++i) {
        ColumnLabels << QString::fromStdString(MSVPASpeList(i));
        fields = {"Year","Abundance"};
        queryStr = "SELECT Year, Sum(" + abundVariable + ") AS Abundance FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND SpeName = '" + MSVPASpeList(i) + "'" +
                  seasonStr +
                " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int j = 0; j <  NumYears; ++j) {
            if (i == 0) {
                RowLabels << QString::number(FirstYear+j); // RSK improve on this, both RowLabels and X... aren't necessary
                XLabelNames.push_back(std::to_string(FirstYear + j));
            }
            ChartData(j,i) = std::stod(dataMap["Abundance"][j]) / 1000.0; // RSK replace with constant
        }
     }
    GridData = ChartData;

    loadChartWithData(chart,
                      ChartData,
                      MSVPASpeList,         // legend names
                      mainTitle,
                      "Year",               // x title
                      "Millions of Fish",   // y title
                      XLabelNames,
                      MaxScaleY,
                      nmfConstants::RotateLabels);

}

void
ChartLineMultispeciesPopulations::getAndLoadAge1PlusBiomassDataMSVPA(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string ModelName,
        std::string MSVPAName,
        std::string ForecastName,
        std::string ScenarioName,
        int FirstYear,
        int NumYears, // These are years selected from the gui
        std::string seasonStr,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        double &MaxScaleY,
        boost::numeric::ublas::matrix<double> &GridData)
{

    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    int NMSVPASpe=0;
    std::string mainTitle = "Age 1+ Biomass (Jan 1.) for MSVPA Species";
    std::vector<std::string> XLabelNames;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::vector<std::string> MSVPASpeList;
    std::vector<int> MSVPASpeAge;

    RowLabels.clear();
    ColumnLabels.clear();

    // Select sum database field
    std::string str1,str2;
    nmfUtils::split(seasonStr,"=",str1,str2);
    int season = stoi(str2);
    std::string abundVariable = (season == 0) ? "AnnBiomass" : "Biomass";

    getMSVPASpecies(databasePtr,MSVPAName,NMSVPASpe,MSVPASpeList);
    getMSVPASpeciesNamesAges(databasePtr,NMSVPASpe,MSVPASpeList,MSVPASpeAge);

    nmfUtils::initialize(ChartData, NumYears, NMSVPASpe);
    nmfUtils::initialize(GridData,  NumYears, NMSVPASpe);

    for (int i = 0; i < NMSVPASpe; ++i) {
        ColumnLabels << QString::fromStdString(MSVPASpeList(i));

        fields = {"Year","Biomass"};
        queryStr = "SELECT Year, Sum(" + abundVariable + ") AS Biomass FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                   " AND SpeName = '" + MSVPASpeList(i) + "'" +
                     seasonStr + " AND Age >= 1 GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int j = 0; j <  NumYears; ++j) {
            if (i == 0) {
                RowLabels << QString::number(FirstYear+j); // RSK improve on this, both RowLabels and X... aren't necessary
                XLabelNames.push_back(std::to_string(FirstYear + j));
            }
            ChartData(j,i) = std::stod(dataMap["Biomass"][j]) / 1000.0; // RSK replace with constant
        }
    }
    GridData = ChartData;

    loadChartWithData(chart,
                      ChartData,
                      MSVPASpeList,
                      mainTitle,
                      "Year",              // x title
                      "000 Metric Tons",   // y title
                      XLabelNames,
                      MaxScaleY,
                      true);

}

void
ChartLineMultispeciesPopulations::getAndLoadAge1PlusAbundanceDataMSVPA(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string ModelName,
        std::string MSVPAName,
        std::string ForecastName,
        std::string ScenarioName,
        int FirstYear,
        int NumYears, // These are years selected from the gui
        std::string seasonStr,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        double &MaxScaleY,
        boost::numeric::ublas::matrix<double> &GridData)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    int NMSVPASpe=0;
    std::string mainTitle = "Age 1+ Abundance (Jan 1.) of MSVPA Species";
    std::vector<std::string> XLabelNames;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::vector<std::string> MSVPASpeList;
    std::vector<int> MSVPASpeAge;
    RowLabels.clear();
    ColumnLabels.clear();

    // Select sum database field
    std::string str1,str2;
    nmfUtils::split(seasonStr,"=",str1,str2);
    int season = stoi(str2);
    std::string abundVariable = (season == 0) ? "AnnAbund" : "SeasAbund";

    getMSVPASpecies(databasePtr,MSVPAName,NMSVPASpe,MSVPASpeList);
    getMSVPASpeciesNamesAges(databasePtr,NMSVPASpe,MSVPASpeList,MSVPASpeAge);

    nmfUtils::initialize(ChartData, NumYears, NMSVPASpe);
    nmfUtils::initialize(GridData,  NumYears, NMSVPASpe);

    for (int i = 0; i < NMSVPASpe; ++i) {
        ColumnLabels << QString::fromStdString(MSVPASpeList(i));

        fields = {"Year","Abundance"};
        queryStr = "SELECT Year, Sum(" + abundVariable + ") AS Abundance FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND SpeName = '" + MSVPASpeList(i) + "'" +
                  seasonStr +
                " AND Age >= 1 GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int j = 0; j <  NumYears; ++j) {
            if (i == 0) {
                RowLabels << QString::number(FirstYear+j); // RSK improve on this, both RowLabels and X... aren't necessary
                XLabelNames.push_back(std::to_string(FirstYear + j));
            }
            ChartData(j,i) = std::stod(dataMap["Abundance"][j]) / 1000.0; // RSK replace with constant
        }
    }
    GridData = ChartData;

    loadChartWithData(chart,
                      ChartData,
                      MSVPASpeList,
                      mainTitle,
                      "Year",               // x title
                      "Millions of Fish",   // y title
                      XLabelNames,
                      MaxScaleY,
                      nmfConstants::RotateLabels);

}

void
ChartLineMultispeciesPopulations::getAndLoadSpawningStockBiomassDataMSVPA(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string ModelName,
        std::string MSVPAName,
        std::string ForecastName,
        std::string ScenarioName,
        int FirstYear,
        int NumYears, // These are years selected from the gui
        std::string seasonStr,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        double &MaxScaleY,
        boost::numeric::ublas::matrix<double> &GridData)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    int NMSVPASpe=0;
    int m=0;
    std::string mainTitle = "Spawning Stock Biomass (Jan 1.) for MSVPA Species";
    std::vector<std::string> XLabelNames;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::vector<std::string> MSVPASpeList;
    std::vector<int> MSVPASpeAge;
    Boost3DArrayDouble Maturity(
        boost::extents[nmfConstants::MaxTotalSpecies][nmfConstants::MaxNumberAges][nmfConstants::MaxNumberYears]);

    // Select sum database field
    std::string str1,str2;
    nmfUtils::split(seasonStr,"=",str1,str2);
    int season = stoi(str2);
    std::string abundVariable = (season == 0) ? "AnnBiomass" : "Biomass";
    RowLabels.clear();
    ColumnLabels.clear();

    getMSVPASpecies(databasePtr,MSVPAName,NMSVPASpe,MSVPASpeList);
    getMSVPASpeciesNamesAges(databasePtr,NMSVPASpe,MSVPASpeList,MSVPASpeAge);
    getMaturity(databasePtr,NumYears,FirstYear,NMSVPASpe,MSVPASpeList,MSVPASpeAge,Maturity);

    nmfUtils::initialize(ChartData, NumYears, NMSVPASpe);
    nmfUtils::initialize(GridData,  NumYears, NMSVPASpe);

    for (int i = 0; i < NMSVPASpe; ++i) {
        ColumnLabels << QString::fromStdString(MSVPASpeList(i));

        fields = {"Year","Age","Biomass"};
        queryStr = "SELECT Year, Age, Sum(" + abundVariable + ") AS Biomass FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND SpeName = '"  + MSVPASpeList(i) + "'" +
                  seasonStr +
                " GROUP By Year, Age";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        m = 0;
        for (int j = 0; j < NumYears; ++j) {
            if (i == 0) {
                RowLabels << QString::number(FirstYear+j); // RSK improve on this, both RowLabels and X... aren't necessary
                XLabelNames.push_back(std::to_string(FirstYear + j));
            }
            for (int k = 0; k < MSVPASpeAge[i]; ++k) {
                ChartData(j,i) += (Maturity[i][k][j] * std::stod(dataMap["Biomass"][m++]) / 1000.0); // RSK replace with constant
            }
            ChartData(j, i) *= 0.5;  // sex ratio again !
        }
    }
    GridData = ChartData;

    loadChartWithData(chart,
                      ChartData,
                      MSVPASpeList,
                      mainTitle,
                      "Year",              // x title
                      "000 Metric Tons",   // y title
                      XLabelNames,MaxScaleY,true);

}

void
ChartLineMultispeciesPopulations::getAndLoadTotalBiomassDataForecast(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string ModelName,
        std::string MSVPAName,
        std::string ForecastName,
        std::string ScenarioName,
        int FirstYear,
        int NumYears, // These are years selected from the gui
        std::string seasonStr,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        double &MaxScaleY,
        boost::numeric::ublas::matrix<double> &GridData)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    int NMSVPASpe=0;
    int Forecast_FirstYear;
    int Forecast_NYears;
    //int Forecast_LastYear;
    std::vector<std::string> XLabelNames;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::vector<std::string> MSVPASpeList;
    std::vector<int> MSVPASpeAge;
    Boost3DArrayDouble Maturity(
        boost::extents[nmfConstants::MaxTotalSpecies][nmfConstants::MaxNumberAges][nmfConstants::MaxNumberYears]);

    // Find number of Forecast years
    fields    = {"InitYear","NYears"};
    queryStr  = "SELECT InitYear,NYears FROM Forecasts WHERE MSVPAName = '" +
                MSVPAName + "' AND ForeName = '" + ForecastName + "'";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["NYears"].size() > 0) {
        Forecast_FirstYear = std::stoi(dataMap["InitYear"][0]);
        Forecast_NYears    = std::stoi(dataMap["NYears"][0]) + 1;
        //Forecast_LastYear  = Forecast_FirstYear + Forecast_NYears ;
    } else {
        //logger->logMsg(nmfConstants::Error,"No NYears data for Forecast: "+ForecastName);
        return;
    }

    RowLabels.clear();
    ColumnLabels.clear();

    getMSVPASpecies(databasePtr,MSVPAName,NMSVPASpe,MSVPASpeList);
    getMSVPASpeciesNamesAges(databasePtr,NMSVPASpe,MSVPASpeList,MSVPASpeAge);
    getMaturity(databasePtr,Forecast_NYears,FirstYear,NMSVPASpe,MSVPASpeList,MSVPASpeAge,Maturity);

    nmfUtils::initialize(ChartData, Forecast_NYears, NMSVPASpe);
    nmfUtils::initialize(GridData,  Forecast_NYears, NMSVPASpe);

    for (int j = 0; j < Forecast_NYears; ++j) {
        RowLabels << QString::number(Forecast_FirstYear+j);
    }

    for (int i = 0; i < NMSVPASpe; ++i) {
        ColumnLabels << QString::fromStdString(MSVPASpeList(i));
        fields = {"Year","Biomass"};
        queryStr = "SELECT Year, Sum(InitBiomass) AS Biomass FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                   " AND ForeName = '" + ForecastName + "'" +
                   " AND Scenario = '" + ScenarioName + "'" +
                   " AND SpeName = '" + MSVPASpeList(i) + "'" +
                   seasonStr + " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int j = 0; j < Forecast_NYears; ++j) {
            XLabelNames.push_back(std::to_string(Forecast_FirstYear + j));
            ChartData(j,i) = std::stod(dataMap["Biomass"][j]) / 1000.0; // RSK replace with constant
        }
    }
    GridData = ChartData;

    loadChartWithData(chart,
                      ChartData,
                      MSVPASpeList,
                      "Total Biomass (Jan 1.) for MSVPA Species",
                      "Year",              // x title
                      "000 Metric Tons",   // y title
                      XLabelNames,
                      MaxScaleY,
                      (Forecast_NYears > nmfConstants::NumCategoriesForVerticalNotation));
}



void
ChartLineMultispeciesPopulations::getAndLoadTotalAbundanceDataForecast(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string ModelName,
        std::string MSVPAName,
        std::string ForecastName,
        std::string ScenarioName,
        int FirstYear,
        int NumYears, // These are years selected from the gui
        std::string seasonStr,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        double &MaxScaleY,
        boost::numeric::ublas::matrix<double> &GridData)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    int NMSVPASpe=0;
    int Forecast_FirstYear;
    int Forecast_NYears;
    //int Forecast_LastYear;
    std::vector<std::string> XLabelNames;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::vector<std::string> MSVPASpeList;
    std::vector<int> MSVPASpeAge;
    Boost3DArrayDouble Maturity(
        boost::extents[nmfConstants::MaxTotalSpecies][nmfConstants::MaxNumberAges][nmfConstants::MaxNumberYears]);

    RowLabels.clear();
    ColumnLabels.clear();

    // Find number of Forecast years
    fields    = {"InitYear","NYears"};
    queryStr  = "SELECT InitYear,NYears FROM Forecasts WHERE MSVPAName = '" +
                MSVPAName + "' AND ForeName = '" + ForecastName + "'";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["NYears"].size() > 0) {
        Forecast_FirstYear = std::stoi(dataMap["InitYear"][0]);
        Forecast_NYears    = std::stoi(dataMap["NYears"][0]) + 1;
        //Forecast_LastYear  = Forecast_FirstYear + Forecast_NYears ;
    } else {
        //logger->logMsg(nmfConstants::Error,"No NYears data for Forecast: "+ForecastName);
        return;
    }

    getMSVPASpecies(databasePtr,MSVPAName,NMSVPASpe,MSVPASpeList);
    getMSVPASpeciesNamesAges(databasePtr,NMSVPASpe,MSVPASpeList,MSVPASpeAge);
    getMaturity(databasePtr,Forecast_NYears,FirstYear,NMSVPASpe,MSVPASpeList,MSVPASpeAge,Maturity);

    nmfUtils::initialize(ChartData, Forecast_NYears, NMSVPASpe);
    nmfUtils::initialize(GridData,  Forecast_NYears, NMSVPASpe);

    for (int j = 0; j < Forecast_NYears; ++j) {
        RowLabels << QString::number(Forecast_FirstYear+j);
    }

    for (int i = 0; i < NMSVPASpe; ++i) {
        ColumnLabels << QString::fromStdString(MSVPASpeList(i));
        fields = {"Year","Abundance"};
        queryStr = "SELECT Year, Sum(InitAbund) AS Abundance FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '"  + MSVPASpeList(i) + "'" +
                  seasonStr + " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int j = 0; j <  Forecast_NYears; ++j) {
            XLabelNames.push_back(std::to_string(Forecast_FirstYear + j));
            ChartData(j,i) = std::stod(dataMap["Abundance"][j]) / 1000.0; // RSK replace with constant
        }
     }
    GridData = ChartData;

    loadChartWithData(chart,
                      ChartData,
                      MSVPASpeList,         // legend names
                      "Total Abundance (Jan 1.) of MSVPA Species",
                      "Year",               // x title
                      "Millions of Fish",   // y title
                      XLabelNames,
                      MaxScaleY,
                      (Forecast_NYears > nmfConstants::NumCategoriesForVerticalNotation));

}


void
ChartLineMultispeciesPopulations::getAndLoadAge1PlusBiomassDataForecast(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string ModelName,
        std::string MSVPAName,
        std::string ForecastName,
        std::string ScenarioName,
        int FirstYear,
        int NumYears, // These are years selected from the gui
        std::string seasonStr,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        double &MaxScaleY,
        boost::numeric::ublas::matrix<double> &GridData)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    int NMSVPASpe=0;
    int Forecast_FirstYear;
    int Forecast_NYears;
    //int Forecast_LastYear;
    std::vector<std::string> XLabelNames;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::vector<std::string> MSVPASpeList;
    std::vector<int> MSVPASpeAge;
    Boost3DArrayDouble Maturity(
        boost::extents[nmfConstants::MaxTotalSpecies][nmfConstants::MaxNumberAges][nmfConstants::MaxNumberYears]);

    RowLabels.clear();
    ColumnLabels.clear();

    // Find number of Forecast years
    fields    = {"InitYear","NYears"};
    queryStr  = "SELECT InitYear,NYears FROM Forecasts WHERE MSVPAName = '" +
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

    getMSVPASpecies(databasePtr,MSVPAName,NMSVPASpe,MSVPASpeList);
    getMSVPASpeciesNamesAges(databasePtr,NMSVPASpe,MSVPASpeList,MSVPASpeAge);
    getMaturity(databasePtr,Forecast_NYears,FirstYear,NMSVPASpe,MSVPASpeList,MSVPASpeAge,Maturity);

    nmfUtils::initialize(ChartData, Forecast_NYears, NMSVPASpe);
    nmfUtils::initialize(GridData,  Forecast_NYears, NMSVPASpe);

    for (int j = 0; j <  Forecast_NYears; ++j) {
        RowLabels << QString::number(Forecast_FirstYear+j);
    }

    for (int i = 0; i < NMSVPASpe; ++i) {
        ColumnLabels << QString::fromStdString(MSVPASpeList(i));

        fields = {"Year","Biomass"};
        queryStr = "SELECT Year, Sum(InitBiomass) AS Biomass FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '"  + MSVPASpeList(i) + "'" +
                  seasonStr + " AND Age >= 1 GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int j = 0; j < Forecast_NYears; ++j) {
            XLabelNames.push_back(std::to_string(Forecast_FirstYear + j));
            ChartData(j,i) = std::stod(dataMap["Biomass"][j]) / 1000.0; // RSK replace with constant
        }
    }
    GridData = ChartData;

    loadChartWithData(chart,
                      ChartData,
                      MSVPASpeList,
                      "Age 1+ Biomass (Jan 1.) for MSVPA Species",
                      "Year",              // x title
                      "000 Metric Tons",   // y title
                      XLabelNames,
                      MaxScaleY,
                      (Forecast_NYears > nmfConstants::NumCategoriesForVerticalNotation));

}

void
ChartLineMultispeciesPopulations::getAndLoadAge1PlusAbundanceDataForecast(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string ModelName,
        std::string MSVPAName,
        std::string ForecastName,
        std::string ScenarioName,
        int FirstYear,
        int NumYears, // These are years selected from the gui
        std::string seasonStr,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        double &MaxScaleY,
        boost::numeric::ublas::matrix<double> &GridData)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    int NMSVPASpe=0;
    int Forecast_FirstYear;
    int Forecast_NYears;
    //int Forecast_LastYear;
    std::vector<std::string> XLabelNames;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::vector<std::string> MSVPASpeList;
    std::vector<int> MSVPASpeAge;
    Boost3DArrayDouble Maturity(
        boost::extents[nmfConstants::MaxTotalSpecies][nmfConstants::MaxNumberAges][nmfConstants::MaxNumberYears]);

    RowLabels.clear();
    ColumnLabels.clear();

    // Find number of Forecast years
    fields    = {"InitYear","NYears"};
    queryStr  = "SELECT InitYear,NYears FROM Forecasts WHERE MSVPAName = '" +
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

    getMSVPASpecies(databasePtr,MSVPAName,NMSVPASpe,MSVPASpeList);
    getMSVPASpeciesNamesAges(databasePtr,NMSVPASpe,MSVPASpeList,MSVPASpeAge);
    getMaturity(databasePtr,Forecast_NYears,FirstYear,NMSVPASpe,MSVPASpeList,MSVPASpeAge,Maturity);

    nmfUtils::initialize(ChartData, Forecast_NYears, NMSVPASpe);
    nmfUtils::initialize(GridData,  Forecast_NYears, NMSVPASpe);

    // Find number of Forecast years
    fields    = {"InitYear","NYears"};
    queryStr  = "SELECT InitYear,NYears FROM Forecasts WHERE MSVPAName = '" +
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

    for (int j = 0; j <  Forecast_NYears; ++j) {
        RowLabels << QString::number(Forecast_FirstYear+j);
    }

    for (int i = 0; i < NMSVPASpe; ++i) {
        ColumnLabels << QString::fromStdString(MSVPASpeList(i));

        fields = {"Year","Abundance"};
        queryStr = "SELECT Year, Sum(InitAbund) AS Abundance FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '"  + MSVPASpeList(i) + "'" +
                  seasonStr + " AND Age >= 1 GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int j = 0; j <  Forecast_NYears; ++j) {
            XLabelNames.push_back(std::to_string(Forecast_FirstYear + j));
            ChartData(j,i) = std::stod(dataMap["Abundance"][j]) / 1000.0; // RSK replace with constant
        }
    }
    GridData = ChartData;

    loadChartWithData(chart,
                      ChartData,
                      MSVPASpeList,
                      "Age 1+ Abundance (Jan 1.) of MSVPA Species",
                      "Year",               // x title
                      "Millions of Fish",   // y title
                      XLabelNames,
                      MaxScaleY,
                      (Forecast_NYears > nmfConstants::NumCategoriesForVerticalNotation));

}


void
ChartLineMultispeciesPopulations::getAndLoadSpawningStockBiomassDataForecast(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string ModelName,
        std::string MSVPAName,
        std::string ForecastName,
        std::string ScenarioName,
        int FirstYear,
        int NumYears, // These are years selected from the gui
        std::string seasonStr,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        double &MaxScaleY,
        boost::numeric::ublas::matrix<double> &GridData)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    int NMSVPASpe=0;
    int m=0;
    int Forecast_FirstYear;
    int Forecast_NYears;
    //int Forecast_LastYear;
    std::vector<std::string> XLabelNames;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::vector<std::string> MSVPASpeList;
    std::vector<int> MSVPASpeAge;
    Boost3DArrayDouble Maturity(
        boost::extents[nmfConstants::MaxTotalSpecies][nmfConstants::MaxNumberAges][nmfConstants::MaxNumberYears]);

    // Find number of Forecast years
    fields    = {"InitYear","NYears"};
    queryStr  = "SELECT InitYear,NYears FROM Forecasts WHERE MSVPAName = '" +
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

    RowLabels.clear();
    ColumnLabels.clear();

    getMSVPASpecies(databasePtr,MSVPAName,NMSVPASpe,MSVPASpeList);
    getMSVPASpeciesNamesAges(databasePtr,NMSVPASpe,MSVPASpeList,MSVPASpeAge);
    getMaturity(databasePtr,NumYears,FirstYear,NMSVPASpe,MSVPASpeList,MSVPASpeAge,Maturity);

    nmfUtils::initialize(ChartData, Forecast_NYears, NMSVPASpe);
    nmfUtils::initialize(GridData,  Forecast_NYears, NMSVPASpe);

    for (int j = 0; j <  Forecast_NYears; ++j) {
        RowLabels << QString::number(Forecast_FirstYear+j);
    }

    for (int i = 0; i < NMSVPASpe; ++i) {
        ColumnLabels << QString::fromStdString(MSVPASpeList(i));

        fields = {"Year","Age","Biomass"};

        queryStr = "SELECT Year, Age, Sum(InitBiomass) AS Biomass FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '"  + MSVPASpeList(i) + "'" +
                seasonStr + " GROUP By Year, Age";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        m = 0;
        for (int j = 0; j < Forecast_NYears; ++j) {

            XLabelNames.push_back(std::to_string(Forecast_FirstYear + j));
            for (int k = 0; k < MSVPASpeAge[i]; ++k) {
                ChartData(j,i) += (Maturity[i][k][j] * std::stod(dataMap["Biomass"][m++]) / 1000.0); // RSK replace with constant
            }
            ChartData(j,i) *= 0.5;  // sex ratio again !
        }
    }
    GridData = ChartData;

    loadChartWithData(chart,
                      ChartData,
                      MSVPASpeList,
                      "Spawning Stock Biomass (Jan 1.) for MSVPA Species",
                      "Year",              // x title
                      "000 Metric Tons",   // y title
                      XLabelNames,
                      MaxScaleY,
                      (Forecast_NYears > nmfConstants::NumCategoriesForVerticalNotation));

}


void
ChartLineMultispeciesPopulations::getMSVPASpecies(
        nmfDatabase* databasePtr,
        const std::string &MSVPAName,
        int &NMSVPASpecies,
        boost::numeric::ublas::vector<std::string> &MSVPASpeList)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    // Load all msvpa species and ages
    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND (Type = 0 or Type = 1)";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    NMSVPASpecies = dataMap["SpeName"].size();
    nmfUtils::initialize(MSVPASpeList,NMSVPASpecies);

    for (int i=0;i<NMSVPASpecies; ++i) {
        MSVPASpeList(i) = dataMap["SpeName"][i];
    }
}


void
ChartLineMultispeciesPopulations::getMSVPASpeciesNamesAges(
        nmfDatabase* databasePtr,
        const int &NMSVPASpe,
        boost::numeric::ublas::vector<std::string> &MSVPASpeList,
        std::vector<int> &MSVPASpeAge)

{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    for (int i=0; i<NMSVPASpe; ++i) {
        fields = {"MaxAge"};
        queryStr = "SELECT MaxAge FROM Species WHERE SpeName = '" + MSVPASpeList(i) + "'";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        MSVPASpeAge.push_back( std::stoi(dataMap["MaxAge"][0]) );
    }
}



void
ChartLineMultispeciesPopulations::getMaturity(
        nmfDatabase* databasePtr,
        const int &Forecast_NYears,
        const int &FirstYear,
        const int &NMSVPASpe,
        const boost::numeric::ublas::vector<std::string> &MSVPASpeList,
        const std::vector<int> &MSVPASpeAge,
        Boost3DArrayDouble &Maturity)
{
    int m = 0;
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    // Get Maturity data....RSK possibly make this an nmfDatabase function
    for (int i = 0; i < NMSVPASpe; ++i) {
        fields = {"PMature"};
        queryStr = "SELECT PMature FROM SpeMaturity WHERE SpeName = '" + MSVPASpeList(i) + "'" +
                   " AND Year = " + std::to_string(FirstYear) +
                   " ORDER By Age";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        m = 0;
        for (int j = 0; j <= MSVPASpeAge[i]; ++j) {
            for (int k = 0; k < Forecast_NYears; ++k) {
                Maturity[i][j][k] = std::stod(dataMap["PMature"][m]);
            }
            ++m;
        }
    }
} // end getMaturity



void
ChartLineMultispeciesPopulations::loadChartWithData(
        QChart *chart,
        boost::numeric::ublas::matrix<double> &ChartData,
        boost::numeric::ublas::vector<std::string> &LegendNames,
        std::string mainTitle,
        std::string xTitle,
        std::string yTitle,
        std::vector<std::string> xLabels,
        double MaxScaleY,
        bool rotateLabels)
{
    QLineSeries *series = NULL;

    chart->removeAxis(chart->axes(Qt::Vertical).back());
    chart->removeAxis(chart->axes(Qt::Horizontal).back());

    // Draw main chart title
    QFont mainTitleFont = chart->titleFont();
    mainTitleFont.setPointSize(14);
    mainTitleFont.setWeight(QFont::Bold);
    chart->setTitleFont(mainTitleFont);
    chart->setTitle(mainTitle.c_str());

    // Draw legend on right
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    // Create a new X axis, add labels and the title
    QCategoryAxis *axisX = new QCategoryAxis();
    int NumXLabels = xLabels.size();
    for (int i=0; i<NumXLabels; ++i) {
        axisX->append(xLabels[i].c_str(), i+1);
    }
   // axisX->setRange(0, NumXLabels);
    if ((NumXLabels > nmfConstants::NumCategoriesForVerticalNotation) && (rotateLabels))
        axisX->setLabelsAngle(-90);
    else
        axisX->setLabelsAngle(0);
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);

    // Create, scale, and label Y axis.
    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%.2f");
    axisY->setMin(0.0);
    for (unsigned int line=0; line<ChartData.size2(); ++line) {
        series = new QLineSeries;
        for (unsigned int val=0; val< ChartData.size1(); ++val) {
            static_yMax = (ChartData(val,line) > static_yMax) ? ChartData(val,line) : static_yMax;
            series->append(val+1,ChartData(val,line));
        }
        chart->addSeries(series);
        series->setName(QString::fromStdString(LegendNames(line)));
        series->setColor(QColor(QString::fromStdString(nmfConstants::LineColors[line % nmfConstants::LineColorNames.size()])));
        //chart->setAxisY(axisY,series);
        nmfUtilsQt::setAxisY(chart,axisY,series);
    }
    axisY = qobject_cast<QValueAxis *>(chart->axes(Qt::Vertical).at(0));
    if (MaxScaleY > 0.0) {
        axisY->setMax(MaxScaleY);
        axisY->setMin(0.0);
    } else {
        //axisY->applyNiceNumbers(); // Adjusting yaxis to be a rounder number, i.e. 838 should be 1000
        axisY->setMax(static_yMax);
        axisY->setMin(0.0);
        axisY->applyNiceNumbers();
    }

    // Set title on X axis
    //chart->setAxisX(axisX, series);
    nmfUtilsQt::setAxisX(chart,axisX,series);
    QFont titleFont = axisX->titleFont();
    titleFont.setPointSize(12);
    titleFont.setWeight(QFont::Bold);
    axisX->setTitleFont(titleFont);
    axisX->setTitleText(xTitle.c_str());
    axisY->setTitleFont(titleFont);
    axisY->setTitleText(yTitle.c_str());

} // end loadChartWithData
