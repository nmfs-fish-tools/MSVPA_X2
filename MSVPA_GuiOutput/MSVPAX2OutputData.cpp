#include "MSVPAX2OutputData.h"

MSVPAX2OutputData::MSVPAX2OutputData(QWidget *parent) : QWidget(parent)
{

    outputDataTW = new QTableWidget();
    outputDataTW->setObjectName("DataTableWidget");

}

QTableWidget*
MSVPAX2OutputData::widget()
{
    return outputDataTW;
}


