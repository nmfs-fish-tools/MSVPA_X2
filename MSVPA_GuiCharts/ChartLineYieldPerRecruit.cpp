
#include "ChartLineYieldPerRecruit.h"

#include "nmfConstants.h"

ChartLineYieldPerRecruit::ChartLineYieldPerRecruit(
        QTableWidget* theDataTable,
        nmfLogger*    theLogger)
{
    setObjectName("Yield Per Recruit");

    DataTable = theDataTable;
    logger    = theLogger;


    // Set up function map so don't have to use series of if...else... statements.
    GetDataFunctionMap["MSVPA YPR vs. F"]                    = getAndLoadYPRvsFDataMSVPA;
    GetDataFunctionMap["MSVPA SSB vs. F"]                    = getAndLoadSSBvsFDataMSVPA;
    GetDataFunctionMap["MSVPA Historical YPR"]               = getAndLoadHistoricalYPRDataMSVPA;
    GetDataFunctionMap["MSVPA Historical F Benchmarks"]      = getAndLoadHistoricalFBenchmarksDataMSVPA;
    GetDataFunctionMap["MSVPA Historical SSB Benchmarks"]    = getAndLoadHistoricalSSBBenchmarksDataMSVPA;

    GetDataFunctionMap["Forecast YPR vs. F"]                 = getAndLoadYPRvsFDataForecast;
    GetDataFunctionMap["Forecast SSB vs. F"]                 = getAndLoadSSBvsFDataForecast;
    GetDataFunctionMap["Forecast Projected YPR"]             = getAndLoadProjectedYPRDataForecast;
    GetDataFunctionMap["Forecast Projected F Benchmarks"]    = getAndLoadProjectedFBenchmarksDataForecast;
    GetDataFunctionMap["Forecast Projected SSB Benchmarks"]  = getAndLoadProjectedSSBBenchmarksDataForecast;
}


void
ChartLineYieldPerRecruit::callback_UpdateChart(nmfStructsQt::UpdateDataStruct data)
{
    int MSVPA_FirstYear = 0;
    int MSVPA_LastYear = 0;
    int MSVPA_NYears = 0;
    int MSVPA_NSeasons = 0;

    QChart*      chart     = getChart();
    QChartView*  chartView = getChartView();
    nmfDatabase* databasePtr                 = data.databasePtr;
    nmfLogger*   logger                      = data.logger;
    std::string  MSVPAName                   = data.MSVPAName;
    std::string  ModelName                   = data.ModelName;
    std::string  ForecastName                = data.ForecastName;
    std::string  ScenarioName                = data.ScenarioName;
    std::string  SelectedSpecies             = data.SelectPredator.toStdString();
    std::string  TitlePrefix                 = data.TitlePrefix;
    std::string  TitleSuffix                 = data.TitleSuffix;
    std::string  XLabel                      = data.XLabel;
    std::string  YLabel                      = data.YLabel;
    std::string  DataTypeLabel               = data.DataTypeLabel;
    std::string  SelectedYPRAnalysisType     = data.SelectYPRAnalysisType.toStdString();
    std::string  SelectedFullyRecruitedAge   = data.SelectFullyRecruitedAge.toStdString();
    int Nage                                 = data.NumAgeSizeClasses;
    std::string  MaxScaleY                   = data.MaxScaleY.toStdString();
    std::vector<std::string> SelectedYears   = data.SelectedYears;

    boost::numeric::ublas::vector<std::string> LegendNames;
    boost::numeric::ublas::vector<int> Years;
    boost::numeric::ublas::matrix<double> GridData;
    QStringList RowLabels;
    QStringList ColumnLabels;
    int RecAge=0;
    std::string str1,str2;

    if (! SelectedFullyRecruitedAge.empty() ) {
        nmfUtils::split(SelectedFullyRecruitedAge," ",str1,str2);
        RecAge = std::stoi(str2);
    }

    std::vector<bool> GridLines = { data.HorizontalGridLines,
                                    data.VerticalGridLines };

    logger->logMsg(nmfConstants::Normal,
                      ModelName + " ChartLineYieldPerRecruit " +
                      DataTypeLabel + ", " +
                      SelectedSpecies  + ", " +
                      SelectedYPRAnalysisType + ", " +
                      SelectedFullyRecruitedAge);

    // Get some initial data
    databasePtr->nmfGetMSVPAInitData(MSVPAName,
                                     MSVPA_FirstYear,
                                     MSVPA_LastYear,
                                     MSVPA_NYears,
                                     MSVPA_NSeasons);

    // Clear chart and show chartView
    chart->removeAllSeries();
    chartView->show();

    //nmfUtils::split(SelectedSeason," ",str1,str2);
    //seasonStr = " AND Season = " + std::to_string(std::stoi(str2)-1);

    nmfUtils::initialize(Years, MSVPA_NYears);
    nmfUtils::initialize(LegendNames, MSVPA_NYears);
    for (int i=0; i<MSVPA_NYears; ++i) {
        Years(i) = MSVPA_FirstYear + i;
        LegendNames(i) = std::to_string(Years(i));
    }

    std::string FcnIndex = ModelName + " " + SelectedYPRAnalysisType;
    if (SelectedSpecies.empty()) {
        std::cout << "Warning: SelectedSpecies is empty (ChartLineYieldPerRecruit::callback_UpdateChart)" << std::endl;
        return;
    }

    if (GetDataFunctionMap.find(FcnIndex) == GetDataFunctionMap.end()) {
        logger->logMsg(nmfConstants::Error,
                       "ChartLineYieldPerRecruit Function: \"" + FcnIndex + "\" not found in GetDataFunctionMap.");

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
                SelectedSpecies,
                SelectedYears,
                MaxScaleY,
                Nage,
                RecAge,
                GridData,
                RowLabels,
                ColumnLabels);
    }

    // Populate the data table that corresponds with the chart
    populateDataTable(DataTable,GridData,RowLabels,ColumnLabels,7,3);
}

void
ChartLineYieldPerRecruit::getForecastYears(
        nmfDatabase* databasePtr,
        const std::string &MSVPAName,
        const std::string &ForecastName,
        int &Forecast_FirstYear,
        int &Forecast_LastYear,
        int &Forecast_NYears)
{
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    Forecast_FirstYear = 0;
    Forecast_LastYear  = 0;
    Forecast_NYears    = 0;

    // Find number of Forecast years
    fields    = {"InitYear","NYears"};
    queryStr  = "SELECT InitYear,NYears FROM " + nmfConstantsMSVPA::TableForecasts +
                " WHERE MSVPAName = '" + MSVPAName +
                "' AND ForeName = '" + ForecastName + "'";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["NYears"].size() > 0) {
        Forecast_FirstYear = std::stoi(dataMap["InitYear"][0]);
        Forecast_NYears    = std::stoi(dataMap["NYears"][0]) + 1;
        Forecast_LastYear  = Forecast_FirstYear + Forecast_NYears;
    } else {
        //logger->logMsg(nmfConstants::Error,"No NYears data for Forecast: "+ForecastName);
        return;
    }
}


