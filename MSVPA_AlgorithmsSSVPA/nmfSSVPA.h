/**
 *
 * @file nmfSSVPA.h
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
 * @brief This file contains the SVPA API definitions.
 */


#ifndef NMFSSVPA_H_
#define NMFSSVPA_H_

#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
#include <map>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/multi_array.hpp>

//#include "nmfssvpa_global.h"

typedef boost::multi_array<double, 3> Boost3DArrayDouble;
typedef boost::multi_array<double, 4> Boost4DArrayDouble;
typedef Boost3DArrayDouble::index index3D;
typedef Boost4DArrayDouble::index index4D;
typedef boost::numeric::ublas::matrix<double> MatrixDouble;
typedef boost::numeric::ublas::vector<double> VectorDouble;
typedef std::map<std::string,int> MapInt;
typedef std::map<std::string,std::string> MapString;


struct FullSVPAInputArgs {
    VectorDouble InitialSelectivity;
    int MaxAge;
    int FirstCatchAge;
    int NumCatchAges;
    int isPlusClass;
    double FullF;
    int ReferenceAge;
    int NumCatchYears;
};

struct FullSVPAOutputArgs {
    MatrixDouble CatchMatrix;
    double CV_CoeffVariation;
    double SE_StandardError;
    int NumIterations;
    MatrixDouble Abundance;
    MatrixDouble M_NaturalMortality;
    MatrixDouble F_FishingMortality;
};

struct CandSVPAInputArgs {
    nmfDatabase* databasePtr;
    MapInt CohortAnalysisGuiData;
    int FirstYear;
    int LastYear;
    int FirstCatchAge;
    int LastCatchAge;
    int Plus;
    int MaxAge;
    int SpeIndex;
    std::string SSVPAName;
    MatrixDouble M_NaturalMortality;
};

struct CandSVPAOutputArgs {
    double CV_CoeffVariation;
    double SE_StandardError;
    int NumIterations;
    MatrixDouble Abundance;
    MatrixDouble F_FishingMortality;
};

struct EffortTuneInputArgs {
    nmfDatabase *databasePtr;
    MapInt EffortTunedGuiData;
    Boost4DArrayDouble FleetData;
    int FirstYear;
    int LastYear;
    int FirstCatchAge;
    int LastCatchAge;
    int Plus;
    int MaxAge;
    int SpeIndex;
    std::string SSVPAName;
};

struct EffortTuneOutputArgs {
    MatrixDouble Abundance;
    MatrixDouble M_NaturalMortality;
    MatrixDouble F_FishingMortality;
};

struct XSAInputArgs {
    nmfDatabase* databasePtr;
    MapString    XSAGuiData;
    int FirstYear;
    int LastYear;
    int SpeIndex;
    std::string SSVPAName;
    MatrixDouble M_NaturalMortality;
};

struct XSAOutputArgs {
    MatrixDouble Abundance;
    MatrixDouble F_FishingMortality;
    int XSAWarn;
};

class nmfSSVPA {

private:

    double SolveIt(
            const double &term,
            const double &M);


    void SolveCohortTermZ(
            const double &Catch,
            const double &M,
            const double &Nend,
            double &F);

    void UpdateNandF(
            const int &Nage,
            const int &Nyear,
            const int &Count,
            boost::numeric::ublas::matrix<double> &N,
            boost::numeric::ublas::matrix<double> &F,
            const boost::numeric::ublas::vector<double> &CohortN,
            const boost::numeric::ublas::vector<double> &CohortF);

    void CatchCurve(
            const boost::numeric::ublas::vector<double> &Cohort,
            double &termZ,
            const int &RecAge,
            const int &Nage);

    void CohortVPA(
            const boost::numeric::ublas::vector<double> &Catch,
            const boost::numeric::ublas::vector<double> &M,
            const double &termZ,
            const int &Nage,
            boost::numeric::ublas::vector<double> &F,
            boost::numeric::ublas::vector<double> &Abundance);

    void CohortAnalysis(
            const boost::numeric::ublas::matrix<double> &Catch,
            const boost::numeric::ublas::matrix<double> &M,
            const int &TermZType,
            const int &Nage,
            const int &Nyear,
            const int &FullRecAge,
            boost::numeric::ublas::matrix<double> &F,
            boost::numeric::ublas::matrix<double> &N,
            const boost::numeric::ublas::vector<double> &initTermZ);

    /**
     * @brief Performs a separable VPA.
     * @param none
     * @return none
     */
    void SVPA(
            boost::numeric::ublas::matrix<double> &Catch,
            double InitFullF,                                          // FullF
            int IntAge,                                                // ReferenceAge
            boost::numeric::ublas::matrix<double> &M_NaturalMortality, // lclM
            boost::numeric::ublas::matrix<double> &Fout,               // lclF
            boost::numeric::ublas::matrix<double> &PopulationSize,     // lclN
            int NumCatchAges,
            int NumCatchYears,
            const boost::numeric::ublas::vector<double> &InitialSelectivity,
            double &CV_coeffVariation,
            double &SE_standardError,
            int &TotalNumIterations);

public:

    /**
     * @brief Class constructor
     */
    nmfSSVPA();

    /**
     * @brief Performs a full separable VPA.
     * @param none
     * @return none
     */
    void Full_SVPA(
            const FullSVPAInputArgs    &argsIn,
                  FullSVPAOutputArgs   &argsOut);

    void CandSVPA(
            const CandSVPAInputArgs    &argsIn,
                  CandSVPAOutputArgs   &argsOut);

    void Effort_Tune_VPA(
            const EffortTuneInputArgs  &argsIn,
                  EffortTuneOutputArgs &argsOut);

    void XSA(
            const XSAInputArgs         &argsIn,
                  XSAOutputArgs        &argsOut);

};


#endif /* NMFSSVPA_H_ */
