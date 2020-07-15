/**
 @file nmfBarGraphModifier.h
 @author rklasky
 @copyright 2017 NOAA - National Marine Fisheries Service
 @brief This contains the class definition for 3D Bar Graph.  If this file changes, moc must be re-run.
 @date Feb 12, 2016
 */

#ifndef NMFBARGRAPHMODIFIER_H
#define NMFBARGRAPHMODIFIER_H

#include <QtDataVisualization/Q3DBars>

#include <QtDataVisualization/qbardataproxy.h>
#include <QtDataVisualization/qabstract3dseries.h>
#include <QtDataVisualization/qcategory3daxis.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/qbardataproxy.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qbar3dseries.h>
#include <QtDataVisualization/q3dtheme.h>

#include <QtGui/QFont>
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QPointer>
#include <QtCore/QPropertyAnimation>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/lexical_cast.hpp>

using namespace QtDataVisualization;


class nmfOutputChart3DBarModifier : public QObject
{
    Q_OBJECT
public:
    explicit nmfOutputChart3DBarModifier(Q3DBars *bargraph);
    ~nmfOutputChart3DBarModifier();

    void resetData(boost::numeric::ublas::matrix<double> &M2Matrix);
    Q3DBars* getGraph();
    void changeSelectionMode(int mode);


public Q_SLOTS:

    void callback_changeSelectionMode(int selectionMode);


private:
    Q3DBars *m_graph;
    float m_xRotation;
    float m_yRotation;
    int m_fontSize;
    int m_segments;
    int m_subSegments;
    float m_minval;
    float m_maxval;
    QStringList m_data1;
    QStringList m_data2;
    QValue3DAxis *m_temperatureAxis;
    QCategory3DAxis *m_yearAxis;
    QCategory3DAxis *m_monthAxis;
    QBar3DSeries *m_primarySeries;
    QBar3DSeries *m_secondarySeries;
    QAbstract3DSeries::Mesh m_barMesh;
    bool m_smooth;
    QPropertyAnimation m_animationCameraX;
    QPropertyAnimation m_animationCameraY;
    QPropertyAnimation m_animationCameraZoom;
    QPropertyAnimation m_animationCameraTarget;
    float m_defaultAngleX;
    float m_defaultAngleY;
    float m_defaultZoom;
    QVector3D m_defaultTarget;

};


#endif // NMFBARGRAPHMODIFIER_H
