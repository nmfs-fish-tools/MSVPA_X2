/**
 @file nmfSSVPA.cpp
 @author rklasky
 @copyright 2017 NOAA - National Marine Fisheries Service
 @brief This file contains the MSVPA API implementations.
 @date Nov 28, 2016
*/
#define DEBUG 1
#ifndef DEBUG
//#define DEBUG_MSG(str) do {std::cout << str << std::endl;} while (false)
#define DEBUG_MSG(str) do {std::cout << str;} while (false)
#else
#define DEBUG_MSG(str) do {} while (false)
#endif

#include <iostream>
#include <fstream>

#include <QSettings>
#include <QThread>

#include "nmfDatabase.h"

#include "nmfSSVPA.h"
#include "nmfMSVPA.h"
#include "nmfUtils.h"
#include "nmfUtilsStatistics.h"
#include "nmfConstants.h"
#include "nmfLogger.h"



nmfMSVPAOthPredInfo::nmfMSVPAOthPredInfo(
        std::string SpeNameIn,
        int SpeTagIn)
{
    SpeName   = SpeNameIn;
    SpeTag    = SpeTagIn;
}


nmfMSVPAOthPreyInfo::nmfMSVPAOthPreyInfo(
            std::string SpeNameIn, double MinSizeIn, double MaxSizeIn,
            double SizeAlphaIn, double SizeBetaIn)
{
    SpeName   = SpeNameIn;
    MinSize   = MinSizeIn;
    MaxSize   = MaxSizeIn;
    SizeAlpha = SizeAlphaIn;
    SizeBeta  = SizeBetaIn;
}

nmfMSVPASpeciesInfo::nmfMSVPASpeciesInfo(std::string SpeNameIn, int SpeTypeIn, int DBSpeIndexIn,
        std::string SVPANameIn, int SVPAIndexIn)
{
    SpeName    = SpeNameIn;
    SpeType    = SpeTypeIn;
    DBSpeIndex = DBSpeIndexIn;
    SVPAName   = SVPANameIn;
    SVPAIndex  = SVPAIndexIn;
}

nmfMSVPA::nmfMSVPA(nmfLogger *theLogger)
{
    ProjectDatabase.clear();
    ProjectDir.clear();
    ProjectName.clear();

    logger = theLogger;

    ReadSettings();

}


void
nmfMSVPA::ReadSettings()
{
    QSettings* settings = nmfUtilsQt::createSettings(nmfConstantsMSVPA::SettingsDirWindows,"MSVPA_X2");

    settings->beginGroup("SetupTab");
    ProjectName     = settings->value("ProjectName","").toString().toStdString();
    ProjectDir      = settings->value("ProjectDir","").toString().toStdString();
    ProjectDatabase = settings->value("ProjectDatabase","").toString().toStdString();
    settings->endGroup();

    delete settings;
}

//bool
//nmfMSVPA::nmfGetInitialData(
//            nmfDatabase* databasePtr, std::string MSVPAName,
//            int &NPreds, int &NPrey, int &NOtherFood, int &NOtherPreds,
//            int &FirstYear, int &LastYear, int &NSeas, int &NMSVPASpe, int &NYears,
//            std::string &FirstYearStr, std::string &LastYearStr)
//{
//    bool retv=true;
//    std::vector<std::string> fields;
//    std::string queryStr;
//    std::map<std::string, std::vector<std::string> > dataMap;

//    fields = {"NSpe","NPreyOnly","NOther","NOtherPred","FirstYear","LastYear","NSeasons"};
//    queryStr = "SELECT Nspe,NPreyOnly,NOther,NOtherPred,FirstYear,LastYear,NSeasons FROM " + nmfConstantsMSVPA::TableMSVPAlist +
//               " WHERE MSVPAname = '" + MSVPAName + "'";
//    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

//    NPreds = std::stoi(dataMap["NSpe"][0]);
//    NPrey = std::stoi(dataMap["NPreyOnly"][0]);
//    NOtherFood = std::stoi(dataMap["NOther"][0]);
//    NOtherPreds = std::stoi(dataMap["NOtherPred"][0]);
//    FirstYear = std::stoi(dataMap["FirstYear"][0]);
//    LastYear = std::stoi(dataMap["LastYear"][0]);
//    NSeas = std::stoi(dataMap["NSeasons"][0]);
//    NMSVPASpe = NPreds + NPrey;
//    NYears = LastYear - FirstYear + 1;
//    FirstYearStr = std::to_string(FirstYear);
//    LastYearStr = std::to_string(LastYear);

//    return retv;
//} // end nmfGetInitialData


bool
nmfMSVPA::GetSeasonalLengthTemperatureData(
        nmfDatabase* databasePtr,
        std::string MSVPAName,
        int NSeas, int NYears,
        boost::numeric::ublas::matrix<double> &SeasLen,
        boost::numeric::ublas::matrix<double> &SeasTemp)
{
    bool retv=true;
    std::vector<std::string> fields;
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;

    /*
     * Get seasonal length and temperature data
     */
    logger->logMsg(nmfConstants::Normal,"MSVPA: Getting Seasonal length and temperature data");

    fields = {"Value"};
    queryStr = "SELECT Value FROM " + nmfConstantsMSVPA::TableMSVPASeasInfo +
               " WHERE MSVPAname='"+MSVPAName+"' AND Variable='SeasLen'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    SeasLen(0,0) = 0;
    SeasLen(NSeas-1,1) = 1;
    for (int i=0; i < NSeas-1; ++i) {
         SeasLen(i, 1) = SeasLen(i, 0) + (std::stod(dataMap["Value"][i]) / 365.0);
         SeasLen(i+1, 0) = SeasLen(i, 1);
    }

    for (int i=0; i < NSeas; ++i) {
         SeasLen(i, 2) = std::stod(dataMap["Value"][i]);
    }

    fields = {"Value"};
    queryStr = "SELECT Value FROM " + nmfConstantsMSVPA::TableMSVPASeasInfo +
               " WHERE MSVPAname='" +MSVPAName +
               "' AND Variable='SeasTemp' ORDER By Year,Season";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    int k = 0;
    for (int i=0; i < NYears; ++i) {
        for (int j=0; j < NSeas; ++j) {
            SeasTemp(i,j) = std::stod(dataMap["Value"][k++]);
        } // end for j
    } // end for i

//nmfUtils::printMatrix("SeasTemp",SeasTemp,9,4);
    return retv;
} // end nmfGetSeasonalLengthTemperatureData


bool
nmfMSVPA::GetPredatorData(
            nmfDatabase* databasePtr,
            std::string MSVPAName,
            boost::numeric::ublas::vector<std::string> &MSVPAPredList)
{
    bool retv=true;
    std::vector<std::string> fields = {"SpeName"};
    std::string queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
                           " WHERE MSVPAname='"+MSVPAName+"' AND Type = 0 ORDER By SpeName";
    std::map<std::string, std::vector<std::string> > dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    int numRecords = dataMap["SpeName"].size();

    for (int i=0; i < numRecords; ++i) {
         MSVPAPredList(i) = dataMap["SpeName"][i];
    }

    return retv;
} // end nmfGetPredatorData


bool
nmfMSVPA::GetSpeciesData(
            nmfDatabase* databasePtr,
            std::string MSVPAName,
            int NMSVPASpe,
            std::vector<std::unique_ptr<nmfMSVPASpeciesInfo>> &MSVPASpeciesList)
{
    bool retv=true;

    logger->logMsg(nmfConstants::Normal,"MSVPA: Loading species information...");

    std::vector<std::string> fields = {"SpeName","Type","SpeIndex","SSVPAname","SSVPAindex"};
    std::string queryStr = "SELECT SpeName,Type,SpeIndex,SSVPAname,SSVPAindex FROM " +
                            nmfConstantsMSVPA::TableMSVPAspecies +
                           " WHERE MSVPAname='" + MSVPAName +
                           "' AND (Type = 0 or Type = 1)";
    std::map<std::string, std::vector<std::string> > dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    int numRecords = dataMap["SpeName"].size();

    for (int i=0; i < numRecords; ++i) {
        MSVPASpeciesList.push_back(std::make_unique<nmfMSVPASpeciesInfo>(
                dataMap["SpeName"][i],
                std::stoi(dataMap["Type"][i]),
                std::stoi(dataMap["SpeIndex"][i]),
                dataMap["SSVPAname"][i],
                std::stoi(dataMap["SSVPAindex"][i])));
    }

    std::vector<std::string> fields1 = {"MinCatAge","MaxCatAge","MaxAge","PlusClass","CatchUnits","wtUnits","SizeUnits"};
    std::vector<std::string> fields2 = {"Type"};
    for (int i=0; i<NMSVPASpe; ++i) {
         queryStr = "SELECT MinCatAge,MaxCatAge,MaxAge,PlusClass,CatchUnits,wtUnits,SizeUnits FROM " +
                     nmfConstantsMSVPA::TableSpecies +
                    " WHERE SpeIndex=" + std::to_string(MSVPASpeciesList[i]->DBSpeIndex);
         dataMap = databasePtr->nmfQueryDatabase(queryStr, fields1);
         MSVPASpeciesList[i]->FirstCatAge = std::stoi(dataMap["MinCatAge"][0]);
         MSVPASpeciesList[i]->LastCatAge  = std::stoi(dataMap["MaxCatAge"][0]);
         MSVPASpeciesList[i]->MaxAge      = std::stoi(dataMap["MaxAge"][0]);
         MSVPASpeciesList[i]->PlusAge     = std::stoi(dataMap["PlusClass"][0]);
         MSVPASpeciesList[i]->CatUnits    = std::stof(dataMap["CatchUnits"][0]);
         MSVPASpeciesList[i]->WtUnits     = std::stof(dataMap["wtUnits"][0]);
         MSVPASpeciesList[i]->SizeUnits   = std::stof(dataMap["SizeUnits"][0]);

         // Load single species vpa (SVPA) type
         queryStr = "SELECT Type FROM " + nmfConstantsMSVPA::TableSpeSSVPA +
                    " WHERE SpeIndex=" + std::to_string(MSVPASpeciesList[i]->DBSpeIndex) +
                    " AND SSVPAname='" + MSVPASpeciesList[i]->SVPAName + "'";
         dataMap = databasePtr->nmfQueryDatabase(queryStr, fields2);
         MSVPASpeciesList[i]->SVPAType = std::stoi(dataMap["Type"][0]);
    }

    return retv;
} // end nmfGetSpeciesData



bool
nmfMSVPA::MSVPA_PredatorGrowth(nmfDatabase* databasePtr, std::string MSVPAName)
{
std::cout << "MSVPA Growth Not Implemented Yet." << std::endl;
    int NPreds;
    int NPrey;
    int NOtherFood;
    int NOtherPreds;
    int FirstYear;
    int LastYear;
    int NSeas;
    int NMSVPASpe;
    int NYears;
    //int NumRecords;
    std::string FirstYearStr;
    std::string LastYearStr;
    //int TolCheck=0;
    //int NumLoops=0;
    //double SeasStart;
    //double SeasEnd;
    //double AvgAbund;
    std::vector<std::string> fields;
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    boost::numeric::ublas::vector<std::string> MSVPAPredList;
    boost::numeric::ublas::matrix<double> SeasLen;
    boost::numeric::ublas::matrix<double> SeasTemp;
    std::vector<std::unique_ptr<nmfMSVPAOthPreyInfo>> OthPreyList;
    std::vector<std::unique_ptr<nmfMSVPAOthPredInfo>> OthPredList;

    // Initialize vectors and matrices
    std::vector<std::unique_ptr<nmfMSVPASpeciesInfo>> MSVPASpeciesList;
    nmfUtils::initialize(SeasLen,  nmfConstants::MaxNumberSeasons, 3); // expressed as proportion of year...actually beginning and end of each
    nmfUtils::initialize(SeasTemp, nmfConstants::MaxNumberYears, nmfConstants::MaxNumberSeasons); // temperature
    nmfUtils::initialize(MSVPAPredList, nmfConstants::MaxTotalSpecies);

    // Clear tables that will store model results
    if (! databasePtr->nmfDeleteRecordsFromTable(nmfConstantsMSVPA::TableMSVPASeasBiomass,MSVPAName))
        return false;

    if (! databasePtr->nmfDeleteRecordsFromTable(nmfConstantsMSVPA::TableMSVPASuitPreyBiomass,MSVPAName)) {
        return false;
    }

    if (! databasePtr->nmfGetInitialData(MSVPAName,
                            NPreds, NPrey, NOtherFood, NOtherPreds,
                            FirstYear, LastYear, NSeas, NMSVPASpe,
                            NYears, FirstYearStr, LastYearStr)) {
        return false;
    }

    if (! GetSeasonalLengthTemperatureData(databasePtr,MSVPAName,NSeas,NYears,SeasLen,SeasTemp)) {
        return false;
    }

    if (! GetPredatorData(databasePtr,MSVPAName,MSVPAPredList)) {
        return false;
    }

    if (! GetSpeciesData(databasePtr,MSVPAName, NMSVPASpe, MSVPASpeciesList)) {
        return false;
    }

/* Compile this and then continue....

    // OK...so have the info for the MSVPA species...now get the other prey.
    fields   = {"OthPreyName","MinSize","MaxSize","SizeAlpha","SizeBeta"};
    queryStr = "SELECT OthPreyName,MinSize,MaxSize,SizeAlpha,SizeBeta FROM " +
                nmfConstantsMSVPA::TableMSVPAOthPrey +
               " WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NumRecords = dataMap["OthPreyName"].size();
    for (int i = 0; i < NumRecords; ++i) {
        OthPreyList.push_back(
             new nmfMSVPAOthPreyInfo(dataMap["OthPreyName"][i],
                                     std::stod(dataMap["MinSize"][i]),
                                     std::stod(dataMap["MaxSize"][i]),
                                     std::stod(dataMap["SizeAlpha"][i]),
                                     std::stod(dataMap["SizeBeta"][i])))
                                     //.FixBM = RecSet.Fields("FixBM");
    }

    // Then get the other predators...if there are any...
    if (NOtherPreds > 0) {
        fields = {"SpeName","SpeIndex"};
        queryStr = "SELECT SpeName,SpeIndex FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
                   " WHERE MSVPAname = '" + MSVPAname + "'" +
                   " AND Type = 3";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        NumRecords = dataMap["SpeName"].size();
        for (int i = 0; i< NumRecords; ++i) {
            OthPredList.push_back(
                        new nmfMSVPAOthPredInfo(dataMap["SpeName"],
                        std::stoi(dataMap["SpeIndex"][i])));
        }

        //Get other predator data from the table
        for (int i = 0; i < NOtherPreds; ++i) {
            fields   = {"BMUnits","MinSize","MaxSize","WtUnits","SizeUnits","SizeStruc","NumSizeCats"};
            queryStr = "SELECT BMUnits,MinSize,MaxSize,WtUnits,SizeUnits,SizeStruc,NumSizeCats FROM " +
                        nmfConstantsMSVPA::TableOtherPredSpecies +
                       " WHERE SpeName = '" + OthPredList[i].SpeName + "'";
            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);

            OthPredList[i].BMunits       = std::stod(dataMap["BMUnits"][i]);
            OthPredList[i].MinSize       = std::stoi(dataMap["MinSize"][i]);
            OthPredList[i].MaxSize       = std::stoi(dataMap["MaxSize"][i]);
            OthPredList[i].WtUnits       = std::stod(dataMap["WtUnits"][i]);
            OthPredList[i].SizeUnits     = std::stod(dataMap["SizeUnits"][i]);
            OthPredList[i].SizeStructure = std::stoi(dataMap["SizeStruc"][i]);
            OthPredList[i].NumSizeCats   = std::stoi(dataMap["NumSizeCats"][i]);

            if (OthPredList[i].NumSizeCats > 0) {
                fields = {"MinLen","MaxLen","PropBM"};
                queryStr = "SELECT MinLen,MaxLen,PropBM FROM " + nmfConstantsMSVPA::TableOthPredSizeData +
                           " WHERE SpeName = '" + OthPredList[i].SpeName + "'";
                dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
                for (int j = 0; j < OthPredList[i].NumSizeCats; ++j) {
                    OthPredList[i].SizeCatMins.push_back(std::stoi(dataMap["MinLen"][j]));
                    OthPredList[i].SizeCatMaxs.push_back(std::stoi(dataMap["MaxLen"][j]));
                    OthPredList[i].SizeCatBM.push_back(std::stod(dataMap["PropBM"][j]));
                }
            } // end if
        } // end for i
    } // end if

*/


























    return false;

} // end nmfMSVPA_PredatorGrowth



void
nmfMSVPA::WriteRunStopFile(std::string value,
                           std::string theMSVPAName,
                           std::string elapsedTime,
                           int numLoops,
                           int convergenceValue)
{
    std::ofstream outputFile(nmfConstantsMSVPA::MSVPAStopRunFile);
    outputFile << value << std::endl;
    outputFile << theMSVPAName << std::endl;
    outputFile << elapsedTime << std::endl;
    outputFile << numLoops << std::endl;
    outputFile << convergenceValue << std::endl;
    outputFile.close();

} // end WriteRunStopFile



std::string
nmfMSVPA::ReadRunStopFile()
{
    std::string retv = "";
    std::string str;

    std::ifstream inputFile(nmfConstantsMSVPA::MSVPAStopRunFile);

    if (inputFile) {
        // Read line of data from file
        std::getline(inputFile,str);
        inputFile.close();
        retv = str;
    }

    return retv;

} // end ReadRunStopFile



void
nmfMSVPA::WriteCurrentLoopFile(std::string MSVPAName,
                               int NumLoops,
                               int ConvergenceValue)
{ 
    std::ofstream outputFile(nmfConstantsMSVPA::MSVPAProgressChartFile,
                             std::ios::out|std::ios::app);
    outputFile << MSVPAName << ", "
               << NumLoops << ", "
               << ConvergenceValue << std::endl;
    outputFile.close();

    emit UpdateMSVPAProgressWidget();
    QApplication::processEvents();

} // end WriteCurrentLoopFile


void
nmfMSVPA::updateProgress(int value, std::string msg)
{
    emit UpdateMSVPAProgressDialog(value,QString::fromStdString(msg));

    // Necessary so progress GUI will refresh
    QApplication::processEvents();

    // Write progress value to output data file
    std::ofstream outputFile(nmfConstantsMSVPA::MSVPAProgressBarFile);
    outputFile << value << "; " << msg << std::endl;
    outputFile.close();

    logger->logMsg(nmfConstants::Normal,"MSVPA: "+msg);
}

//void
//nmfMSVPA::clearProgressBar()
//{
//    updateProgress(0,"");
//}

void
nmfMSVPA::outputProgressLabel(std::string msg)
{
    std::ofstream outputFile(nmfConstantsMSVPA::MSVPAProgressChartLabelFile);
    outputFile << msg << std::endl;
    outputFile.close();

} // end outputProgressLabel


nmfDatabase*
nmfMSVPA::openDatabase(std::string Username,
                       std::string Hostname,
                       std::string Password)
{
    std::string errorMsg;
    nmfDatabase* databasePtr = new nmfDatabase();
    bool openOK = databasePtr->nmfOpenDatabase(Hostname,Username,Password,errorMsg);
    if (! openOK) {
        std::cout << "nmfMSVPA Error: Couldn't open database.";
        std::cout << "nmfMSVPA Error: "+errorMsg;
    }

    databasePtr->nmfSetDatabase(ProjectDatabase);

    return databasePtr;
}

