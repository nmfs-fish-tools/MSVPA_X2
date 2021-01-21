#ifndef NMFMSVPATab9_H
#define NMFMSVPATab9_H

#include <QMainWindow>
#include <QTableWidget>
#include <QObject>
#include <QListWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
#include <QTableView>
#include <QStandardItemModel>
#include <QUiLoader>

#include "nmfDatabase.h"
#include "nmfLogger.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/multi_array.hpp>

typedef boost::multi_array<double, 3> Boost3DArrayDouble;
typedef boost::multi_array<double, 4> Boost4DArrayDouble;
typedef Boost3DArrayDouble::index index3D;
typedef Boost4DArrayDouble::index index4D;

class nmfMSVPATab9: public QObject
{
    Q_OBJECT

    nmfLogger   *logger;

    QTabWidget  *MSVPA_Tabs;
    QWidget     *MSVPA_Tab9_Widget;
    QPushButton *MSVPA_Tab9_PrevPB;
    QPushButton *MSVPA_Tab9_NextPB;
    QComboBox   *MSVPA_Tab9_SpeciesCMB;
    QTableView  *MSVPA_Tab9_EnergyDensityTV;
    Boost3DArrayDouble EnergyDensity;
    QStandardItemModel *smodel;
    bool Enabled;

    int NSeasons;
    int NPreds;
    int NSpecies;
    int NPrey;
    int NOthPrey;
    std::vector<std::string> PredList;
    std::vector<int> NPredAge;
    std::vector<int> PredType;
    std::string ProjectDir;

signals:
    void MSVPALoadWidgets(int TabNum);
    void UpdateNavigator(std::string type, int index);

public:

    nmfMSVPATab9(QTabWidget *tabs, nmfLogger *logger);
    virtual ~nmfMSVPATab9();
    void loadWidgets(nmfDatabase *databasePtr, std::string MSVPAName);
    void clearWidgets();
    void updateProjectDir(std::string newProjectDir);

public Q_SLOTS:
    void callback_MSVPA_Tab9_PrevPage();
    void callback_MSVPA_Tab9_NextPage();
    void callback_MSVPA_Tab9_ItemChanged(QStandardItem *item);
    void callback_MSVPA_Tab9_SpeciesCMB(int index);

};

#endif // NMFMSVPATab9_H
