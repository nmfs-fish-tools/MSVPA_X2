
//============================================================================
// Name        : nmfMSVPAProgressWidget.cpp
// Author      : Ron Klasky
// Version     :
// Copyright   : NOAA - National Marine Fisheries Service
// Description : Widget containing logic for Progress Dock Widget
//============================================================================

#include "nmfMSVPAProgressWidget.h"
#include "nmfConstants.h"

#include <QtCharts/QChartView>
#include <QtCharts/QChart>

QT_CHARTS_USE_NAMESPACE

nmfMSVPAProgressWidget::nmfMSVPAProgressWidget(QTimer *theTimer,
                                     nmfLogger *theLogger,
                                     std::string runType,
                                     QString Title,
                                     QString XAxisLabel,
                                     QString YAxisLabel)
{
    QString whatsThis;

    logger  = theLogger;
    timer   = theTimer;
    RunType = runType;

    // Create layouts and widgets
    hMainLayt   = new QHBoxLayout();
    vChartLayt  = new QVBoxLayout();
    hMinLayt    = new QHBoxLayout();
    hMaxLayt    = new QHBoxLayout();
    hRangeLayt  = new QHBoxLayout();
    vGroupLayt  = new QVBoxLayout();
    buttonLayt  = new QHBoxLayout();
    chartView   = new QChartView();
    controlsGB  = new QGroupBox();
    statusLBL   = new QLabel();
    pointLBL    = new QLabel("Points:");
    labelsCB    = new QCheckBox("Labels");
    rangeLBL    = new QLabel("Iterations (X) Axis:");
    minLBL      = new QLabel("Min:");
    maxLBL      = new QLabel("Max:");
    minSB       = new QSpinBox();
    maxSB       = new QSpinBox();
    clearPB     = new QPushButton("Clear Chart");
    stopPB      = new QPushButton("Stop");

    // Add widgets to layouts
    vChartLayt->addWidget(chartView);
    vChartLayt->addWidget(statusLBL);
    hMainLayt->addLayout(vChartLayt);
    hMainLayt->addWidget(controlsGB);
    hRangeLayt->addWidget(minLBL);
    hRangeLayt->addWidget(minSB);
    hRangeLayt->addWidget(maxLBL);
    hRangeLayt->addWidget(maxSB);
    buttonLayt->addWidget(clearPB);
    buttonLayt->addWidget(stopPB);
    vGroupLayt->addWidget(pointLBL);
    vGroupLayt->addWidget(labelsCB);
    vGroupLayt->addWidget(rangeLBL);
    vGroupLayt->addLayout(hRangeLayt);
    //vGroupLayt->addLayout(hMaxLayt);
    vGroupLayt->addSpacerItem(new QSpacerItem(1,2,QSizePolicy::Minimum,QSizePolicy::Expanding));
    vGroupLayt->addLayout(buttonLayt);

    // Get the chart as we'll need it later
    chart = chartView->chart();

    // Set defaults
    controlsGB->setLayout(vGroupLayt);
    controlsGB->setTitle("Controls");
    minSB->setSingleStep(10);
    maxSB->setSingleStep(10);
    minSB->setValue(0);
    maxSB->setValue(20);
    chart->legend()->hide();
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setFrameShape(QFrame::Box);
    chart->setContentsMargins(-8,-8,-3,-3); // left, top, right, bottom
    pointLBL->setToolTip("Toggles the point labels which represent (iteration number, convergence value)");
    pointLBL->setStatusTip("Toggles the point labels which represent (iteration number, convergence value)");
    labelsCB->setToolTip("Toggles the point labels which represent (iteration number, convergence value)");
    labelsCB->setStatusTip("Toggles the point labels which represent (iteration number, convergence value)");
    minSB->setToolTip("This is the minimum iteration value on the X axis.");
    minSB->setStatusTip("This is the minimum iteration value on the X axis.");
    minLBL->setToolTip("This is the minimum iteration value on the X axis.");
    minLBL->setStatusTip("This is the minimum iteration value on the X axis.");
    maxSB->setToolTip("This is the maximum iteration value on the X axis.");
    maxSB->setStatusTip("This is the maximum iteration value on the X axis.");
    maxLBL->setToolTip("This is the maximum iteration value on the X axis.");
    maxLBL->setStatusTip("This is the maximum iteration value on the X axis.");
    rangeLBL->setToolTip("Sets the Min/Max visible Iterations range for the chart.");
    rangeLBL->setStatusTip("Sets the Min/Max visible Iterations range for the chart.");
    stopPB->setToolTip("Stop the current model run.");
    stopPB->setStatusTip("Stop the current model run.");
    clearPB->setToolTip("Clears the chart of all previous plots.");
    clearPB->setStatusTip("Clears the chart of all previous plots.");

    // Show empty grid
    QValueAxis *newXAxis = new QValueAxis();
    QValueAxis *newYAxis = new QValueAxis();
    chart->setAxisX(newXAxis, new QLineSeries());
    chart->setAxisY(newYAxis, new QLineSeries());
    newXAxis->setTickCount(5);
    chart->axisX()->setRange(0, 20);
    chart->axisY()->setRange(0, 200);
    newXAxis->applyNiceNumbers();
    newYAxis->applyNiceNumbers();
    newXAxis->setLabelFormat("%d");
    newYAxis->setLabelFormat("%d");
    newXAxis->setTitleText(XAxisLabel);
    newYAxis->setTitleText(YAxisLabel);
    if (RunType == "MSVPA") {
        whatsThis = "<strong><center>Progress Chart</center></strong><p>This chart plots the convergence value per loop ";
        whatsThis += "iteration. Once the convergence value is less than 3, ";
        whatsThis += "the model is said to converge. ";
        whatsThis += "The convergence value is the number of differences between ";
        whatsThis += "the M2 matrix at iteration (i) and (i+1).</p>";
    } else if (RunType == "Forecast") {
        whatsThis = "<strong><center>Progress Chart</center></strong><p>This chart plots the Total Biomass per ";
        whatsThis += "Forecast year of the specified species.</p>";
    }
    chartView->setWhatsThis(whatsThis);
    chart->removeAllSeries();
    chart->setTitle(Title);

    SetupConnections();

}

