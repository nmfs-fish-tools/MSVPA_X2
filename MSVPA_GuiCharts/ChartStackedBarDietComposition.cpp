#include "ChartStackedBarDietComposition.h"

#include "nmfConstants.h"

ChartStackedBarDietComposition::ChartStackedBarDietComposition(
        QTableWidget* theDataTable,
        nmfLogger*    theLogger)
{
    setObjectName("Diet Composition");

    DataTableWidget = theDataTable;
    logger          = theLogger;

    // Set up function map so don't have to use series of if...else... statements.
    GetDataFunctionMap["MSVPA Average"]               = getAverageDataMSVPA;
    GetDataFunctionMap["MSVPA Average by Year"]       = getAverageByYearDataMSVPA;
    GetDataFunctionMap["MSVPA Average by Season"]     = getAverageBySeasonDataMSVPA;
    GetDataFunctionMap["MSVPA By Year and Season"]    = getAverageByYearAndSeasonDataMSVPA;

    GetDataFunctionMap["Forecast Average"]            = getAverageDataForecast;
    GetDataFunctionMap["Forecast Average by Year"]    = getAverageByYearDataForecast;
    GetDataFunctionMap["Forecast Average by Season"]  = getAverageBySeasonDataForecast;
    GetDataFunctionMap["Forecast By Year and Season"] = getAverageByYearAndSeasonDataForecast;
}


void
ChartStackedBarDietComposition::callback_UpdateChart(nmfStructsQt::UpdateDataStruct data)
{
    QStringList RowLabels;
    QStringList ColumnLabels;
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;

    QChart*      DataChartWidget  = getChart();
    QChartView*  chartView        = getChartView();

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
    std::string  TitlePrefix                 = data.TitlePrefix;
    std::string  TitleSuffix                 = data.TitleSuffix;
    std::string  XLabel                      = data.XLabel;
    std::string  YLabel                      = data.YLabel;
    std::string  DataTypeLabel               = data.DataTypeLabel;
    int          Theme                       = data.Theme;

    if (ModelName.empty())
        ModelName = "MSVPA";

    std::vector<bool> GridLines = { data.HorizontalGridLines,
                                    data.VerticalGridLines };

    if (logger)
        logger->logMsg(nmfConstants::Normal,
                       "MSVPA ChartStackedBarDietComposition " +
                       DataTypeLabel + ", " +
                       SelectedSpecies + ", " +
                       SelectedVariable + ", " +
                       SelectedByVariables + ", " +
                       SelectedSeason + ", " +
                       SelectedSpeciesAgeSizeClass);


    // Clear chart and show chartView
    DataChartWidget->removeAllSeries();
    chartView->show();

    std::string FcnIndex = ModelName + " " + SelectedByVariables;

    // Load the chart data using the FunctionMap set up in the constructor.
    if (GetDataFunctionMap.find(FcnIndex) == GetDataFunctionMap.end()) {

        logger->logMsg(nmfConstants::Error,
                       "Function: " + FcnIndex + " not found in GetDataFunctionMap.");

    } else {

        GetDataFunctionMap[FcnIndex](
                databasePtr,
                ModelName,
                MSVPAName,
                ForecastName,
                ScenarioName,
                SelectedSpecies,
                SelectedSpeciesAgeSizeClass,
                SelectedSeason,
                GridData,
                ChartData,
                TitlePrefix,
                TitleSuffix,
                XLabel,
                YLabel,
                RowLabels,
                ColumnLabels);

    }
/*
    // Draw the chart now that you have all the data.
    loadChartWithData(chart,
                      ChartData,
                      ColumnLabels);
    setTitlesAndGridLines(chart,
                          series,
                          RowLabels,
                          SelectedSpecies,
                          nmfConstants::DontRearrangeTitle,
                          DataTypeLabel,
                          XLabel,      YLabel,
                          TitlePrefix, TitleSuffix,
                          GridLines);
*/

    // Construct main title
    std::string MainTitle = constructMainTitle(SelectedSpecies,
                 nmfConstants::DontRearrangeTitle,
                 DataTypeLabel, XLabel,
                 TitlePrefix, TitleSuffix);


    // Populate the chart
    populateChart(DataChartWidget,
                  ChartData,
                  RowLabels,
                  ColumnLabels,
                  MainTitle,
                  XLabel,
                  YLabel,
                  GridLines,
                  Theme);

    // Populate the data table that corresponds with the chart
    populateDataTable(DataTableWidget,
                      GridData,
                      RowLabels,
                      ColumnLabels,
                      7,3);
}


