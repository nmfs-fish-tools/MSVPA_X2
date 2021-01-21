#ifndef CHARTSTACKEDBARDIETCOMPOSITION_H
#define CHARTSTACKEDBARDIETCOMPOSITION_H

#include "ChartStackedBar.h"

class ChartStackedBarDietComposition : public ChartStackedBar
{
    QTableWidget* DataTableWidget;
    nmfLogger*    logger;

private:
    std::map<std::string, void(*)(
            nmfDatabase* databasePtr,
            std::string &ModelName,
            std::string &MSVPAName,
            std::string &ForecastName,
            std::string &ScenarioName,
            std::string &SelectedSpecies,
            std::string &SelectedSpeciesAgeSizeClass,
            std::string &SelectedSeason,
            boost::numeric::ublas::matrix<double> &GridData,
            boost::numeric::ublas::matrix<double> &ChartData,
            std::string &TitlePrefix,
            std::string &TitleSuffix,
            std::string &xLabel,
            std::string &yLabel,
            QStringList &RowLabels,
            QStringList &ColumnLabels
            )> GetDataFunctionMap;


public:
    ChartStackedBarDietComposition(
            QTableWidget* theDataTable,
            nmfLogger*    logger);
    virtual ~ChartStackedBarDietComposition() {}

    static void getAverageDataMSVPA(nmfDatabase* databasePtr,
                                    std::string &ModelName,
                                    std::string &MSVPAName,
                                    std::string &ForecastName,
                                    std::string &ScenarioName,
                                    std::string &SelectedSpecies,
                                    std::string &SelectedSpeciesAgeSizeClass,
                                    std::string &SelectedSeason,
                                    boost::numeric::ublas::matrix<double> &GridData,
                                    boost::numeric::ublas::matrix<double> &ChartData,
                                    std::string &TitlePrefix,
                                    std::string &TitleSuffix,
                                    std::string &xLabel,
                                    std::string &yLabel,
                                    QStringList &RowLabels,
                                    QStringList &ColumnLabels);

    static void getAverageByYearDataMSVPA(nmfDatabase* databasePtr,
                                          std::string &ModelName,
                                          std::string &MSVPAName,
                                          std::string &ForecastName,
                                          std::string &ScenarioName,
                                          std::string &SelectedSpecies,
                                          std::string &SelectedSpeciesAgeSizeClass,
                                          std::string &SelectedSeason,
                                          boost::numeric::ublas::matrix<double> &GridData,
                                          boost::numeric::ublas::matrix<double> &ChartData,
                                          std::string &TitlePrefix,
                                          std::string &TitleSuffix,
                                          std::string &xLabel,
                                          std::string &yLabel,
                                          QStringList &RowLabels,
                                          QStringList &ColumnLabels);

    static void getAverageBySeasonDataMSVPA(nmfDatabase* databasePtr,
                                            std::string &ModelName,
                                            std::string &MSVPAName,
                                            std::string &ForecastName,
                                            std::string &ScenarioName,
                                            std::string &SelectedSpecies,
                                            std::string &SelectedSpeciesAgeSizeClass,
                                            std::string &SelectedSeason,
                                            boost::numeric::ublas::matrix<double> &GridData,
                                            boost::numeric::ublas::matrix<double> &ChartData,
                                            std::string &TitlePrefix,
                                            std::string &TitleSuffix,
                                            std::string &xLabel,
                                            std::string &yLabel,
                                            QStringList &RowLabels,
                                            QStringList &ColumnLabels);

    static void getAverageByYearAndSeasonDataMSVPA(nmfDatabase* databasePtr,
                                                   std::string &ModelName,
                                                   std::string &MSVPAName,
                                                   std::string &ForecastName,
                                                   std::string &ScenarioName,
                                                   std::string &SelectedSpecies,
                                                   std::string &SelectedSpeciesAgeSizeClass,
                                                   std::string &SelectedSeason,
                                                   boost::numeric::ublas::matrix<double> &GridData,
                                                   boost::numeric::ublas::matrix<double> &ChartData,
                                                   std::string &TitlePrefix,
                                                   std::string &TitleSuffix,
                                                   std::string &xLabel,
                                                   std::string &yLabel,
                                                   QStringList &RowLabels,
                                                   QStringList &ColumnLabels);

    static void getAverageDataForecast(nmfDatabase* databasePtr,
                                       std::string &ModelName,
                                       std::string &MSVPAName,
                                       std::string &ForecastName,
                                       std::string &ScenarioName,
                                       std::string &SelectedSpecies,
                                       std::string &SelectedSpeciesAgeSizeClass,
                                       std::string &SelectedSeason,
                                       boost::numeric::ublas::matrix<double> &GridData,
                                       boost::numeric::ublas::matrix<double> &ChartData,
                                       std::string &TitlePrefix,
                                       std::string &TitleSuffix,
                                       std::string &xLabel,
                                       std::string &yLabel,
                                       QStringList &RowLabels,
                                       QStringList &ColumnLabels);


    static void getAverageByYearDataForecast(nmfDatabase* databasePtr,
                                             std::string &ModelName,
                                             std::string &MSVPAName,
                                             std::string &ForecastName,
                                             std::string &ScenarioName,
                                             std::string &SelectedSpecies,
                                             std::string &SelectedSpeciesAgeSizeClass,
                                             std::string &SelectedSeason,
                                             boost::numeric::ublas::matrix<double> &GridData,
                                             boost::numeric::ublas::matrix<double> &ChartData,
                                             std::string &TitlePrefix,
                                             std::string &TitleSuffix,
                                             std::string &xLabel,
                                             std::string &yLabel,
                                             QStringList &RowLabels,
                                             QStringList &ColumnLabels);

    static void getAverageBySeasonDataForecast(nmfDatabase* databasePtr,
                                               std::string &ModelName,
                                               std::string &MSVPAName,
                                               std::string &ForecastName,
                                               std::string &ScenarioName,
                                               std::string &SelectedSpecies,
                                               std::string &SelectedSpeciesAgeSizeClass,
                                               std::string &SelectedSeason,
                                               boost::numeric::ublas::matrix<double> &GridData,
                                               boost::numeric::ublas::matrix<double> &ChartData,
                                               std::string &TitlePrefix,
                                               std::string &TitleSuffix,
                                               std::string &xLabel,
                                               std::string &yLabel,
                                               QStringList &RowLabels,
                                               QStringList &ColumnLabels);

    static void getAverageByYearAndSeasonDataForecast(nmfDatabase* databasePtr,
                                                      std::string &ModelName,
                                                      std::string &MSVPAName,
                                                      std::string &ForecastName,
                                                      std::string &ScenarioName,
                                                      std::string &SelectedSpecies,
                                                      std::string &SelectedSpeciesAgeSizeClass,
                                                      std::string &SelectedSeason,
                                                      boost::numeric::ublas::matrix<double> &GridData,
                                                      boost::numeric::ublas::matrix<double> &ChartData,
                                                      std::string &TitlePrefix,
                                                      std::string &TitleSuffix,
                                                      std::string &xLabel,
                                                      std::string &yLabel,
                                                      QStringList &RowLabels,
                                                      QStringList &ColumnLabels);

public slots:
    void callback_UpdateChart(nmfStructsQt::UpdateDataStruct data);

};

#endif // CHARTSTACKEDBARDIETCOMPOSITION_H
