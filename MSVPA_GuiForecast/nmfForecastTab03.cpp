
#include "nmfConstants.h"
#include "nmfForecastTab03.h"


nmfForecastTab3::nmfForecastTab3(QTabWidget  *tabs,
                                 nmfLogger   *theLogger,
                                 std::string &theProjectDir)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfForecastTab3::nmfForecastTab3");

    NYears = 0;
    NSpecies = 0;
    initialized = false;
    chartTopRight = NULL;
    chartBottomRight = NULL;
    readOnlyTables = true;
    Forecast_Tabs = tabs;
    ProjectDir = theProjectDir;

    // Load ui as a widget from disk
    QFile file(":/forms/Forecast/Forecast_Tab03.ui");
    file.open(QFile::ReadOnly);
    Forecast_Tab3_Widget = loader.load(&file,Forecast_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    Forecast_Tabs->addTab(Forecast_Tab3_Widget, tr("3. Stock Recruit Parameters"));

    Forecast_Tab3_LoadPB           = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab3_LoadPB");
    Forecast_Tab3_ClearPB          = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab3_ClearPB");
    Forecast_Tab3_FitSRCurveSavePB = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab3_FitSRCurveSavePB");
    Forecast_Tab3_PrevPB           = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab3_PrevPB");
    Forecast_Tab3_NextPB           = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab3_NextPB");
    Forecast_Tab3_RecruitCurveCB   = Forecast_Tabs->findChild<QCheckBox    *>("Forecast_Tab3_RecruitCurveCB");
    Forecast_Tab3_RecruitCurveCMB  = Forecast_Tabs->findChild<QComboBox    *>("Forecast_Tab3_RecruitCurveCMB");
    Forecast_Tab3_ParameterGB      = Forecast_Tabs->findChild<QGroupBox    *>("Forecast_Tab3_ParameterGB");
    Forecast_Tab3_ParameterW       = Forecast_Tabs->findChild<QWidget      *>("Forecast_Tab3_ParameterW");
    Forecast_Tab3_ParameterTE      = Forecast_Tabs->findChild<QTextEdit    *>("Forecast_Tab3_ParameterTE");
    Forecast_Tab3_QuartileTbW      = Forecast_Tabs->findChild<QTableWidget *>("Forecast_Tab3_QuartileTbW");
    Forecast_Tab3_KLBL             = Forecast_Tabs->findChild<QLabel       *>("Forecast_Tab3_KLBL");
    Forecast_Tab3_SpeciesCMB       = Forecast_Tabs->findChild<QComboBox    *>("Forecast_Tab3_SpeciesCMB");
    Forecast_Tab3_DataTbW          = Forecast_Tabs->findChild<QTableWidget *>("Forecast_Tab3_DataTbW");
    Forecast_Tab3_AlphaLE          = Forecast_Tabs->findChild<QLineEdit    *>("Forecast_Tab3_AlphaLE");
    Forecast_Tab3_BetaLE           = Forecast_Tabs->findChild<QLineEdit    *>("Forecast_Tab3_BetaLE");
    Forecast_Tab3_KLE              = Forecast_Tabs->findChild<QLineEdit    *>("Forecast_Tab3_KLE");
    Forecast_Tab3_TopRightLayout   = Forecast_Tabs->findChild<QVBoxLayout  *>("Forecast_Tab3_TopRightLayout");
    Forecast_Tab3_BottomRightLayout= Forecast_Tabs->findChild<QVBoxLayout  *>("Forecast_Tab3_BottomRightLayout");
    Forecast_Tab3_StockRecruitParametersGB = Forecast_Tabs->findChild<QGroupBox  *>("Forecast_Tab3_StockRecruitParametersGB");
    Forecast_Tab3_RecalcPB         = Forecast_Tabs->findChild<QPushButton  *>("Forecast_Tab3_RecalcPB");

    Forecast_Tab3_ParameterW->hide();
    Forecast_Tab3_QuartileTbW->hide();
    Forecast_Tab3_KLBL->hide();
    Forecast_Tab3_KLE->hide();
    Forecast_Tab3_StockRecruitParametersGB->hide();
    Forecast_Tab3_RecalcPB->hide();

    Forecast_Tab3_PrevPB->setText("\u25C1--");
    Forecast_Tab3_NextPB->setText("--\u25B7");

    connect(Forecast_Tab3_PrevPB,  SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab3_PrevPB(bool)));
    connect(Forecast_Tab3_NextPB,  SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab3_NextPB(bool)));
    connect(Forecast_Tab3_FitSRCurveSavePB,  SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab3_FitSRCurveSavePB(bool)));

    connect(Forecast_Tab3_RecruitCurveCB,  SIGNAL(stateChanged(int)),
            this,                  SLOT(callback_Forecast_Tab3_RecruitCurveCB(int)));
    connect(Forecast_Tab3_RecruitCurveCMB, SIGNAL(currentIndexChanged(int)),
            this,                  SLOT(callback_Forecast_Tab3_RecruitCurveCMB(int)));
    connect(Forecast_Tab3_SpeciesCMB, SIGNAL(currentIndexChanged(int)),
            this,                     SLOT(callback_Forecast_Tab3_SpeciesCMB(int)));

    Forecast_Tab3_DataTbW->setToolTip("These data are derived from values in the following tables:\nSpeMaturity\nMSVPASeasBiomass");
    Forecast_Tab3_DataTbW->setStatusTip("These data are derived from values in the following tables:\nSpeMaturity\nMSVPASeasBiomass");
    Forecast_Tab3_QuartileTbW->setToolTip("These data are derived from values in the following tables:\nSpeMaturity\nMSVPASeasBiomass");
    Forecast_Tab3_QuartileTbW->setStatusTip("These data are derived from values in the following tables:\nSpeMaturity\nMSVPASeasBiomass");
    Forecast_Tab3_LoadPB->hide();
    Forecast_Tab3_ClearPB->hide();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab3::nmfForecastTab3 Complete");

}


nmfForecastTab3::~nmfForecastTab3()
{

}


void
nmfForecastTab3::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir




//void
//nmfForecastTab2::callback_TableDataChanged(QModelIndex unused)
//{
//    emit TableDataChanged("ForeSRR");

//} // end callback_TableDataChanged