void
ChartStackedBarDietComposition::getAverageDataMSVPA(nmfDatabase* databasePtr,
                                               std::string &ModelName,
                                               std::string &MSVPAName,
                                               std::string &ForecastName,
                                               std::string &ScenarioName,
                                               std::string &SelectedSpecies,
                                               std::string &SelectedSpeciesAgeSizeClass,
                                               std::string &unused,
                                               boost::numeric::ublas::matrix<double> &GridData,
                                               boost::numeric::ublas::matrix<double> &ChartData,
                                               std::string &TitlePrefix,
                                               std::string &TitleSuffix,
                                               std::string &xLabel,
                                               std::string &yLabel,
                                               QStringList &RowLabels,
                                               QStringList &ColumnLabels)
{
    int m;
    int NPrey;
    int Nage = 0;
    int NumRecords;
    int NageOrSizeCategories;
    int SizeOffset;
    int MSVPA_FirstYear=0;
    int MSVPA_LastYear=0;
    int MSVPA_NYears=0;
    int MSVPA_NSeasons=0;
    std::string PredAge;
    std::string queryStr;
    std::string AgeSizePrefix = "";
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    TitlePrefix = "Average";

    // Get some initial data
    databasePtr->nmfGetMSVPAInitData(MSVPAName,
                                     MSVPA_FirstYear,
                                     MSVPA_LastYear,
                                     MSVPA_NYears,
                                     MSVPA_NSeasons);

    // Find number of Age groups
    fields = {"NumAges"};
    queryStr = "SELECT COUNT(DISTINCT(Age)) as NumAges from MSVPAprefs WHERE MSVPAname='" + MSVPAName +
            "' and SpeName='" + SelectedSpecies + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    Nage = std::stoi(dataMap["NumAges"][0]);
    databasePtr->nmfQueryMsvpaPreyList(SelectedSpeciesAgeSizeClass,
                                       SelectedSpecies, MSVPAName,
                                       PredAge, NPrey, ColumnLabels, false);

    nmfUtils::initialize(ChartData, Nage, NPrey);
    nmfUtils::initialize(GridData,  Nage, NPrey);

    // Calculate ChartData based upon passed in arguments
    fields = {"PredName","PredAge","PreyName","Year","Season","Diet"};
    for (int i=0; i<Nage; ++i) {
        for (int j=0; j<=NPrey-1; ++j) {
            queryStr = "SELECT PredName,PredAge,PreyName,Year,Season,Sum(PropDiet) as Diet FROM MSVPASuitPreyBiomass WHERE MSVPAName ='" + MSVPAName + "'" +
                    " AND PredName = '" + SelectedSpecies + "'" +
                    " AND PredAge = " + std::to_string(i) +
                    " AND PreyName = '" + ColumnLabels[j].toStdString() + "'" +
                    " GROUP BY PredName,PredAge,PreyName,Year,Season";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            NumRecords = dataMap["PredName"].size();
            if (NumRecords > 0) {
                m = 0;
                for (int k=0; k<MSVPA_NYears; ++k) {
                    for (int l=0; l<MSVPA_NSeasons; ++l) {
                        if (m < NumRecords) {
                            ChartData(i,j) += std::stod(dataMap["Diet"][m++]);
                        }
                    } // end for l
                } // end for k
                ChartData(i,j) /= (MSVPA_NSeasons*(MSVPA_NYears+1));
            } // end if
        } // end for j
    } // end for i
    GridData = ChartData;

    // If current species is of Type=3 then the x axis is Size Class, else it's Age Class
    AgeSizePrefix = "Age ";
    NageOrSizeCategories = Nage;
    SizeOffset = 0;
    fields = {"SpeName"};
    queryStr = "SELECT SpeName from MSVPAspecies WHERE MSVPAName='" + MSVPAName +
               "' and SpeName='" + SelectedSpecies + "' and Type=3";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["SpeName"].size() > 0) {
        if (dataMap["SpeName"][0] == SelectedSpecies) {
            AgeSizePrefix = "Size ";
        }
    }
    fields = {"SpeName","NumSizeCats"};
    queryStr = "SELECT SpeName,NumSizeCats from OtherPredSpecies WHERE SpeName='" + SelectedSpecies + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["SpeName"].size() > 0) {
        if (dataMap["SpeName"][0] == SelectedSpecies) {
            SizeOffset = 1;
            NageOrSizeCategories = std::stoi(dataMap["NumSizeCats"][0]);
        }
    }
    for (int ageOrSize=0; ageOrSize<NageOrSizeCategories; ++ageOrSize) {
        RowLabels << QString::fromStdString(AgeSizePrefix) + QString::number(ageOrSize+SizeOffset);
    }
    xLabel = AgeSizePrefix + "Class";
}