bool
nmfMSVPA::dummyFunc()
{
    qDebug() << "testFunc2!!!!!!! "; // << QString::fromStdString(value);
    return true;
}

bool
nmfMSVPA::MSVPA_NoPredatorGrowth(
         MSVPANoPredatorGrowthStruct &argStruct)
{
    int numRecords;
    int tmpSeas;
    int tmpYear;
    int NPreds;
    int NPrey;
    int NOtherFood;
    int NOtherPreds;
    int FirstYear;
    int LastYear;
    int NSeas;
    int NMSVPASpe;
    int NYears;
    int m=0;
    int progressVal = 0;
    double tempValue = 0;
    std::string FirstYearStr;
    std::string LastYearStr;
    std::string M2Val;
    std::string errorMsg;
    std::string cmd;
    std::string cmd2;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::map<std::string, std::vector<std::string> > dataMap2;
    std::map<std::string, std::vector<std::string> > dataMap3;
    std::vector<std::string> fields,fields1,fields2,fields3;
    std::string queryStr,queryStr2;
    std::string field0,field1,field2,field3,field4,field5,field6,field7;
    std::string field8,field9,field10,field11,field12,field13;
    std::vector<std::unique_ptr<nmfMSVPAOthPredInfo>> OthPredList;
    std::vector<std::unique_ptr<nmfMSVPAOthPreyInfo>> OthPreyList;
    std::vector<std::unique_ptr<nmfMSVPASpeciesInfo>> MSVPASpeciesList;
    boost::numeric::ublas::vector<std::string> MSVPAPredList;
    boost::numeric::ublas::matrix<double> SeasLen;
    boost::numeric::ublas::matrix<double> SeasTemp;
    nmfDatabase* databasePtr;

    databasePtr                                     = argStruct.databasePtr;
    std::string MSVPAName                           = argStruct.MSVPAName;
    std::map<std::string,int> CohortAnalysisGuiData = argStruct.CohortAnalysisGuiData;
    std::map<std::string,int> EffortTunedGuiData    = argStruct.EffortTunedGuiData;
    std::map<std::string,std::string> XSAGuiData    = argStruct.XSAGuiData;

    outputProgressLabel("");

    // Data in here isn't reliable since user may not have used the GUI at all in SSVPA
    // Tabs and gone straight to MSVPA.
    XSAGuiData.clear();

    // Start timer for algorithmic timing
    QDateTime startTime = nmfUtilsQt::getCurrentTime();

    //std::string logFile =  nmfLogger::logFile();

    // Get Catch Matrix
    boost::numeric::ublas::matrix<double> CatchMatrix;

    // Initialize vectors and matrices
    nmfUtils::initialize(MSVPAPredList, nmfConstants::MaxTotalSpecies);
    nmfUtils::initialize(SeasLen,  nmfConstants::MaxNumberSeasons, 3); // expressed as proportion of year...actually beginning and end of each
    nmfUtils::initialize(SeasTemp, nmfConstants::MaxNumberYears, nmfConstants::MaxNumberSeasons); // temperature

    nmfSSVPA svpaApi;

    logger->logMsg(nmfConstants::Normal,"MSVPA: nmfMSVPA_NoPredatorGrowth");

    updateProgress(progressVal++,"Clearing tables: MSVPASeasBiomass, MSVPASuitPreyBiomass");

    if (ReadRunStopFile() == "Stop") {
        return false;
    }
    if (! databasePtr->nmfDeleteRecordsFromTable(nmfConstantsMSVPA::TableMSVPASeasBiomass,MSVPAName)) {
        return false;
    }

    if (! databasePtr->nmfDeleteRecordsFromTable(nmfConstantsMSVPA::TableMSVPASuitPreyBiomass,MSVPAName)) {
        return false;
    }

    if (! databasePtr->nmfGetInitialData(MSVPAName,
                            NPreds, NPrey, NOtherFood, NOtherPreds,
                            FirstYear, LastYear, NSeas, NMSVPASpe,
                            NYears, FirstYearStr, LastYearStr)) {
        return false;
    }

    updateProgress(progressVal++, "Setup: Loading Season, Temperature, Predator, and Species data");

    if (ReadRunStopFile() == "Stop") {
        return false;
    }
    if (! GetSeasonalLengthTemperatureData(databasePtr,MSVPAName,NSeas,NYears,SeasLen,SeasTemp)) {
        return false;
    }

    if (! GetPredatorData(databasePtr,MSVPAName,MSVPAPredList)) {
        return false;
    }

    if (! GetSpeciesData(databasePtr,MSVPAName,NMSVPASpe,MSVPASpeciesList)) {
        return false;
    }

    updateProgress(progressVal++, "Setup: Loading other prey data");
    if (ReadRunStopFile() == "Stop")
        return false;
    /*
     * Load Other Prey Data
     */
    nmfUtils::printMsg("Loading other prey data...");
    fields = { "OthPreyName", "MinSize", "MaxSize", "SizeAlpha", "SizeBeta" };
    queryStr = "SELECT OthPreyName,MinSize,MaxSize,SizeAlpha,SizeBeta FROM " +
                nmfConstantsMSVPA::TableMSVPAOthPrey +
               " WHERE MSVPAname='" + MSVPAName + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
//    if (errorCheck("MSVPAOthPrey","OthPreyName",dataMap,queryStr))
//        return false;

    numRecords = dataMap["OthPreyName"].size();
    for (int i=0; i < numRecords; ++i) {
        OthPreyList.push_back(std::make_unique<nmfMSVPAOthPreyInfo>(
                dataMap["OthPreyName"][i],
                std::stod(dataMap["MinSize"][i]),
                std::stod(dataMap["MaxSize"][i]),
                std::stod(dataMap["SizeAlpha"][i]),
                std::stod(dataMap["SizeBeta"][i])));
    }

    updateProgress(progressVal++,"Setup: Loading biomass predator data");
    if (ReadRunStopFile() == "Stop")
        return false;
    /*
     * Load Biomass Predator Data
     */
    nmfUtils::printMsg("Loading biomass predator data...");
    if (NOtherPreds > 0) {
        fields = { "SpeName", "SpeIndex"};
        queryStr = "SELECT SpeName,SpeIndex FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
                   " WHERE MSVPAname='"+MSVPAName+"' AND Type=3";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        numRecords = dataMap["SpeName"].size();
        for (int i=0; i < numRecords; ++i) {
            OthPredList.push_back(std::make_unique<nmfMSVPAOthPredInfo>(
                    dataMap["SpeName"][i],
                    std::stoi(dataMap["SpeIndex"][i])));
        }

        for (int i=0; i<NOtherPreds; ++i) {

            fields = {"BMUnits", "MinSize", "MaxSize", "WtUnits", "SizeUnits", "SizeStruc", "NumSizeCats"};
            queryStr = "SELECT BMUnits,MinSize,MaxSize,WtUnits,SizeUnits,SizeStruc,NumSizeCats FROM " +
                        nmfConstantsMSVPA::TableOtherPredSpecies +
                       " WHERE SpeName='" + OthPredList[i]->SpeName + "'";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            OthPredList[i]->BMunits       = std::stod(dataMap["BMUnits"][0]);
            OthPredList[i]->MinSize       = std::stoi(dataMap["MinSize"][0]);
            OthPredList[i]->MaxSize       = std::stoi(dataMap["MaxSize"][0]);
            OthPredList[i]->WtUnits       = std::stod(dataMap["WtUnits"][0]);
            OthPredList[i]->SizeUnits     = std::stoi(dataMap["SizeUnits"][0]);
            OthPredList[i]->SizeStructure = std::stoi(dataMap["SizeStruc"][0]);
            OthPredList[i]->NumSizeCats   = std::stoi(dataMap["NumSizeCats"][0]);

            if (OthPredList[i]->NumSizeCats > 0) {
                fields = {"MinLen", "MaxLen", "PropBM"};
                queryStr = "SELECT MinLen,MaxLen,PropBM FROM " + nmfConstantsMSVPA::TableOthPredSizeData +
                           " WHERE SpeName ='"+OthPredList[i]->SpeName+"'";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                for (int j=0; j< OthPredList[i]->NumSizeCats; ++j) {
                    OthPredList[i]->SizeCatMins.push_back(std::stoi(dataMap["MinLen"][j]));
                    OthPredList[i]->SizeCatMaxs.push_back(std::stoi(dataMap["MaxLen"][j]));
                    OthPredList[i]->SizeCatBM.push_back(  std::stod(dataMap["PropBM"][j]));
                }
            }
        } // end for i
    } // end if NOtherPreds

    updateProgress(progressVal++, "Setup: Calculating weight and size at age matrices");
    if (ReadRunStopFile() == "Stop")
        return false;
    nmfUtils::printMsg("Reading Data from Project...");
    /*
     * Calculate weight at age and size at age for all of the MSVPA species and convert units
     */
    nmfUtils::printMsg("Calculating weight at age and size at age for all of the MSVPA species and convert units");
    int SizeIndex;
    int WtIndex;
    Boost3DArrayDouble SizeAtAge(boost::extents[NMSVPASpe][NYears][nmfConstants::MaxNumberAges]);
    Boost3DArrayDouble WtAtAge(boost::extents[NMSVPASpe][NYears][nmfConstants::MaxNumberAges]);
    Boost4DArrayDouble SeasWtAge(boost::extents[NSeas][NMSVPASpe][NYears][nmfConstants::MaxNumberAges]);
    Boost4DArrayDouble SeasSizeAge(boost::extents[NSeas][NMSVPASpe][NYears][nmfConstants::MaxNumberAges]);
    boost::numeric::ublas::matrix<double> tmpSize;
    boost::numeric::ublas::matrix<double> tmpWt;
    bool sizeOK;
    for (int i=0; i<NMSVPASpe; ++i) {

        nmfUtils::initialize(tmpSize, nmfConstants::MaxNumberYears, nmfConstants::MaxNumberAges);
        nmfUtils::initialize(tmpWt, nmfConstants::MaxNumberYears, nmfConstants::MaxNumberAges);
        Boost3DArrayDouble tmpSeasSize(boost::extents[nmfConstants::MaxNumberYears][nmfConstants::MaxNumberAges][nmfConstants::MaxNumberSeasons]);
        Boost3DArrayDouble tmpSeasWt(boost::extents[nmfConstants::MaxNumberYears][nmfConstants::MaxNumberAges][nmfConstants::MaxNumberSeasons]);

        fields = { "SizeTypeIndex", "WtTypeIndex"};
        queryStr = "SELECT SizeTypeIndex,WtTypeIndex FROM " + nmfConstantsMSVPA::TableSpecies +
                   " WHERE SpeIndex=" + std::to_string(MSVPASpeciesList[i]->DBSpeIndex);

        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        SizeIndex = std::stoi(dataMap["SizeTypeIndex"][0]);
        WtIndex   = std::stoi(dataMap["WtTypeIndex"][0]);

        sizeOK = GetSize(databasePtr, FirstYear, LastYear,
                         MSVPASpeciesList[i]->FirstCatAge,
                         MSVPASpeciesList[i]->LastCatAge,
                         MSVPASpeciesList[i]->SpeName, SizeIndex,
                         tmpSize, tmpSeasSize, SeasLen, NYears, NSeas);
        if (! sizeOK)
            return -1;

        GetWt(databasePtr, FirstYear, LastYear,
              MSVPASpeciesList[i]->FirstCatAge,
              MSVPASpeciesList[i]->LastCatAge,
              MSVPASpeciesList[i]->SpeName, WtIndex,
              tmpSize, tmpWt, tmpSeasSize, tmpSeasWt, SeasLen, NYears, NSeas);

        for (int j = 0; j < NYears; ++j)
        {
            for (int k = MSVPASpeciesList[i]->FirstCatAge; k <= MSVPASpeciesList[i]->LastCatAge; ++k)
            {
                SizeAtAge[i][j][k] = tmpSize(j, k) * MSVPASpeciesList[i]->SizeUnits;
                WtAtAge[i][j][k]   = tmpWt(j, k)   * MSVPASpeciesList[i]->WtUnits;

                for (int l = 0; l < NSeas; ++l)
                {
                    SeasSizeAge[l][i][j][k] = tmpSeasSize[j][k][l] * MSVPASpeciesList[i]->SizeUnits;
                    SeasWtAge[l][i][j][k]   = tmpSeasWt[j][k][l]   * MSVPASpeciesList[i]->WtUnits;
                }
            }  // end for k
        } // end for j
    } // end for i

    updateProgress(progressVal++, "Setup: Loading predator consumption and preference parameters");
    if (ReadRunStopFile() == "Stop")
        return false;
    // Load Predator consumption and preference parameters
    nmfUtils::printMsg("Loading predator consumption and preference parameters");
    int NAllPrey = NPreds + NPrey + NOtherFood;
    nmfUtils::printMsg("Reading Data from Project...");
    int SpeCount = 0;
    fields = {"PrefVal"};
    Boost3DArrayDouble MSVPAPredTypePref(boost::extents[NPreds][nmfConstants::MaxNumberAges][NMSVPASpe]);
    for (int i = 0; i < NMSVPASpe; ++i) {
        if (MSVPASpeciesList[i]->SpeType == 0) {
            for (int k = 0; k < NMSVPASpe; ++k) {
                queryStr = "SELECT PrefVal FROM " + nmfConstantsMSVPA::TableMSVPAprefs +
                           " WHERE MSVPAname='" + MSVPAName +
                           "' AND SpeName = '"  + MSVPASpeciesList[i]->SpeName +
                           "' AND PreyName = '" + MSVPASpeciesList[k]->SpeName + "' ORDER By Age";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

                for (int j=MSVPASpeciesList[i]->FirstCatAge; j<=MSVPASpeciesList[i]->LastCatAge; ++j) {
                    MSVPAPredTypePref[SpeCount][j][k] = std::stod(dataMap["PrefVal"][j]);
                } // end for j
            } // end for k
            ++SpeCount;
        } // end if
    } // end for i


    updateProgress(progressVal++, "Setup: Loading seasonal spatial overlap data");
    if (ReadRunStopFile() == "Stop")
        return false;
    // Load seasonal spatial overlap...
    nmfUtils::printMsg("Loading seasonal spatial overlap...");
    fields = {"SpOverlap"};
    Boost4DArrayDouble SeasMSVPAPredSpaceO(boost::extents[NSeas][NPreds][nmfConstants::MaxNumberAges][NMSVPASpe]);
    for (int i = 0; i < NSeas; ++i) {
        SpeCount = 0;
        for (int j = 0; j < NMSVPASpe; ++j) {
            if (MSVPASpeciesList[j]->SpeType == 0) {
                for (int l = 0; l < NMSVPASpe; ++l) {
                    queryStr = "SELECT SpOverlap FROM " + nmfConstantsMSVPA::TableMSVPASpaceO +
                               " WHERE MSVPAname='" + MSVPAName +
                               "' AND SpeName = '"  + MSVPASpeciesList[j]->SpeName +
                               "' AND PreyName = '"	+ MSVPASpeciesList[l]->SpeName +
                               "' AND Season = "	+ std::to_string(i+1) + " ORDER By Age";
                    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                    if (dataMap["SpOverlap"].size() > 0) {
                        for (int k = MSVPASpeciesList[j]->FirstCatAge; k <= MSVPASpeciesList[j]->LastCatAge; ++k) {
                            SeasMSVPAPredSpaceO[i][SpeCount][k][l] = std::stod(dataMap["SpOverlap"][k]);
                        }
                    }
                } // end for l
                ++SpeCount;
            } // end if
        } // end for j
    } // end for i

    updateProgress(progressVal++, "Setup: Loading type preference indices for biomass predators");
    if (ReadRunStopFile() == "Stop")
        return false;
    //  Load type preference indices for biomass predators
    nmfUtils::printMsg("Loading type preference indices for biomass predators...");
    fields = {"PrefVal"};
    Boost3DArrayDouble OtherPredTypePref(boost::extents[NOtherPreds][10][NAllPrey]);
    for (int i = 0; i < NOtherPreds; ++i) {
        queryStr = "Select PrefVal FROM " + nmfConstantsMSVPA::TableMSVPAprefs +
                   " WHERE MSVPAname = '" + MSVPAName +
                   "' AND SpeName = '" + OthPredList[i]->SpeName +
                   "' ORDER By Age, PreyName";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (OthPredList[i]->SizeStructure == 0) {
            for (int k = 0; k < NAllPrey; ++k) {
                OtherPredTypePref[i][0][k] = std::stod(dataMap["PrefVal"][k]);
            }
        } else {
            m = 0;
            for (int j = 0; j < OthPredList[i]->NumSizeCats; ++j) {
                for (int k = 0; k < NAllPrey; ++k) {
                    OtherPredTypePref[i][j][k] = std::stod(dataMap["PrefVal"][m++]);
                }
            }
        } // end if
    } // end for i

    updateProgress(progressVal++, "Setup: Loading average stomach contents");
    if (ReadRunStopFile() == "Stop")
        return false;
    // Load average stomach contents
    nmfUtils::printMsg("Loading average stomach contents");
    fields = {"MeanGutFull"};
    Boost3DArrayDouble MSVPAPredStomCont(boost::extents[NPreds][nmfConstants::MaxNumberAges][NSeas]);
    SpeCount = 0;
    for (int i = 0; i < NMSVPASpe; ++i) {
        if (MSVPASpeciesList[i]->SpeType == 0) {
            queryStr = "SELECT MeanGutFull FROM " + nmfConstantsMSVPA::TableMSVPAStomCont +
                       " WHERE MSVPAname = '" + MSVPAName +
                       "' AND SpeName = '" + MSVPASpeciesList[i]->SpeName + "' ORDER By Age, Season";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            m = 0;
            for (int j = MSVPASpeciesList[i]->FirstCatAge; j <= MSVPASpeciesList[i]->LastCatAge; ++j) {
                for (int k = 0; k < NSeas; ++k) {
                    MSVPAPredStomCont[SpeCount][j][k] = std::stod(dataMap["MeanGutFull"][m++]);
                }
            }  // end for j
            ++SpeCount;
        } // end if
    } // end for i

    updateProgress(progressVal++,"Setup: Loading type preference indices for biomass predators");
    if (ReadRunStopFile() == "Stop")
        return false;
    //  Load type preferences for biomass predators
    nmfUtils::printMsg("Loading type preference indices for biomass predators...");
    fields = {"MeanGutFull"};
    Boost3DArrayDouble OtherPredStomCont(boost::extents[NOtherPreds][10][NSeas]);
    for (int i = 0; i < NOtherPreds; ++i) {
        queryStr = "Select MeanGutFull FROM " + nmfConstantsMSVPA::TableMSVPAStomCont +
                   " WHERE MSVPAname = '" + MSVPAName +
                   "' AND SpeName = '" + OthPredList[i]->SpeName + "' ORDER By Age, Season";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (OthPredList[i]->SizeStructure == 0) {
            for (int k = 0; k < NSeas; ++k) {
                OtherPredStomCont[i][0][k] = std::stod(dataMap["MeanGutFull"][k]);
            }
        } else {
            m = 0;
            for (int j = 0; j < OthPredList[i]->NumSizeCats; ++j) {
                for (int k = 0; k < NSeas; ++k) {
                    OtherPredStomCont[i][j][k] = std::stod(dataMap["MeanGutFull"][m++]);
                }
            }
        } // end if
    } // end for i

    updateProgress(progressVal++,"Setup: Loading Alpha,Beta evacuation rates and size preference values");
    if (ReadRunStopFile() == "Stop")
        return false;
    // Load Evacuation rate Alpha and Beta and Size Preference Alpha&Beta values
    fields = {"EvacAlpha","EvacBeta","SizeAlpha","SizeBeta"};
    int numFields = fields.size();
    Boost3DArrayDouble MSVPAPredCoeff(boost::extents[NPreds][nmfConstants::MaxNumberAges][numFields]);
    SpeCount = 0;
    for (int i = 0; i < NMSVPASpe; ++i) {
        if (MSVPASpeciesList[i]->SpeType == 0) {
            queryStr = "SELECT EvacAlpha,EvacBeta,SizeAlpha,SizeBeta FROM " +
                        nmfConstantsMSVPA::TableMSVPASizePref +
                        " WHERE MSVPAname = '" + MSVPAName +
                        "' AND SpeName = '" + MSVPASpeciesList[i]->SpeName +
                        "' ORDER By Age";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            for (int j = MSVPASpeciesList[i]->FirstCatAge; j <= MSVPASpeciesList[i]->LastCatAge; ++j) {
                for (int m = 0; m < numFields; ++m) {
                    MSVPAPredCoeff[SpeCount][j][m] = std::stod(dataMap[fields[m]][j]);
                }
            }
            ++SpeCount;
        } // end if
    } // end for i

    updateProgress(progressVal++,"Setup: Loading size selectivity for biomass predators");
    if (ReadRunStopFile() == "Stop")
        return false;
    // Load size selectivity for biomass predators
    nmfUtils::printMsg("Loading size selectivity for biomass predators...");
    double SizeSelAlpha;
    double SizeSelBeta;
    fields = {"ConsAlpha","ConsBeta","SizeSelAlpha","SizeSelBeta"};
    numFields = fields.size();
    Boost3DArrayDouble OtherPredCoeff(boost::extents[NOtherPreds][10][numFields]);
    for (int i = 0; i < NOtherPreds; ++i) {
        queryStr = "Select ConsAlpha,ConsBeta,SizeSelAlpha,SizeSelBeta FROM " +
                    nmfConstantsMSVPA::TableOthPredSizeData +
                   " WHERE  SPeName = '" + OthPredList[i]->SpeName +
                   "' ORDER By SizeCat";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (OthPredList[i]->SizeStructure == 0) {
            for (int m = 0; m < numFields; ++m) {
                OtherPredCoeff[i][0][m] = std::stod(dataMap[fields[m]][0]);
            }
        } else {
            for (int j = 0; j < OthPredList[i]->NumSizeCats; ++j) {
                for (int m = 0; m < numFields; ++m) {
                    OtherPredCoeff[i][j][m] = std::stod(dataMap[fields[m]][j]);
                }
            }
        }
    } // end for i

    updateProgress(progressVal++,"Setup: Loading other prey biomass");
    if (ReadRunStopFile() == "Stop")
        return false;
    /*
     * LOAD OTHER PREY BIOMASS
     */
    double AvgBM;
    Boost3DArrayDouble SeasOthPreyBiomass(boost::extents[NSeas][NOtherFood][NYears]);
    fields = {"Biomass"};
    for (int i=0; i<NOtherFood; ++i) {
        queryStr = "SELECT Biomass FROM " + nmfConstantsMSVPA::TableMSVPAOthPreyAnn +
                   " WHERE MSVPAname = '" + MSVPAName +
                   "' AND OthPreyName = '" + OthPreyList[i]->SpeName +
                   "' AND (Year >= " + FirstYearStr +
                   " AND Year <= " + LastYearStr + ") ORDER By Year, Season";
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        m = 0;
        for (int j = 0; j < NYears; ++j) {
            for (int k = 0; k < NSeas; ++k) {
                SeasOthPreyBiomass[k][i][j] = std::stod(dataMap["Biomass"][m++]);
            }
        }
    }

    updateProgress(progressVal++, "Setup: Calculating seasonal biomasses for other predators");
    if (ReadRunStopFile() == "Stop")
        return false;
    /*
     * CALCULATE SEASONAL BIOMASSES FOR OTHER PREDATORS
     */
    // Get annual biomass for the biomass predators....
    nmfUtils::printMsg("Get annual biomass for the biomass predators....");
    if (NOtherPreds > 0) {

        boost::numeric::ublas::matrix<double> OtherPredBiomass;
        nmfUtils::initialize(OtherPredBiomass, NOtherPreds, NYears);
        Boost3DArrayDouble SeasOthPredBiomass(boost::extents[NSeas][NOtherPreds][NYears]);

        fields = {"Biomass"};
        for (int i = 0; i < NOtherPreds; ++i) {
            queryStr = "SELECT Biomass FROM " + nmfConstantsMSVPA::TableOtherPredBM +
                       " WHERE SpeName = '" + OthPredList[i]->SpeName +
                       "' AND (Year >= " + FirstYearStr +
                       " AND Year <= " + LastYearStr + ") ORDER BY Year";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            for (int j = 0; j < NYears; ++j) {
                OtherPredBiomass(i, j) = std::stod(dataMap["Biomass"][j]) * OthPredList[i]->BMunits;

            }
        } // end for j

        // Calculate seasonal biomass assuming exponential rate of change
        nmfUtils::printMsg("Calculating seasonal biomass assuming exponential rate of change...");
        double AnnRate;
        double SeasStart;
        double SeasEnd;

        cmd = "INSERT INTO " + nmfConstantsMSVPA::TableMSVPASeasBiomass +
              " (MSVPAname, Season, SpeName, SpeType, Year, Age, Biomass) VALUES ";
        for (int i=0; i<NSeas; ++i) {

            SeasStart = SeasLen(i,0);
            SeasEnd   = SeasLen(i,1);

            for (int j = 0; j < NOtherPreds; ++j) {

                for (int k = 0; k <= NYears-2; ++k) {
                    // Calculate an annual growth rate
                    tempValue = OtherPredBiomass(j,k);
                    if (tempValue == 0) {
                        logger->logMsg(nmfConstants::Error,"Error MSVPA(1): OtherPredBiomass("+
                                std::to_string(j) + "," +
                                std::to_string(k) + ") = 0. This will cause a divide by 0 error.");
                        return false;
                    }
                    AnnRate = -std::log(OtherPredBiomass(j,k+1) / tempValue);

                    // Calculate avg rel Growth at midpoint
                    if (SeasEnd == SeasStart) {
                        logger->logMsg(nmfConstants::Error,"Error MSVPA(2): Found SeasEnd == SeasStart (each = " +
                                std::to_string(SeasEnd) + "). This will cause a divide by 0 error.");
                        return false;
                    }
                    AvgBM = (AnnRate == 0) ? 1 : (std::exp(-AnnRate*SeasStart) - std::exp(-AnnRate*SeasEnd)) /
                            ((SeasEnd-SeasStart)*AnnRate);

                    // Calculate Biomass at midpoint of season as product
                    SeasOthPredBiomass[i][j][k] = AvgBM	* OtherPredBiomass(j,k);

                    // Apply same rate to last year
                    if (k == NYears - 2) {
                        SeasOthPredBiomass[i][j][k+1] = AvgBM * OtherPredBiomass(j,k+1);
                    } // end if
                } // end for k

                for (int k = 0; k < NYears; ++k) {
                    if (OthPredList[j]->SizeStructure == 0) {
                        field0 = std::string("\"")+MSVPAName+"\"";
                        field1 = std::to_string(i);
                        field2 = std::string("\"")+OthPredList[j]->SpeName+"\"";
                        field3 = std::to_string(2);
                        field4 = std::to_string(k);
                        field5 = std::to_string(0);
                        field6 = std::to_string(SeasOthPredBiomass[i][j][k]);
                        cmd += "(" + field0 + "," + field1 + "," + field2 + "," + field3 +
                               "," + field4 + "," + field5 + "," + field6 +"), ";
                    } else {
                        for (int l = 0; l < OthPredList[j]->NumSizeCats; ++l) {
                            field0 = std::string("\"")+MSVPAName+"\"";
                            field1 = std::to_string(i);
                            field2 = std::string("\"")+OthPredList[j]->SpeName+"\"";
                            field3 = std::to_string(2);
                            field4 = std::to_string(k);
                            field5 = std::to_string(l);
                            field6 = std::to_string(SeasOthPredBiomass[i][j][k] * OthPredList[j]->SizeCatBM[l]);
                            cmd += "("  + field0 + "," + field1 + "," + field2 + "," + field3 +
                                    "," + field4 + "," + field5 + "," + field6 +"), ";
                        } // end for l
                    } // end if else

                } // end for k

            } // end for j

        } // end for i

        // Update the database
        // Remove last comma and space from string
        cmd = cmd.substr(0,cmd.size()-2);
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if ( nmfUtilsQt::isAnError(errorMsg) ) nmfUtils::printError("Section: 1a",errorMsg);

        //databasePtr->nmfSaveDatabase();
        cmd = "INSERT INTO " + nmfConstantsMSVPA::TableMSVPASeasBiomass +
              " (MSVPAname, Season, SpeName, SpeType, Year, Age, Biomass) VALUES ";
        for (int i=0; i < NSeas; ++i) {
            for (int j=0; j < NOtherFood; ++j) {
                for (int k=0; k < NYears; ++k) {
                    field0 = std::string("\"")+MSVPAName+"\"";
                    field1 = std::to_string(i);
                    field2 = std::string("\"")+OthPreyList[j]->SpeName+"\"";
                    field3 = std::to_string(2);
                    field4 = std::to_string(k);
                    field5 = std::to_string(0);
                    field6 = std::to_string(SeasOthPreyBiomass[i][j][k]);
                    cmd += "(" + field0 + "," + field1 + "," + field2 + "," + field3 +
                           "," + field4 + "," + field5 + "," + field6 +"), ";

                } // end for k
            } // end for j
        } // end for i

        // Update the database
        // Remove last comma and space from string
        cmd = cmd.substr(0,cmd.size()-2);
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if ( nmfUtilsQt::isAnError(errorMsg) ) nmfUtils::printError("Error 1b",errorMsg);

    } // end if

    //databasePtr->nmfSaveDatabase();

    // DO SUITABLE PREY BIOMASS CALCULATIONS FOR OTHER PREY
    nmfUtils::printMsg("Calculating other prey suitable biomasses...");
    /*
     *
     * DO SUITABLE PREY BIOMASS CALCULATIONS FOR OTHER PREY (BOTH PREDATOR TYPES) HERE
     * THIS DOES NOT NEED TO BE INCLUDED IN THE MSVPA LOOP AS OTHER PREY BIOMASS IS NOT
     * RECALCULATED DURING THE MSVPA.
     *
     */
    double TypePref;
    double SpaceOverlap;
    double PredSize;
    double PreySize;
    double SizeSelCoeff;
    double MaxVal;
    int NCats;
    int MinPredSize;
    int MaxPredSize;
    double SumSuitBM;
    double SizeInterval;
    int NPySizeInt;
    double RelIntMin;
    double RelIntMax;
    //int SpeIndex;
    boost::numeric::ublas::matrix<double> tmpPreyBiomass;
    boost::numeric::ublas::matrix<double> tmpSuitBiomass; // holds the suitable biomass, by year and age class for a particular prey
    nmfUtils::initialize(tmpSuitBiomass,NYears,nmfConstants::MaxNumberAges);

    /*
     *
     * START CALCULATION FOR MSVPA SPECIES EATING OTHER PREY
     *
     */
    nmfUtils::printMsg("Starting calculations for MSVPA Species eating other prey.");
    bool updateDB = false;
    cmd = "INSERT INTO " + nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
          " (MSVPAname, PredName, PredAge, PreyName, PreyAge, Year, Season, SuitPreyBiomass) VALUES ";
    for (int i = 0; i < NSeas; ++i) {
std::cout << "  Season: " << i+1 << std::endl;
updateProgress(progressVal++,"Setup: Calculating species eating other prey: Season "+std::to_string(i+1));
if (ReadRunStopFile() == "Stop")
    return false;
        SpeCount = 0;
        for (int j = 0; j < NMSVPASpe; ++j) {

            if (MSVPASpeciesList[j]->SpeType == 0) {
                for (int k_Age=MSVPASpeciesList[j]->FirstCatAge; k_Age<=MSVPASpeciesList[j]->LastCatAge; ++k_Age) {

                    SizeSelAlpha = MSVPAPredCoeff[SpeCount][k_Age][2];
                    SizeSelBeta  = MSVPAPredCoeff[SpeCount][k_Age][3];

                    for (int l=0; l<NOtherFood; ++l) {
                        // Get the type pref for that prey type for the predator species/age
                        fields = {"PrefVal"};
                        queryStr = "SELECT PrefVal FROM " + nmfConstantsMSVPA::TableMSVPAprefs +
                                   " WHERE MSVPAname = '" + MSVPAName +
                                   "' AND SpeName = '" + MSVPASpeciesList[j]->SpeName +
                                   "' AND Age = " + std::to_string(k_Age) +
                                   " AND PreyName = '" + OthPreyList[l]->SpeName + "'";
                        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                        TypePref = std::stod(dataMap["PrefVal"][0]);

                        if (TypePref != 0) {
                            // Then get spatial overlap for the prey type, and season
                            fields = {"SpOverlap"};
                            queryStr = "SELECT SpOverlap FROM " + nmfConstantsMSVPA::TableMSVPASpaceO +
                                       " WHERE MSVPAname = '" + MSVPAName +
                                       "' AND SpeName = '" + MSVPASpeciesList[j]->SpeName +
                                       "' AND Age = " + std::to_string(k_Age) +
                                       "  AND PreyName='" + OthPreyList[l]->SpeName +
                                       "' AND Season=" + std::to_string(i+1);
                            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                            SpaceOverlap = (dataMap["SpOverlap"].size() > 0) ? std::stod(dataMap["SpOverlap"][0]) : 0;
                            if (SpaceOverlap != 0) {
                                // Calculate the proportion of prey biomass by size intervals...
                                SizeInterval = OthPreyList[l]->MaxSize - OthPreyList[l]->MinSize;
                                if (SizeInterval == 0) {
                                    logger->logMsg(nmfConstants::Error,"Error MSVPA(3): Found Other Prey Biomass SizeInterval of 0. This will cause a divide by 0 error.");
                                    return false;
                                }
                                NPySizeInt = std::floor((SizeInterval/0.1) + 0.5); // number of 0.1 cm intervals
                                nmfUtils::initialize(tmpPreyBiomass,NPySizeInt,3);
                                for (int X=0; X<NPySizeInt; ++X) {
                                      tmpPreyBiomass(X,0) = OthPreyList[l]->MinSize + (X * 0.1);
                                      tmpPreyBiomass(X,1) = OthPreyList[l]->MinSize + ((X + 1) * 0.1);

                                      RelIntMin = (tmpPreyBiomass(X,0) - OthPreyList[l]->MinSize) / SizeInterval;
                                      RelIntMax = (tmpPreyBiomass(X,1) - OthPreyList[l]->MinSize) / SizeInterval;
                                      tmpPreyBiomass(X,2) =
                                          nmfUtilsStatistics::BetaI(OthPreyList[l]->SizeAlpha, OthPreyList[l]->SizeBeta, RelIntMax) -
                                          nmfUtilsStatistics::BetaI(OthPreyList[l]->SizeAlpha, OthPreyList[l]->SizeBeta, RelIntMin);
                                } // end for X

                                for (int mm= 0; mm<NYears; ++mm) {
                                    PredSize = SeasSizeAge[i][j][mm][k_Age];
                                    if (PredSize == 0) {
                                        logger->logMsg(nmfConstants::Error,"Error MSVPA(4): Found PredSize of 0. This will cause a divide by 0 error.");
                                        return false;
                                    }
                                    SumSuitBM = 0;

                                    for (int X=0; X<NPySizeInt; ++X) {
                                        // Get the size selection index...
                                        if ((tmpPreyBiomass(X,0)/PredSize) > 1) {
                                            SizeSelCoeff = 0;
                                        } else {
                                            MaxVal = ((tmpPreyBiomass(X,1)/PredSize) > 1) ? 1 : tmpPreyBiomass(X,1)/PredSize;
                                            SizeSelCoeff = nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, MaxVal) -
                                                           nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, tmpPreyBiomass(X, 0) / PredSize);
                                        }

                                        SumSuitBM += (SeasOthPreyBiomass[i][l][mm] * tmpPreyBiomass(X,2) * SpaceOverlap * TypePref * SizeSelCoeff);
                                    }

                                    tmpSuitBiomass(mm,0) = SumSuitBM; // since there's no age structure for the prey..its just by year...

                                    if (tmpSuitBiomass(mm,0) != 0) {
                                        updateDB = true;
                                        field0 = std::string("\"")+MSVPAName+"\"";
                                        field1 = std::string("\"")+MSVPASpeciesList[j]->SpeName+"\"";
                                        field2 = std::to_string(k_Age);
                                        field3 = std::string("\"")+OthPreyList[l]->SpeName+"\"";
                                        field4 = std::to_string(0);
                                        field5 = std::to_string(mm);
                                        field6 = std::to_string(i);
                                        field7 = std::to_string(tmpSuitBiomass(mm,0));
                                        cmd += "(" + field0 + "," + field1 + "," + field2 + "," + field3 +
                                               "," + field4 + "," + field5 + "," + field6 + "," + field7 + "), ";

                                    } // end if
                                } // end for mm

                            } // end if space0
                        }
                    } // end for l
                } // end for k
                ++SpeCount;

            } // end if
        } // end for j
    }  // end for i

    // Update the database
    if (updateDB) {
        // Remove last comma and space from string
        cmd = cmd.substr(0,cmd.size()-2);
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if ( nmfUtilsQt::isAnError(errorMsg) ) nmfUtils::printError("Section: Starting calculations for MSVPA Species eating other prey",errorMsg);
    }
    nmfUtils::printMsg("End of suitable biomass calculations for MSVPA Species eating other prey.\n");

    //databasePtr->nmfSaveDatabase();

    /*
     * Calculate SUITABLE Biomass for other predators consuming other prey  >>>>>>
     */
    nmfUtils::printMsg("Calculating SUITABLE Biomass for other predators consuming other prey...");
    updateDB = false;
    std::cout << "Calculate SUITABLE Biomass for other predators consuming other prey..." << std::endl;
    if (NOtherPreds > 0) {

        //queryStr = "SELECT * FROM " + nmfConstantsMSVPA::TableMSVPASuitPreyBiomass + " WHERE MSVPAname = '"	+ MSVPAName + "'";
        cmd = "INSERT INTO " + nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
              " (MSVPAname, PredName, PredAge, PreyName, PreyAge, Year, Season, SuitPreyBiomass) values ";

        for (int i = 0; i < NSeas; ++i) {
std::cout << "  season: " << i+1 << std::endl;
updateProgress(progressVal++, "Setup: Calculating suitable biomass for other predators eating other prey: Season "+std::to_string(i+1));
            if (ReadRunStopFile() == "Stop") {
             return false;
            }
            for (int j = 0; j < NOtherPreds; ++j) {
                NCats = (OthPredList[j]->SizeStructure == 0) ? 0 : OthPredList[j]->NumSizeCats;
                for (int k = 0; k < NCats; ++k) {
                    if (NCats == 0) {
                        MinPredSize = OthPredList[j]->MinSize;
                        MaxPredSize = OthPredList[j]->MaxSize;
                    } else {
                        MinPredSize = OthPredList[j]->SizeCatMins[k];
                        MaxPredSize = OthPredList[j]->SizeCatMaxs[k];
                    }
                    SizeSelAlpha = OtherPredCoeff[j][k][2];
                    SizeSelBeta  = OtherPredCoeff[j][k][3];

                    for (int l=0; l<NOtherFood; ++l) {
                        // Get the type pref for that prey type for the predator species/age
                        fields = {"PrefVal"};
                        queryStr = "SELECT PrefVal FROM " + nmfConstantsMSVPA::TableMSVPAprefs +
                                   " WHERE MSVPAname = '" + MSVPAName +
                                   "' AND SpeName = '" + OthPredList[j]->SpeName +
                                   "' AND Age = " + std::to_string(k) +
                                   " AND PreyName = '" + OthPreyList[l]->SpeName + "'";
                        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                        TypePref = std::stod(dataMap["PrefVal"][0]);

                        if (TypePref != 0) {
                            // Then get spatial overlap for the prey type, and season
                            fields = {"SpOverlap"};
                            queryStr = "SELECT SpOverlap FROM " + nmfConstantsMSVPA::TableMSVPASpaceO +
                                       " WHERE MSVPAname = '" + MSVPAName +
                                       "' AND SpeName = '" + OthPredList[j]->SpeName +
                                       "' AND Age = " + std::to_string(k) +
                                       "  AND PreyName = '" + OthPreyList[l]->SpeName +
                                       "' AND Season = " + std::to_string(i+1);
                            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                            SpaceOverlap = (dataMap["SpOverlap"].size() > 0) ? std::stod(dataMap["SpOverlap"][0]) : 0;
                            if (SpaceOverlap != 0) {
                                // Calculate the proportion of prey biomass by size intervals...
                                SizeInterval = OthPreyList[l]->MaxSize - OthPreyList[l]->MinSize;
                                if (SizeInterval == 0) {
                                    logger->logMsg(nmfConstants::Error,"Error MSVPA(5): Found SizeInterval of 0. This will cause a divide by 0 error.");
                                    return false;
                                }
                                NPySizeInt = std::floor(SizeInterval/0.1 + 0.5);
                                nmfUtils::initialize(tmpPreyBiomass, NPySizeInt, 3);
                                for (int X=0; X<NPySizeInt; ++X) {
                                  tmpPreyBiomass(X,0) = OthPreyList[l]->MinSize + (X * 0.1);
                                  tmpPreyBiomass(X,1) = OthPreyList[l]->MinSize + ((X + 1) * 0.1);

                                  RelIntMin = (tmpPreyBiomass(X,0) - OthPreyList[l]->MinSize) / SizeInterval;
                                  RelIntMax = (tmpPreyBiomass(X,1) - OthPreyList[l]->MinSize) / SizeInterval;

                                  tmpPreyBiomass(X,2) =
                                          nmfUtilsStatistics::BetaI(OthPreyList[l]->SizeAlpha, OthPreyList[l]->SizeBeta, RelIntMax) -
                                          nmfUtilsStatistics::BetaI(OthPreyList[l]->SizeAlpha, OthPreyList[l]->SizeBeta, RelIntMin);
                                }

                                for (int mm=0; mm<NYears; ++mm) {
                                    SumSuitBM = 0;
                                    for (int YY=MinPredSize; YY <= MaxPredSize; ++YY) {
                                        if (YY == 0) {
                                            logger->logMsg(nmfConstants::Error,"Error MSVPA(6): Found MinPredSize of 0. This will cause a divide by 0 error.");
                                            return false;
                                        }
                                        for (int X=0; X<NPySizeInt; ++X) {
                                            // Get the size selection index...
                                            if ((tmpPreyBiomass(X, 0) / YY) > 1) {
                                                SizeSelCoeff = 0;
                                            } else {
                                                MaxVal = ((tmpPreyBiomass(X, 1) / YY) > 1) ? 1 : tmpPreyBiomass(X, 1) / YY;
                                                SizeSelCoeff =
                                                    nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, MaxVal) -
                                                    nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, tmpPreyBiomass(X,0)/YY);
                                            }
                                            SumSuitBM += (SeasOthPreyBiomass[i][l][mm] * tmpPreyBiomass(X,2) * TypePref *
                                                          SpaceOverlap * SizeSelCoeff);
                                        } // end for X
                                    } // end for YY

                                    if (MaxPredSize == MinPredSize) {
                                        logger->logMsg(nmfConstants::Error,"Error MSVPA(7): Found MaxPredSize == MinPredSize (value of " +
                                                std::to_string(MaxPredSize) +
                                                "). This will cause a divide by 0 error.");
                                        return false;
                                    }
                                    tmpSuitBiomass(mm, 0) = SumSuitBM / (MaxPredSize - MinPredSize);
                                    if (tmpSuitBiomass(mm, 0) != 0) {
                                        updateDB = true;

                                        field0 = std::string("\"")+MSVPAName+"\"";
                                        field1 = std::string("\"")+OthPredList[j]->SpeName+"\"";
                                        field2 = std::to_string(k);
                                        field3 = std::string("\"")+OthPreyList[l]->SpeName+"\"";
                                        field4 = std::to_string(0);
                                        field5 = std::to_string(mm);
                                        field6 = std::to_string(i);
                                        field7 = std::to_string(tmpSuitBiomass(mm,0));

                                        cmd += "(" + field0 + "," + field1 + "," + field2 + "," + field3 +
                                        "," + field4 + "," + field5 + "," + field6 + "," + field7 + "), ";
                                    }
                                } // end for mm
                            } // end if SpaceOverlap
                        } // end if TypePref
                    } // end for l
                } // end for k
            } // end for j
        } // end for i

    } // end if

    // Update the database
    if (updateDB) {
        // Remove last comma and space from string
        cmd = cmd.substr(0,cmd.size()-2);
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if ( nmfUtilsQt::isAnError(errorMsg) ) nmfUtils::printError("Section: Calculate SUITABLE Biomass for other predators",errorMsg);
    }
    nmfUtils::printMsg("END SUITABLE BIOMASS CALCULATION FOR OTHER PREY EATEN BY OTHER PREDS\n");

    updateProgress(progressVal++,"Setup: Initializing M1,M2 matrices from entered data");
    if (ReadRunStopFile() == "Stop")
        return false;

    // Initialize M matrices from the entered data.
    int XSAWarn = 0;
    boost::numeric::ublas::matrix<double> tmpF;
    boost::numeric::ublas::matrix<double> tmpM;
    boost::numeric::ublas::matrix<double> tmpN;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> lclM;
    nmfUtils::initialize(tmpF, NYears, nmfConstants::MaxNumberAges);
    nmfUtils::initialize(tmpM, NYears, nmfConstants::MaxNumberAges);
    nmfUtils::initialize(tmpN, NYears, nmfConstants::MaxNumberAges);
    Boost3DArrayDouble M(boost::extents[NMSVPASpe][NYears][nmfConstants::MaxNumberAges+1]);
    Boost3DArrayDouble M1(boost::extents[NMSVPASpe][NYears][nmfConstants::MaxNumberAges+1]);
    Boost3DArrayDouble M2(boost::extents[NMSVPASpe][NYears][nmfConstants::MaxNumberAges+1]);
    Boost3DArrayDouble Z(boost::extents[NMSVPASpe][NYears][nmfConstants::MaxNumberAges]);
    Boost3DArrayDouble N(boost::extents[NMSVPASpe][NYears][nmfConstants::MaxNumberAges]);
    Boost3DArrayDouble F(boost::extents[NMSVPASpe][NYears][nmfConstants::MaxNumberAges]);
    Boost4DArrayDouble SeasBiomass(boost::extents[NSeas][NMSVPASpe][NYears][nmfConstants::MaxNumberAges]); // Holds biomass in a particular season

    fields = {"AgeM1","AgeM2"};
    for (int i = 0; i < NMSVPASpe; ++i) {
        queryStr = "SELECT AgeM1,AgeM2 FROM " + nmfConstantsMSVPA::TableSSVPAAgeM +
                   " WHERE SpeIndex = " + std::to_string(MSVPASpeciesList[i]->DBSpeIndex) +
                   " AND SSVPAindex = " + std::to_string(MSVPASpeciesList[i]->SVPAIndex);
        dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["AgeM1"].size() == 0) {
            std::cout << "Error: No data found for query: " << queryStr << std::endl;
            return -2;
        }
        m = -1;
        for (int j=MSVPASpeciesList[i]->FirstCatAge; j<=MSVPASpeciesList[i]->LastCatAge; ++j) {
            ++m;
            for (int k=0; k<NYears; ++k) {
                M1[i][k][j] = std::stod(dataMap["AgeM1"][m]);
                M2[i][k][j] = std::stod(dataMap["AgeM2"][m]);
            } // end for k
        } // end for j
    } // end for i

    // Comment this out.  I think you want to look at the table SpeXSAIndices and
    // and grab the currently selected index name (and number) and do something with it.
    // tbd...RSK
