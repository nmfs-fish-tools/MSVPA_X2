
#include "ChartBarPopulationSize.h"

#include "nmfConstants.h"

ChartBarPopulationSize::ChartBarPopulationSize(
        QTableWidget* theDataTable,
        nmfLogger*    theLogger)
{
    setObjectName("Population Size");

    DataTable = theDataTable;
    logger    = theLogger;

    // Set up function map so don't have to use series of if...else... statements.
    GetDataFunctionMap["Total Biomass"]          = getAndLoadTotalBiomassData;
    GetDataFunctionMap["Biomass by Age"]         = getAndLoadBiomassByAgeData;
    GetDataFunctionMap["Spawning Stock Biomass"] = getAndLoadSpawningStockBiomassData;
    GetDataFunctionMap["Total Abundance"]        = getAndLoadTotalAbundanceData;
    GetDataFunctionMap["Abundance by Age"]       = getAndLoadAbundanceByAgeData;
    GetDataFunctionMap["Recruit Abundance"]      = getAndLoadRecruitAbundanceData;
    GetDataFunctionMap["Recruit Biomass"]        = getAndLoadRecruitBiomassData;
}


void
ChartBarPopulationSize::callback_UpdateChart(nmfStructsQt::UpdateDataStruct data)
{
    int MSVPA_FirstYear = 0;
    int MSVPA_LastYear = 0;
    int MSVPA_NYears = 0;
    int MSVPA_NSeasons = 0;
    int SeasonInt=0;
    //int offset = 0;
    int SpeAge = 0;
    std::string yLabel="000 Metric Tons";
    boost::numeric::ublas::matrix<double> ChartData;
    boost::numeric::ublas::matrix<double> GridData;
    QStringList ColumnLabels;
    QStringList RowLabels;

    QChart*      chart     = getChart();
    QChartView*  chartView = getChartView();

    nmfDatabase* databasePtr                  = data.databasePtr;
    nmfLogger*   logger                       = data.logger;
    std::string  MSVPAName                    = data.MSVPAName;
    std::string  ForecastName                 = data.ForecastName;
    std::string  ScenarioName                 = data.ScenarioName;
    std::string  ModelName                    = data.ModelName;
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
    int Nage                                  = data.NumAgeSizeClasses;
    std::string MaxScaleY                     = data.MaxScaleY.toStdString();
    int Theme                                 = data.Theme;

    std::vector<bool> GridLines = { data.HorizontalGridLines,
                                    data.VerticalGridLines };
    logger->logMsg(nmfConstants::Normal,
                      "MSVPA ChartBar " +
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
        //offset = (newSelectedPredatorAgeSizeClass.find("Size") != std::string::npos) ? 1 : 0;
        newSelectedPredatorAgeSizeClass.erase(0,newSelectedPredatorAgeSizeClass.find(" "));
        SpeAge = std::stoi(newSelectedPredatorAgeSizeClass);
    }

    std::string FcnIndex = SelectedVariable;

    // Load the chart data using the FunctionMap set up in the constructor.
    if (GetDataFunctionMap.find(FcnIndex) == GetDataFunctionMap.end()) {

        logger->logMsg(nmfConstants::Error,
                       "ChartBarPopulationSize Function: " + FcnIndex + " not found in GetDataFunctionMap.");

    } else {
std::cout << "FcnIndex: " << FcnIndex << std::endl;
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
                SelectedPredator,
                SelectedVariable,
                SelectedByVariables,
                SelectedPredatorAgeSizeClass,
                SelectedSeason,
                SeasonInt,
                SpeAge,
                MaxScaleY,
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
ChartBarPopulationSize::getConversionScaleFactorAndYLabel(
        nmfDatabase* databasePtr,
        std::string &SelectedPredator,
        std::string &SelectedVariable,
        double &NConversion,
        std::string &yLabelConversion)
{
    int i=0;
    double CatchUnits = 0;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> yLabels = {"Hundreds of Fish",
                                        "Thousands of Fish",
                                        "Millions of Fish",
                                        "Billions of Fish"};
    bool isAbundance = isOfTypeAbundance(SelectedVariable);

    // Find NConversion scale factor which is used in some cases
    fields = {"WtUnits","CatchUnits"};
    queryStr = "SELECT WtUnits,CatchUnits FROM " + nmfConstantsMSVPA::TableSpecies +
               " WHERE SpeName='" + SelectedPredator + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["WtUnits"].size() > 0) {
        CatchUnits = std::stod(dataMap["CatchUnits"][0]);
    }

    NConversion = 1.0;
    if (CatchUnits == 0.1) {
        NConversion = 100.0;
        yLabelConversion = yLabels[i];
    } else if (CatchUnits == 1.0) {
        NConversion = 1000.0;
        yLabelConversion = yLabels[++i];
    } else if (CatchUnits == 1000.0) {
        NConversion = 1000000.0;
        yLabelConversion = yLabels[++i];
    } else if (CatchUnits == 1000000.0) {
        NConversion = 1000000000.0;
        yLabelConversion = yLabels[++i];
    }
    yLabelConversion = (isAbundance && (i<3)) ? yLabels[i+1] : yLabels[i];

}

void
ChartBarPopulationSize::getAndLoadTotalBiomassData(
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
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedSeason,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
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
    std::string ageStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    double scaleFactor = 1000.0; // to display on 1000's metric tons

    ColumnLabels << "Biomass";

    nmfUtils::initialize(ChartData,MSVPA_NYears,1);

    if (SelectedByVariables == "Annual") {
        fieldToSum  = "AnnBiomass";
        title       = "Total Biomass (Jan 1.)";
        seasonStr   = " and Season = 0 ";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum  = "Biomass";
        title       = "Total Mid-Season Biomass: ";
        titleSuffix = SelectedSeason;
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    getDataAndLoadChart("Population Size", databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedPredator, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        ageStr, "", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear,
                        SpeAge,
                        chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel,
                        scaleFactor, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData, MSVPA_NYears, 1);
    GridData = ChartData;
}


void
ChartBarPopulationSize::getAndLoadBiomassByAgeData(
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
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedSeason,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
        boost::numeric::ublas::matrix<double> &GridData,
        boost::numeric::ublas::matrix<double> &ChartData,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        QChart *chart,
        std::string &yLabel,
        int &Theme)
{
    int m;
    std::string title;
    std::string seasonStr = "";
    std::string ageStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    double scaleFactor = 1000.0; // to display on 1000's metric tons
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    ColumnLabels << "Biomass";

    if (SelectedByVariables == "Annual") {
        fieldToSum  = "AnnBiomass";
        title       = "Biomass (Jan 1.)";
        ageStr      = " and Age = " + std::to_string(SpeAge);
        seasonStr   = " and Season = 0 ";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum  = "Biomass";
        title       = "Mid-Season Biomass: ";
        titleSuffix = SelectedSeason;
        ageStr      = " and Age = " + std::to_string(SpeAge);
        seasonVal   = SeasonInt;
        seasonStr   = " and Season = " + std::to_string(seasonVal);
    }

    getDataAndLoadChart("Population Size", databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedPredator, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        ageStr, "", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear,
                        SpeAge,
                        chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel,
                        scaleFactor, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    RowLabels.clear();
    ColumnLabels.clear();

    // For age specific data..we want to see the whole matrix and be able to save that out...
    // so have to do a new query and everything
    m = 0;
    ColumnLabels.clear();
    fields = {"Year", "SumField"};
    queryStr = "Select Year, Sum(" + fieldToSum + ") As SumField FROM " +
                nmfConstantsMSVPA::TableMSVPASeasBiomass +
               " WHERE MSVPAname = '" + MSVPAName + "'"
               " AND SpeName = '" + SelectedPredator + "'" +
               seasonStr + " GROUP BY Year, Age";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    nmfUtils::initialize(GridData, MSVPA_NYears, Nage);
    for (int i=0; i<MSVPA_NYears; ++i) {
        RowLabels << QString::number(MSVPA_FirstYear+i);
        for (int j=0; j<Nage; ++j) {
            if (i == 0) {
                ColumnLabels << QString::fromStdString("Age "+std::to_string(j));
            }
            GridData(i, j) = std::stod(dataMap["SumField"][m++]) / 1000.0;
        } // end for j
    } // end for i
}




void
ChartBarPopulationSize::getAndLoadSpawningStockBiomassData(
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
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedSeason,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
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
    std::string ageStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;

    ColumnLabels << "Biomass";

    if (SelectedByVariables == "Annual") {
        fieldToSum = "AnnBiomass";
        //fieldToSum = "Spawning_AnnBiomass";
        title = "Spawning Stock Biomass (Jan 1.)";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum = "Biomass";
        //fieldToSum = "Spawning_Biomass";
        title = "Mid-Season Spawning Stock Biomass: ";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
    } // end Seasonal

    getSpawningStockBiomassDataAndLoadChart(
                "Population Size", databasePtr,
                ModelName,
                ForecastName,
                ScenarioName,
                ChartData, GridData,
                MSVPA_NYears, Nage,
                fieldToSum,
                MSVPAName,
                SelectedPredator,
                ageStr, seasonVal,
                MSVPA_FirstYear, MSVPA_LastYear,
                chart,
                title, titleSuffix, nmfConstants::DontRearrangeTitle,
                yLabel, MaxScaleY,
                RowLabels,
                Theme);
}


void
ChartBarPopulationSize::getAndLoadTotalAbundanceData(
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
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedSeason,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
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
    std::string ageStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    double scaleFactor = 1000.0; // to display on 1000's metric tons

    ColumnLabels << "Abundance";

    getConversionScaleFactorAndYLabel(databasePtr,
                                      SelectedPredator,SelectedVariable,
                                      scaleFactor,yLabel);

    if (SelectedByVariables == "Annual") {
        fieldToSum = "AnnAbund";
        title = "Total Abundance (Jan 1.)";
        seasonStr = " and Season = 0 ";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum = "SeasAbund";
        title = "Total Mid-Season Abundance: ";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    } // end Seasonal

    getDataAndLoadChart("Population Size", databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedPredator, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        ageStr, "", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear,
                        SpeAge,
                        chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel, scaleFactor, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData, MSVPA_NYears, 1);
    GridData = ChartData;

}



void
ChartBarPopulationSize::getAndLoadAbundanceByAgeData(
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
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedSeason,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
        boost::numeric::ublas::matrix<double> &GridData,
        boost::numeric::ublas::matrix<double> &ChartData,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        QChart *chart,
        std::string &yLabel,
        int &Theme)
{
    int m;
    std::string title;
    std::string seasonStr = "";
    std::string ageStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    double scaleFactor = 1000.0; // to display on 1000's metric tons
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    ColumnLabels << "Abundance";
    ageStr = " and Age = " + std::to_string(SpeAge);

    getConversionScaleFactorAndYLabel(databasePtr,
                                      SelectedPredator,SelectedVariable,
                                      scaleFactor,yLabel);

    if (SelectedByVariables == "Annual") {
        fieldToSum = "AnnAbund";
        title = "Abundance (Jan 1.)";
        seasonStr = " and Season = 0 ";
    } else if (SelectedByVariables == "Seasonal") {
        fieldToSum = "SeasAbund";
        title = "Mid-Season Abundance: ";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    } // end Seasonal

    getDataAndLoadChart("Population Size", databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedPredator, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        ageStr, "", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear,
                        SpeAge,
                        chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel, scaleFactor, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    m = 0;
    ColumnLabels.clear();
    fields = {"Year", "SumField"};
    queryStr = "Select Year, Sum(" + fieldToSum + ") As SumField FROM " +
                nmfConstantsMSVPA::TableMSVPASeasBiomass +
               " WHERE MSVPAname = '" + MSVPAName + "'"
               " AND SpeName = '" + SelectedPredator + "'" +
               seasonStr + " GROUP BY Year, Age";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    RowLabels.clear();
    nmfUtils::initialize(GridData, MSVPA_NYears, Nage);
    for (int i=0; i<MSVPA_NYears; ++i) {
        RowLabels << QString::number(MSVPA_FirstYear+i);
        for (int j=0; j<Nage; ++j) {
            if (i == 0) {
                ColumnLabels << QString::fromStdString("Age "+std::to_string(j));
            }
            GridData(i, j) = std::stod(dataMap["SumField"][m++]) / scaleFactor; //1000.0;
        } // end for j
    } // end for i

}


void
ChartBarPopulationSize::getAndLoadRecruitAbundanceData(
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
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedSeason,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
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
    std::string ageStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    double scaleFactor = 1000.0; // to display on 1000's metric tons

    ColumnLabels << "Abundance";
    ageStr = " and Age >= " + std::to_string(SpeAge);

    getConversionScaleFactorAndYLabel(databasePtr,
                                      SelectedPredator,SelectedVariable,
                                      scaleFactor,yLabel);

    if (SelectedByVariables == "Annual") {
        //fieldToSum = "Recruit_AnnAbund";
        fieldToSum = "AnnAbund";
        title = "Abundance (Jan 1.)";
        seasonStr = " and Season = 0 ";
    } else if (SelectedByVariables == "Seasonal") {
        //fieldToSum = "Recruit_SeasAbund";
        fieldToSum = "SeasAbund";
        title = "Mid-Season Abundance: ";
        titleSuffix = SelectedSeason;
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    } // end Seasonal

    getDataAndLoadChart("Population Size", databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedPredator, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        ageStr, "+", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear,
                        SpeAge,
                        chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel, scaleFactor, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData, MSVPA_NYears, 1);
    GridData = ChartData;

}


void
ChartBarPopulationSize::getAndLoadRecruitBiomassData(
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
        std::string &SelectedSpeciesAgeSizeClass,
        std::string &SelectedSeason,
        int &SeasonInt,
        int &SpeAge,
        std::string &MaxScaleY,
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
    std::string ageStr = "";
    int seasonVal=0;
    std::string titleSuffix = "";
    std::string fieldToSum;
    double scaleFactor = 1000.0; // to display on 1000's metric tons

    yLabel="000 Metric Tons";
    ColumnLabels << "Biomass";

    if (SelectedByVariables == "Annual") {
        //fieldToSum = "Recruit_AnnBiomass";
        fieldToSum = "AnnBiomass";
        title = "Biomass (Jan 1.)";
        ageStr = " and Age >= " + std::to_string(SpeAge);
        seasonStr = " and Season = 0 ";
    } else if (SelectedByVariables == "Seasonal") {
        //fieldToSum = "Recruit_Biomass";
        fieldToSum = "Biomass";
        title = "Mid-Season Biomass: ";
        titleSuffix = SelectedSeason;
        ageStr = " and Age >= " + std::to_string(SpeAge);
        seasonVal = SeasonInt;
        seasonStr = " and Season = " + std::to_string(seasonVal);
    }

    getDataAndLoadChart("Population Size", databasePtr,
                        ModelName,
                        ForecastName,
                        ScenarioName,
                        ChartData, GridData,
                        MSVPA_NYears, Nage,
                        fieldToSum,
                        MSVPAName,
                        SelectedPredator, SelectedVariable, SelectedByVariables,
                        SelectedSpeciesAgeSizeClass,
                        ageStr, "+", seasonStr, seasonVal, "",
                        MSVPA_FirstYear, MSVPA_LastYear,
                        SpeAge,
                        chart,
                        title, titleSuffix, nmfConstants::DontRearrangeTitle,
                        yLabel, scaleFactor, 1.0, MaxScaleY,
                        RowLabels, ColumnLabels, Theme);

    nmfUtils::initialize(GridData, MSVPA_NYears, 1);
    GridData = ChartData;

}


