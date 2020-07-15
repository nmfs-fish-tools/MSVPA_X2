#ifndef NMFMSVPATab12_H
#define NMFMSVPATab12_H

#include <QMainWindow>
#include <QTableWidget>
#include <QObject>
#include <QListWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QTextEdit>
#include <QUiLoader>

#include "nmfDatabase.h"
#include "nmfLogger.h"

class nmfMSVPATab12: public QObject
{
    Q_OBJECT

    nmfLogger   *logger;

    QTabWidget  *MSVPA_Tabs;
    QWidget     *MSVPA_Tab12_Widget;
    QPushButton *MSVPA_Tab12_PrevPB;
    QPushButton *MSVPA_Tab12_RunPB;
    bool Enabled;


public:

    nmfMSVPATab12(QTabWidget *tabs, nmfLogger *logger);
    virtual ~nmfMSVPATab12();
    bool loadWidgets(nmfDatabase *databasePtr,
                     std::string MSVPAName,
                     std::string ProjectDir);
    void clearWidgets();
    void outputCurrentConfiguration(
            std::string MSVPAName,
            int FirstYear,
            int LastYear,
            int NumSeasons,
            int isGrowthModel,
            std::vector<std::string> predators,
            std::vector<std::string> prey,
            std::vector<std::string> otherPredators);

signals:
    void MSVPALoadWidgets(int TabNum);
    void UpdateNavigator(std::string type, int index);
    void RunMSVPA();

public Q_SLOTS:
    void callback_MSVPA_Tab12_PrevPB(bool unused);
    void callback_MSVPA_Tab12_RunPB(bool unused);

};

#endif // NMFMSVPATab12_H
