#ifndef CHARTLINEMULTISPECIESPOPULATIONS_H
#define CHARTLINEMULTISPECIESPOPULATIONS_H

#include "ChartLine.h"


class ChartLineMultispeciesPopulations : public ChartLine
{

private:
    QTableWidget* m_dataTable;
    nmfLogger*    m_logger;

    std::map<std::string, void(*)(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string ModelName,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int FirstYear,
            int NumYears, // These are years selected from the gui
            std::string seasonStr,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            double &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData
            )> GetDataFunctionMap;

public:
    ChartLineMultispeciesPopulations(
            QTableWidget* theDataTable,
            nmfLogger*    theLogger);
    virtual ~ChartLineMultispeciesPopulations() {}

    // Had to make these static so the GetDataFunctionMap would work.
    static void getAndLoadTotalBiomassDataMSVPA(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string ModelName,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int FirstYear,
            int NumYears, // These are years selected from the gui
            std::string seasonStr,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            double &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData);

    static void getAndLoadTotalAbundanceDataMSVPA(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string ModelName,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int FirstYear,
            int NumYears, // These are years selected from the gui
            std::string seasonStr,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            double &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData);

    static void getAndLoadAge1PlusBiomassDataMSVPA(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string ModelName,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int FirstYear,
            int NumYears, // These are years selected from the gui
            std::string seasonStr,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            double &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData);

    static void getAndLoadAge1PlusAbundanceDataMSVPA(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string ModelName,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int FirstYear,
            int NumYears, // These are years selected from the gui
            std::string seasonStr,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            double &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData);

    static void getAndLoadSpawningStockBiomassDataMSVPA(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string ModelName,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int FirstYear,
            int NumYears, // These are years selected from the gui
            std::string seasonStr,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            double &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData);

    static void getAndLoadTotalBiomassDataForecast(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string ModelName,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int FirstYear,
            int NumYears, // These are years selected from the gui
            std::string seasonStr,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            double &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData);

    static void getAndLoadTotalAbundanceDataForecast(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string ModelName,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int FirstYear,
            int NumYears, // These are years selected from the gui
            std::string seasonStr,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            double &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData);

    static void getAndLoadAge1PlusBiomassDataForecast(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string ModelName,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int FirstYear,
            int NumYears, // These are years selected from the gui
            std::string seasonStr,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            double &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData);

    static void getAndLoadAge1PlusAbundanceDataForecast(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string ModelName,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int FirstYear,
            int NumYears, // These are years selected from the gui
            std::string seasonStr,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            double &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData);

    static void getAndLoadSpawningStockBiomassDataForecast(
            nmfDatabase* databasePtr,
            QChart* chart,
            std::string ModelName,
            std::string MSVPAName,
            std::string ForecastName,
            std::string ScenarioName,
            int FirstYear,
            int NumYears, // These are years selected from the gui
            std::string seasonStr,
            QStringList &RowLabels,
            QStringList &ColumnLabels,
            double &MaxScaleY,
            boost::numeric::ublas::matrix<double> &GridData);

    static void loadChartWithData(
            QChart *chart,
            boost::numeric::ublas::matrix<double> &ChartData,
            boost::numeric::ublas::vector<std::string> &LegendNames,
            std::string mainTitle,
            std::string xTitle,
            std::string yTitle,
            std::vector<std::string> xLabels,
            double MaxScaleY,
            bool rotateLabels);

    static void getMSVPASpecies(
            nmfDatabase* databasePtr,
            const std::string &MSVPAName,
            int &NMSVPASpecies,
            boost::numeric::ublas::vector<std::string> &MSVPASpeList);

    static void getMSVPASpeciesNamesAges(
            nmfDatabase* databasePtr,
            const int &NMSVPASpe,
            boost::numeric::ublas::vector<std::string> &MSVPASpeList,
            std::vector<int> &MSVPASpeAge);

    static void getMaturity(
            nmfDatabase* databasePtr,
            const int &Forecast_NYears,
            const int &FirstYear,
            const int &NMSVPASpe,
            const boost::numeric::ublas::vector<std::string> &MSVPASpeList,
            const std::vector<int> &MSVPASpeAge,
            Boost3DArrayDouble &Maturity);

public slots:
    void callback_UpdateChart(nmfStructsQt::UpdateDataStruct data);

};

#endif // CHARTLINEMULTISPECIESPOPULATIONS_H