void
ChartLineYieldPerRecruit::getAndLoadYPRvsFDataMSVPA(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int FirstYear,
        int NYears,
        std::string selectedSpecies,
        std::vector<std::string> &SelectedYears,
        std::string MaxScaleY,
        int Nage,
        int RecAge,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList &RowLabels,
        QStringList &ColumnLabels)
{
    bool ok;
    int LoopCount = 0;
    int NumYears = SelectedYears.size();
    char buf[100];
    std::vector<std::string> XLabelNames;
    double YMax=0.0;
    std::string yAxisUnits = getYAxisUnits(databasePtr,selectedSpecies);
    boost::numeric::ublas::matrix<double> Pmature;
    boost::numeric::ublas::matrix<double> FatAge;
    boost::numeric::ublas::matrix<double> WtAtAge;
    boost::numeric::ublas::matrix<double> M1atAge;
    boost::numeric::ublas::matrix<double> M2atAge;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> YPROut;
    boost::numeric::ublas::matrix<double> PRF;
    boost::numeric::ublas::vector<double> FullF;
    boost::numeric::ublas::vector<double> YPRObs;
    boost::numeric::ublas::vector<double> F01Val;
    boost::numeric::ublas::vector<double> FMaxVal;
    boost::numeric::ublas::vector<double> tmpPRF;
    boost::numeric::ublas::vector<double> tmpM2;
    boost::numeric::ublas::vector<double> tmpM1;
    boost::numeric::ublas::vector<double> tmpWt;
    boost::numeric::ublas::vector<double> tmpMat;
    boost::numeric::ublas::vector<std::string> LegendNames;

    nmfUtils::initialize(Pmature, NumYears, Nage);
    nmfUtils::initialize(FatAge,  NumYears, Nage);
    nmfUtils::initialize(WtAtAge, NumYears, Nage);
    nmfUtils::initialize(M1atAge, NumYears, Nage);
    nmfUtils::initialize(M2atAge, NumYears, Nage);
    nmfUtils::initialize(Catch,   NumYears, Nage);
    nmfUtils::initialize(FullF,   NumYears);
    nmfUtils::initialize(PRF,     NumYears, Nage);
    nmfUtils::initialize(YPROut,  26, NumYears);  // RSK replace the hardcoded 26 with a constant
    nmfUtils::initialize(GridData,26, NumYears+1);  // RSK replace the hardcoded 26 with a constant, +1 because stored XX values in 1st column
    nmfUtils::initialize(YPRObs,  NumYears);
    nmfUtils::initialize(F01Val,  NumYears);
    nmfUtils::initialize(FMaxVal, NumYears);
    nmfUtils::initialize(LegendNames, NumYears);

    // OK...then will have to select appropriate data for your species and such
    // Get prop mature and wt at age
    int year;
    for (int i=0; i<NumYears; ++i) {
        year = std::stoi(SelectedYears[i]);
        LegendNames(i) = std::to_string(year);

        // Get prop mature and wt at age
        databasePtr->nmfQueryMaturityData(
            i,year,Nage,selectedSpecies,Pmature);

        // Get Wt At Age...wts at beginning of year
        ok = databasePtr->nmfQueryWeightAtAgeData(
                    i,year,Nage,selectedSpecies,WtAtAge);
        if (!ok) return;

        // Get mortalities at age F, M2, M1
        databasePtr->nmfQueryMortalityData(
                    MSVPAName,i,year-FirstYear,Nage,
                    selectedSpecies,FatAge,M1atAge,M2atAge);

        // Need to get catches to calculate a weighted average FullF
        databasePtr->nmfQueryAgeCatchData(
                    i,year,Nage,selectedSpecies,Catch);

    } // end for i

    // Calculate weighted average F..flat top above RecAge..do we want an option though..probably
    calculateWeightAveFAndAssignPRFs(NumYears,RecAge,Nage,FatAge,Catch,FullF,PRF);
    RowLabels.clear();
    ColumnLabels.clear();

    // Ok..that's all we need to send to the SSB calc.
    nmfUtils::initialize(tmpPRF, Nage);
    nmfUtils::initialize(tmpM2,  Nage);
    nmfUtils::initialize(tmpM1,  Nage);
    nmfUtils::initialize(tmpWt,  Nage);
    nmfUtils::initialize(tmpMat, Nage);
    int k;
    ColumnLabels << "Avg. F";
    //int firstYear = (theModelName == "MSVPA") ? FirstYear : Forecast_FirstYear;
    int firstYear = FirstYear;

    for (int i = 0; i < NumYears; ++ i) {
        ColumnLabels << QString::number(firstYear+i);
        for (int j = 0; j < Nage; ++j) {
            tmpPRF(j) = PRF(i,j);
            tmpM2(j)  = M2atAge(i,j);
            tmpM1(j)  = M1atAge(i,j);
            tmpWt(j)  = WtAtAge(i,j);
            tmpMat(j) = Pmature(i,j);
        }

        YPRObs(i) = YPR(tmpWt, tmpPRF, tmpM2, FullF(i), tmpM1, Nage);
        LoopCount = 0;
        // Generate YPR across a range of Fs..25 values in all
        k = 0;
        for (double XX = 0.0; XX <= 2.6; XX += 0.1) {
            if (i == 0) {
                sprintf(buf,"%0.1f",XX);
                XLabelNames.push_back(buf);
            }
            YPROut(LoopCount,i) = YPR(tmpWt, tmpPRF, tmpM2, XX, tmpM1, Nage);
            if (YPROut(LoopCount,i) > YMax)
                YMax = YPROut(LoopCount,i);
            if (i == 0) {
                RowLabels << "";
                GridData(k,0) = XX;
            }
            GridData(k,i+1) = YPROut(LoopCount,i);

            ++LoopCount;

            ++k;
        } // end XX
        F01Val(i)  = F01(tmpWt, tmpPRF, tmpM2, tmpM1, Nage);
        // Calculate Fmax
        FMaxVal(i) = FMax(tmpWt, tmpPRF, tmpM2, tmpM1, Nage);
    } // end for i

    YMax = (MaxScaleY.empty()) ? YMax : std::stod(MaxScaleY);
    loadChartWithData(chart,
                      YMax,
                      selectedSpecies,
                      YPROut,
                      LegendNames,
                      "Yield Per Recruit vs. Fishing Mortality Rate",
                      "Fully Recruited F", "Yield Per Recruit "+yAxisUnits,
                      XLabelNames,
                      false);

}



void
ChartLineYieldPerRecruit::getAndLoadSSBvsFDataMSVPA(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int FirstYear,
        int NYears,
        std::string selectedSpecies,
        std::vector<std::string> &SelectedYears,
        std::string MaxScaleY,
        int Nage,
        int RecAge,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList &RowLabels,
        QStringList &ColumnLabels)
{
    int LoopCount = 0;
    int k;
    int year;
    int NumYears = SelectedYears.size();
    char buf[100];
    std::vector<std::string> XLabelNames;
    double YMax=0.0;
    std::string yAxisUnits = getYAxisUnits(databasePtr,selectedSpecies);
    boost::numeric::ublas::matrix<double> Pmature;
    boost::numeric::ublas::matrix<double> FatAge;
    boost::numeric::ublas::matrix<double> WtAtAge;
    boost::numeric::ublas::matrix<double> M1atAge;
    boost::numeric::ublas::matrix<double> M2atAge;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> YPROut;
    boost::numeric::ublas::matrix<double> PRF;
    boost::numeric::ublas::vector<double> FullF;
    boost::numeric::ublas::vector<double> YPRObs;
    boost::numeric::ublas::vector<double> tmpPRF;
    boost::numeric::ublas::vector<double> tmpM2;
    boost::numeric::ublas::vector<double> tmpM1;
    boost::numeric::ublas::vector<double> tmpWt;
    boost::numeric::ublas::vector<double> tmpMat;
    boost::numeric::ublas::vector<double> SSB03;
    boost::numeric::ublas::vector<double> SSB10;
    boost::numeric::ublas::vector<double> SSBMax;
    boost::numeric::ublas::vector<std::string> LegendNames;

    nmfUtils::initialize(Pmature, NumYears, Nage);
    nmfUtils::initialize(FatAge,  NumYears, Nage);
    nmfUtils::initialize(WtAtAge, NumYears, Nage);
    nmfUtils::initialize(M1atAge, NumYears, Nage);
    nmfUtils::initialize(M2atAge, NumYears, Nage);
    nmfUtils::initialize(Catch,   NumYears, Nage);
    nmfUtils::initialize(FullF,   NumYears);
    nmfUtils::initialize(PRF,     NumYears, Nage);
    nmfUtils::initialize(YPROut,  26, NumYears);  // RSK replace the hardcoded 25 with a constant
    nmfUtils::initialize(GridData,26, NumYears+1);  // RSK replace the hardcoded 26 with a constant, +1 because stored XX values in 1st column
    nmfUtils::initialize(YPRObs,  NumYears);
    nmfUtils::initialize(SSB03,   NumYears);
    nmfUtils::initialize(SSB10,   NumYears);
    nmfUtils::initialize(SSBMax,  NumYears);
    nmfUtils::initialize(LegendNames, NumYears);
    RowLabels.clear();
    ColumnLabels.clear();
    ColumnLabels << "Avg. F";

    // OK...then will have to select appropriate data for your species and such
    // Get prop mature and wt at age
    for (int i=0; i<NumYears; ++i) {
        year = std::stoi(SelectedYears[i]);
        LegendNames(i) = std::to_string(year);

        // Get prop mature and wt at age
        databasePtr->nmfQueryMaturityData(
            i,year,Nage,selectedSpecies,Pmature);

        // Get Wt At Age...wts at beginning of year
        databasePtr->nmfQueryWeightAtAgeData(
                    i,year,Nage,selectedSpecies,WtAtAge);

        // Get mortalities at age F, M2, M1
        databasePtr->nmfQueryMortalityData(
                    MSVPAName,i,year-FirstYear,Nage,
                    selectedSpecies,FatAge,M1atAge,M2atAge);

        // Need to get catches to calculate a weighted average FullF
        databasePtr->nmfQueryAgeCatchData(
                    i,year,Nage,selectedSpecies,Catch);

    } // end for i

    // Calculate weighted average F..flat top above RecAge..do we want an option though..probably
    calculateWeightAveFAndAssignPRFs(NumYears,RecAge,Nage,FatAge,Catch,FullF,PRF);

    // Ok..that's all we need to send to the SSB calc.
    nmfUtils::initialize(tmpPRF, Nage);
    nmfUtils::initialize(tmpM2,  Nage);
    nmfUtils::initialize(tmpM1,  Nage);
    nmfUtils::initialize(tmpWt,  Nage);
    nmfUtils::initialize(tmpMat, Nage);
    int firstYear = FirstYear; //(theModelName == "MSVPA") ? FirstYear : Forecast_FirstYear;

    for (int i = 0; i < NumYears; ++ i) {
        ColumnLabels << QString::number(firstYear+i);

        for (int j = 0; j < Nage; ++j) {
            tmpPRF(j) = PRF(i,j);
            tmpM2(j)  = M2atAge(i,j);
            tmpM1(j)  = M1atAge(i,j);
            tmpWt(j)  = WtAtAge(i,j);
            tmpMat(j) = Pmature(i,j);
        }
        YPRObs(i) = SSB(tmpWt, tmpPRF, tmpM2, FullF(i), tmpM1, Nage, tmpMat);

        LoopCount = 0;
        // Generate YPR across a range of Fs..25 values in all
        k = 0;
        for (double XX = 0.0; XX <= 2.6; XX += 0.1) {
            if (i == 0) {
                RowLabels << "";
                GridData(k,0) = XX;
                sprintf(buf,"%0.1f",XX);
                XLabelNames.push_back(buf);
            }
            YPROut(LoopCount,i) = SSB(tmpWt, tmpPRF, tmpM2, XX, tmpM1, Nage, tmpMat);
            if (YPROut(LoopCount,i) > YMax)
                YMax = YPROut(LoopCount,i);
            GridData(k,i+1) = YPROut(LoopCount,i);

            ++LoopCount;
            ++k;
        } // end XX

        SSBMax(i) = SSB(tmpWt, tmpPRF, tmpM2, 0, tmpM1, Nage, tmpMat);
        SSB03(i)  = SSBBench(tmpWt, tmpPRF, tmpM2, tmpM1, Nage, tmpMat, 0.03);
        SSB10(i)  = SSBBench(tmpWt, tmpPRF, tmpM2, tmpM1, Nage, tmpMat, 0.1);
    } // end for i

    YMax = (MaxScaleY.empty()) ? YMax : std::stod(MaxScaleY);

    loadChartWithData(chart, YMax,
                      selectedSpecies,
                      YPROut, LegendNames,
                      "SSB Per Recruit vs. Fishing Mortality Rate",
                      "Fully Recruited F", "SSB Per Recruit "+yAxisUnits,
                      XLabelNames,false);

}