void
nmfForecastTab3::redrawTable()
{
    boost::numeric::ublas::matrix<double> SRFit;
    boost::numeric::ublas::matrix<double> SRTemp;

    QTableWidgetItem *item;
    if (! initialized)
        return;

    LinRegOut RegressOut;

    int SpeNum    = Forecast_Tab3_SpeciesCMB->currentIndex();
    int CurveType = Forecast_Tab3_RecruitCurveCMB->currentIndex();

    nmfUtils::initialize(SRFit,int(SSBMax(SpeNum)),2);
    nmfUtils::initialize(SRTemp,NYears+1,2);

    if (CurveType == Ricker) {

        // Do the transformation for the ricker curve
        for (int i = 0; i<= NYears-1; ++i) {
            SRTemp(i,0) = SRData[SpeNum][i][0]; // SSB values
            SRTemp(i,1) = std::log(SRData[SpeNum][i][1] / SRData[SpeNum][i][0]); // Y-values for Ricker Curve
        }

        if (Forecast_Tab3_RecruitCurveCB->isChecked()) {
        //if (UserDefinedSR(SpeNum)) {
            RegressOut.Alpha = Forecast_Tab3_AlphaLE->text().toDouble();
            RegressOut.Beta  = Forecast_Tab3_BetaLE->text().toDouble();
        } else {
            // This fills the public variable regress with all the info you need
            nmfUtilsStatistics::LinReg(SRTemp, NYears, RegressOut);
        }

        for (int i = 0; i <= NYears-1; ++i) {
            SRData[SpeNum][i][2] = std::exp(RegressOut.Alpha + RegressOut.Beta * SRTemp(i,0)) * SRTemp(i,0);
            SRData[SpeNum][i][3] = SRData[SpeNum][i][1] - SRData[SpeNum][i][2];
        }

        // 1. Load bottom left text edit
        loadRegressionWidget(RegressOut,false,0);

        // 2. Generate fitted values and load top right chart
        for (int i=0; i<int(SSBMax(SpeNum)); ++i) {
            SRFit(i,0) = i;
            SRFit(i,1) = std::exp(RegressOut.Alpha + RegressOut.Beta * i) * i;
        }
        loadSSBRecruitsChart(SRData, SRFit, SpeNum);

        // 3. Load bottom right chart
        loadYearResidualChart(SRData, SpeNum, "Year", "Residual");

/*
        // 4. Load output data structure
        SpeRegData(SpeNum).SpeIndex = SpeNum;
        SpeRegData(SpeNum).SpeName  = SpeList(SpeNum);
        SpeRegData(SpeNum).SRRType  = 0;
        SpeRegData(SpeNum).SRRA     = RegressOut.Alpha;
        SpeRegData(SpeNum).SRRB     = RegressOut.Beta;
        SpeRegData(SpeNum).SRRK     = 0;
        SpeRegData(SpeNum).Flag     = (RegressOut.Alpha <= 0) ? 1 : 0;
*/

    }  else if (Forecast_Tab3_RecruitCurveCMB->currentIndex() == BevertonHolt) {

        for (int i = 0; i <= NYears-1; ++i) {
            SRTemp(i,0) = SRData[SpeNum][i][0];
            SRTemp(i,1) = SRData[SpeNum][i][0] / SRData[SpeNum][i][1]; // S/R is the linear transform for the BH curve
        }

        if (Forecast_Tab3_RecruitCurveCB->isChecked()) {
            RegressOut.Alpha = Forecast_Tab3_AlphaLE->text().toDouble();
            RegressOut.Beta  = Forecast_Tab3_BetaLE->text().toDouble();
        } else {
            nmfUtilsStatistics::LinReg(SRTemp, NYears, RegressOut); // this fills the public variable regress with all the info you need
        }

        for (int i = 0; i <= NYears-1; ++i) {
            SRData[SpeNum][i][2] = SRTemp(i,0) / (RegressOut.Alpha + RegressOut.Beta * SRTemp(i, 0));
            SRData[SpeNum][i][3] = SRData[SpeNum][i][1] - SRData[SpeNum][i][2];
        }

        // 1. Update bottom left text edit
        loadRegressionWidget(RegressOut,false,0);

        // 2. Generate fitted values and load top right chart
        for (int i=0; i<int(SSBMax(SpeNum)); ++i) {
            SRFit(i,0) = i;
            SRFit(i,1) = i / (RegressOut.Alpha+RegressOut.Beta*i);
        }
        loadSSBRecruitsChart(SRData, SRFit, SpeNum);

        // 3. Load bottom right chart
        loadYearResidualChart(SRData, SpeNum, "Year", "Residual");

        /*
        // 4. Load output data structure
        SpeRegData(SpeNum).SpeIndex = SpeNum
        SpeRegData(SpeNum).SpeName  = SpeList(SpeNum)
        SpeRegData(SpeNum).SRRType  = 1
        SpeRegData(SpeNum).SRRA     = RegressOut.Alpha
        SpeRegData(SpeNum).SRRB     = RegressOut.Beta
        SpeRegData(SpeNum).SRRK     = 0
        SpeRegData(SpeNum).Flag     = (RegressOut.Alpha <= 0) ? 1 : 0;
        */

    }  else if (Forecast_Tab3_RecruitCurveCMB->currentIndex() == RandomFromQuartiles) {

        // 1. Load bottom left table.
        int numRows = 4;
        int numCols = 5;
        Forecast_Tab3_ParameterTE->hide();
        Forecast_Tab3_QuartileTbW->show();
        Forecast_Tab3_QuartileTbW->setRowCount(numRows);
        Forecast_Tab3_QuartileTbW->setColumnCount(numCols);
        QStringList hheaderNames,vheaderNames;
        hheaderNames << tr("Min SSB") << tr("Max SSB") << tr("Mean R") << tr("Min R") << tr("Max R");
        Forecast_Tab3_QuartileTbW->setHorizontalHeaderLabels(hheaderNames);
        vheaderNames << tr("Q1") << tr("Q2") << tr("Q3") << tr("Q4");
        Forecast_Tab3_QuartileTbW->setVerticalHeaderLabels(vheaderNames);
        for (int r=1; r<=numRows; ++r) {

            item = new QTableWidgetItem(QString::number(PdSSBQ(SpeNum,r-1),'f',1));
            item->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
            if (readOnlyTables)
                item->setFlags(item->flags() &  ~Qt::ItemIsEditable); // read-only
            Forecast_Tab3_QuartileTbW->setItem(r-1,0,item);

            item = new QTableWidgetItem(QString::number(PdSSBQ(SpeNum,r),'f',1));
            item->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
            if (readOnlyTables)
                item->setFlags(item->flags() &  ~Qt::ItemIsEditable); // read-only
            Forecast_Tab3_QuartileTbW->setItem(r-1,1,item);

            item = new QTableWidgetItem(QString::number(PdMeanRec(SpeNum,r),'f',1));
            item->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
            if (readOnlyTables)
                item->setFlags(item->flags() &  ~Qt::ItemIsEditable); // read-only
            Forecast_Tab3_QuartileTbW->setItem(r-1,2,item);

            item = new QTableWidgetItem(QString::number(PdMinRec(SpeNum,r),'f',1));
            item->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
            if (readOnlyTables)
                item->setFlags(item->flags() &  ~Qt::ItemIsEditable); // read-only
            Forecast_Tab3_QuartileTbW->setItem(r-1,3,item);

            item = new QTableWidgetItem(QString::number(PdMaxRec(SpeNum,r),'f',1));
            item->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
            if (readOnlyTables)
                item->setFlags(item->flags() &  ~Qt::ItemIsEditable); // read-only
            Forecast_Tab3_QuartileTbW->setItem(r-1,4,item);

        }
        Forecast_Tab3_QuartileTbW->resizeColumnsToContents();

        // 2. Generate fitted values and load top right chart
        loadSSBRecruitsChart(SRData, SRFit, SpeNum); // just a scatterplot..no line

        // 3. Load bottom right chart
        loadYearResidualChart(SRData, SpeNum, "Year", "R / SSB");

        /*
        // 4. Load output data structure
        SpeRegData(SpeNum).SpeIndex = SpeNum;
        SpeRegData(SpeNum).SpeName  = SpeList(SpeNum);
        SpeRegData(SpeNum).SRRType  = 2;
        SpeRegData(SpeNum).SRRA     = 0;
        SpeRegData(SpeNum).SRRB     = 0;
        SpeRegData(SpeNum).SRRK     = 0;
        SpeRegData(SpeNum).Flag     = 0;
        */


    }  else if (Forecast_Tab3_RecruitCurveCMB->currentIndex() == ShepherdFlexible) { // Shepherd flexible curve
        double ShepA;
        double ShepK;
        double ShepB;
        NLRegOut NLOutput;
        boost::numeric::ublas::matrix<double> NLData;
        boost::numeric::ublas::vector<double> NLOut;
        boost::numeric::ublas::vector<double> NLVarEst;

        nmfUtils::initialize(NLData,NYears+1,2);
        nmfUtils::initialize(NLOut,NYears+1);
        nmfUtils::initialize(NLVarEst,3+1);

        for (int i = 0; i <= NYears-1; ++i) {
         NLData(i,0) = SRData[SpeNum][i][0];
         NLData(i,1) = SRData[SpeNum][i][1];
        }

        if (Forecast_Tab3_RecruitCurveCB->isChecked()) {
            NLOutput.A = Forecast_Tab3_AlphaLE->text().toDouble();
            NLOutput.B = Forecast_Tab3_BetaLE->text().toDouble();
            NLOutput.K = Forecast_Tab3_KLE->text().toDouble();
        } else {
            nmfUtilsStatistics::ShepSRR(NLData, NYears, NLOutput);
        }

        ShepA = NLOutput.A;
        ShepB = NLOutput.B;
        ShepK = NLOutput.K;

        for (int i = 0; i <= NYears - 1; ++i) {
            NLOut(i) = (ShepA * NLData(i, 0)) / (1+std::pow((NLData(i,0)/ShepK),ShepB));
            SRData[SpeNum][i][2] = NLOut(i);
            SRData[SpeNum][i][3] = NLData(i,1) - NLOut(i);
        }

        // 1. Update bottom left text edit
        RegressOut.Alpha   = NLOutput.A;
        //RegressOut.AlphaSE = "";
        //RegressOut.AlphaT = "";
        RegressOut.Beta    = NLOutput.B;
        //RegressOut.BetaSE = "";
        //RegressOut.BetaT = "";
        RegressOut.K       = NLOutput.K;
        RegressOut.ErrorDF = NLOutput.ErrorDF;
        RegressOut.ErrorMS = NLOutput.ErrorMS;
        RegressOut.F       = NLOutput.F;
        RegressOut.RegDf   = 3;
        RegressOut.RegMS   = NLOutput.RegMS;
        RegressOut.ResSE   = NLOutput.ResSE;
        RegressOut.Rsquare = NLOutput.Rsquare;
        loadRegressionWidget(RegressOut,true,NLOutput.ItFlag);

        // 2. Generate fitted values and load top right chart
        for (int i=0; i<int(SSBMax(SpeNum)); ++i) {
            SRFit(i,0) = i;
            SRFit(i,1) = (ShepA * i) / (1.0 + std::pow((i/ShepK),ShepB));
        }
        loadSSBRecruitsChart(SRData, SRFit, SpeNum);

        // 3. Load bottom right chart
        loadYearResidualChart(SRData, SpeNum, "Year", "Residual");

        /*
        // 4. Load output data structure
        SpeRegData(SpeNum).SpeIndex = SpeNum;
        SpeRegData(SpeNum).SpeName  = SpeList(SpeNum);
        SpeRegData(SpeNum).SRRType  = 3;
        SpeRegData(SpeNum).SRRA     = NLOutPut.A;
        SpeRegData(SpeNum).SRRB     = NLOutPut.B;
        SpeRegData(SpeNum).SRRK     = NLOutPut.k;
        SpeRegData(SpeNum).Flag     = (NLOutPut.ItFlag == 2) ? 1 : 0;
        */
    }


    // Update top left table widget
    for (int i=0; i<=NYears; ++i) {

        item = new QTableWidgetItem(QString::number(SRData[SpeNum][i][0],'f',1));
        item->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
        if (readOnlyTables)
            item->setFlags(item->flags() &  ~Qt::ItemIsEditable); // read-only
        Forecast_Tab3_DataTbW->setItem(i,1,item);

        item = new QTableWidgetItem(QString::number(SRData[SpeNum][i][1],'f',1));
        item->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
        if (readOnlyTables)
            item->setFlags(item->flags() &  ~Qt::ItemIsEditable); // read-only
        Forecast_Tab3_DataTbW->setItem(i,2,item);

        item = new QTableWidgetItem(QString::number(SRData[SpeNum][i][2],'f',1));
        item->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
        if (readOnlyTables)
            item->setFlags(item->flags() &  ~Qt::ItemIsEditable); // read-only
        Forecast_Tab3_DataTbW->setItem(i /*+1*/,3,item);

        item = new QTableWidgetItem(QString::number(SRData[SpeNum][i][3],'f',2));
        item->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
        if (readOnlyTables)
            item->setFlags(item->flags() &  ~Qt::ItemIsEditable); // read-only
        Forecast_Tab3_DataTbW->setItem(i /*+1*/,4,item);

    }
    Forecast_Tab3_DataTbW->resizeColumnsToContents();



    // update bottom right scatter plot



} // end redrawTable


