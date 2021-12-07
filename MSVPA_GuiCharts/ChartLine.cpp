#include "ChartLine.h"

#include "nmfConstants.h"

ChartLine::ChartLine()
{

}

void
ChartLine::callback_UpdateChart(nmfStructsQt::UpdateDataStruct data)
{
  std::cout << "TBD" << std::endl;
}



void
ChartLine::loadChartWithData(
        QChart *chart,
        double YMaxOverride,
        std::string species,
        boost::numeric::ublas::matrix<double> &ChartData,
        boost::numeric::ublas::vector<std::string> &LegendNames,
        std::string mainTitle,
        std::string xTitle,
        std::string yTitle,
        std::vector<std::string> xLabels,
        bool rotateLabels)
{
    QLineSeries *series = NULL;

    chart->removeAxis(chart->axes(Qt::Vertical).back());
    chart->removeAxis(chart->axes(Qt::Horizontal).back());

    // Draw main chart title
    std::stringstream ss;
    QFont mainTitleFont = chart->titleFont();
    mainTitleFont.setPointSize(14);
    mainTitleFont.setWeight(QFont::Bold);
    chart->setTitleFont(mainTitleFont);
    std::string delim = (species.empty()) ? "" : " - ";
    ss << species << delim << mainTitle;
    chart->setTitle(ss.str().c_str());

    // Draw legend on right
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);


    // Create a new X axis, add labels and the title
    QCategoryAxis *axisX = new QCategoryAxis();
    int NumXLabels = xLabels.size();
    for (int i=0; i<NumXLabels; ++i) {
        axisX->append(xLabels[i].c_str(), i+1);
    }
   // axisX->setRange(0, NumXLabels);
    if ((NumXLabels > nmfConstants::NumCategoriesForVerticalNotation) && (rotateLabels))
        axisX->setLabelsAngle(-90);
    else
        axisX->setLabelsAngle(0);
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);

    // Create, scale, and label Y axis.
    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%.2f");
    axisY->setMin(0.0);
    for (unsigned int line=0; line<ChartData.size2(); ++line) {
        series = new QLineSeries;
        for (unsigned int val=0; val< ChartData.size1(); ++val) {
            series->append(val+1,ChartData(val,line));
        }
        chart->addSeries(series);
        series->setName(QString::fromStdString(LegendNames(line)));
        //chart->setAxisY(axisY,series);
        nmfUtilsQt::setAxisY(chart,axisY,series);
    }
    axisY = qobject_cast<QValueAxis *>(chart->axes(Qt::Vertical).at(0));
    if (YMaxOverride > 0.0) {
        axisY->setMax(YMaxOverride); // YMax); // *1.2);
        axisY->setMin(0.0);
    } else {
        axisY->applyNiceNumbers(); // Adjusting yaxis to be a rounder number, i.e. 838 should be 1000
        //axisY->setMax(ymax);
        axisY->setMin(0.0);
    }

    // Set title on X axis
    //chart->setAxisX(axisX, series);
    nmfUtilsQt::setAxisX(chart,axisX,series);
    QFont titleFont = axisX->titleFont();
    titleFont.setPointSize(12);
    titleFont.setWeight(QFont::Bold);
    axisX->setTitleFont(titleFont);
    axisX->setTitleText(xTitle.c_str());
    axisY->setTitleFont(titleFont);
    axisY->setTitleText(yTitle.c_str());

} // end loadChartWithData



