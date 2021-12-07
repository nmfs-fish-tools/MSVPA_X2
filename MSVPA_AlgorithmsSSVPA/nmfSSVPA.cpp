
/**
 *
 * @file nmfSSVPA.cpp
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
 * @brief This file contains the SVPA API implementation.
 */

#define DEBUG 1
#ifndef DEBUG
#define DEBUG_MSG(str) do {std::cout << str << std::endl;} while (false)
#else
#define DEBUG_MSG(str) do {} while (false)
#endif


#include "nmfDatabase.h"
#include "nmfConstants.h"

#include "nmfUtils.h"
#include "nmfSSVPA.h"


nmfSSVPA::nmfSSVPA()
{
}

void
nmfSSVPA::SVPA(
        boost::numeric::ublas::matrix<double> &CatchMatrix,
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
        int &TotalNumIterations)
{
    //std::cout << "Begin SVPA " << std::endl;

    // Constants
    const int    MAXIMUM_NUMBER_OF_ITERATIONS = 500;
    const double TOLERANCE_LEVEL              = 0.0001;
    const double FACTOR_TO_FORCE_CONVERGENCE  = 0.5;

    // Define and initialize local variables
    boost::numeric::ublas::matrix<double> d_Actual_Ratios; // initialized
    boost::numeric::ublas::matrix<double> Dhat_Calculated_Ratios; // initialized
    boost::numeric::ublas::matrix<double> Z_TotalMortality; // initialized
    boost::numeric::ublas::matrix<double> Fval; // initialized
    boost::numeric::ublas::vector<double> FullF; // initialized
    boost::numeric::ublas::vector<double> Selectivity; // initialized
    boost::numeric::ublas::vector<double> YearResiduals; // initialized
    boost::numeric::ublas::vector<double> AgeResiduals; // initialized
    boost::numeric::ublas::vector<double> ExpectedCatch; // initialized

    nmfUtils::initialize(Z_TotalMortality,       NumCatchYears, NumCatchAges);
    nmfUtils::initialize(d_Actual_Ratios,        NumCatchYears, NumCatchAges);
    nmfUtils::initialize(Dhat_Calculated_Ratios, NumCatchYears,	NumCatchAges);
    nmfUtils::initialize(Fval,                   NumCatchYears, NumCatchAges);
    nmfUtils::initialize(FullF,         NumCatchYears);
    nmfUtils::initialize(Selectivity,   NumCatchAges);
    nmfUtils::initialize(YearResiduals, NumCatchYears);
    nmfUtils::initialize(AgeResiduals,  NumCatchAges);
    nmfUtils::initialize(ExpectedCatch, NumCatchAges);

    int NumIterations = 0;
    int Kmax = 0; // max age in the catch record for a cohort
    double TermTop = 0.0;
    double TermBot = 0.0;
    double SumDhat = 0.0;
    double SumD = 0.0;
    double SumSqResidual = 0.0;
    double RefSumSqResidual = 0.0;
    double Residual = 0.0;
    double DoneCheck = 1.0;
    double tempValue = 0.0;
    double SumCatchExpected = 0.0;
    double SumZ = 0.0;

    // First step...transform the catch matrix into the log transformed catch ratio
    for (auto row = 0; row < NumCatchYears - 1; ++row) {
        for (auto col = 0; col < NumCatchAges - 1; ++col) {
            if (CatchMatrix(row, col) == 0) {
                CatchMatrix(row, col) = 0.001;
            }
            d_Actual_Ratios(row, col) =
                    std::log(CatchMatrix(row + 1, col + 1) / CatchMatrix(row, col));

        }
    }


    // Initialize FullF() and Selectivity()
    for (auto row = 0; row < NumCatchYears; ++row) {
        FullF(row) = InitFullF;
    }
    for (auto col = 0; col < NumCatchAges; ++col) {
        Selectivity(col) = InitialSelectivity(col);
    }

    //nmfUtils::printVector("Selectivity", Selectivity);

    // This is going to be an iteration loop...
    while ((DoneCheck > TOLERANCE_LEVEL)
            && (NumIterations < MAXIMUM_NUMBER_OF_ITERATIONS))
    {
        // Calculate Dhats eq. 4 of Pope and Shepard 1982
        // First calculate Fvals from FullF() x Selectivity()
        for (auto row = 0; row < NumCatchYears; ++row) {
            for (auto col = 0; col < NumCatchAges; ++col) {
                if (Selectivity(col) == 0.0) {
                    Selectivity(col) = 0.0001;
                }
                Fval(row, col) = Selectivity(col) * FullF(row);
                Z_TotalMortality(row, col) = Fval(row, col)
                        + M_NaturalMortality(row, col);
            }
        }

        // Calculate Dhats
        for (auto row = 0; row < NumCatchYears - 1; ++row) {
            for (auto col = 0; col < NumCatchAges - 1; ++col) {

                TermTop = Z_TotalMortality(row, col)
                        * (1 - std::exp(-Z_TotalMortality(row + 1, col + 1)))
                        * std::exp(-Z_TotalMortality(row, col));
                TermBot = Fval(row, col) * Z_TotalMortality(row + 1, col + 1)
                        * (1 - std::exp(-Z_TotalMortality(row, col)));

                Dhat_Calculated_Ratios(row, col) =
                        std::log(Fval(row + 1, col + 1) * (TermTop / TermBot));
            }
        }

        // Calculate the marginal totals and residuals...first across years
        for (auto row = 0; row < NumCatchYears - 1; ++row) {
            SumDhat = 0.0;
            SumD = 0.0;
            for (auto col = 0; col < NumCatchAges - 1; ++col) {
                SumDhat += Dhat_Calculated_Ratios(row, col);
                SumD += d_Actual_Ratios(row, col);
            }
            YearResiduals(row) = SumDhat - SumD;
        }
        //nmfUtils::printVector("YearResiduals", YearResiduals);

        // Calculate the marginal totals across ages
        for (auto col = 0; col < NumCatchAges - 1; ++col) {
            SumDhat = 0.0;
            SumD = 0.0;
            for (auto row = 0; row < NumCatchYears - 1; ++row) {
                SumDhat += Dhat_Calculated_Ratios(row, col);
                SumD += d_Actual_Ratios(row, col);
            }
            AgeResiduals(col) = SumDhat - SumD;
        }
        //boost::numeric::ublas::vector<double> InitialSelectivity;

        //nmfUtils::printVector("AgeResiduals", AgeResiduals);

        // Calculate residual and sum of squares
        SumSqResidual = 0.0;
        for (auto row = 0; row < NumCatchYears; ++row) {
            for (auto col = 0; col < NumCatchAges; ++col) {
                Residual = d_Actual_Ratios(row, col)
                         - Dhat_Calculated_Ratios(row, col);
                SumSqResidual += (Residual * Residual);
            }
        }

        // Calculate Fhat and SelHat values normalized by residuals
        for (auto row = 0; row < NumCatchYears - 1; ++row) {
            // Another cheat in D. Clay's code
            nmfUtils::clamp(YearResiduals(row),-NumCatchAges,NumCatchAges);

            //Eq. 8 from Pope and Shepard
            tempValue = FullF(row);
            tempValue *= std::exp(FACTOR_TO_FORCE_CONVERGENCE *
                                  YearResiduals(row) / NumCatchAges);
            // Cheat in D.Clay's code to avoid super high F-values
            nmfUtils::clamp(tempValue,0.001,10.0);
            FullF(row) = tempValue;
        }

        for (auto col = 0; col < NumCatchAges - 1; ++col) {
            // Another cheat in D. Clay's code - similar to above
            nmfUtils::clamp(AgeResiduals(col),-NumCatchYears,NumCatchYears);

            //Eq. 8 from Pope and Shepard
            tempValue = Selectivity(col);
            tempValue *= std::exp(FACTOR_TO_FORCE_CONVERGENCE *
                                  AgeResiduals(col) / NumCatchYears);
            Selectivity(col) = tempValue;
            // 'renormalize Selhat relative to fully recruited age
            try {
                Selectivity(col) /= Selectivity(IntAge);
            } catch (const std::exception& error) {
                std::cout << "Error: " << error.what() << std::endl;
                std::cout << "There are: " << NumCatchAges << " items in array\n";
                std::cout << "Tried to index with: " << IntAge << "\n";
                Selectivity(col) = 0.0;
            }

            // Cheat in D.Clay's code to avoid super hi selection values
            nmfUtils::clamp(Selectivity(col),0.0001,3.0);
        }

        // Check for doneness
        DoneCheck = std::fabs(RefSumSqResidual - SumSqResidual);
        RefSumSqResidual = SumSqResidual;
        ++NumIterations;

    } // end while

    if (NumIterations >= MAXIMUM_NUMBER_OF_ITERATIONS) {
        std::cout << "\nWarning:  Maximum number of iterations reached in SVPA\n" << std::endl;
    }

    // Calculate variance terms for the estimate...from SumSqResidual
    CV_coeffVariation = 100 * std::sqrt(SumSqResidual / (2 * (NumCatchAges - 2)
                                            * (NumCatchYears - 2)));
    SE_standardError = 2 * std::log(1 + (CV_coeffVariation / 100));
    TotalNumIterations = NumIterations;

    // Calculate F and Z matrices from output
    for (auto row = 0; row < NumCatchYears; ++row) {
        for (auto col = 0; col < NumCatchAges; ++col) {
            tempValue = FullF(row) * Selectivity(col);
            Z_TotalMortality(row, col) = tempValue + M_NaturalMortality(row, col);
            Fout(row, col) = tempValue;
        }
    }

    // Calculate initial population sizes
    // First calculate P(i,1), age zero population sizes for all years
    for (auto row = 0; row < NumCatchYears; ++row) {
        SumCatchExpected = 0;
        if ((NumCatchYears - 1 - row) < NumCatchAges)
            Kmax = NumCatchYears - 1 - row;
        else
            Kmax = NumCatchAges - 1;

        // In eqn 11 of Pope & Shepard.
        for (auto ageCohort = 0; ageCohort <= Kmax; ++ageCohort) {
            SumZ = 0;
            if (ageCohort > 0) {
                for (auto age = 0; age < ageCohort; ++age) {
                    SumZ += Z_TotalMortality(row + age, age);
                }
            }

            ExpectedCatch(ageCohort) = (Fout(row + ageCohort, ageCohort)
                    / Z_TotalMortality(row + ageCohort, ageCohort))
                    * (1 - std::exp(-Z_TotalMortality(row + ageCohort,
                                            ageCohort))) * std::exp(-SumZ);
            SumCatchExpected += std::log(CatchMatrix(row + ageCohort, ageCohort))
                              - std::log(ExpectedCatch(ageCohort));
        }

        PopulationSize(row, 0) = std::exp(SumCatchExpected / (Kmax + 1));

        // This works great !

    }

    // Now need to calculate the pop(1,j)...popsizes in first year for ages 1-Nage
    // Basically the same idea.
    for (auto col = 1; col < NumCatchAges; ++col) // since already did age 0 in year 1
    {
        SumCatchExpected = 0;
        if ((NumCatchYears + col - 1) < NumCatchAges)
            Kmax = NumCatchYears + col - 1;
        else
            Kmax = NumCatchAges - 1;
        // Kmax = max age in the catch record for a cohort, min age is j

        for (auto ageCohort = col; ageCohort <= Kmax; ++ageCohort) {
            SumZ = 0;
            if (ageCohort > col) {
                for (auto age = col; age < ageCohort; ++age) {
                    SumZ += Z_TotalMortality(age - col, age);
                }
            }
            ExpectedCatch(ageCohort) = (Fout(ageCohort - col, ageCohort)
                    / Z_TotalMortality(ageCohort - col, ageCohort))
                    * (1 - std::exp(-Z_TotalMortality(ageCohort - col,
                                            ageCohort))) * std::exp(-SumZ);
            SumCatchExpected += (std::log(CatchMatrix(ageCohort - col, ageCohort))
                    - std::log(ExpectedCatch(ageCohort)));
        }

        PopulationSize(0, col) = std::exp(SumCatchExpected / (Kmax - col + 1));
        // This works too !
    }

    // Calculate population sizes for the rest of the matrix
    for (auto row = 0; row < NumCatchYears - 1; ++row) {
        for (auto col = 0; col < NumCatchAges - 1; ++col) {
            PopulationSize(row + 1, col + 1) = PopulationSize(row,col) *
                    std::exp(-Z_TotalMortality(row, col));
        }
    }

    // Cool...I'm finished...this works against data generated to match assumptions...
    // and works compared to Doug Clay's code.
    // Function will return by reference Fout and PopulationSize...all done.
//std::cout << "End SVPA" << std::endl;

} // end SVPA



void
nmfSSVPA::Full_SVPA(        
        const FullSVPAInputArgs  &argsIn,
              FullSVPAOutputArgs &argsOut)