void
ChartLineYieldPerRecruit::getAndLoadHistoricalYPRDataMSVPA(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int FirstYear,
        int NYears,
        std::string selectedSpecies,
        std::vector<std::string> &SelectedYears,
        std::string MaxScaleY,
        int Nage,
        int RecAge,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList &RowLabels,
        QStringList &ColumnLabels)
{
    int year;
    int NumYears = NYears;
    std::vector<std::string> XLabelNames;
    double YMax=0.0;
    std::string yAxisUnits = getYAxisUnits(databasePtr,selectedSpecies);
    boost::numeric::ublas::matrix<double> Pmature;
    boost::numeric::ublas::matrix<double> FatAge;
    boost::numeric::ublas::matrix<double> WtAtAge;
    boost::numeric::ublas::matrix<double> M1atAge;
    boost::numeric::ublas::matrix<double> M2atAge;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> YPROut;
    boost::numeric::ublas::matrix<double> PRF;
    boost::numeric::ublas::vector<double> FullF;
    boost::numeric::ublas::vector<double> YPRObs;
    boost::numeric::ublas::vector<double> tmpPRF;
    boost::numeric::ublas::vector<double> tmpM2;
    boost::numeric::ublas::vector<double> tmpM1;
    boost::numeric::ublas::vector<double> tmpWt;
    boost::numeric::ublas::vector<double> tmpMat;
    boost::numeric::ublas::vector<double> FMaxVal;
    boost::numeric::ublas::matrix<double> CombinedYPROut;
    boost::numeric::ublas::vector<std::string> LegendNames;

    nmfUtils::initialize(Pmature, NumYears, Nage);
    nmfUtils::initialize(FatAge,  NumYears, Nage);
    nmfUtils::initialize(WtAtAge, NumYears, Nage);
    nmfUtils::initialize(M1atAge, NumYears, Nage);
    nmfUtils::initialize(M2atAge, NumYears, Nage);
    nmfUtils::initialize(Catch,   NumYears, Nage);
    nmfUtils::initialize(FullF,   NumYears);
    nmfUtils::initialize(PRF,     NumYears, Nage);
    nmfUtils::initialize(YPROut,  NumYears, 1);  // RSK replace the hardcoded 25 with a constant
    nmfUtils::initialize(GridData,NumYears, 2);
    nmfUtils::initialize(YPRObs,  NumYears);
    nmfUtils::initialize(FMaxVal, NumYears);
    nmfUtils::initialize(CombinedYPROut,  NumYears, 2);
    nmfUtils::initialize(LegendNames, 2);
    RowLabels.clear();
    ColumnLabels.clear();
    ColumnLabels << "YPR Obs." << "YPR Max.";

    int firstYear = FirstYear; // (theModelName == "MSVPA") ? FirstYear : Forecast_FirstYear;

    // OK...then will have to select appropriate data for your species and such
    // Get prop mature and wt at age
    for (int i=0; i<NumYears; ++i) {
        year = FirstYear + i; //std::stoi(SelectedYears[i]);

        RowLabels << QString::number(firstYear+i);
        XLabelNames.push_back(std::to_string(year));

        // Get prop mature and wt at age
        databasePtr->nmfQueryMaturityData(
            i,year,Nage,selectedSpecies,Pmature);

        // Get Wt At Age...wts at beginning of year
        databasePtr->nmfQueryWeightAtAgeData(
                    i,year,Nage,selectedSpecies,WtAtAge);

        // Get mortalities at age F, M2, M1
        databasePtr->nmfQueryMortalityData(
                    MSVPAName,i,year-FirstYear,Nage,
                    selectedSpecies,FatAge,M1atAge,M2atAge);

        // Need to get catches to calculate a weighted average FullF
        databasePtr->nmfQueryAgeCatchData(
                    i,year,Nage,selectedSpecies,Catch);

    } // end for i

    // Calculate weighted average F..flat top above RecAge..do we want an option though..probably
    calculateWeightAveFAndAssignPRFs(NumYears,RecAge,Nage,FatAge,Catch,FullF,PRF);

    // Ok..that's all we need to send to the YPR calc.
    nmfUtils::initialize(tmpPRF, Nage);
    nmfUtils::initialize(tmpM2,  Nage);
    nmfUtils::initialize(tmpM1,  Nage);
    nmfUtils::initialize(tmpWt,  Nage);
    nmfUtils::initialize(tmpMat, Nage);

    for (int i = 0; i < NumYears; ++ i) {

        for (int j = 0; j < Nage; ++j) {
            tmpPRF(j) = PRF(i,j);
            tmpM2(j)  = M2atAge(i,j);
            tmpM1(j)  = M1atAge(i,j);
            tmpWt(j)  = WtAtAge(i,j);
            tmpMat(j) = Pmature(i,j);
        }

        YPRObs(i)  = YPR( tmpWt, tmpPRF, tmpM2, FullF(i), tmpM1, Nage);
        FMaxVal(i) = FMax(tmpWt, tmpPRF, tmpM2, tmpM1, Nage);

        // Get YPR at Fmax
        YPROut(i,0) = (FMaxVal(i) > 0) ?
            YPR(tmpWt, tmpPRF, tmpM2, FMaxVal(i), tmpM1, Nage) :
            YPR(tmpWt, tmpPRF, tmpM2, 3.0, tmpM1, Nage);
        CombinedYPROut(i,0) = YPRObs(i);
        CombinedYPROut(i,1) = YPROut(i,0);
        YMax = std::max({YMax,
                         CombinedYPROut(i,0),
                         CombinedYPROut(i,1)});
        GridData(i,0) = YPRObs(i);
        GridData(i,1) = YPROut(i,0);

    } // end for i

    LegendNames(0) = "Observed YPR";
    LegendNames(1) = "Maximum YPR";

    YMax = (MaxScaleY.empty()) ? YMax : std::stod(MaxScaleY);
    loadChartWithData(chart, YMax,
                      selectedSpecies,
                      CombinedYPROut, LegendNames,
                      "Historical Yield Per Recruit",
                      "Year", "Yield Per Recruit "+yAxisUnits,
                      XLabelNames,true);

}



