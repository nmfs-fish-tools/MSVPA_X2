#ifndef NMFMSVPATab10_H
#define NMFMSVPATab10_H

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

class nmfMSVPATab10: public QObject
{
    Q_OBJECT

    nmfLogger   *logger;

    QTabWidget  *MSVPA_Tabs;
    QWidget     *MSVPA_Tab10_Widget;
    QPushButton *MSVPA_Tab10_PrevPB;
    QPushButton *MSVPA_Tab10_NextPB;
    QComboBox   *MSVPA_Tab10_SpeciesCMB;
    QTableView  *MSVPA_Tab10_DietEnergyDensityTV;
    Boost3DArrayDouble DietEnergyDensity;
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

    nmfMSVPATab10(QTabWidget *tabs, nmfLogger *logger);
    virtual ~nmfMSVPATab10();
    void loadWidgets(nmfDatabase *databasePtr, std::string MSVPAName);
    void clearWidgets();
    void updateProjectDir(std::string newProjectDir);

public Q_SLOTS:
    void callback_MSVPA_Tab10_PrevPage();
    void callback_MSVPA_Tab10_NextPage();
    void callback_MSVPA_Tab10_ItemChanged(QStandardItem *item);
    void callback_MSVPA_Tab10_SpeciesCMB(int index);

};

#endif // NMFMSVPATab10_H
