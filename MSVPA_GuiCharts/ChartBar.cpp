#include "ChartBar.h"
#include "nmfConstants.h"

ChartBar::ChartBar()
{

}



bool
ChartBar::isOfTypeAbundance(std::string selectedVariable)
{
    return ((selectedVariable == "Total Abundance")  ||
            (selectedVariable == "Abundance by Age") ||
            (selectedVariable == "Recruit Abundance"));
}



void
ChartBar::getMaturityData(
        nmfDatabase *databasePtr,
        const std::string &ModelName,
        const int &Nage,
        const int &NYears,
        const int &FirstYear,
        const int &LastYear,
        const std::string &SpeName,
        boost::numeric::ublas::matrix<double> &Maturity)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    int m = 0;

    fields = {"PMature"};

    if (ModelName == "MSVPA") {
        queryStr = "SELECT PMature from SpeMaturity where SpeName='" + SpeName +
                   "' and Year >= " + std::to_string(FirstYear) +
                   "  and Year <= " + std::to_string(LastYear) +
                   "  ORDER By Age,Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (int i=0; i<Nage; ++i) {
            for (int j=0; j<NYears; ++j) {
                Maturity(i,j) = std::stod(dataMap["PMature"][m++]);
            }
        }
    } else if (ModelName == "Forecast") {
        queryStr = "SELECT PMature from SpeMaturity where SpeName='" + SpeName +
                   "' and Year = " + std::to_string(FirstYear) +
                   "  ORDER By Age";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (int i=0; i<Nage; ++i) {
            for (int j=0; j<NYears; ++j) {
                    Maturity(i,j) = std::stod(dataMap["PMature"][m]);
            }
            ++m;
        }
    }


} // end getMaturityData


void
ChartBar::getSpawningStockBiomassDataAndLoadChart(
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
        const std::string &SelectedSpecies,
        const std::string &ageStr,
        const int &season,
        const int &FirstYear,
        const int &LastYear,
        QChart *chart,
        const std::string &title,
        const std::string &titleSuffix,
        const bool &rearrangeTitle,
        const std::string &yLabel,
        const std::string &MaxScaleY,
        QStringList &RowLabels,
        int &Theme)
{
    int m = 0;
    int SSBNumYears=0;
    int SSBFirstYear=0;
    int SSBLastYear=0;
    int Forecast_FirstYear=0;
    //int Forecast_LastYear=0;
    int Forecast_NYears=0;
    double TmpBM = 0.0;
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string field;
    QStackedBarSeries *series = NULL;
    std::size_t foundSpawning;
    QStringList categories;
    boost::numeric::ublas::matrix<double> Maturity;
    boost::numeric::ublas::matrix<double> tempData;
    QStringList LegendNames = {""};

    series = NULL;

    if (ModelName == "MSVPA") {

        // RSK  - make lstrip function in nmfUtils
        field = fieldToSum;
        foundSpawning = field.find("Spawning");
        if (foundSpawning != std::string::npos) { // then Spawning was found...
            field.erase(0,field.find("_")+1);
        }

        nmfUtils::initialize(ChartData, NYears, 1);
        nmfUtils::initialize(GridData,  NYears, 1);

        fields = {"Year", "Age", "Biomass"};
        queryStr = "SELECT Year, Age, Sum(" + field + ") as Biomass from MSVPASeasBiomass WHERE MSVPAname='" + MSVPAName +
                "' and SpeName='" + SelectedSpecies +
                "' and Season="   + std::to_string(season) +
                "  GROUP BY Age,Year ";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        SSBNumYears  = NYears;
        SSBFirstYear = FirstYear;
        SSBLastYear  = LastYear;

    } else if (ModelName == "Forecast") {

        // Find number of Forecast years
        fields    = {"InitYear","NYears"};
        queryStr  = "SELECT InitYear,NYears FROM Forecasts WHERE MSVPAName = '" +
                    MSVPAName + "' AND ForeName = '" + ForecastName + "'";
        dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["NYears"].size() > 0) {
            Forecast_FirstYear = std::stoi(dataMap["InitYear"][0]);
            Forecast_NYears    = std::stoi(dataMap["NYears"][0]);
            //Forecast_LastYear  = Forecast_FirstYear + Forecast_NYears - 1;
        } else {
            //logger->logMsg(nmfConstants::Error,"No NYears data for Forecast: "+ForecastName);
            return;
        }
        nmfUtils::initialize(ChartData, Forecast_NYears+1, 1);
        nmfUtils::initialize(GridData,  Forecast_NYears+1, 1);

        // Get annual biomass by age class
        fields = {"Year", "Age", "Biomass"};
        queryStr = "SELECT Year, Age, Sum(InitBiomass) AS Biomass FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                        " AND ForeName = '" + ForecastName + "'" +
                        " AND Scenario = '" + ScenarioName + "'" +
                        " AND SpeName = '" + SelectedSpecies + "'" +
                        " AND Season = " + std::to_string(season) +
                        " GROUP By Age,Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        SSBNumYears  = Forecast_NYears+1;
        SSBFirstYear = Forecast_FirstYear;
        SSBLastYear  = Forecast_FirstYear + Forecast_NYears;
    }

    m = 0;
    nmfUtils::initialize(tempData, Nage, SSBNumYears);
    for (int i=0; i<Nage; ++i) {
        for (int j=0; j<SSBNumYears; ++j) {
            tempData(i,j) = std::stod(dataMap["Biomass"][m++]);
        }
    }
    nmfUtils::initialize(Maturity, Nage, SSBNumYears);
    getMaturityData(databasePtr, ModelName, Nage, SSBNumYears, SSBFirstYear, SSBLastYear,
                    SelectedSpecies, Maturity);
    for (int i=0; i<SSBNumYears; ++i) {
        categories << QString::number(SSBFirstYear + i);
        TmpBM = 0.0;
        for (int j=0; j<Nage; ++j) {
            TmpBM += tempData(j,i) * Maturity(j,i);
        }        
        ChartData(i,0) = (0.5 * TmpBM) / 1000.0; // 50% sex ration...but will need to include a place to enter it somewhere
    }

    RowLabels = categories;
    GridData  = ChartData;

    loadChartWithData(chart,series,ChartData, LegendNames,
                      false, MaxScaleY, false);

    setTitles(chart,
              series,
              categories,
              "",
              SelectedSpecies,
              title,
              titleSuffix,
              "Year",
              yLabel,
              nmfConstants::DontRearrangeTitle,
              false,
              Theme);

} // end getSpawningStockBiomassDataAndLoadChart




void
ChartBar::getDataAndLoadChart(
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
        const std::string &selectedSpeciesAgeSizeClass,
        const std::string &ageStr,
        const std::string &recruitChar,
        const std::string &seasonStr,
        const int &season,
        const std::string &selectedPreyName,
        const int &FirstYear,
        const int &LastYear,
        const int &SpeAge,
        QChart *chart,
        std::string &title,
        const std::string &titleSuffix,
        const bool &rearrangeTitle,
        const std::string &yLabel,
        const double &scaleFactor,
        const double &NConversion,
        const std::string &MaxScaleY,
        QStringList &RowLabels,
        QStringList &LegendNames,
        int &Theme)
{
    int ageVal=0;
    std::string newAgeStr = ageStr;
    std::string newYLabel="";
    std::string SizeUnitString;
    std::string WtUnitString;

    bool scaleFactorBool = isOfTypeAbundance(selectedVariable);

    if (newAgeStr != "") {
        ageVal = std::stoi(newAgeStr.erase(0,newAgeStr.find("=")+1));
    }

    if (ModelName == "MSVPA") {

        getChartDataMSVPA(databasePtr,
                          MSVPAName,
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
                          RowLabels,
                          LegendNames,
                          SizeUnitString,
                          WtUnitString);

    } else if (ModelName == "Forecast") {

        getChartDataForecast(databasePtr,
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
                             selectedSpeciesAgeSizeClass,
                             ageStr,
                             seasonStr,
                             selectedPreyName,
                             scaleFactor,
                             NConversion,
                             ChartData,
                             GridData,
                             RowLabels,
                             LegendNames,
                             SizeUnitString,
                             WtUnitString);
    }

    if (ChartData.size1() == 0) {
std::cout << "Error: ChartBar::getDataAndLoadChart ChartData.size1 = 0" << std::endl;
        return;
    }

    newYLabel = yLabel;
    if (yLabel == "SizeUnitString") {
        newYLabel = SizeUnitString;
    } else if (yLabel == "WtUnitString") {
        newYLabel = WtUnitString;
    }

    if (selectedVariable == "Food Availability by Prey Age") {
        title = title + " " + selectedPreyName;
    }

    if ((selectedVariable == "Total Fishery Catch (Numbers)") ||
        (selectedVariable == "Total Fishery Yield (Biomass)") ||
        (selectedVariable == "Consumption by Prey Type")) {
        newAgeStr = "";
    } else if (newAgeStr != "") {
        newAgeStr = "Age " + std::to_string(ageVal) + recruitChar;
    }

    if ((selectedVariable == "Consumption by Prey Type")         ||
        (selectedVariable == "Fishing Mortality")                ||
        (selectedVariable == "Average Recruited F")              ||
        (selectedVariable == "Predation Mortality")              ||
        (selectedVariable == "Predation Mortality by Predator")  ||
        (selectedVariable == "Cons. by Pred. Age and Prey Type") ||
        (selectedVariable == "Consumption by Predator Age")      ||
        (selectedVariable == "Consumption by Prey Age"))
    {
        QBarSeries *series = NULL;
        loadChartWithData(chart,series,ChartData, LegendNames,
                          false, MaxScaleY, scaleFactorBool);
        setTitles(chart, series, RowLabels,
                  newAgeStr, selectedSpecies,
                  title, titleSuffix, "Year",
                  newYLabel, rearrangeTitle, ChartData.size2()>1, Theme);

        // Test constructMainTitle
        // Construct main title
//        std::string MainTitle = constructMainTitle(
//                    rearrangeTitle, title, newAgeStr,
//                    selectedSpecies, titleSuffix);
//std::cout << "MainTitle: " << MainTitle << std::endl;


//        // Populate the chart
//        populateChart(chart,
//                      ChartData,
//                      RowLabels,
//                      LegendNames,
//                      MainTitle,
//                      XLabel,
//                      YLabel,
//                      GridLines,
//                      Theme);
    } else {
        QStackedBarSeries *series = NULL;
        loadChartWithData(chart,series,ChartData, LegendNames,
                          false, MaxScaleY, scaleFactorBool);
        setTitles(chart, series, RowLabels,
                  newAgeStr, selectedSpecies,
                  title, titleSuffix, "Year",
                  newYLabel, rearrangeTitle, ChartData.size2()>1, Theme);
    }

} // end getDataAndLoadChart


