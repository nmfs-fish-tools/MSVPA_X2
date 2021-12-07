#include "nmfConstants.h"
#include "nmfUtils.h"

#include "nmfMSVPATab10.h"


nmfMSVPATab10::nmfMSVPATab10(QTabWidget *tabs, nmfLogger *theLogger)
{
    QUiLoader loader;

    logger = theLogger;
    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab10::nmfMSVPATab10");

    MSVPA_Tabs = tabs;
    Enabled = false;
    smodel = NULL;

    // Load ui as a widget from disk
    QFile file(":/forms/MSVPA/MSVPA_Tab10.ui");
    file.open(QFile::ReadOnly);
    MSVPA_Tab10_Widget = loader.load(&file,MSVPA_Tabs);
    file.close();

    // Add the loaded widget as the new tabbed page
    MSVPA_Tabs->addTab(MSVPA_Tab10_Widget, tr("10. [ Diet Energy Data ]"));

    // Setup connections
    MSVPA_Tab10_PrevPB              = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab10_PrevPB");
    MSVPA_Tab10_NextPB              = MSVPA_Tabs->findChild<QPushButton *>("MSVPA_Tab10_NextPB");
    MSVPA_Tab10_SpeciesCMB          = MSVPA_Tabs->findChild<QComboBox *>("MSVPA_Tab10_SpeciesCMB");
    MSVPA_Tab10_DietEnergyDensityTV = MSVPA_Tabs->findChild<QTableView *>("MSVPA_Tab10_DietEnergyDensityTV");

    MSVPA_Tab10_PrevPB->setText("\u25C1--");
    MSVPA_Tab10_NextPB->setText("--\u25B7");

    connect(MSVPA_Tab10_PrevPB,  SIGNAL(pressed()),
            this,               SLOT(callback_MSVPA_Tab10_PrevPage()));
    connect(MSVPA_Tab10_NextPB,  SIGNAL(pressed()),
            this,               SLOT(callback_MSVPA_Tab10_NextPage()));

    logger->logMsg(nmfConstants::Normal,"nmfMSVPATab10::nmfMSVPATab10 Complete");

}


nmfMSVPATab10::~nmfMSVPATab10()
{
    std::cout << "Freeing Tab9 memory" << std::endl;
    clearWidgets();
}

void
nmfMSVPATab10::updateProjectDir(std::string newProjectDir)
{

    ProjectDir = newProjectDir;

} // end updateProjectDir


void nmfMSVPATab10::clearWidgets()
{
    if (smodel)
        smodel->clear();
}


void
nmfMSVPATab10::callback_MSVPA_Tab10_ItemChanged(QStandardItem *item)
{
   int species = MSVPA_Tab10_SpeciesCMB->currentIndex();
   DietEnergyDensity[species][item->row()][item->column()] = item->index().data().toDouble();

} // end callback_MSVPA_Tab10_ItemChanged



void nmfMSVPATab10::loadWidgets(nmfDatabase *databasePtr, std::string MSVPAName)
{
    std::map<std::string, std::vector<std::string> > dataMap,dataMap2;
    std::vector<std::string> fields,fields2;
    std::string queryStr,queryStr2;
    int m;

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab10::loadWidgets");

    PredList.clear();
    NPredAge.clear();
    PredType.clear();

    smodel = new QStandardItemModel(nmfConstants::MaxNumberAges, nmfConstants::MaxNumberSeasons );

    connect(smodel, SIGNAL(itemChanged(QStandardItem *)),
            this,   SLOT(callback_MSVPA_Tab10_ItemChanged(QStandardItem *)));

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


    DietEnergyDensity.resize(boost::extents[NSpecies][nmfConstants::MaxNumberAges][nmfConstants::MaxNumberSeasons]);
    nmfUtils::initialize(DietEnergyDensity);

    // initialize data - will need to have these routines to load data if it exists
    fields   = {"AvgDietE"};
    queryStr = "SELECT AvgDietE FROM " + nmfConstantsMSVPA::TableMSVPAEnergyDens +
               " WHERE MSVPAname = '" + MSVPAName + "'" +
               " ORDER BY SpeIndex, Age, Season";
    dataMap  = databasePtr->nmfQueryDatabase(queryStr, fields);
    std::string valStr;
    if (dataMap["AvgDietE"].size() > 0) {
        m = 0;
        for (int i = 0; i < NSpecies; ++i) {
            for (int j = 0; j <= NPredAge[i]; ++j) {
                for (int k = 0; k < NSeasons; ++k) {
                    valStr = dataMap["AvgDietE"][m];
                    if (! valStr.empty()) {
                        DietEnergyDensity[i][j][k] = std::stod(valStr);
                    }
                    ++m;
                } // end for k
            } // end for j
        } // end for i
    } // end if


    // Update species combobox
    MSVPA_Tab10_SpeciesCMB->clear();
    for (unsigned int i=0;i<PredList.size(); ++i) {
        MSVPA_Tab10_SpeciesCMB->addItem(QString::fromStdString(PredList[i]));
    }
    MSVPA_Tab10_SpeciesCMB->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    connect(MSVPA_Tab10_SpeciesCMB, SIGNAL(currentIndexChanged(int)),
            this,                  SLOT(callback_MSVPA_Tab10_SpeciesCMB(int)));

    // Update widgets
    callback_MSVPA_Tab10_SpeciesCMB(0);

    //logger->logMsg(nmfConstants::Normal,"nmfMSVPATab10::loadWidgets Complete");

} // end loadWidgets


void
nmfMSVPATab10::callback_MSVPA_Tab10_SpeciesCMB(int index)
{
    QStringList strList;

    if (smodel != NULL) {
        smodel->clear();
    }

    disconnect(smodel, SIGNAL(itemChanged(QStandardItem *)),
               this,   SLOT(callback_MSVPA_Tab10_ItemChanged(QStandardItem *)));

    // Load the model
    for (int j=0; j<=NPredAge[index]; ++j) {
        for (int num=0;num<NSeasons;++num){
            QStandardItem *item = new QStandardItem(QString::number(DietEnergyDensity[index][j][num]));
            item->setTextAlignment(Qt::AlignCenter);
            smodel->setItem(j, num, item);
        }
    }
    MSVPA_Tab10_DietEnergyDensityTV->setModel(smodel);

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
            this,   SLOT(callback_MSVPA_Tab10_ItemChanged(QStandardItem *)));


} // end callback_MSVPA_Tab10_SpeciesCMB


void
nmfMSVPATab10::callback_MSVPA_Tab10_PrevPage()
{
    MSVPA_Tabs->setCurrentIndex(MSVPA_Tabs->currentIndex()-1);
}

void
nmfMSVPATab10::callback_MSVPA_Tab10_NextPage()
{
    MSVPA_Tabs->setCurrentIndex(MSVPA_Tabs->currentIndex()+1);
}

