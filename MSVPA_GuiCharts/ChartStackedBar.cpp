#include "ChartStackedBar.h"
#include "nmfConstants.h"

ChartStackedBar::ChartStackedBar()
{

}


//void
//ChartStackedBar::callback_UpdateTheme(int theme)
//{
//    getChart()->setTheme(static_cast<QChart::ChartTheme>(theme));
//}


void
ChartStackedBar::callback_UpdateChartGridLines(nmfStructsQt::UpdateDataStruct data)
{
    std::vector<bool> GridLines = { data.HorizontalGridLines,
                                    data.VerticalGridLines };

    QChart* chart = getChart();

    // Set grid line visibility
    chart->axisY()->setGridLineVisible(GridLines[0]);
    chart->axisX()->setGridLineVisible(GridLines[1]);
}


void
ChartStackedBar::loadChartWithData(
        QChart *chart,
        const boost::numeric::ublas::matrix<double> &ChartData,
        const QStringList &ColumnLabels)
{
    QBarSet           *newSet;
    QStackedBarSeries *series = new QStackedBarSeries();

    // Load data into series and then add series to the chart
    for (unsigned int pred=0; pred<ChartData.size2(); ++pred) {
        newSet = new QBarSet((ColumnLabels[pred]));
        for (unsigned int val=0; val<ChartData.size1(); ++val) {
            *newSet << ChartData(val,pred);
        }
        series->append(newSet);
    }
    chart->addSeries(series);

} // end loadChartWithData


void
ChartStackedBar::setTitlesAndGridLines(
        QChart *chart,
        QStackedBarSeries *series,
        QStringList rowLabels,
        std::string species,
        bool rearrangeTitle,
        std::string dataType,
        std::string xLabel,
        std::string yLabel,
        std::string titlePrefix,
        std::string titleSuffix,
        const std::vector<bool> &GridLines)
{
    std::stringstream ss;
    QFont mainTitleFont = chart->titleFont();
    mainTitleFont.setPointSize(14);
    mainTitleFont.setWeight(QFont::Bold);
    chart->setTitleFont(mainTitleFont);
    if (rearrangeTitle) {
        ss << titlePrefix << " " << dataType << " " << species;
    } else {
        ss << titlePrefix << " " << species << " " << dataType << " By " << xLabel << titleSuffix;
    }
    chart->setTitle(ss.str().c_str());

    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(rowLabels);
    chart->createDefaultAxes();
    chart->setAxisX(axis, series);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    QAbstractAxis *axisX = chart->axisX();
    QFont titleFont = axisX->titleFont();
    titleFont.setPointSize(12);
    titleFont.setWeight(QFont::Bold);
    axisX->setTitleFont(titleFont);
    axisX->setTitleText(xLabel.c_str());
    if (rowLabels.count() > nmfConstants::NumCategoriesForVerticalNotation)
        axis->setLabelsAngle(-90);
    else
        axis->setLabelsAngle(0);

    // Rescale vertical axis....0 to 1 in increments of 0.2
    QValueAxis *newAxisY = new QValueAxis();
    newAxisY->setTitleFont(titleFont);
    newAxisY->setTitleText(yLabel.c_str());
    newAxisY->setRange(0,1.0);
    newAxisY->setTickCount(6);
    chart->setAxisY(newAxisY,series);

    // Set grid line visibility
    chart->axisX()->setGridLineVisible(GridLines[0]);
    chart->axisY()->setGridLineVisible(GridLines[1]);
}



void
ChartStackedBar::populateChart(
        QChart *chart,
        const boost::numeric::ublas::matrix<double> &ChartData,
        const QStringList &RowLabels,
        const QStringList &ColumnLabels,
        std::string &MainTitle,
        std::string &XTitle,
        std::string &YTitle,
        const std::vector<bool> &GridLines,
        const int Theme)
{
    QBarSet           *newSet;
    QStackedBarSeries *series1 = new QStackedBarSeries();

    // Set current theme
    chart->setTheme(static_cast<QChart::ChartTheme>(Theme));

    // Load data into series and then add series to the chart
    for (unsigned int pred=0; pred<ChartData.size2(); ++pred) {
        newSet = new QBarSet((ColumnLabels[pred]));
        for (unsigned int val=0; val<ChartData.size1(); ++val) {
            *newSet << ChartData(val,pred);
        }
        series1->append(newSet);
    }
    chart->addSeries(series1);

    // Set main title
    QStackedBarSeries *series = NULL; // ???
    QFont mainTitleFont = chart->titleFont();
    mainTitleFont.setPointSize(14);
    mainTitleFont.setWeight(QFont::Bold);
    chart->setTitleFont(mainTitleFont);
    chart->setTitle(QString::fromStdString(MainTitle));

    // Setup X and Y axes
    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(RowLabels);
    chart->createDefaultAxes();
    chart->setAxisX(axis, series);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    QAbstractAxis *axisX = chart->axisX();
    QFont titleFont = axisX->titleFont();
    titleFont.setPointSize(12);
    titleFont.setWeight(QFont::Bold);
    axisX->setTitleFont(titleFont);
    axisX->setTitleText(QString::fromStdString(XTitle));
    if (RowLabels.count() > nmfConstants::NumCategoriesForVerticalNotation)
        axis->setLabelsAngle(-90);
    else
        axis->setLabelsAngle(0);

    // Rescale vertical axis....0 to 1 in increments of 0.2
    QValueAxis *newAxisY = new QValueAxis();
    newAxisY->setTitleFont(titleFont);
    newAxisY->setTitleText(QString::fromStdString(YTitle));
    newAxisY->setRange(0,1.0);
    newAxisY->setTickCount(6);
    chart->setAxisY(newAxisY,series);

    // Set grid line visibility
    chart->axisX()->setGridLineVisible(GridLines[0]);
    chart->axisY()->setGridLineVisible(GridLines[1]);
}


std::string
ChartStackedBar::constructMainTitle(
        const std::string &Species,
        const bool        &rearrangeTitle,
        const std::string &DataTypeLabel,
        const std::string &XLabel,
        const std::string &TitlePrefix,
        const std::string &TitleSuffix)
{
    std::string MainTitle="";
    std::stringstream ss;

    if (rearrangeTitle) {
        ss << TitlePrefix << " " << DataTypeLabel << " " << Species;
    } else {
        ss << TitlePrefix << " " << Species << " " << DataTypeLabel <<
              " By " << XLabel << TitleSuffix;
    }

    return ss.str();
}