{
    VectorDouble InitialSelectivity = argsIn.InitialSelectivity;
    int MaxAge                      = argsIn.MaxAge;
    int FirstCatchAge               = argsIn.FirstCatchAge;
    int NumCatchAges                = argsIn.NumCatchAges;
    int isPlusClass                 = argsIn.isPlusClass;
    double FullF                    = argsIn.FullF;
    int ReferenceAge                = argsIn.ReferenceAge;
    int NumCatchYears               = argsIn.NumCatchYears;
    MatrixDouble CatchMatrix        = argsOut.CatchMatrix;
    double CV_CoeffVariation        = argsOut.CV_CoeffVariation;
    double SE_StandardError         = argsOut.SE_StandardError;
    int TotalNumIterations          = argsOut.NumIterations;
    MatrixDouble Abundance          = argsOut.Abundance;
    MatrixDouble M_NaturalMortality = argsOut.M_NaturalMortality;
    MatrixDouble F_FishingMortality = argsOut.F_FishingMortality;

    int tmpNumCatchAges = 0;
    boost::numeric::ublas::matrix<double> lclF;
    boost::numeric::ublas::matrix<double> lclM;
    boost::numeric::ublas::matrix<double> lclN;

    nmfUtils::initialize(lclM, NumCatchYears,NumCatchAges);
    nmfUtils::initialize(lclF, NumCatchYears,NumCatchAges);
    nmfUtils::initialize(lclN, NumCatchYears,NumCatchAges);

    tmpNumCatchAges  = (isPlusClass) ? NumCatchAges-1 : NumCatchAges;

    // Reference M into NumCatchAges..delivered as 0 to MaxAge
    for (auto row=0; row<NumCatchYears; ++row) {
        for (auto col=0; col<tmpNumCatchAges; ++col) {
            lclM(row,col) = M_NaturalMortality(row, col);
        }
    }

    /*********************************
     *                               *
     *  Call the Main SVPA routine   *
     *                               *
     *********************************/
    SVPA(CatchMatrix,
         FullF,
         ReferenceAge,
         lclM,
         lclF,
         lclN,
         tmpNumCatchAges,
         NumCatchYears,
         InitialSelectivity,
         CV_CoeffVariation,
         SE_StandardError,
         TotalNumIterations);

    //  Fill results into Full F and Abundance matrices
    for (auto row = 0; row < NumCatchYears; ++row) {
        for (auto col = 0; col < tmpNumCatchAges; ++col) {
            F_FishingMortality(row, FirstCatchAge + col) = lclF(row, col);
            Abundance(row, FirstCatchAge + col) = lclN(row, col);
        }
    }


    // Fill Fs across remaining age classes
    for (auto row = 0; row < NumCatchYears; ++row) {
        for (auto col = (FirstCatchAge + tmpNumCatchAges); col <= MaxAge; ++col) {
//std::cout << "row,col: " << row << ", " << col << std::endl;
//std::cout << "row,FirstCatchAge+tmpNumCatchAges-1: " << row << ", " << FirstCatchAge + tmpNumCatchAges - 1 << std::endl;
            F_FishingMortality(row, col) = F_FishingMortality(row,
                    FirstCatchAge + tmpNumCatchAges - 1);
        }
    }

    // Project abundances to the end of the matrix
    for (auto row = 1; row < NumCatchYears; ++row) {
        for (auto col = tmpNumCatchAges; col <= MaxAge; ++col) {
            Abundance(row, col) = Abundance(row - 1, col - 1)
                    * std::exp(-1 * (F_FishingMortality(row - 1, col - 1)
                                            + M_NaturalMortality(row-1,col-1)));
        }
    }

    // Have to fill in the upper right corner of the matrix...all the earlier cohorts.
    for (auto row = 0; row < NumCatchYears; ++row) {
        for (auto col = tmpNumCatchAges; col <= MaxAge; ++col) {
            if (Abundance(row, col) == 0) {
                Abundance(row, col) = Abundance(row, col - 1)
                        * std::exp(-1 * (F_FishingMortality(row, col - 1)
                                         + M_NaturalMortality(row,col-1)));
            }
        }
    }

    // Last have to solve for earliest age class if FirstCatchAge <> 0
    for (auto row = 0; row < NumCatchYears - 1; ++row) {
        for (auto col = 0; col < FirstCatchAge; ++col) {
            Abundance(row, FirstCatchAge - 1 - col) = Abundance(row + 1,
                                                              FirstCatchAge - col) /
                                          std::exp(-M_NaturalMortality(row, col));

        }
    }


    // Finally..in the last year, cohorts with no catch will have no information so...
    for (auto col = 0; col < FirstCatchAge; ++col) {
        Abundance(NumCatchYears - 1, col) = Abundance(NumCatchYears - 2, col);
    }

    // Pack output struct to return
    argsOut.CatchMatrix        = CatchMatrix;
    argsOut.CV_CoeffVariation  = CV_CoeffVariation;
    argsOut.SE_StandardError   = SE_StandardError;
    argsOut.NumIterations      = TotalNumIterations;
    argsOut.Abundance          = Abundance;
    argsOut.M_NaturalMortality = M_NaturalMortality;
    argsOut.F_FishingMortality = F_FishingMortality;

} // end nmfFullSVPA