void
nmfForecastTab3::loadSSBRecruitsChart(Boost3DArrayDouble &SRData,
                                      boost::numeric::ublas::matrix<double> &SRFit,
                                      int &SpeNum)
{
    QValueAxis     *axisX;
    QValueAxis     *axisY;
    QLineSeries    *lineSeries    = new QLineSeries();
    QScatterSeries *scatterSeries = new QScatterSeries();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab3::loadSSBRecruitsChart");

    // Load series variables with appropriate data for chart display.
    for (int i=0; i<int(SSBMax(SpeNum)); ++i) {
         if (i <= NYears-1) {
//std::cout <<  "SpeNum: " << SpeNum << ", scatterSeries data: " << SRData[SpeNum][i][0] << ", " << SRData[SpeNum][i][1] << std::endl;
             scatterSeries->append(
                 SRData[SpeNum][i][0],
                 SRData[SpeNum][i][1]); // scatter points
         } else {
//             scatterSeries->append(0,0);
         }
         lineSeries->append(SRFit(i,0),SRFit(i,1));
    }

    // Create scatter chart with superimposed best fit curve.
    if (Forecast_Tab3_TopRightLayout->count() == 0) {
        chartTopRight = new QChart();
        chartTopRight->legend()->hide();
        chartTopRight->addSeries(scatterSeries);
        chartTopRight->addSeries(lineSeries);
        chartTopRight->createDefaultAxes();
        chartTopRight->setTitle("Plot of SSB vs Recruits");
        QChartView *chartView = new QChartView(chartTopRight);
        chartView->setRenderHint(QPainter::Antialiasing);
        Forecast_Tab3_TopRightLayout->addWidget(chartView);
    } else {
        axisX = qobject_cast<QValueAxis *>(chartTopRight->axes(Qt::Horizontal).at(0));
        axisY = qobject_cast<QValueAxis *>(chartTopRight->axes(Qt::Vertical).at(0));
        chartTopRight->removeAllSeries();
        chartTopRight->addSeries(scatterSeries);
        chartTopRight->addSeries(lineSeries);
        chartTopRight->createDefaultAxes();
    }

    // Create new X axis with "nice" numbers set on scale.
    axisX = new QValueAxis();
    axisX->setLabelFormat("%.1f");
    axisX->setTitleText("SSB (000 mt)");
    chartTopRight->setAxisX(axisX,scatterSeries);
    chartTopRight->setAxisX(axisX,lineSeries);
//    RSK - double check the following logic for the deprecated setAxisX above calls
//    chartTopRight->removeAxis(chartTopRight->axes(Qt::Horizontal).back());
//    chartTopRight->addAxis(axisX, Qt::AlignBottom);
//    scatterSeries->attachAxis(axisX);
//    chartTopRight->addAxis(axisX, Qt::AlignBottom);
//    lineSeries->attachAxis(axisX);
    axisX->applyNiceNumbers();

    // Create new Y axis with "nice" numbers set on scale.
    axisY = new QValueAxis();
    axisY->setLabelFormat("%.1f");
    axisY->setTitleText("Recruits (000 000)");
    chartTopRight->setAxisY(axisY,scatterSeries);
    chartTopRight->setAxisY(axisY,lineSeries);
    axisY->applyNiceNumbers();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab3::loadSSBRecruitsChart Complete");

} // end loadTopRightChart


