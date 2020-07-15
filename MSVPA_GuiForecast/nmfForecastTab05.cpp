#include "nmfConstants.h"

#include "nmfForecastTab05.h"


nmfForecastTab5::nmfForecastTab5(QTabWidget *tabs,
                                 nmfLogger *theLogger)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfForecastTab5::nmfForecastTab5");

    Forecast_Tabs = tabs;

    // Load ui as a widget from disk
    QFile file(":/forms/Forecast/Forecast_Tab05.ui");
    file.open(QFile::ReadOnly);
    Forecast_Tab5_Widget = loader.load(&file,Forecast_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    Forecast_Tabs->addTab(Forecast_Tab5_Widget, tr("5. Execute Forecast Model"));

    Forecast_Tab5_PrevPB                 = Forecast_Tabs->findChild<QPushButton *>("Forecast_Tab5_PrevPB");
    Forecast_Tab5_RunPB                  = Forecast_Tabs->findChild<QPushButton *>("Forecast_Tab5_RunPB");
    Forecast_Tab5_CurrentConfigurationTE = Forecast_Tabs->findChild<QTextEdit *>("Forecast_Tab5_CurrentConfigurationTE");
    Forecast_Tab5_CurrentConfigurationTE = Forecast_Tabs->findChild<QTextEdit *>("Forecast_Tab5_CurrentConfigurationTE");
    Forecast_Tab5_ProgressBarPBR         = Forecast_Tabs->findChild<QProgressBar *>("Forecast_Tab5_ProgressBarPBR");
    Forecast_Tab5_ProgressBarLBL         = Forecast_Tabs->findChild<QLabel *>("Forecast_Tab5_ProgressBarLBL");

    Forecast_Tab5_ProgressBarPBR->hide(); // Delete widget eventually, if not going to use it.
    Forecast_Tab5_ProgressBarLBL->hide(); // Delete widget eventually, if not going to use it.

    Forecast_Tab5_PrevPB->setText("\u25C1--");

    connect(Forecast_Tab5_PrevPB,  SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab5_PrevPB(bool)));
    connect(Forecast_Tab5_RunPB,   SIGNAL(clicked(bool)),
            this,                  SLOT(callback_Forecast_Tab5_RunPB(bool)));

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab5::nmfForecastTab5 Complete");

}


nmfForecastTab5::~nmfForecastTab5()
{

}


void
nmfForecastTab5::loadWidgets(std::string MSVPAName,
                             std::string ForecastName,
                             std::string ScenarioName,
                             int InitYear,
                             int NYears,
                             bool isGrowthModel,
                             std::string &configInfo)
{
    std::string msg = "";

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab5::loadWidgets");

    // Write output message so user can see the current Forecast's initial conditions.
    Forecast_Tab5_CurrentConfigurationTE->clear();

    msg += "<br><b>Reference MSVPA Run:</b>&nbsp;&nbsp;" + MSVPAName;
    msg += "<br><b>Forecast Name:</b>&nbsp;&nbsp;" + ForecastName;
    msg += "<br><b>Scenario Name:</b>&nbsp;&nbsp;" + ScenarioName;

    msg += "<br><br><b>Total number of years forecasted:</b>&nbsp;&nbsp;" + std::to_string(NYears);
    msg += "<br><b>Initial Year:</b>&nbsp;&nbsp;" + std::to_string(InitYear);
    if (isGrowthModel)
        msg += "<br><b>Predator Growth:</b>&nbsp;&nbsp;Modeled";
    else
        msg += "<br><b>Predator Growth:</b>&nbsp;&nbsp;Fixed";

    msg += "<br><br><b>Scenario Configuration:</b><br>";
    msg += configInfo;

    Forecast_Tab5_CurrentConfigurationTE->setText(msg.c_str());

    logger->logMsg(nmfConstants::Normal,"nmfForecastTab5::loadWidgets Complete");
}

void
nmfForecastTab5::callback_Forecast_Tab5_PrevPB(bool unused)
{
    Forecast_Tabs->setCurrentIndex(Forecast_Tabs->currentIndex()-1);
}


void
nmfForecastTab5::callback_Forecast_Tab5_RunPB(bool unused)
{
    emit RunForecast();
}


void
nmfForecastTab5::makeCurrent()
{
    Forecast_Tabs->setCurrentIndex(Forecast_Tabs->count()-1);
}


//void
//nmfForecastTab5::loadWidgets(QString ForecastName, int NYears, int FirstYear, int LastYear)
//{
//    Forecast_Tab5_ScenarioNameLE->clear();
//    Forecast_Tab5_ForecastNameLE->setText(ForecastName);
//    Forecast_Tab5_NumYearsLE->setText(QString::number(NYears));
//    Forecast_Tab5_InitialYearCMB->clear();
//    for (int year=FirstYear; year<=LastYear; ++year) {
//        Forecast_Tab5_InitialYearCMB->addItem(QString::number(year));
//    }
//    Forecast_Tab5_InitialYearCMB->setCurrentIndex(Forecast_Tab5_InitialYearCMB->count()-1);
//}

void
nmfForecastTab5::clearWidgets()
{
//    Forecast_Tab5_MsvpaNameLE->clear();
//    Forecast_Tab5_ForecastNameLE->clear();
//    Forecast_Tab5_ScenarioNameLE->clear();
//    Forecast_Tab5_InitialYearCMB->clear();
//    Forecast_Tab5_NumYearsLE->clear();
//    Forecast_Tab5_PredatorGrowthCB->setChecked(false);
}