//    int XSAIndex;
//    fields = {"NXSAIndex"};
//    queryStr = "SELECT NXSAIndex FROM " + nmfConstantsMSVPA::TableSpeSSVPA + " WHERE SpeIndex = " + std::to_string(MSVPASpeciesList[i]->DBSpeIndex);
//    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
//    XSAIndex = std::stoi(dataMap["NXSAIndex"][0]);

    // first term was -1
    updateProgress(progressVal++,"Setup Complete. Progress may now be monitored in Progress Chart window.");
    if (ReadRunStopFile() == "Stop")
        return false;


    /*
     *
     * START OF MSVPA LOOP
     *
     */
    logger->logMsg(nmfConstants::Bold,"MSVPA: START OF MSVPA LOOP");

    int numItems = 0;
    int numItems2 = 0;
    int DiffCount = -999999;
    int NumLoops = -1;
    int TolCheck = 0;

    double AvgAbund;
    double SeasStart;
    double SeasEnd;
    double EvacRate;
    double PDiet;

    boost::numeric::ublas::vector<double> YearWeights;
    boost::numeric::ublas::vector<double> Alpha;
    boost::numeric::ublas::vector<double> Beta;

    std::string msg;
    std::string loopPrefix="";
    Boost3DArrayDouble AvgSuitBM(boost::extents[NPreds][nmfConstants::MaxNumberAges][NSeas]);
    Boost4DArrayDouble AnnTotSuitBiomass(boost::extents[NPreds][nmfConstants::MaxNumberAges][NYears][NSeas]);
    Boost4DArrayDouble StomCorrection(boost::extents[NPreds][nmfConstants::MaxNumberAges][NYears][NSeas]);
    Boost4DArrayDouble PredConsumption(boost::extents[NPreds][nmfConstants::MaxNumberAges][NYears][NSeas]);
    Boost4DArrayDouble TotConsumption(boost::extents[NPreds][nmfConstants::MaxNumberAges][NYears][NSeas]);
    int    NIt = 0;
    int RefAge=0;
    int NumFleets;
    unsigned int NumCatchAges=0;
    unsigned int NumCatchYears=0;
    double CV  = 0.0;
    double SE  = 0.0;
    double ZCheck = 0;
    double FullF=0.0;
    boost::numeric::ublas::vector<double> InitialSelectivity;
    Boost4DArrayDouble FleetData;
    std::string val="";
    bool userHalted = false;
    FullSVPAInputArgs    FullSVPAArgsIn;
    FullSVPAOutputArgs   FullSVPAArgsOut;
    CandSVPAInputArgs    CandSVPAArgsIn;
    CandSVPAOutputArgs   CandSVPAArgsOut;
    EffortTuneInputArgs  EffortTuneArgsIn;
    EffortTuneOutputArgs EffortTuneArgsOut;
    XSAInputArgs         XSAArgsIn;
    XSAOutputArgs        XSAArgsOut;

    while ((TolCheck < 1) && (NumLoops < nmfConstantsMSVPA::MaxMSVPALoops))
    {
        ++NumLoops;

        while ((val=ReadRunStopFile()) == "Pause") {;}
        if (val == "Stop") {
            userHalted = true;
            break;
        }

        loopPrefix = "MSVPA Loop "+std::to_string(NumLoops+1);

        updateProgress(progressVal++, loopPrefix);
        logger->logMsg(nmfConstants::Bold,loopPrefix);

        for (int i = 0; i < NMSVPASpe; ++i) {
            for (int j = MSVPASpeciesList[i]->FirstCatAge; j <= MSVPASpeciesList[i]->LastCatAge; ++j) {
                for (int k = 0; k < NYears; ++k) {
                    M[i][k][j] = M1[i][k][j] + M2[i][k][j];
                }
            }
        } // end for i

        // Run appropriate SVPA for each MSVPA species
        logger->logMsg(nmfConstants::Normal,"MSVPA:  --- SSVPA Run Begin ---");

        for (int i = 0; i < NMSVPASpe; ++i) {

            //  First get M for the particular species and put it in tmpM
            for (int j = 0; j < NYears; ++j) {
                for (int k = MSVPASpeciesList[i]->FirstCatAge; k <= MSVPASpeciesList[i]->LastCatAge; ++k) {
                    tmpM(j, k) = M[i][j][k];
                }
            }
            msg = "Processing Single Species (" + std::to_string(i+1) +
                  " of " + std::to_string(NMSVPASpe) +
                  ") VPA for:  " + MSVPASpeciesList[i]->SpeName;
            logger->logMsg(nmfConstants::Normal,"MSVPA: "+msg);
            outputProgressLabel(loopPrefix+": "+msg);

            switch (MSVPASpeciesList[i]->SVPAType) {

                case 0: // cohort analysis/partial SVPA
                    logger->logMsg(nmfConstants::Normal,"MSVPA: Calling svpaApi.CandSVPA...");

                    // Pack input and output arg structs
                    CandSVPAArgsIn.databasePtr           = databasePtr;
                    CandSVPAArgsIn.CohortAnalysisGuiData = CohortAnalysisGuiData;
                    CandSVPAArgsIn.FirstYear             = FirstYear;
                    CandSVPAArgsIn.LastYear              = LastYear;
                    CandSVPAArgsIn.FirstCatchAge         = MSVPASpeciesList[i]->FirstCatAge;
                    CandSVPAArgsIn.LastCatchAge          = MSVPASpeciesList[i]->LastCatAge;
                    CandSVPAArgsIn.Plus                  = MSVPASpeciesList[i]->PlusAge;
                    CandSVPAArgsIn.MaxAge                = MSVPASpeciesList[i]->MaxAge;
                    CandSVPAArgsIn.SpeIndex              = MSVPASpeciesList[i]->DBSpeIndex;
                    CandSVPAArgsIn.SSVPAName             = MSVPASpeciesList[i]->SVPAName;
                    CandSVPAArgsIn.M_NaturalMortality    = tmpM;
                    CandSVPAArgsOut.CV_CoeffVariation    = CV;
                    CandSVPAArgsOut.SE_StandardError     = SE;
                    CandSVPAArgsOut.NumIterations        = NIt;
                    CandSVPAArgsOut.Abundance            = tmpN;
                    CandSVPAArgsOut.F_FishingMortality   = tmpF;

                    //
                    // Run the CandSVPA model
                    //
                    svpaApi.CandSVPA( CandSVPAArgsIn, CandSVPAArgsOut );

                    // Unpack the output arg struct
                    CV   = CandSVPAArgsOut.CV_CoeffVariation;
                    SE   = CandSVPAArgsOut.SE_StandardError;
                    NIt  = CandSVPAArgsOut.NumIterations;
                    tmpN = CandSVPAArgsOut.Abundance;
                    tmpF = CandSVPAArgsOut.F_FishingMortality;

                    break;

                case 1: //  Full SVPA
                    logger->logMsg(nmfConstants::Normal,"MSVPA: Calling svpaApi.Full_SVPA...");
                    NumCatchYears = LastYear-FirstYear+1;
                    NumCatchAges  = MSVPASpeciesList[i]->LastCatAge - MSVPASpeciesList[i]->FirstCatAge+1;
                    nmfUtils::initialize(InitialSelectivity, NumCatchAges);
                    nmfUtils::initialize(CatchMatrix,        NumCatchYears, NumCatchAges);

                    fields   = {"Catch"};
                    queryStr = "SELECT Catch FROM " + nmfConstantsMSVPA::TableSpeCatch +
                               " WHERE SpeIndex = " + std::to_string(MSVPASpeciesList[i]->DBSpeIndex);
                    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
                    if (dataMap["Catch"].size() != NumCatchYears*NumCatchAges) {
                        std::cout << "Error: SpeCatch size of " << dataMap["Catch"].size() <<
                                     " different from NumCatchYears*NumCatchAges (" <<NumCatchYears <<
                                     " x " << NumCatchAges << ") of " << NumCatchYears*NumCatchAges <<
                                     std::endl;
                        std::cout << queryStr << "\n" << std::endl;
                        return false;
                    }
                    m = 0;
                    for (unsigned int i = 0; i < NumCatchYears; ++i) {
                        for (unsigned int j = 0; j < NumCatchAges; ++j) {
                            CatchMatrix(i,j) = std::stod(dataMap["Catch"][m++]);
                        }
                    }
                    fields   = {"TermF","RefAge"};
                    queryStr = "SELECT TermF,RefAge FROM " + nmfConstantsMSVPA::TableSpeSSVPA +
                               " WHERE SpeIndex=" + std::to_string(MSVPASpeciesList[i]->DBSpeIndex);
                    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields, nmfConstants::nullsAsEmptyString);
                    if (dataMap["TermF"].size() > 0) {
                        FullF  = (dataMap["TermF"][0].empty())  ? 0.0 : std::stod(dataMap["TermF"][0]);
                        RefAge = (dataMap["RefAge"][0].empty()) ? 0   : std::stod(dataMap["RefAge"][0]);
                    }

                    // Pack input and output arg structs
                    FullSVPAArgsIn.InitialSelectivity  = InitialSelectivity;
                    FullSVPAArgsIn.MaxAge              = MSVPASpeciesList[i]->MaxAge;
                    FullSVPAArgsIn.FirstCatchAge       = MSVPASpeciesList[i]->FirstCatAge;
                    FullSVPAArgsIn.NumCatchAges        = NumCatchAges;
                    FullSVPAArgsIn.isPlusClass         = MSVPASpeciesList[i]->PlusAge;
                    FullSVPAArgsIn.FullF               = FullF;
                    FullSVPAArgsIn.ReferenceAge        = RefAge;
                    FullSVPAArgsIn.NumCatchYears       = NumCatchYears;
                    FullSVPAArgsOut.CatchMatrix        = CatchMatrix;
                    FullSVPAArgsOut.CV_CoeffVariation  = CV;
                    FullSVPAArgsOut.SE_StandardError   = SE;
                    FullSVPAArgsOut.NumIterations      = NIt;
                    FullSVPAArgsOut.Abundance          = tmpN;
                    FullSVPAArgsOut.M_NaturalMortality = tmpM;
                    FullSVPAArgsOut.F_FishingMortality = tmpF;

                    //
                    // Run the Full_SVPA model
                    //
                    svpaApi.Full_SVPA( FullSVPAArgsIn, FullSVPAArgsOut );

                    // Unpack output arg struct
                    CV   = FullSVPAArgsOut.CV_CoeffVariation;
                    SE   = FullSVPAArgsOut.SE_StandardError;
                    NIt  = FullSVPAArgsOut.NumIterations;
                    tmpN = FullSVPAArgsOut.Abundance;
                    tmpM = FullSVPAArgsOut.M_NaturalMortality;
                    tmpF = FullSVPAArgsOut.F_FishingMortality;

                    break;

                case 2: // Effort tuned VPA.......
                    logger->logMsg(nmfConstants::Normal,"MSVPA: Calling svpaApi.Effort_Tune_VPA...");

                    NumCatchYears = LastYear-FirstYear+1;
                    NumCatchAges  = MSVPASpeciesList[i]->LastCatAge -
                                    MSVPASpeciesList[i]->FirstCatAge + 1;

                    FleetData.resize(boost::extents[nmfConstants::MaxNumberFleets]
                                                   [nmfConstants::MaxNumberSpecies]
                                                   [nmfConstants::MaxNumberYears]
                                                   [nmfConstants::MaxNumberAges+1]);
                    EffortTuneArgsIn.FleetData.resize(boost::extents[nmfConstants::MaxNumberFleets]
                            [nmfConstants::MaxNumberSpecies]
                            [nmfConstants::MaxNumberYears]
                            [nmfConstants::MaxNumberAges+1]);

                    fields    = {"SpeIndex","NFleets"};
                    queryStr  = "SELECT SpeIndex,NFleets FROM " + nmfConstantsMSVPA::TableSpeSSVPA +
                                " WHERE SpeIndex = " + std::to_string(MSVPASpeciesList[i]->DBSpeIndex) +
                                " AND SSVPAName = '" + MSVPASpeciesList[i]->SVPAName + "'";
                    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
                    if (dataMap["SpeIndex"].size() == 0) {
                        return false;
                    }
                    NumFleets = std::stoi(dataMap["NFleets"][0]);


                    fields   = {"SpeIndex"};
                    queryStr = "SELECT SpeIndex FROM " + nmfConstantsMSVPA::TableSpeTuneEffort +
                               " WHERE SpeIndex = " + std::to_string(MSVPASpeciesList[i]->DBSpeIndex);
                    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
                    if (dataMap["SpeIndex"].size() == 0) {
                        fields2   = {"Catch"};
                        queryStr2 = "SELECT Catch FROM " + nmfConstantsMSVPA::TableSpeCatch +
                                    " WHERE SpeIndex = " + std::to_string(MSVPASpeciesList[i]->DBSpeIndex);
                        dataMap2  = databasePtr->nmfQueryDatabase(queryStr2, fields2);
                        for (unsigned int i = 0; i < NumCatchYears; ++i) {
                            for (unsigned int j = 0; j < NumCatchAges; ++j) {
                                FleetData[0][MSVPASpeciesList[i]->DBSpeIndex][i][j+1] =
                                        std::stod(dataMap2["Catch"][m++]);
                            }
                        }
                    } else {

                        // Load Fleet data into FleetData multiarray
                        m = 0;
                        fields   = {"SpeIndex","Year","Fleet","Effort"};
                        queryStr = "SELECT SpeIndex,Year,Fleet,Effort FROM " + nmfConstantsMSVPA::TableSpeTuneEffort +
                                   " WHERE SpeIndex = " + std::to_string(MSVPASpeciesList[i]->DBSpeIndex) +
                                   " ORDER BY SpeIndex,Fleet,Year,Effort";
                        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
                        for (int i=0; i<NumFleets; ++i) {
                            for (unsigned int j=0; j<NumCatchYears; ++j) {
                                FleetData[i][MSVPASpeciesList[i]->DBSpeIndex][j][0] =
                                        std::stod(dataMap["Effort"][m++]);
                            }
                        }

                        // Load Catch data into FleetData multiarray
                        m = 0;
                        fields   = {"SpeIndex","Year","Fleet","Age","Catch"};
                        queryStr = "SELECT SpeIndex,Year,Fleet,Age,Catch FROM " + nmfConstantsMSVPA::TableSpeTuneCatch +
                                   " WHERE SpeIndex = " + std::to_string(MSVPASpeciesList[i]->DBSpeIndex) +
                                   " ORDER BY SpeIndex,Fleet,Year,Age,Catch";
                        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);

                        if (dataMap["Catch"].size() == NumFleets*NumCatchYears*NumCatchAges) {
                            for (int i=0; i<NumFleets; ++i) {
                                for (unsigned int j=0; j<NumCatchYears; ++j) {
                                    for (unsigned int age=0; age<NumCatchAges; ++age) {
                                        FleetData[i][MSVPASpeciesList[i]->DBSpeIndex][j][age+1] =
                                                std::stod(dataMap["Catch"][m++]);
                                    } // end for age
                                } // end for j
                            } // end for i
                        } else {
                            std::cout << "Warning nmfMSVPA: SpeTuneCatch size of " << dataMap["Catch"].size() <<
                                         " didn't match computed size of " << NumFleets*NumCatchYears*NumCatchAges << std::endl;
                        }


                    }

                    // Pack arg structures
                    EffortTuneArgsIn.databasePtr         = databasePtr;
                    EffortTuneArgsIn.EffortTunedGuiData  = EffortTunedGuiData;
                    EffortTuneArgsIn.FleetData           = FleetData;
                    EffortTuneArgsIn.FirstYear           = FirstYear;
                    EffortTuneArgsIn.LastYear            = LastYear;
                    EffortTuneArgsIn.FirstCatchAge       = MSVPASpeciesList[i]->FirstCatAge;
                    EffortTuneArgsIn.LastCatchAge        = MSVPASpeciesList[i]->LastCatAge;
                    EffortTuneArgsIn.Plus                = MSVPASpeciesList[i]->PlusAge;
                    EffortTuneArgsIn.MaxAge              = MSVPASpeciesList[i]->MaxAge;
                    EffortTuneArgsIn.SpeIndex            = MSVPASpeciesList[i]->DBSpeIndex;
                    EffortTuneArgsIn.SSVPAName           = MSVPASpeciesList[i]->SVPAName;
                    EffortTuneArgsOut.Abundance          = tmpN;
                    EffortTuneArgsOut.M_NaturalMortality = tmpM;
                    EffortTuneArgsOut.F_FishingMortality = tmpF;

                    //
                    // Run an Effort_Tune_VPA
                    //
                    svpaApi.Effort_Tune_VPA( EffortTuneArgsIn, EffortTuneArgsOut );

                    // unpack return values
                    tmpN = EffortTuneArgsOut.Abundance;
                    tmpM = EffortTuneArgsOut.M_NaturalMortality;
                    tmpF = EffortTuneArgsOut.F_FishingMortality;

                    break;

                case 3: //  XSA
                    logger->logMsg(nmfConstants::Normal,"MSVPA: Calling svpaApi.nmfXSA...");

                    // pack input and output arguments for passing to XSA run
                    XSAArgsIn.databasePtr         = databasePtr;
                    XSAArgsIn.XSAGuiData          = XSAGuiData;
                    XSAArgsIn.FirstYear           = FirstYear;
                    XSAArgsIn.LastYear            = LastYear;
                    XSAArgsIn.SpeIndex            = MSVPASpeciesList[i]->DBSpeIndex;
                    XSAArgsIn.SSVPAName           = MSVPASpeciesList[i]->SVPAName;
                    XSAArgsIn.M_NaturalMortality  = tmpM;
                    XSAArgsOut.Abundance          = tmpN;
                    XSAArgsOut.F_FishingMortality = tmpF;
                    XSAArgsOut.XSAWarn            = XSAWarn;

                    //
                    // Run XSA
                    //
                    svpaApi.XSA(XSAArgsIn, XSAArgsOut);

                    // Unpack output args
                    tmpN    = XSAArgsOut.Abundance;
                    tmpF    = XSAArgsOut.F_FishingMortality;
                    XSAWarn = XSAArgsOut.XSAWarn;

                    break;

            } // end switch

            ZCheck = 0;
            // Put Ns and Fs into a full matrix and include unit conversion
            for (int j = 0; j < NYears; ++j) {
                for (int k = MSVPASpeciesList[i]->FirstCatAge; k <= MSVPASpeciesList[i]->LastCatAge; ++k) {

                    N[i][j][k] = tmpN(j, k) * MSVPASpeciesList[i]->CatUnits;
                    F[i][j][k] = tmpF(j, k);
                    tmpF(j, k) = 0; // need to clear this for each iteration
                    Z[i][j][k] = F[i][j][k] + M[i][j][k];
                    ZCheck  += Z[i][j][k]; // Added check in case user forgot to give Mortality values;
                } // end for k
            } // end for j

            if (ZCheck == 0) {
                logger->logMsg(nmfConstants::Error,"Error: Total Mortality is 0. Please enter values for time-based Mortality.");
                return false;
            }
        } // end for i
        logger->logMsg(nmfConstants::Normal,"MSVPA:  --- SSVPA Run End ---");

        //  END OF SVPA RUNS

        cmd  = "INSERT INTO " + nmfConstantsMSVPA::TableMSVPASeasBiomass +
               " (MSVPAName, Season, SpeName, SpeType, Year, Age, Biomass, " +
               "AnnBiomass, SeasAbund, AnnAbund, SeasF, SeasM1, SeasWt, SeasSize ) VALUES ";
        logger->logMsg(nmfConstants::Normal,"MSVPA: Calculating Predator and Prey Biomass");
        // CALCULATE BIOMASSES FOR MSVPA SPECIES BY SEASON
        updateDB = false;
        for (int j = 0; j < NMSVPASpe; ++j) {

            // Delete previously saved biomasses in each iteration
            // RecSet.LockType = adLockOptimistic // RSK check this
            queryStr = "DELETE FROM " + nmfConstantsMSVPA::TableMSVPASeasBiomass +
                       " WHERE MSVPAName='"	+ MSVPAName +
                       "' AND SpeName='" + MSVPASpeciesList[j]->SpeName + "'";
            errorMsg = databasePtr->nmfUpdateDatabase(queryStr);
            if (nmfUtilsQt::isAnError(errorMsg)) {
                logger->logMsg(nmfConstants::Error,"Error: "+queryStr+"; "+errorMsg);
                //nmfUtils::printError(queryStr, errorMsg);
            }

            // RecSet.LockType = adLockBatchOptimistic // RSK check this

            for (int i = 0; i < NSeas; ++i) {
                SeasStart = SeasLen(i, 0);
                SeasEnd   = SeasLen(i, 1);

                for (int k = 0; k < NYears; ++k) {
                    for (int l = MSVPASpeciesList[j]->FirstCatAge; l <= MSVPASpeciesList[j]->LastCatAge; ++l) {
//std::cout << "i,k,l: " << i << ", " << k << ", " << l << std::endl;

                        updateDB = true;
                        tempValue = Z[j][k][l];
                        if (tempValue == 0 ) {
                            logger->logMsg(nmfConstants::Error,"Error MSVPA(8): Found Z[" +
                                    std::to_string(j) + "][" +
                                    std::to_string(k) + "][" +
                                    std::to_string(l) + "] == 0. This will cause a divide by 0 error for AvgAbund.");
                            return false;
                        }
                        //Calculate the correction factor for abundance to get to midpoint numbers in interval
                        AvgAbund = (std::exp(-tempValue * SeasStart) - std::exp(-tempValue * SeasEnd))
                                / ((SeasEnd - SeasStart) * tempValue);
//std::cout << "AvgAbund,Z: " << AvgAbund << ", " << Z[j][k][l] << std::endl;

                        // Then multiply by mid-point Weight at age to get biomass in interval
                        SeasBiomass[i][j][k][l] = N[j][k][l] * AvgAbund	* SeasWtAge[i][j][k][l];

                        field0  = std::string("\"")+MSVPAName+"\"";
                        field1  = std::to_string(i);
                        field2  = std::string("\"")+MSVPASpeciesList[j]->SpeName+"\"";
                        field3  = std::to_string(MSVPASpeciesList[j]->SpeType);
                        field4  = std::to_string(k);
                        field5  = std::to_string(l);
                        field6  = std::to_string(SeasBiomass[i][j][k][l]);
                        field7  = std::to_string(N[j][k][l] * WtAtAge[j][k][l]);
                        field8  = std::to_string(N[j][k][l] * AvgAbund);
                        field9  = std::to_string(N[j][k][l]);
                        field10 = std::to_string(F[j][k][l] * (SeasLen(i, 2) / 365.0));
                        field11 = std::to_string(M1[j][k][l] * (SeasLen(i, 2) / 365.0));
                        field12 = std::to_string(SeasWtAge[i][j][k][l]);
                        field13 = std::to_string(SeasSizeAge[i][j][k][l]);

                        cmd += "("  + field0  + "," + field1  + "," + field2  + "," + field3 +
                                "," + field4  + "," + field5  + "," + field6  + "," + field7 +
                                "," + field8  + "," + field9  + "," + field10 + "," + field11 +
                                "," + field12 + "," + field13 + "), ";

                    } // end for l
                } // end for k
            } // end for i

        } // end for j