void
ChartLineYieldPerRecruit::getAndLoadHistoricalFBenchmarksDataMSVPA(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int FirstYear,
        int NYears,
        std::string selectedSpecies,
        std::vector<std::string> &SelectedYears,
        std::string MaxScaleY,
        int Nage,
        int RecAge,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList &RowLabels,
        QStringList &ColumnLabels)
{
    int year;
    int NumYears = NYears;
    std::vector<std::string> XLabelNames;
    double YMax=0.0;
    std::string yAxisUnits = getYAxisUnits(databasePtr,selectedSpecies);
    boost::numeric::ublas::matrix<double> Pmature;
    boost::numeric::ublas::matrix<double> FatAge;
    boost::numeric::ublas::matrix<double> WtAtAge;
    boost::numeric::ublas::matrix<double> M1atAge;
    boost::numeric::ublas::matrix<double> M2atAge;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> PRF;
    boost::numeric::ublas::vector<double> FullF;
    boost::numeric::ublas::vector<double> YPRObs;
    boost::numeric::ublas::vector<double> tmpPRF;
    boost::numeric::ublas::vector<double> tmpM2;
    boost::numeric::ublas::vector<double> tmpM1;
    boost::numeric::ublas::vector<double> tmpWt;
    boost::numeric::ublas::vector<double> tmpMat;
    boost::numeric::ublas::vector<double> FMaxVal;
    boost::numeric::ublas::vector<double> SSB03;
    boost::numeric::ublas::vector<double> SSB10;
    boost::numeric::ublas::vector<double> F01Val;
    boost::numeric::ublas::matrix<double> CombinedYPROut;
    boost::numeric::ublas::vector<std::string> LegendNames;

    nmfUtils::initialize(Pmature, NumYears, Nage);
    nmfUtils::initialize(FatAge,  NumYears, Nage);
    nmfUtils::initialize(WtAtAge, NumYears, Nage);
    nmfUtils::initialize(M1atAge, NumYears, Nage);
    nmfUtils::initialize(M2atAge, NumYears, Nage);
    nmfUtils::initialize(Catch,   NumYears, Nage);
    nmfUtils::initialize(FullF,   NumYears);
    nmfUtils::initialize(PRF,     NumYears, Nage);
    nmfUtils::initialize(YPRObs,  NumYears);
    nmfUtils::initialize(GridData,NumYears, 5);
    nmfUtils::initialize(FMaxVal, NumYears);
    nmfUtils::initialize(F01Val,  NumYears);
    nmfUtils::initialize(SSB03,   NumYears);
    nmfUtils::initialize(SSB10,   NumYears);
    nmfUtils::initialize(CombinedYPROut,  NumYears, 5); // RSK replace with constant
    nmfUtils::initialize(LegendNames, 5);
    RowLabels.clear();
    ColumnLabels.clear();
    ColumnLabels << "F Obs." << "F Max." << "F 0.1"
              << "F at 3% Max SSB/R"  << "F at 10% Max SSB/R";
    int firstYear = FirstYear; // (theModelName == "MSVPA") ? FirstYear : Forecast_FirstYear;

    // OK...then will have to select appropriate data for your species and such
    // Get prop mature and wt at age

    for (int i=0; i<NumYears; ++i) {
        year = firstYear + i;

        RowLabels << QString::number(year);

        XLabelNames.push_back(std::to_string(year));

        // Get prop mature and wt at age
        databasePtr->nmfQueryMaturityData(
            i,year,Nage,selectedSpecies,Pmature);

        // Get Wt At Age...wts at beginning of year
        databasePtr->nmfQueryWeightAtAgeData(
                    i,year,Nage,selectedSpecies,WtAtAge);

        // Get mortalities at age F, M2, M1
        databasePtr->nmfQueryMortalityData(
                    MSVPAName,i,year-FirstYear,Nage,
                    selectedSpecies,FatAge,M1atAge,M2atAge);

        // Need to get catches to calculate a weighted average FullF
        databasePtr->nmfQueryAgeCatchData(
                    i,year,Nage,selectedSpecies,Catch);

    } // end for i


    // Calculate weighted average F..flat top above RecAge..do we want an option though..probably
    calculateWeightAveFAndAssignPRFs(NumYears,RecAge,Nage,FatAge,Catch,FullF,PRF);

    // Ok..that's all we need to send to the YPR calc.
    nmfUtils::initialize(tmpPRF, Nage);
    nmfUtils::initialize(tmpM2,  Nage);
    nmfUtils::initialize(tmpM1,  Nage);
    nmfUtils::initialize(tmpWt,  Nage);
    nmfUtils::initialize(tmpMat, Nage);

    for (int i = 0; i < NumYears; ++ i) {

        for (int j = 0; j < Nage; ++j) {
            tmpPRF(j) = PRF(i,j);
            tmpM2(j)  = M2atAge(i,j);
            tmpM1(j)  = M1atAge(i,j);
            tmpWt(j)  = WtAtAge(i,j);
            tmpMat(j) = Pmature(i,j);
        }

        // Calculate F0.1
        F01Val(i) = F01(tmpWt, tmpPRF, tmpM2, tmpM1, Nage);
        // Calculate Fmax
        FMaxVal(i) = FMax(tmpWt, tmpPRF, tmpM2, tmpM1, Nage);
        // Calculate FSSB03
        SSB03(i) = SSBBench(tmpWt, tmpPRF, tmpM2, tmpM1, Nage, tmpMat, 0.03);
        // Calculate FSSB10
        SSB10(i) = SSBBench(tmpWt, tmpPRF, tmpM2, tmpM1, Nage, tmpMat, 0.1);

        CombinedYPROut(i,0) = FullF(i);
        CombinedYPROut(i,1) = (FMaxVal(i) > 0) ? FMaxVal(i) : 2.5;
        CombinedYPROut(i,2) = F01Val(i);
        CombinedYPROut(i,3) = SSB03(i);
        CombinedYPROut(i,4) = SSB10(i);

        YMax = std::max({YMax,
                         CombinedYPROut(i,0),
                         CombinedYPROut(i,1),
                         CombinedYPROut(i,2),
                         CombinedYPROut(i,3),
                         CombinedYPROut(i,4)});

        GridData(i,0) = FullF(i);
        GridData(i,1) = FMaxVal(i);
        GridData(i,2) = F01Val(i);
        GridData(i,3) = SSB03(i);
        GridData(i,4) = SSB10(i);

    } // end for i

    LegendNames(0) = "Observed F";
    LegendNames(1) = "Fmax";
    LegendNames(2) = "F 0.1";
    LegendNames(3) = "F at 3% Max SSB/R";
    LegendNames(4) = "F at 10% Max SSB/R";

    YMax = (MaxScaleY.empty()) ? YMax : std::stod(MaxScaleY);

    loadChartWithData(chart, YMax,
                      selectedSpecies,
                      CombinedYPROut, LegendNames,
                      "HistoricaL F Benchmarks",
                      "Year", "Fishing Mortality Rate "+yAxisUnits,
                      XLabelNames,true);

}

