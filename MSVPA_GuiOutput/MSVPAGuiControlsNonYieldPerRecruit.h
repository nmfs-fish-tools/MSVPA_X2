#ifndef MSVPAGUICONTROLSNONYIELDPERRECRUIT_H
#define MSVPAGUICONTROLSNONYIELDPERRECRUIT_H

#include "MSVPAGuiControls.h"

class MSVPAGuiControlsNonYieldPerRecruit : public MSVPAGuiControls
{

    Q_OBJECT

public:
    QLabel*      SelectByVariablesLBL;
    QLabel*      SelectSeasonLBL;
    QLabel*      SelectPredatorAgeSizeClassLBL;
    QLabel*      SelectPreyLBL;
    QComboBox*   SelectByVariablesCMB;
    QComboBox*   SelectSeasonCMB;
    QComboBox*   SelectPredatorAgeSizeClassCMB;
    QComboBox*   SelectPreyCMB;
    QCheckBox*   SeasonScaleCB;
    QCheckBox*   AgeSizeScaleCB;
    QLineEdit*   SeasonScaleLE;
    QLineEdit*   AgeSizeScaleLE;
    QHBoxLayout* SeasonLayt;
    QHBoxLayout* AgeSizeLayt;
    QDoubleValidator* validMaxSeasonScaleY;
    QDoubleValidator* validMaxAgeSizeScaleY;

    MSVPAGuiControlsNonYieldPerRecruit();
   ~MSVPAGuiControlsNonYieldPerRecruit() {}

    virtual void resetWidgets() {}
    virtual void loadWidgets(nmfDatabase* databasePtr,
                             nmfLogger*   logger,
                             std::string  MSVPAName,
                             std::string  ModelName,
                             std::string  ForecastName,
                             std::string  ScenarioName) {}
    void loadSelectSeasonCMB(nmfDatabase* databasePtr,
                             std::string MSVPAName);
    void loadSelectPredatorAgeSizeClassCMB(nmfDatabase* databasePtr,
                                           std::string MSVPAName);

signals:
    void UpdateTheme(int newTheme);

public slots:
    void callback_SeasonScaleChanged(bool value);
    void callback_AgeSizeScaleChanged(bool value);
};

#endif // MSVPAGUICONTROLSNONYIELDPERRECRUIT_H
