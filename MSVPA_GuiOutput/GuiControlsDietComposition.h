#ifndef GUICONTROLSDIETCOMPOSITION_H
#define GUICONTROLSDIETCOMPOSITION_H

#include "MSVPAGuiControlsNonYieldPerRecruit.h"
#include "nmfStructsQt.h"

class GuiControlsDietComposition : public MSVPAGuiControlsNonYieldPerRecruit
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
    GuiControlsDietComposition();
    virtual ~GuiControlsDietComposition();

    virtual void resetWidgets();

    void loadWidgets(nmfDatabase* databasePtr,
                     nmfLogger*   logger,
                     std::string  MSVPAName,
                     std::string  ModelName,
                     std::string  ForecastName,
                     std::string  ScenarioName);
    void loadSelectPredatorCMB(nmfDatabase* databasePtr,
                               std::string MSVPAName);

    nmfStructsQt::UpdateDataStruct getUpdateDataStruct();

signals:
    void UpdateChart(nmfStructsQt::UpdateDataStruct data);
    void UpdateChartGridLines(nmfStructsQt::UpdateDataStruct data);

public slots:
    void callback_SelectPredatorChanged(QString value);
    void callback_SelectVariableChanged(QString value);
    void callback_SelectByVariablesChanged(QString value);
    void callback_SelectSeasonChanged(QString value);
    void callback_SelectPredatorAgeSizeClassChanged(QString value);
    void callback_HorizontalGridLinesChanged(bool toggle);
    void callback_VerticalGridLinesChanged(bool toggle);
    void callback_chartThemeChanged(int value);

};

#endif // GUICONTROLSDIETCOMPOSITION_H
