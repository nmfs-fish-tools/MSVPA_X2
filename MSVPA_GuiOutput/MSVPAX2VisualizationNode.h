#ifndef MSVPAX2VISUALIZATIONNODE_H
#define MSVPAX2VISUALIZATIONNODE_H

class MSVPAX2VisualizationNode : public QWidget
{

public:

    QLabel*    SelectDataTypeLBL;
    QComboBox* SelectDataTypeCMB;

    explicit MSVPAX2VisualizationNode(QWidget *parent = 0);

signals:

public slots:

};

#endif // MSVPAX2VISUALIZATIONNODE_H
