#ifndef GUICONTROLSMORTALITYRATES_H
#define GUICONTROLSMORTALITYRATES_H

#include "MSVPAGuiControlsNonYieldPerRecruit.h"
#include "nmfStructsQt.h"

class GuiControlsMortalityRates : public MSVPAGuiControlsNonYieldPerRecruit
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
    GuiControlsMortalityRates();
    virtual ~GuiControlsMortalityRates();

    virtual void resetWidgets();
    nmfStructsQt::UpdateDataStruct getUpdateDataStruct();
    void loadWidgets(nmfDatabase* databasePtr,
                     nmfLogger*   logger,
                     std::string  MSVPAName,
                     std::string  ModelName,
                     std::string  ForecastName,
                     std::string  ScenarioName);
    void loadSelectPredatorCMB(nmfDatabase* databasePtr,
                               std::string MSVPAName);

signals:
    void UpdateChart(nmfStructsQt::UpdateDataStruct data);
    void UpdateChartGridLines(nmfStructsQt::UpdateDataStruct data);

public slots:
    void callback_SelectPredatorChanged(QString value);
    void callback_SelectVariableChanged(QString value);
    void callback_SelectByVariablesChanged(QString value);
    void callback_SelectSeasonChanged(QString value);
    void callback_SelectPredatorAgeSizeClassChanged(QString value);
    void callback_HorizontalGridLinesChanged(bool toggled);
    void callback_VerticalGridLinesChanged(bool toggled);
    void callback_SeasonScaleChanged(QString value);
    void callback_AgeSizeScaleChanged(QString value);
    void callback_SeasonScaleCBChanged(int toggle);
    void callback_AgeSizeScaleCBChanged(int toggle);
    void callback_chartThemeChanged(int newTheme);
    void callback_SelectionModeChanged(int mode);

};

#endif // GUICONTROLSMORTALITYRATES_H
