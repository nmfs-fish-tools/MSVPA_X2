#include "MSVPAX2VisualizationNode.h"

MSVPAX2VisualizationNode::MSVPAX2VisualizationNode(QWidget *parent) : QWidget(parent)
{
    // Create and load first label and combo box
    SelectDataTypeLBL = new QLabel("Select Data Type:");
    SelectDataTypeCMB = new QComboBox();
    SelectDataTypeCMB->addItem("Diet Composition");
    SelectDataTypeCMB->addItem("Population Size");
    SelectDataTypeCMB->addItem("Mortality Rates");
    SelectDataTypeCMB->addItem("Consumption Rates");
    SelectDataTypeCMB->addItem("Yield Per Recruit");
    SelectDataTypeCMB->addItem("Food Availability");
    SelectDataTypeCMB->addItem("Multispecies Populations");
    SelectDataTypeCMB->addItem("Growth");
    SelectDataTypeLBL->setObjectName("SelectDataTypeLBL");
    SelectDataTypeCMB->setObjectName("SelectDataTypeCMB");

}
