#ifndef NMFFORECASTTAB3_H
#define NMFFORECASTTAB3_H

#include <algorithm>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QObject>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QUiLoader>
#include <QValueAxis>
#include <QVBoxLayout>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>

#include "nmfDatabase.h"
#include "nmfUtilsQt.h"
#include "nmfUtils.h"
#include "nmfUtilsStatistics.h"
#include "nmfLogger.h"

QT_CHARTS_USE_NAMESPACE

struct ForeSRRInfo
{
    std::string SpeName;
    int SpeIndex;
    int SRRType;
    double SRRA;
    double SRRB;
    double SRRK;
    int Flag;
};

class nmfForecastTab3: public QObject
{
    Q_OBJECT

    // Constants for curve types
    const int Ricker              = 0;
    const int BevertonHolt        = 1;
    const int RandomFromQuartiles = 2;
    const int ShepherdFlexible    = 3;

    int NYears;
    int NSpecies;
    int FirstYear;
    bool initialized;
    bool readOnlyTables;
    nmfDatabase *databasePtr;
    std::string MSVPAName;
    std::string ForecastName;
    std::string ProjectDir;
    std::vector<std::string> SpeList;
    std::vector<int> SpeNAge;
    std::map<std::string,int> SpeciesToIndexMap;

    QChart *chartTopRight;
    QChart *chartBottomRight;
    nmfLogger *logger;

    QTabWidget   *Forecast_Tabs;
    QWidget      *Forecast_Tab3_Widget;
    QPushButton  *Forecast_Tab3_LoadPB;
    QPushButton  *Forecast_Tab3_ClearPB;
    QPushButton  *Forecast_Tab3_SavePB;
    QPushButton  *Forecast_Tab3_FitSRCurveSavePB;
    QPushButton  *Forecast_Tab3_PrevPB;
    QPushButton  *Forecast_Tab3_NextPB;
    QCheckBox    *Forecast_Tab3_RecruitCurveCB;
    QComboBox    *Forecast_Tab3_RecruitCurveCMB;
    QGroupBox    *Forecast_Tab3_ParameterGB;
    QWidget      *Forecast_Tab3_ParameterW;
    QTextEdit    *Forecast_Tab3_ParameterTE;
    QTableWidget *Forecast_Tab3_QuartileTbW;
    QLabel       *Forecast_Tab3_KLBL;
    QComboBox    *Forecast_Tab3_SpeciesCMB;
    QTableWidget *Forecast_Tab3_DataTbW;
    QLineEdit    *Forecast_Tab3_AlphaLE;
    QLineEdit    *Forecast_Tab3_BetaLE;
    QLineEdit    *Forecast_Tab3_KLE;
    QVBoxLayout  *Forecast_Tab3_TopRightLayout;
    QVBoxLayout  *Forecast_Tab3_BottomRightLayout;
    QGroupBox    *Forecast_Tab3_StockRecruitParametersGB;
    QPushButton  *Forecast_Tab3_RecalcPB;

    Boost3DArrayDouble SRData; // last holds SSB, R, Fitted R, Residual
    boost::numeric::ublas::matrix<double> PdSSBQ;
    boost::numeric::ublas::matrix<double> PdMeanRec;
    boost::numeric::ublas::matrix<double> PdMinRec;
    boost::numeric::ublas::matrix<double> PdMaxRec;
    boost::numeric::ublas::vector<double> SSBMax;
    boost::numeric::ublas::vector<double> SpeRegData;
    boost::numeric::ublas::vector<int>    SRType;


public:

    nmfForecastTab3(QTabWidget *tabs,
                    nmfLogger *logger,
                    std::string &theProjectDir);
    virtual ~nmfForecastTab3();
    void clearWidgets();
    void loadWidgets(nmfDatabase *databasePtr, std::string MSVPAName,
                     std::string ForecastName, int NYears, int FirstYear);
    void loadSRTable();
    void redrawTable();
    void loadRegressionWidget(LinRegOut &RegressOut, bool showK, int warn);
    void loadSSBRecruitsChart(Boost3DArrayDouble &SRData,
                              boost::numeric::ublas::matrix<double> &SRFit,
                              int &SpeNum);
    void loadYearResidualChart(Boost3DArrayDouble &SRData,
                               int &SpeNum,
                               std::string xTitle,
                               std::string yTitle);
    void restoreCSVFromDatabase(nmfDatabase *databasePtr);
    void updateProjectDir(std::string newProjectDir);

//signals:
//    void TableDataChanged(std::string tableName);

public Q_SLOTS:
    void callback_Forecast_Tab3_PrevPB(bool unused);
    void callback_Forecast_Tab3_NextPB(bool unused);
    void callback_Forecast_Tab3_RecruitCurveCB(int checked);
    void callback_Forecast_Tab3_RecruitCurveCMB(int curveType);
    void callback_Forecast_Tab3_SpeciesCMB(int speciesNum);
    void callback_Forecast_Tab3_RecalcPB();
    void callback_Forecast_Tab3_FitSRCurveSavePB(bool unused);
//    void callback_TableDataChanged(QModelIndex unused);

};

#endif // NMFFORECASTTAB3_H
