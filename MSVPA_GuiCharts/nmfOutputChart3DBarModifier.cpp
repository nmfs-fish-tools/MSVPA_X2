
//============================================================================
// Name        : nmfBarGraphModifier.cpp
// Author      : Ron Klasky
// Version     :
// Copyright   : NOAA - National Marine Fisheries Service
// Description :
//============================================================================

#include "nmfOutputChart3DBarModifier.h"

#include <QtCore/QTime>
#include <QtWidgets/QComboBox>
#include <QtCore/qmath.h>


#include <math.h>

using namespace QtDataVisualization;


nmfOutputChart3DBarModifier::nmfOutputChart3DBarModifier(Q3DBars *bargraph)
    : m_graph(bargraph),
      m_xRotation(0.0f),
      m_yRotation(0.0f),
      m_fontSize(30),
      m_segments(4),
      m_subSegments(3),
      m_minval(-20.0f),
      m_maxval(20.0f),
      m_temperatureAxis(new QValue3DAxis),
      m_yearAxis(new QCategory3DAxis),
      m_monthAxis(new QCategory3DAxis),
      m_primarySeries(new QBar3DSeries),
      m_secondarySeries(new QBar3DSeries),
      m_barMesh(QAbstract3DSeries::MeshBevelBar),
      m_smooth(false)
{

    m_primarySeries->setMesh(QAbstract3DSeries::MeshBevelBar);
    m_primarySeries->setMeshSmooth(false);

    m_graph->addSeries(m_primarySeries);

}

nmfOutputChart3DBarModifier::~nmfOutputChart3DBarModifier()
{
    delete m_graph;
}

Q3DBars*
nmfOutputChart3DBarModifier::getGraph()
{
    return m_graph;
}


void
nmfOutputChart3DBarModifier::changeSelectionMode(int mode)
{
    m_graph->setSelectionMode(QAbstract3DGraph::SelectionFlags(mode));
}

void
nmfOutputChart3DBarModifier::resetData(boost::numeric::ublas::matrix<double> &M2Matrix)
{
    QBarDataArray *dataSet1 = new QBarDataArray;
    QBarDataRow *dataRow1;
    char buf[50];


    dataSet1->reserve(M2Matrix.size1());
    for (unsigned int i = 0; i < M2Matrix.size1(); i++) {
        m_data1 << std::to_string(1982+i).c_str();
        // Create a data row
        dataRow1 = new QBarDataRow(M2Matrix.size2());
        for (unsigned int j = 0; j < M2Matrix.size2(); j++) {
            // Add data to the row
            (*dataRow1)[j].setValue(M2Matrix(i,j));
        }
        // Add the row to the set
        dataSet1->append(dataRow1);
    }
    for (unsigned int j = 0; j < M2Matrix.size2(); j++) {
        sprintf(buf,"Age %d",j);
        m_data2 << buf;

    }
    m_primarySeries->dataProxy()->resetArray(dataSet1, m_data1, m_data2 );}


void
nmfOutputChart3DBarModifier::callback_changeSelectionMode(int selectionMode)
{
std::cout << "*** nmfOutputChart3DBarModifier callback" << std::endl;
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox) {
        int flags = comboBox->itemData(selectionMode).toInt();
        m_graph->setSelectionMode(QAbstract3DGraph::SelectionFlags(flags));
    }
}