void ChartBar::getChartDataMSVPA(
        nmfDatabase *databasePtr,
        const std::string &MSVPAName,
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
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        std::string &SizeUnitString,
        std::string &WtUnitString)
{
    int m;
    int NPrey;
    int firstYear;
    int nYears;
    int NumRecords;
    unsigned int NPreyAge;
    double AvgFA = 0.0;
    std::string queryStr,queryStr2,queryStr3;
    std::map<std::string, std::vector<std::string> > dataMap,dataMap2,dataMap3;
    std::vector<std::string> fields,fields2,fields3;
    std::string field;
    std::size_t found;
    QStringList PreyList;
    boost::numeric::ublas::matrix<double> CatchData;
    boost::numeric::ublas::matrix<double> tempData;
    double tmpSum;
    double tmpTotCat;

    // Get conversion factor
    double WtConversion   = 1.0;
    double SizeConversion = 1.0;
    fields = {"WtUnits","SizeUnits"};
    queryStr = "SELECT WtUnits,SizeUnits FROM Species WHERE SpeName = '" + selectedSpecies + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["WtUnits"].size() != 0) {
        WtConversion   = std::stod(dataMap["WtUnits"][0]);
        SizeConversion = std::stod(dataMap["SizeUnits"][0]);
    } else {
        fields = {"WtUnits","SizeUnits"};
        queryStr = "SELECT WtUnits,SizeUnits FROM OtherPredSpecies WHERE SpeName = '" + selectedSpecies + "'";
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
        WtUnitString = "Metric Tons";
    else if (WtConversion == (1.0/2.2)) // want weight in pounds so 000 pounds
        WtUnitString = "000 Pounds";

    double sf = 1000.0;
    if ((selectedVariable == "Fishing Mortality") ||
        (selectedVariable == "Predation Mortality")) {
            sf = 1.0;
    }


    if ((selectedVariable == "Total Biomass")     ||
        (selectedVariable == "Biomass by Age")    ||
        (selectedVariable == "Total Abundance")   ||
        (selectedVariable == "Abundance by Age")  ||
        (selectedVariable == "Recruit Abundance") ||
        (selectedVariable == "Recruit Biomass")   ||
        (selectedVariable == "Fishing Mortality") ||
        (selectedVariable == "Predation Mortality"))
    {
        nmfUtils::initialize(ChartData, NYears, 1);
        fields = {"Year", "SumField"};

        // RSK  - make lstrip function in nmfUtils
        field = fieldToSum;
        found = field.find("Recruit");
        if (found != std::string::npos) { // then Recruit was found...
            field.erase(0,field.find("_")+1);
        }
        queryStr = "SELECT Year, Sum(" + field + ") as SumField from MSVPASeasBiomass WHERE MSVPAname='" + MSVPAName + "'"
                   " and SpeName='" + selectedSpecies + "' " +
                   ageStr + seasonStr + " GROUP BY YEAR ";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() > 0) {
            for (int i=0; i<NYears; ++i) {
                RowLabels << QString::number(FirstYear + i);
                ChartData(i,0) = std::stod(dataMap["SumField"][i])/sf; //scaleFactor;
            }
        }

    } else if (selectedVariable == "Relative Food Availability") {

        boost::numeric::ublas::vector<double> tempData;
        nmfUtils::initialize(tempData,  NYears);
        nmfUtils::initialize(ChartData, NYears, 1);

        fields = {"Year","TotFA"};
        queryStr = "SELECT Year, Sum(SuitPreyBiomass) AS TotFA FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                   " AND PredName = '" + selectedSpecies + "'" +
                     ageStr + seasonStr +
                   " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() == 0) {
            return;
        }
        for (int i = 0; i < NYears; ++i) {
            tempData(i) = std::stod(dataMap["TotFA"][i]);
            AvgFA += tempData(i);
        }
        AvgFA /= double(NYears + 1.0);

        // Convert this to a relative amount
        for (int i = 0; i < NYears; ++i) {
            RowLabels << QString::number(FirstYear + i);
            ChartData(i,0) = tempData(i) / AvgFA;
        }
        GridData = ChartData;

    } else if (selectedVariable == "Per Capita Food Availability") {
        boost::numeric::ublas::vector<double> tempData;
        boost::numeric::ublas::vector<double> TotBM;

        std::string newAgeStr = ageStr;
        int PredAgeVal = (! newAgeStr.empty()) ? std::stoi(newAgeStr.erase(0,newAgeStr.find("=")+1)) : 0;
        fields  = {"Year","Biomass"};
        fields2 = {"Year","TotFA"};
        queryStr = "SELECT Year, Avg(Biomass) AS Biomass FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND SpeName = '"  + selectedSpecies + "'" +
                " AND Age = " + std::to_string(PredAgeVal) +
                  seasonStr +
                " GROUP By Year";
        queryStr2 = "SELECT Year, Sum(SuitPreyBiomass) AS TotFA FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND PredName = '" + selectedSpecies + "'" +
                  ageStr + seasonStr +
                " GROUP By Year";

        nmfUtils::initialize(tempData,  NYears);
        nmfUtils::initialize(ChartData, NYears, 1);
        nmfUtils::initialize(GridData,  NYears, 1);
        nmfUtils::initialize(TotBM, NYears);

        // Get total biomass for predator/age class..in this case its the average of the seasonal biomasses
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() == 0) {
            return;
        }
        for (int i = 0; i < NYears; ++i) {
            TotBM(i) = std::stod(dataMap["Biomass"][i]);
        }

        // Then get and calculate food availability stuff
        dataMap2 = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        if (dataMap2.size() == 0) {
            return;
        }
        for (int i = 0; i < NYears; ++i) {
            tempData(i) = std::stod(dataMap2["TotFA"][i]) / TotBM(i);
            AvgFA += tempData(i);
        }
        AvgFA /= (NYears + 1.0);

        // Convert this to a relative amount
        for (int i = 0; i < NYears; ++i) {
            RowLabels << QString::number(FirstYear + i);
            ChartData(i,0) = tempData(i) / AvgFA;
        }
        GridData = ChartData;

    } else if (selectedVariable == "Stomach Content Weight") {

        nmfUtils::initialize(ChartData, NYears, 1);
        nmfUtils::initialize(GridData,  NYears, 1);

        fields = {"Year","Stom"};
        queryStr = "SELECT Year, Avg(StomCont) as Stom FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                   " AND SpeName = '"  + selectedSpecies + "'" +
                   ageStr + seasonStr +
                   " Group By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() == 0) {
            return;
        }
        for (int i = 0; i < NYears; ++i) {
            RowLabels << QString::number(FirstYear + i);
            ChartData(i,0) = std::stod(dataMap["Stom"][i]);
        }
        GridData = ChartData;
    } else if (selectedVariable == "Food Availability by Prey Age") {

        //QStringList PreyAges;
        ColumnLabels.clear();
        std::string PreyName = selectedPreyName;
        boost::numeric::ublas::vector<double> PreyAge;
        nmfUtils::initialize(PreyAge,  nmfConstants::MaxNumberAges);
        int NPreyAge;
        boost::numeric::ublas::vector<double> tempData;
        std::string PredAgeStr = "";

        fields  = {"PreyAge"};
        fields2 = {"Year","TotFA"};
        fields3 = {"Year","PreyAge","SBM"};


            nYears    = NYears;
            firstYear = FirstYear;

            // First get a list of ages for the prey species...might only be one if it's another prey...
            queryStr = "SELECT DISTINCT PreyAge FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND PredName = '" + selectedSpecies + "'" +
                    PredAgeStr + seasonStr +
                    " AND PreyName = '" + PreyName + "'" +
                    " ORDER By PreyAge";

            // Then want to get total suitable biomass and calculate average as previously
            queryStr2 = "SELECT Year, Sum(SuitPreyBiomass) AS TotFA FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND PredName = '" + selectedSpecies + "'" +
                    PredAgeStr + seasonStr +
                    " GROUP By Year";
            // Then want to get suitable prey biomass by prey age class
            queryStr3 = "SELECT Year, PreyAge, Sum(SuitPreyBiomass) AS SBM FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND PredName = '" + selectedSpecies + "'" +
                    PredAgeStr + seasonStr +
                    " AND PreyName = '" + PreyName + "'" +
                    " GROUP By Year, PreyAge";


        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() == 0) {
            return;
        }
        NPreyAge = dataMap["PreyAge"].size();

        nmfUtils::initialize(tempData, nYears);
        nmfUtils::initialize(ChartData, nYears, NPreyAge);
        nmfUtils::initialize(GridData,  nYears, NPreyAge);

        dataMap2 = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        if (dataMap2.size() == 0) {
            return;
        }
        for (int i = 0; i < nYears; ++i) {
            tempData(i) = std::stod(dataMap2["TotFA"][i]);
            AvgFA += tempData(i);
        }
        AvgFA /= (nYears + 1.0);

        dataMap3 = databasePtr->nmfQueryDatabase(queryStr3, fields3);
        m = 0;
        RowLabels.clear();
        for (int i = 0; i < nYears; ++i) {
            ColumnLabels.clear();
            RowLabels << QString::number(firstYear + i);
            for (int j = 0; j < NPreyAge; ++j) {
                if (m < int(dataMap3["SBM"].size())) {
                    if (std::stoi(dataMap3["PreyAge"][j]) == j) {
                        ColumnLabels << QString::fromStdString("Age "+std::to_string(j));
                        ChartData(i,j) = std::stod(dataMap3["SBM"][m++]) / AvgFA;
                        GridData(i,j)  = ChartData(i,j);
                    }
                }
            } // end for j
        } // end for i
        //GridData = ChartData;

    } else if (selectedVariable == "Total Biomass Consumed") {

        fields = {"Year","TotCons"};

            nYears = NYears;
            firstYear = FirstYear;
            queryStr = "SELECT Year, Sum(BMConsumed) AS TotCons FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND PredName = '" +  selectedSpecies + "'" +
                      seasonStr + ageStr +
                    " GROUP By Year";
        nmfUtils::initialize(ChartData, nYears, 1);

        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() == 0) {
            return;
        }
        RowLabels.clear();
        for (int i = 0; i < nYears; ++i) {
            RowLabels << QString::number(firstYear + i);
            ChartData(i,0) = std::stod(dataMap["TotCons"][i])/1000.0; // Replace 1000 with a variable
        }

    } else if (selectedVariable == "Consumption by Predator Age") {

        fields = {"Year","TotCons"};

            nYears    = NYears;
            firstYear = FirstYear;
            queryStr = "SELECT Year, Sum(BMConsumed) AS TotCons FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND PredName = '" +  selectedSpecies + "'" +
                      seasonStr +
                    " AND PredAge = " + std::to_string(SpeAge) +
                    " GROUP By Year";

        nmfUtils::initialize(ChartData, nYears, 1);

        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() == 0) {
            return;
        }
        RowLabels.clear();
        for (int i = 0; i < nYears; ++i) {
            RowLabels << QString::number(firstYear + i);
            ChartData(i,0) = std::stod(dataMap["TotCons"][i])/1000.0; // Replace 1000 with a variable
        }

        // Calculate Grid Data
        nmfUtils::initialize(GridData, nYears, Nage);
        ColumnLabels.clear();
        for (int j=0; j<Nage; ++j) {
            ColumnLabels << "Age "+QString::number(j);
        }
        m = 0;
        fields = {"Year", "TotCons"};
//        if (theModelName == "MSVPA") {
            queryStr = "Select Year, Sum(BMConsumed) As TotCons FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'"
                       " AND PredName = '" + selectedSpecies + "'" +
                       seasonStr + " GROUP BY Year,PredAge";
