#include "MSVPAX2GuiControls.h"

MSVPAX2GuiControls::MSVPAX2GuiControls(QWidget *parent) : QWidget(parent)
{
    // Create gui widgets
    SelectPredatorLBL             = new QLabel();
    SelectVariableLBL             = new QLabel();
    GridLinesLBL                  = new QLabel();
    SelectionModeLBL              = new QLabel();
    ColorSchemesLBL               = new QLabel();
    SelectPredatorCMB             = new QComboBox();
    SelectVariableCMB             = new QComboBox();
    SelectionModeCMB              = new QComboBox();
    ThemesCMB                     = new QComboBox();
    HorizontalLinesCB             = new QCheckBox("Horizontal");
    VerticalLinesCB               = new QCheckBox("Vertical");
    GridLinesLayt                 = new QHBoxLayout();

    // Load main gui elements
    SelectPredatorLBL->setText("Select Species:");
    SelectVariableLBL->setText("Select Variable:");
    SelectionModeLBL->setText("Selection Mode:");
    GridLinesLBL->setText("Grid Lines");
    GridLinesLayt->addWidget(HorizontalLinesCB);
    GridLinesLayt->addWidget(VerticalLinesCB);

    ColorSchemesLBL->setText("Color schemes:");
    ThemesCMB->addItem("Light");
    ThemesCMB->addItem("Blue Cerulean");
    ThemesCMB->addItem("Dark");
    ThemesCMB->addItem("Brown Sand");
    ThemesCMB->addItem("Blue NCS");
    ThemesCMB->addItem("High Contrast");
    ThemesCMB->addItem("Blue Icy");
    ThemesCMB->addItem("Qt");

    ColorSchemesLBL->setToolTip("Change the color theme for the current chart.");
    ThemesCMB->setStatusTip("Change the color theme for the current chart.");

}
