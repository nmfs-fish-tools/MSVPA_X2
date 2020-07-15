#ifndef CHARTSTACKEDBAR_H
#define CHARTSTACKEDBAR_H

#include "MSVPAX2OutputChart.h"

//#include "GuiControlsDietComposition.h"

QT_CHARTS_USE_NAMESPACE

class ChartStackedBar : public MSVPAX2OutputChart
{

    Q_OBJECT

private:

public:
    ChartStackedBar();
    virtual ~ChartStackedBar() {}

    // Implemented here like this so other applications could use this class.
    void callback_UpdateChart(nmfStructsQt::UpdateDataStruct data) {};

    void loadChartWithData(
            QChart *chart,
            const boost::numeric::ublas::matrix<double> &ChartData,
            const QStringList &ColumnLabels);

    void setTitlesAndGridLines(
            QChart *chart,
            QStackedBarSeries *series,
            QStringList categories,
            std::string species,
            bool rearrangeTitle,
            std::string dataType,
            std::string xLabel,
            std::string yLabel,
            std::string titlePrefix,
            std::string titleSuffix,
            const std::vector<bool> &GridLines);


    std::string constructMainTitle(
            const std::string &Species,
            const bool        &rearrangeTitle,
            const std::string &DataTypeLabel,
            const std::string &XLabel,
            const std::string &TitlePrefix,
            const std::string &TitleSuffix);

    void populateChart(
            QChart *chart,
            const boost::numeric::ublas::matrix<double> &ChartData,
            const QStringList &RowLabels,
            const QStringList &ColumnLabels,
            std::string &MainTitle,
            std::string &XTitle,
            std::string &YTitle,
            const std::vector<bool> &GridLines,
            const int Theme);

public slots:
    void callback_UpdateChartGridLines(nmfStructsQt::UpdateDataStruct data);
//    void callback_UpdateTheme(int theme);

};

#endif // CHARTSTACKEDBAR_H
