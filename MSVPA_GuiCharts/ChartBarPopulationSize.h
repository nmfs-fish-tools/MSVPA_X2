#ifndef CHARTBARPOPULATIONSIZE_H
#define CHARTBARPOPULATIONSIZE_H

#include "ChartBar.h"

class ChartBarPopulationSize : public ChartBar
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
            std::string &SelectedPredator,
            std::string &SelectedVariable,
            std::string &SelectedByVariables,
            std::string &SelectedSpeciesAgeSizeClass,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            QChart *chart,
            std::string &yLabel,
            int &Theme
            )> GetDataFunctionMap;

public:
    ChartBarPopulationSize(
            QTableWidget* theDataTable,
            nmfLogger*    logger);
    virtual ~ChartBarPopulationSize() {}

    // Had to make these static so the GetDataFunctionMap would work.
    static void getConversionScaleFactorAndYLabel(
            nmfDatabase* databasePtr,
            std::string &SelectedPredator,
            std::string &SelectedVariable,
            double &NConversion,
            std::string &yLabelConversion);

    static void getAndLoadTotalBiomassData(
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
            std::string &SelectedSpeciesAgeSizeClass,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            QChart *chart,
            std::string &yLabel,
            int &Theme);

    static void getAndLoadBiomassByAgeData(
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
            std::string &SelectedSpeciesAgeSizeClass,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            QChart *chart,
            std::string &yLabel,
            int &Theme);

    static void getAndLoadSpawningStockBiomassData(
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
            std::string &SelectedSpeciesAgeSizeClass,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            QChart *chart,
            std::string &yLabel,
            int &Theme);

    static void getAndLoadTotalAbundanceData(
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
            std::string &SelectedSpeciesAgeSizeClass,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            QChart *chart,
            std::string &yLabel,
            int &Theme);

    static void getAndLoadAbundanceByAgeData(
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
            std::string &SelectedSpeciesAgeSizeClass,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            QChart *chart,
            std::string &yLabel,
            int &Theme);

    static void getAndLoadRecruitAbundanceData(
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
            std::string &SelectedSpeciesAgeSizeClass,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            QChart *chart,
            std::string &yLabel,
            int &Theme);

    static void getAndLoadRecruitBiomassData(
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
            std::string &SelectedSpeciesAgeSizeClass,
            std::string &SelectedSeason,
            int &SeasonInt,
            int &SpeAge,
            std::string &MaxScaleY,
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

#endif // CHARTBARPOPULATIONSIZE_H