void
ChartStackedBarDietComposition::getAverageByYearDataMSVPA(nmfDatabase* databasePtr,
                                                     std::string &ModelName,
                                                     std::string &MSVPAName,
                                                     std::string &ForecastName,
                                                     std::string &ScenarioName,
                                                     std::string &SelectedSpecies,
                                                     std::string &SelectedSpeciesAgeSizeClass,
                                                     std::string &unused,
                                                     boost::numeric::ublas::matrix<double> &GridData,
                                                     boost::numeric::ublas::matrix<double> &ChartData,
                                                     std::string &TitlePrefix,
                                                     std::string &TitleSuffix,
                                                     std::string &xLabel,
                                                     std::string &yLabel,
                                                     QStringList &RowLabels,
                                                     QStringList &ColumnLabels)
{
    int NPrey;
    int MSVPA_FirstYear=0;
    int MSVPA_LastYear=0;
    int MSVPA_NYears=0;
    int MSVPA_NSeasons=0;
    std::string PredAge;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    TitlePrefix = "Average " + SelectedSpeciesAgeSizeClass;
    xLabel = "Year";

    // Get some initial data
    databasePtr->nmfGetMSVPAInitData(MSVPAName,
                                     MSVPA_FirstYear,
                                     MSVPA_LastYear,
                                     MSVPA_NYears,
                                     MSVPA_NSeasons);

    databasePtr->nmfQueryMsvpaPreyList(SelectedSpeciesAgeSizeClass, SelectedSpecies, MSVPAName,
                                      PredAge, NPrey, ColumnLabels, true);

    nmfUtils::initialize(ChartData, MSVPA_NYears, NPrey);
    nmfUtils::initialize(GridData,  MSVPA_NYears, NPrey);

    for (int i=0; i<NPrey; ++i) {
        fields = {"Year","Diet"};
        queryStr = "SELECT Year,Sum(PropDiet) as Diet FROM MSVPASuitPreyBiomass WHERE PredName = '" + SelectedSpecies +
                "' and PredAge = " + PredAge +
                " and MSVPAName='" + MSVPAName +
                "' and PreyName='" + ColumnLabels[i].toStdString() +
                "' GROUP BY Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (int j=0; j<MSVPA_NYears; ++j) {
            ChartData(j,i) = std::stod(dataMap["Diet"][j]) / MSVPA_NSeasons;
        }
    }
    GridData = ChartData;

    for (int year=MSVPA_FirstYear; year<=MSVPA_LastYear; ++year) {
        RowLabels << QString::number(year);
    }
}