void
ChartLineYieldPerRecruit::getAndLoadHistoricalSSBBenchmarksDataMSVPA(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int FirstYear,
        int NYears,
        std::string selectedSpecies,
        std::vector<std::string> &SelectedYears,
        std::string MaxScaleY,
        int Nage,
        int RecAge,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList &RowLabels,
        QStringList &ColumnLabels)
{
    int year;
    int NumYears = NYears;
    std::vector<std::string> XLabelNames;
    double YMax=0.0;
    std::string yAxisUnits = getYAxisUnits(databasePtr,selectedSpecies);
    boost::numeric::ublas::matrix<double> Pmature;
    boost::numeric::ublas::matrix<double> FatAge;
    boost::numeric::ublas::matrix<double> WtAtAge;
    boost::numeric::ublas::matrix<double> M1atAge;
    boost::numeric::ublas::matrix<double> M2atAge;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> YPROut;
    boost::numeric::ublas::matrix<double> PRF;
    boost::numeric::ublas::vector<double> FullF;
    boost::numeric::ublas::vector<double> YPRObs;
    boost::numeric::ublas::vector<double> tmpPRF;
    boost::numeric::ublas::vector<double> tmpM2;
    boost::numeric::ublas::vector<double> tmpM1;
    boost::numeric::ublas::vector<double> tmpWt;
    boost::numeric::ublas::vector<double> tmpMat;
    boost::numeric::ublas::vector<double> SSBMax;
    boost::numeric::ublas::matrix<double> CombinedYPROut;
    boost::numeric::ublas::vector<std::string> LegendNames;

    nmfUtils::initialize(Pmature, NumYears, Nage);
    nmfUtils::initialize(FatAge,  NumYears, Nage);
    nmfUtils::initialize(WtAtAge, NumYears, Nage);
    nmfUtils::initialize(M1atAge, NumYears, Nage);
    nmfUtils::initialize(M2atAge, NumYears, Nage);
    nmfUtils::initialize(Catch,   NumYears, Nage);
    nmfUtils::initialize(FullF,   NumYears);
    nmfUtils::initialize(PRF,     NumYears, Nage);
    nmfUtils::initialize(YPROut,  26, NumYears);  // RSK replace the hardcoded 25 with a constant
    nmfUtils::initialize(GridData,NumYears, 4);
    nmfUtils::initialize(YPRObs,  NumYears);
    nmfUtils::initialize(SSBMax,  NumYears);
    nmfUtils::initialize(CombinedYPROut,  NumYears, 3); // RSK replace with constant
    nmfUtils::initialize(LegendNames, 3);
    RowLabels.clear();
    ColumnLabels.clear();
    ColumnLabels << "Obs. SSB/R" << "SSB/R Max." << "3% SSB/R Max."
              << "10% SSB/R Max.";
    int firstYear = FirstYear; // (theModelName == "MSVPA") ? FirstYear : Forecast_FirstYear;

    // OK...then will have to select appropriate data for your species and such
    // Get prop mature and wt at age

    for (int i=0; i<NumYears; ++i) {
        year = firstYear + i;
        RowLabels << QString::number(year);

        XLabelNames.push_back(std::to_string(year));

        // Get prop mature and wt at age
        databasePtr->nmfQueryMaturityData(
            i,year,Nage,selectedSpecies,Pmature);

        // Get Wt At Age...wts at beginning of year
        databasePtr->nmfQueryWeightAtAgeData(
                    i,year,Nage,selectedSpecies,WtAtAge);

        // Get mortalities at age F, M2, M1
        databasePtr->nmfQueryMortalityData(
                    MSVPAName,i,year-FirstYear,Nage,
                    selectedSpecies,FatAge,M1atAge,M2atAge);

        // Need to get catches to calculate a weighted average FullF
        databasePtr->nmfQueryAgeCatchData(
                    i,year,Nage,selectedSpecies,Catch);

    } // end for i


    // Calculate weighted average F..flat top above RecAge..do we want an option though..probably
    calculateWeightAveFAndAssignPRFs(NumYears,RecAge,Nage,FatAge,Catch,FullF,PRF);

    // Ok..that's all we need to send to the YPR calc.
    nmfUtils::initialize(tmpPRF, Nage);
    nmfUtils::initialize(tmpM2,  Nage);
    nmfUtils::initialize(tmpM1,  Nage);
    nmfUtils::initialize(tmpWt,  Nage);
    nmfUtils::initialize(tmpMat, Nage);

    for (int i = 0; i < NumYears; ++ i) {

        for (int j = 0; j < Nage; ++j) {
            tmpPRF(j) = PRF(i,j);
            tmpM2(j)  = M2atAge(i,j);
            tmpM1(j)  = M1atAge(i,j);
            tmpWt(j)  = WtAtAge(i,j);
            tmpMat(j) = Pmature(i,j);
        }

        // Calculate Observed SSB
        YPRObs(i) = SSB(tmpWt, tmpPRF, tmpM2, FullF(i), tmpM1, Nage, tmpMat);
        // Calculate SSBMax
        SSBMax(i) = SSB(tmpWt, tmpPRF, tmpM2, 0, tmpM1, Nage, tmpMat);

        CombinedYPROut(i,0) = YPRObs(i);
        CombinedYPROut(i,1) = SSBMax(i) * 0.03;
        CombinedYPROut(i,2) = SSBMax(i) * 0.1;

        YMax = std::max({YMax,
                         CombinedYPROut(i,0),
                         CombinedYPROut(i,1),
                         CombinedYPROut(i,2)});

        GridData(i,0) = YPRObs(i);
        GridData(i,1) = SSBMax(i);
        GridData(i,2) = 0.03 * SSBMax(i);
        GridData(i,3) = 0.10 * SSBMax(i);

    } // end for i

    LegendNames(0) = "Observed SSB/R";
    LegendNames(1) = "3% of Max SSB/R";
    LegendNames(2) = "10% of Max SSB/R";

    YMax = (MaxScaleY.empty()) ? YMax : std::stod(MaxScaleY);

    loadChartWithData(chart, YMax,
                      selectedSpecies,
                      CombinedYPROut, LegendNames,
                      "Historical SSB/R Benchmarks",
                      "Year", "Fishing Mortality Rate "+yAxisUnits,
                      XLabelNames,true);

}






void
ChartLineYieldPerRecruit::getAndLoadYPRvsFDataForecast(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int FirstYear,
        int NYears,
        std::string selectedSpecies,
        std::vector<std::string> &SelectedYears,
        std::string MaxScaleY,
        int Nage,
        int RecAge,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList &RowLabels,
        QStringList &ColumnLabels)
{
    int NumYears = SelectedYears.size();
    int LoopCount = 0;
    int year;
    int Forecast_FirstYear;
    int Forecast_NYears;
    int Forecast_LastYear;
    double yprOut;
    double YMax=0.0;
    char buf[1000];
    std::string yAxisUnits = getYAxisUnits(databasePtr,selectedSpecies);
    std::vector<std::string> XLabelNames;
    boost::numeric::ublas::vector<std::string> LegendNames;
    boost::numeric::ublas::matrix<double> Pmature;
    boost::numeric::ublas::matrix<double> FatAge;
    boost::numeric::ublas::matrix<double> WtAtAge;
    boost::numeric::ublas::matrix<double> M1atAge;
    boost::numeric::ublas::matrix<double> M2atAge;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> YPROut;
    boost::numeric::ublas::matrix<double> PRF;
    boost::numeric::ublas::vector<double> FullF;
    boost::numeric::ublas::vector<double> YPRObs;
    boost::numeric::ublas::vector<double> F01Val;
    boost::numeric::ublas::vector<double> FMaxVal;
    boost::numeric::ublas::vector<double> tmpPRF;
    boost::numeric::ublas::vector<double> tmpM2;
    boost::numeric::ublas::vector<double> tmpM1;
    boost::numeric::ublas::vector<double> tmpWt;
    boost::numeric::ublas::vector<double> tmpMat;

    nmfUtils::initialize(Pmature, NumYears, Nage);
    nmfUtils::initialize(FatAge,  NumYears, Nage);
    nmfUtils::initialize(WtAtAge, NumYears, Nage);
    nmfUtils::initialize(M1atAge, NumYears, Nage);
    nmfUtils::initialize(M2atAge, NumYears, Nage);
    nmfUtils::initialize(Catch,   NumYears, Nage);
    nmfUtils::initialize(FullF,   NumYears);
    nmfUtils::initialize(PRF,     NumYears, Nage);
    nmfUtils::initialize(YPROut,  26, NumYears);    // RSK replace the hardcoded 26 with a constant
    nmfUtils::initialize(GridData,26, NumYears+1);  // RSK replace the hardcoded 26 with a constant
    nmfUtils::initialize(YPRObs,  NumYears);
    nmfUtils::initialize(F01Val,  NumYears);
    nmfUtils::initialize(FMaxVal, NumYears);
    nmfUtils::initialize(LegendNames, NumYears);

    getForecastYears(databasePtr,MSVPAName,ForecastName,
                     Forecast_FirstYear,Forecast_LastYear,Forecast_NYears);

    // OK...then will have to select appropriate data for your species and such
    // Get prop mature and wt at age
    for (int i=0; i<NumYears; ++i) {
        year = std::stoi(SelectedYears[i]);
        LegendNames(i) = SelectedYears[i];

        // Get prop mature and wt at age
        databasePtr->nmfQueryMaturityData(
            i,FirstYear,Nage,selectedSpecies,Pmature);

        // Get Wt At Age...wts at beginning of year
        databasePtr->nmfQueryForecastWeightAtAgeData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,WtAtAge);

        // Get mortalities at age F, M2, M1
        databasePtr->nmfQueryForecastMortalityData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,FatAge,M1atAge,M2atAge);

        // Need to get catches to calculate a weighted average FullF
        databasePtr->nmfQueryForecastAgeCatchData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,Catch);

    } // end for i

    // Calculate weighted average F..flat top above RecAge..do we want an option though..probably
    calculateWeightAveFAndAssignPRFs(NumYears,RecAge,Nage,FatAge,Catch,FullF,PRF);

    RowLabels.clear();
    ColumnLabels.clear();

    // Ok..that's all we need to send to the SSB calc.
    nmfUtils::initialize(tmpPRF, Nage);
    nmfUtils::initialize(tmpM2,  Nage);
    nmfUtils::initialize(tmpM1,  Nage);
    nmfUtils::initialize(tmpWt,  Nage);
    nmfUtils::initialize(tmpMat, Nage);

    ColumnLabels << "Avg F";

    for (int i = 0; i < NumYears; ++ i) {

        ColumnLabels << QString::number(Forecast_FirstYear+i);

        for (int j = 0; j < Nage; ++j) {
            tmpPRF(j) = PRF(i,j);
            tmpM2(j)  = M2atAge(i,j);
            tmpM1(j)  = M1atAge(i,j);
            tmpWt(j)  = WtAtAge(i,j);
            tmpMat(j) = Pmature(i,j);
        }

        YPRObs(i) = YPR(tmpWt, tmpPRF, tmpM2, FullF(i), tmpM1, Nage);

        // Generate YPR across a range of Fs..25 values in all
        LoopCount = 0;
        for (double XX = 0.0; XX <= 2.6; XX += 0.1) {
            if (i == 0) {
                RowLabels << QString::number(XX);
                GridData(LoopCount,0) = XX;
            }
            sprintf(buf,"%0.1f",XX);
            XLabelNames.push_back(buf);
            yprOut = YPR(tmpWt, tmpPRF, tmpM2, XX, tmpM1, Nage);   // RSK - tbd - error here, -nan with Menhaden
            YPROut(LoopCount,i) = yprOut;
            if (yprOut > YMax)
                YMax = yprOut;
            GridData(LoopCount,i+1) = yprOut;
            ++LoopCount;
        } // end XX
        F01Val(i) = F01(tmpWt, tmpPRF, tmpM2, tmpM1, Nage);
        // Calculate Fmax
        FMaxVal(i) = FMax(tmpWt, tmpPRF, tmpM2, tmpM1, Nage);
    } // end for i

    YMax = (MaxScaleY.empty()) ? YMax : std::stod(MaxScaleY);
    loadChartWithData(chart, YMax,
                      selectedSpecies,
                      YPROut, LegendNames,
                      "Yield Per Recruit vs. Fishing Mortality Rate",
                      "Fully Recruited F", "Yield Per Recruit "+yAxisUnits,
                      XLabelNames,
                      false);

}



