/**
 *
 * @file nmfForecast.cpp
 * @date Nov 28, 2016
 * @author Ronald Klasky
 *
 * Public Domain Notice
 * National Oceanic And Atmospheric Administration
 *
 * This software is a "United States Government Work" under the terms of the
 * United States Copyright Act.  It was written as part of the author's official
 * duties as a United States Government employee/contractor and thus cannot be copyrighted.
 * This software is freely available to the public for use. The National Oceanic
 * And Atmospheric Administration and the U.S. Government have not placed any
 * restriction on its use or reproduction.  Although all reasonable efforts have
 * been taken to ensure the accuracy and reliability of the software and data,
 * the National Oceanic And Atmospheric Administration and the U.S. Government
 * do not and cannot warrant the performance or results that may be obtained
 * by using this software or data. The National Oceanic And Atmospheric
 * Administration and the U.S. Government disclaim all warranties, express
 * or implied, including warranties of performance, merchantability or fitness
 * for any particular purpose.
 *
 * Please cite the author(s) in any work or product based on this material.
 *
 * @brief This file contains the Forecast API implementation.
 */

#include "nmfDatabase.h"
#include "nmfUtils.h"

#include "nmfForecast.h"
#include "nmfConstants.h"
#include "nmfUtilsStatistics.h"
#include "nmfUtilsSolvers.h"
#include "nmfLogger.h"



nmfForecast::nmfForecast(nmfDatabase* theDatabasePtr,
                         nmfLogger *theLogger)
{
    databasePtr = theDatabasePtr;
    logger      = theLogger;

}

nmfForecast::~nmfForecast()
{
}


bool
nmfForecast::GetAverageTotalSuitableBiomass(nmfDatabase* databasePtr,
        const std::string &MSVPAName,
        const std::string &Species,
        const int &UpperLimJ,
        const int &UpperLimK,
        const int &UpperLimL,
        const int &index1,
        Boost3DArrayDouble &Array3D)
{
    int m = 0;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    // Load average total suitable prey biomass by predator, year, season
    fields   = {"PredName","PredAge","Season","Year","TotSBM"};
    queryStr = "SELECT PredName, PredAge, Season, Year, Sum(SuitPreyBiomass) as TotSBM FROM MSVPASuitPreyBiomass WHERE MSVPAname='" + MSVPAName + "' " +
            " AND PredName = '" + Species + "' GROUP BY PredName, PredAge, Season, Year";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (nmfUtils::checkForError(logger,"MSVPASuitPreyBiomass","PredName",dataMap,queryStr))
        return false;
    for (int j = 0; j <= UpperLimJ; ++j) {
        for (int k = 0; k < UpperLimK; ++k) {
            Array3D[index1][j][k] = 0;
            for (int l = 0; l < UpperLimL; ++l) {
                Array3D[index1][j][k] += std::stod(dataMap["TotSBM"][m++]);
            }
            Array3D[index1][j][k] /= UpperLimL;
        }
    }

    return true;
} // end getAverageTotalSuitableBiomass



bool
nmfForecast::GetTypePreferences(nmfDatabase* databasePtr,
        const std::string &MSVPAName,
        const std::string &Species,
        const boost::numeric::ublas::vector<std::string> &SpeList,
        const int &UpperLimK,
        const int &UpperLimJ,
        const int &index1,
        Boost3DArrayDouble &Array3D)
{
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    // Load type preferences with MSVPA species
    for (int k = 0; k < UpperLimK; ++k) {
        fields   = {"PrefVal"};
        queryStr = "SELECT PrefVal FROM MSVPAprefs WHERE MSVPAname = '" + MSVPAName + "' " +
                " AND SpeName = '"  + Species + "' " +
                " AND PreyName = '" + SpeList(k) + "' ORDER By Age";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (nmfUtils::checkForError(logger,"MSVPAprefs","PrefVal",dataMap,queryStr))
            return false;
        for (int j = 0; j <= UpperLimJ; ++j) {
            Array3D[index1][j][k] = std::stod(dataMap["PrefVal"][j]);
        }
    }

    return true;
} // end getTypePreferences



bool
nmfForecast::GetSeasonSpatialOverlap(nmfDatabase* databasePtr,
     const std::string &MSVPAName,
     const std::string &Species,
     const boost::numeric::ublas::vector<std::string> &SpeList,
     const int &UpperLimK,
     const int &UpperLimJ,
     const int &UpperLimL,
     const int &index2,
     Boost4DArrayDouble &Array4D)
{
    int m;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    // Load Seasonal Spatial overlap with MSVPASpecies
    for (int k = 0; k < UpperLimK; ++k) {
        fields   = {"SpOverlap"};
        queryStr = "SELECT SpOverlap FROM MSVPASpaceO WHERE MSVPAname = '" + MSVPAName + "' " +
                " AND SpeName = '"  + Species + "' " +
                " AND PreyName = '" + SpeList(k) + "' ORDER By Age, Season";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (nmfUtils::checkForError(logger,"MSVPASpaceO","SpOverlap",dataMap,queryStr))
            return false;
        m = 0;
        for (int j = 0; j <= UpperLimJ; ++j) {
            for  (int l = 0; l < UpperLimL; ++l) {
                Array4D[l][index2][j][k] = std::stod(dataMap["SpOverlap"][m++]);
            }
        }
    }

    return true;
} // end getSeasonSpatialOverlap


//void
//nmfForecast::updateProgress(int value, std::string msg){
//    std::ofstream outputFile(nmfConstants::ForecastProgressBarFile);
//    outputFile << value << "; " << msg << std::endl;
//    outputFile.close();
//}


bool
nmfForecast::Forecast_NoPredatorGrowth(
        const ForecastNoPredatorGrowthStruct &argStruct)

