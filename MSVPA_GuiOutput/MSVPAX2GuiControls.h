#ifndef MSVPAX2GUICONTROLS_H
#define MSVPAX2GUICONTROLS_H

class MSVPAX2GuiControls : public QWidget
{
    Q_OBJECT

public:

    QLabel*      SelectPredatorLBL;
    QLabel*      SelectVariableLBL;
    QLabel*      SelectionModeLBL;
    QLabel*      GridLinesLBL;
    QLabel*      ColorSchemesLBL;
    QComboBox*   SelectPredatorCMB;
    QComboBox*   SelectVariableCMB;
    QComboBox*   SelectionModeCMB;
    QComboBox*   ThemesCMB;
    QCheckBox*   HorizontalLinesCB;
    QCheckBox*   VerticalLinesCB;
    QHBoxLayout* GridLinesLayt;

    explicit MSVPAX2GuiControls(QWidget *parent = 0);

    virtual void resetWidgets() = 0;

signals:
    void UpdateTheme(int newTheme);

public slots:

};

#endif // MSVPAX2GUICONTROLS_H