void
nmfForecastTab3::loadYearResidualChart(Boost3DArrayDouble &SRData,
                                      int &SpeNum,
                                       std::string xTitle,
                                       std::string yTitle)
{
    QValueAxis     *axisX;
    QValueAxis     *axisY;
    QLineSeries    *lineSeries    = new QLineSeries();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab3::loadYearResidualChart");

    // Load series variables with appropriate data for chart display.
    for (int i=0; i<=NYears-1; ++i) {
        if (yTitle == "Residual")
            lineSeries->append(FirstYear+i,SRData[SpeNum][i][3]);
        else if (yTitle == "R / SSB")
            lineSeries->append(FirstYear+i,SRData[SpeNum][i][1]/SRData[SpeNum][i][0]);
    }
    lineSeries->setPointsVisible(true);

    // Create scatter chart with superimposed best fit curve.
    if (Forecast_Tab3_BottomRightLayout->count() == 0) {
        chartBottomRight = new QChart();
        chartBottomRight->legend()->hide();
        chartBottomRight->addSeries(lineSeries);
        chartBottomRight->createDefaultAxes();
        chartBottomRight->setTitle("Plot of Residuals by Year");
        QChartView *chartView = new QChartView(chartBottomRight);
        chartView->setRenderHint(QPainter::Antialiasing);
        Forecast_Tab3_BottomRightLayout->addWidget(chartView);
    } else {
        axisX = qobject_cast<QValueAxis *>(chartBottomRight->axes(Qt::Horizontal).at(0));
        axisY = qobject_cast<QValueAxis *>(chartBottomRight->axes(Qt::Vertical).at(0));
        chartBottomRight->removeAxis(axisX);
        chartBottomRight->removeAxis(axisY);
        chartBottomRight->removeAllSeries();
        chartBottomRight->addSeries(lineSeries);
    }

    // Create new X axis with "nice" numbers set on scale.
    axisX = new QValueAxis();
    axisX->setLabelFormat("%d");
    //axisX->setMin(0.0);
    axisX->setTitleText(QString::fromStdString(xTitle));
    //chartBottomRight->setAxisX(axisX,lineSeries);
    nmfUtilsQt::setAxisX(chartBottomRight,axisX,lineSeries);
    //axisX->applyNiceNumbers();
    //int numTicks = (NYears % 2) ? NYears/2 : NYears/2+1;
    axisX->setTickCount(NYears);
    axisX->setLabelsAngle(-90);
//    if (NYears > nmfConstants::NumCategoriesForVerticalNotation)
//        axisX->setLabelsAngle(-90);
//    else
//        axisX->setLabelsAngle(0);



    // Create new Y axis with "nice" numbers set on scale.
    axisY = new QValueAxis();
    axisY->setLabelFormat("%.1f");
    axisY->setMin(0.0);
    axisY->setTitleText(QString::fromStdString(yTitle));
    //chartBottomRight->setAxisY(axisY,lineSeries);
    nmfUtilsQt::setAxisY(chartBottomRight,axisY,lineSeries);
    axisY->applyNiceNumbers();

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab3::loadYearResidualChart Complete");

} // end loadYearResidualChart


