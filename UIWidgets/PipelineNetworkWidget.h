#ifndef UQ_ENGINE_SELECTION_H
#define UQ_ENGINE_SELECTION_H
/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written by: Dr. Stevan Gavrilovic, UC Berkeley

#include <MultiComponentR2D.h>

class VisualizationWidget;
class ComponentInputWidget;

class QGroupBox;
class QGISGasPipelineInputWidget;
class QGISWellsCaprocksInputWidget;
class QGISAboveGroundGasNetworkInputWidget;

class PipelineNetworkWidget : public  MultiComponentR2D
{
    Q_OBJECT

public:
    explicit PipelineNetworkWidget(QWidget *parent, VisualizationWidget* visWidget);
    ~PipelineNetworkWidget();

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);
    bool copyFiles(QString &destName);

    void clear(void);

    ComponentInputWidget* getTheCurrentComponentInputWidget() const;

signals:

    void componentChangedSignal(ComponentInputWidget* widget);

public slots:

    void handleComponentChanged(QString compName);

private:

    QGISGasPipelineInputWidget* theBelowGroundInputWidget = nullptr;
    QGISWellsCaprocksInputWidget* theWellsCaprocksWidget = nullptr;
    QGISAboveGroundGasNetworkInputWidget* theAboveGroundInfWidget = nullptr;

    VisualizationWidget* theVisualizationWidget;

    QVector<ComponentInputWidget*> vectorOfComponents;
};

#endif // WIND_SELECTION_H