//std::cout << "---" << std::endl;

        // Update the database
        if (updateDB) {
            // Remove last comma and space from string
            cmd = cmd.substr(0, cmd.size() - 2);
//std::cout << "the cmd: " << cmd << std::endl;

            errorMsg = databasePtr->nmfUpdateDatabase(cmd);
            if (nmfUtilsQt::isAnError(errorMsg)) nmfUtils::printError("Section: CALCULATE BIOMASSES FOR MSVPA SPECIES BY SEASON", errorMsg);
        }
        // END OF SEASONAL BIOMASS CALCULATION LOOP >>>>>>>>>

        updateProgress(progressVal++, loopPrefix + ":  Calculating suitable prey biomass for MSVPA prey");

        // Calculate suitable prey biomass for msvpa prey for all msvpa predators
        // Clear previously stored suitable biomasses for MSVPAPrey Types.  Still want to retain other prey calculations.
        logger->logMsg(nmfConstants::Normal,"MSVPA: Calculating Suitable Prey Biomass for Predators");
        outputProgressLabel(loopPrefix+": "+"Calculating Suitable Prey Biomass for Predators");

        for (int i = 0; i < NMSVPASpe; ++i) {
            queryStr = "DELETE FROM " + nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
                       " WHERE MSVPAName = '" + MSVPAName +
                       "' AND PreyName = '"	+ MSVPASpeciesList[i]->SpeName + "';";
            errorMsg = databasePtr->nmfUpdateDatabase(queryStr);
            if (nmfUtilsQt::isAnError(errorMsg)) nmfUtils::printError(queryStr, errorMsg);
        }

        // Updating MSVPASuitPreyBiomass
        cmd = "INSERT INTO " + nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
              " (MSVPAname, PredName, PredAge, PreyName, PreyAge, Year, Season, SuitPreyBiomass ) VALUES ";
        updateDB = false;
        MaxVal = 0;
        for (int i = 0; i < NSeas; ++i)
        {
            SpeCount = 0;
            for (int j = 0; j < NMSVPASpe; ++j) {

                if (MSVPASpeciesList[j]->SpeType == 0) {

                    // Calculate suitable biomass for each age class by year, prey type, prey age
                    for (int k = MSVPASpeciesList[j]->FirstCatAge; k <= MSVPASpeciesList[j]->LastCatAge; ++k) {

                        SizeSelAlpha = MSVPAPredCoeff[SpeCount][k][2];
                        SizeSelBeta  = MSVPAPredCoeff[SpeCount][k][3];
                        for (int l = 0; l < NMSVPASpe; ++l) {
                            // Then get the type pref for that prey type for the predator species/age

                            TypePref = MSVPAPredTypePref[SpeCount][k][l];
                            if (TypePref != 0) {
                                // Then get spatial overlap for the prey type, and season

                                SpaceOverlap = SeasMSVPAPredSpaceO[i][SpeCount][k][l];
                                if (SpaceOverlap != 0) {
                                    for (int mm = 0; mm < NYears; ++mm) {
                                        PredSize = SeasSizeAge[i][j][mm][k];
                                        if (PredSize == 0 ) {
                                            logger->logMsg(nmfConstants::Error,"Error MSVPA(9): Found PredSize == 0. This will cause a divide by 0 error.");
                                            return false;
                                        }
                                        for (int nn = MSVPASpeciesList[l]->FirstCatAge; nn <= MSVPASpeciesList[l]->LastCatAge;	++nn) {
                                            // Get prey size
                                            PreySize = SeasSizeAge[i][l][mm][nn];
                                            SizeSelCoeff = 0.0;
                                            if ((PreySize/PredSize) > 1.0) {
                                                SizeSelCoeff = 0.0;
                                            } else {
                                                MaxVal = (((PreySize+0.05)/PredSize) > 1.0) ? 1.0 : (PreySize + 0.05)/PredSize;
                                                SizeSelCoeff = nmfUtilsStatistics::BetaI(SizeSelAlpha,SizeSelBeta,MaxVal) -
                                                               nmfUtilsStatistics::BetaI(SizeSelAlpha,SizeSelBeta,(PreySize-0.05)/PredSize);
                                            } // end if
                                            tmpSuitBiomass(mm, nn) = SeasBiomass[i][l][mm][nn] * TypePref * SpaceOverlap * SizeSelCoeff;
                                            // SAVE NON ZERO VALUES TO THE DATABASE.
                                            if (tmpSuitBiomass(mm, nn) != 0) {
                                                updateDB = true;
                                                field0 = std::string("\"")+MSVPAName+"\"";
                                                field1 = std::string("\"")+MSVPASpeciesList[j]->SpeName+"\"";
                                                field2 = std::to_string(k);
                                                field3 = std::string("\"")+MSVPASpeciesList[l]->SpeName+"\"";
                                                field4 = std::to_string(nn);
                                                field5 = std::to_string(mm);
                                                field6 = std::to_string(i);
                                                field7 = std::to_string(tmpSuitBiomass(mm, nn));
                                                cmd += "(" + field0 + "," + field1 + "," + field2
                                                        + "," + field3 + "," + field4 + "," + field5
                                                        + "," + field6 + "," + field7 + "), ";

                                            } // end if
                                        } // end for nn
                                    } // end for mm

                                } // end if SpaceOverlap
                            } // end if TypePref
                        } // end for l
                    } // end for k
                    ++SpeCount;
                } // end if
            } // end for j
        } // end for i

        // Update the database
        if (updateDB) {
            // Remove last comma and space from string
            cmd = cmd.substr(0,cmd.size()-2);
            errorMsg = databasePtr->nmfUpdateDatabase(cmd);
            if ( nmfUtilsQt::isAnError(errorMsg) ) nmfUtils::printError("Section: Calculate suitable prey biomass",errorMsg);
        }
        // END OF SUITABLE PREY BIOMASS CALCULATION LOOP >>>>>>>
        //std::cout << "END OF SUITABLE PREY BIOMASS CALCULATION LOOP" << std::endl;
        //databasePtr->nmfSaveDatabase();

        // SuitPreyBiomass a little different here