void
nmfForecastTab3::loadRegressionWidget(LinRegOut &RegressOut, bool showK, int warn)
{
    QString v11,v12,v13,v14;
    QString v21,v22,v23,v24;
    QString v31,v32,v33,v34;
    QString v41,v42,v51,v61;
    QString k11;
    QString warning;

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab3::loadRegressionWidget");

    Forecast_Tab3_QuartileTbW->hide();
    Forecast_Tab3_ParameterTE->show();

    v11 = QString::number(RegressOut.Alpha,'f',4);
    v12 = (! showK) ? QString::number(RegressOut.AlphaSE,'f',4) : "";
    v13 = (! showK) ? QString::number(RegressOut.AlphaT,'f',2)  : "";
    v14 = (! showK) ? QString::number(nmfUtilsStatistics::ProbT(RegressOut.AlphaT,RegressOut.ErrorDF),'f',4) : "";
    v21 = QString::number(RegressOut.Beta,'f',4);
    v22 = (! showK) ? QString::number(RegressOut.BetaSE,'f',4) : "";
    v23 = (! showK) ? QString::number(RegressOut.BetaT,'f',2)  : "";
    v24 = (! showK) ? QString::number(nmfUtilsStatistics::ProbT(RegressOut.BetaT,RegressOut.ErrorDF),'f',4)  : "";
    v31 = QString::number(1,'f',0);
    v32 = QString::number(RegressOut.RegMS,'f',4);
    v33 = QString::number(RegressOut.F,'f',4);
    v34 = QString::number(nmfUtilsStatistics::ProbF(RegressOut.F,RegressOut.RegDf,RegressOut.ErrorDF),'f',4);
    v41 = QString::number(RegressOut.ErrorDF,'f',0);
    v42 = QString::number(RegressOut.ErrorMS,'f',4);
    v51 = QString::number(RegressOut.Rsquare,'f',3);
    v61 = QString::number(RegressOut.ResSE,'f',3);
    k11 = (showK) ? QString::number(RegressOut.K,'f',4) : "";
    nmfUtilsQt::equalizeQStringLengths(v11,v21);
    nmfUtilsQt::equalizeQStringLengths(v12,v22);
    nmfUtilsQt::equalizeQStringLengths(v13,v23);
    nmfUtilsQt::equalizeQStringLengths(v14,v24);
    nmfUtilsQt::equalizeQStringLengths(v31,v41);
    nmfUtilsQt::equalizeQStringLengths(v32,v42);
    nmfUtilsQt::equalizeQStringLengths(v51,v61);
    if (showK) {
        nmfUtilsQt::equalizeQStringLengths(v21,k11);
    }

    switch (warn) {
      case 0:
        warning = "";
        break;
      case 1:
        warning = "Warning: Max Iterations Reached - Solution may be unreliable.";
        break;
      case 2:
        warning = "Warning: Invalid Solution";
        break;
    };



    /*

RSK continue here......

     gvim frmStockRec2.frm
     gvim NL_SRR_Fit.bas
     gvim Complex2.bas


     */






    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    QString regressionMsg="";

    regressionMsg += "\n";
    regressionMsg += "Param.     Value      SE      T-Value     P\n";
    regressionMsg += "--------------------------------------------\n";
    regressionMsg += "Alpha     " + v11 + "    " + v12 + "    " + v13 + "   " + v14 + "\n";
    regressionMsg += "Beta      " + v21 + "    " + v22 + "    " + v23 + "   " + v24 + "\n";
    if (showK) {
        regressionMsg += "K         " + k11 + "\n";
    } else {
        regressionMsg += "\n";
    }
    regressionMsg += "\n";
    regressionMsg += "Term         DF       MS      F Ratio     P\n";
    regressionMsg += "--------------------------------------------\n";
    regressionMsg += "Regression   " + v31 + "      " + v32 + "    " + v33 + "  " + v34 + "\n";
    regressionMsg += "Error        " + v41 + "      " + v42 + "\n";
    regressionMsg += "\n";
    regressionMsg += "R-squared:     " + v51 + "\n";
    regressionMsg += "Residual SE:   " + v61 + "\n";
    regressionMsg += "\n";
    int beginHighlight = regressionMsg.length();
    regressionMsg += warning;
    int endHighlight = regressionMsg.length();
    Forecast_Tab3_ParameterTE->setText(regressionMsg);
    Forecast_Tab3_ParameterTE->setFont(font);

    // Highlight warning message.
    QTextCharFormat fmt;
    fmt.setBackground(Qt::yellow);
    QTextCursor cursor(Forecast_Tab3_ParameterTE->document());
    cursor.setPosition(beginHighlight, QTextCursor::MoveAnchor);
    cursor.setPosition(endHighlight, QTextCursor::KeepAnchor);
    cursor.setCharFormat(fmt);

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab3::loadRegressionWidget Complete");

} // end loadRegressionWidget


void
nmfForecastTab3::callback_Forecast_Tab3_RecalcPB()
{
    std::cout << "Recalculating Shepherd Flexible curve..." << std::endl;
    redrawTable();
}


void
nmfForecastTab3::callback_Forecast_Tab3_SpeciesCMB(int SpeNum)
{
    if (SRType.size() > 0) {
        Forecast_Tab3_RecruitCurveCMB->setCurrentIndex(SRType(SpeNum));
        redrawTable();
    }

} // end callback_Forecast_Tab3_SpeciesCMB


