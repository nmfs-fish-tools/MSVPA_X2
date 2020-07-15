#ifndef NMFSETUPTAB3_H
#define NMFSETUPTAB3_H

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <set>

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QProgressDialog>
#include <QSettings>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTextEdit>
#include <QUiLoader>
#include <QWidget>

#include "nmfDatabase.h"
#include "nmfLogger.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

Q_DECLARE_METATYPE(QStringList)

const int Column_SpeIndex         = 0;
const int Column_SpeName          = 1;
const int Column_SpeFirstYear     = 2;
const int Column_SpeLastYear      = 3;
const int Column_SpeMaxAge        = 4;
const int Column_SpeMinCatchAge   = 5;
const int Column_SpeMaxCatchAge   = 6;
const int Column_SpePlusClass     = 7;
const int Column_SpeCatchUnits    = 8;
const int Column_SpeWtAtAgeData   = 9;
const int Column_SpeWtUnits       = 10;
const int Column_SpeSzAtAgeData   = 11;
const int Column_SpeSzUnits       = 12;
const int Column_SpeFixedMaturity = 13;

const int Column_OthIndex         = 0;
const int Column_OthName          = 1;
const int Column_OthFirstYear     = 2;
const int Column_OthLastYear      = 3;
const int Column_OthMinSize       = 4;
const int Column_OthMaxSize       = 5;
const int Column_OthSizeUnits     = 6;
const int Column_OthBMUnits       = 7;
const int Column_OthWtUnits       = 8;
const int Column_OthSizeStruc     = 9;
const int Column_OthNumSizeCats   = 10;

class nmfSetup_Tab3: public QObject
{
    Q_OBJECT

    nmfLogger    *logger;

    QTabWidget   *Setup_Tabs;

    nmfDatabase  *databasePtr;
    QTextEdit    *SetupOutputTE;
    QWidget      *Setup_Tab3_Widget;
    QPushButton  *Setup_Tab3_CreateTemplatesPB;
    QSpinBox     *Setup_Tab3_NumSpeciesSB;
    QSpinBox     *Setup_Tab3_NumOtherPredatorsSB;
    QTableWidget *Setup_Tab3_SpeciesTW;
    QTableWidget *Setup_Tab3_OtherPredatorsTW;
    QLineEdit    *Setup_Tab2_ProjectDirLE;
    QLineEdit    *Setup_Tab2_FirstYearLE;
    QLineEdit    *Setup_Tab2_LastYearLE;
    QLineEdit    *Setup_Tab2_NumSeasonsLE;
    QCheckBox    *Setup_Tab3_SpeciesCB;
    QCheckBox    *Setup_Tab3_OtherPredatorsCB;
    QLabel       *Setup_Tab3_NumSpeciesLBL;
    QLabel       *Setup_Tab3_NumOtherPredatorsLBL;
    QPushButton  *Setup_Tab3_AddSpeciesPB;
    QPushButton  *Setup_Tab3_DelSpeciesPB;
    QPushButton  *Setup_Tab3_AddOtherPredatorsPB;
    QPushButton  *Setup_Tab3_DelOtherPredatorsPB;
    QPushButton  *Setup_Tab3_SavePB;
    QPushButton  *Setup_Tab3_LoadPB;
    QPushButton  *Setup_Tab3_ReloadSpeciesPB;
    QPushButton  *Setup_Tab3_ReloadOtherPredatorsPB;
    //QPushButton  *Setup_Tab3_ContinuePB;

    std::string  ProjectDir;
    QStandardItemModel* smodelSpecies;
    QStandardItemModel* smodelOtherPredSpecies;

    std::map<std::string,std::vector<std::string> > TableNames[3];

    std::map<std::string, void(*)(
            //std::map<std::string,std::vector<std::string> > *TableNames,
            std::string &csvPathFileName,
            std::string &tableName,
            QMap<std::string,int> &FirstYear,
            QMap<std::string,int> &LastYear,
            QMap<std::string,int> &MinCatchAge,
            QMap<std::string,int> &MaxCatchAge,
            QStringList &Species,
            QStringList &OtherPredSpecies,
            QMap<std::string,int> &OtherPredCategories
            )> FunctionMap;

public:

    nmfSetup_Tab3(QTabWidget  *tabs,
                 nmfLogger   *logger,
                 QTextEdit   *theSetupOutputTE,
                 std::string &theProjectDir);
    virtual ~nmfSetup_Tab3();
    void clearWidgets();
    void loadWidgets(nmfDatabase *theDatabasePtr);
    void updateOutputWidget();
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void renameSpecies(std::string newSpeName, std::string existingSpeName);

    void readSettings();
    void saveSettings();

    void buildCSVFile(std::string &csvPathFileName,
                      std::string &csvFileName,
                      QMap<std::string,int> &FirstYear,
                      QMap<std::string,int> &LastYear,
                      int NumSeasons,
                      QMap<std::string,int> &MinCatchAge,
                      QMap<std::string,int> &MaxCatchAge,
                      QStringList Species,
                      QStringList OtherPredators,
                      QMap<std::string,int> &OtherPredCategories);

    static void buildCSVFileForeEnergyDens(//std::map<std::string,std::vector<std::string> > *TableNames,
                                           std::string &csvPathFileName, std::string &tableName,
                                           QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                           QMap<std::string,int> &MinCatchAge,
                                           QMap<std::string,int> &MaxCatchAge,
                                           QStringList &Species,
                                           QStringList &OtherPredSpecies,
                                           QMap<std::string,int> &OtherPredCategories);       // Table  1 of 39
    static void buildCSVFileForeOutput(//std::map<std::string,std::vector<std::string> > *TableNames,
                                       std::string &csvPathFileName,std::string &tableName,
                                       QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                       QMap<std::string,int> &MinCatchAge,
                                       QMap<std::string,int> &MaxCatchAge,
                                       QStringList &Species,
                                       QStringList &OtherPredSpecies,
                                       QMap<std::string,int> &OtherPredCategories);           // Table  2 of 39
    static void buildCSVFileForePredGrowth(//std::map<std::string,std::vector<std::string> > *TableNames,
                                           std::string &csvPathFileName,std::string &tableName,
                                           QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                           QMap<std::string,int> &MinCatchAge,
                                           QMap<std::string,int> &MaxCatchAge,
                                           QStringList &Species,
                                           QStringList &OtherPredSpecies,
                                           QMap<std::string,int> &OtherPredCategories);       // Table  3 of 39
    static void buildCSVFileForePredVonB(//std::map<std::string,std::vector<std::string> > *TableNames,
                                         std::string &csvPathFileName,std::string &tableName,
                                         QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                         QMap<std::string,int> &MinCatchAge,
                                         QMap<std::string,int> &MaxCatchAge,
                                         QStringList &Species,
                                         QStringList &OtherPredSpecies,
                                         QMap<std::string,int> &OtherPredCategories);         // Table  4 of 39
    static void buildCSVFileForeSRQ(//std::map<std::string,std::vector<std::string> > *TableNames,
                                    std::string &csvPathFileName,std::string &tableName,
                                    QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                    QMap<std::string,int> &MinCatchAge,
                                    QMap<std::string,int> &MaxCatchAge,
                                    QStringList &Species,
                                    QStringList &OtherPredSpecies,
                                    QMap<std::string,int> &OtherPredCategories);              // Table  5 of 39
    static void buildCSVFileForeSRR(//std::map<std::string,std::vector<std::string> > *TableNames,
                                    std::string &csvPathFileName,std::string &tableName,
                                    QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                    QMap<std::string,int> &MinCatchAge,
                                    QMap<std::string,int> &MaxCatchAge,
                                    QStringList &Species,
                                    QStringList &OtherPredSpecies,
                                    QMap<std::string,int> &OtherPredCategories);              // Table  6 of 39
    static void buildCSVFileForeSuitPreyBiomass(//std::map<std::string,std::vector<std::string> > *TableNames,
                                                std::string &csvPathFileName,std::string &tableName,
                                                QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                                QMap<std::string,int> &MinCatchAge,
                                                QMap<std::string,int> &MaxCatchAge,
                                                QStringList &Species,
                                                QStringList &OtherPredSpecies,
                                                QMap<std::string,int> &OtherPredCategories);  // Table  7 of 39
    static void buildCSVFileForecasts(//std::map<std::string,std::vector<std::string> > *TableNames,
                                      std::string &csvPathFileName,std::string &tableName,
                                      QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                      QMap<std::string,int> &MinCatchAge,
                                      QMap<std::string,int> &MaxCatchAge,
                                      QStringList &Species,
                                      QStringList &OtherPredSpecies,
                                      QMap<std::string,int> &OtherPredCategories);            // Table  8 of 39
    static void buildCSVFileMSVPAEnergyDens(//std::map<std::string,std::vector<std::string> > *TableNames,
                                            std::string &csvPathFileName,std::string &tableName,
                                            QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                            QMap<std::string,int> &MinCatchAge,
                                            QMap<std::string,int> &MaxCatchAge,
                                            QStringList &Species,
                                            QStringList &OtherPredSpecies,
                                            QMap<std::string,int> &OtherPredCategories);      // Table  9 of 39
    static void buildCSVFileMSVPAOthPrey(//std::map<std::string,std::vector<std::string> > *TableNames,
                                         std::string &csvPathFileName,std::string &tableName,
                                         QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                         QMap<std::string,int> &MinCatchAge,
                                         QMap<std::string,int> &MaxCatchAge,
                                         QStringList &Species,
                                         QStringList &OtherPredSpecies,
                                         QMap<std::string,int> &OtherPredCategories);         // Table 10 of 39
    static void buildCSVFileMSVPAOthPreyAnn(//std::map<std::string,std::vector<std::string> > *TableNames,
                                            std::string &csvPathFileName,std::string &tableName,
                                            QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                            QMap<std::string,int> &MinCatchAge,
                                            QMap<std::string,int> &MaxCatchAge,
                                            QStringList &Species,
                                            QStringList &OtherPredSpecies,
                                            QMap<std::string,int> &OtherPredCategories);      // Table 11 of 39
    static void buildCSVFileMSVPASeasBiomass(//std::map<std::string,std::vector<std::string> > *TableNames,
                                             std::string &csvPathFileName,std::string &tableName,
                                             QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                             QMap<std::string,int> &MinCatchAge,
                                             QMap<std::string,int> &MaxCatchAge,
                                             QStringList &Species,
                                             QStringList &OtherPredSpecies,
                                             QMap<std::string,int> &OtherPredCategories);     // Table 12 of 39
    static void buildCSVFileMSVPASeasInfo(//std::map<std::string,std::vector<std::string> > *TableNames,
                                          std::string &csvPathFileName,std::string &tableName,
                                          QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                          QMap<std::string,int> &MinCatchAge,
                                          QMap<std::string,int> &MaxCatchAge,
                                          QStringList &Species,
                                          QStringList &OtherPredSpecies,
                                          QMap<std::string,int> &OtherPredCategories);        // Table 13 of 39
    static void buildCSVFileMSVPASizePref(//std::map<std::string,std::vector<std::string> > *TableNames,
                                          std::string &csvPathFileName,std::string &tableName,
                                          QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                          QMap<std::string,int> &MinCatchAge,
                                          QMap<std::string,int> &MaxCatchAge,
                                          QStringList &Species,
                                          QStringList &OtherPredSpecies,
                                          QMap<std::string,int> &OtherPredCategories);        // Table 14 of 39
    static void buildCSVFileMSVPASpaceO(//std::map<std::string,std::vector<std::string> > *TableNames,
                                        std::string &csvPathFileName,std::string &tableName,
                                        QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                        QMap<std::string,int> &MinCatchAge,
                                        QMap<std::string,int> &MaxCatchAge,
                                        QStringList &Species,
                                        QStringList &OtherPredSpecies,
                                        QMap<std::string,int> &OtherPredCategories);          // Table 15 of 39
    static void buildCSVFileMSVPAStomCont(//std::map<std::string,std::vector<std::string> > *TableNames,
                                          std::string &csvPathFileName,std::string &tableName,
                                          QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                          QMap<std::string,int> &MinCatchAge,
                                          QMap<std::string,int> &MaxCatchAge,
                                          QStringList &Species,
                                          QStringList &OtherPredSpecies,
                                          QMap<std::string,int> &OtherPredCategories);        // Table 16 of 39
    static void buildCSVFileMSVPASuitPreyBiomass(//std::map<std::string,std::vector<std::string> > *TableNames,
                                                 std::string &csvPathFileName,std::string &tableName,
                                                 QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                                 QMap<std::string,int> &MinCatchAge,
                                                 QMap<std::string,int> &MaxCatchAge,
                                                 QStringList &Species,
                                                 QStringList &OtherPredSpecies,
                                                 QMap<std::string,int> &OtherPredCategories); // Table 17 of 39
    static void buildCSVFileMSVPAlist(//std::map<std::string,std::vector<std::string> > *TableNames,
                                      std::string &csvPathFileName,std::string &tableName,
                                      QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                      QMap<std::string,int> &MinCatchAge,
                                      QMap<std::string,int> &MaxCatchAge,
                                      QStringList &Species,
                                      QStringList &OtherPredSpecies,
                                      QMap<std::string,int> &OtherPredCategories);            // Table 18 of 39
    static void buildCSVFileMSVPAprefs(//std::map<std::string,std::vector<std::string> > *TableNames,
                                       std::string &csvPathFileName,std::string &tableName,
                                       QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                       QMap<std::string,int> &MinCatchAge,
                                       QMap<std::string,int> &MaxCatchAge,
                                       QStringList &Species,
                                       QStringList &OtherPredSpecies,
                                       QMap<std::string,int> &OtherPredCategories);           // Table 19 of 39
    static void buildCSVFileMSVPAspecies(//std::map<std::string,std::vector<std::string> > *TableNames,
                                         std::string &csvPathFileName,std::string &tableName,
                                         QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                         QMap<std::string,int> &MinCatchAge,
                                         QMap<std::string,int> &MaxCatchAge,
                                         QStringList &Species,
                                         QStringList &OtherPredSpecies,
                                         QMap<std::string,int> &OtherPredCategories);         // Table 20 of 39
    static void buildCSVFileOthPredSizeData(//std::map<std::string,std::vector<std::string> > *TableNames,
                                            std::string &csvPathFileName,std::string &tableName,
                                            QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                            QMap<std::string,int> &MinCatchAge,
                                            QMap<std::string,int> &MaxCatchAge,
                                            QStringList &Species,
                                            QStringList &OtherPredSpecies,
                                            QMap<std::string,int> &OtherPredCategories);      // Table 21 of 39
    static void buildCSVFileOtherPredBM(//std::map<std::string,std::vector<std::string> > *TableNames,
                                        std::string &csvPathFileName,std::string &tableName,
                                        QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                        QMap<std::string,int> &MinCatchAge,
                                        QMap<std::string,int> &MaxCatchAge,
                                        QStringList &Species,
                                        QStringList &OtherPredSpecies,
                                        QMap<std::string,int> &OtherPredCategories);          // Table 22 of 39
    static void buildCSVFileOtherPredSpecies(//std::map<std::string,std::vector<std::string> > *TableNames,
                                             std::string &csvPathFileName,std::string &tableName,
                                             QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                             QMap<std::string,int> &MinCatchAge,
                                             QMap<std::string,int> &MaxCatchAge,
                                             QStringList &Species,
                                             QStringList &OtherPredSpecies,
                                             QMap<std::string,int> &OtherPredCategories);     // Table 23 of 39
    static void buildCSVFileSSVPAAgeM(//std::map<std::string,std::vector<std::string> > *TableNames,
                                      std::string &csvPathFileName,std::string &tableName,
                                      QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                      QMap<std::string,int> &MinCatchAge,
                                      QMap<std::string,int> &MaxCatchAge,
                                      QStringList &Species,
                                      QStringList &OtherPredSpecies,
                                      QMap<std::string,int> &OtherPredCategories);            // Table 24 of 39
    static void buildCSVFileScenarioF(//std::map<std::string,std::vector<std::string> > *TableNames,
                                      std::string &csvPathFileName,std::string &tableName,
                                      QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                      QMap<std::string,int> &MinCatchAge,
                                      QMap<std::string,int> &MaxCatchAge,
                                      QStringList &Species,
                                      QStringList &OtherPredSpecies,
                                      QMap<std::string,int> &OtherPredCategories);            // Table 25 of 39
    static void buildCSVFileScenarioOthPred(//std::map<std::string,std::vector<std::string> > *TableNames,
                                            std::string &csvPathFileName,std::string &tableName,
                                            QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                            QMap<std::string,int> &MinCatchAge,
                                            QMap<std::string,int> &MaxCatchAge,
                                            QStringList &Species,
                                            QStringList &OtherPredSpecies,
                                            QMap<std::string,int> &OtherPredCategories);      // Table 26 of 39
    static void buildCSVFileScenarioOthPrey(//std::map<std::string,std::vector<std::string> > *TableNames,
                                            std::string &csvPathFileName,std::string &tableName,
                                            QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                            QMap<std::string,int> &MinCatchAge,
                                            QMap<std::string,int> &MaxCatchAge,
                                            QStringList &Species,
                                            QStringList &OtherPredSpecies,
                                            QMap<std::string,int> &OtherPredCategories);      // Table 27 of 39
    static void buildCSVFileScenarioRec(//std::map<std::string,std::vector<std::string> > *TableNames,
                                        std::string &csvPathFileName,std::string &tableName,
                                        QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                        QMap<std::string,int> &MinCatchAge,
                                        QMap<std::string,int> &MaxCatchAge,
                                        QStringList &Species,
                                        QStringList &OtherPredSpecies,
                                        QMap<std::string,int> &OtherPredCategories);          // Table 28 of 39
    static void buildCSVFileScenarios(//std::map<std::string,std::vector<std::string> > *TableNames,
                                      std::string &csvPathFileName,std::string &tableName,
                                      QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                      QMap<std::string,int> &MinCatchAge,
                                      QMap<std::string,int> &MaxCatchAge,
                                      QStringList &Species,
                                      QStringList &OtherPredSpecies,
                                      QMap<std::string,int> &OtherPredCategories);            // Table 29 of 39
    static void buildCSVFileSpeCatch(//std::map<std::string,std::vector<std::string> > *TableNames,
                                     std::string &csvPathFileName,std::string &tableName,
                                     QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                     QMap<std::string,int> &MinCatchAge,
                                     QMap<std::string,int> &MaxCatchAge,
                                     QStringList &Species,
                                     QStringList &OtherPredSpecies,
                                     QMap<std::string,int> &OtherPredCategories);             // Table 30 of 39
    static void buildCSVFileSpeMaturity(//std::map<std::string,std::vector<std::string> > *TableNames,
                                        std::string &csvPathFileName,std::string &tableName,
                                        QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                        QMap<std::string,int> &MinCatchAge,
                                        QMap<std::string,int> &MaxCatchAge,
                                        QStringList &Species,
                                        QStringList &OtherPredSpecies,
                                        QMap<std::string,int> &OtherPredCategories);          // Table 31 of 39
    static void buildCSVFileSpeSSVPA(//std::map<std::string,std::vector<std::string> > *TableNames,
                                     std::string &csvPathFileName,std::string &tableName,
                                     QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                     QMap<std::string,int> &MinCatchAge,
                                     QMap<std::string,int> &MaxCatchAge,
                                     QStringList &Species,
                                     QStringList &OtherPredSpecies,
                                     QMap<std::string,int> &OtherPredCategories);             // Table 32 of 39
    static void buildCSVFileSpeSize(//std::map<std::string,std::vector<std::string> > *TableNames,
                                    std::string &csvPathFileName,std::string &tableName,
                                    QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                    QMap<std::string,int> &MinCatchAge,
                                    QMap<std::string,int> &MaxCatchAge,
                                    QStringList &Species,
                                    QStringList &OtherPredSpecies,
                                    QMap<std::string,int> &OtherPredCategories);              // Table 33 of 39
    static void buildCSVFileSpeTuneCatch(//std::map<std::string,std::vector<std::string> > *TableNames,
                                         std::string &csvPathFileName,std::string &tableName,
                                         QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                         QMap<std::string,int> &MinCatchAge,
                                         QMap<std::string,int> &MaxCatchAge,
                                         QStringList &Species,
                                         QStringList &OtherPredSpecies,
                                         QMap<std::string,int> &OtherPredCategories);         // Table 34 of 39
    static void buildCSVFileSpeTuneEffort(//std::map<std::string,std::vector<std::string> > *TableNames,
                                          std::string &csvPathFileName,std::string &tableName,
                                          QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                          QMap<std::string,int> &MinCatchAge,
                                          QMap<std::string,int> &MaxCatchAge,
                                          QStringList &Species,
                                          QStringList &OtherPredSpecies,
                                          QMap<std::string,int> &OtherPredCategories);        // Table 35 of 39
    static void buildCSVFileSpeWeight(//std::map<std::string,std::vector<std::string> > *TableNames,
                                      std::string &csvPathFileName,std::string &tableName,
                                      QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                      QMap<std::string,int> &MinCatchAge,
                                      QMap<std::string,int> &MaxCatchAge,
                                      QStringList &Species,
                                      QStringList &OtherPredSpecies,
                                      QMap<std::string,int> &OtherPredCategories);            // Table 36 of 39
    static void buildCSVFileSpeXSAData(//std::map<std::string,std::vector<std::string> > *TableNames,
                                       std::string &csvPathFileName,std::string &tableName,
                                       QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                       QMap<std::string,int> &MinCatchAge,
                                       QMap<std::string,int> &MaxCatchAge,
                                       QStringList &Species,
                                       QStringList &OtherPredSpecies,
                                       QMap<std::string,int> &OtherPredCategories);           // Table 37 of 39
    static void buildCSVFileSpeXSAIndices(//std::map<std::string,std::vector<std::string> > *TableNames,
                                          std::string &csvPathFileName,std::string &tableName,
                                          QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                          QMap<std::string,int> &MinCatchAge,
                                          QMap<std::string,int> &MaxCatchAge,
                                          QStringList &Species,
                                          QStringList &OtherPredSpecies,
                                          QMap<std::string,int> &OtherPredCategories);        // Table 38 of 39
    static void buildCSVFileSpecies(//std::map<std::string,std::vector<std::string> > *TableNames,
                                    std::string &csvPathFileName,std::string &tableName,
                                    QMap<std::string,int> &FirstYear, QMap<std::string,int> &LastYear,
                                    QMap<std::string,int> &MinCatchAge,
                                    QMap<std::string,int> &MaxCatchAge,
                                    QStringList &Species,
                                    QStringList &OtherPredSpecies,
                                    QMap<std::string,int> &OtherPredCategories);              // Table 39 of 39