//        } else if (theModelName == "Forecast") {
//            queryStr = "Select Year, Sum(TotalBMConsumed) As TotCons FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'"
//                       " AND ForeName = '" + ForecastName + "'" +
//                       " AND Scenario = '" + ScenarioName + "'" +
//                       " AND SpeName = '" + selectedSpecies + "'" +
//                       seasonStr + " GROUP BY Year,Age";
//        }
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() == 0) {
            return;
        }
        for (int i=0; i<nYears; ++i) {
            for (int j=0; j<Nage; ++j) {
                GridData(i,j) = nmfUtils::round(std::stod(dataMap["TotCons"][m++]),1)/1000.0;
            } // end for j
        } // end for i

    } else if ((selectedVariable == "Consumption by Prey Type") ||
               (selectedVariable == "Cons. by Pred. Age and Prey Type")) {  // This should not be a Stacked Bar

        bool inclPredAge = (selectedVariable == "Cons. by Pred. Age and Prey Type");
        std::string predAgeStr = inclPredAge ? ageStr : "";

        fields = {"Year","PreyName","TotCons"};

        nYears    = NYears;
        firstYear = FirstYear;
        // First need to get a list of prey names and a count of prey for the predator.
        databasePtr->nmfQueryMsvpaPreyList( "", selectedSpecies, MSVPAName, predAgeStr,
                                            NPrey, PreyList, false);

        queryStr = "SELECT Year, PreyName, Sum(BMConsumed) as TotCons FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND PredName = '" + selectedSpecies + "'" +
                seasonStr + predAgeStr +
                " GROUP BY Year, PreyName";

        NPrey = PreyList.size();
        nmfUtils::initialize(ChartData, nYears, NPrey);
        //nmfUtils::initialize(GridData,  nYears, NPrey);
        ColumnLabels.clear();
        for (int i=0; i<NPrey; ++i) {
            ColumnLabels << PreyList[i];
        }
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() == 0) {
            return;
        }
        m = 0;
        for (int i = 0; i< nYears; ++i) {
            RowLabels << QString::number(firstYear + i);
            for (int j = 0; j < NPrey; ++j) {
                // Need this check since there may be data for some missing species
                if (dataMap["PreyName"][m] == PreyList[j].toStdString()) {
                    ChartData(i,j) = std::stod(dataMap["TotCons"][m++]) / 1000.0;
                }
                //GridData(i,j) = ChartData(i,j);
            }
        }

    } else if (selectedVariable == "Consumption by Prey Age") {

        std::string newAgeStr = ageStr;
        int PredAgeVal = (! newAgeStr.empty()) ? std::stoi(newAgeStr.erase(0,newAgeStr.find("=")+1)) : 0;

        boost::numeric::ublas::vector<int> PreyAge;
        nmfUtils::initialize(PreyAge, nmfConstants::MaxNumberAges);

        // First get a list of ages for the prey species...might only be one if it's another prey...
        fields = {"PreyAge"};
        fields2 = {"Year","PreyAge","TotCons"};

        nYears    = NYears;
        firstYear = FirstYear;
        queryStr = "SELECT DISTINCT(PreyAge) FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND PredName = '" + selectedSpecies + "'" +
                " AND PredAge = "   + std::to_string(PredAgeVal) +
                " AND PreyName = '" + selectedPreyName + "'" + seasonStr +
                " ORDER BY PreyAge";
        // Then select the biomass consumed of that prey age by predator age across the byvar
        queryStr2 = "SELECT Year, PreyAge, Sum(BMConsumed) as TotCons FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND PredName = '" + selectedSpecies + "'" +
                " AND PredAge = "   + std::to_string(PredAgeVal) +
                " AND PreyName = '" + selectedPreyName + "'" + seasonStr +
                " GROUP By Year, PreyAge";

        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() == 0) {
            return;
        }
        NPreyAge = dataMap["PreyAge"].size();
        ColumnLabels.clear();
        for (unsigned i = 0; i < NPreyAge; ++i) {
            PreyAge(i) = std::stoi(dataMap["PreyAge"][i]);
            ColumnLabels << QString::fromStdString("Age " + std::to_string(PreyAge(i)));
        }

        nmfUtils::initialize(ChartData, nYears, NPreyAge);
        nmfUtils::initialize(GridData,  nYears, NPreyAge);
        dataMap2 = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        if (dataMap2.size() == 0) {
            return;
        }
        m = 0;
        NumRecords = dataMap2["PreyAge"].size();
        for (auto i = 0; i < nYears; ++i) {
            RowLabels << QString::number(firstYear + i);
            for (unsigned j = 0; j < NPreyAge; ++j) {
                if (m < NumRecords) {
                    if (std::stod(dataMap2["PreyAge"][m]) == PreyAge(j)) {
                        ChartData(i,j) = std::stod(dataMap2["TotCons"][m]) / 1000.0; // RSK - use a constant here
                        GridData(i,j) = ChartData(i,j);
                        ++m;
                    }
                } else {
                    break;
                }
            }
        }

    } else if (selectedVariable == "Wt. Specific Consumption") {

        // weight specific consumption rates..load params and calculate for a particular season
        // include age class in the grid matrix, but plot for a single age.

         boost::numeric::ublas::matrix<double> AgeStomCont;
         boost::numeric::ublas::matrix<double> AgeConsump;
         boost::numeric::ublas::vector<double> SeasTemp;
         //boost::numeric::ublas::vector<int>    PredType;

         nYears    = NYears;
         firstYear = FirstYear;

         queryStr2 = "SELECT Year, Age, StomCont FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                 " AND SpeName = '"  + selectedSpecies +  "'" +
                 seasonStr +
                 " ORDER By Year, Age";

         nmfUtils::initialize(AgeStomCont, nYears, Nage+1);
         nmfUtils::initialize(AgeConsump,  Nage+1,  2);
         nmfUtils::initialize(SeasTemp,    nYears);
         nmfUtils::initialize(ChartData,   nYears, 1);
         nmfUtils::initialize(GridData,    nYears, Nage);
         //nmfUtils::initialize(PredType,    nmfConstants::MaxNumberSpecies);

         // Load predator name and age lists for use later
         QStringList PredType0;
         fields = {"SpeName"};
         queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName +
                    "' AND Type = 0";
         dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
         if (dataMap.size() == 0) {
             return;
         }
         int NPreds = dataMap["SpeName"].size();
         for (int i = 0; i < NPreds; ++i) {
             PredType0 << QString::fromStdString(dataMap["SpeName"][i]);
             //PredType(i) = 0;
         } // end for i

//         // Load Other Predator Names and Ages
//         fields = {"SpeName"};
//         queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "' AND Type = 3";
//         dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
//         int NOthPreds = dataMap["SpeName"].size();
//         for (int i = 0; i < NOthPreds; ++i) {
//             PredType(i+NPreds) = 1;
//         } // end for i

         fields2 = {"Year","Age","StomCont"};
         dataMap2 = databasePtr->nmfQueryDatabase(queryStr2, fields2);
         if (dataMap2.size() == 0) {
             return;
         }
         m = 0;
         for (int i = 0; i < nYears; ++i) {
             for (int j = 0; j < Nage; ++j) {
                 AgeStomCont(i,j) = std::stod(dataMap2["StomCont"][m++]);
             }
         }

         //if (PredType(selectedSpeciesNum) == 0) {
         if (PredType0.contains(QString::fromStdString(selectedSpecies))) {
             fields = {"Age","EvacAlpha","EvacBeta"};
             queryStr = "SELECT Age, EvacAlpha, EvacBeta FROM MSVPASizePref WHERE MSVPAname = '" + MSVPAName + "'" +
                        " AND SpeName = '" + selectedSpecies + "'" +
                        " ORDER By Age";
             dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
             if (dataMap.size() == 0) {
                 return;
             }
             for (int j = 0; j < Nage; ++j) {
                 AgeConsump(j,0) = std::stod(dataMap["EvacAlpha"][j]);
                 AgeConsump(j,1) = std::stod(dataMap["EvacBeta"][j]);
             }
         } else {
             fields = {"SizeCat","ConsAlpha","ConsBeta"};
             queryStr = "SELECT SizeCat, ConsAlpha, ConsBeta FROM OthPredSizeData WHERE SpeName = '" + selectedSpecies + "'" +
                        " ORDER BY SizeCat";
             dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
             if (dataMap.size() == 0) {
                 return;
             }
             for (int j = 0; j < Nage; ++j) {
                 AgeConsump(j, 0) = std::stod(dataMap["ConsAlpha"][j]);
                 AgeConsump(j, 1) = std::stod(dataMap["ConsBeta"][j]);
             }
         }

         fields = {"Year","Season","Value"};
         std::string newSeasonStr = seasonStr;
         int SeasonVal = (! newSeasonStr.empty()) ? std::stoi(newSeasonStr.erase(0,newSeasonStr.find("=")+1))+1 : 0;
         queryStr = "SELECT Year, Season, Value FROM MSVPASeasInfo WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND Season = " + std::to_string(SeasonVal) +
                    " AND Variable = 'SeasTemp' ORDER by Year"; // AND Year = " + std::to_string(FirstYear);
         dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
         if (dataMap.size() == 0) {
             return;
         }
         std::string newAgeStr = ageStr;
         int PredAgeVal = (! newAgeStr.empty()) ? std::stoi(newAgeStr.erase(0,newAgeStr.find("=")+1)) : 0;

         for (int i = 0; i < nYears; ++i) {
             RowLabels << QString::number(firstYear + i);
             SeasTemp(i) = std::stod(dataMap["Value"][0]);
             ChartData(i,0) = AgeStomCont(i,PredAgeVal) * 24.0 * AgeConsump(PredAgeVal,0) *
                              std::exp( AgeConsump(PredAgeVal,1) * SeasTemp(i) );
         }

         for (int i = 0; i < nYears; ++i) {
             for (int j = 0; j < Nage; ++j) {
                 if (i == 0) {
                     ColumnLabels << "Age " + QString::number(j);
                 }
                 GridData(i,j) = AgeStomCont(i,j) * 24.0 * AgeConsump(j,0) *
                                 std::exp(AgeConsump(j,1) * SeasTemp(i));
             }
         }

    } else if (selectedVariable == "Average Recruited F") {
        // Check if there's incomplete data...
        if (Nage == 0) {
            return;
        }
        nmfUtils::initialize(CatchData, NYears, Nage);
        nmfUtils::initialize(tempData,  NYears, Nage);
        nmfUtils::initialize(ChartData, NYears, 1);
        nmfUtils::initialize(GridData,  NYears, 1);

        // Need the catch data
        fields = {"MinCatAge","MaxCatAge"};
        queryStr = "SELECT MinCatAge,MaxCatAge from Species WHERE SpeName = '" + selectedSpecies + "' ";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() == 0) {
            return;
        }
        int FirstCatchAge = std::stoi(dataMap["MinCatAge"][0]);
        int LastCatchAge  = std::stoi(dataMap["MaxCatAge"][0]);
        fields = {"Catch"};
        queryStr = "SELECT Catch from SpeCatch WHERE SpeName = '" + selectedSpecies + "' " +
                " AND (Year >= " + std::to_string(FirstYear) +
                " AND  Year <= " + std::to_string(LastYear)  + ") " +
                " ORDER by Year, Age";

        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() == 0) {
            return;
        }
        m = 0;
        for (int i=0; i<NYears; ++i) {
            for (int j=FirstCatchAge; j<=LastCatchAge; ++j) {
                CatchData(i,j) = std::stod(dataMap["Catch"][m++]);
            }
        }

        // Get the species data
        fields = {"Year","Age","SumField"};
        queryStr = "SELECT Year, Age, Sum(SeasF) as SumField from MSVPASeasBiomass WHERE MSVPAname='" + MSVPAName + "' "                                                                                                                                     " and SpeName='" + selectedSpecies + "' " +
                ageStr + seasonStr + " GROUP BY YEAR, Age ";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() == 0) {
            return;
        }
        m = 0;
        for (int i=0; i<NYears; ++i) {
            for (int j=SpeAge; j<Nage; ++j) {
                tempData(i,j) = std::stod(dataMap["SumField"][m++]);
            }
        }

        // Calculate the ChartData values
        for (int i=0; i<NYears; ++i) {
            tmpSum = 0.0;
            tmpTotCat = 0.0;
            for (int j=SpeAge; j<Nage; ++j) {
                tmpSum += tempData(i,j)*CatchData(i,j);
                tmpTotCat += CatchData(i,j);
            }
            RowLabels << QString::number(FirstYear + i);
            ChartData(i,0) = tmpSum / tmpTotCat;
            GridData(i,0) = ChartData(i,0);
        }

    } else if (selectedVariable == "Predation Mortality by Predator") {

        getChartDataOfPredationMortalityByPredator(
              databasePtr,
               MSVPAName,
              "MSVPA",
              "",
              "",
              "MSVPASuitPreyBiomass",
              "MSVPASeasBiomass",
               ageStr,
               seasonStr,
               FirstYear,
               NYears,
               selectedSpecies,
               SpeAge,
               ColumnLabels,
               RowLabels,
               ChartData);
    }

}


