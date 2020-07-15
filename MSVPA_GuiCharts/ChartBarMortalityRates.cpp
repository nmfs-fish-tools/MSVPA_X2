
#include "ChartBarMortalityRates.h"

#include "nmfConstants.h"

ChartBarMortalityRates::ChartBarMortalityRates(
        QVBoxLayout*                 theVlayt,
        nmfOutputChart3DBarModifier* theM_modifier,
        QWidget*                     theChartView3DContainer,
        QTableWidget*                theDataTable,
        nmfLogger*                   theLogger)
{
    setObjectName("Mortality Rates");

    DataTable            = theDataTable;
    logger               = theLogger;
    vlayt                = theVlayt;
    chartView3DContainer = theChartView3DContainer;
    m_modifier           = theM_modifier;

    // Set up function map so don't have to use series of if...else... statements.
    GetDataFunctionMap["MSVPA Fishing Mortality"]                  = getAndLoadFishingMortalityDataMSVPA;
    GetDataFunctionMap["MSVPA Average Recruited F"]                = getAndLoadAverageRecruitedFDataMSVPA;
    GetDataFunctionMap["MSVPA Predation Mortality"]                = getAndLoadPredationMortalityDataMSVPA;
    GetDataFunctionMap["MSVPA Predation Mortality - 3D"]           = getAndLoadPredationMortality3DDataMSVPA;
    GetDataFunctionMap["MSVPA Predation Mortality by Predator"]    = getAndLoadPredationMortalityByPredatorDataMSVPA;

    GetDataFunctionMap["Forecast Total Fishery Catch (Numbers)"]   = getAndLoadTotalFisheryCatchDataForecast;
    GetDataFunctionMap["Forecast Fishery Catch at Age (Numbers)"]  = getAndLoadFisheryCatchAtAgeDataForecast;
    GetDataFunctionMap["Forecast Total Fishery Yield (Biomass)"]   = getAndLoadTotalFisheryYieldDataForecast;
    GetDataFunctionMap["Forecast Fishery Yield at Age (Biomass)"]  = getAndLoadFisheryYieldAtAgeDataForecast;
    GetDataFunctionMap["Forecast Fishery Mortality Rate at Age"]   = getAndLoadFisheryMortalityRateAtAgeDataForecast;
    GetDataFunctionMap["Forecast Average Recruited F"]             = getAndLoadAverageRecruitedFDataForecast;
    GetDataFunctionMap["Forecast Predation Mortality Rate"]        = getAndLoadPredationMortalityRateDataForecast;
    GetDataFunctionMap["Forecast Predation Mortality by Predator"] = getAndLoadPredationMortalityByPredatorDataForecast;

}


void
ChartBarMortalityRates::callback_UpdateChart(nmfStructsQt::UpdateDataStruct data)
{
    int MSVPA_FirstYear = 0;
    int MSVPA_LastYear = 0;
    int MSVPA_NYears = 0;
    int MSVPA_NSeasons = 0;
    int SeasonInt=0;
    int offset = 0;
    int SpeAge = 0;
    double NConversion=0;
    double CatchUnits=0;
    std::string yLabel="000 Metric Tons";
    std::string yLabelConversion="";
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    QStringList ColumnLabels;
    QStringList RowLabels;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;

    QChart*      chart     = getChart();
    QChartView*  chartView = getChartView();

    int          Nage                        = data.NumAgeSizeClasses;
    std::string  MaxScaleY                   = data.MaxScaleY.toStdString();
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
    std::string  SelectedMode                = data.SelectMode.toStdString();
    int          Theme                       = data.Theme;

    nmfOutputChart3DBarModifier *m_modifier2 = m_modifier;
    QVBoxLayout* vlayt2                      = vlayt;
    QWidget *chartView3DContainer2           = chartView3DContainer;

    logger->logMsg(nmfConstants::Normal,
                      "MSVPA ChartBarMortalityRates " +
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

    // Find NConversion scale factor which is used in some cases
    fields = {"WtUnits","CatchUnits"};
    queryStr = "SELECT WtUnits,CatchUnits from Species where SpeName='" + SelectedSpecies + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["WtUnits"].size() > 0) {
        CatchUnits = std::stod(dataMap["CatchUnits"][0]);
    }

    std::vector<std::string> yLabels = {"Hundreds of Fish",
                                        "Thousands of Fish",
                                        "Millions of Fish",
                                        "Billions of Fish"};
    NConversion = 1000.0 * CatchUnits;
    if (CatchUnits == 0.1) {
        yLabelConversion = yLabels[0];
    } else if (CatchUnits == 1.0) {
        yLabelConversion = yLabels[1];
    } else if (CatchUnits == 1000.0) {
        yLabelConversion = yLabels[2];
    } else if (CatchUnits == 1000000.0) {
        yLabelConversion = yLabels[3];
    }

    std::string FcnIndex = ModelName + " " + SelectedVariable;

    // Load the chart data using the FunctionMap set up in the constructor.
    if (GetDataFunctionMap.find(FcnIndex) == GetDataFunctionMap.end()) {

        logger->logMsg(nmfConstants::Error,
                       "ChartBarMortalityRates Function: \"" + FcnIndex + "\" not found in GetDataFunctionMap.");

    } else {

        GetDataFunctionMap[FcnIndex](
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
                SelectedMode,
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
                yLabelConversion,
                NConversion,
                vlayt2,
                m_modifier2,
                chartView3DContainer2,
                Theme);

    }

    if (SelectedVariable != "Predation Mortality - 3D") {
        callback_UpdateChartGridLines(data);

        // Populate the data table that corresponds with the chart
        populateDataTable(DataTable,GridData,RowLabels,ColumnLabels,7,3);
    }

}