void
nmfSSVPA::Effort_Tune_VPA(
        const EffortTuneInputArgs  &argsIn,
              EffortTuneOutputArgs &argsOut)
{
    nmfDatabase *databasePtr       = argsIn.databasePtr;
    MapInt EffortTunedGuiData      = argsIn.EffortTunedGuiData;
    Boost4DArrayDouble FleetData   = argsIn.FleetData;
    int FirstYear                  = argsIn.FirstYear;
    int LastYear                   = argsIn.LastYear;
    int FirstCatchAge              = argsIn.FirstCatchAge;
    int LastCatchAge               = argsIn.LastCatchAge;
    int Plus                       = argsIn.Plus;
    int MaxAge                     = argsIn.MaxAge;
    int SpeIndex                   = argsIn.SpeIndex;
    std::string SSVPAName          = argsIn.SSVPAName;
    MatrixDouble Abundance         = argsOut.Abundance;
    MatrixDouble M                 = argsOut.M_NaturalMortality;
    MatrixDouble F                 = argsOut.F_FishingMortality;

    bool Converge;
    double AvgF;
    int TermZType;
    int NIncYrs;
    int NumF;
    int m;
    int NYears       = LastYear - FirstYear + 1;
    int NCatchAge    = LastCatchAge - FirstCatchAge + 1;
    int tmpNCatchAge = (Plus == 1) ? NCatchAge-1 : NCatchAge;
    int FullRecAge;
    int LoopCount=0;
    double NFleets;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    //double checksum;
//std::cout << "nmfSSVPA::Effort_Tune_VPA" << std::endl;

    boost::numeric::ublas::matrix<double> Effort;
    boost::numeric::ublas::matrix<double> tmpCatch;
    boost::numeric::ublas::matrix<double> tmpM;
    boost::numeric::ublas::matrix<double> tmpF;
    boost::numeric::ublas::matrix<double> tmpN;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> lclF;
    boost::numeric::ublas::matrix<double> lclM;
    boost::numeric::ublas::matrix<double> lclN;
    boost::numeric::ublas::matrix<double> InitSel;

    nmfUtils::initialize(Catch,    NYears+1, NCatchAge+1);
    nmfUtils::initialize(lclF,     NYears+1, NCatchAge+1);
    nmfUtils::initialize(lclN,     NYears+1, NCatchAge+1);
    nmfUtils::initialize(lclM,     NYears+1, NCatchAge+1);
    nmfUtils::initialize(tmpCatch, NYears+1, NCatchAge+1);
    nmfUtils::initialize(tmpF,     NYears+1, NCatchAge+1);
    nmfUtils::initialize(tmpN,     NYears+1, NCatchAge+1);
    nmfUtils::initialize(tmpM,     NYears+1, NCatchAge+1);
//double sum = 0.0;
//std::cout.precision(10);
    m = 0;
    fields   = {"Catch"};
    queryStr = "SELECT Catch FROM " + nmfConstantsMSVPA::TableSpeCatch +
               " WHERE SpeIndex = " + std::to_string(SpeIndex) +
               " ORDER BY Year, Age";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (int i = 0; i < NYears; ++i) {
        for (int j = 0; j < NCatchAge; ++j) {
            Catch(i,j) = std::stod(dataMap["Catch"][m++]);
//sum += Catch(i,j);

        } // end for j
    } // end for i
//std::cout << "Catch total: " << sum << std::endl;

//checksum = 0.0;
    // Reference M into NCatAge..delivered as 0 to MaxAge
    for (int i = 0; i < NYears; ++i) {
        for (int j = 0; j < tmpNCatchAge; ++j) {
            lclM(i,j) = M(i,FirstCatchAge+j);
//checksum += lclM(i,j);
        } // end for j
    } // end for i
//std::cout << "lclM total: " << checksum << std::endl;

    fields     = {"NFleets","FullRecAge"};
    queryStr   = "SELECT NFleets,FullRecAge FROM " + nmfConstantsMSVPA::TableSpeSSVPA +
                 " WHERE SpeIndex = " + std::to_string(SpeIndex) +
                 " AND SSVPAName = '" + SSVPAName + "'";
    dataMap    = databasePtr->nmfQueryDatabase(queryStr, fields);
    NFleets    = std::stod(dataMap["NFleets"][0]);

    if (EffortTunedGuiData.empty()) {
        FullRecAge = std::stod(dataMap["FullRecAge"][0]);
        //NFleets    = std::stod(dataMap["NFleets"][0]);
    } else {
        FullRecAge = EffortTunedGuiData.at("FullRecAge");
        //NFleets    = EffortTunedGuiData.at("NFleets");
    }

    boost::numeric::ublas::vector<double> initTermZ;
    nmfUtils::initialize(initTermZ, 10);

    // Run the cohort analysis..using the catchcurve analysis for Z in last age class
    TermZType = 1;
    CohortAnalysis(Catch,
                   lclM,
                   TermZType,
                   tmpNCatchAge,
                   NYears,
                   FullRecAge,
                   lclF,
                   lclN,
                   initTermZ);
//checksum = 0.0;
    nmfUtils::initialize(Effort, NFleets, NYears);
//double sum=0;
    if (EffortTunedGuiData.empty()) {
//std::cout << "A" << std::endl;
        // Get all the effort data, by fleet for each fleet, year
        fields   = {"Effort"};
        queryStr = "SELECT Effort FROM " + nmfConstantsMSVPA::TableSpeTuneEffort +
                   " WHERE SpeIndex = " + std::to_string(SpeIndex) +
                   " AND SSVPAName = '" + SSVPAName + "'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        m = 0;
        for (int i = 0; i < NFleets; ++i) {
            for (int j = 0; j < NYears; ++j) {
                Effort(i,j) = std::stod(dataMap["Effort"][m++]);
            } // end for j
        } // end for i
    } else {
//std::cout << "B" << std::endl;
        for (int i = 0; i < NFleets; ++i) {
            for (int j = 0; j < NYears; ++j) {
                    Effort(i,j) = FleetData[i][SpeIndex][j][0];
//sum += Effort(i,j);
                //}
            } // end for j
        } // end for i
    }
//std::cout << "NYears: " << NYears << std::endl;
//std::cout << "NCatchAge:  " << NCatchAge  << std::endl;
//std::cout << "Effort total: " << sum << std::endl;


    Boost3DArrayDouble FleetCatch(boost::extents[NFleets][NYears][NCatchAge]);
//checksum = 0.0;
    if (1) { //EffortTunedGuiData.empty()) {
        m = 0;
        fields   = {"Catch"};
        queryStr = "SELECT Catch FROM " + nmfConstantsMSVPA::TableSpeTuneCatch +
                   " WHERE SpeIndex = " + std::to_string(SpeIndex) +
                   " AND SSVPAName = '" + SSVPAName + "' ORDER BY Fleet, Year, Age";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        for (int i = 0; i < NFleets; ++i) {
            for (int j = 0; j < NYears; ++j) {
                for (int k = 0; k < tmpNCatchAge; ++k) {
                    FleetCatch[i][j][k] = std::stod(dataMap["Catch"][m++]);
//checksum += FleetCatch[i][j][k];
                } // end for k
            } // end for j
        } // end for i
    }
//    else {
//std::cout << "BB" << std::endl;
//        for (int i = 0; i < NFleets; ++i) {
//            for (int j = 0; j < NYears; ++j) {
//                for (int k = 0; k < tmpNCatchAge; ++k) {
//                    FleetCatch[i][j][k] = FleetData[i][SpeIndex][j][k+1];
//checksum += FleetCatch[i][j][k];
//                } // end for k
//            } // end for j
//        } // end for i
//    }
//std::cout << "FleetCatch total: " << checksum << std::endl;

    // >>>>>>>>>>>>>>>This is going to be the start of the iteration loop >>>>>>>>>>>>>>>>>'
    Converge = false;
    boost::numeric::ublas::vector<double> MeanFest;    // This is the F at age vector for the terminal year
    nmfUtils::initialize(MeanFest, NCatchAge);
    boost::numeric::ublas::vector<double> RefMeanFest; // This matrix holds the value for the previous iteration
    nmfUtils::initialize(RefMeanFest, NCatchAge);

    Boost3DArrayDouble FleetF(boost::extents[NFleets][NYears][tmpNCatchAge]);
    Boost3DArrayDouble LogQ(boost::extents[NFleets][NYears][tmpNCatchAge]);
    boost::numeric::ublas::matrix<double> AvgLogQ;
    boost::numeric::ublas::matrix<double> SumLogQ2;
    boost::numeric::ublas::matrix<double> VarLogQ;
    boost::numeric::ublas::matrix<double> TermFest;
    boost::numeric::ublas::matrix<int>    YrCount;
    boost::numeric::ublas::vector<double> SumInvVar;
    boost::numeric::ublas::vector<double> MeanTop;
    boost::numeric::ublas::matrix<double> CohortFleetCatch;
    boost::numeric::ublas::vector<double> CohortTotalCatch;
    boost::numeric::ublas::vector<double> CohortEffort;
    boost::numeric::ublas::vector<double> FOldestAge;
    boost::numeric::ublas::matrix<double> CohortFest;
    boost::numeric::ublas::vector<double> CohortMeanFest; // this is the F at age vector for the cohort
    boost::numeric::ublas::vector<double> CohortSumInvVar;
    boost::numeric::ublas::vector<double> CohortMeanTop;


    while (! Converge) {
//checksum = 0.0;
//sum=0.0;

        // OK...so already have F() matrix for all complete years and true age classes....
        // Also already have the Catch data for everyone
        // So calculate F(Fleet,Year,Age) = F(year,age) * Catch(fleet,year,age)/Catch(year,age)
        nmfUtils::initialize(FleetF); // this may be slow...
        for (int i = 0; i < NFleets; ++i) {
            for (int j = 0; j < NYears; ++j) {
                for (int k = 0; k < tmpNCatchAge; ++k) {
//sum += lclF(j,k);
                    if (lclF(j, k) > 0) {
                        FleetF[i][j][k] = lclF(j,k) * (FleetCatch[i][j][k] / Catch(j,k));
//checksum += FleetF[i][j][k];
                    } // end if
                } // end for k
            } // end for j
        } // end for i
//std::cout << "lclF total: " << sum << std::endl;
//std::cout << "FleetF total: " << checksum << std::endl;

        // Next calculate logQ(Fleet, Year, Age)= ln[FleetF(fleet, year, age)/Effort(fleet,year)]
        nmfUtils::initialize(LogQ); // this may be slow...
//checksum = 0.0;
//double sum = 0;
        for (int i = 0; i < NFleets; ++i) {
            for (int j = 0; j < NYears; ++j) {
                for (int k = 0; k < tmpNCatchAge; ++k) {
                    LogQ[i][j][k] =  (FleetF[i][j][k] > 0) ?
                         std::log(FleetF[i][j][k] / Effort(i,j)) : 99;
//sum += LogQ[i][j][k];
                } // end for k
            } // end for j
        } // end for i
//std::cout << "LogQ total: " << sum << std::endl;

        // Then calculate average and variance of LogQ by Fleet, Age..across all available LogQ for each age class
        nmfUtils::initialize(AvgLogQ,  NFleets, NCatchAge+1);
        nmfUtils::initialize(SumLogQ2, NFleets, NCatchAge+1);
        nmfUtils::initialize(VarLogQ,  NFleets, NCatchAge+1);
        nmfUtils::initialize(YrCount,  NFleets, NCatchAge+1);
        nmfUtils::initialize(TermFest, NFleets, NCatchAge+1); // This is the F at age estimate in the last year...
        double tempLogQ;
        for (int i = 0; i < NFleets; ++i) {
            for (int j = 0; j < NYears; ++j) {
                for (int k = 0; k < tmpNCatchAge; ++k) {
                    tempLogQ = LogQ[i][j][k];
                    if ( tempLogQ != 99) {
                        AvgLogQ(i,k)  += tempLogQ;
                        SumLogQ2(i,k) += tempLogQ*tempLogQ;
                        ++YrCount(i,k);
                    } // end if

                } // end for k
            } // end for j
        } // end for i



        double yrCount;
        double avgLogQ;
        for (int i = 0; i < NFleets; ++i) {
            for (int k = 0; k < tmpNCatchAge; ++k) {
                yrCount = YrCount(i,k);
                avgLogQ = AvgLogQ(i,k);
                VarLogQ(i, k) = (SumLogQ2(i,k) - (avgLogQ*avgLogQ)/yrCount) / (yrCount - 1);
//sum1 += VarLogQ(i, k); // this doesn't change with new Effort
                AvgLogQ(i, k) /= yrCount;
//sum2 += AvgLogQ(i, k); // this changes with new Effort
            } // end for k
        } // end for i
//std::cout << "VarLogQ total: " << sum1 << std::endl;
//std::cout << "AvgLogQ total: " << sum2 << std::endl;

        // Calculate Estimated F by age in last year..based upon effort in last year..and raised by inverse proportion of catch
        for (int i = 0; i < NFleets; ++i) {
            for (int k = 0; k < tmpNCatchAge; ++k) {
                TermFest(i, k) = (Catch(NYears-1,k) / FleetCatch[i][NYears-1][k]) *
                                  std::exp(AvgLogQ(i,k)) * Effort(i,NYears-1);
//sum1 += AvgLogQ(i,k);
//sum2 += Effort(i,NYears-1);
//sum3 += std::exp(AvgLogQ(i,k)) * Effort(i,NYears-1);
//std::cout << "prod: " << AvgLogQ(i,k) << " * " << Effort(i,NYears-1) << ": " << std::exp(AvgLogQ(i,k)) * Effort(i,NYears-1) << std::endl;
//sum9 += TermFest(i,k);
            } // end for k
        } // end for i
//std::cout << "AvgLogQ, Effort, prod, TermFest totals: " << sum1 << ", " << sum2 << ", " << sum3 << ", " << sum9 << std::endl;

        // Calculate inverse variance weighted terminal F across fleets...this is your final answer..for the last year
        nmfUtils::initialize(SumInvVar, NCatchAge+1);
        nmfUtils::initialize(MeanTop,   NCatchAge+1);

        double varLogQ;
        for (int i = 0; i < NFleets; ++i) {
            for (int k = 0; k < tmpNCatchAge; ++k) {
                varLogQ = VarLogQ(i,k);
                MeanTop(k)   += ((1.0 / varLogQ) * std::log(TermFest(i,k)));
                SumInvVar(k) +=  (1.0 / varLogQ);
            } // end for k
        } // end for i
//double sum = 0;
        for (int k = 0; k < tmpNCatchAge; ++k) {
            MeanFest(k) = std::exp(MeanTop(k)/SumInvVar(k));
//sum += MeanFest(k);
        } // end for k
//std::cout << "MeanFest: " << sum << std::endl;

        // >>>>>>This is where you will check for convergence....compare MeanFest(k) to previous>>>>>>'
        Converge = true;
        for (int k = 0; k < NCatchAge; ++k) {
            if (std::fabs(RefMeanFest(k) - MeanFest(k)) > 0.0001) {
                Converge = false;
                break;
            } // end if
        } // end for k

        ++LoopCount;
        for (int k = 0; k < tmpNCatchAge; ++k) {
            RefMeanFest(k) = MeanFest(k);
        } // end for k

        // All of the above code initializes the F matrix, AvgLogQs, etc
        // The next section of code becomes the iterative part.  At each iteration, we are going to calculate F in the oldest
        // age class for each cohort in the catch matrix based upon catchabilities and effort.  The F on the oldest age is
        // the average F of the last three ages, calculated in the same way as the Estimates in the terminal year.  The result is
        // a vector of F rates on the oldest age class for each year

        // So..the first step is to calculate the Fest for the oldest age of each cohort.
        int FirstCohort = -tmpNCatchAge + 1;
        int LastCohort = NYears - 2;
        int Row;
        int Age;

        nmfUtils::initialize(CohortFleetCatch, NFleets, NCatchAge+1);
        nmfUtils::initialize(CohortTotalCatch, NCatchAge+1);
        nmfUtils::initialize(CohortEffort,     NCatchAge+1);
        nmfUtils::initialize(FOldestAge,       NYears+tmpNCatchAge+1);
//sum1 = 0;

        for (int Count = FirstCohort; Count <= LastCohort; ++Count)
        {
//std::cout << "nmfSSVPA::Effort_Tune_VPA Count: " << Count << std::endl;

            // First get the catch at age by fleet for the cohort...
            Row = Count + (tmpNCatchAge - 1);
            for (int i = 0; i < tmpNCatchAge; ++i) {
                for (int j = 0; j < NFleets; ++j) {
                    // Initialize cohortF and CohortN vectors
                    // CohortFleetCatch(j, i) = -9
                    Age = tmpNCatchAge - i - 1;
                    if (Row >= 0 && Row < NYears) {
                        CohortFleetCatch(j,Age) = FleetCatch[j][Row][Age];
                        CohortTotalCatch(Age) = Catch(Row,Age);
                        CohortEffort(Age) = Effort(j,Row);
                    } else {
                        CohortFleetCatch(j,Age) = -9;
                        CohortTotalCatch(Age) = -9;
                        CohortEffort(Age) = -9;
                    } // end if
                } // end for j
                --Row;
            } // end for i

            // Calculate Estimated F by age in for the cohort..based upon effort and raised by inverse proportion of total catch

            nmfUtils::initialize(CohortFest, NFleets, NCatchAge);

            for (int i = 0; i < NFleets; ++i) {
                for (int j = 0; j < tmpNCatchAge; ++j) {
                    CohortFest(i, j) =
                        (CohortFleetCatch(i, j) > -9) ?
                        (CohortTotalCatch(j)/CohortFleetCatch(i,j)) * std::exp(AvgLogQ(i,j)) * CohortEffort(j) :
                        -9;
                } // end for j
            } // end for i

            // Calculate inverse variance weighted terminal F at age across fleets for the cohort
            nmfUtils::initialize(CohortMeanFest,  NCatchAge+1);
            nmfUtils::initialize(CohortSumInvVar, NCatchAge+1);
            nmfUtils::initialize(CohortMeanTop,   NCatchAge+1);

            for (int i = 0; i < NFleets; ++i) {
                for (int j = 0; j < tmpNCatchAge; ++j) {
                    if (CohortFest(i,j) != -9) {
                        varLogQ = VarLogQ(i,j);
                        CohortMeanTop(j)   += ((1.0 / varLogQ) * std::log(CohortFest(i,j)));
                        CohortSumInvVar(j) +=  (1.0 / varLogQ);
                    } else {
                        CohortMeanTop(j) = -9;
                    } // end if
                } // end for j
            } // end for i

            for (int j = 0; j < tmpNCatchAge; ++j) {
                CohortMeanFest(j) = (CohortMeanTop(j) != -9) ?
                                    std::exp(CohortMeanTop(j)/CohortSumInvVar(j)) :
                                    -9;
            } // end for j

            // Now calculate the average F for the last 3 ages....this will be your answer
            NumF = 0;
            AvgF = 0.0;
            for (int i = tmpNCatchAge - 3; i < tmpNCatchAge; ++i) {
                if (CohortMeanFest(i) != -9) {
                    AvgF += CohortMeanFest(i);
                    ++NumF;
                } // end if
            } // end for i
            if (NumF > 0) {
                AvgF /= NumF;
            }

            // Put this in an appropriate place...this will be a F at oldest age (1x year) vector
            if ((Count + tmpNCatchAge - 1) >= 0) {
                FOldestAge(Count + tmpNCatchAge - 1) = AvgF;
//sum1 += AvgF;

            }
        } // end for Count

//std::cout << "FOldestAge total: " << sum1 << std::endl;

        // Do the cohort analysis using FOldestAge as source for terminal Z values instead of the catch curve
        TermZType = 2;
        CohortAnalysis(Catch,
                       lclM,
                       TermZType,
                       tmpNCatchAge,
                       NYears,
                       FullRecAge,
                       lclF,
                       lclN,
                       FOldestAge);

        // Could potentially fill in the additional, incomplete cohorts here..then the whole thing would
        // be converged togethor...probably makes sense...

        // Send Fatage values them to the cohort_analysis sub with termztype = 3
        // and the appropriate years of the catch matrix...will also need a temporary Catch, M, abundance, and F matrix to go with it
        // Fill tmpM up with Mat age for the last Ninc age classes
        NIncYrs = tmpNCatchAge - 3;
        int diffYears = NYears-NIncYrs;
        for (int i = diffYears; i < NYears; ++i) {
            for (int j = 0; j < tmpNCatchAge; ++j) {
                tmpM(i-diffYears,j)     = M(i,j);
                tmpCatch(i-diffYears,j) = Catch(i,j);
            } // end for j
        } // end for i

        TermZType  = 3;
        FullRecAge = 1;
        CohortAnalysis(tmpCatch,
                       tmpM,
                       TermZType,
                       tmpNCatchAge,
                       NIncYrs,
                       FullRecAge,
                       tmpF,
                       tmpN,
                       MeanFest);


        // OK..that works just freakin fine...now put the output from tmpF and tmpAbund into the F and Abundance matrices
        for (int i = diffYears; i < NYears; ++i) {
            for (int j = 0; j < tmpNCatchAge; ++j) {
                if (lclN(i,j) == -9) {
                    lclN(i,j) = tmpN(i-diffYears,j);
                    lclF(i,j) = tmpF(i-diffYears,j);
                } // end if
            } // end for j
        } // end for i

        if (LoopCount > 30)
            break;

        // Then at the end of the loop..you would be finished..and have a whole matrix to work
        // with to calculate logQs in the next loop
    } // end while

//double sum3 = 0;
    // Fill results into Full F and Abundance matrices
    for (int i = 0; i < NYears; ++i) {
        for (int j = 0; j < tmpNCatchAge; ++j) {
            F(i,FirstCatchAge+j)         = lclF(i,j);
//sum3 += F(i,FirstCatchAge+j);
            Abundance(i,FirstCatchAge+j) = lclN(i,j);
        } // end for j
    } // end for i

    // Fill Fs across remaining age classes
    for (int i = 0; i < NYears; ++i) {
        for (int j = (FirstCatchAge + tmpNCatchAge); j <= MaxAge; ++j) {
            F(i,j) = F(i,FirstCatchAge+tmpNCatchAge-1);
//sum3 += F(i,j);
        } // end for j
    } // end for i

    // Project abundances to the end of the matrix
    for (int i = 1; i < NYears; ++i) {
        for (int j = tmpNCatchAge; j <= MaxAge; ++j) {
            Abundance(i,j) = Abundance(i-1,j-1) * std::exp(-(F(i-1,j-1) + M(i-1,j-1)));
        } // end for j
    } // end for i


    // Last have to solve for earliest age class if FirstCatchAge <> 0
    for (int i = 0; i < NYears-1; ++i) {
        for (int j = 0; j < FirstCatchAge; ++j) {
            Abundance(i,FirstCatchAge-1-j) = Abundance(i+1,FirstCatchAge-j)/std::exp(-M(i,j));
        } // end for j
    } // end for i
//double sum2 = 0;
    // Finally..in the last year, cohorts with no catch will have no information so...
    for (int i = 0; i < FirstCatchAge; ++i) {
        Abundance(NYears-1,i) = Abundance(NYears-2,i);
//sum2 +=   Abundance(NYears-1,i);

    } // end for i
//std::cout << "Abundance total: " << sum2 << std::endl;
//std::cout << "F total: " << sum3 << std::endl;



    std::cout << "nmfSSVPA::Effort_Tune_VPA Abundance complete" << std::endl;

    // Load up struct to return matrices
    argsOut.Abundance          = Abundance;
    argsOut.M_NaturalMortality = M;
    argsOut.F_FishingMortality = F;

} // end Effort_Tune_VPA