void
ChartStackedBarDietComposition::getAverageBySeasonDataMSVPA(nmfDatabase* databasePtr,
                                                       std::string &ModelName,
                                                       std::string &MSVPAName,
                                                       std::string &ForecastName,
                                                       std::string &ScenarioName,
                                                       std::string &SelectedSpecies,
                                                       std::string &SelectedSpeciesAgeSizeClass,
                                                       std::string &unused,
                                                       boost::numeric::ublas::matrix<double> &GridData,
                                                       boost::numeric::ublas::matrix<double> &ChartData,
                                                       std::string &TitlePrefix,
                                                       std::string &TitleSuffix,
                                                       std::string &xLabel,
                                                       std::string &yLabel,
                                                       QStringList &RowLabels,
                                                       QStringList &ColumnLabels)
{
    int index;
    int NPrey;
    int MSVPA_FirstYear=0;
    int MSVPA_LastYear=0;
    int MSVPA_NYears=0;
    int MSVPA_NSeasons=0;
    std::string PredAge;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    TitlePrefix = "Average " + SelectedSpeciesAgeSizeClass;
    xLabel = "Season";

    // Get some initial data
    databasePtr->nmfGetMSVPAInitData(MSVPAName,
                                     MSVPA_FirstYear,
                                     MSVPA_LastYear,
                                     MSVPA_NYears,
                                     MSVPA_NSeasons);

    databasePtr->nmfQueryMsvpaPreyList(SelectedSpeciesAgeSizeClass, SelectedSpecies, MSVPAName,
                                      PredAge, NPrey, ColumnLabels, true);

    nmfUtils::initialize(ChartData, MSVPA_NSeasons, NPrey);
    nmfUtils::initialize(GridData,  MSVPA_NSeasons, NPrey);

    for (int i=0; i<NPrey; ++i) {
        fields = {"Season","Diet"};
        queryStr = "SELECT Season,Sum(PropDiet) as Diet FROM MSVPASuitPreyBiomass WHERE PredName = '" + SelectedSpecies +
                "' and PredAge = " + PredAge +
                "  and MSVPAName='" + MSVPAName +
                "' and PreyName='" + ColumnLabels[i].toStdString() +
                "' GROUP BY Season";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (unsigned int j=0; j<dataMap["Season"].size(); ++j) {
            index = std::stoi(dataMap["Season"][j]);
            ChartData(index,i) = std::stod(dataMap["Diet"][j]) / (MSVPA_NYears+1);
        }
    }
    GridData = ChartData;

    for (int season=0; season<MSVPA_NSeasons; ++season) {
        RowLabels << "Season " + QString::number(season+1);
    }
}

void
ChartStackedBarDietComposition::getAverageByYearAndSeasonDataMSVPA(nmfDatabase* databasePtr,
                                                              std::string &ModelName,
                                                              std::string &MSVPAName,
                                                              std::string &ForecastName,
                                                              std::string &ScenarioName,
                                                              std::string &SelectedSpecies,
                                                              std::string &SelectedSpeciesAgeSizeClass,
                                                              std::string &SelectedSeason,
                                                              boost::numeric::ublas::matrix<double> &GridData,
                                                              boost::numeric::ublas::matrix<double> &ChartData,
                                                              std::string &TitlePrefix,
                                                              std::string &TitleSuffix,
                                                              std::string &xLabel,
                                                              std::string &yLabel,
                                                              QStringList &RowLabels,
                                                              QStringList &ColumnLabels)
{
    int m;
    int NPrey;
    int MSVPA_FirstYear=0;
    int MSVPA_LastYear=0;
    int MSVPA_NYears=0;
    int MSVPA_NSeasons=0;
    std::string PredAge;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    TitlePrefix = "Average " + SelectedSpeciesAgeSizeClass;
    TitleSuffix = ": Season " + SelectedSeason;
    xLabel = "Year";

    SelectedSeason.erase(0,SelectedSeason.find(" "));
    int Season = std::stoi(SelectedSeason)-1;

    // Get some initial data
    databasePtr->nmfGetMSVPAInitData(MSVPAName,
                                     MSVPA_FirstYear,
                                     MSVPA_LastYear,
                                     MSVPA_NYears,
                                     MSVPA_NSeasons);

    databasePtr->nmfQueryMsvpaPreyList(SelectedSpeciesAgeSizeClass, SelectedSpecies, MSVPAName,
                                      PredAge, NPrey, ColumnLabels, true);

    nmfUtils::initialize(ChartData, MSVPA_NYears, NPrey);
    nmfUtils::initialize(GridData,  MSVPA_NYears, NPrey);

    fields = {"PredName","PredAge","Year","PreyName","Diet"};
    queryStr = "SELECT PredName,PredAge,Year,PreyName,Sum(PropDiet) as Diet FROM MSVPASuitPreyBiomass WHERE PredName = '" + SelectedSpecies +
            "' and PredAge = " + PredAge + " and Season = " + std::to_string(Season) + " and MSVPAName='" + MSVPAName +
            "' GROUP BY PredName,PredAge,Year,PreyName";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

    m = 0;
    for (int i=0; i<MSVPA_NYears; ++i) {
        for (int j=0; j<NPrey; ++j) {
            if (dataMap["PreyName"][m] == ColumnLabels[j].toStdString()) {
                ChartData(i,j) = std::stod(dataMap["Diet"][m]);
                ++m;
            } else {
                ChartData(i,j) = 0.0;
            }
        }
    }
    GridData = ChartData;

    for (int year=MSVPA_FirstYear; year<=MSVPA_LastYear; ++year) {
        RowLabels << QString::number(year);
    }
}