void ChartBar::getChartDataForecast(
        nmfDatabase *databasePtr,
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
        const std::string &selectedSpeciesAgeSizeClass,
        const std::string &ageStr,
        const std::string &seasonStr,
        const std::string &selectedPreyName,
        const double &scaleFactor,
        const double &NConversion,
        boost::numeric::ublas::matrix<double> &ChartData,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList &RowLabels,
        QStringList &ColumnLabels,
        std::string &SizeUnitString,
        std::string &WtUnitString)
{
    int m = 0;
    int NPrey = 0;
    int firstYear = 0;
    int nYears = 0;
    int offset=0;
    int NumRecords = 0;
    int Forecast_FirstYear = 0;
    //int Forecast_LastYear;
    int Forecast_NYears = 0;
    int PredAgeVal = 0;
    unsigned int NPreyAge = 0;
    double AvgFA = 0.0;
    double tmpSum = 0;
    double tmpTotCat = 0;
    std::string queryStr,queryStr2,queryStr3;
    std::map<std::string, std::vector<std::string> > dataMap,dataMap2,dataMap3;
    std::vector<std::string> fields,fields2,fields3;
    std::string PredAgeStr;
    std::string valStr;
    QStringList PreyList;
    boost::numeric::ublas::matrix<double> CatchData;
    boost::numeric::ublas::matrix<double> tempData;

    // Get conversion factor
    double WtConversion = 1.0;
    double SizeConversion = 1.0;
    fields   = {"WtUnits","SizeUnits"};
    queryStr = "SELECT WtUnits,SizeUnits FROM Species WHERE SpeName = '" + selectedSpecies + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["WtUnits"].size() != 0) {
        WtConversion   = std::stod(dataMap["WtUnits"][0]);
        SizeConversion = std::stod(dataMap["SizeUnits"][0]);
    } else {
        fields = {"WtUnits","SizeUnits"};
        queryStr = "SELECT WtUnits,SizeUnits FROM OtherPredSpecies WHERE SpeName = '" + selectedSpecies + "'";
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

    // Find number of Forecast years
    fields    = {"InitYear","NYears"};
    queryStr  = "SELECT InitYear,NYears FROM Forecasts WHERE MSVPAName = '" +
                MSVPAName + "' AND ForeName = '" + ForecastName + "'";
    dataMap   = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["NYears"].size() > 0) {
        Forecast_FirstYear = std::stoi(dataMap["InitYear"][0]);
        Forecast_NYears    = std::stoi(dataMap["NYears"][0])+1;
        //Forecast_LastYear  = Forecast_FirstYear + Forecast_NYears - 1;
    } else {
        //logger->logMsg(nmfConstants::Error,"No NYears data for Forecast: "+ForecastName);
        return;
    }
    if ((selectedVariable == "Total Biomass")     ||
        (selectedVariable == "Biomass by Age")    ||
        (selectedVariable == "Total Abundance")   ||
        (selectedVariable == "Abundance by Age")  ||
        (selectedVariable == "Recruit Abundance") ||
        (selectedVariable == "Recruit Biomass")   ||
        (selectedVariable == "Fishing Mortality") ||
        (selectedVariable == "Predation Mortality"))
    {
        nmfUtils::initialize(ChartData, Forecast_NYears, 1);

        fields = {"Year","Biomass"};
        queryStr = "SELECT Year, Sum(InitBiomass) AS Biomass FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                        " AND ForeName = '" + ForecastName + "'" +
                        " AND Scenario = '" + ScenarioName + "'" +
                        " AND SpeName = '" + selectedSpecies + "' " +
                        ageStr + seasonStr +
                        "  GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i<Forecast_NYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear + i);
            ChartData(i,0) = std::stod(dataMap["Biomass"][i]) / 1000.0; //scaleFactor; //1000;
        }

    } else if (selectedVariable == "Total Fishery Catch (Numbers)") {

        nmfUtils::initialize(ChartData, Forecast_NYears, 1);
        nmfUtils::initialize(GridData,  Forecast_NYears, 1);

        fields = {"Year","Catch"};
        queryStr = "SELECT Year, Sum(SeasCatch) AS Catch FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '" +  selectedSpecies + "'" +
                  seasonStr +
                " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (int i = 0; i < Forecast_NYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear + i);
            ChartData(i,0) = std::stod(dataMap["Catch"][i]) / scaleFactor;
        }
        GridData = ChartData;
    } else if (selectedVariable == "Fishery Catch at Age (Numbers)") {
std::cout << "*** scaleFF: " << scaleFactor << std::endl;
std::cout << "*** NConversion: " << NConversion << std::endl;

        nmfUtils::initialize(ChartData, Forecast_NYears, 1);

        fields = {"Year","Catch"};
        queryStr = "SELECT Year, Sum(SeasCatch) AS Catch FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '" +  selectedSpecies + "'" +
                  seasonStr + ageStr +
                " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i < Forecast_NYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear + i);
            ChartData(i,0) = std::stod(dataMap["Catch"][i]) / scaleFactor;
        }

        m = 0;
        fields = {"Year","Age","SumField"};
        queryStr = "SELECT Year, Age, Sum(SeasCatch) as SumField FROM ForeOutput WHERE MSVPAName = '" + MSVPAName + "'" +
                   " AND ForeName = '" + ForecastName + "'" +
                   " AND Scenario = '" + ScenarioName + "'" +
                   " AND SpeName  = '" + selectedSpecies + "'" +
                   " GROUP BY Year, Age";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        nmfUtils::initialize(GridData, Forecast_NYears, Nage);
        RowLabels.clear();
        ColumnLabels.clear();
        for (int i=0; i<Forecast_NYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear+i);
            for (int j=0; j<Nage; ++j) {
                if (i == 0) {
                    ColumnLabels << QString::fromStdString("Age "+std::to_string(j));
                }
                GridData(i, j) = nmfUtils::round(std::stod(dataMap["SumField"][m++])/NConversion,1);
            } // end for j
        } // end for i

    } else if (selectedVariable == "Total Fishery Yield (Biomass)") {

        nmfUtils::initialize(ChartData, Forecast_NYears, 1);

        fields = {"Year","Yield"};
        queryStr = "SELECT Year, Sum(SeasYield) AS Yield FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '" +  selectedSpecies + "'" +
                  seasonStr +
                " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i < Forecast_NYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear + i);
            ChartData(i,0) = std::stod(dataMap["Yield"][i]) / scaleFactor;
        }

        m = 0;
        RowLabels.clear();
        ColumnLabels.clear();
        fields = {"Year", "SumField"};
        queryStr = "Select Year, Sum(SeasYield) As SumField FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'"
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName  = '" + selectedSpecies + "'" +
                seasonStr + " GROUP BY Year, Age";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        nmfUtils::initialize(GridData, Forecast_NYears, Nage);
        for (int i=0; i<Forecast_NYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear+i);
            for (int j=0; j<Nage; ++j) {
                if (i == 0) {
                    ColumnLabels << QString::fromStdString("Age "+std::to_string(j));
                }
                GridData(i, j) = std::stod(dataMap["SumField"][m++]) / 1000.0;
            } // end for j
        } // end for i
    } else if (selectedVariable == "Fishery Yield at Age (Biomass)") {

        nmfUtils::initialize(ChartData, Forecast_NYears, 1);

        fields = {"Year","Yield"};
        queryStr = "SELECT Year, Sum(SeasYield) AS Yield FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '" +  selectedSpecies + "'" +
                  seasonStr + ageStr + " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i < Forecast_NYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear + i);
            ChartData(i,0) = std::stod(dataMap["Yield"][i]) / scaleFactor;
        }

        m = 0;
        RowLabels.clear();
        ColumnLabels.clear();
        fields = {"Year", "SumField"};
        queryStr = "Select Year, Sum(SeasYield) As SumField FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'"
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName  = '" + selectedSpecies + "'" +
                seasonStr + " GROUP BY Year, Age";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        RowLabels.clear();
        nmfUtils::initialize(GridData, Forecast_NYears, Nage);
        for (int i=0; i<Forecast_NYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear+i);
            for (int j=0; j<Nage; ++j) {
                if (i == 0) {
                    ColumnLabels << QString::fromStdString("Age "+std::to_string(j));
                }
                GridData(i, j) = std::stod(dataMap["SumField"][m++]) / 1000.0;
            } // end for j
        } // end for i

    } else if (selectedVariable == "Fishery Mortality Rate at Age") {

        nmfUtils::initialize(ChartData, Forecast_NYears, 1);

        fields = {"Year","F"};
        queryStr = "SELECT Year, Sum(SeasF) AS F FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '" +  selectedSpecies + "'" +
                  seasonStr + ageStr +
                " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i < Forecast_NYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear + i);
            ChartData(i,0) = std::stod(dataMap["F"][i]) / scaleFactor;
        }

        m = 0;
        RowLabels.clear();
        ColumnLabels.clear();
        fields = {"Year", "SumField"};
        queryStr = "Select Year, Sum(SeasF) As SumField FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'"
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName  = '" + selectedSpecies + "'" +
                seasonStr + " GROUP BY Year, Age";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        RowLabels.clear();
        nmfUtils::initialize(GridData, Forecast_NYears, Nage);
        for (int i=0; i<Forecast_NYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear+i);
            for (int j=0; j<Nage; ++j) {
                if (i == 0) {
                    ColumnLabels << QString::fromStdString("Age "+std::to_string(j));
                }
                valStr = dataMap["SumField"][m];
                GridData(i,j) = (valStr.empty()) ? 0 : nmfUtils::round(std::stod(valStr),3);
            } // end for j
        } // end for i

    } else if (selectedVariable == "Average Recruited F") {

        // a little tricky..want it to be weighted by catch at age...
        nmfUtils::initialize(CatchData, Forecast_NYears, Nage+1);
        nmfUtils::initialize(tempData,  Forecast_NYears, Nage+1);
        nmfUtils::initialize(ChartData, Forecast_NYears, 1);

        // Go Ahead and get the annual catch data...fill with zeros where appropriate
        fields = {"Year","Catch"};
        queryStr = "SELECT Year, Sum(SeasCatch) AS Catch FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '" +  selectedSpecies + "'" +
                seasonStr +
                " GROUP By Year,Age";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        m = 0;
        for (int i = 0; i < Forecast_NYears; ++i) {
            for (int j=0; j<Nage; ++j) {
                CatchData(i,j) = std::stod(dataMap["Catch"][m++]);
            }
        }
        fields = {"Year","Age","F"};
        queryStr = "SELECT Year,Age,Sum(SeasF) AS F FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '" +  selectedSpecies + "'" +
                seasonStr + ageStr +
                " GROUP By Year,Age";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        m = 0;
        for (int i = 0; i < Forecast_NYears; ++i) {
            for (int j = SpeAge; j < Nage; ++j) {
                tempData(i,j) = nmfUtils::round(std::stod(dataMap["F"][m++]),3);
            }
        }

        for (int i = 0; i < Forecast_NYears; ++i) {
            tmpSum = 0.0;
            tmpTotCat = 0.0;
            for (int j = SpeAge; j <= Nage; ++j) {
                tmpSum    += (tempData(i,j) * CatchData(i,j));
                tmpTotCat += CatchData(i,j);
            }
            RowLabels << QString::number(Forecast_FirstYear + i);
            if (tmpTotCat == 0.0) {
                std::cout << "Error: Got denominator tmpTotCat of 0. Setting ChartData(" << i << ",0) to 0." << std::endl;
                ChartData(i,0) = 0.0;
            } else {
                ChartData(i,0) = tmpSum / tmpTotCat;
            }
        }

        nmfUtils::initialize(GridData,nYears,1);
        GridData = ChartData;

    } else if (selectedVariable == "Predation Mortality Rate") {

        nmfUtils::initialize(ChartData, Forecast_NYears, 1);

        fields = {"Year","M2"};
        queryStr = "SELECT Year, Sum(SeasM2) AS M2 FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '" +  selectedSpecies + "'" +
                  seasonStr + ageStr +
                " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i < Forecast_NYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear + i);
            ChartData(i,0) = ((dataMap["M2"][i]).empty()) ? 0 : nmfUtils::round(std::stod(dataMap["M2"][i]), 3);
        }

        nmfUtils::initialize(GridData, Forecast_NYears, 1);
        GridData = ChartData;

    } else if (selectedVariable == "Predation Mortality by Predator") {

        boost::numeric::ublas::vector<double> TotConsumed;
        boost::numeric::ublas::vector<std::string> PdList;
        std::string forecastNameStr = "";
        std::string scenarioNameStr = "";
        int NumRecords = 0;

        nmfUtils::initialize(TotConsumed, NYears);
        nmfUtils::initialize(PdList, nmfConstants::MaxNumberSpecies);
        int NumPredators; // NPd

        forecastNameStr = " AND ForeName = '" + ForecastName + "'";
        scenarioNameStr = " AND Scenario = '" + ScenarioName + "'";

        // First select total amount of prey type/age consumed by all predators
        fields = {"Year","TotCons"};
        queryStr = "SELECT Year, Sum(BMConsumed) as TotCons from ForeSuitPreyBiomass WHERE MSVPAname='" + MSVPAName + "' "
                   " and PreyName='" + selectedSpecies + "' " +
                   " and PreyAge = " + std::to_string(SpeAge) +
                   seasonStr + " GROUP BY YEAR ";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["TotCons"].size() == 0) {
            std::cout << "No predation on Age = " << SpeAge << " " << selectedSpecies << std::endl;
            nmfUtils::initialize(ChartData, 0, 0);
            // RSK make this a popup!!
            return;
        }
        for (int i=0; i<Forecast_NYears; ++i) {
            TotConsumed(i) = std::stod(dataMap["TotCons"][i]);
        }

        // Next get the list of unique predators for your prey type and age class
        fields = {"PredName"};
        queryStr = "SELECT DISTINCT(PredName) from ForeSuitPreyBiomass WHERE MSVPAname='" + MSVPAName + "' " +
                   forecastNameStr +
                   scenarioNameStr +
                   " and PreyName='" + selectedSpecies + "' " +
                   " and PreyAge = " + std::to_string(SpeAge) +
                   seasonStr + " ORDER BY PredName ";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        NumPredators = dataMap["PredName"].size();
        ColumnLabels.clear();
        for (int i=0; i<NumPredators; ++i) {
            PdList(i) = dataMap["PredName"][i];
            ColumnLabels << QString::fromStdString(PdList(i));
        }

        nmfUtils::initialize(ChartData, Forecast_NYears, NumPredators);

        // Then get BM consumed by predator and calculate proportaion of mortality = bm pred / tot bm
        for (int i=0; i<NumPredators; ++i) {
            fields = {"Year","TotCons"};
            queryStr = "SELECT Year, Sum(BMConsumed) as TotCons from ForeSuitPreyBiomass WHERE MSVPAname='" + MSVPAName + "' "
                       " and PreyName='" + selectedSpecies + "' " +
                       " and PreyAge = " + std::to_string(SpeAge) +
                       " and PredName = '" + PdList(i) + "' " +
                       seasonStr + " GROUP BY YEAR ";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            NumRecords = dataMap["TotCons"].size();
            if (Forecast_NYears != int(dataMap["TotCons"].size())) {
                std::cout << queryStr << std::endl;
                std::cout << "Warning: NYears (" << Forecast_NYears << ") not equal to number of records from above query ("
                          << dataMap["TotCons"].size() << ").  Re-run MSVPA configuration." << std::endl;
            }
            for (int j=0; j<NumRecords; ++j) {
                ChartData(j,i) = std::stod(dataMap["TotCons"][j])/TotConsumed(j);
            }
        } // end for i

        // Last get annual M2...then multiply
        fields = {"Year","M2"};
        queryStr = "SELECT Year, Sum(SeasM2) as M2 from ForeOutput WHERE MSVPAname='" + MSVPAName + "' " +
                   forecastNameStr +
                   scenarioNameStr +
                   " and SpeName = '" + selectedSpecies + "' " +
                   ageStr + seasonStr + " GROUP BY YEAR ";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i=0; i<Forecast_NYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear + i);
            for (int j=0; j<NumPredators; ++j) {
                ChartData(i,j) *= std::stod(dataMap["M2"][i]);
            }
        }

        nmfUtils::initialize(GridData, Forecast_NYears, NumPredators);
        GridData = ChartData;

    } else if (selectedVariable == "Relative Food Availability") {
        boost::numeric::ublas::vector<double> tempData;
        RowLabels.clear();

        firstYear = Forecast_FirstYear;
        nYears    = Forecast_NYears;

        // First we are going to get tot suit bm and average across years
        nmfUtils::initialize(tempData,  nYears);
        nmfUtils::initialize(ChartData, nYears, 1);
        nmfUtils::initialize(GridData,  nYears, 1);

        fields = {"Year","TotFA"};
        queryStr = "SELECT Year, Sum(SuitPreyBiomass) AS TotFA FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                   " AND ForeName = '" + ForecastName + "'" +
                   " AND Scenario = '" + ScenarioName + "'" +
                   " AND PredName = '" + selectedSpecies + "'" +
                     ageStr + seasonStr +
                   " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i < nYears; ++i) {
            tempData(i) = std::stod(dataMap["TotFA"][i]);
            AvgFA += tempData(i);
        }

        AvgFA /= double(nYears + 1.0);

        // Convert this to a relative amount
        for (int i = 0; i < nYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear + i);
            ChartData(i,0) = tempData(i) / AvgFA;
        }
        GridData = ChartData;

    } else if (selectedVariable == "Per Capita Food Availability") {

        boost::numeric::ublas::vector<double> tempData;
        boost::numeric::ublas::vector<double> TotBM;

        std::string newAgeStr = ageStr;
        int PredAgeVal = (! newAgeStr.empty()) ? std::stoi(newAgeStr.erase(0,newAgeStr.find("=")+1)) : 0;
        fields  = {"Year", "Biomass"};
        fields2 = {"Year", "TotFA"};

        firstYear = Forecast_FirstYear;
        nYears    = Forecast_NYears;

        queryStr = "SELECT Year, Avg(InitBiomass) AS Biomass FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '"  + selectedSpecies + "'" +
                " AND Age = " + std::to_string(PredAgeVal) +
                  seasonStr +
                " GROUP By Year";
        queryStr2 = "SELECT Year, Sum(SuitPreyBiomass) AS TotFA FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND PredName = '" + selectedSpecies + "'" +
                  ageStr + seasonStr +
                " GROUP By Year";

        nmfUtils::initialize(tempData,  nYears);
        nmfUtils::initialize(ChartData, nYears, 1);
        nmfUtils::initialize(GridData,  nYears, 1);
        nmfUtils::initialize(TotBM,     nYears);

        // Get total biomass for predator/age class..in this case its the average of the seasonal biomasses
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (int i = 0; i < nYears; ++i) {
            TotBM(i) = std::stod(dataMap["Biomass"][i]);
        }

        // Then get and calculate food availability stuff
        dataMap2 = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        for (int i = 0; i < nYears; ++i) {
            tempData(i) = std::stod(dataMap2["TotFA"][i]) / TotBM(i);
            AvgFA += tempData(i);
        }
        AvgFA /= (nYears + 1.0);

        // Convert this to a relative amount
        for (int i = 0; i < nYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear + i);
            ChartData(i,0) = tempData(i) / AvgFA;
        }
        GridData = ChartData;

    } else if (selectedVariable == "Stomach Content Weight") {

        RowLabels.clear();

        firstYear = Forecast_FirstYear;
        nYears    = Forecast_NYears;
        nmfUtils::initialize(ChartData, nYears, 1);
        nmfUtils::initialize(GridData,  nYears, 1);

        fields = {"Year","Stom"};
        queryStr = "SELECT Year, Avg(StomCont) as Stom FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '"  + selectedSpecies + "'" +
                ageStr + seasonStr +
                " Group By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i < nYears; ++i) {
            RowLabels << QString::number(firstYear + i);
            ChartData(i,0) = std::stod(dataMap["Stom"][i]);
        }
        GridData = ChartData;

    } else if (selectedVariable == "Food Availability by Prey Type") {

        std::string str1,str2;
        boost::numeric::ublas::vector<double> tempData;

        nmfUtils::split(selectedSpeciesAgeSizeClass," ",str1,str2);
        PredAgeVal = (str1 == "Size") ? std::stoi(str2)-1 : std::stoi(str2);
        PredAgeStr = " AND PredAge = " + std::to_string(PredAgeVal);

        nmfUtils::initialize(tempData,  nYears);
        nmfUtils::initialize(ChartData, nYears, NPrey);
        nmfUtils::initialize(GridData,  nYears, NPrey);

        // Then want to get total suitable biomass and calculate average as previously
        fields = {"Year","TotFA"};
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        for (int i = 0; i < nYears; ++i) {
            tempData(i) = std::stod(dataMap["TotFA"][i]);
            AvgFA += tempData(i);
        }
        AvgFA /= (nYears + 1.0);

        // Then want to get suitable prey biomass by prey species
        fields2  = {"Year","PreyName","SBM"};
        dataMap2 = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        m = 0;
        for (auto i = 0; i < nYears; ++i) {
            RowLabels << QString::number(firstYear + i);
            for (auto j = 0; j < NPrey; ++j) {
                if (dataMap2["PreyName"][m] == ColumnLabels[j].toStdString()) {
                    ChartData(i,j) = std::stod(dataMap2["SBM"][m++]) / AvgFA;
                } else {
                    ChartData(i,j) = 0.0;
                }
            }
        }
        GridData = ChartData;

    } else if (selectedVariable == "Food Availability by Prey Age") {

        RowLabels.clear();
        ColumnLabels.clear();
        std::string PreyName = selectedPreyName;
        boost::numeric::ublas::vector<double> PreyAge;
        nmfUtils::initialize(PreyAge,  nmfConstants::MaxNumberAges);
        boost::numeric::ublas::vector<double> tempData;
        PredAgeStr = "";

        fields  = {"PreyAge"};
        fields2 = {"Year","TotFA"};
        fields3 = {"Year","PreyAge","SBM"};

        nYears    = Forecast_NYears;
        firstYear = Forecast_FirstYear;

        // First get a list of ages for the prey species...might only be one if it's another prey...
        queryStr = "SELECT DISTINCT PreyAge FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND PredName = '" + selectedSpecies + "'" +
                PredAgeStr + seasonStr +
                " AND PreyName = '" + PreyName + "'" +
                " ORDER By PreyAge";
        // Then want to get total suitable biomass and calculate average as previously
        queryStr2 = "SELECT Year, Sum(SuitPreyBiomass) AS TotFA FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND PredName = '" + selectedSpecies + "'" +
                PredAgeStr + seasonStr +
                " GROUP By Year";
        // Then want to get suitable prey biomass by prey age class
        queryStr3 = "SELECT Year, PreyAge, Sum(SuitPreyBiomass) AS SBM FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND PredName = '" + selectedSpecies + "'" +
                PredAgeStr + seasonStr +
                " AND PreyName = '" + PreyName + "'" +
                " GROUP By Year, PreyAge";

        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        NPreyAge = dataMap["PreyAge"].size();

        nmfUtils::initialize(tempData,  nYears);
        nmfUtils::initialize(ChartData, nYears, NPreyAge);
        nmfUtils::initialize(GridData,  nYears, NPreyAge);

        dataMap2 = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        for (int i = 0; i < nYears; ++i) {
            tempData(i) = std::stod(dataMap2["TotFA"][i]);
            AvgFA += tempData(i);
        }
        AvgFA /= (nYears + 1.0);

        dataMap3 = databasePtr->nmfQueryDatabase(queryStr3, fields3);
        m = 0;
        for (int i = 0; i < nYears; ++i) {
            RowLabels << QString::number(Forecast_FirstYear + i);
            for (unsigned j = 0; j < NPreyAge; ++j) {
                if (i == 0) {
                    ColumnLabels << QString::fromStdString("Age "+std::to_string(j));
                }
                if (m < int(dataMap3["SBM"].size())) {
                    if (std::stoi(dataMap3["PreyAge"][j]) == int(j)) {
                        ChartData(i,j) = std::stod(dataMap3["SBM"][m++]) / AvgFA;
                    }
                }
            } // end for j
        } // end for i
        GridData = ChartData;

    } else if (selectedVariable == "Total Biomass Consumed") {

        nYears    = Forecast_NYears;
        firstYear = Forecast_FirstYear;

        nmfUtils::initialize(ChartData, nYears, 1);

        fields = {"Year","TotCons"};
        queryStr = "SELECT Year, Sum(TotalBMConsumed) AS TotCons FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '" +  selectedSpecies + "'" +
                  seasonStr + ageStr +
                " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        RowLabels.clear();
        for (int i = 0; i < nYears; ++i) {
            RowLabels << QString::number(firstYear + i);
            ChartData(i,0) = std::stod(dataMap["TotCons"][i])/1000.0; // Replace 1000 with a variable
        }

    } else if (selectedVariable == "Consumption by Predator Age") {

        std::string newSelectedSpeciesAgeSizeClass = selectedSpeciesAgeSizeClass;
        int ageDiff = SpeAge-offset;
        if (! newSelectedSpeciesAgeSizeClass.empty()) {
            offset = (newSelectedSpeciesAgeSizeClass.find("Size") != std::string::npos) ? 1 : 0;
            newSelectedSpeciesAgeSizeClass.erase(0,newSelectedSpeciesAgeSizeClass.find(" "));
            ageDiff = std::stoi(newSelectedSpeciesAgeSizeClass) - offset;
        }

        nYears    = Forecast_NYears;
        firstYear = Forecast_FirstYear;

        nmfUtils::initialize(ChartData, nYears, 1);

        fields = {"Year","TotCons"};
        queryStr = "SELECT Year, Sum(TotalBMConsumed) AS TotCons FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND SpeName = '" +  selectedSpecies + "'" +
                  seasonStr + " and Age = " + std::to_string(ageDiff) +
                " GROUP By Year";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

        RowLabels.clear();
        for (int i = 0; i < nYears; ++i) {
            RowLabels << QString::number(firstYear + i);
            ChartData(i,0) = std::stod(dataMap["TotCons"][i])/1000.0; // Replace 1000 with a variable
        }

        // Calculate Grid Data
        nmfUtils::initialize(GridData, nYears, Nage);
        ColumnLabels.clear();
        for (int j=0; j<Nage; ++j) {
            ColumnLabels << "Age "+QString::number(j);
        }
        m = 0;
        fields = {"Year", "TotCons"};
        queryStr = "Select Year, Sum(BMConsumed) As TotCons FROM MSVPASuitPreyBiomass WHERE MSVPAname = '" + MSVPAName +
                   "' AND PredName = '" + selectedSpecies + "'" +
                   seasonStr + " GROUP BY Year,PredAge";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (int i=0; i<nYears; ++i) {
            for (int j=0; j<Nage; ++j) {
                GridData(i,j) = nmfUtils::round(std::stod(dataMap["TotCons"][m++]),1)/1000.0;
            } // end for j
        } // end for i

    } else if ((selectedVariable == "Consumption by Prey Type") ||
               (selectedVariable == "Cons. by Pred. Age and Prey Type")) {  // This should not be a Stacked Bar

        bool inclPredAge = (selectedVariable == "Cons. by Pred. Age and Prey Type");
        std::string predAgeStr = inclPredAge ? ageStr : "";

        fields = {"Year","PreyName","TotCons"};

        nYears    = Forecast_NYears;
        firstYear = Forecast_FirstYear;

        // First need to get a list of prey names and a count of prey for the predator.
        databasePtr->nmfQueryForecastPreyList(MSVPAName, ForecastName, ScenarioName,
                                              selectedSpecies, inclPredAge, ageStr, "", PreyList);
        queryStr = "SELECT Year, PreyName, Sum(BMConsumed) as TotCons FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND PredName = '" + selectedSpecies + "'" +
                seasonStr + predAgeStr +
                " GROUP BY Year, PreyName";

        NPrey = PreyList.size();
        nmfUtils::initialize(ChartData, nYears, NPrey);
        //nmfUtils::initialize(GridData,  nYears, NPrey);
        ColumnLabels.clear();
        for (int i=0; i<NPrey; ++i) {
            ColumnLabels << PreyList[i];
        }
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        m = 0;
        for (int i = 0; i< nYears; ++i) {
            RowLabels << QString::number(firstYear + i);
            for (int j = 0; j < NPrey; ++j) {
                // Need this check since there may be data for some missing species
                if (dataMap["PreyName"][m] == PreyList[j].toStdString()) {
                    ChartData(i,j) = std::stod(dataMap["TotCons"][m++]) / 1000.0;
                }
                //GridData(i,j) = ChartData(i,j);
            }
        }

    } else if (selectedVariable == "Consumption by Prey Age") {

        std::string newAgeStr = ageStr;
        int PredAgeVal = (! newAgeStr.empty()) ? std::stoi(newAgeStr.erase(0,newAgeStr.find("=")+1)) : 0;

        boost::numeric::ublas::vector<int> PreyAge;
        nmfUtils::initialize(PreyAge, nmfConstants::MaxNumberAges);

        // First get a list of ages for the prey species...might only be one if it's another prey...
        fields  = {"PreyAge"};
        fields2 = {"Year", "PreyAge", "TotCons"};

        nYears    = Forecast_NYears;
        firstYear = Forecast_FirstYear;

        queryStr = "SELECT DISTINCT(PreyAge) FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND PredName = '" + selectedSpecies + "'" +
                " AND PredAge = "   + std::to_string(PredAgeVal) +
                " AND PreyName = '" + selectedPreyName + "'" + seasonStr +
                " ORDER BY PreyAge";
        // Then select the biomass consumed of that prey age by predator age across the byvar
        queryStr2 = "SELECT Year, PreyAge, Sum(BMConsumed) as TotCons FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND Scenario = '" + ScenarioName + "'" +
                " AND PredName = '" + selectedSpecies + "'" +
                " AND PredAge = "   + std::to_string(PredAgeVal) +
                " AND PreyName = '" + selectedPreyName + "'" + seasonStr +
                " GROUP By Year, PreyAge";

        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        NPreyAge = dataMap["PreyAge"].size();
        ColumnLabels.clear();
//std::cout << "hhere: " << queryStr << ", NPreyAge: " << NPreyAge << std::endl;
        for (unsigned i = 0; i < NPreyAge; ++i) {
            PreyAge(i) = std::stoi(dataMap["PreyAge"][i]);
            ColumnLabels << QString::fromStdString("Age " + std::to_string(PreyAge(i)));
        }

        nmfUtils::initialize(ChartData, nYears, NPreyAge);
        nmfUtils::initialize(GridData,  nYears, NPreyAge);
        dataMap2 = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        m = 0;
        NumRecords = dataMap2["PreyAge"].size();
        for (int i = 0; i < nYears; ++i) {
            RowLabels << QString::number(firstYear + i);
            for (unsigned j = 0; j < NPreyAge; ++j) {
                if (m < NumRecords) {
                    if (std::stod(dataMap2["PreyAge"][m]) == PreyAge(j)) {
                        ChartData(i,j) = std::stod(dataMap2["TotCons"][m]) / 1000.0; // RSK - use a constant here
                        GridData(i,j) = ChartData(i,j);
                        ++m;
                    }
                } else {
                    break;
                }
            }
        }

    } else if (selectedVariable == "Wt. Specific Consumption") {

        // weight specific consumption rates..load params and calculate for a particular season
        // include age class in the grid matrix, but plot for a single age.


        std::string newSelectedSpeciesAgeSizeClass = selectedSpeciesAgeSizeClass;
        //int ageDiff = SpeAge-offset;
        if (! newSelectedSpeciesAgeSizeClass.empty()) {
            offset = (newSelectedSpeciesAgeSizeClass.find("Size") != std::string::npos) ? 1 : 0;
            newSelectedSpeciesAgeSizeClass.erase(0,newSelectedSpeciesAgeSizeClass.find(" "));
            //ageDiff = std::stoi(newSelectedSpeciesAgeSizeClass) - offset;
        }




         boost::numeric::ublas::matrix<double> AgeStomCont;
         boost::numeric::ublas::matrix<double> AgeConsump;
         boost::numeric::ublas::vector<double> SeasTemp;
         boost::numeric::ublas::vector<int>    PredType;

         nYears    = Forecast_NYears;
         firstYear = Forecast_FirstYear;

         queryStr2 = "SELECT Year, Age, StomCont FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND ForeName = '" + ForecastName + "'" +
                    " AND Scenario = '" + ScenarioName + "'" +
                    " AND SpeName = '"  + selectedSpecies +  "'" +
                      seasonStr +
                    " ORDER By Year, Age";

         nmfUtils::initialize(AgeStomCont, nYears, Nage+1);
         nmfUtils::initialize(AgeConsump,  Nage+1,  2);
         nmfUtils::initialize(SeasTemp,    nYears);
         nmfUtils::initialize(ChartData,   nYears, 1);
         nmfUtils::initialize(GridData,    nYears, Nage);
         //nmfUtils::initialize(PredType,    nmfConstants::MaxNumberSpecies);

         // Load predator name and age lists for use later
         QStringList PredType0;
         fields = {"SpeName"};
         queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "' AND Type = 0";
         dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
         int NPreds = dataMap["SpeName"].size();
         for (int i = 0; i < NPreds; ++i) {
             PredType0 << QString::fromStdString(dataMap["SpeName"][i]);
             //PredType(i) = 0;
         } // end for i

//         // Load Other Predator Names and Ages
//         fields = {"SpeName"};
//         queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "' AND Type = 3";
//         dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
//         int NOthPreds = dataMap["SpeName"].size();
//         for (int i = 0; i < NOthPreds; ++i) {
//             PredType(i+NPreds) = 1;
//         } // end for i

         fields2 = {"Year","Age","StomCont"};
         dataMap2 = databasePtr->nmfQueryDatabase(queryStr2, fields2);
         m = 0;
         for (int i = 0; i < nYears; ++i) {
             for (int j = 0; j < Nage; ++j) {
                 AgeStomCont(i,j) = std::stod(dataMap2["StomCont"][m++]);
             }
         }

         //if (PredType(selectedSpeciesNum) == 0) {
         if (PredType0.contains(QString::fromStdString(selectedSpecies))) {
             fields = {"Age","EvacAlpha","EvacBeta"};
             queryStr = "SELECT Age, EvacAlpha, EvacBeta FROM MSVPASizePref WHERE MSVPAname = '" + MSVPAName + "'" +
                        " AND SpeName = '" + selectedSpecies + "'" +
                        " ORDER By Age";
             dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
             for (int j = 0; j < Nage; ++j) {
                 AgeConsump(j,0) = std::stod(dataMap["EvacAlpha"][j]);
                 AgeConsump(j,1) = std::stod(dataMap["EvacBeta"][j]);
             }
         } else {
             fields = {"SizeCat","ConsAlpha","ConsBeta"};
             queryStr = "SELECT SizeCat, ConsAlpha, ConsBeta FROM OthPredSizeData WHERE SpeName = '" + selectedSpecies + "'" +
                        " ORDER BY SizeCat";
             dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
             for (int j = 0; j < Nage; ++j) {
                 AgeConsump(j, 0) = std::stod(dataMap["ConsAlpha"][j]);
                 AgeConsump(j, 1) = std::stod(dataMap["ConsBeta"][j]);
             }
         }

         fields = {"Year","Season","Value"};
         std::string newSeasonStr = seasonStr;
         int SeasonVal = (! newSeasonStr.empty()) ? std::stoi(newSeasonStr.erase(0,newSeasonStr.find("=")+1))+1 : 0;
         queryStr = "SELECT Year, Season, Value FROM MSVPASeasInfo WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND Season = " + std::to_string(SeasonVal) +
                    " AND Variable = 'SeasTemp' AND Year = " + std::to_string(FirstYear);
         dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
         std::string newAgeStr = ageStr;
         int PredAgeVal = (! newAgeStr.empty()) ? std::stoi(newAgeStr.erase(0,newAgeStr.find("=")+1)) : 0;

         for (int i = 0; i < nYears; ++i) {
             RowLabels << QString::number(firstYear + i);
             SeasTemp(i) = std::stod(dataMap["Value"][0]);
             ChartData(i,0) = AgeStomCont(i,PredAgeVal) * 24.0 * AgeConsump(PredAgeVal,0) *
                              std::exp( AgeConsump(PredAgeVal,1) * SeasTemp(i) );
         }

         for (int i = 0; i < nYears; ++i) {
             for (int j = 0; j < Nage; ++j) {
                 if (i == 0) {
                     ColumnLabels << "Age " + QString::number(j);
                 }
                 GridData(i,j) = AgeStomCont(i,j) * 24.0 * AgeConsump(j,0) *
                                 std::exp(AgeConsump(j,1) * SeasTemp(i));
             }
         }

    } else if (selectedVariable == "Average Recruited F") {

        nmfUtils::initialize(CatchData, NYears, Nage);
        nmfUtils::initialize(tempData,  NYears, Nage);
        nmfUtils::initialize(ChartData, NYears, 1);
        nmfUtils::initialize(GridData,  NYears, 1);

        // Need the catch data
        fields = {"MinCatAge","MaxCatAge"};
        queryStr = "SELECT MinCatAge,MaxCatAge from Species WHERE SpeName = '" + selectedSpecies + "' ";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        int FirstCatchAge = std::stoi(dataMap["MinCatAge"][0]);
        int LastCatchAge  = std::stoi(dataMap["MaxCatAge"][0]);
        fields = {"Catch"};
        queryStr = "SELECT Catch from SpeCatch WHERE SpeName = '" + selectedSpecies + "' " +
                " AND (Year >= " + std::to_string(FirstYear) +
                " AND  Year <= " + std::to_string(LastYear)  + ") " +
                " ORDER by Year, Age";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        m = 0;
        for (int i=0; i<NYears; ++i) {
            for (int j=FirstCatchAge; j<=LastCatchAge; ++j) {
                CatchData(i,j) = std::stod(dataMap["Catch"][m++]);
            }
        }

        // Get the species data
        fields = {"Year","Age","SumField"};
        queryStr = "SELECT Year, Age, Sum(SeasF) as SumField from MSVPASeasBiomass WHERE MSVPAname='" + MSVPAName + "' "                                                                                                                                     " and SpeName='" + selectedSpecies + "' " +
                ageStr + seasonStr + " GROUP BY YEAR, Age ";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        m = 0;
        for (int i=0; i<NYears; ++i) {
            for (int j=SpeAge; j<Nage; ++j) {
                tempData(i,j) = std::stod(dataMap["SumField"][m++]);
            }
        }

        // Calculate the ChartData values
        for (int i=0; i<NYears; ++i) {
            tmpSum = 0.0;
            tmpTotCat = 0.0;
            for (int j=SpeAge; j<Nage; ++j) {
                tmpSum += tempData(i,j)*CatchData(i,j);
                tmpTotCat += CatchData(i,j);
            }
            RowLabels << QString::number(FirstYear + i);
            ChartData(i,0) = tmpSum / tmpTotCat;
            GridData(i,0) = ChartData(i,0);
        }

    } else if (selectedVariable == "Predation Mortality by Predator") {

        getChartDataOfPredationMortalityByPredator(
              databasePtr,
               MSVPAName,
              "MSVPA",
              "",
              "",
              "MSVPASuitPreyBiomass",
              "MSVPASeasBiomass",
               ageStr,
               seasonStr,
               FirstYear,
               NYears,
               selectedSpecies,
               SpeAge,
               ColumnLabels,
               RowLabels,
               ChartData);
    }

}