void
ChartBarMortalityRates::getAndLoadFishingMortalityDataMSVPA(
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
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *m_modifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    std::string title;
    std::string seasonStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    std::string predAgeStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;

    chartView3DContainer->hide();

    chart->removeAllSeries();

    ColumnLabels << "Fishing Mortality";

    if (SelectedByVariables == "Annual") {
        fieldToSum = "SeasF";
        title      = "Fishing Mortality Rate";
        seasonStr  = "";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum  = "SeasF";
        title       = "Fishing Mortality Rate: ";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    predAgeStr = " and Age = " + std::to_string(SpeAge);

    getDataAndLoadChart("Mortality Rates",
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
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        "Fishing Mortality", 0.001, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    // For age specific data..we want to see the whole matrix and be able to save that out...
    // so have to do a new query and everything
    nmfUtils::initialize(GridData, MSVPA_NYears, Nage);
    int m = 0;
    RowLabels.clear();
    ColumnLabels.clear();
    fields = {"Year", "SumField"};
    queryStr = "Select Year, Sum(" + fieldToSum + ") As SumField FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'"
               " AND SpeName = '" + SelectedSpecies + "'" +
               seasonStr + " GROUP BY Year, Age";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (int i=0; i<MSVPA_NYears; ++i) {
        RowLabels << QString::number(MSVPA_FirstYear+i);
        for (int j=0; j<Nage; ++j) {
            if (i == 0) {
                ColumnLabels << QString::fromStdString("Age "+std::to_string(j));
            }
            GridData(i,j) = nmfUtils::round(std::stod(dataMap["SumField"][m++]),3);
        } // end for j
    } // end for i

}


void
ChartBarMortalityRates::getAndLoadAverageRecruitedFDataMSVPA(
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
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedVariable,
        std::string &SelectedByVariables,
        std::string &SelectedSeason,
        std::string &SelectedPreyName,
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *m_modifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    std::string title;
    std::string seasonStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    std::string predAgeStr;

    chartView3DContainer->hide();

    ColumnLabels << "Average F";

    if (SelectedByVariables == "Annual") {
        fieldToSum = "SeasF";
        title = "Average Fishing Mortality Rate";
        seasonStr = "";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum = "SeasF";
        title = "Average Fishing Mortality Rate: ";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }
    predAgeStr = " and Age >= " + std::to_string(SpeAge);

    getDataAndLoadChart("Mortality Rates", databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage, fieldToSum, MSVPAName,
                        selectedSpecies, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        predAgeStr, "", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear, SpeAge, chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        "Fishing Mortality", 0.001, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

}


void
ChartBarMortalityRates::getAndLoadPredationMortalityDataMSVPA(
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
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *m_modifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    int m;
    std::string title;
    std::string seasonStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    std::string predAgeStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string queryStr;
    std::string valStr;

    chartView3DContainer->hide();

    ColumnLabels << "M2 by Age";

    if (SelectedByVariables == "Annual") {
        fieldToSum = "SeasM2";
        title      = "Predation Mortality Rate";
        seasonStr  = "";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum  = "SeasM2";
        title       = "Predation Mortality Rate: ";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    predAgeStr = " and Age = " + std::to_string(SpeAge);

    getDataAndLoadChart("Mortality Rates", databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage, fieldToSum, MSVPAName,
                        SelectedSpecies, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        predAgeStr, "", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear, SpeAge, chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                         "Predation Mortality (M2)", 0.001, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    RowLabels.clear();
    ColumnLabels.clear();
    nmfUtils::initialize(GridData, MSVPA_NYears, Nage);
    m = 0;
    fields = {"Year", "SumField"};
    queryStr = "Select Year, Sum(SeasM2) As SumField FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'"
               " AND SpeName  = '" + SelectedSpecies + "'" +
               seasonStr + " GROUP BY Year, Age";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (int i=0; i<MSVPA_NYears; ++i) {
        RowLabels << QString::number(MSVPA_FirstYear+i);
        for (int j=0; j<Nage; ++j) {
            if (i == 0) {
                ColumnLabels << QString::fromStdString("Age "+std::to_string(j));
            }
            valStr = dataMap["SumField"][m++];
            GridData(i,j) = (valStr.empty()) ? 0 : nmfUtils::round(std::stod(valStr),3);
        } // end for j
    } // end for i

}

void
ChartBarMortalityRates::getAndLoadPredationMortalityByPredatorDataMSVPA(
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
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *m_modifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    std::string title;
    std::string seasonStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum = "AnnBiomass";
    std::string predAgeStr;

    chartView3DContainer->hide();

    ColumnLabels << "M2 by Predator";

    if (SelectedByVariables == "Annual") {
        fieldToSum = "BMConsumed";
        title      = "M2 by Predator";
        seasonStr  = "";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum  = "BMConsumed";
        title       = "M2 by Predator: ";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }
    predAgeStr = " and Age = " + std::to_string(SpeAge);

    getDataAndLoadChart("Mortality Rates", databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage, fieldToSum, MSVPAName,
                        SelectedSpecies, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        predAgeStr, "", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear, SpeAge, chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                         "Predation Mortality", 0.001, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData,ChartData.size1(),ChartData.size2());
    GridData = ChartData;
}


void
ChartBarMortalityRates::getYearsAndAges(
        nmfDatabase *databasePtr,
        const std::string &MSVPAName,
        const std::string &species,
        int &NumYears,
        int &NumAges)
{
    std::string queryStr = "";
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    // Find NumYears and NumAges per species
    fields   = {"NumYears","NumAges"};
    queryStr = "SELECT count(DISTINCT(Year)) as NumYears, count(DISTINCT(Age)) as NumAges FROM MSVPASeasBiomass WHERE MSVPAName = '" + MSVPAName +
               "' and SpeName='" + species + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumYears = std::stoi(dataMap["NumYears"][0]);
    NumAges  = std::stoi(dataMap["NumAges"][0]);
}



void
ChartBarMortalityRates::clearWidgets(QVBoxLayout *vlayt)
{
    for (int i = 0; i < vlayt->count(); ++i)
    {
        QWidget *widget = vlayt->itemAt(i)->widget();
        if (widget != NULL)
        {
            widget->setVisible(false);
        }
    }
}

void
ChartBarMortalityRates::getAndLoadPredationMortality3DDataMSVPA(
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
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *chartModifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    int NumAges;
    int NumYears;
    int NumSpecies;
    std::string queryStr = "";
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::vector<std::string> species;
    std::map<std::string, int> NumYearsMap;
    std::map<std::string, int> NumAgesMap;
    boost::numeric::ublas::matrix<double> M2Matrix;

    chart->setTheme(static_cast<QChart::ChartTheme>(Theme));

    //
    // Load the data matrix for 3d bar plot
    //

    // Find all species
    fields = {"SpeName"};
    queryStr = "SELECT SpeName from MSVPAspecies WHERE MSVPAName='" + MSVPAName + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumSpecies = dataMap["SpeName"].size();
    for (int i=0;i<NumSpecies;++i) {
        species.push_back(dataMap["SpeName"][i]);
    }

    // Get NumYears and NumAges for all species and read into maps
    for (int i=0; i<NumSpecies; ++i) {
        getYearsAndAges(databasePtr,MSVPAName,species[i],NumYears,NumAges);
        NumYearsMap[species[i]] = NumYears;
        NumAgesMap[species[i]]  = NumAges;
    }

    // Load M2 rates
    fields = {"Year","Age","M2"};
    queryStr = "SELECT Year,Age,Sum(SeasM2) as M2 FROM MSVPASeasBiomass WHERE MSVPAName = '" + MSVPAName +"'" +
               " and SpeName='" + SelectedSpecies + "'" +
               " GROUP BY Year, Age";

    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);  // RSK - error message here if queried table is empty
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

    //
    // Draw Graph
    //
    clearWidgets(vlayt);

    if (SelectedMode == "No Selection")
        chartModifier->getGraph()->setSelectionMode(QAbstract3DGraph::SelectionNone);
    else if (SelectedMode == "Item")
        chartModifier->getGraph()->setSelectionMode(QAbstract3DGraph::SelectionItem);
    else if (SelectedMode == "Row Slice") {
        chartModifier->getGraph()->setSelectionMode(QAbstract3DGraph::SelectionItemAndRow);
    } else if (SelectedMode == "Column Slice")
        chartModifier->getGraph()->setSelectionMode(QAbstract3DGraph::SelectionItemAndColumn);

    chartModifier->resetData(M2Matrix);
    chartView3DContainer->show();

}

void
ChartBarMortalityRates::loadChartWithData(
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
ChartBarMortalityRates::setTitles(
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


void
ChartBarMortalityRates::getAndLoadTotalFisheryCatchDataForecast(
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
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *m_modifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    int seasonVal=0;
    double CatchUnits=NConversion/1000.0;
    std::string title;
    std::string seasonStr = "";
    std::string titleSuffix = "";
    std::string fieldToSum = "AnnBiomass";
    std::string predAgeStr;

    chart->removeAllSeries();

    yLabel = "Catch in " + yLabelConversion;

    ColumnLabels << "Average F";

    if (SelectedByVariables == "Annual") {
        title     = "Catch At Age";
        seasonStr = "";
    } else if (SelectedByVariables == "Seasonal") {
        title       = "Catch At Age";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    fieldToSum = "SeasF";
    predAgeStr = " and Age = " + std::to_string(SpeAge);

    getDataAndLoadChart("Mortality Rates",
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
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel, CatchUnits, NConversion, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);
}


void
ChartBarMortalityRates::getAndLoadFisheryCatchAtAgeDataForecast(
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
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *m_modifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    int seasonVal=0;
    double CatchUnits=NConversion/1000.0;
    std::string title;
    std::string seasonStr = "";
    std::string titleSuffix = "";
    std::string fieldToSum = "AnnBiomass";
    std::string predAgeStr;

    chart->removeAllSeries();

    yLabel = "Catch in " + yLabelConversion;

    ColumnLabels << "Catch (Numbers)";

    if (SelectedByVariables == "Annual") {
        title     = "Catch at Age";
        seasonStr = "";
    } else if (SelectedByVariables == "Seasonal") {
        title       = "Catch at Age";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    fieldToSum = "SeasF";
    predAgeStr = " and Age = " + std::to_string(SpeAge);

    getDataAndLoadChart("Mortality Rates",
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
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel, CatchUnits, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);
}


void
ChartBarMortalityRates::getAndLoadTotalFisheryYieldDataForecast(
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
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *m_modifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    int seasonVal=0;
    double CatchUnits=NConversion/1000.0;
    std::string title;
    std::string seasonStr = "";
    std::string titleSuffix = "";
    std::string fieldToSum = "AnnBiomass";
    std::string predAgeStr;

    chart->removeAllSeries();

    yLabel = "Catch in " + yLabelConversion;

    ColumnLabels << "Yield";

    if (SelectedByVariables == "Annual") {
        title     = "Total Fishery Yield";
        seasonStr = "";
    } else if (SelectedByVariables == "Seasonal") {
        title       = "Total Fishery Yield";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    fieldToSum = "SeasF";
    predAgeStr = " and Age = " + std::to_string(SpeAge);

    getDataAndLoadChart("Mortality Rates",
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
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel, CatchUnits, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

}


void
ChartBarMortalityRates::getAndLoadFisheryYieldAtAgeDataForecast(
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
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *m_modifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    int seasonVal=0;
    double CatchUnits=NConversion/1000.0;
    std::string title;
    std::string seasonStr = "";
    std::string titleSuffix = "";
    std::string fieldToSum = "AnnBiomass";
    std::string predAgeStr;

    chart->removeAllSeries();

    yLabel = "Yield in " + yLabelConversion;

    ColumnLabels << "Yield";

    if (SelectedByVariables == "Annual") {
        title     = "Fishery Yield at Age";
        seasonStr = "";
    } else if (SelectedByVariables == "Seasonal") {
        title       = "Fishery Yield at Age";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    fieldToSum = "SeasF";
    predAgeStr = " and Age = " + std::to_string(SpeAge);

    getDataAndLoadChart("Mortality Rates",
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
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel, CatchUnits, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

}


void
ChartBarMortalityRates::getAndLoadFisheryMortalityRateAtAgeDataForecast(
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
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *m_modifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    int seasonVal=0;
    double CatchUnits=NConversion/1000.0;
    std::string title;
    std::string seasonStr = "";
    std::string titleSuffix = "";
    std::string fieldToSum = "AnnBiomass";
    std::string predAgeStr;

    chart->removeAllSeries();

    yLabel = "Fishery Mortality (F)";

    ColumnLabels << "F at Age";

    if (SelectedByVariables == "Annual") {
        title     = "Fishery Mortality Rate";
        seasonStr = "";
    } else if (SelectedByVariables == "Seasonal") {
        title       = "Fishery Mortality Rate";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    fieldToSum = "SeasF";
    predAgeStr = " and Age >= " + std::to_string(SpeAge);

    getDataAndLoadChart("Mortality Rates",
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
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel, CatchUnits, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

}


void
ChartBarMortalityRates::getAndLoadAverageRecruitedFDataForecast(
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
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *m_modifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    int seasonVal=0;
    double CatchUnits=NConversion/1000.0;
    std::string title;
    std::string seasonStr = "";
    std::string titleSuffix = "";
    std::string fieldToSum = "AnnBiomass";
    std::string predAgeStr;

    chart->removeAllSeries();

    yLabel = "Fishing Mortality";

    ColumnLabels << "Average F";

    if (SelectedByVariables == "Annual") {
        title     = "Average Fishing Mortality Rate";
        seasonStr = "";
    } else if (SelectedByVariables == "Seasonal") {
        title       = "Average Fishing Mortality Rate";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    fieldToSum = "SeasF";
    predAgeStr = " and Age >= " + std::to_string(SpeAge);

    getDataAndLoadChart("Mortality Rates",
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
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel, CatchUnits, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

}




void
ChartBarMortalityRates::getAndLoadPredationMortalityRateDataForecast(
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
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *m_modifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    int seasonVal=0;
    double CatchUnits=NConversion/1000.0;
    std::string title;
    std::string seasonStr = "";
    std::string titleSuffix = "";
    std::string fieldToSum = "AnnBiomass";
    std::string predAgeStr;

    chart->removeAllSeries();

    yLabel = "Predation Mortality (M2)";

    ColumnLabels << "M2 Rate";

    if (SelectedByVariables == "Annual") {
        title     = "Predation Mortality Rate";
        seasonStr = "";
    } else if (SelectedByVariables == "Seasonal") {
        title       = "Predation Mortality Rate";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    fieldToSum = "SeasF";
    predAgeStr = " and Age >= " + std::to_string(SpeAge);

    getDataAndLoadChart("Mortality Rates",
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
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel, CatchUnits, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

}




void
ChartBarMortalityRates::getAndLoadPredationMortalityByPredatorDataForecast(
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
        std::string &SelectedMode,
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
        std::string &yLabelConversion,
        double &NConversion,
        QVBoxLayout *vlayt,
        nmfOutputChart3DBarModifier *m_modifier,
        QWidget *chartView3DContainer,
        int &Theme)
{
    int seasonVal=0;
    double CatchUnits=NConversion/1000.0;
    std::string title;
    std::string seasonStr = "";
    std::string titleSuffix = "";
    std::string fieldToSum = "AnnBiomass";
    std::string predAgeStr;

    chart->removeAllSeries();

    yLabel = "Predation Mortality";

    ColumnLabels << "M2 Rate";

    if (SelectedByVariables == "Annual") {
        title     = "M2 by Predator";
        seasonStr = "";
    } else if (SelectedByVariables == "Seasonal") {
        title       = "M2 by Predator";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    fieldToSum = "SeasF";
    predAgeStr = " and Age = " + std::to_string(SpeAge);

    getDataAndLoadChart("Mortality Rates",
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
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel, CatchUnits, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

}