// First time I get:      634155.13872
// First time Lance gets: 634155.137114086

        updateProgress(progressVal++, loopPrefix + ":  Calculating suitable prey biomass for other predators");

        // CALCULATING SUITABLE PREY BIOMASS FOR OTHER PREDATORS EATING MSVPA SPECIES...
        logger->logMsg(nmfConstants::Normal,"MSVPA: Calculating Suitable Prey Biomass for Other Predators...");
        outputProgressLabel(loopPrefix+": "+"Calculating Suitable Prey Biomass for Other Predators...");

        if (NOtherPreds > 0)
        {
            // Delete any previously stored suitable biomasses for the other predators.
            for (int i = 0; i < NOtherPreds; ++i) {
                for (int j = 0; j < NMSVPASpe; ++j) {
                    queryStr = "DELETE FROM " + nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
                               " WHERE MSVPAName = '" + MSVPAName +
                               "' and PredName = '"	+ OthPredList[i]->SpeName +
                               "' AND PreyName = '"	+ MSVPASpeciesList[j]->SpeName + "'";
                    errorMsg = databasePtr->nmfUpdateDatabase(queryStr);
                    if (nmfUtilsQt::isAnError(errorMsg)) nmfUtils::printError(queryStr, errorMsg);
                } // end for j
            }// end for i

            cmd = "INSERT INTO " + nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
                  " (MSVPAname, PredName, PredAge, PreyName, PreyAge, Year, Season, SuitPreyBiomass) VALUES ";

            updateDB = false;
            for (int i = 0; i < NSeas; ++i) {
                for (int j = 0; j < NOtherPreds; ++j) {
                    NCats = (OthPredList[j]->SizeStructure == 0) ? 0 : OthPredList[j]->NumSizeCats;
                    for (int k = 0; k < NCats; ++k) {
                        if (NCats == 0) {
                            MinPredSize = OthPredList[j]->MinSize;
                            MaxPredSize = OthPredList[j]->MaxSize;
                        } else {
                            MinPredSize = OthPredList[j]->SizeCatMins[k];
                            MaxPredSize = OthPredList[j]->SizeCatMaxs[k];
                        }

                        SizeSelAlpha = OtherPredCoeff[j][k][2];
                        SizeSelBeta  = OtherPredCoeff[j][k][3];

                        for (int l = 0; l < NMSVPASpe; ++l) {
                            fields = {"PrefVal"};
                            queryStr = "SELECT PrefVal FROM " + nmfConstantsMSVPA::TableMSVPAprefs +
                                       " WHERE MSVPAname = '" + MSVPAName +
                                       "' AND SpeName = '" + OthPredList[j]->SpeName +
                                       "' AND Age = " + std::to_string(k) +
                                       "  AND PreyName = '" + MSVPASpeciesList[l]->SpeName + "'";
                            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                            TypePref = std::stod(dataMap["PrefVal"][0]);

                            if (TypePref != 0) {
                                // Then get the spatial overlap similarly
                                fields = {"SpOverlap"};
                                queryStr = "SELECT SpOverlap FROM " + nmfConstantsMSVPA::TableMSVPASpaceO +
                                           " WHERE MSVPAname = '" + MSVPAName +
                                           "' AND SpeName = '" + OthPredList[j]->SpeName +
                                           "' AND Age = " + std::to_string(k) +
                                           "  AND PreyName = '" + MSVPASpeciesList[l]->SpeName +
                                           "' AND Season = " + std::to_string(i+1);
                                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                                SpaceOverlap = (dataMap["SpOverlap"].size() > 0) ? std::stod(dataMap["SpOverlap"][0]) : 0;
                                if (SpaceOverlap != 0) {
                                    // Calculate size selection
                                    for (int mm = 0; mm<NYears; ++mm) {
                                        for (int nn = MSVPASpeciesList[l]->FirstCatAge; nn <= MSVPASpeciesList[l]->LastCatAge; ++nn) {
                                            PreySize = SeasSizeAge[i][l][mm][nn];
                                            SumSuitBM = 0;

                                            // integrate size pref across predator size for the category.
                                            for (int X = MinPredSize; X<=MaxPredSize; ++X) {
                                                if (X == 0 ) {
                                                    logger->logMsg(nmfConstants::Error,"Error MSVPA(10): Found MinPredSize == 0. This will cause a divide by 0 error.");
                                                    return false;
                                                }
                                                if (( PreySize/X) > 1) {
                                                    SizeSelCoeff = 0;
                                                } else {
                                                    MaxVal = (((PreySize + 0.05) / X) > 1) ? 1 : (PreySize + 0.05) / X;
                                                    SizeSelCoeff = nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, MaxVal) -
                                                                   nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, (PreySize-0.05)/X);
                                                } // end if
                                                SumSuitBM += (SeasBiomass[i][l][mm][nn] * TypePref * SpaceOverlap * SizeSelCoeff);
                                            } // end for X
                                            if (MaxPredSize == MinPredSize) {
                                                logger->logMsg(nmfConstants::Error,"Error MSVPA(11): Found MaxPredSize == MinPredSize (of " +
                                                        std::to_string(MaxPredSize) +
                                                        "). This will cause a divide by 0 error.");
                                                return false;
                                            }
                                            tmpSuitBiomass(mm, nn) = SumSuitBM / (MaxPredSize - MinPredSize);
                                            tempValue = tmpSuitBiomass(mm, nn);
                                            if (tempValue != 0) {
                                                updateDB = true;
                                                field0 = std::string("\"")+MSVPAName+"\"";
                                                field1 = std::string("\"")+OthPredList[j]->SpeName+"\"";
                                                field2 = std::to_string(k);
                                                field3 = std::string("\"")+MSVPASpeciesList[l]->SpeName+"\"";
                                                field4 = std::to_string(nn);
                                                field5 = std::to_string(mm);
                                                field6 = std::to_string(i);
                                                field7 = std::to_string(tempValue);
                                                cmd += "(" + field0 + "," + field1 + "," + field2
                                                           + "," + field3 + "," + field4 + "," + field5
                                                           + "," + field6 + "," + field7 + "), ";
                                            } // end if
                                        } // end for nn
                                    } // end for mm
                                } // end if SpaceOverlap

                            } // end if TypePref
                        } // end for l
                    } // end for k
                } // end for j
            } // end for i

              // Update the database
            if (updateDB) {
                // Remove last comma and space from string
                cmd = cmd.substr(0, cmd.size() - 2);
                errorMsg = databasePtr->nmfUpdateDatabase(cmd);
                if (nmfUtilsQt::isAnError(errorMsg)) nmfUtils::printError("Section: CALCULATE SUITABLE PREY BIOMASS", errorMsg);
            }
        } // end if NOtherPreds
        // END SUITABLE BIOMASS CALCULATIONS FOR OTHER PREDATORS EATING MSVPA SPECIES