void
nmfSSVPA::CandSVPA(
        const CandSVPAInputArgs  &argsIn,
              CandSVPAOutputArgs &argsOut)
{
    nmfDatabase* databasePtr        = argsIn.databasePtr;
    MapInt CohortAnalysisGuiData    = argsIn.CohortAnalysisGuiData;
    int FirstYear                   = argsIn.FirstYear;
    int LastYear                    = argsIn.LastYear;
    int FirstCatchAge               = argsIn.FirstCatchAge;
    int LastCatchAge                = argsIn.LastCatchAge;
    int Plus                        = argsIn.Plus;
    int MaxAge                      = argsIn.MaxAge;
    int SpeIndex                    = argsIn.SpeIndex;
    std::string SSVPAname           = argsIn.SSVPAName;
    MatrixDouble M_NaturalMortality = argsIn.M_NaturalMortality;
    double CV                       = argsOut.CV_CoeffVariation;
    double SE                       = argsOut.SE_StandardError;
    int NIt                         = argsOut.NumIterations;
    MatrixDouble Abundance          = argsOut.Abundance;
    MatrixDouble F_FishingMortality = argsOut.F_FishingMortality;

    int m;
    int FullRecAge;
    int pSVPANYrs;
    int pSVPANMortYrs;
    int pSVPARefAge;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> lclF;
    boost::numeric::ublas::matrix<double> lclN;
    boost::numeric::ublas::matrix<double> lclM;
    boost::numeric::ublas::matrix<double> tmpM;
    boost::numeric::ublas::matrix<double> tmpF;
    boost::numeric::ublas::matrix<double> tmpAbund;
    boost::numeric::ublas::matrix<double> pSVPACatch;
    boost::numeric::ublas::vector<double> InitSel;
    boost::numeric::ublas::vector<double> initTermZ;

    std::string msg="";
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    double MeanFullF[20];
    //double InitSel[20];
    //double initTermZ[6]; // double check to see if this is needed - RSK

    // First collect the age information for the species...and whether it is a plus class or not.

    int NYears       = LastYear - FirstYear + 1;
    int NCatchAge    = LastCatchAge - FirstCatchAge + 1;
    int tmpNCatchAge = (Plus == 1) ? NCatchAge-1 : NCatchAge;

    nmfUtils::initialize(Catch,    NYears+1, NCatchAge+1);
    nmfUtils::initialize(lclF,     NYears+1, NCatchAge+1);
    nmfUtils::initialize(lclN,     NYears+1, NCatchAge+1);
    nmfUtils::initialize(lclM,     NYears+1, NCatchAge+1);

    // Load the Catch matrix from the Database
    m = 0;
    fields = {"Catch"};
    queryStr = "SELECT Catch FROM " + nmfConstantsMSVPA::TableSpeCatch +
               " WHERE SpeIndex = " + std::to_string(SpeIndex) +
               " ORDER BY Year, Age";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    int NumRecords = dataMap["Catch"].size();
    if (NumRecords != NYears*NCatchAge) {
        std::cout << "Error: NumRecords (" << NumRecords << ") does not equal "
                  << "NYears*NCatchAge (" << NYears << "*" << NCatchAge << ")" << std::endl;
        std::cout << queryStr << std::endl;
        return;
    }
    for (int i = 0; i < NYears; ++i) {
        for (int j = 0; j < NCatchAge; ++j) {
            Catch(i,j) = std::stod(dataMap["Catch"][m++]);
        } // end for j
    } // end for i

    if (CohortAnalysisGuiData.empty()) {
        // Load the SSVPA configuration from the database and associated information
        fields   = {"FullRecAge","pSVPANCatYrs","pSVPANMortYrs","pSVPARefAge"};
        queryStr = "SELECT FullRecAge,pSVPANCatYrs,pSVPANMortYrs,pSVPARefAge FROM " +
                    nmfConstantsMSVPA::TableSpeSSVPA +
                   " WHERE SpeIndex = " + std::to_string(SpeIndex) +
                   " AND SSVPAName = '" + SSVPAname + "'";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        FullRecAge    = std::stoi(dataMap["FullRecAge"][0]);
        pSVPANYrs     = std::stoi(dataMap["pSVPANCatYrs"][0]);
        pSVPANMortYrs = std::stoi(dataMap["pSVPANMortYrs"][0]);
        pSVPARefAge   = std::stoi(dataMap["pSVPARefAge"][0]);
    } else {
        FullRecAge    = CohortAnalysisGuiData.at("FullRecAge");
        pSVPANYrs     = CohortAnalysisGuiData.at("pSVPANCatYrs");
        pSVPANMortYrs = CohortAnalysisGuiData.at("pSVPANMortYrs");
        pSVPARefAge   = CohortAnalysisGuiData.at("pSVPARefAge");
    }

    // Reference M into NCatAge..delivered as 0 to MaxAge
    for (int i = 0; i < NYears; ++i) {
        for (int j = 0; j < tmpNCatchAge; ++j) {
            lclM(i,j) = M_NaturalMortality(i,FirstCatchAge+j);
        } // end for j
    } // end for i

    nmfUtils::initialize(initTermZ, 100);

    // Run the cohort analysis..using the catchcurve analysis for Z in last age class
    int TermZType = 1;
    CohortAnalysis(Catch,
                   lclM,
                   TermZType,
                   tmpNCatchAge,
                   NYears,
                   FullRecAge,
                   lclF,
                   lclN,
                   initTermZ);

    // Now do the partial SPVA
    // First fill the pSVPACatch matrix with data for the last pSVPANCatYrs..can get this from the catch matrix
    // ...natural mortality rates too
    nmfUtils::initialize(pSVPACatch, pSVPANYrs+1, tmpNCatchAge+1);
    nmfUtils::initialize(tmpM,       pSVPANYrs+1, tmpNCatchAge+1);
    for (int i = 0; i < pSVPANYrs; ++i) {
        for (int j = 0; j < tmpNCatchAge; ++j) {
            pSVPACatch(i,j) = Catch(NYears-pSVPANYrs+i,j);
            tmpM(i,j)       = lclM(i,j);
        } // end for j
    } // end for i

    // Now will have to solve for initial selectivity vector and F..not necessarily flat top recruitment
    int pSVPALastYr   = LastYear - (FullRecAge + 1) - FirstYear;
    int NIncYrs       = FullRecAge + 1;
    int pSVPAFirstYr  = pSVPALastYr - pSVPANMortYrs + 1;
    int FirstMortYear = NYears - NIncYrs - pSVPANMortYrs;

    nmfUtils::initialize(tmpF, NYears+1, tmpNCatchAge+1);

    for (int i = 0; i < pSVPANMortYrs; ++i) {
        for (int j = 0; j < tmpNCatchAge; ++j) {
            tmpF(i,j) = lclF(pSVPAFirstYr+i,j);
        } // end for j
    } // end for i

    // Calculate the average F at age across the mort years..these are the last pSVPAnMortyrs before the incomplete cohorts
    int TotN;
    for (int j = 0; j < tmpNCatchAge; ++j) {
        TotN = 0;
        for (int i = 0; i < pSVPANMortYrs; ++i) {
            if (lclF(i+FirstMortYear,j) > 0) {
                MeanFullF[j] += lclF(i + FirstMortYear, j);
                ++TotN;
            } // end if
        } // end for i
        MeanFullF[j] /= TotN;  // gives average F at Age, not weighted by anything
    } // end for j

    double FullF;
    nmfUtils::initialize(InitSel, 100);

    // FullF is the F for the Reference age..right ?..may have to adjust this if first age <> 0
    FullF = MeanFullF[pSVPARefAge - FirstCatchAge];
    // Now need to calculate selectivity vector...referenced to ref age....simple
    for (int j = 0; j < tmpNCatchAge; ++j) {
      InitSel[j] = MeanFullF[j] / FullF;
    } // end for j

    nmfUtils::initialize(tmpAbund, pSVPANYrs+1, tmpNCatchAge+1);

    // Clear out tmpF
    nmfUtils::initialize(tmpF, NYears+1, tmpNCatchAge+1);

    SVPA(pSVPACatch,
         FullF,
         pSVPARefAge,
         tmpM,
         tmpF,
         tmpAbund,
         tmpNCatchAge,
         pSVPANYrs,
         InitSel,
         CV,
         SE,
         NIt);

    if (pSVPANYrs-NIncYrs < 0) {
        msg  = "Error: pSVPANCatYrs of " + std::to_string(pSVPANYrs);
        msg += " < (FullRedAge+1) of " + std::to_string(NIncYrs) + " and shouldn't be.";
        msg += " Check data in table: SpeSSVPA";
        std::cout << msg << std::endl;
        return;
    }

    // Ok...so fill in the lclF and Abundance matrices with output from the SVPA
    // Find the first incomplete year in the output matrix
    int IncYrCount = 0;
    int FirstINcYear = NYears - NIncYrs;
    for (int i = (pSVPANYrs - NIncYrs); i < pSVPANYrs; ++i) {
        for (int j = 0; j < tmpNCatchAge; ++j) {

            if (lclN(FirstINcYear+IncYrCount,j) == -9) {
                lclN(FirstINcYear+IncYrCount,j) = tmpAbund(i,j);
                lclF(FirstINcYear+IncYrCount,j) = tmpF(i,j);
            } // end if
        } // end for j
        ++IncYrCount;
    } // end for i

    // OK....if its a full rec, then the last age class in year 0 will be a -9 and
    // will need to be filled from below--this will be general since catchcurve requires at least 2 recruited ages
    // full rec age has to be <LastCatchAge - 2
    lclF(0,tmpNCatchAge-1) = lclF(1,tmpNCatchAge-1);
    lclN(0,tmpNCatchAge-1) = lclN(1,tmpNCatchAge-1);
    // Ok...now fill information into output F,M, and abundance matrices...this will involve solving for earlier ages
    // and projecting out the last ages to max age

    // First translate F data from lclF to F
    for (int i = 0; i < NYears; ++i) {
        for (int j = 0; j < tmpNCatchAge; ++j) {
            F_FishingMortality(i,FirstCatchAge+j) = lclF(i,j);
            Abundance(i,FirstCatchAge+j) = lclN(i,j);
        } // end for j
    } // end for i

    // Fill in Fs for each additional age class across rows
    for (int i = 0; i < NYears; ++i) {
        for (int j = (FirstCatchAge + tmpNCatchAge); j <= MaxAge; ++j) {
            F_FishingMortality(i,j) = F_FishingMortality(i,(FirstCatchAge+tmpNCatchAge-1));
        } // end for j
    } // end for i

    // Project abundances to the end of the matrix
    for (int i = 1; i < NYears; ++i) {
        for (int j = tmpNCatchAge; j <= MaxAge; ++j) {
            Abundance(i, j) = Abundance(i-1,j-1) * std::exp(-(F_FishingMortality(i-1,j-1) + M_NaturalMortality(i-1,j-1)));
        } // end for j
    } // end for i

    // Have to fill in the upper right corner of the matrix...all the earlier cohorts.
    for (int i = 0; i < NYears; ++i) {
        for (int j = tmpNCatchAge; j <= MaxAge; ++j) {
            if (Abundance(i,j) == 0) {
                Abundance(i,j) = Abundance(i,j-1) * std::exp(-(F_FishingMortality(i,j-1) + M_NaturalMortality(i,j-1)));
            } // end if
        } // end for j
    } // end for i

    // Last have to solve for earliest age class if FirstCatchAge <> 0
    for (int i = 0; i < NYears-1; ++i) {
        for (int j = 0; j < FirstCatchAge; ++j) {
            Abundance(i,FirstCatchAge-1-j) = Abundance(i+1,FirstCatchAge-j) / std::exp(-M_NaturalMortality(i,j));
        } // end for j
    } // end for i

    // Finally..in the last year, cohorts with no catch will have no information so...
    for (int i = 0; i < FirstCatchAge; ++i) {
        Abundance(NYears-1,i) = Abundance(NYears-2,i);
    } // end for i

    // This is a complete matrix....yea...returns F, M, and Abundance from 0 to MaxAge
    //nmfUtils::printMatrix("Abundance",Abundance,9,4);
    //nmfUtils::printMatrix("F_FishingMortality",F_FishingMortality,9,4); // this is all zeros
    //nmfUtils::printMatrix("M_NaturalMortality",M_NaturalMortality,9,4);

    // Pack output args list
    argsOut.CV_CoeffVariation  = CV;
    argsOut.SE_StandardError   = SE;
    argsOut.NumIterations      = NIt;
    argsOut.Abundance          = Abundance;
    argsOut.F_FishingMortality = F_FishingMortality;

} // end CandSVPA