void
ChartBar::getChartDataOfPredationMortalityByPredator(
      nmfDatabase *databasePtr,
      const std::string &MSVPAName,
      const std::string &type,
      const std::string &ForecastName,
      const std::string &ScenarioName,
      const std::string &SuitPreyBiomassTable,
      const std::string &outputTable,
      const std::string &ageStr,
      const std::string &seasonStr,
      const int &FirstYear,
      const int &NYears,
      const std::string &species,
      const int &speciesAge,
      QStringList &LegendNames,
      QStringList &categories,
      boost::numeric::ublas::matrix<double> &ChartData)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    boost::numeric::ublas::vector<double> TotConsumed;
    boost::numeric::ublas::vector<std::string> PdList;
    std::string forecastNameStr = "";
    std::string scenarioNameStr = "";
    int NumRecords = 0;

    nmfUtils::initialize(TotConsumed, NYears);
    nmfUtils::initialize(PdList, nmfConstants::MaxNumberSpecies);
    int NumPredators; // NPd

    if (type == "Forecast") {
        forecastNameStr = " AND ForeName = '" + ForecastName + "'";
        scenarioNameStr = " AND Scenario = '" + ScenarioName + "'";
    }

    // First select total amount of prey type/age consumed by all predators
    fields = {"Year","TotCons"};
    queryStr = "SELECT Year, Sum(BMConsumed) as TotCons from " + SuitPreyBiomassTable + " WHERE MSVPAname='" + MSVPAName + "' "
               " and PreyName='" + species + "' " +
               " and PreyAge = " + std::to_string(speciesAge) +
               seasonStr + " GROUP BY YEAR ";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap.size() == 0) {
        return;
    }
    if (dataMap["TotCons"].size() == 0) {
        std::cout << "No predation on Age = " << speciesAge << " " << species << std::endl;
        nmfUtils::initialize(ChartData, 0, 0);
        // RSK make this a popup!!
        return;
    }
    for (int i=0; i<NYears; ++i) {
        TotConsumed(i) = std::stod(dataMap["TotCons"][i]);
    }

    // Next get the list of unique predators for your prey type and age class
    fields = {"PredName"};
    queryStr = "SELECT DISTINCT(PredName) from " + SuitPreyBiomassTable + " WHERE MSVPAname='" + MSVPAName + "' " +
               forecastNameStr +
               scenarioNameStr +
               " and PreyName='" + species + "' " +
               " and PreyAge = " + std::to_string(speciesAge) +
               seasonStr + " ORDER BY PredName ";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap.size() == 0) {
        return;
    }
    NumPredators = dataMap["PredName"].size();
    LegendNames.clear();
    for (int i=0; i<NumPredators; ++i) {
        PdList(i) = dataMap["PredName"][i];
        LegendNames << QString::fromStdString(PdList(i));
    }

    nmfUtils::initialize(ChartData, NYears, NumPredators);

    // Then get BM consumed by predator and calculate proportaion of mortality = bm pred / tot bm
    for (int i=0; i<NumPredators; ++i) {
        fields = {"Year","TotCons"};
        queryStr = "SELECT Year, Sum(BMConsumed) as TotCons from " + SuitPreyBiomassTable + " WHERE MSVPAname='" + MSVPAName + "' "
                   " and PreyName='" + species + "' " +
                   " and PreyAge = " + std::to_string(speciesAge) +
                   " and PredName = '" + PdList(i) + "' " +
                   seasonStr + " GROUP BY YEAR ";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap.size() > 0) {
            NumRecords = dataMap["TotCons"].size();
            if (NYears != int(dataMap["TotCons"].size())) {
                std::cout << queryStr << std::endl;
                std::cout << "Warning: NYears (" << NYears << ") not equal to number of records from above query ("
                          << dataMap["TotCons"].size() << ").  Re-run MSVPA configuration." << std::endl;
            }
            for (int j=0; j<NumRecords; ++j) {
                ChartData(j,i) = std::stod(dataMap["TotCons"][j])/TotConsumed(j);
            }
        }
    } // end for i

    // Last get annual M2...then multiply
    fields = {"Year","M2"};
    queryStr = "SELECT Year, Sum(SeasM2) as M2 from  " + outputTable + " WHERE MSVPAname='" + MSVPAName + "' " +
               forecastNameStr +
               scenarioNameStr +
               " and SpeName = '" + species + "' " +
               ageStr + seasonStr + " GROUP BY YEAR ";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap.size() > 0) {
        for (int i=0; i<NYears; ++i) {
            categories << QString::number(FirstYear + i);
            for (int j=0; j<NumPredators; ++j) {
                ChartData(i,j) *= std::stod(dataMap["M2"][i]);
            }
        }
    }
} // end getChartDataOfPredationMortalityByPredator