// Slight difference here with SuitPreyBiomass as well
// I get:      78379.6599505
// Lance gets: 78379.6597096519

// GOOD DATA HERE


        //databasePtr->nmfSaveDatabase();

        cmd  = "REPLACE INTO " + nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
               " (MSVPAName,PredName,PredAge,PreyName,PreyAge,Year,Season,";
        cmd += "SuitPreyBiomass,PropDiet,BMConsumed,PreyEDens) values ";

        cmd2  = "REPLACE INTO " + nmfConstantsMSVPA::TableMSVPASeasBiomass +
                " (MSVPAName,SpeName,Season,SpeType,Year,Age,Biomass,";
        cmd2 += "TotalPredCons,SeasM2,AnnAbund,SeasAbund,SeasF,SeasM1,";
        cmd2 += "SeasWt,SeasSize,AnnBiomass,StomCont) values ";

        updateProgress(progressVal++, loopPrefix + ":  Calculating diet and consumption data for MSVPA predators");

        // BEGIN CALCULATING DIETS AND CONSUMPTION FOR MSVPA PREDATORS
        // Begin calculating diets and consumption for MSVPA predators...
        logger->logMsg(nmfConstants::Normal,"MSVPA: Calculating Predator diets...");
        outputProgressLabel(loopPrefix+": "+"Calculating Predator diets....");

        SpeCount = 0;
        if (ReadRunStopFile() == "Stop") {
            userHalted = true;
            break;
        }
        for (int i = 0; i<NMSVPASpe; ++i) {
            if (MSVPASpeciesList[i]->SpeType == 0) {

                fields = {"Year","Season","PredName","PredAge","TotSBM"};
                queryStr = "SELECT Year, Season, PredName, PredAge, Sum(SuitPreyBiomass) as TotSBM FROM " +
                            nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
                           " WHERE MSVPAname = '" + MSVPAName +
                           "' AND PredName = '" + MSVPASpeciesList[i]->SpeName +
                           "' GROUP By PredName, PredAge, Year, Season";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                if (dataMap.size() == 0) {
                    logger->logMsg(nmfConstants::Error,queryStr);
                    return false;
                }
                int m =0;
                for (int j = MSVPASpeciesList[i]->FirstCatAge; j<=MSVPASpeciesList[i]->LastCatAge; ++j) {
                    for (int k = 0; k<NYears; ++k) {
                        for (int l = 0; l<NSeas; ++l) {
                            if (j == std::stoi(dataMap["PredAge"][m])) {
                                AnnTotSuitBiomass[SpeCount][j][k][l] = std::stod(dataMap["TotSBM"][m++]);
                            }
                        } // end for l
                    } // end for k
                } // end for j

                // Get the average (across years) suitable biomass by pred, age, season

                for (int j = MSVPASpeciesList[i]->FirstCatAge; j<=MSVPASpeciesList[i]->LastCatAge; ++j) {
                    for (int k = 0; k<NSeas; ++k) {
                        AvgSuitBM[SpeCount][j][k] = 0;
                        for (int l = 0; l < NYears; ++l) {
                            AvgSuitBM[SpeCount][j][k] += AnnTotSuitBiomass[SpeCount][j][l][k];
                        } // end for l
                        AvgSuitBM[SpeCount][j][k] /= NYears;
                    } // end for k
                } // end for j

                // Calculate the stomach contents correction for each year.
                // This is the feeding functional response
                for (int j = MSVPASpeciesList[i]->FirstCatAge; j<=MSVPASpeciesList[i]->LastCatAge; ++j) {
                    for (int k = 0; k<NSeas; ++k) {
                        for (int l = 0; l<NYears; ++l) {
                            tempValue = AvgSuitBM[SpeCount][j][k];
                            if (tempValue == 0) {
                                logger->logMsg(nmfConstants::Error,"Error MSVPA(12): Found AvgSuitBM[" +
                                        std::to_string(SpeCount) + "][" +
                                        std::to_string(j) + "][" +
                                        std::to_string(k) + "] == 0. This will cause a divide by 0 error. Skipping corresponding StomCorrection.");
//                                return false;
                            }
                            if (tempValue != 0) {
                                StomCorrection[SpeCount][j][l][k] = MSVPAPredStomCont[SpeCount][j][k] +
                                        ( std::log(AnnTotSuitBiomass[SpeCount][j][l][k] / tempValue) *
                                          MSVPAPredStomCont[SpeCount][j][k]);

                                if (StomCorrection[SpeCount][j][l][k] < (0.1 * MSVPAPredStomCont[SpeCount][j][k])) {
                                    StomCorrection[SpeCount][j][l][k] = 0.1 * MSVPAPredStomCont[SpeCount][j][k];
                                }

                                if (StomCorrection[SpeCount][j][l][k] > (3 * MSVPAPredStomCont[SpeCount][j][k])) {
                                    StomCorrection[SpeCount][j][l][k] = 3 * MSVPAPredStomCont[SpeCount][j][k];
                                }
                            }

                        } // end for l
                    } // end for k
                } // end for j

                // Then calculate seasonal diet composition and predator consumption at the same time and save it...
                //fields = {"Biomass","TotalPredCons","StomCont"};
                fields = {"MSVPAName","SpeName","Season","SpeType","Year","Age","Biomass","TotalPredCons","SeasM2","AnnAbund","SeasAbund","SeasF","SeasM1","SeasWt","SeasSize","AnnBiomass","StomCont"};
                queryStr = "SELECT MSVPAName,SpeName,Season,SpeType,Year,Age,Biomass,TotalPredCons,SeasM2,AnnAbund,SeasAbund,SeasF,SeasM1,SeasWt,SeasSize,AnnBiomass,StomCont ";
                queryStr += " FROM " + nmfConstantsMSVPA::TableMSVPASeasBiomass +
                            " WHERE MSVPAName = '" + MSVPAName +
                            "' AND SpeName = '" + MSVPASpeciesList[i]->SpeName +
                            "' ORDER BY Age, Year, Season";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                logger->logMsg(nmfConstants::Normal,"MSVPA: Calculating diet and consumption for predator: " + MSVPASpeciesList[i]->SpeName);
                m = -1;
                if (ReadRunStopFile() == "Stop") {
                    userHalted = true;
                    break;
                }

                for (int j = MSVPASpeciesList[i]->FirstCatAge; j<=MSVPASpeciesList[i]->LastCatAge; ++j) { // RSK Bug - age doesn't start at 0
std::cout << "."; // << std::flush;
                    for (int k = 0; k<NYears; ++k) {
                        for (int l = 0; l<NSeas; ++l) {
                            ++m;
                            // Calculate Evacuation rate and annual consumption correcting for food availability
                            EvacRate = 24.0 * MSVPAPredCoeff[SpeCount][j][0] * std::exp(MSVPAPredCoeff[SpeCount][j][1] * SeasTemp(k,l));
                            PredConsumption[SpeCount][j][k][l] = EvacRate * SeasLen(l,2) * StomCorrection[SpeCount][j][k][l];
                            TotConsumption[SpeCount][j][k][l] = std::stod(dataMap["Biomass"][m]) * PredConsumption[SpeCount][j][k][l];
                            tempValue = TotConsumption[SpeCount][j][k][l];
                            dataMap["TotalPredCons"][m] = (! std::isnan(tempValue)) ? std::to_string(tempValue) : "0.0";
                            tempValue = StomCorrection[SpeCount][j][k][l];
                            dataMap["StomCont"][m]      = (! std::isnan(tempValue)) ? std::to_string(tempValue) : "0.0";

                            fields3 = {"MSVPAName","PredName","PredAge","PreyName","PreyAge","Year","Season","SuitPreyBiomass","PropDiet","BMConsumed","PreyEDens"};
                            queryStr = "SELECT MSVPAName,PredName,PredAge,PreyName,PreyAge,Year,Season,SuitPreyBiomass,PropDiet,BMConsumed,PreyEDens FROM " +
                                        nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
                                       " WHERE MSVPAname = '" + MSVPAName +
                                       "' AND PredName = '" + MSVPASpeciesList[i]->SpeName +
                                       "' AND PredAge = " + std::to_string(j) +
                                       "  AND Year = " + std::to_string(k) +
                                       "  AND Season = " + std::to_string(l);
                            dataMap3 = databasePtr->nmfQueryDatabase(queryStr, fields3);

                            numItems = dataMap3["MSVPAName"].size();
                            if (numItems != 0) {
                                for (int n=0; n<numItems; ++n) {
                                    tempValue = AnnTotSuitBiomass[SpeCount][j][k][l];
                                    if (tempValue == 0) {
                                        logger->logMsg(nmfConstants::Error,"Error MSVPA(13): Found AnnTotSuitBiomass[" +
                                                std::to_string(SpeCount) + "][" +
                                                std::to_string(j) + "][" +
                                                std::to_string(k) + "][" +
                                                std::to_string(l) + "] == 0. This will cause a divide by 0 error.");
                                        return false;
                                    }
                                    PDiet = std::stod(dataMap3["SuitPreyBiomass"][n]) / tempValue;
                                    dataMap3["PropDiet"][n]   = std::to_string(PDiet);
                                    tempValue = PDiet * TotConsumption[SpeCount][j][k][l];
                                    dataMap3["BMConsumed"][n] = (! std::isnan(tempValue)) ? std::to_string(tempValue) : "0.0";

                                    field0  = std::string("\"")+dataMap3["MSVPAName"][n]+"\"";
                                    field1  = std::string("\"")+dataMap3["PredName"][n]+"\"";
                                    field2  = dataMap3["PredAge"][n];
                                    field3  = std::string("\"")+dataMap3["PreyName"][n]+"\"";
                                    field4  = dataMap3["PreyAge"][n];
                                    field5  = dataMap3["Year"][n];
                                    field6  = dataMap3["Season"][n];
                                    field7  = dataMap3["SuitPreyBiomass"][n];
                                    field8  = dataMap3["PropDiet"][n];
                                    field9  = dataMap3["BMConsumed"][n];  // this is nan sometimes
                                    field10 = dataMap3["PreyEDens"][n];
//std::cout << field0 << "," << field1 << "," << field2 << "," << field3 << "," << field4 << ","
//             << field5 << "," << field6 << "," << field7 << "," << field8 << "," << field9 << ","
//             << field10 << std::endl;

                                    cmd +=  "(" + field0 + "," + field1 + "," + field2 + "," + field3 +
                                            "," + field4 + "," + field5 + "," + field6 + "," + field7 +
                                            "," + field8 + "," + field9 + "," + field10 + "), ";
                                } // end for n
                            } // end if

                            cmd2 +=  "( \"" + dataMap["MSVPAName"][m] + "\"" +
                                    ", \"" + dataMap["SpeName"][m] + "\"" +
                                    "," + dataMap["Season"][m] +
                                    "," + dataMap["SpeType"][m] +
                                    "," + dataMap["Year"][m] +
                                    "," + dataMap["Age"][m] +
                                    "," + dataMap["Biomass"][m] +
                                    "," + dataMap["TotalPredCons"][m] + // This can be nan sometimes
                                    "," + dataMap["SeasM2"][m] +
                                    "," + dataMap["AnnAbund"][m] +
                                    "," + dataMap["SeasAbund"][m] +
                                    "," + dataMap["SeasF"][m] +
                                    "," + dataMap["SeasM1"][m] +
                                    "," + dataMap["SeasWt"][m] +
                                    "," + dataMap["SeasSize"][m] +
                                    "," + dataMap["AnnBiomass"][m] +
                                    "," + dataMap["StomCont"][m] + "), "; // This can also be nan sometimes

//std::cout << dataMap["MSVPAName"][m] << "," << dataMap["SpeName"][m] << "," << dataMap["Season"][m] << "," <<
//                                        dataMap["SpeType"][m] << "," << dataMap["Year"][m] << "," << dataMap["Age"][m] << "," <<
//                                        dataMap["Biomass"][m] << "," << dataMap["TotalPredCons"][m] << "," << dataMap["SeasM2"][m] << "," <<
//                                        dataMap["AnnAbund"][m] << "," << dataMap["SeasAbund"][m] << "," << dataMap["SeasF"][m] << "," <<
//                                        dataMap["SeasM1"][m] << "," << dataMap["SeasWt"][m] << "," << dataMap["SeasSize"][m] <<
//                                        dataMap["AnnBiomass"][m] << "," << dataMap["StomCont"][m] << std::endl;

                        } // end for l
                    } // end for k
                } // end for j
std::cout << std::endl;
                ++SpeCount;
            } // end if
        } // end for i