void nmfSSVPA::XSA(
        const XSAInputArgs &inputArgs,
        XSAOutputArgs      &outputArgs)
{
    const int MaxNIndex = 50;
    double Range;
    int Order;
    std::string queryStr;
    std::vector<std::string> fields;
    std::map<std::string, std::vector<std::string> > dataMap;
    boost::numeric::ublas::vector<double> YearWeights;
    boost::numeric::ublas::vector<double> Alpha;
    boost::numeric::ublas::vector<double> Beta;
    boost::numeric::ublas::matrix<double> Catch;
    boost::numeric::ublas::matrix<double> lclF;
    boost::numeric::ublas::matrix<double> lclN;
    boost::numeric::ublas::matrix<double> lclM;
    boost::numeric::ublas::matrix<double> lclZ;
    boost::numeric::ublas::matrix<double> tmpF;
    boost::numeric::ublas::matrix<double> tmpN;
    boost::numeric::ublas::matrix<double> tmpM;
    boost::numeric::ublas::matrix<double> tmpCatch;
    boost::numeric::ublas::matrix<double> CohortN;
    boost::numeric::ublas::vector<double> CohortF;
    boost::numeric::ublas::vector<double> CohortZ;
    boost::numeric::ublas::vector<double> ECZ;
    boost::numeric::ublas::vector<double> ECF;
    boost::numeric::ublas::vector<double> PtShrAge;
    boost::numeric::ublas::vector<double> PtShrYear;

    // Unpack input argument list
    nmfDatabase* databasePtr                     = inputArgs.databasePtr;
    std::map<std::string,std::string> XSAGuiData = inputArgs.XSAGuiData;
    int FirstYear                                = inputArgs.FirstYear;
    int LastYear                                 = inputArgs.LastYear;
    int SpeIndex                                 = inputArgs.SpeIndex;
    std::string SSVPAName                        = inputArgs.SSVPAName;
    MatrixDouble M_NaturalMortality              = inputArgs.M_NaturalMortality;
    MatrixDouble Abundance                       = outputArgs.Abundance;
    MatrixDouble F_FishingMortality              = outputArgs.F_FishingMortality;
    int Warn                                     = outputArgs.XSAWarn;

    double TotZ;
    double TotF;
    int Row;
    int Age;
    double LogPtTop;
    double LogPtBottom;
    double WtPrime;
    double PtShrValue;
    double termF;
    int RefYear;
    int MissingWt;
    int MaxAge;
    int FirstCatchAge;
    int LastCatchAge;
    int isPlusClass;
    int ConvergeCount=0;
    int Nits = 0;
    MatrixDouble LogR; //reciprical cathcability by age, fleet
    MatrixDouble VarR; //variance of logRs

    // RSK continue with implementing nmfXSA; create functions to get required matrices of the model (i.e. Catch)
    // and also required variables (i.e., MaxAge, FirstCatchAge)
    std::tie(MaxAge, FirstCatchAge, LastCatchAge, isPlusClass) =
            databasePtr->nmfQueryAgeFields(nmfConstantsMSVPA::TableSpecies,SpeIndex);
//std::cout << MaxAge << "," << FirstCatchAge << "," << LastCatchAge << std::endl;

    int NYears = LastYear - FirstYear + 1;
    int NCatAge = LastCatchAge - FirstCatchAge + 1;
    int tmpNCatchAge = (isPlusClass) ? NCatAge-1 : NCatAge;

    nmfUtils::initialize(Catch,    NYears, NCatAge);
    nmfUtils::initialize(lclF,     NYears, NCatAge);
    nmfUtils::initialize(lclN,     NYears, NCatAge);
    nmfUtils::initialize(lclM,     NYears, NCatAge);
    nmfUtils::initialize(lclZ,     NYears, NCatAge);
    nmfUtils::initialize(tmpCatch, NYears, NCatAge);
    nmfUtils::initialize(tmpF,     NYears, NCatAge);
    nmfUtils::initialize(tmpN,     NYears, NCatAge);
    nmfUtils::initialize(tmpM,     NYears, NCatAge);
    databasePtr->nmfQueryCatchFields(nmfConstantsMSVPA::TableSpeCatch,
                                     SpeIndex,
                                     std::to_string(FirstCatchAge),
                                     std::to_string(LastCatchAge),
                                     " Order By Year, Age",
                                     Catch);

    // Reference M into catch age classes..0 to MaxAge
    for (int i = 0; i < NYears; ++i) {
        for (int j = 0; j < tmpNCatchAge; ++j) {
            lclM(i,j) = M_NaturalMortality(i, FirstCatchAge + j);
        }
    }    

    fields = {"NXSAIndex","Shrink","DownWeight","DownWeightType","DownWeightYear",
              "ShrinkYears","ShrinkAge","ShrinkCV"};
    queryStr = "SELECT NXSAIndex,Shrink,DownWeight,DownWeightType,DownWeightYear,ShrinkYears,ShrinkAge,ShrinkCV FROM " +
                nmfConstantsMSVPA::TableSpeSSVPA +
               " WHERE SpeIndex = " + std::to_string(SpeIndex) +
               " AND SSVPAName = '" + SSVPAName + "'";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    int NIndex = std::stoi(dataMap["NXSAIndex"][0]);
    int Shrink = std::stoi(dataMap["Shrink"][0]);

    /*
    if (dataMap["NXSAIndex"].size() == 0 || dataMap["Shrink"].size() == 0 ||
        dataMap["NXSAIndex"].empty()     || dataMap["Shrink"].empty())
    {
        nmfUtils::printError("nmfXSA: Table SpeSSVPA is incomplete.", "");
        return;
    }
    int NIndex=0; // 1; // NIndex starts with 1, not 0
    int Shrink=0;
    try {
        NIndex = std::stoi(dataMap["NXSAIndex"][0]);
    } catch (const std::exception &e) {
        NIndex = 0;
    }
    try {
        Shrink = std::stoi(dataMap["Shrink"][0]);
    } catch (const std::exception &e) {
        Shrink = 0;
    }
*/
    int DownWeightType = 0;
    int DownWeightYear = 0;
    int NShrinkYr      = 0;
    int NShrinkAge     = 0;
    double PtShrCV     = 0.0;

    // Use the Abundance index that the user set in the GUI
    //NIndex = (XSAIndex != NIndex) ? XSAIndex : NIndex;
    int DownWeight = (std::stoi(dataMap["DownWeight"][0]) != 0);
    if (! XSAGuiData.empty()) {
        DownWeight = std::stoi(XSAGuiData.at("Downweight"));
        NIndex     = std::stoi(XSAGuiData.at("NXSAIndex")) + 1; // Since NIndex starts with 1 and not 0
        Shrink     = std::stoi(XSAGuiData.at("Shrink"));
    }

    if (DownWeight) {
        DownWeightType = std::stoi(dataMap["DownWeightType"][0]);
        DownWeightYear = std::stoi(dataMap["DownWeightYear"][0]);
        if (DownWeightYear < FirstYear)
            DownWeightYear = FirstYear;
    }
    // Check for any passed GUI data
    if (! XSAGuiData.empty()) {
        DownWeightYear = std::stoi(XSAGuiData.at("DownweightYear"));
        DownWeightType = std::stoi(XSAGuiData.at("DownweightType"));
    }

    if (Shrink) {
        NShrinkYr  = std::stoi(dataMap["ShrinkYears"][0]);
        NShrinkAge = std::stoi(dataMap["ShrinkAge"][0]);
        PtShrCV    = std::stod(dataMap["ShrinkCV"][0]);
        if (! XSAGuiData.empty()) {
            NShrinkYr  = std::stoi(XSAGuiData.at("ShrinkYears"));
            NShrinkAge = std::stoi(XSAGuiData.at("ShrinkAge"));
            PtShrCV    = std::stod(XSAGuiData.at("ShrinkCV"));
        }
    }

    nmfUtils::initialize(YearWeights,  NYears);

    if (! DownWeight) {
        for (int j=0; j<NYears; ++j) {
            YearWeights(j) = 1.0;
        }
    } else {
        Range = LastYear - DownWeightYear;
        Order = DownWeightType + 1;
        for (int j=0; j<NYears; ++j) {
            if ((FirstYear + j) < DownWeightYear)
                YearWeights(j) = 0.0;
            else
                //YearWeights(i) = (1 - ((LastYear - (FirstYear + i)) / Range) ^ Order) ^ Order
                //YearWeights(j) = std::pow(1-std::pow((LastYear-(FirstYear+j))/Range,Order),Order);

            YearWeights(j) = std::pow(1.0-std::pow((LastYear-(FirstYear+j))/Range,Order),Order);

        }
    } // end if
//nmfUtils::printVector("YearWeights", YearWeights);

    Boost3DArrayDouble A(boost::extents[NIndex][NYears][NCatAge]); // Correction for sampling period
    Boost3DArrayDouble Uprime(boost::extents[NIndex][NYears][NCatAge]); // CPUE/A for each index
    Boost3DArrayDouble IndexValues(boost::extents[NIndex][NYears][NCatAge]);
    Boost3DArrayDouble Nest(boost::extents[NIndex][NYears][NCatAge]); // estimated population sizes based on calculated rs

    nmfUtils::initialize(Alpha, MaxNIndex);
    nmfUtils::initialize(Beta,  MaxNIndex);

    databasePtr->nmfQueryAlphaBetaFields(nmfConstantsMSVPA::TableSpeXSAIndices,SpeIndex,
                                         SSVPAName, NIndex, Alpha, Beta);
    if (Alpha.size() == 0)
        return;
    fields = {"Value"};
    queryStr = "SELECT Value FROM " + nmfConstantsMSVPA::TableSpeXSAData +
               " WHERE SpeIndex = " + std::to_string(SpeIndex) +
               "  AND SSVPAName = '" + SSVPAName +
               "' AND Age >= " + std::to_string(FirstCatchAge) +
               "  AND Age <= " + std::to_string(LastCatchAge) +
               "  ORDER BY `Index`, Year, Age";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["Value"].size() == 0) {
        nmfUtils::printError("nmfXSA: Table SpeXSAData is empty", "");
        return;
    }

    int l=0;
    for (int i=0; i < NIndex; ++i) {
        for (int j=0; j < NYears; ++j) {
            for (int k=0; k < NCatAge; ++k) {
                IndexValues[i][j][k] = std::stod(dataMap["Value"][l++]);
            }
        }
    }

    Warn = 0;
    // Set maximum ages and years for cohorts
    int FirstCohort = -(tmpNCatchAge - 1);
    int LastCohort = NYears - 1;
    int NCohorts = NYears + tmpNCatchAge - 1;

    boost::numeric::ublas::vector<double> Amax;
    boost::numeric::ublas::vector<double> Ymax;
    boost::numeric::ublas::vector<double> Pterm;
    boost::numeric::ublas::vector<double> CohortTermZ;

    nmfUtils::initialize(Amax,        NCohorts);
    nmfUtils::initialize(Ymax,        NCohorts);
    nmfUtils::initialize(Pterm,       NCohorts);
    nmfUtils::initialize(CohortTermZ, NCohorts);
    nmfUtils::initialize(CohortF,     NCatAge+1);
    nmfUtils::initialize(CohortZ,     NCatAge+1);
    nmfUtils::initialize(ECZ,         NCatAge+1);
    nmfUtils::initialize(ECF,         NCatAge+1);
    nmfUtils::initialize(lclF,    NYears+1, NCatAge+1);
    nmfUtils::initialize(lclN,    NYears+1, NCatAge+1);
    nmfUtils::initialize(lclZ,    NYears+1, NCatAge+1);
    nmfUtils::initialize(tmpF,    NYears+1, NCatAge+1);
    nmfUtils::initialize(LogR,    NIndex, NCatAge+1);
    nmfUtils::initialize(VarR,    NIndex, NCatAge+1);
    nmfUtils::initialize(CohortN, NIndex, NCatAge+1);

    // RSK - improve this...
    for (int i = FirstCohort; i <= LastCohort; ++i) {
        if ((tmpNCatchAge - 1) < (NYears - 1 - i))
            Amax(i - FirstCohort) = tmpNCatchAge - 1;
        else
            Amax(i - FirstCohort) = NYears - 1 - i;

        if ((tmpNCatchAge - 1 + i) < (NYears - 1))
            Ymax(i - FirstCohort) = tmpNCatchAge - 1 + i;
        else
            Ymax(i - FirstCohort) = NYears - 1;
    } // end for i

//double pSum=0;
    // Initialize the number of survivors for each cohort
    // Initially set to catch in ymax, amax
    for (int i = 0; i < NCohorts; ++i) {
        Pterm(i) = Catch(Ymax(i), Amax(i));
//pSum += Pterm(i);
        DEBUG_MSG("Initial Pterm: " << std::setprecision(12) << Pterm(i));
    }

    double p1 = 0;
    double p2 = 0;
    double t1 = 0;
    double t2 = 0;
    double t3 = 0;
    double c0 = 0;
    double c1 = 0;
    double c2 = 0;
    double c3 = 0;
    double sumTmpF = 0;
    int NsumTmpF = 0;
    int Nt2;
    while ((ConvergeCount <= 3) && (Nits <= 500))
    {
        DEBUG_MSG("");
        sumTmpF = 0;
        NsumTmpF = 0;
        c0 = 0;
        c1 = 0;
        c2 = 0;
        c3 = 0;
        t1 = 0;
        t2 = 0;
        t3 = 0;
        p1 = 0;
        p2 = 0;
        Nt2 = 0;

        ++Nits;

        double sumPterm = 0.0;
        for (int i=0; i<NCohorts; ++i) {
            //DEBUG_MSG("Pterm(" << i << "): " << Pterm(i));   // the Pterm here is good the first time, but off subsequent times
            sumPterm += Pterm(i);
            DEBUG_MSG("Sum Pterm: " << std::setprecision(12) << sumPterm);
        }
        DEBUG_MSG("Sum Pterm: " << std::setprecision(12) << sumPterm);

        // Beginning of the iterative loop.
        // Given pop size at the END of the terminal year, M, and the catch..solve for termZ and do the VPA
        for (int i=0; i<NCohorts; ++i) {
            // Pterm is off going in



         SolveCohortTermZ( Catch(Ymax(i),Amax(i)), lclM(Ymax(i),Amax(i)), Pterm(i), termF );  // termF must be off coming back
//std::cout << "Pterm(" << i << "): " << Pterm(i) << ", termF: " << termF << std::endl;

//std::cout << i << ",  " << Catch(Ymax(i),Amax(i)) << "," << lclM(Ymax(i),Amax(i)) << "," << Pterm(i)
 //         << "," << termF << std::endl;

// Both the Pterm and the termF are off; but termF is off because Pterm is off.  There must be a problem with the first time
//         Pterm is calculated below.

         CohortTermZ(i) = termF;
c0 += Catch(Ymax(i),Amax(i));
c1 += lclM(Ymax(i),Amax(i));
c2 += Pterm(i);
c3 += termF;
        }

        // RSK - after first time through Pterm(i) and termF all good


        // Call cohort analysis to solve for mortalities and abundances...have termZ values for ALL cohorts
//std::cout << "in lclF(" << 0 << "," << 0 << "): " << lclF(0, 0) << ", CohortTermZ(0): " << CohortTermZ(0) ;

        CohortAnalysis( Catch, lclM, 2, tmpNCatchAge, NYears, 0, lclF, lclN, CohortTermZ);  // RSK - lclF is off coming out, CohortTermZ is off going in

        //std::cout << "; out: " << lclF(0, 0) << ", CohortTermZ(0): " << CohortTermZ(0) << std::endl;

double ddiff=0.0;
//std::cout << std::endl;

        //Do the convergence check...go 4 iterations without changing an F
        bool breakout = false;
        for (int i = 0; i <= NYears-1; ++i) {
            for (int j = 0; j <= tmpNCatchAge-1; ++j) {
                ddiff = lclF(i, j) - tmpF(i, j);
//std::cout << "diff2: " << std::setprecision(10) << ddiff << std::endl;
//std::cout << "lclF(" << i << "," << j << "): " << lclF(i, j) << std::endl;
                if (std::fabs(ddiff) > 0.002) {   // > 0.002
                    ConvergeCount = 0;
                    breakout = true;
                    --ConvergeCount;
                    break;
                } // end if
            } // end for j
            if (breakout)
                break;
        } // end for i
        ++ConvergeCount;

//int j;
        // Hold newly calculated Fs in tmpF to check for the next loop
        for (int i = 0; i < NYears; ++i) {
            for (int j = 0; j < tmpNCatchAge; ++j) {
                tmpF(i,j) = lclF(i,j);                              // lclF is off here
sumTmpF += tmpF(i,j);
++NsumTmpF;
            } // end for j
//std::cout << "--> lclF(" << i << ", " << j-1 << "): " << lclF(i,j-1) << std::endl;  // Fshrink is off
        } // end for i

        // Calculate shrinkage mean Fs using Eqns 17-20 in Darby&Flatman
        double Fshrink;
        int FCount;

        nmfUtils::initialize(PtShrYear, NYears);
        nmfUtils::initialize(PtShrAge,  tmpNCatchAge);

        if (Shrink) {

            for (int i = 0; i < NYears; ++i) { // Every year except the last year
                FCount = 0;
                Fshrink = 0;
                for (int j = 0; j < NShrinkAge; ++j) {
                    Fshrink += tmpF(i, tmpNCatchAge - 2 - j);
                    ++FCount;
                } // end for j
                Fshrink /= FCount;
                PtShrYear(i) = (Catch(i, tmpNCatchAge - 1) * (Fshrink + lclM(i, tmpNCatchAge - 1)))
                             / double((std::exp(Fshrink + lclM(i, tmpNCatchAge - 1)) - 1) * Fshrink);
            } // end for i

            for (int i = 0; i < tmpNCatchAge-1; ++i) { // References every age except the last true age
                FCount = 0;
                Fshrink = 0;
                for (int j = 0; j < NShrinkYr; ++j) {
                    Fshrink += tmpF(NYears - j - 2, i);
                    ++FCount;
++Nt2;
                }
//std::cout << "i,Fshrink: " << i << ", " << Fshrink << std::endl;  // Fshrink is off

                Fshrink /= FCount;
                PtShrAge(i) = (Catch(NYears - 1, i)	* (Fshrink + lclM(NYears - 1, i)))
                            / double((std::exp(Fshrink + lclM(NYears - 1, i)) - 1) * Fshrink);
//std::cout << "Fshrink,PtShrAge(" << i << "): " << Fshrink << "," << PtShrAge(i) << std::endl; // Looks like Fshrink is off here

t1 += Catch(NYears - 1, i);
t2 += Fshrink;
t3 += lclM(NYears - 1, i);
            } // end for i

        }
        // End calculation of shrinkage

        for (int i = 0; i < NYears; ++i)
            for (int j = 0; j < tmpNCatchAge; ++j)
                lclZ(i, j) = lclM(i, j) + lclF(i, j);


        // Calculate A and Uprime by index, year, age
        for (int i = 0; i < NIndex; ++i) {
            for (int j = 0; j < NYears; ++j) {
                for (int k = 0; k < tmpNCatchAge; ++k) {
                    if (IndexValues[i][j][k] != 0) {
                        A[i][j][k] = (std::exp(-Alpha(i) * lclZ(j, k)) - std::exp(-Beta(i) * lclZ(j, k)))
                                / double((Beta(i) - Alpha(i)) * lclZ(j, k));
                        Uprime[i][j][k] = IndexValues[i][j][k] / double(A[i][j][k]);
                    } // end if
                } // end for k
            } // end for j
        } // end for i

        int YrCount;
        double YrWeight;


//       Not necessary as the initialize sets all members to 0
       for (int i=0; i<NIndex; ++i) {
           for (int k=0; k<tmpNCatchAge; ++k) {
               LogR(i, k) = 0;
           }
       }

        // Calculate log reciprocal catchabilities and incorporate downweights here.
        // If selected, then the average will actually be a weighted average more heavily wieghted to recent years.

        for (int i = 0; i < NIndex; ++i) {
            for (int k = 0; k < tmpNCatchAge; ++k) {
                YrWeight = 0;
                YrCount = 0;
                for (int j = 0; j < NYears; ++j) {
                    if (IndexValues[i][j][k] > 0) { // this gets rid of zero values..effectively weights them away
                        LogR(i, k) += (YearWeights(j) * std::log(lclN(j, k)	/ double(Uprime[i][j][k])));
                        YrWeight += YearWeights(j);
                        ++YrCount;
                    } // end if
                } // end for j
                LogR(i,k) = (YrCount > 1) ? LogR(i,k)/double(YrWeight) : -9;
            } // end for k
        } // end for i

        // Now need to calculate weighted variance for the estimated population abundances from each index...
        // these are the weights applied to the indices later. This is eqn 12 Darby&Flatman
        for (int i = 0; i < NIndex; ++i) {
            for (int k = 0; k < tmpNCatchAge; ++k) {
                YrWeight = 0;
                YrCount = 0;
                for (int j = 0; j < NYears; ++j) {
                    if (IndexValues[i][j][k] > 0) {
                        VarR(i, k) += YearWeights(j) * std::pow((std::log(lclN(j,k) / Uprime[i][j][k])-LogR(i,k)),2);
                        YrWeight += YearWeights(j);
                        ++YrCount;
                    }
                }
                VarR(i, k) = (YrCount > 1) ? std::sqrt((double(VarR(i,k))/(YrCount-1.0)) * (1.0+(1.0/YrWeight))) : -9;
            } // end for k
        } // end for i


        // Calculate estimated population sizes at age based on indices
        for (int i = 0; i < NIndex; ++i) {
            for (int j = 0; j < NYears; ++j) {
                for (int k = 0; k < tmpNCatchAge; ++k) {
                    // nest = vpa value if there is no index value
                    Nest[i][j][k] = (IndexValues[i][j][k] != 0) ? Uprime[i][j][k] * std::exp(LogR(i,k)) : lclN(j,k);
                } // end for k
            } // end for j
        } // end for i

        double sumRefYear=0;
        double sumMissingWt=0;
        double sumWtPrime=0;
        double sumLogCohort=0;
        double sumVar=0;
        double sumYear=0;
        double sumEcf=0;
        double sumLogTop = 0;
        double sumLogBot = 0;
        sumPterm = 0;


        for (int i = FirstCohort; i<=LastCohort; ++i)
        {
            // Get population size at age for the cohort from the Nest matrix
            // Get F from lclF, get Z from lclZ
            Row = i + tmpNCatchAge - 1;

            for (int j = 0; j < tmpNCatchAge; ++j) {
                Age = tmpNCatchAge - j - 1;

                if ((Row >= 0) && (Row < NYears)) {
                    for (int k = 0; k < NIndex; ++k) {
                        CohortN(k, Age) = Nest[k][Row][Age];
                    } // end for k
                    CohortF(Age) = lclF(Row, Age);
                    CohortZ(Age) = lclZ(Row, Age);
                } else {
                    for (int k = 0; k < NIndex; ++k) {
                        CohortN(k, Age) = -9;
                    }
                    CohortF(Age) = -9;
                    CohortZ(Age) = -9;
                } // end if Row
                --Row;
            } // end for j

            // Look up the PShrValue
            if (Shrink) { // RSK improve this
                if (Amax(i - FirstCohort) == tmpNCatchAge - 1) {
                    PtShrValue = PtShrYear(Ymax(i - FirstCohort));
                } else {
                    PtShrValue = PtShrAge(Amax(i - FirstCohort));
                }
            } else {
                PtShrValue = 0;
            } // end if

            for (int j = 0; j < tmpNCatchAge; ++j) {
                TotZ = 0;
                TotF = 0;
                for (int k = j; k < tmpNCatchAge; ++k) {
                    if (CohortZ(k) != -9) {
                        TotZ += CohortZ(k);
                        TotF += CohortF(k);
                    }
                } // end k

                ECZ(j) = std::exp(TotZ);
                ECF(j) = std::exp(TotF);
            } // end for j
            LogPtTop = 0;
            LogPtBottom = 0;
            WtPrime = 0;

p1 += PtShrValue;
p2 += PtShrCV;
//std::cout << "PtShrValue,PtShrCV: " << PtShrValue << ", "  << PtShrCV << std::endl;   // OK here first time through

            // Calculate the survivors for the cohort....
            // Need to include the annual taper weight in the below
            // and weight away zero values
            // and include F shrinkage value (PtShrValue, PtShrWeight)
            // Ref eqn 13 in darby&flatman
            for (int j = 0; j < NIndex; ++j) {
                for (int k = 0; k < tmpNCatchAge; ++k) {
                    if (CohortN(j, k) != -9) {
                        RefYear = Ymax(i - FirstCohort) - tmpNCatchAge + k + 1;

                        MissingWt = (IndexValues[j][RefYear][k] == 0) ? 0 : 1;
                        WtPrime   = (VarR(j,k) == 0) ? 0 : (MissingWt*YearWeights(RefYear)) / (VarR(j,k)*ECF(k));
sumRefYear += RefYear;
sumMissingWt += MissingWt;
sumWtPrime += WtPrime;
sumLogCohort += std::log(CohortN(j, k));
sumVar += VarR(j,k);
sumYear += YearWeights(RefYear);
sumEcf += ECF(k);

                        LogPtTop += (WtPrime * (std::log(CohortN(j, k)) - std::log(ECZ(k))));
                        LogPtBottom += WtPrime;

                        if (Shrink) {
                            LogPtTop += (std::log(PtShrValue)/PtShrCV);
                            LogPtBottom += (1.0/PtShrCV);
                        } // end if Shrink
sumLogTop += LogPtTop;
sumLogBot += LogPtBottom;
                    } // end if CohortN
                }
            } // end for j


             // All good here except for YearWeights!!!!
            if (LogPtBottom > 0) {

//std::cout << "top,bot: " << LogPtTop << " / "  << LogPtBottom << std::endl;

                Pterm(i + tmpNCatchAge - 1) = std::exp(LogPtTop/LogPtBottom);
sumPterm += Pterm(i+tmpNCatchAge-1);
            }
            // This isn't a condition that was flagged in original code.
            //else {
            //    std::cout << "Warning: LogPtBottom <= 0. Pterm(" << std::to_string(i+tmpNCatchAge-1) <<
            //                 ") assignment skipped." << std::endl;
            //}
        } // end for (int i = FirstCohort; i<=LastCohort; ++i)
//std::cout << "sums: " << sumPterm << ", "  << sumLogTop << ", "  << sumLogBot << ", "  << sumYear << ", "  << sumEcf << std::endl;

DEBUG_MSG("c0,c1,c2,c3: " << std::setprecision(10) << c0 << ", " << std::setprecision(10) << c1 << ", " << std::setprecision(10) << c2 << ", " << std::setprecision(10) << c3);
DEBUG_MSG("NsumTmpF,sumTmpF: " << NsumTmpF << ", " << std::setprecision(10) << sumTmpF);
DEBUG_MSG("t1,(Nt2)t2,t3: " << std::setprecision(10) << t1 << ", (" <<Nt2 << ")" << std::setprecision(10) << t2 << ", " << std::setprecision(10) << t3);
DEBUG_MSG("p1,p2: " << std::setprecision(10) << p1 << ", " << std::setprecision(10) << p2);

    } // end while ((ConvergeCount <= 3) && (Nits <= 500))

