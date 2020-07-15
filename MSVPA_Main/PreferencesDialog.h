#pragma once

#include "nmfDatabase.h"

#include <QTableWidget>
#include <QComboBox>
#include <QVBoxLayout>

class PreferencesDialog : public QDialog
{
    Q_OBJECT

    nmfDatabase* m_databasePtr;

    QVBoxLayout* MainLT;
    QHBoxLayout* BtnLT;
    QHBoxLayout* StyleLT;
    QLabel*      StyleLBL;
    QComboBox*   StyleCMB;
    QPushButton* CancelPB;
    QPushButton* OkPB;

signals:
    void SetStyleSheet(QString style);

public:
    PreferencesDialog(QWidget*     parent,
                      nmfDatabase* databasePtr);
    virtual ~PreferencesDialog() {}

    void loadWidgets();

private Q_SLOTS:
    void callback_StyleCMB(QString algorithm);

};

