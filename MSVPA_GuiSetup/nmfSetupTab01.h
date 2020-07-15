#ifndef NMFSETUPTAB1_H
#define NMFSETUPTAB1_H

#include <QFile>
#include <QString>
#include <QTabWidget>
#include <QTextEdit>
#include <QUiLoader>
#include <QVBoxLayout>
#include <QWidget>

class nmfSetup_Tab1: public QObject
{
    Q_OBJECT

    QTabWidget  *Setup_Tabs;
    QWidget     *Setup_Tab1_Widget;

public:

    nmfSetup_Tab1(QTabWidget *tabs);
    virtual ~nmfSetup_Tab1();
    void clearWidgets();
    void loadWidgets();


//public Q_SLOTS:
//    void callback_Forecast_Tab1_NextPage();


};

#endif // NMFSETUPTAB1_H