//std::cout << "End MSVPA Predator Consumption Calculation." << std::endl;
//        DEBUG_MSG("After Insert1 - Tot SuitBiomass: " << n1 << ", " << s1);
//        DEBUG_MSG("After Insert2 - Tot SuitBiomass: " << n2 << ", " << s2);
//        DEBUG_MSG("After Replace - Tot SuitBiomass: " << n3 << ", " << s3);

        // Remove last comma and space from string
        cmd = cmd.substr(0, cmd.size() - 2);
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if (nmfUtilsQt::isAnError(errorMsg)) nmfUtils::printError("Section: REPLACE INTO MSVPASuitPreyBiomass...",	errorMsg);

        //databasePtr->nmfSaveDatabase();

        // Remove last comma and space from string
        cmd2 = cmd2.substr(0, cmd2.size() - 2);
        errorMsg = databasePtr->nmfUpdateDatabase(cmd2);
        if (nmfUtilsQt::isAnError(errorMsg)) {
            nmfUtils::printError("Section: REPLACE INTO MSVPASeasBiomass...", errorMsg);
            //std::cout << "cmd:  " << cmd2 << std::endl;
        }
        //databasePtr->nmfSaveDatabase();

        updateProgress(progressVal++, loopPrefix + ":  Calculating diet and consumption data for other predators");

        // BEGIN DIET AND CONSUMPTION CALCULATION FOR OTHER PREDATORS
        logger->logMsg(nmfConstants::Normal,"MSVPA: Calculating Other Predator diets...");
        outputProgressLabel(loopPrefix+": "+"Calculating Other Predator diets...");

        Boost3DArrayDouble OthPredAvgSBM(boost::extents[NOtherPreds][10][NSeas]);
        Boost4DArrayDouble OthPredConsumption(boost::extents[NPreds][10][NYears][NSeas]);
        Boost4DArrayDouble OthPredTotConsumption(boost::extents[NPreds][10][NYears][NSeas]);
        Boost4DArrayDouble OthPredStomCorrection(boost::extents[NPreds][nmfConstants::MaxNumberAges][NYears][NSeas]);
        if (ReadRunStopFile() == "Stop") {
            userHalted = true;
            break;
        }
        if (NOtherPreds > 0) {

            Boost4DArrayDouble OthPredTotSuitBiomass(boost::extents[NOtherPreds][10][NYears][NSeas]);

            for (int i = 0; i < NOtherPreds; ++i) {

                NCats = (OthPredList[i]->SizeStructure == 0) ?	0 : OthPredList[i]->NumSizeCats;

                 // CALCULATE TOTAL SUITABLE PREY BIOMASS FOR EACH SIZE CATEGORY
                fields = {"MSVPAName","Year","Season","PredName","PredAge","TotSBM"};
                queryStr = "SELECT MSVPAName,Year,Season,PredName,PredAge, Sum(SuitPreyBiomass) as TotSBM FROM " +
                            nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
                           " WHERE MSVPAname = '" + MSVPAName +
                           "' AND PredName = '" + OthPredList[i]->SpeName +
                           "' GROUP By PredName, PredAge, Year, Season";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                int m = 0;
                for (int j = 0; j < NCats; ++j) {
                    for (int k = 0; k < NYears; ++k) {
                        for (int l = 0; l < NSeas; ++l) {
                           if (m < int(dataMap["TotSBM"].size())) {
                              OthPredTotSuitBiomass[i][j][k][l] = std::stod(dataMap["TotSBM"][m++]);
                           } else {
                               logger->logMsg(nmfConstants::Error,"Error MSVPA(13.5): Missing data in MSVPASuitPreyBiomass");
                           }
                        } // end for l
                    } // end for k
                } // end for j

                 // CALCULATE AVERAGE SBM ACROSS YEARS
                for (int j = 0; j < NCats; ++j) {
                    for (int k = 0; k < NSeas; ++k) {
                        OthPredAvgSBM[i][j][k] = 0;
                        for (int l = 0; l < NYears; ++l) {
                            OthPredAvgSBM[i][j][k] += OthPredTotSuitBiomass[i][j][l][k];
                        } // end for l
                        OthPredAvgSBM[i][j][k] /= NYears;
                    } // end for k
                } // end for j

                // Calculate the stomach contents correction for each year.  This is the feeding functional response
                for (int j = 0; j < NCats; ++j) {
                    for (int k = 0; k < NSeas; ++k) {
                        for (int l = 0; l < NYears; ++l) {
                            tempValue = OthPredAvgSBM[i][j][k];
                            if (tempValue == 0) {
                                logger->logMsg(nmfConstants::Error,"Error MSVPA(14): Found OthPredAvgSBM[" +
                                        std::to_string(i) + "][" +
                                        std::to_string(j) + "][" +
                                        std::to_string(k) + "] == 0. This will cause a divide by 0 error.  Skipping corresponding OthPredStomCorrection");
                                //return false;
                            }
                            if (tempValue != 0) {
                                OthPredStomCorrection[i][j][l][k] =	OtherPredStomCont[i][j][k]
                                        + (std::log(OthPredTotSuitBiomass[i][j][l][k] / tempValue)	* OtherPredStomCont[i][j][k]);

                                if (OthPredStomCorrection[i][j][l][k] < (0.1 * OtherPredStomCont[i][j][k]))
                                    OthPredStomCorrection[i][j][l][k] = 0.1	* OtherPredStomCont[i][j][k];

                                if (OthPredStomCorrection[i][j][l][k] > (3 * OtherPredStomCont[i][j][k]))
                                    OthPredStomCorrection[i][j][l][k] = 3 * OtherPredStomCont[i][j][k];
                            }
                        } // end for l
                    } // end for k
                } // end for j

                cmd  = "REPLACE INTO " + nmfConstantsMSVPA::TableMSVPASeasBiomass +
                       " (MSVPAName,SpeName,Season,SpeType,Year,Age,Biomass,";
                cmd += "TotalPredCons,SeasM2,AnnAbund,SeasAbund,SeasF,SeasM1,";
                cmd += "SeasWt,SeasSize,AnnBiomass,StomCont) VALUES ";

                cmd2  = "REPLACE INTO " + nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
                        " (MSVPAName,PredName,PredAge,PreyName,PreyAge,Year,Season,";
                cmd2 += "SuitPreyBiomass,PropDiet,BMConsumed,PreyEDens) VALUES ";

                // Calculating seasonal diet composition and predator consumption...

                // Then calculate seasonal diet composition and predator consumption at the same time and save it...
                fields    = {"MSVPAName","SpeName","Season","SpeType","Year","Age","Biomass","TotalPredCons","SeasM2","AnnAbund","SeasAbund","SeasF","SeasM1","SeasWt","SeasSize","AnnBiomass","StomCont"};
                queryStr  = "SELECT MSVPAName,SpeName,Season,SpeType,Year,Age,Biomass,TotalPredCons,SeasM2,AnnAbund,SeasAbund,SeasF,SeasM1,SeasWt,SeasSize,AnnBiomass,StomCont ";
                queryStr += " FROM " + nmfConstantsMSVPA::TableMSVPASeasBiomass +
                            " WHERE MSVPAName = '" + MSVPAName +
                            "' AND SpeName = '" + OthPredList[i]->SpeName +
                            "' ORDER BY Age, Year, Season";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

                int numItems = dataMap["MSVPAName"].size();

                // CALCULATE SEASONAL DIET COMPOSITION AND CONSUMPTION
                m = -1;
                for (int j = 0; j < NCats; ++j) {
                    for (int k = 0; k < NYears; ++k) {
                        for (int l = 0; l < NSeas; ++l) {
                            ++m;
                            if (m >= numItems) {
                                std::cout << "Warning: tried to access more values than what's in dataMap." << std::endl;
                                break;
                            }
                            // Calculate Evacuation rate and annual consumption...includes effect of food availability
                            EvacRate = 24.0 * OtherPredCoeff[i][j][0] * std::exp(OtherPredCoeff[i][j][1] * SeasTemp(k,l));
                            OthPredConsumption[i][j][k][l] = EvacRate * SeasLen(l,2) * OthPredStomCorrection[i][j][k][l];
                            tempValue = std::stod(dataMap["Biomass"][m]) * OthPredConsumption[i][j][k][l];
                            dataMap["TotalPredCons"][m] = (! std::isnan(tempValue)) ? std::to_string(tempValue) : "0.0";
                            tempValue = OthPredStomCorrection[i][j][k][l];
                            dataMap["StomCont"][m]      = (! std::isnan(tempValue)) ? std::to_string(tempValue) : "0.0";
                            OthPredTotConsumption[i][j][k][l] = std::stod(dataMap["TotalPredCons"][m]);

                            fields3 = {"MSVPAName","PredName","PredAge","PreyName","PreyAge","Year","Season","SuitPreyBiomass","PropDiet","BMConsumed","PreyEDens"};
                            queryStr = "SELECT MSVPAName,PredName,PredAge,PreyName,PreyAge,Year,Season,SuitPreyBiomass,PropDiet,BMConsumed,PreyEDens FROM " +
                                        nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
                                       " WHERE MSVPAname = '" + MSVPAName +
                                       "' AND PredName = '" + OthPredList[i]->SpeName +
                                       "' AND PredAge = " + std::to_string(j) +
                                       "  AND Year = " + std::to_string(k) +
                                       "  AND Season = " + std::to_string(l);
                            dataMap3 = databasePtr->nmfQueryDatabase(queryStr, fields3);

                            numItems2 = dataMap3["MSVPAName"].size();

                            for (int n = 0; n < numItems2; ++n) {
                                tempValue = OthPredTotSuitBiomass[i][j][k][l];
                                if (tempValue == 0) {
                                    logger->logMsg(nmfConstants::Error,"Error MSVPA(15): Found OthPredTotSuitBiomass[" +
                                            std::to_string(i) + "][" +
                                            std::to_string(j) + "][" +
                                            std::to_string(k) + "][" +
                                            std::to_string(l) + "] = 0. This will cause a divide by 0 error.");
                                    return false;
                                }
                                if (tempValue != 0) {
                                PDiet = std::stod(dataMap3["SuitPreyBiomass"][n]) / tempValue;
//std::cout << "SuitPreyBiomass,OthPredTotSuitBiomass: " << dataMap3["SuitPreyBiomass"][n] << ", " << OthPredTotSuitBiomass[i][j][k][l] << std::endl;
// SuitPreyBiomass is bad
                                dataMap3["PropDiet"][n]   = std::to_string(PDiet);
                                tempValue = std::stod(dataMap3["PropDiet"][n]) * OthPredTotConsumption[i][j][k][l];
                                dataMap3["BMConsumed"][n] =	(! std::isnan(tempValue)) ? std::to_string(tempValue) : "0.0";

//std::cout << "PropDiet,OthPredTotConsumption: " << dataMap3["PropDiet"][n] << ", " << OthPredTotConsumption[i][j][k][l] << std::endl;
// PropDiet is wrong
                                field0 = std::string("\"") + dataMap3["MSVPAName"][n] + "\"";
                                field1 = std::string("\"") + dataMap3["PredName"][n] + "\"";
                                field2 = dataMap3["PredAge"][n];
                                field3 = std::string("\"") + dataMap3["PreyName"][n] + "\"";
                                field4 = dataMap3["PreyAge"][n];
                                field5 = dataMap3["Year"][n];
                                field6 = dataMap3["Season"][n];
                                field7 = dataMap3["SuitPreyBiomass"][n];
                                field8 = dataMap3["PropDiet"][n];
                                field9 = dataMap3["BMConsumed"][n]; // this is nan sometimes due to the -9 default stored in the Abundance vector on initialization
                                field10 = dataMap3["PreyEDens"][n];

                                cmd2 += "(" + field0 + "," + field1 + "," + field2 + "," + field3 + ","
                                            + field4 + "," + field5 + "," + field6 + "," + field7 + ","
                                            + field8 + "," + field9 + "," + field10 + "), ";
                                }
                            } // end for

                            cmd += "( \""  + dataMap["MSVPAName"][m] + "\"" +
                                    ", \"" + dataMap["SpeName"][m] + "\"" +
                                    "," + dataMap["Season"][m] +
                                    "," + dataMap["SpeType"][m] +
                                    "," + dataMap["Year"][m] +
                                    "," + dataMap["Age"][m] +
                                    "," + dataMap["Biomass"][m] +
                                    "," + dataMap["TotalPredCons"][m] +
                                    "," + dataMap["SeasM2"][m] +
                                    "," + dataMap["AnnAbund"][m] +
                                    "," + dataMap["SeasAbund"][m] +
                                    "," + dataMap["SeasF"][m] +
                                    "," + dataMap["SeasM1"][m] +
                                    "," + dataMap["SeasWt"][m] +
                                    "," + dataMap["SeasSize"][m] +
                                    "," + dataMap["AnnBiomass"][m] +
                                    "," + dataMap["StomCont"][m] + "), ";

                        } // end for l
                    } // end for k
                } // end for j

                // Remove last comma and space from string
                cmd = cmd.substr(0, cmd.size() - 2);
                errorMsg = databasePtr->nmfUpdateDatabase(cmd);
                if (nmfUtilsQt::isAnError(errorMsg)) nmfUtils::printError("Section: REPLACE INTO MSVPASeasBiomass(2)...", errorMsg);

                //databasePtr->nmfSaveDatabase();

                // Remove last comma and space from string
                cmd2= cmd2.substr(0, cmd2.size() - 2);
                errorMsg = databasePtr->nmfUpdateDatabase(cmd2);
                if (nmfUtilsQt::isAnError(errorMsg)) nmfUtils::printError("Section: REPLACE INTO MSVPASuitPreyBiomass(2)...",	errorMsg);

                //databasePtr->nmfSaveDatabase();

            } // end for i
        } // else if NOtherPreds

        updateProgress(progressVal++, loopPrefix + ":  Calculating M2 (predation mortality)");

        cmd = "REPLACE INTO " + nmfConstantsMSVPA::TableMSVPASeasBiomass +
              " (MSVPAName,SpeName,Season,SpeType,Year,Age,Biomass,";
        cmd += "TotalPredCons,SeasM2,AnnAbund,SeasAbund,SeasF,SeasM1,";
        cmd += "SeasWt,SeasSize,AnnBiomass,StomCont) VALUES ";
        /*
         * Calculating Predation Mortality
         */
        updateDB = false;
        // CALCULATE M2 VALUES FOR MSVPA SPECIES
        logger->logMsg(nmfConstants::Normal,"MSVPA: Calculating Predation Mortality...");
        outputProgressLabel(loopPrefix+": "+"Calculating Predation Mortality...");

//s1 = 0;
        for (int i = 0; i < NMSVPASpe; ++i) {
            logger->logMsg(nmfConstants::Normal,"MSVPA:   Species: " + MSVPASpeciesList[i]->SpeName);
std::cout << "  Ages: ";// << std::flush;

            if (ReadRunStopFile() == "Stop") {
                userHalted = true;
                break;
            }
            for (int j = MSVPASpeciesList[i]->FirstCatAge; j <= MSVPASpeciesList[i]->LastCatAge; ++j) {

std::cout << ".";// << std::flush;
                fields    = {"MSVPAName", "PreyName", "PreyAge", "Year", "Season", "TotCons"};
                queryStr  = "SELECT MSVPAName,PreyName,PreyAge,Year,Season, Sum(BMConsumed) as TotCons ";
                queryStr += " FROM " + nmfConstantsMSVPA::TableMSVPASuitPreyBiomass +
                            " WHERE MSVPAName = '" + MSVPAName +
                            "' AND PreyName = '" + MSVPASpeciesList[i]->SpeName +
                            "' AND PreyAge = "   + std::to_string(j) +
                            " GROUP BY PreyName, PreyAge, Year, Season";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

                for (unsigned int n=0; n<dataMap["MSVPAName"].size(); ++n) {

                    tmpSeas = std::stoi(dataMap["Season"][n]);
                    tmpYear = std::stoi(dataMap["Year"][n]);

                    fields2 = {"MSVPAName","SpeName","Season","SpeType","Year","Age","Biomass","TotalPredCons","SeasM2","AnnAbund","SeasAbund","SeasF","SeasM1","SeasWt","SeasSize","AnnBiomass","StomCont"};
                    queryStr = "SELECT MSVPAName,SpeName,Season,SpeType,Year,Age,Biomass,TotalPredCons,SeasM2,AnnAbund,SeasAbund,SeasF,SeasM1,SeasWt,SeasSize,AnnBiomass,StomCont FROM " +
                                nmfConstantsMSVPA::TableMSVPASeasBiomass +
                               " WHERE MSVPAName = '" + MSVPAName +
                               "' AND Spename = '" + MSVPASpeciesList[i]->SpeName +
                               "' AND Age = "      + std::to_string(j) +
                               "  AND Year = "     + std::to_string(tmpYear) +
                               "  AND Season = "   + std::to_string(tmpSeas);
                    dataMap2 = databasePtr->nmfQueryDatabase(queryStr, fields2);

                    if (dataMap2["MSVPAName"].size() > 0) {
                        updateDB = true;
//s1 +=  std::stod(dataMap["TotCons"][n]) / std::stod(dataMap2["Biomass"][0]) ;
                        tempValue = std::stod(dataMap2["Biomass"][0]);
                        if (tempValue == 0) {
                            logger->logMsg(nmfConstants::Error,"Error MSVPA(16): Found Biomass == 0 from MSVPASeasBiomass table. This will cause a divide by 0 error.");
//                            return false;
                        }
                        if (tempValue != 0) {
                        dataMap2["SeasM2"][0] = std::to_string( std::stod(dataMap["TotCons"][n]) / tempValue );
// TotCons is off!!!!!
//std::cout << "TotCons,Biomass: " << dataMap["TotCons"][n] << ", " << dataMap2["Biomass"][0] << std::endl;
                        cmd +=  "( \"" + dataMap2["MSVPAName"][0] + "\"" +
                                ", \"" + dataMap2["SpeName"][0] + "\"" +
                                "," + dataMap2["Season"][0] +
                                "," + dataMap2["SpeType"][0] +
                                "," + dataMap2["Year"][0] +
                                "," + dataMap2["Age"][0] +
                                "," + dataMap2["Biomass"][0] +
                                "," + dataMap2["TotalPredCons"][0] +
                                "," + dataMap2["SeasM2"][0] +
                                "," + dataMap2["AnnAbund"][0] +
                                "," + dataMap2["SeasAbund"][0] +
                                "," + dataMap2["SeasF"][0] +
                                "," + dataMap2["SeasM1"][0] +
                                "," + dataMap2["SeasWt"][0] +
                                "," + dataMap2["SeasSize"][0] +
                                "," + dataMap2["AnnBiomass"][0] +
                                "," + dataMap2["StomCont"][0] + "), ";
                        }
                    } // end if

                } // end for n
            } // end for j
std::cout << std::endl;
        } // end for i

//std::cout << "SeasM2 total: " << s1 << std::endl;

        // Remove last comma and space from string
        if (updateDB) {
            cmd = cmd.substr(0, cmd.size() - 2);
            errorMsg = databasePtr->nmfUpdateDatabase(cmd);
            if (nmfUtilsQt::isAnError(errorMsg)) nmfUtils::printError("Section: Calculate M2 values...", errorMsg);
        }
        //databasePtr->nmfSaveDatabase();


        //  COMPARE CALCULATED M2s AGAINST PREVIOUS MATRIX
        logger->logMsg(nmfConstants::Normal,"MSVPA: Comparing calculated M2s against previous matrix");
        outputProgressLabel(loopPrefix+": "+"Comparing calculated M2s against previous matrix");

        Boost3DArrayDouble tmpM2(boost::extents[NMSVPASpe][nmfConstants::MaxNumberAges+1][NYears+1]);

