#ifndef GUICONTROLSMULTISPECIESPOPULATIONS_H
#define GUICONTROLSMULTISPECIESPOPULATIONS_H

#include "MSVPAGuiControlsNonYieldPerRecruit.h"
#include "nmfStructsQt.h"

class GuiControlsMultispeciesPopulations : public MSVPAGuiControlsNonYieldPerRecruit
{
    Q_OBJECT

private:
    nmfDatabase* databasePtr;
    nmfLogger*   logger;
    std::string  MSVPAName;
    std::string  ModelName;
    std::string  ForecastName;
    std::string  ScenarioName;

signals:
    void UpdateChart(nmfStructsQt::UpdateDataStruct data);
    void UpdateChartGridLines(nmfStructsQt::UpdateDataStruct data);

public:
    GuiControlsMultispeciesPopulations();
    virtual ~GuiControlsMultispeciesPopulations();

    virtual void resetWidgets();
    nmfStructsQt::UpdateDataStruct getUpdateDataStruct();
    void loadWidgets(nmfDatabase* databasePtr,
                     nmfLogger*   logger,
                     std::string  MSVPAName,                                         std::string  theModelName,
                     std::string  ForecastName,
                     std::string  ScenarioName);
public slots:
    void callback_SelectVariableChanged(QString value);
    void callback_SelectByVariablesChanged(QString value);
    void callback_SelectSeasonChanged(QString value);
    void callback_SelectPredatorAgeSizeClassChanged(QString value);
    void callback_HorizontalGridLinesChanged(bool toggled);
    void callback_VerticalGridLinesChanged(bool toggled);
    void callback_SeasonScaleChanged(QString value);
    void callback_SeasonScaleCBChanged(int toggle);
    void callback_chartThemeChanged(int newTheme);

};

#endif // GUICONTROLSMULTISPECIESPOPULATIONS_H