{
    int m;
    int NSpe;
    int NPreyOnly;
    int NOthPrey;
    int NOthPred;
    int MSVPAFirstYear;
    int LastYear;
    int NSeasons;
    int NPreds;
    int MSVPANYears;
    int PredCount;
    int Seas = 0;
    int progressVal = 0;
    bool retv;
    std::string FirstYearStr;
    std::string LastYearStr;
    std::string queryStr;
    std::string queryStr2;
    std::vector<std::string> fields;
    std::vector<std::string> fields2;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::map<std::string, std::vector<std::string> > dataMap2;
    boost::numeric::ublas::vector<int> SeasLen;
    boost::numeric::ublas::vector<double> SeasTemp;
    boost::numeric::ublas::vector<double> SeasStart;
    boost::numeric::ublas::vector<double> SeasEnd;
    boost::numeric::ublas::vector<std::string> SpeList;
    boost::numeric::ublas::vector<double> SpeType;
    boost::numeric::ublas::vector<double> NSpeAge;
    boost::numeric::ublas::vector<int> SRRType;
    boost::numeric::ublas::matrix<double> ConsAlpha;
    boost::numeric::ublas::matrix<double> ConsBeta;
    boost::numeric::ublas::matrix<double> PropMature;
    boost::numeric::ublas::vector<std::string> OtherPredList;
    boost::numeric::ublas::vector<int>    NOtherPredAge;
    boost::numeric::ublas::vector<double> OthPredBMUnits;
    boost::numeric::ublas::matrix<double> OthPredConsAlpha;
    boost::numeric::ublas::matrix<double> OthPredConsBeta;
    boost::numeric::ublas::vector<std::string> OtherPreyList;
    boost::numeric::ublas::matrix<double> OtherPreyBiomass;
    boost::numeric::ublas::matrix<double> OtherPreyCoeff;
    boost::numeric::ublas::matrix<double> OthPredMinLen;
    boost::numeric::ublas::matrix<double> OthPredMaxLen;

//    nmfDatabase* databasePtr = argStruct.databasePtr;
    std::string MSVPAName    = argStruct.MSVPAName;
    std::string ForecastName = argStruct.ForecastName;
    std::string ScenarioName = argStruct.ScenarioName;
    int InitYear             = argStruct.InitYear;
    int NYears               = argStruct.NYears;

    logger->logMsg(nmfConstants::Normal,"Forecast: Initial Data: ");
    logger->logMsg(nmfConstants::Normal,"Forecast: -> MSVPA Name:      " + MSVPAName);
    logger->logMsg(nmfConstants::Normal,"Forecast: -> Forecast Name:   " + ForecastName);
    logger->logMsg(nmfConstants::Normal,"Forecast: -> Scenario Name:   " + ScenarioName);
    logger->logMsg(nmfConstants::Normal,"Forecast: -> Initial Year:    " + std::to_string(InitYear));
    logger->logMsg(nmfConstants::Normal,"Forecast: -> Num Years to Forecast: " + std::to_string(NYears));
    logger->logMsg(nmfConstants::Normal,"Forecast: -> Predator Growth: Fixed");
    logger->logMsg(nmfConstants::Normal,"Forecast: Setup - Begin");

    outputProgressLabel("");

    // Start timer for algorithmic timing
    std::chrono::_V2::system_clock::time_point startTime = nmfUtils::startTimer();


// test code RSK
// reopen the database since we're now in a new thread
//databasePtr = new nmfDatabase();
//std::string Hostname = "localhost";
//std::string Username = "root";
//std::string Password = "rklasky$$";
//std::string errorMsg;
//databasePtr->nmfCloseDatabase();
//databasePtr->nmfOpenDatabase(Hostname,Username,Password,errorMsg);

    fields = {"SpeName"};
    queryStr = "SELECT SpeName from Species";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    std::cout << "num species: " << dataMap["SpeName"].size() << std::endl;


    updateProgress(progressVal++,"Clearing tables ForeSuitPreyBiomass, ForeOutput");
    if (ReadRunStopFile() == "Stop")
        return false;

    if (! databasePtr->nmfDeleteRecordsFromTable("ForeSuitPreyBiomass",MSVPAName,ForecastName,ScenarioName))
        return false;

    if (! databasePtr->nmfDeleteRecordsFromTable("ForeOutput",MSVPAName,ForecastName,ScenarioName)) {
        return false;
    }

// test code
fields = {"MSVPAName","NSpe","NPreyOnly","NOther","NOtherPred","FirstYear","LastYear","NSeasons"};
queryStr = "SELECT MSVPAName,NSpe,NPreyOnly,NOther,NOtherPred,FirstYear,LastYear,NSeasons FROM MSVPAlist WHERE MSVPAName = '"
            + MSVPAName + "'";
dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
std::cout << queryStr << std::endl;
std::cout << "size: " << dataMap["NSpe"].size() << std::endl;

//dataMap = databasePtr->nmfQueryDatabase(queryStr, fields, nmfConstants::nullAsZero);
NPreds       = std::stoi(dataMap["NSpe"][0]);






    databasePtr->nmfGetInitialData(MSVPAName,
        NPreds, NPreyOnly, NOthPrey, NOthPred,
        MSVPAFirstYear, LastYear, NSeasons, NSpe, MSVPANYears,
        FirstYearStr, LastYearStr);

    updateProgress(progressVal++, "Loading Season, Temperature, Predator, and Species data");
    if (ReadRunStopFile() == "Stop")
        return false;

    // Load Season Information
    nmfUtils::initialize(SeasLen, NSeasons);
    nmfUtils::initialize(SeasTemp,NSeasons);

    fields = {"Value"};
    queryStr = "SELECT Value FROM MSVPASeasInfo WHERE MSVPAname = '" + MSVPAName + "' "
               " AND Variable = 'SeasLen'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (nmfUtils::checkForError(logger,"MSVPASeasInfo","Value",dataMap,queryStr))
        return false;
    for (int i = 0; i < NSeasons; ++i) {
     SeasLen(i) = std::stod(dataMap["Value"][i]);
    }

    nmfUtils::initialize(SeasStart,NSeasons);
    nmfUtils::initialize(SeasEnd,  NSeasons);
    nmfUtils::initialize(SpeList,  NSpe);
    nmfUtils::initialize(NSpeAge,  NSpe);
    nmfUtils::initialize(SpeType,  NSpe);

    SeasStart(0) = 0.0;
    SeasEnd(NSeasons-1) = 1.0;
    for (int i=0; i<NSeasons-1; ++i) {
        SeasEnd(i) = SeasStart(i) + std::stod(dataMap["Value"][i])/365.0;
        SeasStart(i+1) = SeasEnd(i);
    }

    fields = {"Value"};
    queryStr = "SELECT Value FROM MSVPASeasInfo WHERE MSVPAname = '" + MSVPAName + "' "
               " AND Year = " + std::to_string(InitYear) +
               " AND Variable = 'SeasTemp'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (nmfUtils::checkForError(logger,"MSVPASeasInfo","Value",dataMap,queryStr))
        return false;
    for (int i=0; i<NSeasons; ++i) {
        SeasTemp(i) = std::stod(dataMap["Value"][i]);
    }

    // LOAD MSVPA SPECIES both predators and prey Type 0 = Predator, Type 1 = Prey
    fields = {"SpeName","MaxAge","CatchUnits"};
    queryStr = "SELECT SpeName,MaxAge,CatchUnits FROM Species";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (nmfUtils::checkForError(logger,"Species","SpeName",dataMap,queryStr))
        return false;
    std::string indexStr;
    std::map<std::string,int> MaxAges;
    std::map<std::string,int> CatchUnits;
    for (unsigned int i=0; i<dataMap["SpeName"].size(); ++i) {
        indexStr = dataMap["SpeName"][i];
        MaxAges[indexStr]    = std::stod(dataMap["MaxAge"][i]);
        CatchUnits[indexStr] = std::stod(dataMap["CatchUnits"][i]);
    }

    fields = {"Type","SpeName"};
    queryStr = "SELECT Type,SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "' "
               " AND (Type = 0 or Type = 1)";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (nmfUtils::checkForError(logger,"MSVPAspecies","SpeName",dataMap,queryStr))
        return false;
    PredCount = 0;
    for (int i=0; i<NSpe; ++i) {
        SpeType(i) = std::stod(dataMap["Type"][i]);
        SpeList(i) = dataMap["SpeName"][i];
        NSpeAge(i) = MaxAges[SpeList(i)];
    } // end for i

    // Load MSVPA predator evacuation rates, size selectivity, and mean stomach contents
    nmfUtils::initialize(ConsAlpha,  NSpe, nmfConstants::MaxNumberAges);
    nmfUtils::initialize(ConsBeta,   NSpe, nmfConstants::MaxNumberAges);

    Boost3DArrayDouble MSVPASizeSel(boost::extents[NSpe][nmfConstants::MaxNumberAges][2]);
    Boost3DArrayDouble StomCont(boost::extents[NSpe][nmfConstants::MaxNumberAges][NSeasons]);
    Boost3DArrayDouble MSVPAAvgSBM(boost::extents[NSpe][nmfConstants::MaxNumberAges][NSeasons]);
    Boost3DArrayDouble MSVPATypePref(boost::extents[NSpe][nmfConstants::MaxNumberAges][NSeasons]);
    Boost4DArrayDouble MSVPASpaceO(boost::extents[NSeasons][NSpe][nmfConstants::MaxNumberAges][NSpe]);

    updateProgress(progressVal++, "Loading MSVPA predator parameters...");
    if (ReadRunStopFile() == "Stop")
        return false;

    for (int i = 0; i < NSpe; ++i) {
        if (SpeType(i) == 0) {

            fields   = {"EvacAlpha","EvacBeta","SizeAlpha","SizeBeta"};
            queryStr = "SELECT EvacAlpha,EvacBeta,SizeAlpha,SizeBeta FROM MSVPASizePref WHERE MSVPAname = '" + MSVPAName + "' " +
                    " AND SpeName = '" + SpeList(i) + "' ORDER BY Age";
            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
            if (nmfUtils::checkForError(logger,"MSVPASizePref","EvacAlpha",dataMap,queryStr))
                return false;

            fields2   = {"MeanGutFull"};
            queryStr2 = "SELECT MeanGutFull FROM MSVPAStomCont WHERE MSVPAname = '" + MSVPAName + "' " +
                    " AND SpeName = '" + SpeList(i) + "' ORDER BY Age,Season ";
            dataMap2  = databasePtr->nmfQueryDatabase(queryStr2, fields2);
            if (nmfUtils::checkForError(logger,"MSVPAStomCont","MeanGutFull",dataMap2,queryStr2))
                return false;

            m = 0;
            for (int j = 0; j <= NSpeAge(i); ++j) {
                ConsAlpha(PredCount,j)        = std::stod(dataMap["EvacAlpha"][j]);
                ConsBeta(PredCount,j)         = std::stod(dataMap["EvacBeta"][j]);
                MSVPASizeSel[PredCount][j][0] = std::stod(dataMap["SizeAlpha"][j]);
                MSVPASizeSel[PredCount][j][1] = std::stod(dataMap["SizeBeta"][j]);
                for (int k = 0; k < NSeasons; ++k) {
                    StomCont[PredCount][j][k] = std::stod(dataMap2["MeanGutFull"][m++]);
                }
            }
std::cout << 11 << std::endl;
            // Load average total suitable prey biomass by predator, year, season
            retv = GetAverageTotalSuitableBiomass(databasePtr, MSVPAName,
                                                  SpeList(i), NSpeAge(i), NSeasons,
                                                  MSVPANYears, PredCount, MSVPAAvgSBM);
            if (! retv)
                return false;
std::cout << 22 << std::endl;
            // Load type preferences with MSVPA species
            retv = GetTypePreferences(databasePtr, MSVPAName, SpeList(i), SpeList, NSpe, NSpeAge(i), PredCount, MSVPATypePref);
            if (! retv)
                return false;
std::cout << 33 << std::endl;
            // Load Seasonal Spatial overlap with MSVPASpecies
            retv = GetSeasonSpatialOverlap(databasePtr, MSVPAName, SpeList(i), SpeList, NSpe, NSpeAge(i), NSeasons, PredCount, MSVPASpaceO);
            if (! retv)
                return false;
std::cout << 44 << std::endl;
            ++PredCount;
        } // end if

    } // end for i



    // LOAD PROPORTION MATURE FOR SPAWNING STOCK BIOMASS CALCULATIONS
    updateProgress(progressVal++, "Loading proportion mature for spawning stock biomass calculations...");
    if (ReadRunStopFile() == "Stop")
        return false;

    nmfUtils::initialize(PropMature, NSpe, nmfConstants::MaxNumberAges);

    for (int i = 0; i < NSpe; ++i) {
        fields   = {"PMature"};
        queryStr = "SELECT PMature FROM SpeMaturity WHERE SpeName = '" + SpeList(i) + "' " +
                   " AND Year = " + std::to_string(InitYear) + " ORDER BY Age";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (nmfUtils::checkForError(logger,"SpeMaturity","PMature",dataMap,queryStr))
            return false;
        for (int j = 0; j <= NSpeAge(i); ++j) {
            PropMature(i,j) = std::stod(dataMap["PMature"][j]);
        }
    }

    // LOAD STOCK-RECRUIT RELATIONSHIP TYPE
    nmfUtils::initialize(SRRType,NSpe);

    for (int i = 0; i < NSpe; ++i) {
        fields   = {"SRRType"};
        queryStr = "SELECT SRRType FROM ForeSRR WHERE MSVPAname = '" + MSVPAName + "' " +
                " AND ForeName = '" + ForecastName + "' " +
                " AND SpeName = '"  + SpeList(i) + "'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (nmfUtils::checkForError(logger,"ForeSRR","SRRType",dataMap,queryStr))
            return false;
        SRRType(i) = std::stoi(dataMap["SRRType"][0]);
    }

    updateProgress(progressVal++, "Loading biomass predator data...");
    if (ReadRunStopFile() == "Stop")
        return false;

    nmfUtils::initialize(OtherPredList,   NOthPred);
    nmfUtils::initialize(NOtherPredAge,   NOthPred);
    nmfUtils::initialize(OthPredBMUnits,  NOthPred);
    nmfUtils::initialize(OthPredConsAlpha,NOthPred, nmfConstants::MaxNumberAges);
    nmfUtils::initialize(OthPredConsBeta, NOthPred, nmfConstants::MaxNumberAges);

    Boost3DArrayDouble OthPredStomCont(boost::extents[NOthPred][nmfConstants::MaxNumberAges][NSeasons]);
    Boost3DArrayDouble OthPredSizeSel(boost::extents[NOthPred][nmfConstants::MaxNumberAges][2]);
    Boost3DArrayDouble OthPredTypePref(boost::extents[NOthPred][nmfConstants::MaxNumberAges][NSpe]);
    Boost4DArrayDouble OthPredSpaceO(boost::extents[NSeasons][NOthPred][nmfConstants::MaxNumberAges][NSpe]);
    Boost3DArrayDouble OthPredAvgSBM(boost::extents[NOthPred][nmfConstants::MaxNumberAges][NSeasons]);

    if (NOthPred > 0) {

        fields   = {"SpeName"};
        queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "' AND Type = 3";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (nmfUtils::checkForError(logger,"MSVPAspecies","SpeName",dataMap,queryStr))
            return false;
        for (int i = 0; i < NOthPred; ++i) {
            OtherPredList(i) = dataMap["SpeName"][i];
            fields2   = {"NumSizeCats","BMUnits"};
            queryStr2 = "SELECT NumSizeCats,BMUnits FROM OtherPredSpecies WHERE SpeName = '" + OtherPredList(i) + "'";
            dataMap2  = databasePtr->nmfQueryDatabase(queryStr2, fields2);
            if (nmfUtils::checkForError(logger,"OtherPredSpecies","NumSizeCats",dataMap2,queryStr2))
                return false;
            NOtherPredAge(i)  = std::stoi(dataMap2["NumSizeCats"][0]) - 1;
            OthPredBMUnits(i) = std::stod(dataMap2["BMUnits"][0]);
        } // end for i

        for (int i = 0; i < NOthPred; ++i) {
            fields   = {"ConsAlpha","ConsBeta","SizeSelAlpha","SizeSelBeta"};
            queryStr = "SELECT ConsAlpha,ConsBeta,SizeSelAlpha,SizeSelBeta FROM OthPredSizeData WHERE SpeName = '" + OtherPredList(i) + "'" +
                      " ORDER BY SizeCat";
            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
            if (nmfUtils::checkForError(logger,"OthPredSizeData","ConsAlpha",dataMap,queryStr))
                return false;
            fields2   = {"MeanGutFull"};
            queryStr2 = "SELECT MeanGutFull FROM MSVPAStomCont WHERE MSVPAname = '" + MSVPAName + "'" +
                        " AND SpeName = '" + OtherPredList(i) + "' ORDER By Age,Season";
            dataMap2  = databasePtr->nmfQueryDatabase(queryStr2, fields2);
            if (nmfUtils::checkForError(logger,"MSVPAStomCont","MeanGutFull",dataMap2,queryStr2))
                return false;
            for (int j = 0; j <= NOtherPredAge(i); ++j) {
                OthPredConsAlpha(i,j)   = std::stod(dataMap["ConsAlpha"][j]);
                OthPredConsBeta(i,j)    = std::stod(dataMap["ConsBeta"][j]);
                OthPredSizeSel[i][j][0] = std::stod(dataMap["SizeSelAlpha"][j]);
                OthPredSizeSel[i][j][1] = std::stod(dataMap["SizeSelBeta"][j]);
                for (int k = 0; k < NSeasons; ++k) {
                    OthPredStomCont[i][j][k] = std::stod(dataMap2["MeanGutFull"][k]);
                }
            } // end for j

            // Load average total suitable prey biomass by predator, year, season
            retv = GetAverageTotalSuitableBiomass(databasePtr, MSVPAName, OtherPredList(i),
                                                  NOtherPredAge(i), NSeasons, MSVPANYears,
                                                  i, OthPredAvgSBM);
            if (! retv)
                return false;

            // Load type preferences with MSVPA species
            retv = GetTypePreferences(databasePtr, MSVPAName, OtherPredList(i), SpeList, NSpe, NOtherPredAge(i), i, OthPredTypePref);
            if (! retv)
                return false;

            // Load Seasonal Spatial overlap with MSVPASpecies
            retv = GetSeasonSpatialOverlap(databasePtr, MSVPAName, OtherPredList(i), SpeList, NSpe, NOtherPredAge(i), NSeasons, i, OthPredSpaceO);
            if (! retv)
                return false;

        } // end i

    } // end if
    // END LOADING OTHER PREDATOR DATA


    // LOAD OTHER PREY PARAMETERS
    updateProgress(progressVal++, "Loading other prey parameters...");
    if (ReadRunStopFile() == "Stop")
        return false;

    nmfUtils::initialize(OtherPreyList, NOthPrey);
    nmfUtils::initialize(OtherPreyBiomass, NOthPrey, NSeasons);
    nmfUtils::initialize(OtherPreyCoeff, NOthPrey, 4); // MinSize, MaxSize, SizeAlpha, SizeBeta

    fields   = {"OthPreyName","MinSize","MaxSize","SizeAlpha","SizeBeta"};
    queryStr = "SELECT OthPreyName,MinSize,MaxSize,SizeAlpha,SizeBeta FROM MSVPAOthPrey WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (nmfUtils::checkForError(logger,"MSVPAOthPrey","OthPreyName",dataMap,queryStr))
        return false;
    for (int i = 0; i < NOthPrey; ++i) {
        OtherPreyList(i)    = dataMap["OthPreyName"][i];
        OtherPreyCoeff(i,0) = std::stod(dataMap["MinSize"][i]);
        OtherPreyCoeff(i,1) = std::stod(dataMap["MaxSize"][i]);
        OtherPreyCoeff(i,2) = std::stod(dataMap["SizeAlpha"][i]);
        OtherPreyCoeff(i,3) = std::stod(dataMap["SizeBeta"][i]);
    }

    // Load type prefs and spatial O for msvpa and other preds eating other prey
    Boost3DArrayDouble MSVPAOthPyType(boost::extents[NSpe][nmfConstants::MaxNumberAges][NOthPrey]);
    Boost3DArrayDouble OthPredOthPyType(boost::extents[NOthPred][nmfConstants::MaxNumberAges][NOthPrey]);
    Boost4DArrayDouble MSVPAOthPySpaceO(boost::extents[NSeasons][NSpe][nmfConstants::MaxNumberAges][NOthPrey]);
    Boost4DArrayDouble OthPredOthPySpaceO(boost::extents[NSeasons][NOthPred][nmfConstants::MaxNumberAges][NOthPrey]);

    PredCount = 0;
    for (int i = 0; i < NSpe; ++i) {
        if (SpeType(i) == 0) {
            retv = GetTypePreferences(databasePtr, MSVPAName, SpeList(i), OtherPreyList,
                               NOthPrey, NSpeAge(i), PredCount, MSVPAOthPyType);
            if (! retv)
                return false;
            retv = GetSeasonSpatialOverlap(databasePtr, MSVPAName, SpeList(i), OtherPreyList,
                                    NOthPrey, NSpeAge(i), NSeasons, PredCount, MSVPAOthPySpaceO);
            if (! retv)
                return false;
            ++PredCount;
        } // end if
    } // end for i


    updateProgress(progressVal++, "Loading parameters for biomass predators interacting with other prey...");
    if (ReadRunStopFile() == "Stop")
        return false;

    if (NOthPred > 0) {
        for (int i = 0; i < NOthPred; ++i) {
            retv = GetTypePreferences(databasePtr, MSVPAName, OtherPredList(i), OtherPreyList,
                               NOthPrey, NOtherPredAge(i), i, OthPredOthPyType);
            if (! retv)
                return false;
            retv = GetSeasonSpatialOverlap(databasePtr, MSVPAName, OtherPredList(i), OtherPreyList,
                                    NOthPrey, NOtherPredAge(i), NSeasons, i, OthPredOthPySpaceO);
            if (! retv)
                return false;
        } // end for i
    } // end if

    // END LOADING MODEL CONFIGURATION AND PARAMETERS

    // LOAD SCENARIO INFORMATION AND INITIAL DATA
    updateProgress(progressVal++, "Loading scenario information and initial data...");
    if (ReadRunStopFile() == "Stop")
        return false;

    struct Scenarios Scenario;
    Scenario.Name = ScenarioName;

    fields   = {"FishAsF","VarF","VarOthPred","VarOthPrey","VarRec"};
    queryStr = "SELECT FishAsF,VarF,VarOthPred,VarOthPrey,VarRec FROM Scenarios WHERE MSVPAname = '" + MSVPAName + "'" +
            " AND Forename = '" + ForecastName + "'" +
            " AND Scenario = '" + ScenarioName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (nmfUtils::checkForError(logger,"Scenarios","FishAsF",dataMap,queryStr))
        return false;
    Scenario.FishAsF    = std::stoi(dataMap["FishAsF"][0]);
    Scenario.VarF       = std::stoi(dataMap["VarF"][0]);
    Scenario.VarOthPred = std::stoi(dataMap["VarOthPred"][0]);
    Scenario.VarOthPrey = std::stoi(dataMap["VarOthPrey"][0]);
    Scenario.VarRec     = std::stoi(dataMap["VarRec"][0]);

    // LOAD SEASONAL SIZE AND WEIGHT AT AGE FOR MSVPA SPECIES

    updateProgress(progressVal++, "Loading seasonal size and weight at age for MSVPA species...");
    if (ReadRunStopFile() == "Stop")
        return false;

    Boost4DArrayDouble SeasPreySize(boost::extents[NSpe][nmfConstants::MaxNumberAges][NSeasons][365]);
    Boost4DArrayDouble SeasPreyWeight(boost::extents[NSpe][nmfConstants::MaxNumberAges][NSeasons][365]);
    double SeasBeg;
    double SeasLast;
    double DayAge;

    for (int i = 0; i < NSpe; ++i) {
        fields   = {"Linf","GrowthK","Tzero","LWAlpha","LWBeta"};
        queryStr = "SELECT Linf,GrowthK,Tzero,LWAlpha,LWBeta FROM ForePredVonB WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND ForeName = '" + ForecastName + "'" +
                " AND PredName = '" + SpeList(i) + "'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (nmfUtils::checkForError(logger,"ForePredVonB","Linf",dataMap,queryStr))
            return false;
        SeasBeg = 0;
        SeasLast = 0;
        for (int j = 0; j < NSeasons; ++j) {
            SeasLast += (SeasLen(Seas) / 365.0);
            SeasBeg = SeasLast - (SeasLen(Seas) / 365.0);
            for (int k = 0; k <= NSpeAge(i); ++k) {
                for (int l = 0; l <= SeasLen(j); ++l) {
                    DayAge = k + SeasBeg + l / 365.0;
                    SeasPreySize[i][k][j][l] = std::stod(dataMap["Linf"][0]) *
                            (1.0 - std::exp(-std::stod(dataMap["GrowthK"][0]) * (DayAge - std::stod(dataMap["Tzero"][0]))));
                    SeasPreyWeight[i][k][j][l] = std::exp(std::stod(dataMap["LWAlpha"][0]) +
                            std::stod(dataMap["LWBeta"][0]) * std::log(SeasPreySize[i][k][j][l]));
                } // end for l
            } // end for k
        } // end for j
    } // end for i

    // LOAD OTHER PRED BIOMASS AND CALCULATE SEASONAL BIOMASSESS

    updateProgress(progressVal++, "Loading other pred biomass and calculating seasonal biomasses...");
    if (ReadRunStopFile() == "Stop")
        return false;

    double AnnRate;
    double AvgBM;
    Boost3DArrayDouble OthPredBiomass(boost::extents[NOthPred][NYears+1][NSeasons]);
    Boost4DArrayDouble OthPredBiomassSize(boost::extents[NOthPred][nmfConstants::MaxNumberAges][NYears+1][NSeasons]);
    nmfUtils::initialize(OthPredMinLen,NOthPred,nmfConstants::MaxNumberAges);
    nmfUtils::initialize(OthPredMaxLen,NOthPred,nmfConstants::MaxNumberAges);


    for (int i = 0; i < NOthPred; ++i) {

        fields   = {"Biomass"};
        queryStr = "SELECT Biomass FROM OtherPredBM WHERE SpeName = '" + OtherPredList(i) + "'" +
                " AND Year = " + std::to_string(InitYear);
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (nmfUtils::checkForError(logger,"OtherPredBM","Biomass",dataMap,queryStr))
            return false;
        OthPredBiomass[i][0][0] = std::stod(dataMap["Biomass"][0]) * OthPredBMUnits(i);
        if (Scenario.VarOthPred == 0) {
            for (int j = 1; j <= NYears; ++j) {
                OthPredBiomass[i][j][0] = OthPredBiomass[i][0][0];
            }
        } else {
            fields   = { "SpeName", "Year", "AnnTotBM" };
            queryStr = "SELECT SpeName,Year,Sum(Biomass) as AnnTotBM FROM ScenarioOthPred WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND ForeName = '" + ForecastName + "'" +
                    " AND Scenario = '" + ScenarioName + "'" +
                    " AND SpeName = '"  + OtherPredList(i) + "' AND Year > 0 GROUP BY SpeName,Year";
            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
            if (nmfUtils::checkForError(logger,"ScenarioOthPred","SpeName",dataMap,queryStr))
                return false;
            for (int j = 1; j <= NYears; ++j) {
                OthPredBiomass[i][j][0] = std::stod(dataMap["AnnTotBM"][j]) * OthPredBMUnits(i);
            }
        } // end if

        if (Scenario.VarOthPred == 0) {
            for (int j = 0; j <= NYears; ++j) {
                for (int k = 0; k < NSeasons; ++k) {
                    OthPredBiomass[i][j][k] = OthPredBiomass[i][j][0];
                }
            }
        } else {
            for (int j = 0; j < NYears; ++j) {
                // Calculate an annual growth rate
                AnnRate = -std::log(OthPredBiomass[i][j+1][0] / OthPredBiomass[i][j][0]);
                for (int k = 0; k < NSeasons; ++k) {
                    // Calculate average relative Growth at midpoint
                    AvgBM = (AnnRate != 0.0) ? (std::exp(-AnnRate*SeasStart(k)) - std::exp(-AnnRate*SeasEnd(k))) / ((SeasEnd(k)-SeasStart(k))*AnnRate) : 1;

                    // Calculate Wt at Age at midpoint of season as product
                    OthPredBiomass[i][j][k] = AvgBM * OthPredBiomass[i][j][0];

                    // Apply same rate to last year
                    if (j == (NYears-1)) {
                        OthPredBiomass[i][j+1][k+1] = AvgBM * OthPredBiomass[i][j+1][0];
                    }
                } // end for k
            }

        } // end if

        // Partition total biomass into seasonal biomass by age class
        fields   = { "MinLen", "MaxLen", "PropBM" };
        queryStr = "SELECT MinLen,MaxLen,PropBM FROM OthPredSizeData WHERE SpeName = '" + OtherPredList(i) + "' ORDER By SizeCat";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (nmfUtils::checkForError(logger,"OthPredSizeData","MinLen",dataMap,queryStr))
            return false;
        for (int j = 0; j <= NOtherPredAge(i); ++j) {
            OthPredMinLen(i,j) = std::stod(dataMap["MinLen"][j]);
            OthPredMaxLen(i,j) = std::stod(dataMap["MaxLen"][j]);
        }

        if (Scenario.VarOthPred == 0) {
            for (int j = 0; j <= NOtherPredAge(i); ++j) {
                for (int k = 0; k <= NYears; ++k) {
                    for (int l = 0; l < NSeasons; ++l) {
                        OthPredBiomassSize[i][j][k][l] = std::stod(dataMap["PropBM"][j]) * OthPredBiomass[i][k][l];
                    }
                }
            }
        } else {
            fields   = { "SpeName", "Year", "SizeClass", "PropBM"};
            queryStr = "SELECT SpeName, Year, SizeClass, PropBM FROM ScenarioOthPred WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND ForeName = '" + ForecastName + "'" +
                    " AND Scenario = '" + Scenario.Name + "'" +
                    " AND SpeName = '"  + OtherPredList(i) + "' ORDER BY SpeName,SizeClass,Year";
            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
            if (nmfUtils::checkForError(logger,"ScenarioOthPred","SpeName",dataMap,queryStr))
                return false;
            for (int j = 0; j <= NOtherPredAge(i); ++j) {
                for (int k = 0; k <= NYears; ++k) {
                    for (int l = 0; l < NSeasons; ++l) {
                        OthPredBiomassSize[i][j][k][l] = std::stod(dataMap["PropBM"][k]) * OthPredBiomass[i][k][l];
                    }
                }
            }
        } // end if

    }  // end for i
    // DONE LOADING OTHER PREDATOR BIOMASS FOR THE FORECAST TIME PERIOD


    // LOAD SEASONAL OTHER PREY BIOMASS
    updateProgress(progressVal++, "Loading seasonal other prey biomass...");
    if (ReadRunStopFile() == "Stop")
        return false;

    Boost3DArrayDouble OthPreySeasBiomass(boost::extents[NOthPrey][NYears+1][NSeasons]);

    // First load initial year
    for (int i = 0; i < NOthPrey; ++i) {

        fields = {"Biomass"};
        queryStr = "SELECT Biomass FROM MSVPAOthPreyAnn WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND OthPreyName  = '" + OtherPreyList(i) + "'" +
                " AND Year = " + std::to_string(InitYear) + " ORDER BY Season";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (nmfUtils::checkForError(logger,"MSVPAOthPreyAnn","Biomass",dataMap,queryStr))
            return false;
        for (int j = 0; j < NSeasons; ++j) {
            OthPreySeasBiomass[i][0][j] = std::stod(dataMap["Biomass"][j]);
        }

        if (Scenario.VarOthPrey == 0) {
            for (int j = 1; j <= NYears; ++j) {
                for (int k = 0; k < NSeasons; ++k) {
                    OthPreySeasBiomass[i][j][k] = OthPreySeasBiomass[i][0][k];
                }
            }
        } else {
            fields = {"Biomass"};
            queryStr = "SELECT Biomass FROM ScenarioOthPrey WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND ForeName = '" + ForecastName + "'" +
                    " AND Scenario = '" + Scenario.Name + "'" +
                    " AND SpeName = '"  + OtherPreyList(i) + "' AND Year > 0 ORDER By Year,Season";
            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
            if (nmfUtils::checkForError(logger,"ScenarioOthPrey","Biomass",dataMap,queryStr))
                return false;
            m = 0;
            for (int j = 1; j <= NYears; ++j) {
                for (int k = 0; k < NSeasons; ++k) {
                    OthPreySeasBiomass[i][j][k] = std::stod(dataMap["Biomass"][m++]);
                }
            }
        } // end if
    } // end for i


    // DONE LOADING OTHER PREY SEASONAL BIOMASS >>>>>>>>>>
    // CALCULATE SUITABLE PREY BIOMASS FOR OTHER PREDS EATING OTHER PREY >>>>>>>
    updateProgress(progressVal++, "Loading suitable prey biomass for other predators eating other prey...");
    if (ReadRunStopFile() == "Stop")
        return false;

    // THE BIOMASS OF OTHER PREY AND BIOMASS PREDATORS DO NOT CHANGE, SO THESE CAN BE CALCULATED OUTSIDE OF
    // THE FORECAST ITERATION LOOP FOR EFFICIENCY
    int MinPredSize;
    int MaxPredSize;
    int NPySizeInt;
    bool updateDB = false;
    double TypePref;
    double SpaceO;
    double PredSize;
    double PreySize;
    double SizeSelCoeff;
    double MaxVal;
    double SumSuitBM;
    double SizeInterval;
    double RelIntMin;
    double RelIntMax;
    double SizeSelAlpha;
    double SizeSelBeta;
    double SSB;
    double Recruits = 0.0;
    double tempAnnF;
    std::string cmd;
    std::string errorMsg;
    std::string field0,field1,field2,field3,field4,field5,field6,field7,field8,field9,field10;
    boost::numeric::ublas::matrix<double> tmpSuitBiomass;
    boost::numeric::ublas::matrix<double> tmpPreyBiomass;
    nmfUtils::initialize(tmpSuitBiomass,NYears+1,nmfConstants::MaxNumberAges); // Holds the suitable biomass, by year and age class for a particular prey
    std::vector<std::string> outputFields;

//int i1=0;
//double d1 = 0;

    if (NOthPred > 0) {
        cmd  = "INSERT INTO ForeSuitPreyBiomass (MSVPAname, ForeName, Scenario, PredName, PredAge, PredType, PreyName, ";
        cmd += "PreyAge, Year, Season, SuitPreyBiomass ) values ";
        updateDB = false;
        for (int season = 0; season < NSeasons; ++season) {
            for (int predNum = 0; predNum< NOthPred; ++predNum) {
                for (int age = 0; age <= NOtherPredAge(predNum); ++age) {
                    MinPredSize  = OthPredMinLen(predNum,age);
                    MaxPredSize  = OthPredMaxLen(predNum,age);
                    SizeSelAlpha = OthPredSizeSel[predNum][age][0];
                    SizeSelBeta  = OthPredSizeSel[predNum][age][1];

                    for (int preyNum = 0; preyNum < NOthPrey; ++preyNum) {

                        updateProgress(progressVal++,
                                       "Saving suitable prey biomass for Season: " + std::to_string(season)  +
                                       ", Other Predator: " + std::to_string(predNum) +
                                       ", Age: " + std::to_string(age) +
                                       ", Other Prey: " + std::to_string(preyNum));
                        if (ReadRunStopFile() == "Stop")
                            return false;

                        // Get the type preference for that prey type for the predator species/age
                        TypePref = OthPredOthPyType[predNum][age][preyNum];

                        if (TypePref != 0)
                        {
                            // Then get spatial overlap for the prey type, and season
                            SpaceO = OthPredOthPySpaceO[season][predNum][age][preyNum];
                            if (SpaceO != 0.0)
                            {
                                // Calculate the proportion of prey biomass by size intervals...
                                SizeInterval = OtherPreyCoeff(preyNum,1) - OtherPreyCoeff(preyNum,0);
                                //NPySizeInt = Round(SizeInterval / 0.1, 0)
                                NPySizeInt = int(SizeInterval/0.1+0.5);

                                nmfUtils::initialize(tmpPreyBiomass,NPySizeInt,3); // RSK - remove hardcoding

                                for (int X = 0; X < NPySizeInt; ++X) {
                                    tmpPreyBiomass(X,0) = OtherPreyCoeff(preyNum,0) + (X * 0.1);
                                    tmpPreyBiomass(X,1) = OtherPreyCoeff(preyNum, 0) + ((X + 1) * 0.1);
                                    RelIntMin = (tmpPreyBiomass(X,0) - OtherPreyCoeff(preyNum,0))/SizeInterval;
                                    RelIntMax = (tmpPreyBiomass(X,1) - OtherPreyCoeff(preyNum,0))/SizeInterval;
                                    tmpPreyBiomass(X,2) = nmfUtilsStatistics::BetaI(OtherPreyCoeff(preyNum, 2), OtherPreyCoeff(preyNum, 3), RelIntMax) -
                                                          nmfUtilsStatistics::BetaI(OtherPreyCoeff(preyNum, 2), OtherPreyCoeff(preyNum, 3), RelIntMin);
                                } // end for X

                                for (int year = 0; year <= NYears; ++year) {
                                    SumSuitBM = 0;
                                    for (int YY = MinPredSize; YY <= MaxPredSize; ++YY) {
                                        for (int X = 0; X < NPySizeInt; ++X) {
                                            // Get the size selection index...
                                            if (tmpPreyBiomass(X,0)/YY > 1.0) {
                                                SizeSelCoeff = 0;
                                            } else {
                                                MaxVal = (tmpPreyBiomass(X,1)/YY > 1.0) ? 1.0 : tmpPreyBiomass(X,1)/YY;
                                                SizeSelCoeff = nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, MaxVal) -
                                                               nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, tmpPreyBiomass(X,0)/YY);
                                            }
                                            SumSuitBM += (OthPreySeasBiomass[preyNum][year][season] * tmpPreyBiomass(X,2) * TypePref * SpaceO * SizeSelCoeff);
                                        } // end for X
                                    } // end for YY

                                    tmpSuitBiomass(year,0) = SumSuitBM / (MaxPredSize - MinPredSize);
                                    if (tmpSuitBiomass(year,0) != 0.0) {
                                        updateDB = true;
                                        field0  = std::string("\"")+MSVPAName+"\"";
                                        field1  = std::string("\"")+ForecastName+"\"";
                                        field2  = std::string("\"")+Scenario.Name+"\"";
                                        field3  = std::string("\"")+OtherPredList(predNum)+"\"";
                                        field4  = std::to_string(age);
                                        field5  = std::to_string(0);
                                        field6  = std::string("\"")+ OtherPreyList(preyNum)+"\"";
                                        field7  = std::to_string(0);
                                        field8  = std::to_string(year);
                                        field9  = std::to_string(season);
                                        field10 = std::to_string(tmpSuitBiomass(year,0));

                                        cmd += "("  + field0  + "," + field1  + "," + field2  + "," + field3 +
                                                "," + field4  + "," + field5  + "," + field6  + "," + field7 +
                                                "," + field8  + "," + field9  + "," + field10 + "), ";

                                    } // end if
                                } // end for mm
                            } // end if SpaceO
                        } // end if TypePref
                    } // end for l
                } // end for k
            } // end for j
        } // end for i
    } // end if

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~RSK OK to here

    // Update the database
    if (updateDB) {
        // Remove last comma and space from string
        cmd = cmd.substr(0, cmd.size() - 2);
        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
        if (nmfUtilsQt::isAnError(errorMsg)) {
            logger->logMsg(nmfConstants::Error,"nmfForecast::ModelNoPredatorGrowth: SUITABLE BIOMASS CALCULATION FOR OTHER PREY EATEN BY OTHER PREDS: " + errorMsg);
        }
    }
    //  END SUITABLE BIOMASS CALCULATION FOR OTHER PREY EATEN BY OTHER PREDS >>>>>>>> '