void
nmfForecastTab3::callback_Forecast_Tab3_RecruitCurveCMB(int curveType)
{
    // Always turn check box off when user selects a curve type
    Forecast_Tab3_RecruitCurveCB->setChecked(false);
    //Forecast_Tab3_RecalcPB->hide();

    if (curveType == RandomFromQuartiles) {
        Forecast_Tab3_ParameterGB->setTitle("Quartile Information");
        Forecast_Tab3_ParameterW->hide();
        Forecast_Tab3_ParameterTE->hide();
        Forecast_Tab3_KLBL->hide();
        Forecast_Tab3_KLE->hide();
        Forecast_Tab3_QuartileTbW->show();
        Forecast_Tab3_RecruitCurveCB->setEnabled(false);

    } else {
        Forecast_Tab3_ParameterGB->setTitle("Regression Parameters");
        Forecast_Tab3_ParameterW->hide();
        Forecast_Tab3_QuartileTbW->hide();
        Forecast_Tab3_KLBL->hide();
        Forecast_Tab3_KLE->hide();
        Forecast_Tab3_ParameterTE->show();
        Forecast_Tab3_RecruitCurveCB->setEnabled(true);
    }

    if (curveType == ShepherdFlexible) {
        //Forecast_Tab3_RecalcPB->show();
        if (Forecast_Tab3_RecruitCurveCB->isChecked()) {
            Forecast_Tab3_KLBL->show();
            Forecast_Tab3_KLE->show();
        }
    }

    redrawTable();
}

void
nmfForecastTab3::callback_Forecast_Tab3_RecruitCurveCB(int checked)
{
    if (checked == Qt::Checked) {
        //Forecast_Tab3_ParameterGB->setTitle("Enter Stock Recruit Parameters");
        Forecast_Tab3_ParameterGB->hide();
        Forecast_Tab3_ParameterTE->hide();
        Forecast_Tab3_QuartileTbW->hide();
        Forecast_Tab3_ParameterW->show();
        Forecast_Tab3_StockRecruitParametersGB->show();

    } else {
        //Forecast_Tab3_ParameterGB->setTitle("Regression Parameters");
        Forecast_Tab3_StockRecruitParametersGB->hide();
        Forecast_Tab3_ParameterW->hide();
        Forecast_Tab3_QuartileTbW->hide();
        Forecast_Tab3_ParameterTE->show();
        Forecast_Tab3_ParameterGB->show();
    }

    if (Forecast_Tab3_RecruitCurveCMB->currentIndex() == ShepherdFlexible) {
        Forecast_Tab3_ParameterW->show();
        Forecast_Tab3_KLBL->show();
        Forecast_Tab3_KLE->show();
    }
}


void
nmfForecastTab3::loadWidgets(nmfDatabase *theDatabasePtr, std::string theMSVPAName,
                             std::string theForecastName, int NumYears, int theFirstYear)
{
    int RecordCount;
    std::string queryStr,queryStr2;
    std::map<std::string, std::vector<std::string> > dataMap,dataMap2;
    std::vector<std::string> fields,fields2;
    std::string species;
    std::vector<ForeSRRInfo> SpeRegData;
    std::vector<bool> UserDefinedSR;

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab3::loadWidgets");

    //boost::numeric::ublas::vector<int> SRType;
    NYears       = NumYears;
    ForecastName = theForecastName;
    MSVPAName    = theMSVPAName;
    databasePtr  = theDatabasePtr;
    FirstYear    = theFirstYear;
std::cout << "Loading widgets for forecast: " << ForecastName << std::endl;
    if (databasePtr == NULL) {
        std::cout << "Error: Can't load widgets for Forecast Tab3. The databasePtr has not been set." << std::endl;
        return;
    }

    databasePtr->getSpeciesToIndexMap(SpeciesToIndexMap);

    // Load species combo box
    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM MSVPAspecies WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND (Type = 0 or Type = 1)";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    Forecast_Tab3_SpeciesCMB->blockSignals(true);
    Forecast_Tab3_SpeciesCMB->clear();
    Forecast_Tab3_SpeciesCMB->blockSignals(false);

    NSpecies = dataMap["SpeName"].size();

    for (int i=0; i<NSpecies; ++i) {
        species = dataMap["SpeName"][i];
        SpeList.push_back(species);
        Forecast_Tab3_SpeciesCMB->addItem(QString::fromStdString(species));

        fields2   = {"MaxAge"};
        queryStr2 = "SELECT MaxAge FROM Species WHERE SpeName = '" + species + "'";
        dataMap2  = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        SpeNAge.push_back(std::stoi(dataMap2["MaxAge"][0]));
    }

    loadSRTable();

    /* RSK not sure what this does?????
     *
    // Set regdata type to -9...makes them actually do something
    for (int i = 0; i < NSpecies; ++i) {
        SpeRegData(i).SRRType = -9;
    }
    */
    nmfUtils::initialize(SRType,NSpecies);

    // Will eventually read stuff from the database ... really just SRType
    fields = {"SpeName","SpeIndex","SRRType","SRRA","SRRB","SRRK","Userdefined"};
    queryStr = "SELECT SpeName,SpeIndex,SRRType,SRRA,SRRB,SRRK,Userdefined FROM ForeSRR WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND ForeName = '" + ForecastName + "'" +
               " ORDER By SpeIndex";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr,fields);
    RecordCount = dataMap["SpeName"].size();
    if (RecordCount > 0) {
        for (int i = 0; i < NSpecies; ++i) {
            ForeSRRInfo foreStruct;
            foreStruct.SpeName  = dataMap["SpeName"][i];
            foreStruct.SpeIndex = std::stoi(dataMap["SpeIndex"][i]);
            foreStruct.SRRType  = std::stoi(dataMap["SRRType"][i]);
            SRType(i)           = foreStruct.SRRType;
            foreStruct.SRRA     = std::stod(dataMap["SRRA"][i]);
            foreStruct.SRRB     = std::stod(dataMap["SRRB"][i]);
            foreStruct.SRRK     = std::stod(dataMap["SRRK"][i]);
            foreStruct.Flag     = 0;
            UserDefinedSR.push_back((dataMap["Userdefined"][i])=="1");

            SpeRegData.push_back(foreStruct);
        } // end for i
    } // end if

    initialized = true;

    // Necessary so first time through charts get drawn correctly.
    callback_Forecast_Tab3_SpeciesCMB(1);
    callback_Forecast_Tab3_SpeciesCMB(0);

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab3::loadWidgets Complete");

} // end loadWidgets