void
ChartStackedBarDietComposition::getAverageDataForecast(nmfDatabase* databasePtr,
                                               std::string &ModelName,
                                               std::string &MSVPAName,
                                               std::string &ForecastName,
                                               std::string &ScenarioName,
                                               std::string &SelectedSpecies,
                                               std::string &SelectedSpeciesAgeSizeClass,
                                               std::string &unused,
                                               boost::numeric::ublas::matrix<double> &GridData,
                                               boost::numeric::ublas::matrix<double> &ChartData,
                                               std::string &TitlePrefix,
                                               std::string &TitleSuffix,
                                               std::string &xLabel,
                                               std::string &yLabel,
                                               QStringList &RowLabels,
                                               QStringList &ColumnLabels)
{
    int m;
    int NPrey;
    int Nage = 0;
    int NumRecords;
    int NPreds;
    int NOthPreds;
    int MSVPA_FirstYear=0;
    int MSVPA_LastYear=0;
    int MSVPA_NYears=0;
    int MSVPA_NSeasons=0;
    int Forecast_NSeasons=0;
    int Forecast_NYears=2;
    std::string queryStr,queryStr2;
    std::vector<std::string> fields,fields2;
    std::map<std::string, std::vector<std::string> > dataMap,dataMap2;

    TitlePrefix = "Average";

    // Get some initial data
    databasePtr->nmfGetMSVPAInitData(MSVPAName,
                                     MSVPA_FirstYear,
                                     MSVPA_LastYear,
                                     MSVPA_NYears,
                                     MSVPA_NSeasons);

    Forecast_NSeasons = MSVPA_NSeasons;

    std::map<std::string,int> NPredAgeMap;
    std::map<std::string,int> PredTypeMap;
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

    // Find number of Forecast years
    fields    = {"NYears"};
    queryStr  = "SELECT NYears FROM Forecasts WHERE MSVPAName = '" +
                MSVPAName + "' AND ForeName = '" + ForecastName + "'";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["NYears"].size() > 0) {
        Forecast_NYears = std::stoi(dataMap["NYears"][0]);
    } else {
        //logger->logMsg(nmfConstants::Error,"No NYears data for Forecast: "+ForecastName);
        return;
    }

    // Load predator name and age lists for use later
    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "' AND Type = 0";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    NPreds = dataMap["SpeName"].size();

    for (int i = 0; i < NPreds; ++i) {
        PredList(i) = dataMap["SpeName"][i];
        fields2     = {"MaxAge"};
        queryStr2   = "SELECT MaxAge FROM Species WHERE SpeName = '" + PredList(i) + "'";
        dataMap2    = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        NPredAgeMap[PredList(i)]  = std::stoi(dataMap2["MaxAge"][0]);
        PredTypeMap[PredList(i)] = 0;
    } // end for i

    // Load Other Predator Names and Ages
    fields    = {"SpeName"};
    queryStr  = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "' AND Type = 3";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    NOthPreds = dataMap["SpeName"].size();

    for (int i = 0; i < NOthPreds; ++i) {
        OthPredList(i) = dataMap["SpeName"][i];
        fields2   = {"NumSizeCats"};
        queryStr2 = "SELECT NumSizeCats FROM OtherPredSpecies WHERE SpeName = '" + OthPredList(i) + "'";
        dataMap2  = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        NPredAgeMap[OthPredList(i)] = std::stoi(dataMap2["NumSizeCats"][0]);
        PredTypeMap[OthPredList(i)] = 1;
    } // end for i

    Nage = NPredAgeMap[SelectedSpecies];
    if (PredTypeMap[SelectedSpecies] == 0)
        --Nage;
    std::string dummyStr = "";
    databasePtr->nmfQueryForecastPreyList(MSVPAName, ForecastName, ScenarioName,
                                          SelectedSpecies, false, dummyStr, "",
                                          ColumnLabels);
    NPrey = ColumnLabels.size();
    if (NPrey == 0) {
        std::cout << "Error ChartStackedBarDietComposition::getAverageDataForecast NPrey = 0" << std::endl;
        return;
    }
    nmfUtils::initialize(ChartData, Nage+1, NPrey);
    nmfUtils::initialize(GridData,  Nage+1, NPrey);

    for (int i = 0; i <= Nage; ++i) {
        for (int j = 0; j < NPrey; ++j) {
            fields = {"PredName","PredAge","PreyName","Year","Season","Diet"};
            queryStr = "SELECT PredName, PredAge, PreyName, Year, Season, Sum(PropDiet) as Diet FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND ForeName = '" + ForecastName + "'" +
                    " AND Scenario = '" + ScenarioName + "'" +
                    " AND PredName = '" + SelectedSpecies + "'" +
                    " AND PredAge = "   + std::to_string(i) +
                    " AND PreyName = '" + ColumnLabels[j].toStdString() + "'" +
                    " GROUP By PredName, PredAge, PreyName, Year, Season ";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            NumRecords = dataMap["Diet"].size();
            if (NumRecords > 0) {
                m = 0;
                for (int k = 0; k<=Forecast_NYears; ++k) { // RSK improve these types of loops, very inefficient
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
    GridData = ChartData;

    xLabel = (PredTypeMap[SelectedSpecies] == 0) ? "Age Class" : "Size Class";
    if (xLabel == "Age Class") {
        for (int ageOrSize=0; ageOrSize<=Nage; ++ageOrSize) {
            RowLabels << "Age " + QString::number(ageOrSize);
        }
    } else {
        for (int ageOrSize=0; ageOrSize<=Nage; ++ageOrSize) {
            RowLabels << "Size " + QString::number(ageOrSize);
        }
    }

}




void
ChartStackedBarDietComposition::getAverageByYearDataForecast(nmfDatabase* databasePtr,
                                                     std::string &ModelName,
                                                     std::string &MSVPAName,
                                                     std::string &ForecastName,
                                                     std::string &ScenarioName,
                                                     std::string &SelectedSpecies,
                                                     std::string &SelectedSpeciesAgeSizeClass,
                                                     std::string &unused,
                                                     boost::numeric::ublas::matrix<double> &GridData,
                                                     boost::numeric::ublas::matrix<double> &ChartData,
                                                     std::string &TitlePrefix,
                                                     std::string &TitleSuffix,
                                                     std::string &xLabel,
                                                     std::string &yLabel,
                                                     QStringList &RowLabels,
                                                     QStringList &ColumnLabels)
{
    int NPrey;
    int MSVPA_FirstYear=0;
    int MSVPA_LastYear=0;
    int MSVPA_NYears=0;
    int MSVPA_NSeasons=0;
    int Forecast_NSeasons;
    int Forecast_FirstYear;
    int Forecast_LastYear;
    int Forecast_NYears=2;
    std::string PredAge;
    std::string PredAgeStr;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    TitlePrefix = "Average " + SelectedSpeciesAgeSizeClass;
    xLabel = "Year";

    // Get some initial data
    databasePtr->nmfGetMSVPAInitData(MSVPAName,
                                     MSVPA_FirstYear,
                                     MSVPA_LastYear,
                                     MSVPA_NYears,
                                     MSVPA_NSeasons);

    // Find number of Forecast years
    fields    = {"InitYear","NYears"};
    queryStr  = "SELECT InitYear,NYears FROM Forecasts WHERE MSVPAName = '" +
                MSVPAName + "' AND ForeName = '" + ForecastName + "'";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["NYears"].size() > 0) {
         Forecast_FirstYear = std::stoi(dataMap["InitYear"][0]);
        Forecast_NYears    = std::stoi(dataMap["NYears"][0]);
        Forecast_LastYear  = Forecast_FirstYear + Forecast_NYears;
    } else {
        //logger->logMsg(nmfConstants::Error,"No NYears data for Forecast: "+ForecastName);
        return;
    }

    Forecast_NSeasons = MSVPA_NSeasons;

    PredAge = SelectedSpeciesAgeSizeClass.erase(0,SelectedSpeciesAgeSizeClass.find(" "));
    PredAgeStr = " AND PredAge = " + PredAge;
    databasePtr->nmfQueryForecastPreyList(MSVPAName, ForecastName, ScenarioName,
                                          SelectedSpecies, true, PredAgeStr, "", ColumnLabels);
    NPrey = ColumnLabels.size();
    nmfUtils::initialize(ChartData, Forecast_NYears+1, NPrey);
    nmfUtils::initialize(GridData,  Forecast_NYears+1, NPrey);

    for (int i = 0; i<NPrey; ++i) {
        fields = {"Year","PDiet"};
        queryStr = "SELECT Year, Sum(PropDiet) as PDiet FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND PredName = '" + SelectedSpecies + "'" +
                " AND PredAge = "   + PredAge +
                " AND PreyName = '" + ColumnLabels[i].toStdString() + "'" +
                " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int j = 0; j <= Forecast_NYears; ++j) {
            ChartData(j,i) = std::stod(dataMap["PDiet"][j]) / Forecast_NSeasons;
        } // end for j
    } // end for i

    GridData = ChartData;

    for (int year=Forecast_FirstYear; year<=Forecast_LastYear; ++year) {
        RowLabels << QString::number(year);
    }

}


void
ChartStackedBarDietComposition::getAverageBySeasonDataForecast(nmfDatabase* databasePtr,
                                                       std::string &ModelName,
                                                       std::string &MSVPAName,
                                                       std::string &ForecastName,
                                                       std::string &ScenarioName,
                                                       std::string &SelectedSpecies,
                                                       std::string &SelectedSpeciesAgeSizeClass,
                                                       std::string &unused,
                                                       boost::numeric::ublas::matrix<double> &GridData,
                                                       boost::numeric::ublas::matrix<double> &ChartData,
                                                       std::string &TitlePrefix,
                                                       std::string &TitleSuffix,
                                                       std::string &xLabel,
                                                       std::string &yLabel,
                                                       QStringList &RowLabels,
                                                       QStringList &ColumnLabels)
{
    int NPrey;
    int MSVPA_FirstYear=0;
    int MSVPA_LastYear=0;
    int MSVPA_NYears=0;
    int MSVPA_NSeasons=0;
    int Forecast_NSeasons;
    int Forecast_NYears=2;
    int NumRecords;
    std::string PredAge;
    std::string PredAgeStr;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    TitlePrefix = "Average " + SelectedSpeciesAgeSizeClass;
    xLabel = "Season";

    // Get some initial data
    databasePtr->nmfGetMSVPAInitData(MSVPAName,
                                     MSVPA_FirstYear,
                                     MSVPA_LastYear,
                                     MSVPA_NYears,
                                     MSVPA_NSeasons);
    Forecast_NSeasons = MSVPA_NSeasons;

    // Find number of Forecast years
    fields    = {"InitYear","NYears"};
    queryStr  = "SELECT InitYear,NYears FROM Forecasts WHERE MSVPAName = '" +
                MSVPAName + "' AND ForeName = '" + ForecastName + "'";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["NYears"].size() > 0) {
        Forecast_NYears    = std::stoi(dataMap["NYears"][0]);
    } else {
        //logger->logMsg(nmfConstants::Error,"No NYears data for Forecast: "+ForecastName);
        return;
    }

    PredAge = SelectedSpeciesAgeSizeClass.erase(0,SelectedSpeciesAgeSizeClass.find(" "));
    PredAgeStr = " AND PredAge = " + PredAge;
    ColumnLabels.clear();
    databasePtr->nmfQueryForecastPreyList(MSVPAName, ForecastName, ScenarioName,
                                          SelectedSpecies, true, PredAgeStr,
                                          "", ColumnLabels);

    NPrey = ColumnLabels.size();
    nmfUtils::initialize(ChartData, Forecast_NSeasons, NPrey);
    nmfUtils::initialize(GridData,  Forecast_NSeasons, NPrey);

    for (int i = 0; i <NPrey; ++i) {
        fields = {"Season","Diet"};
        queryStr = "SELECT Season, Sum(PropDiet) as Diet FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND PredName = '" + SelectedSpecies + "'" +
                " AND PredAge = "   + PredAge +
                " AND PreyName = '" + ColumnLabels[i].toStdString() + "'" +
                " GROUP By Season";

        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        NumRecords = dataMap["Diet"].size();

        for (int j = 0; j < Forecast_NSeasons; ++j) {
            if (j < NumRecords) {
                if (std::stoi(dataMap["Season"][j]) == j) {
                    ChartData(j,i) = std::stod(dataMap["Diet"][j]) / (Forecast_NYears + 1);
                } // end if
            }
        } // end for j
    } // end for i
    GridData = ChartData;

    for (int season=0; season<MSVPA_NSeasons; ++season) {
        RowLabels << "Season " + QString::number(season+1);
    }
}