//s1 = 0;
        updateProgress(progressVal++, loopPrefix + ":  Comparing calculated M2s against previous matrix");


        for (int i = 0; i<NMSVPASpe; ++i) {
            fields = {"MSVPAName", "SpeName", "Age", "Year","M2"};
            queryStr = "SELECT MSVPAName, SpeName, Age, Year, Sum(SeasM2) as M2 FROM " +
                        nmfConstantsMSVPA::TableMSVPASeasBiomass +
                       " WHERE MSVPAname = '" + MSVPAName +
                       "' AND SpeName = '" + MSVPASpeciesList[i]->SpeName +
                       "' GROUP BY SpeName, Age, Year";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            m = 0;
            for (int j = MSVPASpeciesList[i]->FirstCatAge; j<=MSVPASpeciesList[i]->LastCatAge; ++j) {
                for (int k = 0; k<NYears; ++k) {
                    M2Val = dataMap["M2"][m++];
                    tmpM2[i][j][k] = (M2Val.empty()) ? 0 : std::floor(std::stod(M2Val)*1000.0 +0.5)/1000.0;
//s1 += tmpM2[i][j][k];
                } // end for k
            } // end for j
        } // end for i
//std::cout << "M2 FINAL: " << s1 << std::endl;

        //nmfUtils::print3DArray("tmpM2", {NMSVPASpe,nmfConstants::MaxNumberAges,NYears}, tmpM2);
        //nmfUtils::print3DArray("M2", {NMSVPASpe,NYears,nmfConstants::MaxNumberAges}, M2);

        DiffCount = 0;
        // Do the comparison to the original matrix
        for (int i = 0; i < NMSVPASpe; ++i) {
            for (int j = MSVPASpeciesList[i]->FirstCatAge; j <= MSVPASpeciesList[i]->LastCatAge; ++j) {
                for (int k = 0; k < NYears; ++k) {
                    if (std::fabs(tmpM2[i][j][k] - M2[i][k][j]) > 0.005) {  // 0.005  // it converges with 0.05 after 22 loops // it does not converge with 0.005
                        ++DiffCount;
                    } // end if
                } // end k
            } // end j
        } // end i

        logger->logMsg(nmfConstants::Highlight,"MSVPA: DiffCount: " + std::to_string(DiffCount));
        TolCheck = (DiffCount > 3) ? 0 : TolCheck+1; // More than 3 M2s that are too different from previous
        // REPLACE PREVIOUSLY STORED M2 VALUES WITH NEWLY CALCULATED TERMS
        for (int i = 0; i < NMSVPASpe; ++i) {
            for (int j = MSVPASpeciesList[i]->FirstCatAge; j <= MSVPASpeciesList[i]->LastCatAge; ++j) {
                for (int k = 0; k < NYears; ++k) {
                    M2[i][k][j] = tmpM2[i][j][k];
                } // end for k
            } // end for j
        } // end for i

//nmfUtils::print3DArray("M2", {NMSVPASpe,NYears,nmfConstants::MaxNumberAges+1}, M2, 7, 3);

        WriteCurrentLoopFile(MSVPAName, NumLoops+1,DiffCount);

        std::cout << std::endl;

    } // end while

    outputProgressLabel("Done");

    updateProgress(-1,"MSVPA Completed.");

    // End timer for algorithmic timing
    std::string elapsedTimeStr = nmfUtilsQt::elapsedTime(startTime);

    if (userHalted) {
        logger->logMsg(nmfConstants::Normal,"MSVPA: User halted iterations. Output data incomplete.");
    } else {
        logger->logMsg(nmfConstants::Normal,"MSVPA: Everything is all done and written out!");
        WriteRunStopFile("StopAllOk",
                         MSVPAName,
                         elapsedTimeStr,
                         NumLoops+1,
                         DiffCount);
    }


/*
 * Don't think this output is necessary.
 *
    // To see where mysql will write out the tables...
    // mysql> show variables like 'secure_file_priv';
    std::string filename = "/var/lib/mysql-files/test.csv";
    try {
        std::remove(filename.c_str());
    } catch(...) {
        std::cout << "Creating new file: " << filename << std::endl;
    }
    nmfUtils::printMsg("\nWriting table(s) as csv file(s) to: " + filename);
    cmd = "SELECT SpeName,SeasM2,AnnBiomass,StomCont FROM " + nmfConstantsMSVPA::TableMSVPASeasBiomass + " WHERE SpeName=\"Menhaden\" INTO OUTFILE \"" + filename + "\"";
    cmd = "SELECT SpeName,Year,Age,Season,sum(SeasM2) as TotM2, sum(AnnBiomass) as TotBM,sum(StomCont) as TotSC FROM " + nmfConstantsMSVPA::TableMSVPASeasBiomass + " WHERE SpeName='Menhaden' and Age=0  and Season = 0 group by SpeName,Year,Age,Season INTO OUTFILE \"" + filename + "\"";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) nmfUtils::printError("Error exporting test.csv file", errorMsg);
*/
    updateProgress(nmfConstantsMSVPA::MaxMSVPALoops + nmfConstantsMSVPA::NumMSVPASetupSteps,
                   loopPrefix + ":  Done");

    return (NumLoops+1);

} // end nmfMSVPA_NoPredatorGrowth (new version)



bool
nmfMSVPA::GetSize(nmfDatabase* databasePtr, int FirstYear, int LastYear,
        int FirstCatAge, int LastCatAge, std::string SpeName, int SizeIndex,
        boost::numeric::ublas::matrix<double> &Size,
        Boost3DArrayDouble &SeasSize,
        boost::numeric::ublas::matrix<double> &SeasLen,
        int NYears, int NSeas)
{
    int m;
    double SeasStart;
    double SeasEnd;
    double SeasMidAge; // Age at season midpoint
    double SeasMid;
    double ProductSizeDiffSeasMid;
    std::string FirstYearStr = std::to_string(FirstYear);
    std::string LastYearStr  = std::to_string(LastYear);
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    boost::numeric::ublas::vector<double> VbLinf;
    boost::numeric::ublas::vector<double> VbTzero;
    boost::numeric::ublas::vector<double> VBK;
    nmfUtils::initialize(VbLinf,  nmfConstants::MaxNumberYears);
    nmfUtils::initialize(VbTzero, nmfConstants::MaxNumberYears);
    nmfUtils::initialize(VBK,     nmfConstants::MaxNumberYears);

    std::vector<std::string> Variables = {"vbLinf","vbStzero","vbSk"};
    std::vector<boost::numeric::ublas::vector<double>* > Vectors = {&VbLinf,&VbTzero,&VBK};

    switch (SizeIndex) {

        case 0: // fixed VB..calculate size at age for each year from same VB equations
            fields = {"Value"};
            for (unsigned int i=0; i<Variables.size(); ++i) {
                queryStr = "SELECT Value FROM " + nmfConstantsMSVPA::TableSpeSize +
                           " WHERE SpeName='" + SpeName +
                           "' AND Variable = '" + Variables[i] + "'";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                if (dataMap["Value"].size() == 0) {
                    return false;
                }
                (*Vectors[i])(0) = std::stod(dataMap["Value"][0]);
            }

            for (int i=0; i<NYears; ++i) {
                for (int j=FirstCatAge; j<=LastCatAge; ++j) {
                    Size(i,j) = VbLinf(0) * (1 - std::exp(-VBK(0) * (j - VbTzero(0))));
                    for (int k=0; k<NSeas; ++k) {
                        SeasStart = SeasLen(k, 0);
                        SeasEnd = SeasLen(k, 1);
                        SeasMidAge = j + ((SeasEnd + SeasStart) / 2.0);
                        SeasSize[i][j][k] = VbLinf(0) * (1 - std::exp(-VBK(0) * (SeasMidAge - VbTzero(0))));
//std::cout  <<  "SizeIndex: " << SizeIndex << std::endl;
                    }
                }
            }
            break;

        case 1: // ann VB

            fields = {"Value"};
            for (unsigned int i=0; i<Variables.size(); ++i) {
                queryStr = "SELECT Value FROM " + nmfConstantsMSVPA::TableSpeSize +
                           " WHERE SpeName='" + SpeName +
                           "' AND (Year >= " + FirstYearStr + " and Year <= " + LastYearStr +
                           ") AND Variable = '" + Variables[i] + "' ORDER BY Year";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                if (dataMap["Value"].size() == 0) {
                    return false;
                }
                for (int j = 0; j<NYears; ++j) {
                    (*Vectors[i])(j) = std::stod(dataMap["Value"][j]);
                }
            }

            for (int i=0; i<NYears; ++i) {
                for (int j=FirstCatAge; j<=LastCatAge; ++j) {
                    Size(i,j) = VbLinf(i) * (1 - std::exp(-VBK(i) * (j - VbTzero(i))));
                    for (int k=0; k<NSeas; ++k) {
                        SeasStart = SeasLen(k, 0);
                        SeasEnd = SeasLen(k, 1);
                        SeasMidAge = j + ((SeasEnd + SeasStart) / 2.0);
                        SeasSize[i][j][k] = VbLinf(i) * (1 - std::exp(-VBK(i) * (SeasMidAge - VbTzero(i))));
//std::cout  <<  "SizeIndex: " << SizeIndex << std::endl;
                    }
                }
            }
            break;

        default: // fixed size or annual ...everything is already saved by year and age
            fields = { "Value"};
            queryStr = "SELECT Value FROM " + nmfConstantsMSVPA::TableSpeSize +
                       " WHERE SpeName='" + SpeName +
                       "' AND (Year >= " + FirstYearStr +
                       " and Year <= " + LastYearStr +
                       ") AND Variable='Size' ORDER BY Age, Year";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            if (dataMap["Value"].size() == 0) {
                return false;
            }
            m=0;
            for (int j=FirstCatAge; j<=LastCatAge; ++j) {
                for (int i=0; i<NYears; ++i) {
                      Size(i,j) = std::stod(dataMap["Value"][m++]);
                }
            }

            /*
             * To calculate mid-season sizes, apply a linear growth rate within each age class.  This pretty adequetely follows the
             * shape of the VB curve and works moderately well.  Do the growth across the cohort...not across age classes within a
             * particular year.
             */
            double SizeDiff;
            for (int i=0; i <= NYears-2; ++i)  // RSK - changed < to <= due to difference in VB6 and C++ logic
            {
                for (int j=FirstCatAge; j <= LastCatAge-1; ++j) {
                      SizeDiff = Size(i+1,j+1) - Size(i,j);

                      for (int k=0; k <= NSeas-1; ++k) {

                           SeasStart = SeasLen(k, 0);
                           SeasEnd = SeasLen(k, 1);
                           SeasMid = (SeasEnd + SeasStart) / 2.0;
                           ProductSizeDiffSeasMid = SizeDiff * SeasMid;

                           SeasSize[i][j][k] = Size(i,j) + ProductSizeDiffSeasMid;

                           //Apply the same rate to the last age
                           if (j == LastCatAge-1) {
                               SeasSize[i][j+1][k] = Size(i,j+1) + ProductSizeDiffSeasMid;
                           }

                           // Apply the same rate to the last year
                           if (i == NYears-2) {
                               SeasSize[i+1][j][k] = Size(i+1,j) + ProductSizeDiffSeasMid;
                           }

                           // Fix the last age class in the last year
                           if ((i == NYears-2) && (j == LastCatAge-1)) {
                               SeasSize[i+1][j+1][k] = Size(i+1,j+1) + ProductSizeDiffSeasMid;
                           }
                      } // end for k
                } // end for j
            } // end for i
//nmfUtils::print3DArray("SeasSize", {NYears,MaxAge,NSeas}, SeasSize);

            break;

    } // end switch

    return true;

} // end GetSize

void
nmfMSVPA::GetWt(nmfDatabase* databasePtr, int FirstYear, int LastYear,
        int FirstCatAge, int LastCatAge, std::string SpeName, int WtIndex,
        boost::numeric::ublas::matrix<double> &Size,
        boost::numeric::ublas::matrix<double> &Wt,
        Boost3DArrayDouble &SeasSize,
        Boost3DArrayDouble &SeasWt,
        boost::numeric::ublas::matrix<double> &SeasLen,
        int &NYears, int &NSeas)
{
    int m;
    double SeasStart;
    double SeasEnd;
    double SeasMidAge; // age at season midpoint
    double SeasMid;
    double WtDiff;
    double ProductWtDiffSeasMid;
    std::string FirstYearStr = std::to_string(FirstYear);
    std::string LastYearStr  = std::to_string(LastYear);
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    boost::numeric::ublas::vector<double> VbWinf;
    boost::numeric::ublas::vector<double> VbTzero;
    boost::numeric::ublas::vector<double> VBK;
    boost::numeric::ublas::vector<double> WLalpha;
    boost::numeric::ublas::vector<double> WLbeta;
    nmfUtils::initialize(VbWinf,  nmfConstants::MaxNumberYears);
    nmfUtils::initialize(VbTzero, nmfConstants::MaxNumberYears);
    nmfUtils::initialize(VBK,     nmfConstants::MaxNumberYears);
    nmfUtils::initialize(WLalpha, nmfConstants::MaxNumberYears);
    nmfUtils::initialize(WLbeta,  nmfConstants::MaxNumberYears);

//std::cout << "WtIndex: " << WtIndex << std::endl;

    std::vector<std::string> Variables = {"vbWinf","vbWTzero","vbWK"};
    std::vector<boost::numeric::ublas::vector<double>* > Vectors = {&VbWinf,&VbTzero,&VBK};
    std::vector<std::string> FixWlVariables = {"WLAlpha","WLBeta"};
    std::vector<boost::numeric::ublas::vector<double>* > FixWlVectors = {&WLalpha, &WLbeta};

    switch (WtIndex) {

        case 0: // fixed VB
            fields = {"Value"};
            for (unsigned int i=0; i<Variables.size(); ++i) {
                queryStr = "SELECT Value FROM " + nmfConstantsMSVPA::TableSpeWeight +
                           " WHERE SpeName='" + SpeName +
                           "' AND Variable = '" + Variables[i] + "'";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                (*Vectors[i])(0) = std::stod(dataMap[fields[0]][0]);
            }

            for (int i = 0; i<NYears; ++i ) {
                for (int j = FirstCatAge; j<=LastCatAge; ++j) {
                    Wt(i, j) = VbWinf(0) * std::pow((1.0-std::exp(-VBK(0)*(j-VbTzero(0)))),3.0);
                    for (int k=0; k<NSeas; ++k) {
                        SeasStart = SeasLen(k, 0);
                        SeasEnd = SeasLen(k, 1);
                        SeasMidAge = j + ((SeasEnd + SeasStart) / 2.0);
                        SeasWt[i][j][k] = VbWinf(0) * std::pow((1.0 - std::exp(-VBK(0)*(SeasMidAge-VbTzero(0)))),3.0);
                    }
                }
            }
            break;

        case 1: // ann VB
            fields = {"Value"};
            for (unsigned int i=0; i<Variables.size(); ++i) {
                queryStr = "SELECT Value FROM " + nmfConstantsMSVPA::TableSpeWeight +
                           " WHERE SpeName='" + SpeName +
                           "' AND (Year >= " + FirstYearStr + " and Year <= " + LastYearStr +
                           ") AND Variable = '" + Variables[i] + "' ORDER BY Year";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                for (int j = 0; j<NYears; ++j) {
                    (*Vectors[i])(j) = std::stod(dataMap[fields[0]][j]);
                }
            }

            for (int i = 0; i<NYears; ++i ) {
                for (int j = FirstCatAge; j<=LastCatAge; ++j) {
                    Wt(i, j) = VbWinf(i) * std::pow(1.0-std::exp(-VBK(i)*(j-VbTzero(i))),3.0);
                    for (int k=0; k<NSeas; ++k) {
                        SeasStart = SeasLen(k, 0);
                        SeasEnd = SeasLen(k, 1);
                        SeasMidAge = j + ((SeasEnd + SeasStart) / 2.0);
                        SeasWt[i][j][k] = VbWinf(i) * std::pow((1.0-std::exp(-VBK(i)*(SeasMidAge-VbTzero(i)))),3.0);
                    }
                }
            }
            break;

        case 2: // fix WL
            fields = {"Value"};
            for (unsigned int i=0; i<Variables.size(); ++i) {
                queryStr = "SELECT Value FROM " + nmfConstantsMSVPA::TableSpeWeight +
                           " WHERE SpeName='" + SpeName + "' " +
                           " AND Variable = '" + FixWlVariables[i] + "'";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                (*FixWlVectors[i])(0) = std::stod(dataMap[fields[0]][0]);
            }

            for (int i = 0; i<NYears; ++i ) {
                for (int j = FirstCatAge; j<=LastCatAge; ++j) {
                    Wt(i, j) = std::exp(WLalpha(0)) * std::pow(Size(i,j),WLbeta(0));
                    for (int k=0; k<NSeas;++k) {
                        SeasWt[i][j][k] = std::exp(WLalpha(0)) * std::pow(SeasSize[i][j][k],WLbeta(0));
                    }
                }
            }
            break;

        case 3: // ann WL
            fields = {"Value"};
            for (unsigned int i=0; i<Variables.size(); ++i) {
                queryStr = "SELECT Value FROM " + nmfConstantsMSVPA::TableSpeWeight +
                           " WHERE SpeName = '" + SpeName +
                           "' AND (Year >= " + FirstYearStr + " and Year <= " + LastYearStr +
                           ") AND Variable = '" + FixWlVariables[i] +"' ORDER BY Year";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                (*FixWlVectors[i])(0) = std::stod(dataMap[fields[0]][0]);
            }

            for (int i = 0; i<NYears; ++i ) {
                for (int j = FirstCatAge; j<=LastCatAge; ++j) {
                    Wt(i, j) = std::exp(WLalpha(i)) * std::pow(Size(i,j),WLbeta(i));
                    for (int k=0; k<NSeas;++k) {
                        SeasWt[i][j][k] = std::exp(WLalpha(i)) * std::pow(SeasSize[i][j][k],WLbeta(i));
                    }
                }
            }
            break;

        case 4: // wt at age
        case 5:
            fields = {"Value"};
            queryStr = "SELECT Value FROM " + nmfConstantsMSVPA::TableSpeWeight +
                       " WHERE SpeName = '" + SpeName +
                       "' AND (Year >= " + FirstYearStr + " and Year <= " + LastYearStr +
                       ") AND Variable = 'Weight' ORDER BY Year, Age";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            m=0;
            for (int i = 0; i<NYears; ++i ) {
                for (int j = FirstCatAge; j<=LastCatAge; ++j) {
                    Wt(i,j) = std::stod(dataMap["Value"][m++]);
                }
            }

            for (int i=0; i <= NYears-2; ++i) {  // RSK using <= instead of < due to differences in VB6 and C++

                for (int j=0; j <= LastCatAge-1; ++j) {

                    WtDiff = Wt(i+1,j+1) - Wt(i,j);

                    for (int k=0; k <= NSeas-1; ++k) {
                        SeasStart = SeasLen(k, 0);
                        SeasEnd = SeasLen(k, 1);
                        SeasMid = (SeasEnd + SeasStart) / 2.0;

                        ProductWtDiffSeasMid = WtDiff * SeasMid;

                        SeasWt[i][j][k] = Wt(i,j) + ProductWtDiffSeasMid;

                        // Apply the same rate to the last age
                        if (j == LastCatAge - 1) {
                            SeasWt[i][j+1][k] = Wt(i,j+1) + ProductWtDiffSeasMid;
                        }

                        // Apply the same rate to the last year
                        if (i == NYears - 2) {
                            SeasWt[i+1][j][k] = Wt(i+1,j) + ProductWtDiffSeasMid;
                        }

                        // Fix the last age class in the last year
                        if ((i == NYears-2) && (j == LastCatAge-1)) {
                            SeasWt[i+1][j+1][k] = Wt(i+1,j+1) + ProductWtDiffSeasMid;
                        }

                    }
                }
            }

            break;
    }


} // end GetWt



