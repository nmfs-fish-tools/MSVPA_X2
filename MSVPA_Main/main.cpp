//============================================================================
// Name        : main.cpp
// Author      : Ron Klasky
// Version     :
// Copyright   : NOAA - National Marine Fisheries Service
// Description : Main routine for MSVPA_X2
//============================================================================


#include "nmfMainWindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    nmfMainWindow w;
    if (w.isStartUpOK()) {
        w.show();
        return a.exec();
    }
    return 0;
}