void
nmfForecastTab3::loadSRTable() // aka Get_SRData() from frmStockRec2.bas
{
    int m;
    int Nage;
    int NSpecies;
    std::string SpeName;
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab3::loadSRTable");

    NSpecies = Forecast_Tab3_SpeciesCMB->count();

    boost::numeric::ublas::matrix<double> Maturity;
    boost::numeric::ublas::matrix<double> Biomass;
    boost::numeric::ublas::vector<double> Recruits;
    boost::numeric::ublas::vector<double> SSBValue;
    //boost::numeric::ublas::vector<double> tmpData;
    //boost::numeric::ublas::vector<double> RecVal;
    std::vector<double> tmpData;
    std::vector<double> RecVal;

    // Get all necessary data
    //boost::numeric::ublas::vector<double> SRType;
    //boost::numeric::ublas::vector<double> SpeRegData;
    //nmfUtils::initialize(SRType,    NSpecies);
    nmfUtils::initialize(SSBMax,    NSpecies);
    nmfUtils::initialize(PdSSBQ,    NSpecies,  5);     // holds quartile points
    nmfUtils::initialize(PdMeanRec, NSpecies,  5);
    nmfUtils::initialize(PdMinRec,  NSpecies,  5);
    nmfUtils::initialize(PdMaxRec,  NSpecies,  5);
    nmfUtils::initialize(SpeRegData,NSpecies);

    double RecSum;
    int RecCount;

    SRData.resize(boost::extents[NSpecies][NYears+1][4]);// last holds SSB, R, Fitted R, Residual

    for (int Spe=0; Spe<NSpecies; ++Spe) {
        //SpeName = SpeList[Spe];
        SpeName = SpeList[Spe];
        Nage    = SpeNAge[Spe];

        nmfUtils::initialize(Maturity,Nage+1,NYears+1);
        nmfUtils::initialize(Biomass, Nage+1,NYears+1);
        nmfUtils::initialize(Recruits,NYears+1);
        nmfUtils::initialize(SSBValue,NYears+1);

        // Get maturity matrix
        fields = {"PMature"};
        queryStr = "SELECT PMature FROM SpeMaturity WHERE SpeName = '" + SpeName + "'" +
                   " ORDER By Age, Year";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        m = 0;
        for (int i = 0; i<=Nage; ++i) {
            for (int j = 0; j<=NYears; ++j) {
                Maturity(i,j) = std::stod(dataMap["PMature"][m++]);
            }
        }

        // Get annual biomass by age class
        fields = {"Year","Age","Biomass"};
        queryStr = "Select Year, Age, Sum(AnnBiomass) As Biomass FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                   " AND SpeName = '" + SpeName + "'" +
                   " AND Season = 0 GROUP BY Age, Year";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (dataMap["Year"].size() == 0) {
            QMessageBox::information(Forecast_Tabs,
                                     tr("No MSVPA Data"),
                                     tr("\nNo data found in MSVPASeasBiomass table.  Please run an MSVPA config prior to running a Forecast."),
                                     QMessageBox::Ok);
            return;
        }

        m = 0;
        for (int i = 0; i<Nage; ++i) {
            for (int j = 0; j<NYears; ++j) {
                Biomass(i,j) = std::stod(dataMap["Biomass"][m++]);
            }
        }

        // Multiply by maturity and sum across age--this is SSB
        for (int i = 0; i <NYears; ++i) {
            SSBValue(i) = 0;
            for (int j = 0; j <Nage; ++j) {
                SSBValue(i) += Biomass(j,i) * Maturity(j,i);
            }
            SSBValue(i) = 0.5 * SSBValue(i) / 1000.0; // 50% sex ratio...but will need to include a place to enter it somewhere
        }

        // Then get numbers of age 0...
        fields = {"Year","Abundance"};
        queryStr = "Select Year, Sum(AnnAbund) As Abundance FROM MSVPASeasBiomass WHERE MSVPAname = '" + MSVPAName + "'" +
                   " AND SpeName = '" + SpeName + "'" +
                   " AND Age = 0 AND Season = 0 GROUP BY Year";
        dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
        if (int(dataMap["Abundance"].size()) != NYears) {
            logger->logMsg(nmfConstants::Error,"nmfForecastTab3::loadSRTable Missing Abundance data from MSVPASeasBiomass");
            return;
        }
        for (int i = 0; i<NYears; ++i) {
            Recruits(i) = std::stod(dataMap["Abundance"][i]);
        }
        // Then put it in SRData

        for (int i = 0; i<NYears; ++i) {
         SRData[Spe][i][0] = SSBValue(i);
         SRData[Spe][i][1] = Recruits(i+1)/1000.0; // saved in thousands of fish..is this going to work or do I need conversions?
//std::cout <<  "SpeNum: " << Spe << ", loading data: " << SRData[Spe][i][0]
//          << ", " << SRData[Spe][i][1] << std::endl;

        }

        // Might as well go ahead and get and store SSBmax and quartile information for all species...
        // Need to set maximum value of graph for each species based on max SSB values
        // Sort the SSB values and put them in a temporary matrix
        //nmfUtils::initialize(tmpData,NYears+1);
        tmpData.clear();
        for (int i = 0; i < NYears; ++i) {
            //tmpData(i) = SRData[Spe][i][0];
            tmpData.push_back(SRData[Spe][i][0]);
        }
//std::cout << "\nSpeNum: " << Spe << std::endl;
//std::cout << "pre sort: tmpData max: " << tmpData[NYears-1] << std::endl;

        // Sort the vector
        sort(tmpData.begin(),tmpData.end());
        //Call QSORTIT(NYears, tmpData())
//std::cout << "pos sort: tmpData max: " << tmpData[NYears-1] << std::endl;

        // Define SSBmax as 50% higher than actual SSBmax, to provide greater range on the curve
        // (changed to 25% higher).
        // The constant here was 1.5 in original code.  I made it 1.25 as Qt scales up the plot
        // when applyNiceNumbers is called on the axis.
        SSBMax(Spe) = tmpData[NYears-1] * 1.25;
//std::cout << "SSBMax: " << SSBMax(Spe) << std::endl;

        // Get the Quartile information
        PdSSBQ(Spe,0) = tmpData[0];                                     // minimum
        PdSSBQ(Spe,1) = tmpData[nmfUtils::round((NYears-1) * 0.25, 0)]; // Q1
        PdSSBQ(Spe,2) = tmpData[nmfUtils::round((NYears-1) * 0.5,  0)]; // Median
        PdSSBQ(Spe,3) = tmpData[nmfUtils::round((NYears-1) * 0.75, 0)]; // Q3
        PdSSBQ(Spe,4) = tmpData[NYears-1];                              // Max

        // Calculate the mean N, low, and high recruits for each quartile
        for (int i = 1; i <= 4; ++i) {

            //nmfUtils::initialize(RecVal,NYears+1);
            RecVal.clear();

            RecSum = 0.0;
            RecCount = 0;
            for (int j = 0; j < NYears; ++j) {
                if ((SRData[Spe][j][0] >= PdSSBQ(Spe,i-1)) && (SRData[Spe][j][0] < PdSSBQ(Spe,i))) {
                    //RecVal(RecCount) = SRData[Spe][j][1];
                    RecVal.push_back(SRData[Spe][j][1]);
                    RecSum += SRData[Spe][j][1];
                }
            }
            RecCount = RecVal.size();
            PdMeanRec(Spe,i) = RecSum / (double)RecCount;
            // Sort the RecVal array to get the min and max
            sort(RecVal.begin(),RecVal.end());
            //Call QSORTIT(RecCount, RecVal);
            PdMinRec(Spe,i) = RecVal[0];
            PdMaxRec(Spe,i) = RecVal[RecCount - 1];
        } // end for i

    } // end for Spe






//    // Debug code

//    for (int i=0; i<NSpecies; ++i) {
//        std::cout << "Load SRData for species: " << SpeList[i] << std::endl;
//        for (int j=0; j<NYears-1; ++j) {
//            std::cout << SRData[i][j][0] << std::endl;
//        }
//    }

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab3::loadSRTable Complete");

} // end loadSRTable