nmfMSVPAProgressWidget::~nmfMSVPAProgressWidget() {

}

void
nmfMSVPAProgressWidget::updateChart()
{
    chart->update();
}

void
nmfMSVPAProgressWidget::SetupConnections()
{
    stopPB->disconnect();
    clearPB->disconnect();
    labelsCB->disconnect();
    minSB->disconnect();
    maxSB->disconnect();

    connect(stopPB,   SIGNAL(clicked(bool)),     this, SLOT(callback_stopPB(bool)));
    connect(clearPB,  SIGNAL(clicked(bool)),     this, SLOT(callback_clearPB(bool)));
    connect(labelsCB, SIGNAL(stateChanged(int)), this, SLOT(callback_labelsCB(int)));
    connect(minSB,    SIGNAL(valueChanged(int)), this, SLOT(callback_minSB(int)));
    connect(maxSB,    SIGNAL(valueChanged(int)), this, SLOT(callback_maxSB(int)));

} // end SetupConnections

void
nmfMSVPAProgressWidget::startTimer()
{
    logger->logMsg(nmfConstants::Normal,"Start " + RunType + " Progress Chart Timer");

//    QString title = "<b>Convergence Values as a Function of Iteration<\b>";
//    chart->setTitle(title);

    // Start Progress Chart's timer here
    timer->start(1000);
    StartRun();

} // end startTimer

void
nmfMSVPAProgressWidget::callback_stopPB(bool unused)
{
    logger->logMsg(nmfConstants::Normal,"Stop " + RunType + " Progress Chart Timer");
    timer->stop();
    StopRun();

    if (RunType == "MSVPA") {
        updateChartDataLabel(nmfConstantsMSVPA::MSVPAProgressChartLabelFile,
                             "<b>Status:&nbsp;&nbsp;</b>User halted MSVPA run. Output data incomplete.");
    } else if (RunType == "Forecast") {
        updateChartDataLabel(nmfConstantsMSVPA::ForecastProgressChartLabelFile,
                             "<b>Status:&nbsp;&nbsp;</b>User halted Forecast run. Output data incomplete.");
    }

} // end callback_stopPB




void
nmfMSVPAProgressWidget::callback_clearPB(bool unused) {

    chart->removeAllSeries();

    // Initialize progress output file
    if (RunType == "MSVPA") {
        std::ofstream outputFile(nmfConstantsMSVPA::MSVPAProgressChartFile);
        outputFile.close();
    } else if (RunType == "Forecast") {
        std::ofstream outputFile(nmfConstantsMSVPA::ForecastProgressChartFile);
        outputFile.close();
    }

} // end callback_clearPB

