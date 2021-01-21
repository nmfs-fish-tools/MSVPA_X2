#ifndef MSVPAVISUALIZATIONNODE_H
#define MSVPAVISUALIZATIONNODE_H

#include "MSVPAX2VisualizationNode.h"
#include "MSVPAX2OutputData.h"
#include "MSVPAGuiControls.h"
#include "MSVPAGuiControlsNonYieldPerRecruit.h"
#include "MSVPAGuiControlsYieldPerRecruit.h"
#include "GuiControlsDietComposition.h"
#include "GuiControlsPopulationSize.h"
#include "GuiControlsMortalityRates.h"
#include "GuiControlsMortalityRatesForecast.h"
#include "GuiControlsConsumptionRates.h"
#include "GuiControlsYieldPerRecruit.h"
#include "GuiControlsYieldPerRecruitForecast.h"
#include "GuiControlsFoodAvailability.h"
#include "GuiControlsMultispeciesPopulations.h"
#include "GuiControlsGrowth.h"

#include "MSVPAX2OutputChart.h"
#include "ChartStackedBarDietComposition.h"
#include "ChartBarPopulationSize.h"
#include "ChartBarConsumptionRates.h"
#include "ChartBarFoodAvailability.h"
#include "ChartBarMortalityRates.h"
#include "ChartBarGrowth.h"
#include "ChartLineMultispeciesPopulations.h"
#include "ChartLineYieldPerRecruit.h"


class MSVPAVisualizationNode : public MSVPAX2VisualizationNode
{

    Q_OBJECT

private:
    MSVPAX2OutputChart*  OutputChart;
    MSVPAX2OutputData*   OutputData;
    MSVPAGuiControls*    OutputGuiControls;

    QWidget*             subGuiW;
    QHBoxLayout*         mainLayt;
    QSplitter*           mainSplitter;
    QVBoxLayout*         mainGuiLayt;
    QVBoxLayout*         subGuiLayt;
    QTabWidget*          outputTW;
    QGroupBox*           guiControlsGB;
    std::map<QString, std::shared_ptr<MSVPAGuiControls> >   GuiControls;
    std::map<QString, std::shared_ptr<MSVPAX2OutputChart> > OutputCharts;
    std::vector<QString>        DataTypes;
    std::map<QString, QFrame* > DataTypeMap;
    std::string          MSVPAName;
    std::string          ModelName;
    std::string          ForecastName;
    std::string          ScenarioName;
    nmfDatabase*         databasePtr;
    nmfLogger*           logger;


public:
    MSVPAVisualizationNode(nmfLogger* logger);
    virtual ~MSVPAVisualizationNode();

    QTabWidget*   tabWidget();
    QGroupBox*    controlsWidget();

    QHBoxLayout* mainLayout();
    void setDatabaseVars(nmfDatabase* databasePtr,
                         nmfLogger*   logger,
                         std::string  MSVPAName,
                         std::string  ModelName,
                         std::string  ForecastName,
                         std::string  ScenarioName);

public Q_SLOTS:
    void callback_SelectDataTypeChanged(QString value);

};

#endif // MSVPAVISUALIZATIONNODE_H