void
ChartLineYieldPerRecruit::getAndLoadSSBvsFDataForecast(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int FirstYear,
        int NYears,
        std::string selectedSpecies,
        std::vector<std::string> &SelectedYears,
        std::string MaxScaleY,
        int Nage,
        int RecAge,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList &RowLabels,
        QStringList &ColumnLabels)
{
    int LoopCount = 0;
    int year;
    int NumYears = SelectedYears.size();
    int Forecast_FirstYear;
    int Forecast_LastYear;
    int Forecast_NYears;
    char buf[100];
    std::vector<std::string> XLabelNames;
    double YMax=0.0;
    double yprOut;
    std::string yAxisUnits = getYAxisUnits(databasePtr,selectedSpecies);
    boost::numeric::ublas::matrix<double> Pmature;
    boost::numeric::ublas::matrix<double> FatAge;
    boost::numeric::ublas::matrix<double> WtAtAge;
    boost::numeric::ublas::matrix<double> M1atAge;
    boost::numeric::ublas::matrix<double> M2atAge;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> YPROut;
    boost::numeric::ublas::matrix<double> PRF;
    boost::numeric::ublas::vector<double> FullF;
    boost::numeric::ublas::vector<double> YPRObs;
    boost::numeric::ublas::vector<double> tmpPRF;
    boost::numeric::ublas::vector<double> tmpM2;
    boost::numeric::ublas::vector<double> tmpM1;
    boost::numeric::ublas::vector<double> tmpWt;
    boost::numeric::ublas::vector<double> tmpMat;
    boost::numeric::ublas::vector<double> SSB03;
    boost::numeric::ublas::vector<double> SSB10;
    boost::numeric::ublas::vector<double> SSBMax;
    boost::numeric::ublas::vector<std::string> LegendNames;

    nmfUtils::initialize(Pmature, NumYears, Nage);
    nmfUtils::initialize(FatAge,  NumYears, Nage);
    nmfUtils::initialize(WtAtAge, NumYears, Nage);
    nmfUtils::initialize(M1atAge, NumYears, Nage);
    nmfUtils::initialize(M2atAge, NumYears, Nage);
    nmfUtils::initialize(Catch,   NumYears, Nage);
    nmfUtils::initialize(FullF,   NumYears);
    nmfUtils::initialize(PRF,     NumYears, Nage);
    nmfUtils::initialize(YPROut,  26, NumYears);  // RSK replace the hardcoded 25 with a constant
    nmfUtils::initialize(GridData,26, NumYears+1);  // RSK replace the hardcoded 26 with a constant, +1 because stored XX values in 1st column
    nmfUtils::initialize(YPRObs,  NumYears);
    nmfUtils::initialize(SSB03,   NumYears);
    nmfUtils::initialize(SSB10,   NumYears);
    nmfUtils::initialize(SSBMax,  NumYears);
    nmfUtils::initialize(LegendNames, NumYears);

    RowLabels.clear();
    ColumnLabels.clear();
    ColumnLabels << "Avg. F";

    getForecastYears(databasePtr,MSVPAName,ForecastName,
                     Forecast_FirstYear,Forecast_LastYear,Forecast_NYears);

    // OK...then will have to select appropriate data for your species and such
    // Get prop mature and wt at age
    for (int i=0; i<NumYears; ++i) {
        year = std::stoi(SelectedYears[i]);
        LegendNames(i) = std::to_string(year);

        ColumnLabels << QString::number(Forecast_FirstYear+i);

        // Get prop mature and wt at age
        databasePtr->nmfQueryMaturityData(
                    i,FirstYear,Nage,selectedSpecies,Pmature);

        // Get Wt At Age...wts at beginning of year
        databasePtr->nmfQueryForecastWeightAtAgeData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,WtAtAge);

        // Get mortalities at age F, M2, M1
        databasePtr->nmfQueryForecastMortalityData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,FatAge,M1atAge,M2atAge);

        // Need to get catches to calculate a weighted average FullF
        databasePtr->nmfQueryForecastAgeCatchData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,Catch);

    } // end for i

    // Calculate weighted average F..flat top above RecAge..do we want an option though..probably
    calculateWeightAveFAndAssignPRFs(NumYears,RecAge,Nage,FatAge,Catch,FullF,PRF);

    // Ok..that's all we need to send to the SSB calc.
    nmfUtils::initialize(tmpPRF, Nage);
    nmfUtils::initialize(tmpM2,  Nage);
    nmfUtils::initialize(tmpM1,  Nage);
    nmfUtils::initialize(tmpWt,  Nage);
    nmfUtils::initialize(tmpMat, Nage);

    for (int i = 0; i < NumYears; ++ i) {

        for (int j = 0; j < Nage; ++j) {
            tmpPRF(j) = PRF(i,j);
            tmpM2(j)  = M2atAge(i,j);
            tmpM1(j)  = M1atAge(i,j);
            tmpWt(j)  = WtAtAge(i,j);
            tmpMat(j) = Pmature(i,j);
        }
        YPRObs(i) = SSB(tmpWt, tmpPRF, tmpM2, FullF(i), tmpM1, Nage, tmpMat);

        LoopCount = 0;
        // Generate YPR across a range of Fs..25 values in all
        for (double XX = 0.0; XX <= 2.6; XX += 0.1) {
            if (i == 0) {
                RowLabels << QString::number(XX);
                GridData(LoopCount,0) = XX;
            }
            sprintf(buf,"%0.1f",XX);
            XLabelNames.push_back(buf);
            yprOut = SSB(tmpWt, tmpPRF, tmpM2, XX, tmpM1, Nage, tmpMat);
            YPROut(LoopCount,i) = yprOut;
            if (yprOut > YMax)
                YMax = yprOut;
            GridData(LoopCount,i+1) = yprOut;
            ++LoopCount;
        } // end XX

        SSBMax(i) = SSB(tmpWt, tmpPRF, tmpM2, 0, tmpM1, Nage, tmpMat);
        SSB03(i)  = SSBBench(tmpWt, tmpPRF, tmpM2, tmpM1, Nage, tmpMat, 0.03);
        SSB10(i)  = SSBBench(tmpWt, tmpPRF, tmpM2, tmpM1, Nage, tmpMat, 0.1);
    } // end for i

    YMax = (MaxScaleY.empty()) ? YMax : std::stod(MaxScaleY);
    loadChartWithData(chart, YMax,
                      selectedSpecies,
                      YPROut, LegendNames,
                      "SSB Per Recruit vs. Fishing Mortality Rate",
                      "Fully Recruited F", "SSB Per Recruit "+yAxisUnits,
                      XLabelNames,false);

}