DEBUG_MSG("Nits: " << Nits);


        //for (int i=0; i<NCohorts; ++i) {
        //	std::cout << "Pterm(" << i << "): " << Pterm(i) << std::endl;   // the Pterm here is good the first time, but off subsequent times
        //}

double s1=0;
double s2=0;
double s3 = 0;
Warn = (Nits > 500) ? 1 : 0;

    // Fill results into Full F and Abundance matrices
    for (int i = 0; i < NYears; ++i) {
        for (int j = 0; j < tmpNCatchAge; ++j) {
            F_FishingMortality(i, FirstCatchAge + j) = lclF(i, j);
            Abundance(i, FirstCatchAge + j) = lclN(i, j);
s1 += Abundance(i,FirstCatchAge+j);

        } // end for j
    } // end for i

    // Fill Fs across remaining age classes
    for (int i = 0; i < NYears; ++i) {
        for (int j = (FirstCatchAge + tmpNCatchAge); j <= MaxAge; ++j) {
            F_FishingMortality(i, j) = F_FishingMortality(i, FirstCatchAge + tmpNCatchAge - 1);
s2 += F_FishingMortality(i, j);
        } // end for j
    } // end for i

    // Project abundances to the end of the matrix
    int j;
    double Ftemp;
    double Mtemp;
    if (isPlusClass) {
        j = MaxAge - 1;
        for (int i = 0; i < NYears; ++i) {
            Ftemp = F_FishingMortality(i, j);
            Mtemp = M_NaturalMortality(i, j);
            Abundance(i, j + 1) = Catch(i, j) / ((Ftemp / (Ftemp + Mtemp)) * std::exp(-(Ftemp + Mtemp)));
        } // end for i
    } // end if isPlusClass

    // Last have to solve for earliest age class if FirstCatchAge <> 0
    for (int i = 0; i < NYears-1; ++i) {
        for (int j = 0; j < FirstCatchAge; ++j) {
            Abundance(i,FirstCatchAge-1-j) = Abundance(i+1,FirstCatchAge-j) / std::exp(-M_NaturalMortality(i, j));
s3 += Abundance(i,FirstCatchAge-1-j);

        } // end for j
    } // end for i

    // Finally..in the last year, cohorts with no catch will have no information so...
    for (int i = 0; i < FirstCatchAge; ++i) {
        Abundance(NYears-1,i) = Abundance(NYears-2,i);
    }


    // Pack output (i.e., return) list
    outputArgs.Abundance = Abundance;
    outputArgs.F_FishingMortality = F_FishingMortality;
    outputArgs.XSAWarn = Warn;

} // end XSA