//std::cout <<  cmd << std::endl;

    boost::numeric::ublas::matrix<double> InitSpeM1;
    boost::numeric::ublas::matrix<double> InitSpeF;
    boost::numeric::ublas::matrix<double> InitSpeM2;
    boost::numeric::ublas::matrix<double> InitSpeZ;
    nmfUtils::initialize(InitSpeM1,NSpe,nmfConstants::MaxNumberAges);
    nmfUtils::initialize(InitSpeF, NSpe,nmfConstants::MaxNumberAges);
    nmfUtils::initialize(InitSpeM2,NSpe,nmfConstants::MaxNumberAges);
    nmfUtils::initialize(InitSpeZ, NSpe,nmfConstants::MaxNumberAges);
    Boost4DArrayDouble SpeSeasM1(boost::extents[NSpe][nmfConstants::MaxNumberAges][NYears+1][NSeasons]);

    // LOAD INITIAL MORTALITY RATES..these are the annual rates and are used ONLY to scale catches when necessary
    updateProgress(progressVal++, "Loading initial mortality rates (annual rates used only to scale catches when necessary)...");
    if (ReadRunStopFile() == "Stop")
        return false;

    for (int i = 0; i < NSpe; ++i)
    {
        fields = {"SpeName","Age","AnnF","AnnM2","AnnM1"};
        queryStr = "SELECT SpeName, Age, SUM(SeasF) as AnnF, SUM(SeasM2) as AnnM2, Sum(SeasM1) as AnnM1 FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND Spename = '" + SpeList(i) + "'" +
                " AND Year = " + std::to_string(InitYear-MSVPAFirstYear) + " GROUP BY SpeName, Age";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (nmfUtils::checkForError(logger,"MSVPASeasBiomass","SpeName",dataMap,queryStr))
            return false;
        for (int j = 0; j <=NSpeAge(i); ++j) {
            InitSpeF(i,j)  = std::stod(dataMap["AnnF"][j]);
            InitSpeM2(i,j) = (dataMap["AnnM2"][j]).empty() ? 0 : std::stod(dataMap["AnnM2"][j]);
            InitSpeZ(i,j)  = InitSpeF(i,j) + InitSpeM2(i,j) + std::stod(dataMap["AnnM1"][j]);
            // LOAD SEASONAL VALUES FOR M1
            for (int k = 0; k <= NYears; ++k) {
                for (int l = 0; l < NSeasons; ++l) {
                    SpeSeasM1[i][j][k][l] = std::stod(dataMap["AnnM1"][j]) * (SeasLen(l)/365.0);
                } // end for l
            } // end for k
        } // end for j
    } // end for i


    // LOAD SEASONAL Fs FROM THE SCENARIO TABLE IF APPROPRIATE >>>>>>>>>'
    updateProgress(progressVal++, "Loading Seasonal F's from the scenario table if appropriate...");
    if (ReadRunStopFile() == "Stop")
        return false;

    double CatUnits;
    Boost4DArrayDouble SpeSeasF(boost::extents[NSpe][nmfConstants::MaxNumberAges][NYears+1][NSeasons]);
    Boost4DArrayDouble SpeSeasCatch(boost::extents[NSpe][nmfConstants::MaxNumberAges][NYears+1][NSeasons]);
    Boost3DArrayDouble SpeCatch(boost::extents[NSpe][nmfConstants::MaxNumberAges][NYears+1]);
    if (Scenario.FishAsF) {

        // Find number of years of F data (minus Year 0)
        //int MAX_NYEARS_DATA = 20;  // just hardcode it for testing

        for (int i = 0; i < NSpe; ++i)
        {
            fields = {"SpeName","Age","Season","SeasF"};
            queryStr = "SELECT SpeName, Age, Season, SeasF FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND Spename = '" + SpeList(i) + "'" +
                    " AND Year = " + std::to_string(InitYear - MSVPAFirstYear) + " ORDER BY SpeName, Age, Season";
            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
            if (nmfUtils::checkForError(logger,"MSVPASeasBiomass","SpeName",dataMap,queryStr))
                return false;
            m = 0;
            for (int j = 0; j <= NSpeAge(i); ++j) {
                for (int k = 0; k < NSeasons; ++k) {
                    SpeSeasF[i][j][0][k] = std::stod(dataMap["SeasF"][m++]);
                } // end for k
            } // end for j
        } // end for i

        if (Scenario.VarF == 0) {
            for (int i = 0; i < NSpe; ++i) {
                for (int j = 0; j <= NSpeAge(i); ++j) {
                    for (int k = 1; k <= NYears; ++k) {
                        for (int l = 0; l < NSeasons; ++l) {
                            SpeSeasF[i][j][k][l] = SpeSeasF[i][j][0][l];
//std::cout << "1 SpeSeasF[" << i << "][" << j << "][" << k << "][" << l << "]: " << SpeSeasF[i][j][k][l] << std::endl;
                        }
                    } // end for k
                } // end for j
            } // end for i
        } else {

            for (int i = 0; i < NSpe; ++i) {
                fields = {"Age","Year","F"};
                queryStr = "SELECT Age,Year,F FROM ScenarioF WHERE MSVPAname = '" + MSVPAName + "'" +
                        " AND ForeName = '" + ForecastName + "'" +
                        " AND Scenario = '" + Scenario.Name + "'" +
                        " AND SpeName = '"  + SpeList(i) + "' AND Year > 0 ORDER By Age, Year";
//std::cout << queryStr << std::endl;
                dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
                if (nmfUtils::checkForError(logger,"ScenarioF","Age",dataMap,queryStr))
                    return false;
/*
 *
 *
 * The problem is that there are 20 years worth of data but NYears is just set to 4!!!!!!!!!!!!!!!!!!!!!   RSK - fix this!!!!!!
 *
 */
//std::cout << "NSpeAge(i): " << NSpeAge(i) << std::endl;
//std::cout << "NYears: " << NYears << std::endl;

                m = 0;
                for (int j = 0; j <= NSpeAge(i); ++j) {
                    for (int k = 1; k <= NYears; ++k) {
//std::cout << "m: " << m << std::endl;
                        for (int l = 0; l < NSeasons; ++l) {
                            SpeSeasF[i][j][k][l] = std::stod(dataMap["F"][m]) * (SeasLen(l)/365.0);
//std::cout << "2 SpeSeasF[" << i << "][" << j << "][" << k << "][" << l << "]: " << SpeSeasF[i][j][k][l] << std::endl;
                        }
                        ++m;
                    } // end for k

                    // RSK - not sure about this nor about the above "The problem is that..." comment.
//                    for (int skip = 0; skip < MAX_NYEARS_DATA-NYears; ++skip) {
//                        ++m;
//                    }

                } // end j
            } // end for i

        } // end if

    } else {

     for (int i = 0; i < NSpe; ++i)
     {
         CatUnits = CatchUnits[SpeList(i)];

         for (int j = 0; j <= NSpeAge(i); ++j) {
             fields = {"SpeName", "Age", "Catch"};
             queryStr = "SELECT SpeName, Age, Catch FROM SpeCatch WHERE SpeName = '" + SpeList(i) + "'" +
                     " AND Age = "  + std::to_string(j) +
                     " AND Year = " + std::to_string(InitYear) + " ORDER BY SpeName, Age";
             dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
             if (nmfUtils::checkForError(logger,"SpeCatch","SpeName",dataMap,queryStr))
                 return false;
             SpeCatch[i][j][0] = (dataMap["SpeName"].size() > 0) ? std::stod(dataMap["Catch"][0]) * CatUnits : 0;
         } // end for j

         if (Scenario.VarF == 0) {
             for (int j = 0; j <= NSpeAge(i); ++j) {
                 for (int k = 1; k <= NYears; ++k) {
                     SpeCatch[i][j][k] = SpeCatch[i][j][0];
                 } // end for k
             } // end for j
         } else {
             fields = {"F"};
             queryStr = "SELECT F FROM ScenarioF WHERE MSVPAname = '" + MSVPAName + "'" +
                     " AND ForeName = '" + ForecastName + "'" +
                     " AND Scenario = '" + Scenario.Name + "'" +
                     " AND SpeName = '"  + SpeList(i) + "' AND Year > 0 ORDER By Age, Year";
             dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
             if (nmfUtils::checkForError(logger,"ScenarioF","F",dataMap,queryStr))
                 return false;
             m = 0;
             for (int j = 0; j <= NSpeAge(i); ++j) {
                 for (int k = 1; k <= NYears; ++k) {
                     SpeCatch[i][j][k] = std::stod(dataMap["F"][m++]) * CatUnits;
                 } // end for k
             } // end for j
         } // end if
     } // end for i

    } // end if

    //  LOAD RECRUITMENT CORRECTIONS IF APPROPRIATE
    updateProgress(progressVal++, "Loading recruitment corrections if appropriate...");
    if (ReadRunStopFile() == "Stop")
        return false;

    boost::numeric::ublas::vector<int> AbsRecruits;
    boost::numeric::ublas::matrix<double> SpeVarRec;
    nmfUtils::initialize(SpeVarRec,NSpe,NYears+1);
    nmfUtils::initialize(AbsRecruits,NSpe);

    if (Scenario.VarRec == 1) {
        for (int i = 0; i < NSpe; ++i) {
            fields = {"AbsRecruits","RecAdjust"};
            queryStr = "SELECT AbsRecruits,RecAdjust FROM ScenarioRec WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND ForeName = '" + ForecastName + "'" +
                    " AND Scenario = '" + Scenario.Name + "'" +
                    " AND SpeName = '"  + SpeList(i) + "' ORDER By Year";
            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
            if (nmfUtils::checkForError(logger,"ScenarioRec","AbsRecruits",dataMap,queryStr))
                return false;
            AbsRecruits(i) = std::stoi(dataMap["AbsRecruits"][0]);
            for (int j = 0; j <= NYears; ++j) {
                SpeVarRec(i,j) = std::stod(dataMap["RecAdjust"][j]);
            } // end for j
        }
    } else {
        for (int i = 0; i < NSpe; ++i) {
            for (int j = 0; j <= NYears; ++j) {
                SpeVarRec(i,j) = 1.0;
            } // end for j
        } // end for i
    }

    updateProgress(progressVal++, "Loading initial biomasses and abundances for all species...");
    if (ReadRunStopFile() == "Stop")
        return false;

    // LOAD INITIAL BIOMASSES AND ABUNDANCES FOR ALL SPECIES
    boost::numeric::ublas::matrix<double> InitSpeBiomass;
    boost::numeric::ublas::matrix<double> InitSpeAbundance;
    boost::numeric::ublas::matrix<double> AnnInitAbundance;
    boost::numeric::ublas::matrix<double> InitSpeSize;
    boost::numeric::ublas::matrix<double> InitSpeWeight;
    nmfUtils::initialize(InitSpeBiomass,  NSpe, nmfConstants::MaxNumberAges);
    nmfUtils::initialize(InitSpeAbundance,NSpe, nmfConstants::MaxNumberAges);
    nmfUtils::initialize(AnnInitAbundance,NSpe, nmfConstants::MaxNumberAges);
    nmfUtils::initialize(InitSpeSize,     NSpe, nmfConstants::MaxNumberAges);
    nmfUtils::initialize(InitSpeWeight,   NSpe, nmfConstants::MaxNumberAges);

    for (int i = 0; i < NSpe; ++i) {
        fields = {"AnnBiomass","AnnAbund"};
        queryStr = "SELECT AnnBiomass,AnnAbund FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                " AND Spename = '" + SpeList(i) + "'" +
                " AND Season = 0 and Year = " + std::to_string(InitYear-MSVPAFirstYear) + " ORDER By Age";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (nmfUtils::checkForError(logger,"MSVPASeasBiomass","AnnBiomass",dataMap,queryStr))
            return false;
        for (int j = 0; j <= NSpeAge(i); ++j) {
            InitSpeBiomass(i,j)   = std::stod(dataMap["AnnBiomass"][j]);
            InitSpeAbundance(i,j) = std::stod(dataMap["AnnAbund"][j]);
            AnnInitAbundance(i,j) = std::stod(dataMap["AnnAbund"][j]);
        } // end for j
    } // end for i

    // The progressVal term was -1
    updateProgress(progressVal++,"Setup - End. Progress may now be monitored in Progress Chart window.");
    if (ReadRunStopFile() == "Stop")
        return false;

    // ALL INITIAL DATA ARE NOW LOADED >>>> START RUNNING THE FORECAST >>>>>>>'

    // INITIALIZE ARRAYS TO HOLD DAILY TIME STEP OUTPUTS


    // INITIALIZE ARRAYS TO HOLD DAILY TIME STEP OUTPUTS

    //Dim SpeSBM() As Double
    //Dim OthPredSBM() As Double
    //Dim OthPreySBM() As Double

    int FirstSeasDay = 0;
    int NumRecords = 0;
    int FinalYear = 0;
    bool FishWarn = false;
    bool userHalted = false;
    double OtherM = 0.0;
    double InitDayCatch = 0.0;
    double tmpZval = 0.0;
    double tmpFval = 0.0;
    double tmpMval = 0.0;
    double StomCorr = 0.0;
    double TotalBMRemoved = 0.0;
    double TotalNRemoved = 0.0;
    double TotalEaten = 0.0;
    double TotalCaught = 0.0;
    double DailyFandM = 0.0;
    std::string val="";
    std::vector<std::string> TotalInitBiomass;
    std::vector<std::string> SpeciesName;
    std::string msg;
    Boost3DArrayDouble DailySpeBiomass;
    Boost3DArrayDouble DailySpeAbund;
    Boost4DArrayDouble SpeRemovals(boost::extents[NSpe][nmfConstants::MaxNumberAges][NSpe][nmfConstants::MaxNumberAges]); // removals of MSVPA prey by MSVPA predators
    Boost4DArrayDouble OthPdSpeRemovals(boost::extents[NOthPred][nmfConstants::MaxNumberAges][NSpe][nmfConstants::MaxNumberAges]);

    //frmForeCastExecute.lblProgress.Caption = "Finished Loading....Starting Forecast"
    // frmForeCastExecute.ProgBar.Max = (NYears + 1) * NSeas

    logger->logMsg(nmfConstants::Normal,"Forecast: Processing Begin");

    //
    // Start Forecast Year Main Loop
    //
    for (int Year = 0; Year <= NYears; ++Year) {

        if (userHalted)
            break;

        for (int Seas = 0; Seas < NSeasons; ++Seas) {

            while ((val=ReadRunStopFile()) == "Pause") {;}
            if (val == "Stop") {
                userHalted = true;
                break;
            }

            FirstSeasDay = (Seas == 0) ? 0 : FirstSeasDay + SeasLen(Seas - 1);

            Boost3DArrayDouble DailySpeSize(boost::extents[NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            DailySpeBiomass.resize(boost::extents[NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            Boost3DArrayDouble DailySpeWt(boost::extents[NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            //Boost3DArrayDouble DailySpeAbund(boost::extents[NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            DailySpeAbund.resize(boost::extents[NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            Boost3DArrayDouble DailySpeM2(boost::extents[NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            Boost3DArrayDouble DailySpeF(boost::extents[NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            Boost3DArrayDouble DailyZ(boost::extents[NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            Boost3DArrayDouble CatchScale(boost::extents[NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            Boost3DArrayDouble TotSpeSBM(boost::extents[NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            Boost3DArrayDouble SpeTotalBMConsumed(boost::extents[NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]); // total biomass prey consumed by MSVPA preds
            Boost3DArrayDouble TotOthPredSBM(boost::extents[NOthPred][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            Boost3DArrayDouble OthTotalBMConsumed(boost::extents[NOthPred][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            Boost3DArrayDouble OthPredOthPreySBM(boost::extents[NOthPred][nmfConstants::MaxNumberAges][NOthPrey]);
            Boost4DArrayDouble SpeOthPropDiet(boost::extents[NSpe][nmfConstants::MaxNumberAges][NOthPrey][SeasLen(Seas)+1]);
            Boost4DArrayDouble OthPdOthPropDiet(boost::extents[NOthPred][nmfConstants::MaxNumberAges][NOthPrey][SeasLen(Seas)+1]);
            Boost5DArrayDouble SpePropDiet(boost::extents[NSpe][nmfConstants::MaxNumberAges][NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);
            Boost5DArrayDouble OthPdSpePropDiet(boost::extents[NOthPred][nmfConstants::MaxNumberAges][NSpe][nmfConstants::MaxNumberAges][SeasLen(Seas)+1]);

            // Update main GUI
            QApplication::processEvents();

            for (int i = 0; i < NSpe; ++i) {
                for (int j = 0; j <= NSpeAge(i); ++j) {
                    DailySpeBiomass[i][j][0] = InitSpeBiomass(i,j);
                    DailySpeAbund[i][j][0]   = InitSpeAbundance(i,j);

                    for (int k = 0; k <= SeasLen(Seas); ++k) {
                        DailySpeSize[i][j][k] = SeasPreySize[i][j][Seas][k];
                        DailySpeWt[i][j][k]   = SeasPreyWeight[i][j][Seas][k];
                    } // end for k

                    // Approach for estimating seasonal/daily catches in scenarios where fishery mortality is entered as catch
                    if (! Scenario.FishAsF) {
                        if (SpeCatch[i][j][Year] > 0)
                        {
                            tmpMval = InitSpeZ(i,j) - InitSpeF(i,j); // This is M from the previous year used for the scaling
                            nmfUtilsSolvers::SolveF(SpeCatch[i][j][Year], AnnInitAbundance(i,j), tmpMval, tmpFval);

                            FishWarn = (tmpFval == 0.0);

                            tmpZval = (tmpFval + tmpMval) / 365.0;
                            tmpFval /= 365.0;

                            InitDayCatch = (tmpFval / tmpZval) * (1 - std::exp(-tmpZval)) * AnnInitAbundance(i,j);
                            // convert to proportion of total catch
                            InitDayCatch /= SpeCatch[i][j][Year];

                            for (int Day = 1; Day <= SeasLen(Seas); ++Day) {
                                //CatchScale[i][j][Day] = InitDayCatch * std::exp(-tmpZval * 365.0 * ((FirstSeasDay + Day - 1.0) / 365.0));
                                CatchScale[i][j][Day] = InitDayCatch * std::exp(-tmpZval * (FirstSeasDay + Day - 1.0));
                            } // end for Day
                        } // end if SpeCatch...
                    } // end if ! Scenario...

                } // end for j
            } // end for i

            if (NOthPred > 0) {
                for (int i = 0; i < NOthPred; ++i) {
                    for (int j = 0; j < NOthPrey; ++j) {
                        fields = {"PredAge","SuitPreyBiomass"};
                        queryStr = "SELECT PredAge,SuitPreyBiomass FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                                " AND ForeName = '" + ForecastName +     "'" +
                                " AND Scenario = '" + Scenario.Name +  "'" +
                                " AND PredName = '" + OtherPredList(i) + "'" +
                                " AND PreyName = '" + OtherPreyList(j) + "'" +
                                " AND Year = "   + std::to_string(Year) +
                                " AND Season = " + std::to_string(Seas) + " ORDER BY PredAge";
                        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
                        if (nmfUtils::checkForError(logger,"ForeSuitPreyBiomass","PredAge",dataMap,queryStr))
                            return false;
                        NumRecords = dataMap["SuitPreyBiomass"].size();
                        if (NumRecords > 0)
                        {
                            for (int k = 0; k <= NOtherPredAge(i); ++k) { // RSK double check this logic
                                if (k >= NumRecords)
                                    break;
                                if (std::stoi(dataMap["PredAge"][k]) == k) {
                                    OthPredOthPreySBM[i][k][j] = std::stod(dataMap["SuitPreyBiomass"][k]);
                                } // end if
                            } // end for k
                        } // end if
                    } // end j
                } // end for i
            } // end if

            // BEGIN THE DAILY ITERATION LOOP

            boost::numeric::ublas::matrix<double> AvgStomCorr;
            boost::numeric::ublas::matrix<double> AvgOthPredStomCorr;
            nmfUtils::initialize(AvgStomCorr,        nmfConstants::MaxNumberSpecies, nmfConstants::MaxNumberAges);
            nmfUtils::initialize(AvgOthPredStomCorr, nmfConstants::MaxNumberSpecies, nmfConstants::MaxNumberAges);



            msg = "Processing Year " + std::to_string(Year) + " of " + std::to_string(NYears) +
                         ", Season " + std::to_string(Seas+1) + " of " + std::to_string(NSeasons);
            logger->logMsg(nmfConstants::Normal,"Forecast: "+msg);
            outputProgressLabel(msg);



            updateProgress(progressVal++,msg);
            if (ReadRunStopFile() == "Stop")
                return false;




            for (int Day = 1; Day <= SeasLen(Seas); ++Day) {
                Boost3DArrayDouble OthPreySBM(boost::extents[NSpe][nmfConstants::MaxNumberAges][NOthPrey]);
                Boost4DArrayDouble SpeSBM(boost::extents[NSpe][nmfConstants::MaxNumberAges][NSpe][nmfConstants::MaxNumberAges]);
                Boost4DArrayDouble OthPredSBM(boost::extents[NOthPred][nmfConstants::MaxNumberAges][NSpe][nmfConstants::MaxNumberAges]);

                if (NOthPred > 0) {
                    for (int i = 0; i < NOthPred; ++i) {
                        for (int j = 0; j <= NOtherPredAge(i); ++j) {
                            for (int k = 0; k < NOthPrey; ++k) {
                                TotOthPredSBM[i][j][Day] += OthPredOthPreySBM[i][j][k];
                            } // end for k
                        } // end for j
                    } // end for i
                } // end if

                // Calcualte prey selection for the day

                // FIRST DOING MSVPA PREDATORS EATING MSVPA PREY
                PredCount = 0;
                for (int i = 0; i < NSpe; ++i) {
                    if (SpeType(i) == 0) {  // its a predator...so calculate suitable biomass for each age class by year, prey type, prey age
                        for (int j = 0; j <= NSpeAge(i); ++j) {
                            SizeSelAlpha = MSVPASizeSel[PredCount][j][0];
                            SizeSelBeta  = MSVPASizeSel[PredCount][j][1];
                            for (int k = 0; k < NSpe; ++k) {
                                // Then get the type pref for that prey type for the predator species/age
                                TypePref = MSVPATypePref[PredCount][j][k];
                                if (TypePref != 0.0)
                                {
                                    // Then get spatial overlap for the prey type, and season
                                    SpaceO = MSVPASpaceO[Seas][PredCount][j][k];
                                    if (SpaceO != 0.0) {

                                        PredSize = DailySpeSize[i][j][Day-1];

                                        for (int nn = 0; nn <= NSpeAge(k); ++nn)  {
                                            // Get prey size
                                            PreySize = DailySpeSize[k][nn][Day-1];
                                            if (double(PreySize)/PredSize > 1.0) {
                                                SizeSelCoeff = 0.0;
                                            } else {
                                                MaxVal = ((PreySize+0.05)/double(PredSize) > 1.0) ? 1.0 : (PreySize+0.05)/PredSize;
                                                SizeSelCoeff = nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, MaxVal) -
                                                               nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, (PreySize-0.05)/PredSize);
                                            }
                                            SpeSBM[PredCount][j][k][nn]   = DailySpeBiomass[k][nn][Day-1] * TypePref * SpaceO * SizeSelCoeff;
                                            TotSpeSBM[PredCount][j][Day] += SpeSBM[PredCount][j][k][nn];
                                        } // end nn
                                    } // end if
                                } // end if TypeDef
                            } // end for k
                        } // end for j
                        ++PredCount;
                    } // end if
                } // end for i
                // END OF SUITABLE PREY BIOMASS CALCULATION LOOP FOR MSVPA Preds EATING MSVPA Prey

                // THEN DO OTHER PREDATORS EATING MSVPA PREY
                if (NOthPred > 0) {
                    for (int i = 0; i < NOthPred; ++i) {
                        for (int j = 0; j <= NOtherPredAge(i); ++j) {
                            MinPredSize  = OthPredMinLen(i,j);
                            MaxPredSize  = OthPredMaxLen(i,j);
                            SizeSelAlpha = OthPredSizeSel[i][j][0];
                            SizeSelBeta  = OthPredSizeSel[i][j][1];
                            for (int k = 0; k < NSpe; ++k) {
                                // First get the type pref for the SizeCat and Prey Type
                                TypePref = OthPredTypePref[i][j][k];
                                if (TypePref != 0.0) {
                                    // Then get the spatial overlap similarly
                                    SpaceO = OthPredSpaceO[Seas][i][j][k];
                                    if (SpaceO != 0.0)
                                    {
                                        // CALCULATE SIZE SELECTIVITY
                                        for (int nn = 0; nn <= NSpeAge(k); ++nn) {
                                            PreySize = DailySpeSize[k][nn][Day-1];
                                            SumSuitBM = 0;
                                            for (int X = MinPredSize; X <= MaxPredSize; ++X) { // integrate size pref across predator size for the category.
                                                // Get the size selection index...
                                                if (PreySize / double(X) > 1.0) {
                                                    SizeSelCoeff = 0.0;
                                                } else {
                                                    MaxVal = (PreySize+0.05)/double(X) > 1.0  ? 1.0 : (PreySize+0.05)/double(X);
                                                    SizeSelCoeff = nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, MaxVal) -
                                                                   nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, (PreySize-0.05)/double(X));
                                                }
                                                SumSuitBM += (DailySpeBiomass[k][nn][Day-1] * TypePref * SpaceO * SizeSelCoeff);
                                            } // end for X
                                            OthPredSBM[i][j][k][nn]   = SumSuitBM/(MaxPredSize-MinPredSize);
                                            TotOthPredSBM[i][j][Day] += OthPredSBM[i][j][k][nn];
                                        }
                                    } // end if SpaceO
                                } // end if TypePref
                            } // end for k
                        } // end for j
                    } // end for i
                } // end if

                // END SUITABLE BIOMASS CALCULATIONS FOR OTHER PREDATORS EATING MSVPA SPECIES

                // FINALLY DO MSVPA PREDATORS EATING OTHER PREY
                PredCount = 0;
                for (int i = 0; i<NSpe; ++i) {
                    if (SpeType(i) == 0) {
                        for (int j = 0; j <= NSpeAge(i); ++j) {
                            SizeSelAlpha = MSVPASizeSel[PredCount][j][0];
                            SizeSelBeta  = MSVPASizeSel[PredCount][j][1];
                            for (int k = 0; k < NOthPrey; ++k) {
                                // Get the type pref for that prey type for the predator species/age
                                TypePref = MSVPAOthPyType[PredCount][j][k];
                                if (TypePref != 0.0) {
                                    // Then get spatial overlap for the prey type, and season
                                    SpaceO = MSVPAOthPySpaceO[Seas][PredCount][j][k];
                                    if (SpaceO != 0.0) {
                                        // Calculate the proportion of prey biomass by size intervals...
                                        SizeInterval = OtherPreyCoeff(k,1) - OtherPreyCoeff(k,0);
                                        // NPySizeInt = Round(SizeInterval / 0.1, 0)
                                        NPySizeInt = int(SizeInterval/0.1+0.5);
                                        nmfUtils::initialize(tmpPreyBiomass,NPySizeInt,3);
                                        for (int X = 0; X < NPySizeInt; ++X) {
                                            tmpPreyBiomass(X, 0) = OtherPreyCoeff(k, 0) + (X * 0.1);
                                            tmpPreyBiomass(X, 1) = OtherPreyCoeff(k, 0) + ((X + 1) * 0.1);
                                            RelIntMin = (tmpPreyBiomass(X, 0) - OtherPreyCoeff(k, 0)) / SizeInterval;
                                            RelIntMax = (tmpPreyBiomass(X, 1) - OtherPreyCoeff(k, 0)) / SizeInterval;
                                            tmpPreyBiomass(X, 2) = nmfUtilsStatistics::BetaI(OtherPreyCoeff(k,2), OtherPreyCoeff(k,3), RelIntMax) -
                                                                   nmfUtilsStatistics::BetaI(OtherPreyCoeff(k,2), OtherPreyCoeff(k,3), RelIntMin);
                                        }
                                        PredSize = DailySpeSize[i][j][Day-1];
                                        SumSuitBM = 0;
                                        for (int X = 0; X < NPySizeInt; ++X) {
                                            // Get the size selection index...
                                            if (tmpPreyBiomass(X, 0) / PredSize > 1.0) {
                                                SizeSelCoeff = 0.0;
                                            } else {
                                                MaxVal = (tmpPreyBiomass(X,1)/PredSize > 1.0) ? 1.0 : tmpPreyBiomass(X,1)/PredSize;
                                                SizeSelCoeff = nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, MaxVal) -
                                                               nmfUtilsStatistics::BetaI(SizeSelAlpha, SizeSelBeta, tmpPreyBiomass(X, 0) / PredSize);
                                            }
                                            SumSuitBM += (OthPreySeasBiomass[k][Year][Seas] * tmpPreyBiomass(X,2) * SpaceO * TypePref * SizeSelCoeff);
                                        } // end for X
                                        OthPreySBM[PredCount][j][k]   = SumSuitBM;
                                        TotSpeSBM[PredCount][j][Day] += OthPreySBM[PredCount][j][k];
                                    } // end if SpaceO
                                } // end if TypePref
                            } // end for k
                        } // end for j
                        ++PredCount;
                    } // end if SpeType
                } // end for i
                // END OF SUITABLE BIOMASS CALCULATIONS FOR MSVPA SPECIES EATING OTHER PREY

                // Have suitable biomass by prey type and total suitable prey biomass for both MSVPA and Other Predators
                // Can now calculate prop diets, and removals of prey

                PredCount = 0;
                for (int i = 0; i < NSpe; ++i) {
                    if (SpeType(i) == 0) {
                        for (int j = 0; j <= NSpeAge(i); ++j) {
                            // CALUCULATE CORRECTION FOR STOMACH CONTENTS BASED ON PREY AVAILABILITY
                            StomCorr = StomCont[PredCount][j][Seas] +
                                    (std::log(TotSpeSBM[PredCount][j][Day]/MSVPAAvgSBM[PredCount][j][Seas]) * StomCont[PredCount][j][Seas]);

                            if (StomCorr < (0.1 * StomCont[PredCount][j][Seas])) {
                                StomCorr = 0.1 * StomCont[PredCount][j][Seas];
                            }
                            if (StomCorr > (3.0 * StomCont[PredCount][j][Seas])) {
                                StomCorr = 3.0 * StomCont[PredCount][j][Seas];
                            }

                            AvgStomCorr(PredCount,j) += StomCorr;
                            // Calculate total prey consumed
                            SpeTotalBMConsumed[PredCount][j][Day] = DailySpeBiomass[i][j][Day-1] * 24.0 * StomCorr *
                                    (ConsAlpha(PredCount,j) * std::exp(ConsBeta(PredCount,j) * SeasTemp(Seas)));

                            // Calculate PropDiet and removals of Prey Species/Age Class for MSVPA prey
                            for (int k = 0; k < NSpe; ++k) {
                                for (int l = 0; l <= NSpeAge(k); ++l) {
                                    SpePropDiet[PredCount][j][k][l][Day] = SpeSBM[PredCount][j][k][l]/TotSpeSBM[PredCount][j][Day];
                                    SpeRemovals[i][j][k][l] = SpePropDiet[PredCount][j][k][l][Day] * SpeTotalBMConsumed[PredCount][j][Day];
                                } // end for l
                            } // end for k

                            // Calculate prop diet for Other Prey
                            for (int k = 0; k < NOthPrey; ++k) {
                                SpeOthPropDiet[PredCount][j][k][Day] = OthPreySBM[PredCount][j][k] / TotSpeSBM[PredCount][j][Day];
                            } // end for k
                        } // end for j
                        ++PredCount;
                    } // end if SpeType...

                } // end for i
                // END CALCULATION OF REMOVALS BY MSVPA PREDATORS

                // CALCULATE THE MSVPA PREY removals by BIOMASS predators
                if (NOthPred > 0) {
                    for (int i = 0; i <  NOthPred; ++i) {
                        for (int j = 0; j <= NOtherPredAge(i); ++j) {
                            StomCorr = OthPredStomCont[i][j][Seas] +
                                    (std::log(TotOthPredSBM[i][j][Day]/OthPredAvgSBM[i][j][Seas]) * OthPredStomCont[i][j][Seas]);

                            if (StomCorr < 0.1 * OthPredStomCont[i][j][Seas]) {
                                StomCorr = 0.1 * OthPredStomCont[i][j][Seas];
                            }
                            if (StomCorr > 3.0 * OthPredStomCont[i][j][Seas]) {
                                StomCorr = 3.0 * OthPredStomCont[i][j][Seas];
                            }
                            AvgOthPredStomCorr(i, j) += StomCorr;
                            // Calculate total consumption
                            OthTotalBMConsumed[i][j][Day] = OthPredBiomassSize[i][j][Year][Seas] * 24.0 * StomCorr *
                                    (OthPredConsAlpha(i,j) * std::exp(OthPredConsBeta(i,j) * SeasTemp(Seas)));
                            // Calculate PropDiet and removals of Prey Species/Age Class for MSVPA prey
                            for (int k = 0; k < NSpe; ++k) {
                                for (int l = 0; l <= NSpeAge(k); ++l) {
                                    OthPdSpePropDiet[i][j][k][l][Day] = OthPredSBM[i][j][k][l]/TotOthPredSBM[i][j][Day];
                                    OthPdSpeRemovals[i][j][k][l] = OthPdSpePropDiet[i][j][k][l][Day] * OthTotalBMConsumed[i][j][Day];
                                }
                            }
                            // Calculate prop diet for other prey
                            for (int k = 0; k < NOthPrey; ++k) {
                                OthPdOthPropDiet[i][j][k][Day] = OthPredOthPreySBM[i][j][k]/TotOthPredSBM[i][j][Day];
                            }
                        } // end for j
                    } // end for i
                } // end if

                // Finally solve for daily mortality rates for preds and prey and pop sizes for the next day...
                // given initial N, total number removed during the time step, and other mortality rates = F+M1 can solve
                // for M2 and Z iteratively using function SolveF
                // then use that to project the pop size forward a day for both preds and prey...

                for (int i = 0; i < NSpe; ++i) {
                    for (int j = 0; j <= NSpeAge(i); ++j) {
                        TotalBMRemoved = 0.0;
                        // Calculate the total biomass removed by MSVPA predators for this species (i) and age class (j)
                        for (int k = 0; k < NSpe; ++k) {
                            for (int l = 0; l <= NSpeAge(k); ++l) {
                                TotalBMRemoved += SpeRemovals[k][l][i][j];
                            }
                        }

                        if (NOthPred > 0) {
                            for (int k = 0; k < NOthPred; ++k) {
                                for (int l = 0; l <= NOtherPredAge(k); ++l) {
                                    TotalBMRemoved += OthPdSpeRemovals[k][l][i][j];
                                } // end for l
                            } // end for k
                        }  // end if

                        // Convert this to numbers..divided through by speweight
                        TotalNRemoved = TotalBMRemoved / DailySpeWt[i][j][Day-1];

                        if (Scenario.FishAsF) {
                            // Solve for mortality rate given M1 and F

                            OtherM = (SpeSeasM1[i][j][Year][Seas]/SeasLen(Seas)) + (SpeSeasF[i][j][Year][Seas]/SeasLen(Seas));
                            if (TotalBMRemoved > 0) {
                                nmfUtilsSolvers::SolveF(TotalNRemoved, DailySpeAbund[i][j][Day-1], OtherM, DailySpeM2[i][j][Day]);
                                DailyZ[i][j][Day] = OtherM + DailySpeM2[i][j][Day];
                            } else {
                                DailyZ[i][j][Day] = OtherM;
                            }
                            TotalCaught = ((SpeSeasF[i][j][Year][Seas]/SeasLen(Seas)) / DailyZ[i][j][Day]) *
                                    DailySpeAbund[i][j][Day-1] * (1.0 - std::exp(-DailyZ[i][j][Day]));
                        } else {
                            // CODE TO SOLVE FOR F and M2 GIVEN CATCH
                            TotalEaten = TotalNRemoved;
                            TotalCaught = SpeCatch[i][j][Year] * CatchScale[i][j][Day];

                            TotalNRemoved = TotalEaten + TotalCaught;
                            OtherM = SpeSeasM1[i][j][Year][Seas] / SeasLen(Seas);

                            if (TotalNRemoved > 0) {
                                nmfUtilsSolvers::SolveF(TotalNRemoved, DailySpeAbund[i][j][Day-1], OtherM, DailyFandM);
                                DailyZ[i][j][Day] = OtherM + DailyFandM;
                            } else {
                                DailyZ[i][j][Day] = OtherM;
                            }

                            DailySpeM2[i][j][Day] = (TotalEaten == 0.0) ? 0.0 :
                               (TotalEaten * DailyZ[i][j][Day]) / ((1.0 - std::exp(-DailyZ[i][j][Day])) * DailySpeAbund[i][j][Day-1]);

                            DailySpeF[i][j][Day] = (TotalCaught == 0.0) ? 0.0 :
                               (TotalCaught * DailyZ[i][j][Day]) / ((1.0 - std::exp(-DailyZ[i][j][Day])) * DailySpeAbund[i][j][Day-1]);

                        }

                        // project the population forward to today
                        DailySpeAbund[i][j][Day]   = DailySpeAbund[i][j][Day-1] * std::exp(-DailyZ[i][j][Day]);
                        DailySpeBiomass[i][j][Day] = DailySpeAbund[i][j][Day] * DailySpeWt[i][j][Day];

                        SpeSeasCatch[i][j][Year][Seas] += TotalCaught;

                    } // end for j
                } // end for i

            } // end for Day

            // Calculate seasonal averages of otuput variables and store in the database
            double AvgSize;
            double AvgWeight;
            double SeasonalM2;
            double SeasonalF;
            double TotalBMConsumed;
            double TotSBM;
            double InitN;
            double EndN;
            double InitBM;
            double EndBM;
            double Catch;
            double Yield;
            double SeasZ;
            double StomCorrOut;

            boost::numeric::ublas::vector<double> AvgOthPropDiet;
            boost::numeric::ublas::matrix<double> AvgPropDiet;

            PredCount = 0;
            for (int i = 0; i < NSpe; ++i) {
                for (int j = 0; j <= NSpeAge(i); ++j) {
                    StomCorrOut = 0;
                    AvgSize = 0;
                    AvgWeight = 0;
                    SeasonalM2 = 0;
                    SeasonalF = 0;
                    TotalBMConsumed = 0;
                    TotSBM = 0;
                    nmfUtils::initialize(AvgPropDiet, NSpe, nmfConstants::MaxNumberAges);
                    nmfUtils::initialize(AvgOthPropDiet, NOthPrey);

                    if (SpeType(i) == 0) {
                        StomCorrOut = AvgStomCorr(PredCount,j) / SeasLen(Seas);
                    }

                    for (int k = 1; k <= SeasLen(Seas); ++k) {
                        AvgSize    += DailySpeSize[i][j][k];
                        AvgWeight  += DailySpeWt[i][j][k];
                        SeasonalM2 += DailySpeM2[i][j][k];
                        SeasonalF  += DailySpeF[i][j][k];
                        if (SpeType(i) == 0) {
                            TotalBMConsumed += SpeTotalBMConsumed[PredCount][j][k]; // total amount of prey consumed by the predator
                            TotSBM += TotSpeSBM[PredCount][j][k];
                            for (int l = 0; l < NSpe; ++l) {
                                for (int nn = 0; nn <= NSpeAge(l); ++nn) {
                                    AvgPropDiet(l,nn) += SpePropDiet[PredCount][j][l][nn][k];
                                }
                            }
                            for (int l = 0; l < NOthPrey; ++l) {
                                AvgOthPropDiet(l) += SpeOthPropDiet[PredCount][j][l][k];
                            }
                        } // end if
                    } // end for k

                    AvgSize   /= SeasLen(Seas); // average size during the season
                    AvgWeight /= SeasLen(Seas); // average weight during the season
                    TotSBM    /= SeasLen(Seas); // average total suitable prey biomass
                    for (int l = 0; l < NSpe; ++l) {
                        for (int nn = 0; nn <= NSpeAge(l); ++nn) {
                            AvgPropDiet(l,nn) /= SeasLen(Seas);
                        }
                    }

                    for (int l = 0; l < NOthPrey; ++l) {
                        AvgOthPropDiet(l) /= SeasLen(Seas);
                    }

                    // Also save initial and end population size (Abundance) and Biomass
                    InitN  = DailySpeAbund[i][j][0];
                    EndN   = DailySpeAbund[i][j][SeasLen(Seas)];
                    InitBM = DailySpeBiomass[i][j][0];
                    EndBM  = DailySpeBiomass[i][j][SeasLen(Seas)];

                    // Calculate fishery yields
                    if (Scenario.FishAsF) {
//std::cout << "3 SpeSeasF[" << i << "][" << j << "][" << Year << "][" << Seas << "]: " << SpeSeasF[i][j][Year][Seas] << std::endl;
//std::cout << "Catch factors: " << SpeSeasF[i][j][Year][Seas] << ", " << InitN << ", " << std::exp(-SeasZ) << std::endl;
                        SeasZ = SpeSeasF[i][j][Year][Seas] + SpeSeasM1[i][j][Year][Seas] + SeasonalM2;
                        // Catch = (SpeSeasF(i, j, Year, Seas) * InitN) / (1 - Exp(-1 * SeasZ))
                        Catch = (SpeSeasF[i][j][Year][Seas] / SeasZ) * InitN * (1.0 - std::exp(-SeasZ));
                        Yield = Catch * AvgWeight;
                    } else {
                        // SeasZ = SeasonalF + SpeSeasM1(i, j, Year, Seas) + SeasonalM2
                        Catch = SpeSeasCatch[i][j][Year][Seas];
//std::cout << 222 << std::endl;
                        Yield = Catch * AvgWeight;
                    }
//std::cout << i << ", " << j << ", " << Catch << std::endl;

                    //
                    // Write outputs to project file.
                    //


                    cmd  = "INSERT INTO ForeOutput (MSVPAname, ForeName, Scenario, Year, Season, SpeName, Age, ";
                    cmd += "SpeType, InitAbund, EndAbund, InitBiomass, EndBiomass, SeasM2, SeasF, SeasM1, ";
                    cmd += "AvgSize, AvgWeight, InitWt, EndWt, TotalSBM, TotalBMConsumed, StomCont, SeasCatch, SeasYield) values ";
                    outputFields.clear();
                    updateDB = true;
                    outputFields.push_back(std::string("\"")+MSVPAName+"\"");
                    outputFields.push_back(std::string("\"")+ForecastName+"\"");
                    outputFields.push_back(std::string("\"")+Scenario.Name+"\"");
                    outputFields.push_back(std::to_string(Year));
                    outputFields.push_back(std::to_string(Seas));
                    outputFields.push_back(std::string("\"")+ SpeList(i)+"\"");
                    outputFields.push_back(std::to_string(j));
                    outputFields.push_back(std::to_string(SpeType(i)));
                    outputFields.push_back(std::to_string(InitN));
                    outputFields.push_back(std::to_string(EndN));
                    outputFields.push_back(std::to_string(InitBM));
                    outputFields.push_back(std::to_string(EndBM));
                    outputFields.push_back(std::to_string(SeasonalM2));
                    if (Scenario.FishAsF)
                        outputFields.push_back(std::to_string(SpeSeasF[i][j][Year][Seas]));
                    else
                        outputFields.push_back(std::to_string(SeasonalF));
                    outputFields.push_back(std::to_string(SpeSeasM1[i][j][Year][Seas]));
                    outputFields.push_back(std::to_string(AvgSize));
                    outputFields.push_back(std::to_string(AvgWeight));
                    outputFields.push_back(std::to_string(DailySpeWt[i][j][0]));
                    outputFields.push_back(std::to_string(DailySpeWt[i][j][SeasLen(Seas)]));
                    outputFields.push_back(std::to_string(TotSBM));
                    outputFields.push_back(std::to_string(TotalBMConsumed));
                    outputFields.push_back(std::to_string(StomCorrOut));
                    outputFields.push_back(std::to_string(Catch));
                    outputFields.push_back(std::to_string(Yield));
                    cmd = updateCmd(cmd,outputFields);
                    //errorMsg = "Output table generation - ForeOutput";
                    //buildWriteCmdAndRun(databasePtr,cmd,outputFields,errorMsg);
                    if (updateDB) {
                        // Remove last comma and space from string
                        cmd = cmd.substr(0, cmd.size() - 2);
                        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
                        if (nmfUtilsQt::isAnError(errorMsg)) {
                            nmfUtils::printError("Section: ForeOutput {1} ", errorMsg);
                        }
                    }
// RSK check this logic
//std::cout << "cmd1: " << cmd << std::endl;

                    cmd = "";
                    updateDB = false;
                    if (SpeType(i) == 0) {
                        cmd  = "INSERT INTO ForeSuitPreyBiomass (MSVPAname, ForeName, Scenario, Year, Season, PredName, PredAge, ";
                        cmd += "PredType, PreyName, PreyAge, PropDiet, SuitPreyBiomass, BMConsumed) values ";
                        for (int l = 0; l < NSpe; ++l) {
                            for (int nn = 0; nn <= NSpeAge(l); ++nn) {
                                if (AvgPropDiet(l,nn) > 0) {
                                    updateDB = true;
                                    outputFields.clear();
                                    outputFields.push_back(std::string("\"")+MSVPAName+"\"");
                                    outputFields.push_back(std::string("\"")+ForecastName+"\"");
                                    outputFields.push_back(std::string("\"")+Scenario.Name+"\"");
                                    outputFields.push_back(std::to_string(Year));
                                    outputFields.push_back(std::to_string(Seas));
                                    outputFields.push_back(std::string("\"")+ SpeList(i)+"\"");
                                    outputFields.push_back(std::to_string(j));
                                    outputFields.push_back(std::to_string(1));
                                    outputFields.push_back(std::string("\"")+ SpeList(l)+"\"");
                                    outputFields.push_back(std::to_string(nn));
                                    outputFields.push_back(std::to_string(AvgPropDiet(l,nn)));
                                    outputFields.push_back(std::to_string(TotSBM * AvgPropDiet(l,nn)));
                                    outputFields.push_back(std::to_string(TotalBMConsumed * AvgPropDiet(l,nn)));
                                    cmd = updateCmd(cmd,outputFields);
                                } // end if
                            } // end for nn
                        } // end for l

                        for (int l = 0; l < NOthPrey; ++l) {
                            if (AvgOthPropDiet(l) > 0) {
                                updateDB = true;
                                outputFields.clear();
                                outputFields.push_back(std::string("\"")+MSVPAName+"\"");
                                outputFields.push_back(std::string("\"")+ForecastName+"\"");
                                outputFields.push_back(std::string("\"")+Scenario.Name+"\"");
                                outputFields.push_back(std::to_string(Year));
                                outputFields.push_back(std::to_string(Seas));
                                outputFields.push_back(std::string("\"")+ SpeList(i)+"\"");
                                outputFields.push_back(std::to_string(j));
                                outputFields.push_back(std::to_string(1));
                                outputFields.push_back(std::string("\"")+ OtherPreyList(l)+"\"");
                                outputFields.push_back(std::to_string(0));
                                outputFields.push_back(std::to_string(AvgOthPropDiet(l)));
                                outputFields.push_back(std::to_string(TotSBM * AvgOthPropDiet(l)));
                                outputFields.push_back(std::to_string(TotalBMConsumed * AvgOthPropDiet(l)));
                                cmd = updateCmd(cmd,outputFields);
                            } // end if
                        } // end for l
                    } // end if

                    if (updateDB) {
                        // Remove last comma and space from string
                        cmd = cmd.substr(0, cmd.size() - 2);
//std::cout << "cmd2: " << cmd << std::endl;
                        errorMsg = databasePtr->nmfUpdateDatabase(cmd);
                        if (nmfUtilsQt::isAnError(errorMsg)) {
                            nmfUtils::printError("Section: ForeOutput [2] ", errorMsg);
                        }
                    }

                } // end for j <= NSpeAge(i)

                if (SpeType(i) == 0) {
                    ++PredCount;
                }
            } // end for i < NSpe

            std::string cmd1;
            std::string cmd2;
            std::string cmd3;
            if (NOthPred > 0) {

                for (int i = 0; i < NOthPred; ++i) {
                    for (int j = 0; j <= NOtherPredAge(i); ++j) {
                        cmd1  = "INSERT INTO ForeOutput (MSVPAname, ForeName, Scenario, Year, Season, SpeName, Age, ";
                        cmd1 += "SpeType, InitBiomass, TotalSBM, TotalBMConsumed, StomCont) values ";

                        TotalBMConsumed = 0.0;
                        TotSBM = 0.0;
                        nmfUtils::initialize(AvgPropDiet, NSpe, nmfConstants::MaxNumberAges);
                        nmfUtils::initialize(AvgOthPropDiet, NOthPrey);

                        StomCorrOut = AvgOthPredStomCorr(i,j) / SeasLen(Seas);
                        for (int k = 1; k <=  SeasLen(Seas); ++k) {
                            TotalBMConsumed += OthTotalBMConsumed[i][j][k];
                            TotSBM += TotOthPredSBM[i][j][k];
                            for (int l = 0; l < NSpe; ++l) {
                                for (int nn = 0; nn <= NSpeAge(l); ++nn) {
                                    AvgPropDiet(l,nn) += OthPdSpePropDiet[i][j][l][nn][k];
                                }
                            }
                            for (int l = 0; l < NOthPrey; ++l) {
                                AvgOthPropDiet(l) += OthPdOthPropDiet[i][j][l][k];
                            }
                        } // end for k

                        TotSBM /= SeasLen(Seas); // average total suitable prey biomass

                        for (int l = 0; l < NSpe; ++l) {
                            for (int nn = 0; nn <= NSpeAge(l); ++nn) {
                                AvgPropDiet(l,nn) /= SeasLen(Seas);
                            }
                        }

                        for (int l = 0; l < NOthPrey; ++l) {
                            AvgOthPropDiet(l) /= SeasLen(Seas);
                        }
                        outputFields.clear();
                        outputFields.push_back(std::string("\"")+MSVPAName+"\"");
                        outputFields.push_back(std::string("\"")+ForecastName+"\"");
                        outputFields.push_back(std::string("\"")+Scenario.Name+"\"");
                        outputFields.push_back(std::to_string(Year));
                        outputFields.push_back(std::to_string(Seas));
                        outputFields.push_back(std::string("\"")+ OtherPredList(i)+"\"");
                        outputFields.push_back(std::to_string(j));
                        outputFields.push_back(std::to_string(SpeType(i)));
                        outputFields.push_back(std::to_string(OthPredBiomassSize[i][j][Year][Seas]));
                        outputFields.push_back(std::to_string(TotSBM));
                        outputFields.push_back(std::to_string(TotalBMConsumed));
                        outputFields.push_back(std::to_string(StomCorrOut));
                        cmd1 = updateCmd(cmd1,outputFields);
                        // Remove last comma and space from string
                        cmd1 = cmd1.substr(0, cmd1.size() - 2);

                        errorMsg = databasePtr->nmfUpdateDatabase(cmd1);
                        if (nmfUtilsQt::isAnError(errorMsg)) {
                            nmfUtils::printError("Section: ForeOutput ", errorMsg);
                        }

                        updateDB = false;
                        cmd2  = "INSERT INTO ForeSuitPreyBiomass (MSVPAname, ForeName, Scenario, Year, Season, PredName, PredAge, ";
                        cmd2 += "PredType, PreyName, PreyAge, PropDiet, SuitPreyBiomass, BMConsumed) values ";
                        for (int l = 0; l < NSpe; ++l) {
                            for (int nn = 0; nn <= NSpeAge(l); ++nn) {
                                if (AvgPropDiet(l,nn) > 0) {
                                    updateDB = true;
                                    outputFields.clear();
                                    outputFields.push_back(std::string("\"")+MSVPAName+"\"");
                                    outputFields.push_back(std::string("\"")+ForecastName+"\"");
                                    outputFields.push_back(std::string("\"")+Scenario.Name+"\"");
                                    outputFields.push_back(std::to_string(Year));
                                    outputFields.push_back(std::to_string(Seas));
                                    outputFields.push_back(std::string("\"")+ OtherPredList(i)+"\"");
                                    outputFields.push_back(std::to_string(j));
                                    outputFields.push_back(std::to_string(0));
                                    outputFields.push_back(std::string("\"")+ SpeList(l)+"\"");
                                    outputFields.push_back(std::to_string(nn));
                                    outputFields.push_back(std::to_string(AvgPropDiet(l,nn)));
                                    outputFields.push_back(std::to_string(TotSBM * AvgPropDiet(l,nn)));
                                    outputFields.push_back(std::to_string(TotalBMConsumed * AvgPropDiet(l,nn)));
                                    cmd2 = updateCmd(cmd2,outputFields);
                                }
                            } // end for nn
                        } // end for l
                        if (updateDB) {
                            // Remove last comma and space from string
                            cmd2 = cmd2.substr(0, cmd2.size() - 2);
                            errorMsg = databasePtr->nmfUpdateDatabase(cmd2);
                            if (nmfUtilsQt::isAnError(errorMsg)) {
                                nmfUtils::printError("Section: ForeSuitPreyBiomass-1 ", errorMsg);
                            }
                        }

                        // RSK look at improving this and doing the writes after the loops!!!!!
                        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~RSK FIX
                                    /* Error

                        Forecast: Error - Check ForeSuitPreyBiomass table.
                        Forecast: Error - No data found for cmd: SELECT MSVPAName,ForeName,Scenario,PredName,PredAge,PreyName,
                              PreyAge,Year,Season,SuitPreyBiomass FROM ForeSuitPreyBiomass WHERE MSVPAname = 'MSVPA2'
                              AND ForeName = 'zero_men_F' AND Scenario = 'No_men_F'
                              AND PredName = 'Bluefish' AND PredAge = 0
                              AND PreyName = 'clupeids' AND Year = 0  AND Season = 2

                                     */
                        for (int l = 0; l < NOthPrey; ++l) {
                            // Write for other prey...will already have suitpreybiomass saved here
                            cmd3  = "REPLACE INTO ForeSuitPreyBiomass ";
                            cmd3 += "(MSVPAName,ForeName,Scenario,PredName,PredAge,PreyName,PreyAge,Year,";
                            cmd3 += "Season,PropDiet,BMConsumed) values ";

                            updateDB = false;
                            fields = {"MSVPAName","ForeName","Scenario","PredName","PredAge","PreyName","PreyAge","Year","Season","SuitPreyBiomass"};
                            queryStr = "SELECT MSVPAName,ForeName,Scenario,PredName,PredAge,PreyName,PreyAge,Year,Season,SuitPreyBiomass FROM ForeSuitPreyBiomass WHERE MSVPAname = '" + MSVPAName +
                                    "' AND ForeName = '" + ForecastName +
                                    "' AND Scenario = '" + Scenario.Name +
                                    "' AND PredName = '" + OtherPredList(i) +
                                    "' AND PredAge = " + std::to_string(j) +
                                    "  AND PreyName = '" + OtherPreyList(l) +
                                    "' AND Year = " + std::to_string(Year) +
                                    "  AND Season = " + std::to_string(Seas);
                            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ RSK FIX this.....
                            // RSK might not be an error if there are not db matches...
                            //if (nmfUtils::checkForError(logger,"ForeSuitPreyBiomass","MSVPAName",dataMap,queryStr))
                            //    return false;
                            if (dataMap["SuitPreyBiomass"].size() > 0) {

                                updateDB = true;
                                outputFields.clear();
                                outputFields.push_back(std::string("\"")+dataMap["MSVPAName"][0]+"\"");
                                outputFields.push_back(std::string("\"")+dataMap["ForeName"][0]+"\"");
                                outputFields.push_back(std::string("\"")+dataMap["Scenario"][0]+"\"");
                                outputFields.push_back(std::string("\"")+dataMap["PredName"][0]+"\"");
                                outputFields.push_back(dataMap["PredAge"][0]);
                                outputFields.push_back(std::string("\"")+dataMap["PreyName"][0]+"\"");
                                outputFields.push_back(dataMap["PreyAge"][0]);
                                outputFields.push_back(dataMap["Year"][0]);
                                outputFields.push_back(dataMap["Season"][0]);
                                outputFields.push_back(std::to_string(AvgOthPropDiet(l)));
                                outputFields.push_back(std::to_string(TotalBMConsumed * AvgOthPropDiet(l)));
                                cmd3 = updateCmd(cmd3,outputFields);
                            }
                            if (updateDB) {
                                // Remove last comma and space from string
                                cmd3 = cmd3.substr(0, cmd3.size() - 2);
                                errorMsg = databasePtr->nmfUpdateDatabase(cmd3);
                                if (nmfUtilsQt::isAnError(errorMsg)) {
                                    nmfUtils::printError("Section: ForeSuitPreyBiomass-2 ", errorMsg);
                                }
                            }

                        } // end for l
                    } // end for j
                } // end for i

            } // end if NOthPred

            // Reinitialize the day 0 for next season.....
            if (Seas < NSeasons - 1) {
                for (int i = 0; i < NSpe; ++i) {
                    for (int j = 0; j <= NSpeAge(i); ++j) {
                        InitSpeBiomass(i,j)   = DailySpeBiomass[i][j][SeasLen(Seas)];
                        InitSpeAbundance(i,j) = DailySpeAbund[i][j][SeasLen(Seas)];
                        InitSpeSize(i,j)      = DailySpeSize[i][j][SeasLen(Seas)];
                        InitSpeWeight(i,j)    = DailySpeWt[i][j][SeasLen(Seas)];
                    }
                }
            } else {
                for (int i = 0; i < NSpe; ++i) {
                    for (int j = 0; j < NSpeAge(i); ++j) {  // ok, should be < and not <=
                        InitSpeBiomass(i,j+1)   = DailySpeBiomass[i][j][SeasLen(Seas)];
                        InitSpeAbundance(i,j+1) = DailySpeAbund[i][j][SeasLen(Seas)];
                        InitSpeSize(i,j+1)      = DailySpeSize[i][j][SeasLen(Seas)];
                        InitSpeWeight(i,j+1)    = DailySpeWt[i][j][SeasLen(Seas)];
                    }
                }
            } // end if




            // RSK - Allow user to cancel this operation
        } // end for Season




        // Initialize age 0 weight and size again from the database
        for (int i = 0; i < NSpe; ++i) {
            InitSpeSize(i,0)   = SeasPreySize[i][0][0][0];
            InitSpeWeight(i,0) = SeasPreyWeight[i][0][0][0];
        }

        // Use stock recruit relationship to get initial abundance and biomass for age 0 for each MSVPA species


        // Calculate SSB and do SRR for each MSVPA species
        for (int i = 0; i < NSpe; ++i) {

            SSB = 0;
            for (int j = 0; j <= NSpeAge(i); ++j) {
                SSB += (DailySpeBiomass[i][j][0] * PropMature(i,j) * 0.5);
            }
            SSB /= 1000.0;
            // Now do the SRR depending on what relationship you are using

            switch (SRRType(i)) {

            case 0: // Ricker
                retv = Ricker(databasePtr, MSVPAName, ForecastName, SpeList(i), SSB, Recruits);
                if (! retv)
                    return false;
                break;

            case 1:  // Beverton Holt
                retv =  BevertonHolt(databasePtr, MSVPAName, ForecastName, SpeList(i), SSB, Recruits);
                if (! retv)
                    return false;
                break;

            case 2:  // Quartile
                retv =  SRQuartile(databasePtr, MSVPAName, ForecastName, SpeList(i), SSB, Recruits);
                if (! retv)
                    return false;
                break;

            case 3:  // Shepherd
                retv =  Shepherd(databasePtr, MSVPAName, ForecastName, SpeList(i), SSB, Recruits);
                if (! retv)
                    return false;
                break;

            default:
                logger->logMsg(nmfConstants::Normal,"Error: Forecast: Unknown SRRType of " + std::to_string(SRRType(i)));
                break;
            } // end switch

            if (AbsRecruits(i) == 0.0) {
                Recruits =  (AbsRecruits(i) == 0.0) ?
                            Recruits * SpeVarRec(i,Year) * 1000 :
                            SpeVarRec(i,Year); // use absolute values entered by user..units should be fine here.
            }

            // Put the recruits in the initabundance matrices
            InitSpeAbundance(i,0) = Recruits;
            InitSpeBiomass(i,0) = InitSpeWeight(i,0) * Recruits;
        } // end for i

        // Store a static initial abundance for use in the catch scaling
        for (int i = 0; i < NSpe; ++i) {
            for (int j = 0; j <= NSpeAge(i); ++j) {
                AnnInitAbundance(i,j) = InitSpeAbundance(i,j);
            }
        }

        // Reset mortality values for the next iteration to use when scaling catch
        for (int i = 0; i < NSpe; ++i) {
            fields = {"Year","SpeName","Age","AnnM1","AnnM2","AnnF"};
            queryStr = "SELECT Year, SpeName, Age, SUM(SeasM1) as AnnM1, SUM(SeasM2) as AnnM2, SUM(SeasF) As AnnF FROM ForeOutput WHERE MSVPAname = '" + MSVPAName + "'" +
                    " AND ForeName = '" + ForecastName + "'" +
                    " AND Scenario = '" + Scenario.Name + "'" +
                    " AND SpeName = '" + SpeList(i) + "'" +
                    " AND Year = " + std::to_string(Year) + " GROUP BY Year,SpeName,Age";
            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
            if (nmfUtils::checkForError(logger,"ForeOutput","SpeName",dataMap,queryStr))
                return false;
            for (int j = 0; j <= NSpeAge(i); ++j) {
                tempAnnF = std::stod(dataMap["AnnF"][j]);
                if (! (dataMap["AnnM2"][j]).empty()) {
                    InitSpeF(i,j) = tempAnnF;
                    InitSpeZ(i,j) = tempAnnF + std::stod(dataMap["AnnM1"][j]) + std::stod(dataMap["AnnM2"][j]);
                } else {
                    InitSpeF(i, j) = tempAnnF;
                    InitSpeZ(i, j) = tempAnnF + std::stod(dataMap["AnnM1"][j]);
                }
            } // end for j
        } // end for i

        // Finished with the Year and initialized for the next one.


        // Not in original code....
        // Find Total Init Biomass and use that in the Progress Chart for user to see as the Forecast calculates.
        SpeciesName.clear();
        TotalInitBiomass.clear();
        for (int i = 0; i < NSpe; ++i) {
            fields = {"Year","TotalInitBiomass"};
            queryStr = "SELECT SpeName, SUM(InitBiomass) as TotalInitBiomass FROM ForeOutput WHERE MSVPAName = '" + MSVPAName + "'" +
                    " AND ForeName = '" + ForecastName + "'" +
                    " AND Scenario = '" + Scenario.Name + "'" +
                    " AND SpeName = '" + SpeList(i) + "'" +
                    " AND Year = " + std::to_string(Year);
            dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
            SpeciesName.push_back(SpeList(i));
            if (dataMap["Year"].size() > 0) {
                TotalInitBiomass.push_back(dataMap["TotalInitBiomass"][0]);
            } else {
                TotalInitBiomass.push_back("-999");
                logger->logMsg(nmfConstants::Error,
                                  "\nError: Missing Total Init Biomass data for: " + MSVPAName + ", " +
                                  ForecastName + ", " + Scenario.Name + ", " + SpeList(i));
            }
        } // end for i

        WriteCurrentLoopFile(databasePtr,
                             MSVPAName,
                             ForecastName,
                             Scenario.Name,
                             Year,
                             SpeciesName,
                             TotalInitBiomass);

         FinalYear = Year;
    } // end for Year
    outputProgressLabel("Done");

    updateProgress(-1,"Forecast Completed.");

    logger->logMsg(nmfConstants::Normal,"Forecast: Processing End");

    if (FishWarn) {
        logger->logMsg(nmfConstants::Normal,"Warning: Invalid fishery mortality rates encountered during forecast run.");
    }

    // End timer for algorithmic timing
    std::string elapsedTimeStr = nmfUtils::elapsedTime(startTime);

    if (userHalted) {
        logger->logMsg(nmfConstants::Normal,"Forecast: User halted Forecast Run. Output data incomplete.");
    } else {
        logger->logMsg(nmfConstants::Normal,"Forecast: Everything is all done and written out!");
        WriteRunStopFile("StopAllOk",
                         MSVPAName,
                         elapsedTimeStr,
                         FinalYear);
    }


    return true;
} // end ModelNoPredatorGrowth



void
nmfForecast::WriteRunStopFile(std::string value,
                           std::string theMSVPAName,
                           std::string elapsedTime,
                           int numYears)
{
    std::ofstream outputFile(nmfConstantsMSVPA::ForecastStopRunFile);
    outputFile << value << std::endl;
    outputFile << theMSVPAName << std::endl;
    outputFile << elapsedTime << std::endl;
    outputFile << numYears << std::endl;
    outputFile.close();

} // end WriteRunStopFile


std::string
nmfForecast::ReadRunStopFile()
{
    std::string retv = "";
    std::string str;

    std::ifstream inputFile(nmfConstantsMSVPA::ForecastStopRunFile);

    if (inputFile) {
        // Read line of data from file
        std::getline(inputFile,str);
        inputFile.close();
        retv = str;
    }

    return retv;

} // end ReadRunStopFile


void
nmfForecast::WriteCurrentLoopFile(nmfDatabase *databasePtr,
                                  std::string &MSVPAName,
                                  std::string &ForeName,
                                  std::string &ScenarioName,
                                  int &Year,
                                  std::vector<std::string> &SpeciesName,
                                  std::vector<std::string> &TotalInitBiomass)
{
    bool foundSpecies=false;
    std::string SpeciesToUse="";
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    std::ofstream outputFile(nmfConstantsMSVPA::ForecastProgressChartFile,
                             std::ios::out|std::ios::app);

    // Find which species is the Prey Only MSVPA Species and use that species
    fields = {"MSVPAName","SpeName","Type"};
    queryStr = "SELECT MSVPAName,SpeName,Type FROM MSVPAspecies WHERE MSVPAName = '" + MSVPAName + "'" +
               " AND Type = 1";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["SpeName"].size() > 0) {
        SpeciesToUse = dataMap["SpeName"][0];
    }

    for (unsigned int i=0; i<SpeciesName.size(); ++i) {
        if (SpeciesName[i] == SpeciesToUse) {
            outputFile << MSVPAName << ", "
                       << ForeName << ", "
                       << ScenarioName << ", "
                       << Year << ", "
                       << SpeciesName[0] << ", "
                       << std::to_string(std::stod(TotalInitBiomass[0])/1000.0+0.5) << "\n";
            foundSpecies = true;
        }
        if (foundSpecies)
            break;
    }
    outputFile.close();

    if (! foundSpecies) {
        logger->logMsg(nmfConstants::Error,"\nError: Couldn't find Prey Only MSVPA Species in list (i.e., with Type = 1 from table MSVPAspecies).");
    }

    emit UpdateForecastProgressWidget();
    QApplication::processEvents();

} // end WriteCurrentLoopFile


void
nmfForecast::outputProgressLabel(std::string msg)
{
    std::ofstream outputFile(nmfConstantsMSVPA::ForecastProgressChartLabelFile);
    outputFile << msg << std::endl;
    outputFile.close();

} // end outputProgressLabel


void
nmfForecast::updateProgress(int value, std::string msg)
{
    emit UpdateForecastProgressDialog(value,QString::fromStdString(msg));

    // Necessary so progress GUI will refresh
    QApplication::processEvents();

    std::ofstream outputFile(nmfConstantsMSVPA::ForecastProgressBarFile);
    outputFile << value << "; " << msg << std::endl;
    outputFile.close();

    logger->logMsg(nmfConstants::Normal,"Forecast: "+msg);
}


std::string
nmfForecast::updateCmd(const std::string &currentCmd,
    const std::vector<std::string> &outputFields)
{
    std::string cmd = currentCmd;

    cmd += "(";
    for (auto item : outputFields) {
        cmd += item + ",";
    }
    // Remove last ","
    cmd = cmd.substr(0, cmd.size()-1);
    cmd += "), ";

    return cmd;
}


void
nmfForecast::buildWriteCmdAndRun(nmfDatabase *databasePtr,
     const std::string &cmdPrefix,
     const std::vector<std::string> &outputFields,
     const std::string &msg)
{
    std::string errorMsg;
    std::string cmd = cmdPrefix;

    cmd += "(";
    for (auto item : outputFields) {
        cmd += item + ",";
    }
    // Remove last ","
    cmd = cmd.substr(0, cmd.size()-1);
    cmd += ") ";
    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
    if (nmfUtilsQt::isAnError(errorMsg)) {
        nmfUtils::printError("Section: "+msg, errorMsg);
    }
} // end buildWriteCmdAndRun


bool
nmfForecast::GetSize(nmfDatabase* databasePtr, int NYears,
        int Year, int MaxAge,
        std::string SpeName, int SizeIndex,
        boost::numeric::ublas::vector<double> &Size)
{
    std::string YearStr  = std::to_string(Year);
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    double VbLinf = 0.0;
    double VbTzero = 0.0;
    double VBK = 0.0;

    std::vector<std::string> Variables = {"vbLinf","vbStzero","vbSk"};
    std::vector<double * > Vectors = {&VbLinf,&VbTzero,&VBK};

//std::cout  <<  "SizeIndex: " << SizeIndex << std::endl;
    switch (SizeIndex) {

        case 0: // fixed VB..calculate size at age for each year from same VB equations
            fields = {"Value"};
            for (unsigned int i=0; i<Variables.size(); ++i) {
                queryStr = "SELECT Value FROM SpeSize WHERE SpeName='" + SpeName +
                           "' AND Variable = '" + Variables[i] + "'";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                if (nmfUtils::checkForError(logger,"SpeSize","Value",dataMap,queryStr))
                    return false;
                (*Vectors[i]) = std::stod(dataMap[fields[0]][0]);
            }

            for (int j=0; j<=MaxAge; ++j) {
                Size(j) = VbLinf * (1 - std::exp(-VBK * (j - VbTzero)));
            }
            break;

        case 1: // ann VB

            fields = {"Value"};
            for (unsigned int i=0; i<Variables.size(); ++i) {
                queryStr = "SELECT Value FROM SpeSize WHERE SpeName='" + SpeName + "' " +
                           " AND Year = " + YearStr +
                           " AND Variable = '" + Variables[i] + "' ";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                if (nmfUtils::checkForError(logger,"SpeSize","Value",dataMap,queryStr))
                    return false;
                for (int j = 0; j<NYears; ++j) {
                    (*Vectors[i]) = std::stod(dataMap[fields[0]][j]);
                }
            }

            for (int j=0; j<=MaxAge; ++j) {
                Size(j) = VbLinf * (1 - std::exp(-VBK * (j - VbTzero)));
            }
            break;

        default: // fixed size or annual ...everything is already saved by year and age
            fields = { "Value"};
            queryStr = "SELECT Value FROM SpeSize WHERE SpeName='" + SpeName + "' " +
                       " AND Year = " + YearStr +
                       " AND Variable='Size' ORDER BY Age";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            if (nmfUtils::checkForError(logger,"SpeSize","Value",dataMap,queryStr))
                return false;
            for (int j=0; j<=MaxAge; ++j) {
                Size(j) = std::stod(dataMap[fields[0]][j]);
            }
            break;

    } // end switch

    return true;
} // end GetSize



bool
nmfForecast::GetWt(nmfDatabase* databasePtr, int NYears,
        int Year, int MaxAge, std::string SpeName, int WtIndex,
        boost::numeric::ublas::vector<double> &Size,
        boost::numeric::ublas::vector<double> &Wt)
{
    std::string YearStr = std::to_string(Year);
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;

    double VbLinf = 0.0;
    double VbTzero = 0.0;
    double VBK = 0.0;
    double WLalpha = 0.0;
    double WLbeta = 0.0;

//std::cout << "WtIndex: " << WtIndex << std::endl;

    std::vector<std::string> Variables = {"VbLinf","vbWTzero","vbWK"};
    std::vector<double * > Vectors = {&VbLinf,&VbTzero,&VBK};
    std::vector<std::string> FixWlVariables = {"WLAlpha","WLBeta"};
    std::vector<double * > FixWlVectors = {&WLalpha, &WLbeta};

    switch (WtIndex) {

        case 0: // fixed VB
            fields = {"Value"};
            for (unsigned int i=0; i<Variables.size(); ++i) {
                queryStr = "SELECT Value FROM SpeWeight WHERE SpeName='" + SpeName +
                           "' AND Variable = '" + Variables[i] + "'";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                if (nmfUtils::checkForError(logger,"SpeWeight","Value",dataMap,queryStr))
                    return false;
                (*Vectors[i]) = std::stod(dataMap[fields[0]][0]);
            }

            for (int j = 0; j<=MaxAge; ++j) {
                Wt(j) = VbLinf * std::pow((1.0-std::exp(-VBK*(j-VbTzero))),3.0);
            }
            break;

        case 1: // ann VB
            fields = {"Value"};
            for (unsigned int i=0; i<Variables.size(); ++i) {
                queryStr = "SELECT Value FROM SpeWeight WHERE SpeName='" + SpeName + "' " +
                           " AND Year = " + YearStr +
                           " AND Variable = '" + Variables[i] + "' ";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                if (nmfUtils::checkForError(logger,"SpeWeight","Value",dataMap,queryStr))
                    return false;
                for (int j = 0; j<NYears; ++j) {
                    (*Vectors[i]) = std::stod(dataMap[fields[0]][j]);
                }
            }

            for (int j = 0; j<=MaxAge; ++j) {
                Wt(j) = VbLinf * std::pow(1.0-std::exp(-VBK*(j-VbTzero)),3.0);
            }
            break;

        case 2: // fix WL
            fields = {"Value"};
            for (unsigned int i=0; i<Variables.size(); ++i) {
                queryStr = "SELECT Value FROM SpeWeight WHERE SpeName='" + SpeName + "' " +
                           " AND Variable = '" + FixWlVariables[i] + "'";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                if (nmfUtils::checkForError(logger,"SpeWeight","Value",dataMap,queryStr))
                    return false;
                (*FixWlVectors[i]) = std::stod(dataMap[fields[0]][0]);
            }

            for (int j = 0; j<=MaxAge; ++j) {
                Wt(j) = std::exp(WLalpha) * std::pow(Size(j),WLbeta);
            }
            break;

        case 3: // ann WL
            fields = {"Value"};
            for (unsigned int i=0; i<Variables.size(); ++i) {
                queryStr = "SELECT Value FROM SpeWeight WHERE SpeName = '" + SpeName + "' " +
                        " AND Year = " + YearStr +
                        " AND Variable = '" + FixWlVariables[i] +"' ";
                dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
                if (nmfUtils::checkForError(logger,"SpeWeight","Value",dataMap,queryStr))
                    return false;
                (*FixWlVectors[i]) = std::stod(dataMap[fields[0]][0]);
            }

            for (int j = 0; j<=MaxAge; ++j) {
                Wt(j) = std::exp(WLalpha) * std::pow(Size(j),WLbeta);
            }
            break;

        case 4: // wt at age
        case 5:
            fields = {"Value"};
            queryStr = "SELECT Value FROM SpeWeight WHERE SpeName = '" + SpeName + "' " +
                       " AND Year = " + YearStr +
                       " AND Variable = 'Weight' ORDER BY Age";
            dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
            if (nmfUtils::checkForError(logger,"SpeWeight","Value",dataMap,queryStr))
                return false;
            for (int j = 0; j<=MaxAge; ++j) {
                Wt(j) = std::stod(dataMap["Value"][j]);
            }
            break;
    }

    return true;
} // end GetWt




bool
nmfForecast::Ricker(nmfDatabase* databasePtr,
                    std::string &MSVPAName, std::string &ForecastName,
                    std::string &SpeName, double &SSB, double &returnVal)
{
   double Alpha = 0.0;
   double Beta  = 0.0;
   std::string queryStr;
   std::vector<std::string> fields;
   std::map<std::string, std::vector<std::string> > dataMap;

   fields = { "SRRA","SRRB"};
   queryStr = "SELECT SRRA,SRRB FROM ForeSRR WHERE MSVPAName='" + MSVPAName + "' " +
              " AND ForeName = '" + ForecastName + "' " +
              " AND SpeName = '" + SpeName + "' ";
   dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
   if (nmfUtils::checkForError(logger,"ForeSRR","SRRA",dataMap,queryStr))
       return false;

   Alpha = std::stod(dataMap["SRRA"][0]);
   Beta  = std::stod(dataMap["SRRB"][0]);

   returnVal = std::exp(Alpha + Beta * SSB) * SSB;
   return true;
} // end Ricker


bool
nmfForecast::BevertonHolt(nmfDatabase* databasePtr,
                    std::string &MSVPAName, std::string &ForecastName,
                    std::string &SpeName, double &SSB, double &returnVal)
{
   double Alpha = 0.0;
   double Beta  = 0.0;
   std::string queryStr;
   std::vector<std::string> fields;
   std::map<std::string, std::vector<std::string> > dataMap;

   fields = { "SRRA","SRRB"};
   queryStr = "SELECT SRRA,SRRB FROM ForeSRR WHERE MSVPAName='" + MSVPAName + "' " +
              " AND ForeName = '" + ForecastName + "' " +
              " AND SpeName = '" + SpeName + "' ";
   dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
   if (nmfUtils::checkForError(logger,"ForeSRR","SRRA",dataMap,queryStr))
       return false;

   Alpha = std::stod(dataMap["SSRA"][0]);
   Beta  = std::stod(dataMap["SSRB"][0]);

   returnVal = SSB / (Alpha + Beta * SSB);

   return true;

} // end BevertonHolt


bool
nmfForecast::Shepherd(nmfDatabase* databasePtr,
                    std::string &MSVPAName, std::string &ForecastName,
                    std::string &SpeName, double &SSB,
                    double &returnVal)
{
   double Alpha = 0.0;
   double Beta  = 0.0;
   double k     = 0.0;
   std::string queryStr;
   std::vector<std::string> fields;
   std::map<std::string, std::vector<std::string> > dataMap;

   fields = { "SRRA","SRRB","SRRK"};
   queryStr = "SELECT SRRA,SRRB,SRRK FROM ForeSRR WHERE MSVPAName='" + MSVPAName + "' " +
              " AND ForeName = '" + ForecastName + "' " +
              " AND SpeName = '" + SpeName + "' ";
   dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
   if (nmfUtils::checkForError(logger,"ForeSRR","SRRA",dataMap,queryStr))
       return false;

   Alpha = std::stod(dataMap["SSRA"][0]);
   Beta  = std::stod(dataMap["SSRB"][0]);
   k     = std::stod(dataMap["SSRK"][0]);

   returnVal = (Alpha*SSB)/(1.0 + std::pow((SSB/k),Beta));

   return true;

} // end Shepherd


bool
nmfForecast::SRQuartile(nmfDatabase* databasePtr,
                    std::string &MSVPAName, std::string &ForecastName,
                    std::string &SpeName, double &SSB, double &returnVal)
{
   int QuartNum=0;
   double QuartMin[4];
   double QuartMax[4];
   double QuartMinRec[4];
   double QuartMaxRec[4];

   std::string queryStr;
   std::vector<std::string> fields;
   std::map<std::string, std::vector<std::string> > dataMap;

   fields = { "MinSSB","MaxSSB","MinRec","MaxRec"};
   queryStr = "SELECT MinSSB,MaxSSB,MinRec,MaxRec FROM ForeSRQ WHERE MSVPAName='" + MSVPAName + "' " +
              " AND ForeName = '" + ForecastName + "' " +
              " AND SpeName = '" + SpeName + "' ";
   dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
   if (nmfUtils::checkForError(logger,"ForeSRQ","MinSSB",dataMap,queryStr))
       return false;

   for (int i=0; i<4; ++i) {
       QuartMin[i]    = std::stod(dataMap["MinSSB"][i]);
       QuartMax[i]    = std::stod(dataMap["MaxSSB"][i]);
       QuartMinRec[i] = std::stod(dataMap["MinRec"][i]);
       QuartMaxRec[i] = std::stod(dataMap["MaxRec"][i]);
   }

   for (int i=0; i<4; ++i) {
       if ((SSB > QuartMin[i]) && (SSB <= QuartMax[i])) {
           QuartNum = i;
       }
   }

   returnVal = QuartMinRec[QuartNum] + nmfUtils::getRandomNumber(-1,0.0,1.0)*(QuartMaxRec[QuartNum]-QuartMinRec[QuartNum]);

   return true;

} // end SRQuartile

