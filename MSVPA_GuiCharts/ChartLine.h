#ifndef CHARTLINE_H
#define CHARTLINE_H

#include "MSVPAX2OutputChart.h"

QT_CHARTS_USE_NAMESPACE

class ChartLine : public MSVPAX2OutputChart
{

    Q_OBJECT

public:
    ChartLine();
    virtual ~ChartLine() {}

    static void loadChartWithData(
            QChart *chart,
            double YMaxOverride,
            std::string species,
            boost::numeric::ublas::matrix<double> &ChartData,
            boost::numeric::ublas::vector<std::string> &LegendNames,
            std::string mainTitle,
            std::string xTitle,
            std::string yTitle,
            std::vector<std::string> xLabels,
            bool rotateLabels);

    static double YPR(boost::numeric::ublas::vector<double> &WeightAtAge,
            boost::numeric::ublas::vector<double> &FatAge,
            boost::numeric::ublas::vector<double> &M2atAge,
            double FullF,
            boost::numeric::ublas::vector<double> &M1,
            int Nage);

    static double F01(boost::numeric::ublas::vector<double> &WeightAtAge,
            boost::numeric::ublas::vector<double> &FatAge,
            boost::numeric::ublas::vector<double> &M2atAge,
            boost::numeric::ublas::vector<double> &M1,
            double Nage);

    static double FMax(boost::numeric::ublas::vector<double> &WeightAtAge,
            boost::numeric::ublas::vector<double> &FatAge,
            boost::numeric::ublas::vector<double> &M2atAge,
            boost::numeric::ublas::vector<double> &M1,
            double Nage);

    static double SSB(boost::numeric::ublas::vector<double> &WeightAtAge,
            boost::numeric::ublas::vector<double> &FatAge,
            boost::numeric::ublas::vector<double> &M2atAge,
            double FullF,
            boost::numeric::ublas::vector<double> &M1,
            int &Nage,
            boost::numeric::ublas::vector<double> &Pmature);

    static double SSBBench(boost::numeric::ublas::vector<double> &WeightAtAge,
            boost::numeric::ublas::vector<double> &FatAge,
            boost::numeric::ublas::vector<double> &M2atAge,
            boost::numeric::ublas::vector<double> &M1,
            int &Nage,
            boost::numeric::ublas::vector<double> &Pmature,
            double BenchVal);

    static void calculateWeightAveFAndAssignPRFs(
            const int &NumYears,
            const int &RecAge,
            const int &Nage,
            boost::numeric::ublas::matrix<double> &FatAge,
            boost::numeric::ublas::matrix<double> &Catch,
            boost::numeric::ublas::vector<double> &FullF,
            boost::numeric::ublas::matrix<double> &PRF);

    static std::string getYAxisUnits(
            nmfDatabase* databasePtr,
            std::string selectedSpecies);

public slots:
    void callback_UpdateChart(nmfStructsQt::UpdateDataStruct data);

};

#endif // CHARTLINE_H