/*
void nmfSSVPA::nmfXSA_OLD(const double PtShrCV,
        const int NIndex, const int Shrink, const int NShrinkAge, const int NShrinkYr,
        const int FirstYear, const int LastYear, const int NYears,
        const int MaxAge, const int isPlusClass,
        const int FirstCatchAge, const int LastCatchAge, const int NCatAge,
        const int tmpNCatchAge,
        const int SpeIndex, const int SSVPAindex, const std::string SSVPAname,
        boost::numeric::ublas::matrix<double> &Abundance,
        const boost::numeric::ublas::matrix<double> &M_NaturalMortality,            // this is tmpM coming in!!
        boost::numeric::ublas::matrix<double> &F_FishingMortality,
        int Warn,
        const boost::numeric::ublas::matrix<double> &Catch,
        const boost::numeric::ublas::matrix<double> &lclM,
        const boost::numeric::ublas::vector<double> &Alpha,
        const boost::numeric::ublas::vector<double> &Beta,
        const boost::numeric::ublas::vector<double> &YearWeights)
{
    int Nits=0;
    int ConvergeCount = 0;
    double termF;

    boost::numeric::ublas::vector<int> Amax;
    boost::numeric::ublas::vector<int> Ymax;
    boost::numeric::ublas::vector<double> Pterm;
    boost::numeric::ublas::vector<double> CohortTermZ;
    boost::numeric::ublas::matrix<double> lclF;
    boost::numeric::ublas::matrix<double> lclN;
    boost::numeric::ublas::matrix<double> lclZ;
    boost::numeric::ublas::matrix<double> tmpF;
    boost::numeric::ublas::vector<double> PtShrYear;
    boost::numeric::ublas::vector<double> PtShrAge;

    boost::numeric::ublas::matrix<double> CohortN;
    boost::numeric::ublas::vector<double> CohortF;
    boost::numeric::ublas::vector<double> CohortZ;
    boost::numeric::ublas::vector<double> ECZ;
    boost::numeric::ublas::vector<double> ECF;

    double TotZ;
    double TotF;
    int Row;
    int Age;
    double LogPtTop;
    double LogPtBottom;
    double WtPrime;
    double PtShrValue;
    int RefYear;
    int MissingWt;

    boost::numeric::ublas::matrix<double> LogR; //reciprical cathcability by age, fleet
    boost::numeric::ublas::matrix<double> VarR; //variance of logRs
    Boost3DArrayDouble A(boost::extents[NIndex][NYears][NCatAge]); // Correction for sampling period
    Boost3DArrayDouble Uprime(boost::extents[NIndex][NYears][NCatAge]); // CPUE/A for each index
    Boost3DArrayDouble IndexValues(boost::extents[NIndex][NYears][NCatAge]);
    Boost3DArrayDouble Nest(boost::extents[NIndex][NYears][NCatAge]); // estimated population sizes based on calculated rs


    // Set maximum ages and years for cohorts
    int FirstCohort = -(tmpNCatchAge - 1);
    int LastCohort = NYears - 1;
    int NCohorts = NYears + tmpNCatchAge - 1;

    nmfUtils::initialize(Amax,  NCohorts);
    nmfUtils::initialize(Ymax,  NCohorts);
    nmfUtils::initialize(Pterm, NCohorts);
    nmfUtils::initialize(CohortTermZ, NCohorts);
    nmfUtils::initialize(CohortF,     NCatAge+1);
    nmfUtils::initialize(CohortZ,     NCatAge+1);
    nmfUtils::initialize(ECZ,  NCatAge+1);
    nmfUtils::initialize(ECF,  NCatAge+1);
    nmfUtils::initialize(lclF, NYears+1, NCatAge+1);
    nmfUtils::initialize(lclN, NYears+1, NCatAge+1);
    nmfUtils::initialize(lclZ, NYears+1, NCatAge+1);
    nmfUtils::initialize(tmpF, NYears+1, NCatAge+1);
    nmfUtils::initialize(LogR,    NIndex, NCatAge+1);
    nmfUtils::initialize(VarR,    NIndex, NCatAge+1);
    nmfUtils::initialize(CohortN, NIndex, NCatAge+1);

    // RSK - improve this...
    for (int i = FirstCohort; i <= LastCohort; ++i) {
        if ((tmpNCatchAge - 1) < (NYears - 1 - i))
            Amax(i - FirstCohort) = tmpNCatchAge - 1;
        else
            Amax(i - FirstCohort) = NYears - 1 - i;

        if ((tmpNCatchAge - 1 + i) < (NYears - 1))
            Ymax(i - FirstCohort) = tmpNCatchAge - 1 + i;
        else
            Ymax(i - FirstCohort) = NYears - 1;
    } // end for i


    // Initialize the number of survivors for each cohort
    // Initially set to catch in ymax, amax
    for (int i = 0; i < NCohorts; ++i) {
        Pterm(i) = Catch(Ymax(i), Amax(i));
    }

    while ((ConvergeCount <= 3) && (Nits <= 500)) {
        ++Nits;
//std::cout << "Nits: " << Nits << std::endl;

//        std::cout << std::endl;
 //       for (int i=0; i<NCohorts; ++i) {
  //      	std::cout << "Pterm(" << i << "): " << Pterm(i) << std::endl;   // the Pterm here is good the first time, but off subsequent times
   //     }

//
//  RSK I think the error is around here......
//

        // Beginning of the iterative loop.
        // Given pop size at the END of the terminal year, M, and the catch..solve for termZ and do the VPA
        for (int i=0; i<NCohorts; ++i) {
            // Pterm is off going in
         SolveCohortTermZ( Catch(Ymax(i),Amax(i)), lclM(Ymax(i),Amax(i)), Pterm(i), termF );  // termF must be off coming back
//std::cout << "Pterm(" << i << "): " << Pterm(i) << ", termF: " << termF << std::endl;

         CohortTermZ(i) = termF;
        }

        // RSK - after first time through Pterm(i) and termF all good


        // Call cohort analysis to solve for mortalities and abundances...have termZ values for ALL cohorts
//std::cout << "in lclF,CohortTermZ: " << lclF(0, 0) << ",  " << CohortTermZ(0) ;
        CohortAnalysis( Catch, lclM, 2, tmpNCatchAge, NYears, 0, lclF, lclN, CohortTermZ);  // RSK - lclF is off coming out,
//std::cout << "  " << lclF(0, 0) << ", " << CohortTermZ(0) << std::endl;


//std::cout << std::endl;

        //Do the convergence check...go 4 iterations without changing an F
        bool breakout = false;
        for (int i = 0; i <= NYears - 1; ++i) {
            for (int j = 0; j <= tmpNCatchAge - 1; ++j) {
//std::cout << "diff2: " << double(lclF(i, j) - tmpF(i, j)) << std::endl;
//std::cout << "lclF(" << i << "," << j << "): " << lclF(i, j) << std::endl;
                if (std::fabs(lclF(i, j) - tmpF(i, j)) > 0.002) {
                    ConvergeCount = 0;
                    breakout = true;
                    --ConvergeCount;
                    break;
                } // end if
            } // end for j
            if (breakout)
                break;
        } // end for i
        ++ConvergeCount;

//int j;
        // Hold newly calculated Fs in tmpF to check for the next loop
        for (int i = 0; i < NYears; ++i) {
            for (int j = 0; j < tmpNCatchAge; ++j) {
                tmpF(i,j) = lclF(i,j);                              // lclF is off here
            } // end for j
//std::cout << "--> lclF(" << i << ", " << j-1 << "): " << lclF(i,j-1) << std::endl;  // Fshrink is off
        } // end for i

        // Calculate shrinkage mean Fs using Eqns 17-20 in Darby&Flatman
        double Fshrink;
        int FCount;

        nmfUtils::initialize(PtShrYear, NYears);
        nmfUtils::initialize(PtShrAge,  tmpNCatchAge);

        if (Shrink) {

            for (int i = 0; i < NYears; ++i) { // Every year except the last year
                FCount = 0;
                Fshrink = 0;
                for (int j = 0; j < NShrinkAge; ++j) {
                    Fshrink += tmpF(i, tmpNCatchAge - 2 - j);
                    ++FCount;
                } // end for j
                Fshrink /= FCount;
                PtShrYear(i) = (Catch(i, tmpNCatchAge - 1) * (Fshrink + lclM(i, tmpNCatchAge - 1)))
                             / ((std::exp(Fshrink + lclM(i, tmpNCatchAge - 1)) - 1) * Fshrink);
            } // end for i

            for (int i = 0; i < tmpNCatchAge-1; ++i) { // References every age except the last true age
                FCount = 0;
                Fshrink = 0;
                for (int j = 0; j < NShrinkYr; ++j) {
                    Fshrink += tmpF(NYears - j - 2, i);
                    ++FCount;
                }
//std::cout << "i,Fshrink: " << i << ", " << Fshrink << std::endl;  // Fshrink is off

                Fshrink /= FCount;
                PtShrAge(i) = (Catch(NYears - 1, i)	* (Fshrink + lclM(NYears - 1, i)))
                            / ((std::exp(Fshrink + lclM(NYears - 1, i)) - 1) * Fshrink);
//std::cout << "Fshrink,PtShrAge(" << i << "): " << Fshrink << "," << PtShrAge(i) << std::endl; // Looks like Fshrink is off here

            } // end for i

        }
        // End calculation of shrinkage

        for (int i = 0; i < NYears; ++i)
            for (int j = 0; j < tmpNCatchAge; ++j)
                lclZ(i, j) = lclM(i, j) + lclF(i, j);


        // Calculate A and Uprime by index, year, age
        for (int i = 0; i < NIndex; ++i) {
            for (int j = 0; j < NYears; ++j) {
                for (int k = 0; k < tmpNCatchAge; ++k) {
                    if (IndexValues[i][j][k] != 0) {
                        A[i][j][k] = (std::exp(-Alpha(i) * lclZ(j, k)) - std::exp(-Beta(i) * lclZ(j, k)))
                                / ((Beta(i) - Alpha(i)) * lclZ(j, k));
                        Uprime[i][j][k] = IndexValues[i][j][k] / A[i][j][k];
                    } // end if
                } // end for k
            } // end for j
        } // end for i

        int YrCount;
        double YrWeight;


//       Not necessary as the initialize sets all members to 0

       for (int i=0; i<NIndex; ++i) {
           for (int k=0; k<tmpNCatchAge; ++k) {
               LogR(i, k) = 0;
           }
       }

        // Calculate log reciprocal catchabilities and incorporate downweights here.
        // If selected, then the average will actually be a weighted average more heavily wieghted to recent years.

        for (int i = 0; i < NIndex; ++i) {
            for (int k = 0; k < tmpNCatchAge; ++k) {
                YrWeight = 0;
                YrCount = 0;
                for (int j = 0; j < NYears; ++j) {
                    if (IndexValues[i][j][k] > 0) { // this gets rid of zero values..effectively weights them away
                        LogR(i, k) += (YearWeights(j) * std::log(lclN(j, k)	/ Uprime[i][j][k]));
                        YrWeight += YearWeights(j);
                        ++YrCount;
                    } // end if
                } // end for j
                LogR(i,k) = (YrCount > 1) ? LogR(i,k)/YrWeight : -9;
            } // end for k
        } // end for i

        // Now need to calculate weighted variance for the estimated population abundances from each index...
        // these are the weights applied to the indices later. This is eqn 12 Darby&Flatman
        for (int i = 0; i < NIndex; ++i) {
            for (int k = 0; k < tmpNCatchAge; ++k) {
                YrWeight = 0;
                YrCount = 0;
                for (int j = 0; j < NYears; ++j) {
                    if (IndexValues[i][j][k] > 0) {
                        VarR(i, k) += YearWeights(j) * std::pow((std::log(lclN(j,k) / Uprime[i][j][k])-LogR(i,k)),2);
                        YrWeight += YearWeights(j);
                        ++YrCount;
                    }
                }
                VarR(i, k) = (YrCount > 1) ? std::sqrt((VarR(i,k)/(YrCount-1)) * (1+(1/YrWeight))) : -9;
            } // end for k
        } // end for i


        // Calculate estimated population sizes at age based on indices
        for (int i = 0; i < NIndex; ++i) {
            for (int j = 0; j < NYears; ++j) {
                for (int k = 0; k < tmpNCatchAge; ++k) {
                    // nest = vpa value if there is no index value
                    Nest[i][j][k] = (IndexValues[i][j][k] != 0) ? Uprime[i][j][k] * std::exp(LogR(i,k)) : lclN(j,k);
                } // end for k
            } // end for j
        } // end for i

        for (int i = FirstCohort; i<=LastCohort; ++i)
        {
            // Get population size at age for the cohort from the Nest matrix
            // Get F from lclF, get Z from lclZ
            Row = i + tmpNCatchAge - 1;
            for (int j = 0; j < tmpNCatchAge; ++j) {
                Age = tmpNCatchAge - j - 1;
                if ((Row >= 0) && (Row < NYears)) {
                    for (int k = 0; k < NIndex; ++k) {
                        CohortN(k, Age) = Nest[k][Row][Age];
                    } // end for k
                    CohortF(Age) = lclF(Row, Age);
                    CohortZ(Age) = lclZ(Row, Age);
                } else {
                    for (int k = 0; k < NIndex; ++k) {
                        CohortN(k, Age) = -9;
                    }
                    CohortF(Age) = -9;
                    CohortZ(Age) = -9;
                } // end if Row
                --Row;
            } // end for j

            // Look up the PShrValue
            if (Shrink) { // RSK improve this
                if (Amax(i - FirstCohort) == tmpNCatchAge - 1) {
                    PtShrValue = PtShrYear(Ymax(i - FirstCohort));

                } else {
                    PtShrValue = PtShrAge(Amax(i - FirstCohort));

                }

            } else {
                PtShrValue = 0;
            } // end if

            for (int j = 0; j < tmpNCatchAge; ++j) {
                TotZ = 0;
                TotF = 0;
                for (int k = j; k < tmpNCatchAge; ++k) {
                    if (CohortZ(k) != -9) {
                        TotZ += CohortZ(k);
                        TotF += CohortF(k);
                    }
                } // end k

                ECZ(j) = std::exp(TotZ);
                ECF(j) = std::exp(TotF);
            } // end for j

            LogPtTop = 0;
            LogPtBottom = 0;
            WtPrime = 0;

//std::cout << "PtShrValue,PtShrCV: " << PtShrValue << ", "  << PtShrCV << std::endl;


            // Calculate the survivors for the cohort....
            // Need to include the annual taper weight in the below
            // and weight away zero values
            // and include F shrinkage value (PtShrValue, PtShrWeight)
            // Ref eqn 13 in darby&flatman

            for (int j = 0; j < NIndex; ++j) {
                for (int k = 0; k < tmpNCatchAge; ++k) {
                    if (CohortN(j, k) != -9) {
                        RefYear = Ymax(i - FirstCohort) - tmpNCatchAge + k + 1;

                        MissingWt = (IndexValues[j][RefYear][k] == 0) ? 0 : 1;
                        WtPrime   = (VarR(j,k) == 0) ? 0 : (MissingWt*YearWeights(RefYear)) / (VarR(j,k)*ECF(k));

                        LogPtTop += (WtPrime * (std::log(CohortN(j, k)) - std::log(ECZ(k))));
                        LogPtBottom += WtPrime;

                        if (Shrink) {
                            LogPtTop += (std::log(PtShrValue)/PtShrCV);
                            LogPtBottom += (1.0/PtShrCV);
                        } // end if Shrink

                    } // end if CohortN
                }
            } // end for j

            if (LogPtBottom > 0) {

//std::cout << "top,bot: " << LogPtTop << ", "  << LogPtBottom << std::endl;

                Pterm(i + tmpNCatchAge - 1) = std::exp(LogPtTop/LogPtBottom);
            }
        } // end for i


    } // end while

std::cout << "Nits: " << Nits << std::endl;

    Warn = (Nits > 500) ? 1 : 0;

    // Fill results into Full F and Abundance matrices
    for (int i = 0; i < NYears; ++i) {
        for (int j = 0; j < tmpNCatchAge; ++j) {
            F_FishingMortality(i, FirstCatchAge + j) = lclF(i, j);
            Abundance(i, FirstCatchAge + j) = lclN(i, j);
        } // end for j
    } // end for i

    // Fill Fs across remaining age classes
    for (int i = 0; i < NYears; ++i) {
        for (int j = (FirstCatchAge + tmpNCatchAge); j <= MaxAge; ++j) {
            F_FishingMortality(i, j) = F_FishingMortality(i, FirstCatchAge + tmpNCatchAge - 1);
        } // end for j
    } // end for i

    // Project abundances to the end of the matrix
    int j;
    if (isPlusClass) {
        j = MaxAge - 1;
        for (int i = 0; i < NYears; ++i) {
            Abundance(i, j + 1) = Catch(i, j) /
                ((F_FishingMortality(i, j) / (F_FishingMortality(i, j) + M_NaturalMortality(i, j)))	*
                        std::exp(-(F_FishingMortality(i, j) + M_NaturalMortality(i, j))));
        } // end for i
    } // end if isPlusClass

    // Last have to solve for earliest age class if FirstCatchAge <> 0
    for (int i = 0; i < NYears-1; ++i) {
        for (int j = 0; j < FirstCatchAge; ++j)
            Abundance(i,FirstCatchAge-1-j) = Abundance(i+1,FirstCatchAge-j) / std::exp(-M_NaturalMortality(i, j));
    } // end for i

    // Finally..in the last year, cohorts with no catch will have no information so...
    for (int i = 0; i < FirstCatchAge; ++i) {
        Abundance(NYears-1,i) = Abundance(NYears-2,i);
    }

//nmfUtils::printVector("CohortTermZ", CohortTermZ);


} // end nmfXSA_OLD
*/