    void PopulateARowSpecies(int row, int ncols);
    void PopulateARowOtherPredators(int row, int ncols);
    void LoadSpeciesTable();
    void LoadOtherPredatorsTable();
    bool SpeciesFieldsOK(int NumSpecies);
    bool OtherPredatorsFieldsOK(int OtherPredators);
    void UpdateSpeciesTable(int NumSpecies);
    void UpdateOtherPredatorsTable(int NumOtherPredators);
    void createTheTemplates(const std::vector<std::string> &tableVec, bool verbose);

signals:
    void UpdateNavigator(int item);
    void EnableNavigator();
    void DeactivateRunButtons();
    void CreateAllDatabaseTables();
    void SSVPALoadWidgets(int TabNum);
    void UpdateNavigator(std::string type, int index);

public Q_SLOTS:
    void callback_Setup_Tab3_CreateTemplates(bool unused);
    void callback_Setup_Tab3_NumSpecies(int value);
    void callback_Setup_Tab3_SpeciesChanged(QTableWidgetItem *item);
    void callback_LoadDatabase(QString database);
    void callback_Setup_Tab3_SpeciesCB(bool state);
    void callback_Setup_Tab3_OtherPredatorsCB(bool state);
    void callback_Setup_Tab3_NumOtherPredators(int numOtherPredSpecies);
    void callback_Setup_Tab3_OtherPredatorsChanged(QTableWidgetItem *item);
    void callback_Setup_Tab3_AddSpeciesPB(bool unused);
    void callback_Setup_Tab3_DelSpeciesPB(bool unused);
    void callback_Setup_Tab3_ReloadSpeciesPB(bool unused);
    void callback_Setup_Tab3_AddOtherPredatorsPB(bool unused);
    void callback_Setup_Tab3_DelOtherPredatorsPB(bool unused);
    void callback_Setup_Tab3_ReloadOtherPredatorsPB(bool unused);
    void callback_Setup_Tab3_SavePB(bool unused);
    void callback_Setup_Tab3_LoadPB(bool unused);

};

#endif // NMFSETUPTAB3_H
