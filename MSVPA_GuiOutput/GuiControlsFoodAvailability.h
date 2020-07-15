#ifndef GUICONTROLSFOODAVAILABILITY_H
#define GUICONTROLSFOODAVAILABILITY_H

#include "MSVPAGuiControlsNonYieldPerRecruit.h"
#include "nmfStructsQt.h"

class GuiControlsFoodAvailability : public MSVPAGuiControlsNonYieldPerRecruit
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
    GuiControlsFoodAvailability();
    virtual ~GuiControlsFoodAvailability();

    virtual void resetWidgets();
    void loadWidgets(nmfDatabase* databasePtr,
                     nmfLogger*   logger,
                     std::string  MSVPAName,
                     std::string  ModelName,
                     std::string  ForecastName,
                     std::string  ScenarioName);
    void loadSelectPredatorCMB(nmfDatabase* databasePtr,
                               std::string  MSVPAName);
    void loadSelectPreyCMB(nmfDatabase* databasePtr,
                           std::string  MSVPAName);
    nmfStructsQt::UpdateDataStruct getUpdateDataStruct();

signals:
    void UpdateChart(nmfStructsQt::UpdateDataStruct data);
    void UpdateChartGridLines(nmfStructsQt::UpdateDataStruct data);

public slots:
    void callback_SelectVariableChanged(QString value);
    void callback_SelectByVariablesChanged(QString value);
    void callback_SelectPredatorChanged(QString value);
    void callback_SelectSeasonChanged(QString value);
    void callback_SelectPredatorAgeSizeClassChanged(QString value);
    void callback_SelectPreyChanged(QString value);
    void callback_HorizontalGridLinesChanged(bool toggled);
    void callback_VerticalGridLinesChanged(bool toggled);
    void callback_SeasonScaleChanged(QString value);
    void callback_AgeSizeScaleChanged(QString value);
    void callback_SeasonScaleCBChanged(int toggle);
    void callback_AgeSizeScaleCBChanged(int toggle);
    void callback_chartThemeChanged(int newTheme);

};

#endif // GUICONTROLSFOODAVAILABILITY_H