void
ChartBar::loadChartWithData(
        QChart *chart,
        QBarSeries *series,
        boost::numeric::ublas::matrix<double> &ChartData,
        QStringList &LegendNames,
        const bool &useMaxValue,
        const std::string &MaxScaleY,
        const bool &inThousands)
{
    std::string msg;
    QBarSet *newSet;

    series = new QBarSeries();

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
            *newSet << ChartData(val,i); //*scaleFactor;;
        }
        series->append(newSet);
    }
    chart->addSeries(series);

    chart->legend()->setVisible(showLegend);

    QValueAxis *axisY = new QValueAxis();
    //chart->setAxisY(axisY, series);
    nmfUtilsQt::setAxisY(chart,axisY,series);

    if (! MaxScaleY.empty()) {
        double maxScaleY = std::stod(MaxScaleY);
        if (maxScaleY > 0.0)
            axisY->setRange(0, maxScaleY);
    } else {
        axisY->applyNiceNumbers();
    }

} // end loadChartWithData


void
ChartBar::loadChartWithData(
        QChart *chart,
        QStackedBarSeries *series,
        boost::numeric::ublas::matrix<double> &ChartData,
        QStringList &LegendNames,
        const bool &useMaxValue,
        const std::string &MaxScaleY,
        const bool &inThousands)
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
            *newSet << ChartData(val,i); //*scaleFactor;;
        }
        series->append(newSet);
    }
    chart->addSeries(series);
    chart->legend()->setVisible(showLegend);

    QValueAxis *axisY = new QValueAxis();
    //chart->setAxisY(axisY, series);
    nmfUtilsQt::setAxisY(chart,axisY,series);

    if (! MaxScaleY.empty()) {
        double maxScaleY = std::stod(MaxScaleY);
        if (maxScaleY > 0.0)
            axisY->setRange(0, maxScaleY);
    } else {
        axisY->applyNiceNumbers();
    }

} // end loadChartWithData



