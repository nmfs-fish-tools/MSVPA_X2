
#include "nmfConstants.h"
#include "nmfUtils.h"

#include "nmfMSVPATab09.h"



nmfMSVPATab9::nmfMSVPATab9(QTabWidget *tabs,
                           nmfLogger *theLogger)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab9::nmfMSVPATab9");

    MSVPA_Tabs = tabs;
    Enabled = false;
    smodel = NULL;

    // Load ui as a widget from disk
    QFile file(":/forms/MSVPA/MSVPA_Tab09.ui");
    file.open(QFile::ReadOnly);
    MSVPA_Tab9_Widget = loader.load(&file,MSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    MSVPA_Tabs->addTab(MSVPA_Tab9_Widget, tr("9. [ Energy Density ]"));

    // Setup connections
    MSVPA_Tab9_PrevPB          = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab9_PrevPB");
    MSVPA_Tab9_NextPB          = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab9_NextPB");
    MSVPA_Tab9_SpeciesCMB      = MSVPA_Tabs->findChild<QComboBox *>("MSVPA_Tab9_SpeciesCMB");
    MSVPA_Tab9_EnergyDensityTV = MSVPA_Tabs->findChild<QTableView *>("MSVPA_Tab9_EnergyDensityTV");

    MSVPA_Tab9_PrevPB->setText("\u25C1--");
    MSVPA_Tab9_NextPB->setText("--\u25B7");

    connect(MSVPA_Tab9_PrevPB,  SIGNAL(pressed()),
            this,               SLOT(callback_MSVPA_Tab9_PrevPage()));
    connect(MSVPA_Tab9_NextPB,  SIGNAL(pressed()),
            this,               SLOT(callback_MSVPA_Tab9_NextPage()));

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab9::nmfMSVPATab9 Complete");

}


nmfMSVPATab9::~nmfMSVPATab9()
{
    std::cout << "Freeing Tab8 memory" << std::endl;
    clearWidgets();
}

void
nmfMSVPATab9::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir


void nmfMSVPATab9::clearWidgets()
{
    if (smodel)
        smodel->clear();
}

void
nmfMSVPATab9::callback_MSVPA_Tab9_ItemChanged(QStandardItem *item)
{
   int species = MSVPA_Tab9_SpeciesCMB->currentIndex();
   EnergyDensity[species][item->row()][item->column()] = item->index().data().toDouble();

} // end callback_MSVPA_Tab9_ItemChanged


