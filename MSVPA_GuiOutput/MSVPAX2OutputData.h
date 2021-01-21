#ifndef MSVPAX2OUTPUTDATA_H
#define MSVPAX2OUTPUTDATA_H

#include "MSVPAGuiControls.h"

class MSVPAX2OutputData : public QWidget
{
    Q_OBJECT

private:
    QTableWidget *outputDataTW;

public:
    explicit MSVPAX2OutputData(QWidget *parent = 0);

    QTableWidget* widget();

signals:

public slots:

};

#endif // MSVPAX2OUTPUTDATA_H
