
#include "nmfSetupTab01.h"
#include "nmfConstants.h"



nmfSetup_Tab1::nmfSetup_Tab1(QTabWidget *tabs)
{
    QUiLoader loader;

    Setup_Tabs = tabs;

    // Load ui as a widget from disk
    QFile file(":/forms/Setup/Setup_Tab01.ui");
    file.open(QFile::ReadOnly);
    Setup_Tab1_Widget = loader.load(&file,Setup_Tabs);
    Setup_Tab1_Widget->setObjectName("Setup_Tab1_Widget");
    file.close();

    // Add the loaded widget as the new tabbed page
    Setup_Tabs->addTab(Setup_Tab1_Widget, tr("1. Getting Started"));

}


nmfSetup_Tab1::~nmfSetup_Tab1()
{

}


//void
//nmfSetupTab1::callback_Forecast_Tab1_NextPage()
//{
//    Forecast_Tabs->setCurrentIndex(Forecast_Tabs->currentIndex()+1);
//}


void
nmfSetup_Tab1::loadWidgets()
{


} // end loadWidgets


void
nmfSetup_Tab1::clearWidgets()
{

}
