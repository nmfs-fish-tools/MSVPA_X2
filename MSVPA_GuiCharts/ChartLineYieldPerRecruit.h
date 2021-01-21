#ifndef CHARTLINEYIELDPERRECRUIT_H
#define CHARTLINEYIELDPERRECRUIT_H

#include "ChartLine.h"

class ChartLineYieldPerRecruit : public ChartLine
{

private:
    QTableWidget* DataTable;
    nmfLogger*    logger;

    std::map<std::string, void(*)(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int FirstYear,
            int NYears,
            std::string selectedSpecies,
            std::vector<std::string> &SelectedYears,
            std::string MaxScaleY,
            int Nage,
            int RecAge,
            boost::numeric::ublas::matrix<double> &GridData,
            QStringList &RowLabels,
            QStringList &ColumnLabels
            )> GetDataFunctionMap;

public:
    ChartLineYieldPerRecruit(
            QTableWidget* theDataTable,
            nmfLogger*    theLogger);
    virtual ~ChartLineYieldPerRecruit() {}

    // Had to make these static so the GetDataFunctionMap would work.

    static void getForecastYears(
            nmfDatabase* databasePtr,
            const std::string &MSVPAName,
            const std::string &ForecastName,
            int &Forecast_FirstName,
            int &Forecast_LastName,
            int &Forecast_NYears);

    static void getAndLoadYPRvsFDataMSVPA(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int FirstYear,
            int NYears,
            std::string selectedSpecies,
            std::vector<std::string> &SelectedYears,
            std::string MaxScaleY,
            int Nage,
            int RecAge,
            boost::numeric::ublas::matrix<double> &GridData,
            QStringList &RowLabels,
            QStringList &ColumnLabels);

    static void getAndLoadSSBvsFDataMSVPA(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int FirstYear,
            int NYears,
            std::string selectedSpecies,
            std::vector<std::string> &SelectedYears,
            std::string MaxScaleY,
            int Nage,
            int RecAge,
            boost::numeric::ublas::matrix<double> &GridData,
            QStringList &RowLabels,
            QStringList &ColumnLabels);

    static void getAndLoadHistoricalYPRDataMSVPA(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int FirstYear,
            int NYears,
            std::string selectedSpecies,
            std::vector<std::string> &SelectedYears,
            std::string MaxScaleY,
            int Nage,
            int RecAge,
            boost::numeric::ublas::matrix<double> &GridData,
            QStringList &RowLabels,
            QStringList &ColumnLabels);

    static void getAndLoadHistoricalFBenchmarksDataMSVPA(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int FirstYear,
            int NYears,
            std::string selectedSpecies,
            std::vector<std::string> &SelectedYears,
            std::string MaxScaleY,
            int Nage,
            int RecAge,
            boost::numeric::ublas::matrix<double> &GridData,
            QStringList &RowLabels,
            QStringList &ColumnLabels);

    static void getAndLoadHistoricalSSBBenchmarksDataMSVPA(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int FirstYear,
            int NYears,
            std::string selectedSpecies,
            std::vector<std::string> &SelectedYears,
            std::string MaxScaleY,
            int Nage,
            int RecAge,
            boost::numeric::ublas::matrix<double> &GridData,
            QStringList &RowLabels,
            QStringList &ColumnLabels);

    static void getAndLoadYPRvsFDataForecast(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int FirstYear,
            int NYears,
            std::string selectedSpecies,
            std::vector<std::string> &SelectedYears,
            std::string MaxScaleY,
            int Nage,
            int RecAge,
            boost::numeric::ublas::matrix<double> &GridData,
            QStringList &RowLabels,
            QStringList &ColumnLabels);

    static void getAndLoadSSBvsFDataForecast(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int FirstYear,
            int NYears,
            std::string selectedSpecies,
            std::vector<std::string> &SelectedYears,
            std::string MaxScaleY,
            int Nage,
            int RecAge,
            boost::numeric::ublas::matrix<double> &GridData,
            QStringList &RowLabels,
            QStringList &ColumnLabels);

    static void getAndLoadProjectedYPRDataForecast(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int FirstYear,
            int NYears,
            std::string selectedSpecies,
            std::vector<std::string> &SelectedYears,
            std::string MaxScaleY,
            int Nage,
            int RecAge,
            boost::numeric::ublas::matrix<double> &GridData,
            QStringList &RowLabels,
            QStringList &ColumnLabels);

    static void getAndLoadProjectedFBenchmarksDataForecast(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int FirstYear,
            int NYears,
            std::string selectedSpecies,
            std::vector<std::string> &SelectedYears,
            std::string MaxScaleY,
            int Nage,
            int RecAge,
            boost::numeric::ublas::matrix<double> &GridData,
            QStringList &RowLabels,
            QStringList &ColumnLabels);

    static void getAndLoadProjectedSSBBenchmarksDataForecast(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            int FirstYear,
            int NYears,
            std::string selectedSpecies,
            std::vector<std::string> &SelectedYears,
            std::string MaxScaleY,
            int Nage,
            int RecAge,
            boost::numeric::ublas::matrix<double> &GridData,
            QStringList &RowLabels,
            QStringList &ColumnLabels);

public slots:
    void callback_UpdateChart(nmfStructsQt::UpdateDataStruct data);

};

#endif // CHARTLINEYIELDPERRECRUIT_H