void
ChartLineYieldPerRecruit::getAndLoadProjectedYPRDataForecast(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int FirstYear,
        int NYears,
        std::string selectedSpecies,
        std::vector<std::string> &SelectedYears,
        std::string MaxScaleY,
        int Nage,
        int RecAge,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList &RowLabels,
        QStringList &ColumnLabels)
{
    int year;
    int Forecast_FirstYear;
    int Forecast_LastYear;
    int Forecast_NYears;
    int NumYears = SelectedYears.size();
    std::vector<std::string> XLabelNames;
    double YMax=0.0;

    std::string yAxisUnits = getYAxisUnits(databasePtr,selectedSpecies);
    boost::numeric::ublas::matrix<double> Pmature;
    boost::numeric::ublas::matrix<double> FatAge;
    boost::numeric::ublas::matrix<double> WtAtAge;
    boost::numeric::ublas::matrix<double> M1atAge;
    boost::numeric::ublas::matrix<double> M2atAge;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> YPROut;
    boost::numeric::ublas::matrix<double> PRF;
    boost::numeric::ublas::vector<double> FullF;
    boost::numeric::ublas::vector<double> YPRObs;
    boost::numeric::ublas::vector<double> tmpPRF;
    boost::numeric::ublas::vector<double> tmpM2;
    boost::numeric::ublas::vector<double> tmpM1;
    boost::numeric::ublas::vector<double> tmpWt;
    boost::numeric::ublas::vector<double> tmpMat;
    boost::numeric::ublas::vector<double> FMaxVal;
    boost::numeric::ublas::matrix<double> CombinedYPROut;
    boost::numeric::ublas::vector<std::string> LegendNames;

    nmfUtils::initialize(Pmature, NumYears, Nage);
    nmfUtils::initialize(FatAge,  NumYears, Nage);
    nmfUtils::initialize(WtAtAge, NumYears, Nage);
    nmfUtils::initialize(M1atAge, NumYears, Nage);
    nmfUtils::initialize(M2atAge, NumYears, Nage);
    nmfUtils::initialize(Catch,   NumYears, Nage);
    nmfUtils::initialize(FullF,   NumYears);
    nmfUtils::initialize(PRF,     NumYears, Nage);
    nmfUtils::initialize(YPROut,  26, NumYears);  // RSK replace the hardcoded 25 with a constant
    nmfUtils::initialize(YPRObs,  NumYears);
    nmfUtils::initialize(FMaxVal, NumYears);
    nmfUtils::initialize(CombinedYPROut,  NumYears, 2);
    nmfUtils::initialize(GridData,        NumYears, 2);
    nmfUtils::initialize(LegendNames, 2);

    RowLabels.clear();
    ColumnLabels.clear();

    getForecastYears(databasePtr,MSVPAName,ForecastName,
                     Forecast_FirstYear,Forecast_LastYear,Forecast_NYears);

    // OK...then will have to select appropriate data for your species and such
    // Get prop mature and wt at age
    for (int i=0; i<NumYears; ++i) {

        year = std::stoi(SelectedYears[i]);
        XLabelNames.push_back(std::to_string(year));

        // Get prop mature and wt at age
        databasePtr->nmfQueryMaturityData(
                    i,FirstYear,Nage,selectedSpecies,Pmature);

        // Get Wt At Age...wts at beginning of year
        databasePtr->nmfQueryForecastWeightAtAgeData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,WtAtAge);

        // Get mortalities at age F, M2, M1
        databasePtr->nmfQueryForecastMortalityData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,FatAge,M1atAge,M2atAge);

        // Need to get catches to calculate a weighted average FullF
        databasePtr->nmfQueryForecastAgeCatchData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,Catch);

    } // end for i

    // Calculate weighted average F..flat top above RecAge..do we want an option though..probably
    calculateWeightAveFAndAssignPRFs(NumYears,RecAge,Nage,FatAge,Catch,FullF,PRF);

    // Ok..that's all we need to send to the YPR calc.
    nmfUtils::initialize(tmpPRF, Nage);
    nmfUtils::initialize(tmpM2,  Nage);
    nmfUtils::initialize(tmpM1,  Nage);
    nmfUtils::initialize(tmpWt,  Nage);
    nmfUtils::initialize(tmpMat, Nage);

    ColumnLabels << "YPR Obs." << "YPR Max.";
    for (int i = 0; i < NumYears; ++ i) {
        RowLabels << QString::number(Forecast_FirstYear+i);
        for (int j = 0; j < Nage; ++j) {
            tmpPRF(j) = PRF(i,j);
            tmpM2(j)  = M2atAge(i,j);
            tmpM1(j)  = M1atAge(i,j);
            tmpWt(j)  = WtAtAge(i,j);
            tmpMat(j) = Pmature(i,j);
        }
        YPRObs(i)  = YPR( tmpWt, tmpPRF, tmpM2, FullF(i), tmpM1, Nage);;
        FMaxVal(i) = FMax(tmpWt, tmpPRF, tmpM2, tmpM1, Nage);

        // Get YPR at Fmax
        YPROut(i,0) = (FMaxVal(i) > 0) ?
            YPR(tmpWt, tmpPRF, tmpM2, FMaxVal(i), tmpM1, Nage) :
            YPR(tmpWt, tmpPRF, tmpM2, 3.0, tmpM1, Nage);

        CombinedYPROut(i,0) = YPRObs(i);
        CombinedYPROut(i,1) = YPROut(i,0);

        YMax = std::max({YMax,
                         CombinedYPROut(i,0),
                         CombinedYPROut(i,1)});
    } // end for i

    LegendNames(0) = "Observed YPR";
    LegendNames(1) = "Maximum YPR";

    YMax = (MaxScaleY.empty()) ? YMax : std::stod(MaxScaleY);
    loadChartWithData(chart, YMax,
                      selectedSpecies,
                      CombinedYPROut, LegendNames,
                      "Projected Yield Per Recruit",
                      "Year", "Yield Per Recruit "+yAxisUnits,
                      XLabelNames,true);

    GridData = CombinedYPROut;
}



void
ChartLineYieldPerRecruit::getAndLoadProjectedFBenchmarksDataForecast(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int FirstYear,
        int NYears,
        std::string selectedSpecies,
        std::vector<std::string> &SelectedYears,
        std::string MaxScaleY,
        int Nage,
        int RecAge,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList &RowLabels,
        QStringList &ColumnLabels)
{
    int year;
    int NumYears = SelectedYears.size();
    int Forecast_FirstYear;
    int Forecast_LastYear;
    int Forecast_NYears;
    std::vector<std::string> XLabelNames;
    double YMax=0.0;
    std::string yAxisUnits = getYAxisUnits(databasePtr,selectedSpecies);
    boost::numeric::ublas::matrix<double> Pmature;
    boost::numeric::ublas::matrix<double> FatAge;
    boost::numeric::ublas::matrix<double> WtAtAge;
    boost::numeric::ublas::matrix<double> M1atAge;
    boost::numeric::ublas::matrix<double> M2atAge;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> PRF;
    boost::numeric::ublas::vector<double> FullF;
    boost::numeric::ublas::vector<double> YPRObs;
    boost::numeric::ublas::vector<double> tmpPRF;
    boost::numeric::ublas::vector<double> tmpM2;
    boost::numeric::ublas::vector<double> tmpM1;
    boost::numeric::ublas::vector<double> tmpWt;
    boost::numeric::ublas::vector<double> tmpMat;
    boost::numeric::ublas::vector<double> FMaxVal;
    boost::numeric::ublas::vector<double> SSB03;
    boost::numeric::ublas::vector<double> SSB10;
    boost::numeric::ublas::vector<double> F01Val;
    boost::numeric::ublas::matrix<double> CombinedYPROut;
    boost::numeric::ublas::vector<std::string> LegendNames;

    nmfUtils::initialize(Pmature, NumYears, Nage);
    nmfUtils::initialize(FatAge,  NumYears, Nage);
    nmfUtils::initialize(WtAtAge, NumYears, Nage);
    nmfUtils::initialize(M1atAge, NumYears, Nage);
    nmfUtils::initialize(M2atAge, NumYears, Nage);
    nmfUtils::initialize(Catch,   NumYears, Nage);
    nmfUtils::initialize(FullF,   NumYears);
    nmfUtils::initialize(PRF,     NumYears, Nage);
    nmfUtils::initialize(YPRObs,  NumYears);
    nmfUtils::initialize(GridData,NumYears, 5);
    nmfUtils::initialize(FMaxVal, NumYears);
    nmfUtils::initialize(F01Val,  NumYears);
    nmfUtils::initialize(SSB03,   NumYears);
    nmfUtils::initialize(SSB10,   NumYears);
    nmfUtils::initialize(CombinedYPROut,  NumYears, 5); // RSK replace with constant
    nmfUtils::initialize(LegendNames, 5);

    RowLabels.clear();
    ColumnLabels.clear();

    getForecastYears(databasePtr,MSVPAName,ForecastName,
                     Forecast_FirstYear,Forecast_LastYear,Forecast_NYears);

    // OK...then will have to select appropriate data for your species and such
    // Get prop mature and wt at age

    for (int i=0; i<NumYears; ++i) {
        year = std::stoi(SelectedYears[i]);

        XLabelNames.push_back(std::to_string(year));

        // Get prop mature and wt at age
        databasePtr->nmfQueryMaturityData(
                    i,FirstYear,Nage,selectedSpecies,Pmature);

        // Get Wt At Age...wts at beginning of year
        databasePtr->nmfQueryForecastWeightAtAgeData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,WtAtAge);

        // Get mortalities at age F, M2, M1
        databasePtr->nmfQueryForecastMortalityData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,FatAge,M1atAge,M2atAge);

        // Need to get catches to calculate a weighted average FullF
        databasePtr->nmfQueryForecastAgeCatchData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,Catch);

    } // end for i


    // Calculate weighted average F..flat top above RecAge..do we want an option though..probably
    calculateWeightAveFAndAssignPRFs(NumYears,RecAge,Nage,FatAge,Catch,FullF,PRF);

    // Ok..that's all we need to send to the YPR calc.
    nmfUtils::initialize(tmpPRF, Nage);
    nmfUtils::initialize(tmpM2,  Nage);
    nmfUtils::initialize(tmpM1,  Nage);
    nmfUtils::initialize(tmpWt,  Nage);
    nmfUtils::initialize(tmpMat, Nage);
    ColumnLabels <<  "F Obs." << "F Max." << "F 0.1"
                  << "F at 3% Max SSB/R"  << "F at 10% Max SSB/R ";

    for (int i = 0; i < NumYears; ++ i) {
        RowLabels << QString::number(Forecast_FirstYear+i);
        for (int j = 0; j < Nage; ++j) {
            tmpPRF(j) = PRF(i,j);
            tmpM2(j)  = M2atAge(i,j);
            tmpM1(j)  = M1atAge(i,j);
            tmpWt(j)  = WtAtAge(i,j);
            tmpMat(j) = Pmature(i,j);
        }

        // Calculate F0.1
        F01Val(i) = F01(tmpWt, tmpPRF, tmpM2, tmpM1, Nage);
        // Calculate Fmax
        FMaxVal(i) = FMax(tmpWt, tmpPRF, tmpM2, tmpM1, Nage);
        // Calculate FSSB03
        SSB03(i) = SSBBench(tmpWt, tmpPRF, tmpM2, tmpM1, Nage, tmpMat, 0.03);
        // Calculate FSSB10
        SSB10(i) = SSBBench(tmpWt, tmpPRF, tmpM2, tmpM1, Nage, tmpMat, 0.1);

        CombinedYPROut(i,0) = FullF(i);
        CombinedYPROut(i,1) = (FMaxVal(i) > 0) ? FMaxVal(i) : 2.5;
        CombinedYPROut(i,2) = F01Val(i);
        CombinedYPROut(i,3) = SSB03(i);
        CombinedYPROut(i,4) = SSB10(i);

        YMax = std::max({YMax,
                         CombinedYPROut(i,0),
                         CombinedYPROut(i,1),
                         CombinedYPROut(i,2),
                         CombinedYPROut(i,3),
                         CombinedYPROut(i,4)});

    } // end for i

    LegendNames(0) = "Observed F";
    LegendNames(1) = "Fmax";
    LegendNames(2) = "F 0.1";
    LegendNames(3) = "F at 3% Max SSB/R";
    LegendNames(4) = "F at 10% Max SSB/R";

    YMax = (MaxScaleY.empty()) ? YMax : std::stod(MaxScaleY);
    loadChartWithData(chart, YMax,
                      selectedSpecies,
                      CombinedYPROut, LegendNames,
                      "Projected F Benchmarks",
                      "Year", "Fishing Mortality Rate "+yAxisUnits,
                      XLabelNames,true);
    GridData = CombinedYPROut;

}

