#ifndef CHARTBAR_H
#define CHARTBAR_H

#include "MSVPAX2OutputChart.h"


QT_CHARTS_USE_NAMESPACE

class ChartBar : public MSVPAX2OutputChart
{

    Q_OBJECT

public:
    ChartBar();
    virtual ~ChartBar() {}

    static bool isOfTypeAbundance(std::string selectedVariable);

    static void getMaturityData(
            nmfDatabase *databasePtr,
            const std::string &ModelName,
            const int &Nage,
            const int &NYears,
            const int &FirstYear,
            const int &LastYear,
            const std::string &SpeName,
            boost::numeric::ublas::matrix<double> &Maturity);

    static void getSpawningStockBiomassDataAndLoadChart(
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
            QStringList &rowLabels,
            int &Theme);

    static void getDataAndLoadChart(
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
            QStringList &rowLabels,
            QStringList &LegendNames,
            int &Theme);

    static void loadChartWithData(
            QChart *chart,
            QBarSeries *series,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &LegendNames,
            const bool &useMaxValue,
            const std::string &MaxScaleY,
            const bool &inThousands);

    static void loadChartWithData(
            QChart *chart,
            QStackedBarSeries *series,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &LegendNames,
            const bool &useMaxValue,
            const std::string &MaxScaleY,
            const bool &inThousands);

    static void getChartDataOfPredationMortalityByPredator(
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
          boost::numeric::ublas::matrix<double> &ChartData);

    static void getChartDataMSVPA(
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
            QStringList &categories,
            QStringList &LegendNames,
            std::string &SizeUnitString,
            std::string &WtUnitString);

    static void getChartDataForecast(
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
            QStringList &categories,
            QStringList &LegendNames,
            std::string &SizeUnitString,
            std::string &WtUnitString);

    static void setTitles(
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
            int Theme);

    static void setTitles(
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
            int Theme);

    std::string constructMainTitle(
            const bool        &rearrangeTitle,
            const std::string &main,
            const std::string &agePrefix,
            const std::string &species,
            const std::string &sub);

    void populateChart(
            QChart *chart,
            std::string &type,
            const boost::numeric::ublas::matrix<double> &ChartData,
            const QStringList &RowLabels,
            const QStringList &ColumnLabels,
            std::string &MainTitle,
            std::string &XTitle,
            std::string &YTitle,
            const std::vector<bool> &GridLines,
            const int Theme);

public slots:
    void callback_UpdateChart(nmfStructsQt::UpdateDataStruct data) {};


};

#endif // CHARTBAR_H