void
ChartBar::setTitles(
        QChart *chart,
        QBarSeries *series,
        QStringList categories,
        std::string agePrefix,
        std::string species,
        std::string main,
        std::string sub,
        std::string xLabel,
        std::string yLabel,
        bool rearrangeTitle,
        bool showLegend,
        int Theme)
{
    if (agePrefix[agePrefix.size()-1] == '+')
        agePrefix += " ";
    else if (agePrefix != "")
        agePrefix += " - ";

    if (! sub.empty()) {
        sub = ": " + sub;
    }

    chart->setTheme(static_cast<QChart::ChartTheme>(Theme));

    // Set main title font and text
    std::stringstream ss;
    QFont mainTitleFont = chart->titleFont();
    mainTitleFont.setPointSize(14);
    mainTitleFont.setWeight(QFont::Bold);
    chart->setTitleFont(mainTitleFont);
    if (rearrangeTitle) {
        ss << main << " " << agePrefix << species << sub;
    } else {
        ss << agePrefix << species << " - " << main << " " << sub;
    }
    chart->setTitle(ss.str().c_str());

    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(categories);
    chart->createDefaultAxes();
    //chart->setAxisX(axis, series);
    nmfUtilsQt::setAxisX(chart,axis,series);
    chart->legend()->setVisible(showLegend);
    chart->legend()->setAlignment(Qt::AlignRight);

    // Set font and labels of x axis
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
    axisX->setTitleVisible(true);

    // Set font and labels of y axis
    QAbstractAxis *axisY = chart->axes(Qt::Vertical).back();
    axisY->setTitleFont(titleFont);
    axisY->setTitleText(yLabel.c_str());

} // end setTitles

