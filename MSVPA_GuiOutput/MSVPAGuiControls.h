#ifndef MSVPAGUICONTROLS_H
#define MSVPAGUICONTROLS_H

#include "MSVPAX2GuiControls.h"

struct UpdateDataStruct {
    nmfDatabase* databasePtr;
    nmfLogger*   logger;
    std::string  MSVPAName;
    std::string  ForecastName;
    std::string  ScenarioName;
    std::string  ModelName;
    int  Theme;
    int  NumAgeSizeClasses;
    bool HorizontalGridLines;
    bool VerticalGridLines;
    std::string DataTypeLabel;
    std::string XLabel;
    std::string YLabel;
    std::string TitlePrefix;
    std::string TitleSuffix;
    QString SelectDataType;
    QString SelectPredator;
    QString SelectVariable;
    QString SelectByVariables;
    QString SelectSeason;
    QString SelectPredatorAgeSizeClass;
    QString SelectPreyName;
    QString SelectYPRAnalysisType;
    QString SelectFullyRecruitedAge;
    QString MaxScaleY;
    QString SelectMode;
    std::vector<std::string> SelectedYears;
};

class MSVPAGuiControls : public MSVPAX2GuiControls
{

public:
    QVBoxLayout* MainLayt;

    MSVPAGuiControls();
    virtual ~MSVPAGuiControls() {}

    QVBoxLayout* mainLayout();

    virtual void loadWidgets(nmfDatabase* databasePtr,
                             nmfLogger*   logger,
                             std::string  MSVPAName,
                             std::string  ModelName,
                             std::string  ForecastName,
                             std::string  ScenarioName) = 0;


};

#endif // MSVPAGUICONTROLS_H