void
nmfMSVPAProgressWidget::callback_labelsCB(int state)
{

    if (isStopped()) {
        QLineSeries *lineSeries;
        QList<QAbstractSeries *> allSeries = chart->series();
        for (int i=0; i<allSeries.count(); ++i) {
            lineSeries = qobject_cast<QLineSeries *>(allSeries[i]);
            lineSeries->setPointLabelsVisible(state == Qt::Checked);
        }
        chartView->update();
        chartView->repaint();
        chart->update();
    }

} // end callback_labelsCB

void
nmfMSVPAProgressWidget::callback_minSB(int value) {

    if (isStopped()) {
        chart->axisX()->setMin(value);
        chart->update();
    }

} // end callback_minSB

void
nmfMSVPAProgressWidget::callback_maxSB(int value) {

    if (isStopped()) {
        chart->axisX()->setMax(value);
        chart->update();
    }

} // end callback_maxSB

void
nmfMSVPAProgressWidget::callback_lineHovered(QPointF point, bool state)
{
    QString msg;
    if (state) {
        msg = QString::number(point.x()) + "," +
                QString::number(point.y());
        chartView->setToolTip(msg);
    }
} // end callback_lineHovered

void
nmfMSVPAProgressWidget::callback_scatterSeriesHovered(QPointF point,bool state)
{
    if (state) {
        QString newToolTip = QString::number(point.x()) + "," + QString::number(point.y());
        chart->setToolTip(newToolTip);
        //std::cout << "POINT: " << point.x() << ", " << point.y() << std::endl;
    } else {
        chart->setToolTip("");
    }

} // end callback_scatterSeriesHovered


void
nmfMSVPAProgressWidget::StopRun()
{
    if (RunType == "MSVPA") {
        std::ofstream outputFile(nmfConstantsMSVPA::MSVPAStopRunFile);
        outputFile << "Stop" << std::endl;
        outputFile.close();
    } else if (RunType == "Forecast") {
        std::ofstream outputFile(nmfConstantsMSVPA::ForecastStopRunFile);
        outputFile << "Stop" << std::endl;
        outputFile.close();
    }

    logger->logMsg(nmfConstants::Bold,RunType + " Run - End");
    logger->logMsg(nmfConstants::Section,"================================================================================");


} // end StopRun

void
nmfMSVPAProgressWidget::StartRun()
{
    if (RunType == "MSVPA") {
        std::ofstream outputFile(nmfConstantsMSVPA::MSVPAStopRunFile);
        outputFile << "Start" << std::endl;
        outputFile.close();
    } else {
        std::ofstream outputFile(nmfConstantsMSVPA::ForecastStopRunFile);
        outputFile << "Start" << std::endl;
        outputFile.close();
    }
} // end StartRun

bool
nmfMSVPAProgressWidget::isStopped()
{
    std::string cmd;

    if (RunType == "MSVPA") {
        std::ifstream inputFile(nmfConstantsMSVPA::MSVPAStopRunFile);
        if (inputFile) {
            std::getline(inputFile,cmd);
        }
        inputFile.close();
    } else if (RunType == "Forecast") {
        std::ifstream inputFile(nmfConstantsMSVPA::ForecastStopRunFile);
        if (inputFile) {
            std::getline(inputFile,cmd);
        }
        inputFile.close();
    }

    return (cmd == "Stop");

} // end isStopped


void
nmfMSVPAProgressWidget::updateChartDataLabel(std::string inputLabelFileName,
                                        std::string overrideMsg)
{
    // Update progress chart label
    QString msg;
    std::string line;

    std::ifstream inputLabelFile(inputLabelFileName);
    std::getline(inputLabelFile,line);
    if (line.empty()) {
        msg.clear();
    } else {
        msg = "<b>Status:&nbsp;&nbsp;</b>" + QString::fromStdString(line);
    }
    if (overrideMsg.empty())
        statusLBL->setText(msg);
    else
        statusLBL->setText(QString::fromStdString(overrideMsg));
    inputLabelFile.close();

} // end updateChartDataLabel


