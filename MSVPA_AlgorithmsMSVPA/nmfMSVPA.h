/**
 @file nmfMSVPA.h
 @author rklasky
 @copyright 2017 NOAA - National Marine Fisheries Service
 @brief This file contains the MSVPA API class definitions.
 @date Jan 18, 2017
*/

#ifndef NMF_MSVPA_H_
#define NMF_MSVPA_H_

#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>

#include <QObject>
#include <QApplication>
#include <QTimer>

#include "nmfLogger.h"
#include "nmfDatabase.h"
#include "nmfConstantsMSVPA.h"
#include "nmfUtilsQt.h"

//#include "nmfmsvpa_global.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/multi_array.hpp>

typedef boost::multi_array<double, 3> Boost3DArrayDouble;
typedef boost::multi_array<double, 4> Boost4DArrayDouble;
typedef Boost3DArrayDouble::index index3D;
typedef Boost4DArrayDouble::index index4D;

class nmfMSVPAOthPredInfo {

public:
     std::string SpeName;
     int SpeTag;
     int FirstYear;
     int LastYear;
     double BMunits;
     int MinSize;
     int MaxSize;
     int SizeUnitIndex;
     int SizeUnits;
     double SizeAlpha;
     double SizeBeta;
     double LWalpha;
     double LWbeta;
     int WtUnitIndex;
     double WtUnits;
     int SizeStructure;
     int NumSizeCats;
     std::vector<int> SizeCatMins;
     std::vector<int> SizeCatMaxs;
     std::vector<double> SizeCatBM;

     nmfMSVPAOthPredInfo(std::string SpeNameIn, int SpeTagIn);
};

class nmfMSVPAOthPreyInfo {

public:
    std::string SpeName;
    int FixBM;
    int SeasBM;
    double MinSize;
    double MaxSize;
    double SizeAlpha;
    double SizeBeta;
    double BMValue;
    int FirstYear;
    int LastYear;
    int NumSeas;


    nmfMSVPAOthPreyInfo(
            std::string SpeNameIn, double MinSizeIn, double MaxSizeIn,
            double SizeAlphaIn, double SizeBetaIn);
};

class nmfMSVPASpeciesInfo {

public:

     std::string SpeName;
     int SpeType;
     int DBSpeIndex;
     std::string SVPAName;
     int SVPAIndex;
     int SVPAType;
     int Nage;
     int PlusAge;
     int FirstCatAge;
     int LastCatAge;
     int MaxAge;
     float CatUnits;
     float WtUnits;
     float SizeUnits;

    nmfMSVPASpeciesInfo(
            std::string SpeNameIn, int SpeTypeIn, int DBSpeIndexIn,
            std::string SVPANameIn, int SVPAIndexIn);

};

struct MSVPANoPredatorGrowthStruct {
    nmfDatabase* databasePtr;
    std::string  Username;
    std::string  Hostname;
    std::string  Password;
    std::string  MSVPAName;
    std::map<std::string,int> CohortAnalysisGuiData;
    std::map<std::string,int> EffortTunedGuiData;
    std::map<std::string,std::string> XSAGuiData;
};

class nmfMSVPA : public QObject {

    Q_OBJECT

private:
    nmfLogger *logger;
    std::string ProjectName;
    std::string ProjectDir;
    std::string ProjectDatabase;

    void ReadSettings();

    void printError(const std::string query, const std::string errorMsg);

    bool GetSeasonalLengthTemperatureData(
            nmfDatabase* databasePtr,
            std::string MSVPAName,
            int NSeas, int NYears,
            boost::numeric::ublas::matrix<double> &SeasLen,
            boost::numeric::ublas::matrix<double> &SeasTemp);

    bool GetPredatorData(
            nmfDatabase* databasePtr,
            std::string MSVPAName,
            boost::numeric::ublas::vector<std::string> &MSVPAPredList);

    bool GetSpeciesData(
            nmfDatabase* databasePtr,
            std::string MSVPAName,
            int NMSVPASpe,
            std::vector<std::unique_ptr<nmfMSVPASpeciesInfo>> &MSVPASpeciesList);

    bool GetSize(nmfDatabase* databasePtr, int FirstYear, int LastYear,
            int FirstCatAge, int LastCatAge,
            std::string SpeName, int SizeIndex,
            boost::numeric::ublas::matrix<double> &tmpSize,
            Boost3DArrayDouble &tmpSeasSize,
            boost::numeric::ublas::matrix<double> &SeasLen, int NYears, int NSeas);

    void GetWt(nmfDatabase* databasePtr, int FirstYear, int LastYear,
            int FirstCatAge, int LastCatAge,
            std::string SpeName, int WtIndex,
            boost::numeric::ublas::matrix<double> &Size,
            boost::numeric::ublas::matrix<double> &Wt,
            Boost3DArrayDouble &SeasSize,
            Boost3DArrayDouble &SeasWt,
            boost::numeric::ublas::matrix<double> &SeasLen,
            int &NYears, int &NSeas);

    void WriteRunStopFile(std::string value,
                                 std::string theMSVPAName,
                                 std::string elapsedTime,
                                 int numLoops,
                                 int convergenceDiff);
    void WriteCurrentLoopFile(std::string MSVPAName,
                                     int NumLoops,
                                     int ConvergenceValue);

    std::string ReadRunStopFile();

    void outputProgressLabel(std::string msg);

    void updateProgress(int value, std::string msg);


    /**
     * @brief Runs an MSVPA with a Growth model (not currently implemented).
     * @param none
     * @return none
     */
    //
    // When implemented move into: public
    //
    bool MSVPA_PredatorGrowth(nmfDatabase* databasePtr, std::string MSVPAName);

    nmfDatabase* openDatabase(std::string Username,
                              std::string Hostname,
                              std::string Password);

public:

    /**
     * @brief Class constructor
     */
    nmfMSVPA(nmfLogger *logger);

    //void clearProgressBar();

    /**
     * @brief Runs an MSVPA without a Growth model.
     * @param none
     * @return none
     */
    bool MSVPA_NoPredatorGrowth(
             MSVPANoPredatorGrowthStruct &argStruct);

    bool dummyFunc();

signals:
    void UpdateMSVPAProgressDialog(int val, QString msg);
    void UpdateMSVPAProgressWidget();

};


#endif /* NMF_MSVPA_H_ */
