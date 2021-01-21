#ifndef GUICONTROLSYIELDPERRECRUITFORECAST_H
#define GUICONTROLSYIELDPERRECRUITFORECAST_H

#include "MSVPAGuiControlsYieldPerRecruit.h"
#include "nmfStructsQt.h"

class GuiControlsYieldPerRecruitForecast : public MSVPAGuiControlsYieldPerRecruit
{
    Q_OBJECT

private:
    nmfDatabase* databasePtr;
    nmfLogger*   logger;
    std::string  MSVPAName;
    std::string  ModelName;
    std::string  ForecastName;
    std::string  ScenarioName;

public:
    GuiControlsYieldPerRecruitForecast();
    virtual ~GuiControlsYieldPerRecruitForecast();

    virtual void resetWidgets();
    void loadWidgets(nmfDatabase* databasePtr,
                     nmfLogger*   logger,
                     std::string  MSVPAName,
                     std::string  ModelName,
                     std::string  ForecastName,
                     std::string  ScenarioName);
    nmfStructsQt::UpdateDataStruct getUpdateDataStruct();
    void loadSelectPredatorCMB(nmfDatabase* databasePtr,
                               std::string MSVPAName);
    void loadSelectYearsLW(nmfDatabase* databasePtr,
                           std::string MSVPAName);
    void checkForProjectedType();

signals:
    void UpdateChart(nmfStructsQt::UpdateDataStruct data);
    void UpdateChartGridLines(nmfStructsQt::UpdateDataStruct data);

public slots:
    void callback_SelectPredatorChanged(QString value);
    void callback_SelectYPRAnalysisTypeChanged(QString value);

    void callback_HorizontalGridLinesChanged(bool toggled);
    void callback_VerticalGridLinesChanged(bool toggled);
    void callback_SelectYearsChanged();
    void callback_SelectFullyRecruitedAgeChanged(QString value);
    void callback_FullyRecruitedAgeCBChanged(int value);
    void callback_FullyRecruitedAgeLEChanged(QString value);
    void callback_chartThemeChanged(int newTheme);

};

#endif // GUICONTROLSYIELDPERRECRUITFORECAST_H