void nmfMSVPATab9::loadWidgets(nmfDatabase *databasePtr, std::string MSVPAName)
{
    std::map<std::string, std::vector<std::string> > dataMap,dataMap2;
    std::vector<std::string> fields,fields2;
    std::string queryStr,queryStr2;
    int m;

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab9::loadWidgets");

    PredList.clear();
    NPredAge.clear();
    PredType.clear();

    smodel = new QStandardItemModel(nmfConstants::MaxNumberAges, nmfConstants::MaxNumberSeasons );

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab9_ItemChanged(QStandardItem *)));

    // Select the predator list..associated with the MSVPA
    // Load MSVPA predator name and age lists for use later
    fields   = {"NSeasons"};
    queryStr = "SELECT NSeasons FROM " + nmfConstantsMSVPA::TableMSVPAlist +
               " WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NSeasons = std::stoi(dataMap["NSeasons"][0]);

    fields = {"SpeName"};
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Type = 0";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);

    NPreds = dataMap["SpeName"].size();
    NSpecies = NPreds;
    for (int i = 0; i < NPreds; ++i) {
        PredList.push_back(dataMap["SpeName"][i]);
        fields2   = {"MaxAge"};
        queryStr2 = "SELECT MaxAge FROM " + nmfConstantsMSVPA::TableSpecies +
                    " WHERE SpeName = '" + PredList[i] + "'";
        dataMap2  = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        NPredAge.push_back(std::stoi(dataMap2["MaxAge"][0]));

        PredType.push_back(0);
    } // end for i

    // Load MSVPA Prey Names and Ages
    fields   = {"SpeName"};
    queryStr = "SELECT SpeName FROM " + nmfConstantsMSVPA::TableMSVPAspecies +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " AND Type = 1";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NPrey    = dataMap["SpeName"].size();

    for (int i = 0; i < NPrey; ++i) {
        PredList.push_back(dataMap["SpeName"][i]);
        PredType.push_back(2);
        fields2   = {"MaxAge"};
        queryStr2 = "SELECT MaxAge FROM " + nmfConstantsMSVPA::TableSpecies +
                    " WHERE SpeName = '" + PredList[i+NSpecies] + "'";
        dataMap2  = databasePtr->nmfQueryDatabase(queryStr2, fields2);
        NPredAge.push_back(std::stoi(dataMap2["MaxAge"][0]));
    } // end for i
    NSpecies += NPrey;

    // Load Other Prey
    fields   = {"OthPreyName"};
    queryStr = "SELECT OthPreyName FROM " + nmfConstantsMSVPA::TableMSVPAOthPrey +
               " WHERE MSVPAname = '" + MSVPAName + "'";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    NOthPrey = dataMap["OthPreyName"].size();

    for (int i = 0; i < NOthPrey; ++i) {
      PredList.push_back(dataMap["OthPreyName"][i]);
      PredType.push_back(3);
      NPredAge.push_back(0);
    } // end for i
    NSpecies += NOthPrey;

    EnergyDensity.resize(boost::extents[NSpecies][nmfConstants::MaxNumberAges][nmfConstants::MaxNumberSeasons]);
    nmfUtils::initialize(EnergyDensity);

    // initialize data - will need to have these routines to load data if it exists
    fields   = {"EnergyDens"};
    queryStr = "SELECT EnergyDens FROM " + nmfConstantsMSVPA::TableMSVPAEnergyDens +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " ORDER BY SpeIndex, Age, Season";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    if (dataMap["EnergyDens"].size() > 0) {
        m = 0;
        for (int i = 0; i < NSpecies; ++i) {
            for (int j = 0; j <= NPredAge[i]; ++j) {
                for (int k = 0; k < NSeasons; ++k) {
                    EnergyDensity[i][j][k] = std::stod(dataMap["EnergyDens"][m++]);
                } // end for k
            } // end for j
        } // end for i
    } // end if


    // Update species combobox
    MSVPA_Tab9_SpeciesCMB->clear();
    for (unsigned int i=0;i<PredList.size(); ++i) {
        MSVPA_Tab9_SpeciesCMB->addItem(QString::fromStdString(PredList[i]));
    }
    MSVPA_Tab9_SpeciesCMB->setSizeAdjustPolicy(QComboBox::AdjustToContents);



    connect(MSVPA_Tab9_SpeciesCMB, SIGNAL(currentIndexChanged(int)),
            this,                  SLOT(callback_MSVPA_Tab9_SpeciesCMB(int)));

    // Update widgets
    callback_MSVPA_Tab9_SpeciesCMB(0);

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab9::loadWidgets Complete");

} // end loadWidgets


void
nmfMSVPATab9::callback_MSVPA_Tab9_SpeciesCMB(int index)
{
    QStringList strList;

    if (smodel != NULL) {
        smodel->clear();
    }

    disconnect(smodel, SIGNAL(itemChanged(QStandardItem *)),
               this,   SLOT(callback_MSVPA_Tab9_ItemChanged(QStandardItem *)));


    // Load the model
    for (int j=0; j<=NPredAge[index]; ++j) {
        for (int num=0;num<NSeasons;++num){
            QStandardItem *item = new QStandardItem(QString::number(EnergyDensity[index][j][num]));
            item->setTextAlignment(Qt::AlignCenter);
            smodel->setItem(j, num, item);
        }
    }
    MSVPA_Tab9_EnergyDensityTV->setModel(smodel);

    // Make row headers
    strList.clear();
    std::string prefix = (PredType[index] == 1) ? "Size ": "Age ";
    if (PredType[index] == 3) {
        strList << QString::fromStdString(PredList[index]);
    } else {
        for (int k=0; k<=NPredAge[index]; ++k) {
            strList << QString::fromStdString(prefix) + QString::number(k);
        }
    }
    smodel->setVerticalHeaderLabels(strList);

    // Make column headers
    strList.clear();
    for (int k=0; k<NSeasons; ++k) {
        strList << "Season " + QString::number(k+1);
    }
    smodel->setHorizontalHeaderLabels(strList);

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab9_ItemChanged(QStandardItem *)));

} // end callback_MSVPA_Tab9_SpeciesCMB


void
nmfMSVPATab9::callback_MSVPA_Tab9_PrevPage()
{
    MSVPA_Tabs->setCurrentIndex(MSVPA_Tabs->currentIndex()-1);
}

void
nmfMSVPATab9::callback_MSVPA_Tab9_NextPage()
{
    MSVPA_Tabs->setCurrentIndex(MSVPA_Tabs->currentIndex()+1);
}