void
ChartLineYieldPerRecruit::getAndLoadProjectedSSBBenchmarksDataForecast(
        nmfDatabase* databasePtr,
        QChart* chart,
        std::string &ModelName,
        std::string &MSVPAName,
        std::string &ForecastName,
        std::string &ScenarioName,
        int FirstYear,
        int NYears,
        std::string selectedSpecies,
        std::vector<std::string> &SelectedYears,
        std::string MaxScaleY,
        int Nage,
        int RecAge,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList &RowLabels,
        QStringList &ColumnLabels)
{
    int year;
    int NumYears = SelectedYears.size();
    std::vector<std::string> XLabelNames;
    double YMax=0.0;
    double yprObs;
    double ssbMax03;
    double ssbMax10;
    int Forecast_FirstYear;
    int Forecast_LastYear;
    int Forecast_NYears;
    std::string yAxisUnits = getYAxisUnits(databasePtr,selectedSpecies);
    boost::numeric::ublas::matrix<double> Pmature;
    boost::numeric::ublas::matrix<double> FatAge;
    boost::numeric::ublas::matrix<double> WtAtAge;
    boost::numeric::ublas::matrix<double> M1atAge;
    boost::numeric::ublas::matrix<double> M2atAge;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> YPROut;
    boost::numeric::ublas::matrix<double> PRF;
    boost::numeric::ublas::vector<double> FullF;
    boost::numeric::ublas::vector<double> YPRObs;
    boost::numeric::ublas::vector<double> tmpPRF;
    boost::numeric::ublas::vector<double> tmpM2;
    boost::numeric::ublas::vector<double> tmpM1;
    boost::numeric::ublas::vector<double> tmpWt;
    boost::numeric::ublas::vector<double> tmpMat;
    boost::numeric::ublas::vector<double> SSBMax;
    boost::numeric::ublas::matrix<double> CombinedYPROut;
    boost::numeric::ublas::vector<std::string> LegendNames;

    nmfUtils::initialize(Pmature, NumYears, Nage);
    nmfUtils::initialize(FatAge,  NumYears, Nage);
    nmfUtils::initialize(WtAtAge, NumYears, Nage);
    nmfUtils::initialize(M1atAge, NumYears, Nage);
    nmfUtils::initialize(M2atAge, NumYears, Nage);
    nmfUtils::initialize(Catch,   NumYears, Nage);
    nmfUtils::initialize(FullF,   NumYears);
    nmfUtils::initialize(PRF,     NumYears, Nage);
    nmfUtils::initialize(YPROut,  26, NumYears);  // RSK replace the hardcoded 25 with a constant
    nmfUtils::initialize(YPRObs,  NumYears);
    nmfUtils::initialize(SSBMax, NumYears);
    nmfUtils::initialize(CombinedYPROut, NumYears, 3); // RSK replace with constant
    nmfUtils::initialize(GridData,       NumYears, 4);
    nmfUtils::initialize(LegendNames, 3);

    RowLabels.clear();
    ColumnLabels.clear();

    getForecastYears(databasePtr,MSVPAName,ForecastName,
                     Forecast_FirstYear,Forecast_LastYear,Forecast_NYears);

    // OK...then will have to select appropriate data for your species and such
    // Get prop mature and wt at age

    for (int i=0; i<NumYears; ++i) {
        year = std::stoi(SelectedYears[i]);

        XLabelNames.push_back(std::to_string(year));

        // Get prop mature and wt at age
        databasePtr->nmfQueryMaturityData(
                    i,FirstYear,Nage,selectedSpecies,Pmature);

        // Get Wt At Age...wts at beginning of year
        databasePtr->nmfQueryForecastWeightAtAgeData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,WtAtAge);

        // Get mortalities at age F, M2, M1
        databasePtr->nmfQueryForecastMortalityData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,FatAge,M1atAge,M2atAge);

        // Need to get catches to calculate a weighted average FullF
        databasePtr->nmfQueryForecastAgeCatchData(
                    MSVPAName,ForecastName,ScenarioName,selectedSpecies,
                    year-Forecast_FirstYear,Nage,i,Catch);

    } // end for i


    // Calculate weighted average F..flat top above RecAge..do we want an option though..probably
    calculateWeightAveFAndAssignPRFs(NumYears,RecAge,Nage,FatAge,Catch,FullF,PRF);

    // Ok..that's all we need to send to the YPR calc.
    nmfUtils::initialize(tmpPRF, Nage);
    nmfUtils::initialize(tmpM2,  Nage);
    nmfUtils::initialize(tmpM1,  Nage);
    nmfUtils::initialize(tmpWt,  Nage);
    nmfUtils::initialize(tmpMat, Nage);

    ColumnLabels << "Obs. SSB/R" << "SSB/R Max"
                 << "3% SSB/R Max" << "10% SSB/R Max";

    for (int i = 0; i < NumYears; ++ i) {
        RowLabels << QString::number(Forecast_FirstYear+i);
        for (int j = 0; j < Nage; ++j) {
            tmpPRF(j) = PRF(i,j);
            tmpM2(j)  = M2atAge(i,j);
            tmpM1(j)  = M1atAge(i,j);
            tmpWt(j)  = WtAtAge(i,j);
            tmpMat(j) = Pmature(i,j);
        }

        // Calculate Observed SSB
        YPRObs(i) = SSB(tmpWt, tmpPRF, tmpM2, FullF(i), tmpM1, Nage, tmpMat);
        // Calculate SSBMax
        SSBMax(i) = SSB(tmpWt, tmpPRF, tmpM2, 0, tmpM1, Nage, tmpMat);

        yprObs = YPRObs(i);
        ssbMax03 = SSBMax(i) * 0.03;
        ssbMax10 = SSBMax(i) * 0.10;

        CombinedYPROut(i,0) = yprObs;
        CombinedYPROut(i,1) = ssbMax03;
        CombinedYPROut(i,2) = ssbMax10;

        GridData(i,0) = yprObs;
        GridData(i,1) = SSBMax(i);
        GridData(i,2) = ssbMax03;
        GridData(i,3) = ssbMax10;

        YMax = std::max({YMax,
                         CombinedYPROut(i,0),
                         CombinedYPROut(i,1),
                         CombinedYPROut(i,2)});

    } // end for i

    LegendNames(0) = "Observed SSB/R";
    LegendNames(1) = "3% of Max SSB/R";
    LegendNames(2) = "10% of Max SSB/R";

    YMax = (MaxScaleY.empty()) ? YMax : std::stod(MaxScaleY);
    loadChartWithData(chart, YMax,
                      selectedSpecies,
                      CombinedYPROut, LegendNames,
                      "Projected SSB/R Benchmarks",
                      "Year", "Fishing Mortality Rate "+yAxisUnits,
                      XLabelNames,true);
}

