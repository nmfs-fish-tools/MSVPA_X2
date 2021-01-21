#ifndef CHARTBARGROWTH_H
#define CHARTBARGROWTH_H

#include "ChartBar.h"

class ChartBarGrowth : public ChartBar
{

private:
    QTableWidget* DataTable;
    nmfLogger* logger;

    std::map<std::string, void(*)(
            nmfDatabase *databasePtr,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int &MSVPA_NYears,
            int &MSVPA_FirstYear,
            int &MSVPA_LastYear,
            int &Nage,
            std::string &SelectedPredator,
            std::string &SelectedVariable,
            std::string &SelectedByVariables,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
            int &offset,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            QChart *chart,
            std::string &yLabel,
            int &Theme
            )> GetDataFunctionMap;

public:
    ChartBarGrowth(
            QTableWidget* theDataTable,
            nmfLogger*    logger);
    virtual ~ChartBarGrowth() {}

    // Had to make these static so the GetDataFunctionMap would work.
    static void getConversionScaleFactorAndYLabel(
            nmfDatabase* databasePtr,
            std::string &SelectedPredator,
            std::string &SelectedVariable,
            double &NConversion,
            std::string &yLabelConversion);

    static void getAndLoadAverageWeightData(
            nmfDatabase *databasePtr,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int &MSVPA_NYears,
            int &MSVPA_FirstYear,
            int &MSVPA_LastYear,
            int &Nage,
            std::string &SelectedPredator,
            std::string &SelectedVariable,
            std::string &SelectedByVariables,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
            int &offset,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            QChart *chart,
            std::string &yLabel,
            int &Theme);

    static void getAndLoadAverageSizeData(
            nmfDatabase *databasePtr,
            std::string &MSVPAName,
            std::string &ModelName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int &MSVPA_NYears,
            int &MSVPA_FirstYear,
            int &MSVPA_LastYear,
            int &Nage,
            std::string &SelectedPredator,
            std::string &SelectedVariable,
            std::string &SelectedByVariables,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
            int &offset,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            QChart *chart,
            std::string &yLabel,
            int &Theme);

    static void getAndLoadWeightAtAgeData(
            nmfDatabase *databasePtr,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int &MSVPA_NYears,
            int &MSVPA_FirstYear,
            int &MSVPA_LastYear,
            int &Nage,
            std::string &SelectedPredator,
            std::string &SelectedVariable,
            std::string &SelectedByVariables,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
            int &offset,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            QChart *chart,
            std::string &yLabel,
            int &Theme);

    static void getAndLoadSizeAtAgeData(
            nmfDatabase *databasePtr,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int &MSVPA_NYears,
            int &MSVPA_FirstYear,
            int &MSVPA_LastYear,
            int &Nage,
            std::string &SelectedPredator,
            std::string &SelectedVariable,
            std::string &SelectedByVariables,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
            int &offset,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            QChart *chart,
            std::string &yLabel,
            int &Theme);

    static void getChartData(
            nmfDatabase *databasePtr,
            const std::string &ModelName,
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
            const std::string &ageStr,
            const std::string &recruitChar,
            const std::string &seasonStr,
            const int &season,
            const std::string &selectedPreyName,
            const int &FirstYear,
            const int &LastYear,
            const int &SpeAge,
            QChart *chart,
            const std::string &title,
            const std::string &titleSuffix,
            const bool &rearrangeTitle,
            const std::string &xLabel,
            const std::string &yLabel,
            const double &scaleFactor,
            const std::string &MaxScaleY,
            QStringList &rowLabels,
            QStringList &LegendNames,
            int &Theme);

public slots:
    void callback_UpdateChart(nmfStructsQt::UpdateDataStruct data);

};

#endif // CHARTBARGROWTH_H