void
nmfMSVPAProgressWidget::readChartDataFile(std::string type,
                                     std::string inputFileName,
                                     std::string inputLabelFileName,
                                     std::string title,
                                     std::string xLabel,
                                     std::string yLabel)
{
    // Read progress file that has all of the convergence values in
    // it and draw a line chart.
    int maxXRange;
    int minXRange;
    int numPlots;
    QValueAxis *newXAxis = NULL;
    QValueAxis *newYAxis = NULL;
    std::string line;
    std::vector<std::string> parts;
    //QScatterSeries *sseries = NULL;
    QLineSeries    *lseries = NULL;
    QList<QColor> colors = {QColor(100,100,255),
                            QColor(255,100,100),
                            QColor(0,0,0),
                            Qt::lightGray,
                            Qt::darkGray};
    QList<double> sizes  = {10.0, 10.0, 8.0, 8.0, 6.0};
    QList<QScatterSeries::MarkerShape> shapes = {
        QScatterSeries::MarkerShapeRectangle,
        QScatterSeries::MarkerShapeCircle,
        QScatterSeries::MarkerShapeRectangle,
        QScatterSeries::MarkerShapeCircle,
        QScatterSeries::MarkerShapeRectangle
    };

    updateChartDataLabel(inputLabelFileName,"");


    // try to hide QLineSeries from legend
    // RSK use QPen dash, dot variations with color for various line types
    // Show legend to the right.
    chart->legend()->show();
    chart->legend()->setAlignment(Qt::AlignRight);

    chart->update();
    chartView->repaint();
    updateChart();


    // This is so that if the user specifies the exact same plot again, it'll
    // draw as a new plot and not as a continuation of the original plot. Why?
    // This would be desirable if a person first runs a Forecast with 5 years and
    // then runs the exact same Forecast with 10 years.
    int lastX=0;

    int x;
    std::string species="";
    std::string theMSVPAName="",lastMSVPAName="";
    std::string theForecastName="";
    std::string theScenarioName="";
    std::ifstream inputFile(inputFileName);
    std::vector<std::vector<std::string> > chartData;
    std::vector<QPoint> readVector;
    std::vector<QPoint> pointVector;
    std::vector<std::string> strVector;
    bool isFirstLine = true;
    while (std::getline(inputFile,line)) {
        boost::split(parts,line,boost::is_any_of(","));
        if (type == "MSVPA") { // Means we're looking at MSVPA data
            theMSVPAName = parts[0];
            x = std::stoi(parts[1]);
            //y = std::stoi(parts[2]);
            if (isFirstLine || ((theMSVPAName == lastMSVPAName) && (lastX < x))) {
                //pointVector.push_back(QPoint(x,y));
                strVector.push_back(line);
            } else {
                chartData.push_back(strVector);
                strVector.clear();
                strVector.push_back(line);
            }
            lastMSVPAName = theMSVPAName;
            lastX = x;
            isFirstLine = false;
        }
        else if (type == "Forecast") { // Means we're looking at Forecast data
            theMSVPAName    = parts[0];
            theForecastName = parts[1];
            theScenarioName = parts[2];
            x = std::stoi(parts[3]);
            //y = std::stoi(parts[5]);
            species = parts[4];
            if (isFirstLine || ((theMSVPAName == lastMSVPAName) && (lastX < x))) {
                strVector.push_back(line);
            } else {
                chartData.push_back(strVector);
                strVector.clear();
                strVector.push_back(line);
            }
            lastMSVPAName = theMSVPAName;
            lastX = x;
            isFirstLine = false;
        }
    }

    if (strVector.size() > 0)
        chartData.push_back(strVector);
    numPlots = chartData.size();
    if (numPlots == 0)
        return;



    // Add points to series
    int numExistingPoints;
    int numPointsRead;
    QList<QAbstractSeries *> allSeries = chart->series();

    if (allSeries.count() == 0) {
        lseries = new QLineSeries();
        // Number aren't correct when they appear.  Could be a bug in 5.8, try
        // it in 5.10
//         connect(lseries, SIGNAL(hovered(QPointF,bool)),
//                 this, SLOT(callback_lineHovered(QPointF,bool)));
        strVector = chartData[0];
        boost::split(parts,strVector[0],boost::is_any_of(","));
        lseries->setName(QString::fromStdString(parts[0]));
        if (type == "MSVPA")
            lseries->append(std::stoi(parts[1]),std::stoi(parts[2]));
        else if (type == "Forecast") {
            lseries->append(std::stoi(parts[3]),std::stoi(parts[5]));
            species = parts[4];
        }
        chart->addSeries(lseries);
        chart->createDefaultAxes();
    } else if (allSeries.count() < numPlots) {
        lseries = new QLineSeries();
//        connect(lseries, SIGNAL(hovered(QPointF,bool)),
//                this, SLOT(callback_lineHovered(QPointF,bool)));
        strVector = chartData[allSeries.count()];
        boost::split(parts,strVector[0],boost::is_any_of(","));
        lseries->setName(QString::fromStdString(parts[0]));
        if (type == "MSVPA")
            lseries->append(std::stoi(parts[1]),std::stoi(parts[2]));
        else if (type == "Forecast") {
            lseries->append(std::stoi(parts[3]),std::stoi(parts[5]));
            species = parts[4];
        }
        chart->addSeries(lseries);
        chart->createDefaultAxes();

    } else {
       lseries = qobject_cast<QLineSeries *>(allSeries[chartData.size()-1]);

       strVector = chartData[chartData.size()-1];
       numExistingPoints = lseries->count();
       numPointsRead     = strVector.size();
       if (numExistingPoints < numPointsRead) {
           for (int i=numExistingPoints; i<numPointsRead; ++i) {
                boost::split(parts,strVector[i],boost::is_any_of(","));
                lseries->setName(QString::fromStdString(parts[0]));
                if (type == "MSVPA")
                    lseries->append(std::stoi(parts[1]),std::stoi(parts[2]));
                else if (type == "Forecast") {
                    lseries->append(std::stoi(parts[3]),std::stoi(parts[5]));
                    species = parts[4];
                }
           }
       }
       // Necessary for the chart to update
       chart->update(); // necessary for labels to paint completely
    }

    QLineSeries *lineSeries=NULL;
    QColor whichColor;
    newXAxis = new QValueAxis();
    newYAxis = new QValueAxis();
    allSeries = chart->series();
    for (int i=0; i<numPlots; ++i) {
        lineSeries    = qobject_cast<QLineSeries *>(allSeries[i]);
        whichColor    = colors[i % colors.count()];
        lineSeries->setPointsVisible(true);
        lineSeries->setPointLabelsVisible(labelsCB->isChecked());
        lineSeries->setPen(QPen(whichColor,2));
        lineSeries->setColor(whichColor);

        //scatterSeries = sseriesList[i];
        //chart->setAxisX(newXAxis, scatterSeries);
        chart->setAxisX(newXAxis, lineSeries);
        //chart->setAxisY(newYAxis, scatterSeries);
        chart->setAxisY(newYAxis, lineSeries);
    }


    // Find max y value in lineSeries
    int numPoints = lineSeries->count();
    double maxY = 0;
    double yValue;
    for (int i=0; i<numPoints; ++i) {
        yValue = lineSeries->at(i).y();
        if (yValue > maxY)
            maxY = yValue;
    }


    // Set axis to be last set of series...should be OK I think...since
    // all series should have approximately the same ranges.
    int maxValue = maxSB->value();
    newXAxis->setTickCount(5);
    maxXRange = int(int(lineSeries->count()/10.0))*10.0+10;
    maxXRange = (maxXRange <= maxValue) ? maxValue : maxXRange;
    maxSB->setValue(maxXRange);
    minXRange = minSB->value();
    if (minXRange >= maxXRange) {
        minXRange = maxXRange-20;
        minSB->setValue(minXRange);
    }
//    maxXRange = ProgressLastIterationSB->value();
//    if (maxXRange <= minXRange) {
//        maxXRange = minXRange+20;
//        ProgressLastIterationSB->setValue(maxXRange);
//    }
    chart->axisX()->setRange(minXRange, maxXRange);
    chart->axisY()->setRange(0, maxY);
    //chart->axisY()->setMin(0);
    newYAxis->applyNiceNumbers();
    newXAxis->applyNiceNumbers();
    newXAxis->setLabelFormat("%d");
    newYAxis->setLabelFormat("%d");
    newXAxis->setTitleText(QString::fromStdString(xLabel)); //"Iterations (i)");
    newYAxis->setTitleText(QString::fromStdString(yLabel)); //"Convergence Value");
    if (type == "Forecast")
        title += species;
    chart->setTitle(QString::fromStdString(title));

    chartView->repaint();

    return;

} // end readChartDataFile