// Public function to calculate YPR given an array of weights at age, M at age, and F at Age
double
ChartLine::YPR(
        boost::numeric::ublas::vector<double> &WeightAtAge,
        boost::numeric::ublas::vector<double> &FatAge,  //FatAge holds PRF values
        boost::numeric::ublas::vector<double> &M2atAge,
        double FullF,
        boost::numeric::ublas::vector<double> &M1,
        int Nage)
{
    double Death;   // for death rate
    double PropF;   // for F/Z
    double TempVal; // hold sum through iterations
    double NAlive;  // holds the product for number alive at age t
    double Z;       // mortality as sum of M1 + M2 + F
    double M2;

    TempVal = 0.0;
    NAlive = 1.0;

    for (int i = 0; i < Nage; ++i) {
        M2       = (M2atAge(i) < 0) ? 0 : M2atAge(i);
        Z        = (FatAge(i) * FullF) + M2 + M1(i);   // total mortality
        Death    = NAlive * (1 - std::exp(-Z));        // death rate
        NAlive  *= std::exp(-Z);                       // number alive at age - accumulating product
        PropF    = (FatAge(i) * FullF) / Z;            // proportion of Z due to fishing
        TempVal += (Death * PropF * WeightAtAge(i));   // accumulates yield at age
    }

    return TempVal;

} // end YPR



double
ChartLine::F01(boost::numeric::ublas::vector<double> &WeightAtAge,
        boost::numeric::ublas::vector<double> &FatAge,
        boost::numeric::ublas::vector<double> &M2atAge,
        boost::numeric::ublas::vector<double> &M1,
        double Nage)
{

    double YPR0;
    double YPR1;
    double Slope;
    double SlopeRef;
    double F0;
    double Diff;

    // Find F0.1
    // First calculate slope at origin - Slope0
    YPR1 = YPR(WeightAtAge, FatAge, M2atAge, 0.001, M1, Nage);
    YPR0 = YPR(WeightAtAge, FatAge, M2atAge, 0.0, M1, Nage);

    // Ref slope is 10% of slope at origin
    SlopeRef = ((YPR1 - YPR0) / 0.001) * 0.1;
    Slope = (YPR1 - YPR0) / 0.001; // this is the slope at the origin
    Diff = Slope - SlopeRef; // this is the difference to minimize
    F0 = 0.001;
    while (Diff > 0.001) {

        YPR1 = YPR(WeightAtAge, FatAge, M2atAge, F0 + 0.001, M1, Nage);
        YPR0 = YPR(WeightAtAge, FatAge, M2atAge, F0, M1, Nage);

        Slope = (YPR1 - YPR0) / 0.001;
        Diff = Slope - SlopeRef; // this is the difference to minimize

        F0 += 0.001;
    }

    return (F0-0.001);

} // end F01


double
ChartLine::FMax(boost::numeric::ublas::vector<double> &WeightAtAge,
        boost::numeric::ublas::vector<double> &FatAge,
        boost::numeric::ublas::vector<double> &M2atAge,
        boost::numeric::ublas::vector<double> &M1,
        double Nage)
{
    double YPR0;
    double YPR1;
    double Slope;
    double F0;

    // Calculate Fmax
    YPR1 = YPR(WeightAtAge, FatAge, M2atAge, 0.001, M1, Nage);
    YPR0 = YPR(WeightAtAge, FatAge, M2atAge, 0.0, M1, Nage);

    Slope = (YPR1 - YPR0) / 0.001; // this is the slope at the origin
    F0 = 0.001;
    bool done=false;
    while ((Slope > 0) and (! done)) {

        YPR1 = YPR(WeightAtAge, FatAge, M2atAge, F0 + 0.001, M1, Nage);
        YPR0 = YPR(WeightAtAge, FatAge, M2atAge, F0, M1, Nage);

        Slope = (YPR1 - YPR0) / 0.001;
        F0 += 0.001;

        // Put in a break to bounce it out if F gets bigger than 4....max Fmax undefined
        // this will at least prevent infinite loops...
        done = (F0 > 4);
    }

    return (F0 < 4) ? (F0-0.001) : -9;
} // end FMax