//setTitles(chart, series, RowLabels,
//          newAgeStr, selectedSpecies,
//          title, titleSuffix, "Year",
//          newYLabel, rearrangeTitle, ChartData.size2()>1, Theme);

void
ChartBar::setTitles(
        QChart *chart,
        QStackedBarSeries *series,
        QStringList categories,
        std::string agePrefix,
        std::string species,
        std::string main,
        std::string sub,
        std::string xLabel,
        std::string yLabel,
        bool rearrangeTitle,
        bool showLegend,
        int Theme)
{
    if (agePrefix[agePrefix.size()-1] == '+')
        agePrefix += " ";
    else if (agePrefix != "")
        agePrefix += " - ";

    if (! sub.empty()) {
        sub = ": " + sub;
    }

    chart->setTheme(static_cast<QChart::ChartTheme>(Theme));

    // Set main title font and text
    std::stringstream ss;
    QFont mainTitleFont = chart->titleFont();
    mainTitleFont.setPointSize(14);
    mainTitleFont.setWeight(QFont::Bold);
    chart->setTitleFont(mainTitleFont);
    if (rearrangeTitle) {
        ss << main << " " << agePrefix << species << sub;
    } else {
        ss << agePrefix << species << " - " << main << " " << sub;
    }
    chart->setTitle(ss.str().c_str());

    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(categories);
    chart->createDefaultAxes();
    //chart->setAxisX(axis, series);
    nmfUtilsQt::setAxisX(chart,axis,series);
    chart->legend()->setVisible(showLegend);
    chart->legend()->setAlignment(Qt::AlignRight);

    // Set font and labels of x axis
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
    axisX->setTitleVisible(true);

    // Set font and labels of y axis
    QAbstractAxis *axisY = chart->axes(Qt::Vertical).back();
    axisY->setTitleFont(titleFont);
    axisY->setTitleText(yLabel.c_str());

} // end setTitles


std::string
ChartBar::constructMainTitle(
        const bool        &rearrangeTitle,
        const std::string &main,
        const std::string &agePrefix,
        const std::string &species,
        const std::string &sub)
{
    std::stringstream ss;

    if (rearrangeTitle) {
        ss << main << " " << agePrefix << species << sub;
    } else {
        ss << agePrefix << species << " - " << main << " " << sub;
    }

    return ss.str();
}


void
ChartBar::populateChart(
        QChart *chart,
        std::string &type,
        const boost::numeric::ublas::matrix<double> &ChartData,
        const QStringList &RowLabels,
        const QStringList &ColumnLabels,
        std::string &MainTitle,
        std::string &XTitle,
        std::string &YTitle,
        const std::vector<bool> &GridLines,
        const int Theme)
{
    QBarSet *newSet;
    QBarSeries *series = nullptr;

    // Set current theme
    chart->setTheme(static_cast<QChart::ChartTheme>(Theme));
    chart->removeAllSeries();

    if (type == "StackedBar") {
        QStackedBarSeries *series = new QStackedBarSeries();

        // Load data into series and then add series to the chart
        for (unsigned int i=0; i<ChartData.size2(); ++i) {
            if (ColumnLabels.size() == int(ChartData.size2())) {
                newSet = new QBarSet((ColumnLabels[i]));
            } else {
                newSet = new QBarSet("");
            }
            for (unsigned int val=0; val<ChartData.size1(); ++val) {
                *newSet << ChartData(val,i);
            }
            series->append(newSet);
        }
        chart->addSeries(series);

    } else if (type == "Line") {

        // Load data into series and then add series to the chart
        QLineSeries *series;
        for (unsigned int line=0; line<ChartData.size2(); ++line) {
            series = new QLineSeries();
            for (unsigned int j=0; j<ChartData.size1(); ++j) {
                series->append(j+1,ChartData(j,line));
            }
            chart->addSeries(series);
            series->setName(ColumnLabels[line]);

        }

    } else if (type == "Bar") {
        QBarSeries *series = new QBarSeries();

        // Load data into series and then add series to the chart
        for (int i=0; i<int(ChartData.size2()); ++i) {
            if (ColumnLabels.size() == int(ChartData.size2()))
                newSet = new QBarSet((ColumnLabels[i]));
            else
                newSet = new QBarSet("");
            for (unsigned int j=0; j<ChartData.size1(); ++j) {
                *newSet << ChartData(j,i);
            }
            series->append(newSet);
        }
        chart->addSeries(series);
    }
    // Set main title
    //QStackedBarSeries *series = NULL; // ???
    QFont mainTitleFont = chart->titleFont();
    mainTitleFont.setPointSize(14);
    mainTitleFont.setWeight(QFont::Bold);
    chart->setTitleFont(mainTitleFont);
    chart->setTitle(QString::fromStdString(MainTitle));

    // Setup X and Y axes
    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(RowLabels);
    chart->createDefaultAxes();

    //chart->setAxisX(axis, NULL); // RSK - check this
    nmfUtilsQt::setAxisX(chart,axis,new QBarSeries());

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    QAbstractAxis *axisX = chart->axes(Qt::Horizontal).back();
    QFont titleFont = axisX->titleFont();
    titleFont.setPointSize(12);
    titleFont.setWeight(QFont::Bold);
    axisX->setTitleFont(titleFont);
    axisX->setTitleText(QString::fromStdString(XTitle));
    if (RowLabels.count() > nmfConstants::NumCategoriesForVerticalNotation)
        axis->setLabelsAngle(-90);
    else
        axis->setLabelsAngle(0);

    // Rescale vertical axis....0 to 1 in increments of 0.2
    if (type == "StackedBar") {
        QValueAxis *newAxisY = new QValueAxis();
        //newAxisY->setTitleFont(titleFont);
        //newAxisY->setTitleText(QString::fromStdString(YTitle));
        newAxisY->setRange(0,1.0);
        newAxisY->setTickCount(6);
        //newAxisY->applyNiceNumbers();
        //chart->setAxisY(newAxisY,series);
        nmfUtilsQt::setAxisY(chart,newAxisY,series);
    }

    QValueAxis *currentAxisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).back());
    currentAxisY->setTitleFont(titleFont);
    currentAxisY->setTitleText(QString::fromStdString(YTitle));
    currentAxisY->applyNiceNumbers();

    // Set grid line visibility
    chart->axes(Qt::Horizontal).back()->setGridLineVisible(GridLines[0]);
    chart->axes(Qt::Vertical).back()->setGridLineVisible(GridLines[1]);
}



