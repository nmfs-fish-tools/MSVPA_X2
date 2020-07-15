#ifndef MSVPAGUICONTROLSYIELDPERRECRUIT_H
#define MSVPAGUICONTROLSYIELDPERRECRUIT_H

#include "MSVPAGuiControls.h"

class MSVPAGuiControlsYieldPerRecruit : public MSVPAGuiControls
{

    Q_OBJECT

public:
    QLabel*      SelectYPRAnalysisTypeLBL;
    QLabel*      SelectYearsLBL;
    QLabel*      SelectFullyRecruitedAgeLBL;
    QComboBox*   SelectByVariablesCMB;
    QComboBox*   SelectSeasonCMB;
    QComboBox*   SelectYPRAnalysisTypeCMB;
    QComboBox*   SelectFullyRecruitedAgeCMB;
    QCheckBox*   SelectFullyRecruitedAgeCB;
    QLineEdit*   SelectFullyRecruitedAgeLE;
    QListWidget* SelectYearsLW;
    QHBoxLayout* FullyRecruitedLayt;

    MSVPAGuiControlsYieldPerRecruit();
   ~MSVPAGuiControlsYieldPerRecruit() {}

    virtual void resetWidgets() {}
    virtual void loadWidgets(nmfDatabase* databasePtr,
                             nmfLogger*   logger,
                             std::string  MSVPAName,
                             std::string  ModelName,
                             std::string  ForecastName,
                             std::string  ScenarioName) {}
    void loadSelectFullyRecruitedAgeCMB(nmfDatabase* databasePtr,
                                        std::string MSVPAName);
    void getSelectedYears(QListWidget *selectYearsLW,
                          std::vector<std::string> &selectedYears);


public slots:
    void callback_FullyRecruitedScaleChanged(bool value);

};

#endif // MSVPAGUICONTROLSYIELDPERRECRUIT_H
