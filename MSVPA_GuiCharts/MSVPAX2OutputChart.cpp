#include "nmfStructsQt.h"
#include "MSVPAX2OutputChart.h"

MSVPAX2OutputChart::MSVPAX2OutputChart(QWidget *parent) : QWidget(parent)
{
  chart     = new QChart();
  chartView = new QChartView(chart);
  chartView->setRenderHint(QPainter::Antialiasing);
}

QChart*
MSVPAX2OutputChart::getChart()
{
    return chart;
}

QChartView*
MSVPAX2OutputChart::getChartView()
{
    return chartView;
}

QChartView*
MSVPAX2OutputChart::widget()
{
    return chartView;
}

void
MSVPAX2OutputChart::restoreTitleFonts()
{
    QFont mainTitleFont = chart->titleFont();
    mainTitleFont.setPointSize(14);
    mainTitleFont.setWeight(QFont::Bold);
    chart->setTitleFont(mainTitleFont);

    QAbstractAxis *axisX = chart->axisX();
    QFont titleFont = axisX->titleFont();
    titleFont.setPointSize(12);
    titleFont.setWeight(QFont::Bold);
    axisX->setTitleFont(titleFont);

    QAbstractAxis *axisY = chart->axisY();
    titleFont = axisY->titleFont();
    titleFont.setPointSize(12);
    titleFont.setWeight(QFont::Bold);
    axisY->setTitleFont(titleFont);
}

void
MSVPAX2OutputChart::callback_UpdateTheme(int theme)
{
    chart->setTheme(static_cast<QChart::ChartTheme>(theme));
    restoreTitleFonts();
}

void
MSVPAX2OutputChart::callback_UpdateChartGridLines(nmfStructsQt::UpdateDataStruct data)
{
    std::vector<bool> GridLines = { data.HorizontalGridLines,
                                    data.VerticalGridLines };

    // Set grid line visibility
    chart->axisY()->setGridLineVisible(GridLines[0]);
    chart->axisX()->setGridLineVisible(GridLines[1]);
}


bool
MSVPAX2OutputChart::populateDataTable(
        QTableWidget* dataTable,
        boost::numeric::ublas::matrix<double> &GridData,
        QStringList RowLabels,
        QStringList ColumnLabels,
        int numDigits,
        int numDecimals)
{
    char buf[100];
    int numRows = GridData.size1();
    int numCols = GridData.size2();
    int numRowLabels = RowLabels.size();
    int numColLabels = ColumnLabels.size();

    QTableWidgetItem *item;
    std::string format = "%"+std::to_string(numDigits)+"."+std::to_string(numDecimals)+"f";

    if ((GridData.size1() == 0) && (GridData.size2() == 0)) {
        std::cout << "Note: No data found in GridData. This may be OK, if there's no associated data for the selected Species." << std::endl;
        return false;
    }

    if ((numRows <= 0) || (numRowLabels <= 0) ||
        (numCols <= 0) || (numColLabels <= 0) ||
        (numRows != numRowLabels) || (numCols != numColLabels))
    {
        std::cout << "ERROR: Mismatched row and/or col size:" << std::endl;
        std::cout << "       GridData: " << GridData.size1() << ", " << GridData.size2() << std::endl;
        std::cout << "       ColLabels size: " << ColumnLabels.size() << std::endl;
        std::cout << "       RowLabels size: " << RowLabels.size() << std::endl;
        return false;
    }

    dataTable->clear();
    dataTable->setRowCount(numRows);
    dataTable->setColumnCount(numCols);

    for (int j=0; j<numCols; ++j) {
        dataTable->setHorizontalHeaderItem(j, new QTableWidgetItem(ColumnLabels[j]));
    }

    for (int i=0; i<numRows; ++i) {
        dataTable->setVerticalHeaderItem(i, new QTableWidgetItem(RowLabels[i]));
        for (int j=0; j<numCols; ++j) {
            sprintf(buf,format.c_str(),GridData(i,j));
            std::string valStr(buf);
            item = new QTableWidgetItem(QString::fromStdString(valStr));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            dataTable->setItem(i, j, item);
        }
    }
    dataTable->resizeColumnsToContents();

    return true;
}
