/**
 *
 * @file nmfForecast.h
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
 * @brief This file contains the Forecast API definition.
 */

#ifndef NMFFORECAST_H
#define NMFFORECAST_H

#include <iostream>
#include <string>
#include <cmath>
#include <random>
#include <algorithm>

#include <QApplication>

#include "nmfDatabase.h"
#include "nmfLogger.h"

//#include "nmfforecast_global.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/multi_array.hpp>

typedef boost::multi_array<double, 3> Boost3DArrayDouble;
typedef boost::multi_array<double, 4> Boost4DArrayDouble;
typedef Boost3DArrayDouble::index index3D;
typedef Boost4DArrayDouble::index index4D;

struct ForecastNoPredatorGrowthStruct {
    nmfDatabase* databasePtr;
    std::string MSVPAName;
    std::string ForecastName;
    std::string ScenarioName;
    int InitYear;
    int NYears;
};

class nmfForecast : public QObject {

    Q_OBJECT

private:
    nmfDatabase* databasePtr;
    nmfLogger*   logger;

    bool GetAverageTotalSuitableBiomass(nmfDatabase* databasePtr,
            const std::string &MSVPAName,
            const std::string &Species,
            const int &UpperLimJ,
            const int &UpperLimK,
            const int &UpperLimL,
            const int &index1,
            Boost3DArrayDouble &Array3D);

    bool GetSize(nmfDatabase* databasePtr, int NYears,
         int Year, int MaxAge,
         std::string SpeName, int SizeIndex,
         boost::numeric::ublas::vector<double> &Size);

    bool GetSeasonSpatialOverlap(nmfDatabase* databasePtr,
             const std::string &MSVPAName,
             const std::string &Species,
             const boost::numeric::ublas::vector<std::string> &SpeList,
             const int &UpperLimK,
             const int &UpperLimJ,
             const int &UpperLimL,
             const int &index2,
             Boost4DArrayDouble &Array4D);

    bool GetTypePreferences(nmfDatabase* databasePtr,
            const std::string &MSVPAName,
            const std::string &Species,
            const boost::numeric::ublas::vector<std::string> &SpeList,
            const int &UpperLimK,
            const int &UpperLimJ,
            const int &index1,
            Boost3DArrayDouble &Array3D);

    bool GetWt(nmfDatabase* databasePtr, int NYears,
            int Year, int MaxAge, std::string SpeName, int WtIndex,
            boost::numeric::ublas::vector<double> &Size,
            boost::numeric::ublas::vector<double> &Wt);

    bool Ricker(nmfDatabase* databasePtr,
        std::string &MSVPAName, std::string &ForecastName,
        std::string &SpeName, double &SSB,
        double &returnVal);

    bool BevertonHolt(nmfDatabase* databasePtr,
        std::string &MSVPAName, std::string &ForecastName,
        std::string &SpeName, double &SSB,
        double &returnVal);

    bool Shepherd(nmfDatabase* databasePtr,
        std::string &MSVPAName, std::string &ForecastName,
        std::string &SpeName, double &SSB,
        double &returnVal);

    bool SRQuartile(nmfDatabase* databasePtr,
        std::string &MSVPAName, std::string &ForecastName,
        std::string &SpeName, double &SSB,
        double &returnVal);

    void buildWriteCmdAndRun(nmfDatabase *databasePtr,
             const std::string &cmdPrefix,
             const std::vector<std::string> &outputFields,
             const std::string &msg);

    void outputProgressLabel(std::string msg);

    std::string ReadRunStopFile();

    std::string updateCmd(const std::string &currentCmd,
             const std::vector<std::string> &outputFields);

    void updateProgress(int value, std::string msg);

    void WriteRunStopFile(std::string value,
                               std::string theMSVPAName,
                               std::string elapsedTime,
                               int numYears);

    void WriteCurrentLoopFile(nmfDatabase *databasePtr,
                                     std::string &MSVPAName,
                                     std::string &ForeName,
                                     std::string &ScenarioName,
                                     int &Year,
                                     std::vector<std::string> &SpeciesName,
                                     std::vector<std::string> &TotalInitBiomass);

public:

    nmfForecast(nmfDatabase* databasePtr,
                nmfLogger *logger);
   ~nmfForecast();

    bool Forecast_NoPredatorGrowth(
            const ForecastNoPredatorGrowthStruct &argStruct);

signals:
    void UpdateForecastProgressDialog(int val, QString msg);
    void UpdateForecastProgressWidget();

};

#endif // NMFFORECAST_H
