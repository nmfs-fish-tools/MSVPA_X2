#ifndef CHARTBARFOODAVAILABILITY_H
#define CHARTBARFOODAVAILABILITY_H

#include "ChartBar.h"

class ChartBarFoodAvailability : public ChartBar
{

private:
    QTableWidget* DataTable;
    nmfLogger*    logger;

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
            std::string &SelectedSpecies,
            std::string &selectedSpeciesAgeSizeClass,
            std::string &SelectedVariable,
            std::string &SelectedByVariables,
            std::string &SelectedSeason,
            std::string &SelectedPreyName,
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
    ChartBarFoodAvailability(QTableWidget* DataTable,
                             nmfLogger*    logger);
    virtual ~ChartBarFoodAvailability() {}

    // Had to make these static so the GetDataFunctionMap would work.
    static void getConversionScaleFactorAndYLabel(
            nmfDatabase* databasePtr,
            std::string &SelectedPredator,
            std::string &SelectedVariable,
            double &NConversion,
            std::string &yLabelConversion);

    static void loadChartWithData(
            QChart *chart,
            QStackedBarSeries *series,
            std::string &maxValueStr,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &LegendNames);

    static void setTitles(
            QChart *chart,
            QStackedBarSeries *series, QStringList categories,
            std::string species, std::string byVariables,
            bool rearrangeTitle, std::string dataType,
            std::string xLabel, std::string yLabel,
            std::string suffix,
            int Theme);

    static void getAndLoadFoodAvailabilityByPreyTypeData(
            nmfDatabase *databasePtr,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int &MSVPA_NYears,
            int &MSVPA_FirstYear,
            int &MSVPA_LastYear,
            int &Nage,
            std::string &selectedSpecies,
            std::string &selectedSpeciesAgeSizeClass,
            std::string &SelectedVariable,
            std::string &SelectedByVariables,
            std::string &SelectedSeason,
            std::string &SelectedPreyName,
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

    static void getAndLoadFoodAvailabilityByPreyAgeData(
            nmfDatabase *databasePtr,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int &MSVPA_NYears,
            int &MSVPA_FirstYear,
            int &MSVPA_LastYear,
            int &Nage,
            std::string &selectedSpecies,
            std::string &selectedSpeciesAgeSizeClass,
            std::string &SelectedVariable,
            std::string &SelectedByVariables,
            std::string &SelectedSeason,
            std::string &SelectedPreyName,
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

    static void getAndLoadRelativeFoodAvailabilityData(
            nmfDatabase *databasePtr,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int &MSVPA_NYears,
            int &MSVPA_FirstYear,
            int &MSVPA_LastYear,
            int &Nage,
            std::string &SelectedSpecies,
            std::string &selectedSpeciesAgeSizeClass,
            std::string &SelectedVariable,
            std::string &SelectedByVariables,
            std::string &SelectedSeason,
            std::string &SelectedPrey,
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

    static void getAndLoadPerCapitaFoodAvailabilityData(
            nmfDatabase *databasePtr,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int &MSVPA_NYears,
            int &MSVPA_FirstYear,
            int &MSVPA_LastYear,
            int &Nage,
            std::string &SelectedSpecies,
            std::string &selectedSpeciesAgeSizeClass,
            std::string &SelectedVariable,
            std::string &SelectedByVariables,
            std::string &SelectedSeason,
            std::string &SelectedPrey,
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

    static void getAndLoadStomachContentWeightData(
            nmfDatabase *databasePtr,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int &MSVPA_NYears,
            int &MSVPA_FirstYear,
            int &MSVPA_LastYear,
            int &Nage,
            std::string &SelectedSpecies,
            std::string &SelectedSpeciesAgeSizeClass,
            std::string &SelectedVariable,
            std::string &SelectedByVariables,
            std::string &SelectedSeason,
            std::string &SelectedPrey,
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


public slots:
    void callback_UpdateChart(nmfStructsQt::UpdateDataStruct data);

};

#endif // CHARTBARFOODAVAILABILITY_H