void
ChartStackedBarDietComposition::getAverageByYearAndSeasonDataForecast(nmfDatabase* databasePtr,
                                                              std::string &ModelName,
                                                              std::string &MSVPAName,
                                                              std::string &ForecastName,
                                                              std::string &ScenarioName,
                                                              std::string &SelectedSpecies,
                                                              std::string &SelectedSpeciesAgeSizeClass,
                                                              std::string &SelectedSeason,
                                                              boost::numeric::ublas::matrix<double> &GridData,
                                                              boost::numeric::ublas::matrix<double> &ChartData,
                                                              std::string &TitlePrefix,
                                                              std::string &TitleSuffix,
                                                              std::string &xLabel,
                                                              std::string &yLabel,
                                                              QStringList &RowLabels,
                                                              QStringList &ColumnLabels)
{
    int m;
    int NPrey;
    int MSVPA_FirstYear=0;
    int MSVPA_LastYear=0;
    int MSVPA_NYears=0;
    int MSVPA_NSeasons=0;
    int Forecast_FirstYear=0;
    int Forecast_LastYear=0;
    int Forecast_NYears=2;
    int NumRecords;
    //int Forecast_NSeasons;
    std::string PredAge;
    std::string PredAgeStr;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    TitlePrefix = "Average " + SelectedSpeciesAgeSizeClass;
    TitleSuffix = ": " + SelectedSeason;
    xLabel = "Year";

    SelectedSeason.erase(0,SelectedSeason.find(" "));
    //int Season = std::stoi(SelectedSeason)-1;

    // Get some initial data
    databasePtr->nmfGetMSVPAInitData(MSVPAName,
                                     MSVPA_FirstYear,
                                     MSVPA_LastYear,
                                     MSVPA_NYears,
                                     MSVPA_NSeasons);
    //Forecast_NSeasons = MSVPA_NSeasons;

    // Find number of Forecast years
    fields    = {"InitYear","NYears"};
    queryStr  = "SELECT InitYear,NYears FROM Forecasts WHERE MSVPAName = '" +
                MSVPAName + "' AND ForeName = '" + ForecastName + "'";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["NYears"].size() > 0) {
        Forecast_FirstYear = std::stoi(dataMap["InitYear"][0]);
        Forecast_NYears    = std::stoi(dataMap["NYears"][0]) + 1;
        Forecast_LastYear  = Forecast_FirstYear + Forecast_NYears -1;
    } else {
        //logger->logMsg(nmfConstants::Error,"No NYears data for Forecast: "+ForecastName);
        return;
    }

    PredAge = SelectedSpeciesAgeSizeClass.erase(0,SelectedSpeciesAgeSizeClass.find(" "));
    SelectedSeason = SelectedSeason.erase(0,SelectedSeason.find(" "));

    int SelectedSeasonInt = std::stoi(SelectedSeason)-1;
    PredAgeStr = " AND PredAge = " + PredAge;
    ColumnLabels.clear();

    databasePtr->nmfQueryForecastPreyList(MSVPAName, ForecastName, ScenarioName,
                                          SelectedSpecies, true, PredAgeStr, "",
                                          ColumnLabels);
    NPrey = ColumnLabels.size();
    nmfUtils::initialize(ChartData, Forecast_NYears, NPrey);
    nmfUtils::initialize(GridData,  Forecast_NYears, NPrey);

    fields = {"PredName","PredAge","Year","PreyName","Diet"};
    queryStr = "SELECT PredName,PredAge,Year,PreyName,Sum(PropDiet) as Diet FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND ForeName = '" + ForecastName +  "'" +
               " AND Scenario = '" + ScenarioName + "'" +
               " AND PredName = '" + SelectedSpecies + "'" +
               " AND PredAge = "   + PredAge +
               " AND Season = "    + std::to_string(SelectedSeasonInt) +
               " GROUP By PredName, PredAge, Year, PreyName";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    m = 0;
    NumRecords = dataMap["Diet"].size();
    for (int i = 0; i < Forecast_NYears; ++i) {
        for (int j = 0; j < NPrey; ++j) {
            if (m < NumRecords) {
                if (dataMap["PreyName"][m] == ColumnLabels[j].toStdString()) {
                    ChartData(i,j) = std::stod(dataMap["Diet"][m++]);
                } else {
                    ChartData(i,j) = 0.0;
                }
            }
        } // end for j
    } // end for i
    GridData = ChartData;

    for (int year=Forecast_FirstYear; year<=Forecast_LastYear; ++year) {
        RowLabels << QString::number(year);
    }
    SelectedSeason = std::to_string(SelectedSeasonInt+1);
}