double
ChartLine::SSB(boost::numeric::ublas::vector<double> &WeightAtAge,
        boost::numeric::ublas::vector<double> &FatAge,
        boost::numeric::ublas::vector<double> &M2atAge,
        double FullF,
        boost::numeric::ublas::vector<double> &M1,
        int &Nage,
        boost::numeric::ublas::vector<double> &Pmature)
{
    double Z;  // mortality as sum of M + F
    double M2;
    double SSBtemp = 0.0; // hold sum through iterations
    double NAlive = 1.0;  // holds the product for number alive at age t

    for (int i = 0; i < Nage; ++i) {
        M2 = (M2atAge(i) < 0) ? 0 : M2atAge(i);
        Z = M1(i) + M2 + (FullF * FatAge(i));  // total mortality for that age
        NAlive *= std::exp(-Z);  // number alive at age - accumulating product
        SSBtemp += (NAlive * WeightAtAge(i) * Pmature(i)); //  accumulates SSB at age
    } // end for i

    return SSBtemp;
} // end SSB


double
ChartLine::SSBBench(boost::numeric::ublas::vector<double> &WeightAtAge,
        boost::numeric::ublas::vector<double> &FatAge,
        boost::numeric::ublas::vector<double> &M2atAge,
        boost::numeric::ublas::vector<double> &M1,
        int &Nage,
        boost::numeric::ublas::vector<double> &Pmature,
        double BenchVal)
{
    double SSBtemp;
    double SSBMax;
    double F0 = 0.001;
    double Ratio;

    // Find F giving SSB/R = benchval of max
    SSBMax  = SSB(WeightAtAge, FatAge, M2atAge, 0, M1, Nage, Pmature);
    SSBtemp = SSBMax;
    Ratio   = SSBtemp / SSBMax;

    // RSK check this logic for potential of infinite loop
    while (Ratio > BenchVal) {
        SSBtemp = SSB(WeightAtAge, FatAge, M2atAge, F0, M1, Nage, Pmature);
        Ratio = SSBtemp / SSBMax;
        F0 += 0.001;
    }

    return (F0 - 0.001);

} // end SSBBench


void
ChartLine::calculateWeightAveFAndAssignPRFs(
            const int &NumYears,
            const int &RecAge,
            const int &Nage,
            boost::numeric::ublas::matrix<double> &FatAge,
            boost::numeric::ublas::matrix<double> &Catch,
            boost::numeric::ublas::vector<double> &FullF,
            boost::numeric::ublas::matrix<double> &PRF)
{
    double tmpSum;
    double tmpTotCat;

    for (int i=0; i<NumYears; ++i) {
        tmpSum = 0.0;
        tmpTotCat = 0.0;
        for (int j=RecAge; j<Nage; ++j) {
            tmpSum    += (FatAge(i,j) * Catch(i,j));
            tmpTotCat += Catch(i,j);
        }
        FullF(i) = tmpSum / tmpTotCat;
    } // end i

    // Assign PRFs to unrecruited age classes
    for (int i = 0; i < NumYears; ++i) {
        for (int j = 0; j < RecAge; ++j) {
            PRF(i, j) = FatAge(i, j) / FullF(i);
        }
        for (int j = RecAge; j < Nage; ++j) {
            PRF(i, j) = 1;
        }
    }

} // end calculateWeightAveFAndAssignPRFs


std::string
ChartLine::getYAxisUnits(nmfDatabase* databasePtr,
                         std::string selectedSpecies)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string yAxisUnits="";

    // Find units
    fields = {"WtUnits"};
    queryStr = "SELECT WtUnits FROM " + nmfConstantsMSVPA::TableSpecies +
               " WHERE SpeName = '" + selectedSpecies + "' ";
    dataMap = databasePtr->nmfQueryDatabase(queryStr, fields);

    double units = std::stod(dataMap["WtUnits"][0]);
    if (units == 1.0)
        yAxisUnits = "(kg)";
    else if (units == 0.001)
        yAxisUnits = "(g)";
    else if (units == (1/2.2)) // In original code. Not sure about this. -RSK
        yAxisUnits = "(lbs.)";

    return yAxisUnits;

} // end getYAxisUnits