void
nmfForecastTab3::callback_Forecast_Tab3_PrevPB(bool unused)
{
    Forecast_Tabs->setCurrentIndex(Forecast_Tabs->currentIndex()-1);
}

void
nmfForecastTab3::callback_Forecast_Tab3_NextPB(bool unused)
{
    Forecast_Tabs->setCurrentIndex(Forecast_Tabs->currentIndex()+1);
}


void
nmfForecastTab3::callback_Forecast_Tab3_FitSRCurveSavePB(bool unused)
{
    std::string queryStr;
    std::map<std::string, std::vector<std::string> > dataMap;
    std::vector<std::string> fields;
    std::string cmd;
    double SRRA = 0.0;
    double SRRB = 0.0;
    double SRRK = 0.0;
    int SpeIndex = 0;
    int SRRType = Forecast_Tab3_RecruitCurveCMB->currentIndex();
    int Userdefined = Forecast_Tab3_RecruitCurveCB->isChecked();
    std::string errorMsg;
    std::string SpeciesName = Forecast_Tab3_SpeciesCMB->currentText().toStdString();

    // This function will return the species index value set for the named species in the
    // Species table.  However, in the original code, Lance used the order of the species in
    // the pulldown on this form as the species index...which isn't the same as what's in the
    // Species table.  However, looks like he never used that species number anywhere else for
    // anything, so I believe it's a benign bug.
    //SpeIndex = SpeciesToIndexMap[SpeciesName];         // It should probably be this.
    SpeIndex = Forecast_Tab3_SpeciesCMB->currentIndex(); // ...but Lance did this.

    // User values in ForeSRR as default if they exist
    fields   = {"MSVPAName","ForeName","SpeName","SpeIndex","SRRType","SRRA","SRRB","SRRK","Userdefined"};
    queryStr = "SELECT MSVPAName,ForeName,SpeName,SpeIndex,SRRType,SRRA,SRRB,SRRK,Userdefined FROM ForeSRR WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND ForeName = '" + ForecastName + "'" +
               " ORDER By SpeIndex";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    for (unsigned int i=0; i<dataMap["MSVPAName"].size(); ++i) {
        if ((MSVPAName   == dataMap["MSVPAName"][i]) && (ForecastName == dataMap["ForeName"][i]) &&
            (SpeciesName == dataMap["SpeName"][i])   && (SpeIndex     == std::stoi(dataMap["SpeIndex"][i])))
        {
            SRRA = std::stod(dataMap["SRRA"][i]);
            SRRB = std::stod(dataMap["SRRA"][i]);
            SRRK = std::stod(dataMap["SRRA"][i]);
            break;
        }
    }

    if (Userdefined) {
        SRRA = Forecast_Tab3_AlphaLE->text().toDouble();
        SRRB = Forecast_Tab3_BetaLE->text().toDouble();
        if (SRRType == 3) {
            SRRK = Forecast_Tab3_KLE->text().toDouble();
        }
    }


    // Update ForeSRR database table
    cmd  = "INSERT INTO ForeSRR ";
    cmd += "(MSVPAName,ForeName,SpeName,SpeIndex,SRRType,SRRA,SRRB,SRRK,Userdefined) values ";
    cmd += "(\"" + MSVPAName + "\", " +
            "\"" + ForecastName + "\", " +
            "\"" + SpeciesName + "\", " +
            std::to_string(SpeIndex) + "," +
            std::to_string(SRRType) + "," +
            std::to_string(SRRA) + "," +
            std::to_string(SRRB) + "," +
            std::to_string(SRRK) + "," +
            std::to_string(Userdefined) + ") ";
    cmd += "ON DUPLICATE KEY UPDATE ";
    cmd += "SRRType=values(SRRType),SRRA=values(SRRA),SRRB=values(SRRB),";
    cmd += "SRRK=values(SRRK),Userdefined=values(Userdefined); ";
std::cout << cmd << std::endl;
//    errorMsg = databasePtr->nmfUpdateDatabase(cmd);
//    if (nmfUtilsQt::isAnError(errorMsg)) {
//        std::cout << cmd << std::endl;
//        nmfUtils::printError("callback_Forecast_Tab3_FitSRCurveSavePB: INSERT INTO ForeSRR...", errorMsg);
//    }


    // Redraw
    redrawTable();

} // end callback_Forecast_Tab3_FitSRCurveSavePB


void
nmfForecastTab3::restoreCSVFromDatabase(nmfDatabase *databasePtr)
{
    QString TableName;
    std::vector<std::string> fields;

    TableName = "ForeSRR";
    fields    = {"MSVPAName","ForeName","SpeName","SpeIndex",
                 "SRRType","SRRA","SRRB","SRRK","Userdefined"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

    TableName = "ForeSRQ";  // RSK - Find out where this is used?
    fields    = {"MSVPAName","ForeName","SpeName","SpeIndex","Quartile",
                 "MinSSB","MaxSSB","MinRec","MaxRec","MeanRec"};
    databasePtr->RestoreCSVFile(TableName,ProjectDir,fields);

} // end restoreCSVFromDatabase



void
nmfForecastTab3::clearWidgets()
{
//    Forecast_Tab3_MsvpaNameLE->clear();
//    Forecast_Tab3_ForecastNameLE->clear();
//    Forecast_Tab3_ScenarioNameLE->clear();
//    Forecast_Tab3_InitialYearCMB->clear();
//    Forecast_Tab3_NumYearsLE->clear();
//    Forecast_Tab3_PredatorGrowthCB->setChecked(false);
}
