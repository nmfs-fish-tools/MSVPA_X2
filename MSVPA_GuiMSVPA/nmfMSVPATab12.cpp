#include "nmfConstants.h"

#include "nmfMSVPATab12.h"



nmfMSVPATab12::nmfMSVPATab12(QTabWidget *tabs, nmfLogger *theLogger)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab12::nmfMSVPATab12");

    MSVPA_Tabs = tabs;
    Enabled = false;

    // Load ui as a widget from disk
    QFile file(":/forms/MSVPA/MSVPA_Tab12.ui");
    file.open(QFile::ReadOnly);
    MSVPA_Tab12_Widget = loader.load(&file,MSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    MSVPA_Tabs->addTab(MSVPA_Tab12_Widget, tr("12. Execute"));

    // Setup connections
    MSVPA_Tab12_PrevPB  = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab12_PrevPB");
    MSVPA_Tab12_RunPB   = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab12_RunPB");


    MSVPA_Tab12_PrevPB->setText("\u25C1--");

    connect(MSVPA_Tab12_PrevPB,  SIGNAL(clicked(bool)),
            this,                SLOT(callback_MSVPA_Tab12_PrevPB(bool)));

    connect(MSVPA_Tab12_RunPB,   SIGNAL(clicked(bool)),
            this,                SLOT(callback_MSVPA_Tab12_RunPB(bool)));

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab12::nmfMSVPATab12 Complete");
}


nmfMSVPATab12::~nmfMSVPATab12()
{

}


void nmfMSVPATab12::outputCurrentConfiguration(
        std::string MSVPAName, int FirstYear, int LastYear,
        int NumSeasons, int isGrowthModel,
        std::vector<std::string> predators,
        std::vector<std::string> prey,
        std::vector<std::string> otherPredators)
{
    std::string msg;
    QTextEdit *MSVPA_Tab12_CurrentConfigTE = MSVPA_Tabs->findChild<QTextEdit *>("MSVPA_Tab12_CurrentConfigTE");
    std::string runningGrowthModel = (isGrowthModel) ? "Yes" : "No";

    msg  = "<br><b>MSVPA Configuration:</b>&nbsp;&nbsp;" + MSVPAName;
    msg += "<br><br><b>Running Growth Model:</b>&nbsp;&nbsp;" + runningGrowthModel;
    msg += "<br><br><b>Year Range:</b>&nbsp;&nbsp;" + std::to_string(FirstYear) + " - " + std::to_string(LastYear);
    msg += "<br><b>Number of Years:</b>&nbsp;&nbsp;" + std::to_string(LastYear-FirstYear+1);
    msg += "<br><b>Number of Seasons:</b>&nbsp;&nbsp;" + std::to_string(NumSeasons);
    msg += "<br><br><b>MSVPA Predators:</b>";
    for (unsigned int i=0;i<predators.size();++i)
        msg += "<br>"+predators[i];
    msg += "<br><br><b>MSVPA Prey:</b>";
    for (unsigned int i=0;i<prey.size();++i)
        msg += "<br>"+prey[i];
    msg += "<br><br><b>Other Predators:</b>";
    for (unsigned int i=0;i<otherPredators.size();++i)
        msg += "<br>"+otherPredators[i];

    MSVPA_Tab12_CurrentConfigTE->setHtml(msg.c_str());
}



void nmfMSVPATab12::clearWidgets()
{

}

bool
nmfMSVPATab12::loadWidgets(nmfDatabase *theDatabasePtr,
                           std::string  theMSVPAName,
                           std::string  theProjectDir)
{
    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab12::loadWidgets (Nothing to load)");

    // Just a placeholder
    return true;
}

void
nmfMSVPATab12::callback_MSVPA_Tab12_PrevPB(bool unused)
{
    int prevPage = MSVPA_Tabs->currentIndex()-1;

    MSVPA_Tabs->blockSignals(true);
    MSVPA_Tabs->setCurrentIndex(prevPage);
    MSVPA_Tabs->blockSignals(false);

    emit MSVPALoadWidgets(prevPage);
    emit UpdateNavigator("MSVPA",prevPage);
}

void
nmfMSVPATab12::callback_MSVPA_Tab12_RunPB(bool unused)
{

    emit RunMSVPA();

} // end callback_SSVPA_Tab4_RunPB

