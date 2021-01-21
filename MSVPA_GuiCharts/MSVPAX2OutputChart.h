#ifndef MSVPAX2OUTPUTCHART_H
#define MSVPAX2OUTPUTCHART_H

#include "nmfStructsQt.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

QT_CHARTS_USE_NAMESPACE


class MSVPAX2OutputChart : public QWidget
{
    Q_OBJECT

    QChart*     chart;
    QChartView* chartView;

public:
    explicit MSVPAX2OutputChart(QWidget *parent = 0);
    QChart*     getChart();
    QChartView* getChartView();
    QChartView* widget();

    bool populateDataTable(
            QTableWidget* dataTable,
            boost::numeric::ublas::matrix<double> &GridData,
            QStringList RowLabels,
            QStringList ColumnLabels,
            int numDigits,
            int numDecimals);

    void restoreTitleFonts();


public slots:
    virtual void callback_UpdateChart(nmfStructsQt::UpdateDataStruct data) = 0;
    void callback_UpdateChartGridLines(nmfStructsQt::UpdateDataStruct data);
    void callback_UpdateTheme(int theme);

};

#endif // MSVPAX2OUTPUTCHART_H