void nmfSSVPA::CohortAnalysis(
    const boost::numeric::ublas::matrix<double> &Catch,
    const boost::numeric::ublas::matrix<double> &M,
    const int &TermZType,
    const int &Nage,
    const int &Nyear,
    const int &FullRecAge,
    boost::numeric::ublas::matrix<double> &F_FishingMortality,          // This is off (passed back to lclF which is off
    boost::numeric::ublas::matrix<double> &N,
    const boost::numeric::ublas::vector<double> &initTermZ)  // RSK tbd make this const and move all const to beginning of list
{
    boost::numeric::ublas::vector<double> Cohort;
    boost::numeric::ublas::vector<double> CohortM;
    boost::numeric::ublas::vector<double> CohortF;  // solved for by VPA
    boost::numeric::ublas::vector<double> CohortN;  // solved for by VPA

    double termZ;
    int FirstCohort;
    int LastCohort;
    int Row = 0;
    int Age = 0;
    int LastAge = 0;

    nmfUtils::initialize(Cohort,  Nage+1);
    nmfUtils::initialize(CohortM, Nage+1);
    nmfUtils::initialize(CohortF, Nage+1);
    nmfUtils::initialize(CohortN, Nage+1);

    // Accept catch matrix, m matrix, and other info...develop the cohort vectors for delivery to other subs
    // Loop through and pull out a cohort....
    FirstCohort = -Nage;
    LastCohort  =  Nyear - 2;

//std::cout << "CohortAnalysis -----------------------------------------------------------\n " << std::endl;

    for (int Count=FirstCohort; Count<=LastCohort; ++Count)
    {
        Row = Count + Nage;
        for (int i = 0; i < Nage; ++i) {
            // Initialize cohortF and CohortN vectors
            CohortF(i) = -9;
            CohortN(i) = -9;
            Age = Nage - i - 1;

            if ((Row >= 0) && (Row < Nyear)) {
                Cohort(Age)  = Catch(Row, Age);
                CohortM(Age) = M(Row, Age);
                LastAge = i;
            } else {
                Cohort(Age)  = -9;
                CohortM(Age) = -9;
            }
            --Row;
        } // end for i

        for (int i = 0; i < Nage; ++i) {
            if (Cohort(i) != -9)
                LastAge = i;
        }

        // Ok....so have the cohort vector...call or calculate terminal mortality
        termZ = 0;
        switch (TermZType) {
            case 0:  // log catch ratio
                //std::cout << "Warning: case TermZType=0 not yet implemented. " << std::endl;
                // The following was commented out in the VB6 code.
                /*If Cohort(Age1) > 0 And Cohort(Age2) > 0 Then
                 TermZ = -1 * Log(Cohort(Age2) / Cohort(Age1))
                Else
                 GoTo Barf
                End If
                TermZ = 0.35
                GoTo Skip*/
                CohortVPA(Cohort, CohortM, termZ, Nage, CohortF, CohortN);
                UpdateNandF(Nage,Nyear,Count,N,F_FishingMortality,CohortN,CohortF);
                break;

            case 1: // catchcurve analysis

// RSK - debug this code

            //std::cout << "Warning: case TermZType=1 not fully implemented. Check logic. " << std::endl;
                CatchCurve(Cohort, termZ, FullRecAge, Nage);
                if (termZ != 0) {
                    CohortVPA(Cohort, CohortM, termZ, Nage, CohortF, CohortN);
                }
                UpdateNandF(Nage,Nyear,Count,N,F_FishingMortality,CohortN,CohortF);
                 // The following was commented out in the VB6 code.
                 /*If termZ = 0 Then GoTo Barf
                  'A bug fix for something funky that happens in the MSVPA with very large m2 values
                  'need to Elevate termz so you dont get negative Fs...
                  'on another note..since the catch curve assumes constant mortality after rec age
                  'does it really make sense for the MSVPA loops ??
                  'Debug.Print Count; termZ; CohortM(LastAge)
                  'If termZ <= CohortM(LastAge) Then
                  ' termZ = termZ + CohortM(LastAge)
                  'End If
                 */
                break;

            case 2:  // special case of effort tuned VPA with f at oldest age
                // TermZ is read from InitTermZ Matrix...

                termZ = (initTermZ(Count + Nage) > 0) ? initTermZ(Count+Nage) + CohortM(LastAge) : 0;  //RSK initTermZ is off
                if (termZ != 0) {
                    CohortVPA(Cohort, CohortM, termZ, Nage, CohortF, CohortN);  // RSK continue here debugging.....termZ isn't the same as what's on the laptop
                }
                UpdateNandF(Nage,Nyear,Count,N,F_FishingMortality,CohortN,CohortF);  // if F is off.....CohortF must be off as well.

//std::cout << "---------------Sum of all elements of N is " << nmfUtils::matrixSum(N) << std::endl;

                break;

            case 3: // effort tuned VPA for last year F at age matrix

                termZ = initTermZ(LastAge) + CohortM(LastAge);
                CohortVPA(Cohort, CohortM, termZ, Nage, CohortF, CohortN);
                UpdateNandF(Nage,Nyear,Count,N,F_FishingMortality,CohortN,CohortF);
                break;

        } // end switch
        // Do the VPA for the cohort here after calculating Terminal Z
        // Debug.Print Count; termZ
    } // end for Count


} // end CohortAnalysis


void
nmfSSVPA::UpdateNandF(
        const int &Nage,
        const int &Nyear,
        const int &Count,
        boost::numeric::ublas::matrix<double> &N,
        boost::numeric::ublas::matrix<double> &F,
        const boost::numeric::ublas::vector<double> &CohortN,
        const boost::numeric::ublas::vector<double> &CohortF)
{
//std::cout << "Nage: " << Nage << std::endl;
//std::cout << "Count: " << Count << std::endl;
//std::cout << "N: " << N.size1() << "," << N.size2() << std::endl;
//std::cout << "F: " << F.size1() << "," << F.size2() << std::endl;
//std::cout << "CohortN: " << CohortN.size() << std::endl;
//std::cout << "CohortF: " << CohortF.size() << std::endl;
//double sum=0;
    for (int i = 0; i < Nage; ++i) {
        if (((Count + i + 1) >= 0) && ((Count + i + 1) <= Nyear)) {
            N(Count + i + 1, i) = CohortN(i);
            F(Count + i + 1, i) = CohortF(i);
//sum +=F(Count + i + 1, i);
        }
    } // end for i
//std::cout << "UpdateNandF total: " << sum << std::endl;

    //std::cout << "Done " << std::endl;

} // end UpdateNandF


void nmfSSVPA::CohortVPA(
        const boost::numeric::ublas::vector<double> &Catch,
        const boost::numeric::ublas::vector<double> &M,
        const double &termZ,
        const int &Nage,
        boost::numeric::ublas::vector<double> &F_FishingMortality,   // F  is probably off here
        boost::numeric::ublas::vector<double> &Abundance)
{
    bool cont;
    bool Rswitch;
    int VPAMaxAge=0;
    int k=0;
    double Ftemp;
    double Z;
    double E;
    double E2;
    double term;
    boost::numeric::ublas::vector<double> R;
//nmfUtils::printVector("\nAbundance in ", Abundance);
    nmfUtils::initialize(R, Nage);

    // Initialize output matrix
    for (int i = 0; i < Nage; ++i) {
        F_FishingMortality(i) = -9;
        Abundance(i) = -9;
    } // end for i

    // Find the maximum age in the matrix
    for (int i = 0; i < Nage; ++i) {
        if (Catch(i) > 0)
            VPAMaxAge = i;
    } // end for i

    // Calculate Rs for each age class and flag if zeros
    Rswitch = false;
    for (int i = 0; i < VPAMaxAge; ++i) {
        cont = true;
        if (Catch(i) == -9) {
            R(i) = -9;
            cont = false;
        }
        if (cont) {
            if ((Catch(i + 1) == 0)) { // turn on the switch for zero catches for the next iteration
                R(i) = 0;
                Rswitch = true;
            } else { // code for normal calculation - depends on switch
                if (!Rswitch) {
                    R(i) = Catch(i + 1) / Catch(i);
                } else {
                    R(i) = Catch(i + 1) / Catch(i - 1);
                    Rswitch = false;
                }
            } // end if Catch
        }
    } // end for i

    // 2-solve for terms in last age class
    // Get the terminal mortality
    Ftemp = termZ - M(VPAMaxAge);   // RSK termZ is off here
    E = Ftemp * (1 - std::exp(-termZ)) / termZ;
    //term = E * std::exp(termZ); // RSK - this doesn't seem to do anything...

//std::cout << "termZ,M(VPAMaxAge): " << termZ << ", " << M(VPAMaxAge) << std::endl;


    // Put FPluSM2 and N into Output matrix for last age class
    Abundance(VPAMaxAge) = Catch(VPAMaxAge) / E;  // Solve for N
    F_FishingMortality(VPAMaxAge) = Ftemp;

    // 3 - Backward solution for VPA

    for (int j = 1; j <= VPAMaxAge; ++j) {
        cont = true;
        k = VPAMaxAge - j;

        if (Catch(k) == -9) {
            Abundance(k) = -9;
            F_FishingMortality(k) = -9;
            Ftemp = 0;
        } else if (Catch(k) == 0) {
            // Calcuate Abundance
            Abundance(k) = Abundance(k + 1) / std::exp(-M(k));
            // Ftemp is zero
            F_FishingMortality(k) = 0;
        } else {
            if (Catch(k + 1) > 0) { // normal solution
                term = E / R(k);
//std::cout << "E0: " << E << std::endl;
//std::cout << "R(k): " << R(k) << std::endl;

                Ftemp = SolveIt(term, M(k));

                if (Ftemp < 0) {
                    //intresponse = MsgBox("Failure to converge in mortality estimate. Age = " & k)
                    std::cout << "Failure to converge in mortality estimate. Age = " + std::to_string(k) << std::endl;
                    Abundance(k) = -9;
                    F_FishingMortality(k) = -9;
                    cont = false;
                } // end if
                if (cont) {
                    Z = Ftemp + M(k);
                    E = (Ftemp / Z) * (1 - std::exp(-Z)); // this is Ei+1 calculated for check in next round
//std::cout << "Z1: " << Z << std::endl;
//std::cout << "E1: " << E << std::endl;
                    // Solve for N
                    Abundance(k) = Catch(k) / E;
                    // Store Ftemp
                    F_FishingMortality(k) = Ftemp;
                } // end if cont

            } else { // case where Ck+1 = 0
                //E2 = Catch(k + 2) / Abundance(k + 2, 0); // RSK - I think the ", 0" here is a mistake in the original code.
                E2 = Catch(k + 2) / Abundance(k + 2);
                term = (E2 * std::exp(-M(k))) / R(k + 1);
                Ftemp = SolveIt(term, M(k));

                if (Ftemp < 0) {
                    std::cout << "Failure to converge in mortality estimate.  Age = " + std::to_string(k) << std::endl;
                    Abundance(k) = -9;
                    F_FishingMortality(k) = -9;
                    cont = false;
                } // end if
                if (cont) {
                    Z = Ftemp + M(k);
                    E = (Ftemp / Z) * (1 - std::exp(-Z)); // this is Ei+1 calculated for check in next round
//std::cout << "Z2: " << Z << std::endl;
//std::cout << "E2: " << E << std::endl;
                    // Solve for N
                    Abundance(k) = Catch(k) / E;
                    // Store Z
                    F_FishingMortality(k) = Ftemp;
                } // end if cont
            } // end if Catch

        } // end if Catch
//std::cout << "CohortVPA: F_FishingMortality(k): " << F_FishingMortality(k) << std::endl;
    } // end for j



//std::cout << "F_FishingMortality total: " << nmfUtils::vectorSum(F_FishingMortality) << std::endl;
    DEBUG_MSG("---------------Sum of all elements of F is " << sum(F_FishingMortality));

//nmfUtils::printVector("Abundance out ", Abundance);

/*
 * RSK - not sure if this is correct.  However, -9 that are still in the Abundance vector
 * seem to cause problems further along.
 */
/*bool checkForNeg = false;
for (unsigned int fix=0; fix<Abundance.size(); ++fix) {
    if (Abundance[fix] < 0) {
        checkForNeg = true;
        Abundance[fix] = 0;
    }
}
if (checkForNeg) {
    //std::cout << "Warning: Found negative Abundance value. Resetting it to 0." << std::endl;
}*/

} // end CohortVPA



double
nmfSSVPA::SolveIt(const double &term, const double &M) {
    // start at 10
    double Upper = 10.0;
    double Lower = 0.0;
    double tmpTerm = 0.0;
    double tmpVal = 0.0; // place holder
    int numits = 0;
    double retv;


    while ((std::fabs(term-tmpTerm) >= 0.00001) && (numits < 5000)) {
        ++numits;
        tmpTerm = Upper * (std::exp(Upper + M) - 1) / (Upper + M);
        // If the value of tmpFplusM2 is to large then do something to it
        if ((term-tmpTerm) < 0) {
            tmpVal = Upper; // hold the previous upper value
            Upper = (Upper + Lower) / 2; // reduce upper bound and loop
        }

        // If the value of tmpFplusM2 is too small then do something to it
        if ((term-tmpTerm) > 0) {
            Lower = Upper;
            Upper = tmpVal;
        } // end if

    } // end while

    retv = Upper;

    if (numits >= 5000) {
        retv = -1.0;
    }
    //std::cout << "SolveIt: " << retv << ", " << numits << std::endl;

    return retv;
} // end SolveIt


void
nmfSSVPA::CatchCurve(
        const boost::numeric::ublas::vector<double> &Cohort,
        double &termZ,
        const int &RecAge,
        const int &Nage)
{

    int AgeCount = 0;
    double T = 0.0;
    double N = 0.0;
    double S = 0.0;
    boost::numeric::ublas::vector<double> tmpData;
    nmfUtils::initialize(tmpData, Nage);

    /*
     This is code to calculate annual terminal mortality rates from the Chapman & Robson method for analyzing
     catch data.  You do the analysis for all fully recruited age-classes, 2+ in the case of menhaden.
     Assumes that effort/cpue are constant across these ages...probably not the best assumption if there
     is alot of variation in effort across the time period.  So could try to incorporate effort data
     here to provide some standardization..eventually.  First, just going to implement it in a
     straightforward way...calculate MVUE estimate,variance 95%CI of it for each cohort in the catch
     and predator removals data sets.
     */

    // If there are enough ages, then calculate the terms for the survival rate estimate
    for (int i = RecAge; i < Nage; ++i) {
        if (Cohort(i) >= 0) {
            tmpData(AgeCount) = Cohort(i);
            ++AgeCount;
        }
    } // end for i

    if (AgeCount >= 2) {
        for (int j = 0; j < AgeCount; ++j) {

            T += (j*tmpData(j));
            N += tmpData(j);

        } // end for j

        S = T / (N + T - 1.0);
        termZ = -std::log(S);

        // VarMort = S * (S - ((T - 1) / (N + T - 2)))
        /*
         Later include a variance/CI calculation here to deal with uncertainty in term.
         Mortatlity estimate...one problem is that the estimate is dependant upon the
         number of years of data that you have for the species..so early cohorts have
         a different estimate of terminal Z than later cohorts, even for the same data
         how to deal with this ?...I dunno, read something - basically this is why VPA estimates
         change every year that you get new data....oh well
         In the meantime..this gets me to where I need to go, similar to approach by Cadrin
         */

        //Put variance calculation in here and store cohortnum, termz, and variance in the database,
        //let's you call things out later.
    } // end if AgeCount

} // end CatchCurve




void
nmfSSVPA::SolveCohortTermZ(const double &Catch, const double &M, const double &Nend, double &F)
{
    const double Tol = 0.001;
    int NIt = 0;
    double Ref = 0.0;
    double LowF = 0.0;
    double HiF = 0.0;
    double Solve = 0.0;
    double tmpZ = 0.0;
    double tmpVal = 0.0;

    // Set reference value..Catch/Nend
    Ref = Catch / Nend;  // Nend is off

    LowF = 0.001;
    HiF = 5.0;

    // Start Loop
    bool done = false;
    while ((NIt < 1000) && (!done)) {
        tmpZ = HiF + M;

        Solve = (HiF / tmpZ) * (std::exp(tmpZ) - 1.0);

        if (std::fabs(Solve-Ref) >= Tol) {
            if ((Solve-Ref) < Tol) { // F is too low..
                LowF = HiF;
                HiF = tmpVal;
            } else { // F is too high......reduce the HiF value
                tmpVal = HiF;// holds the previous upper value
                HiF = (HiF + LowF) / 2.0; // reduce upper bound
            }
            ++NIt;

        } else {
            done = true;
        }

    } // end while
//std::cout << "NIt,Catch,M,Nend,HiF: " << NIt << ", " << Catch << ", " << M << ", " << Nend << ", " << HiF  << std::endl;

    F = HiF;

} // end SolveCohortTermZ






