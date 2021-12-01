#ifndef WORKFLOW_APP_EE_UQ_H
#define WORKFLOW_APP_EE_UQ_H
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
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

// Written by: Stevan Gavrilovic

#include "MainWindow.h"
#include "WorkflowAppWidget.h"
#include "WidgetFactory.h"

#include <QWidget>
#include <QJsonObject>

class SimCenterComponentSelection;
class InputWidgetBIM;
class InputWidgetUQ;
class PipelineNetworkWidget;
class DecisionVariableWidget;
class DamageMeasureWidget;
class UQOptions;
class ResultsWidget;
class CustomVisualizationWidget;
class EngDemandParamWidget;
class GeneralInformationWidget;
class IntensityMeasureWidget;
class LoadResultsDialog;
class RunLocalWidget;
class RunWidget;
class QGISVisualizationWidget;
class UncertaintyQuantificationWidget;

class QStackedWidget;
class Application;
class QNetworkAccessManager;
class QNetworkReply;

class WorkflowAppOpenSRA : public WorkflowAppWidget
{
    Q_OBJECT
public:
    explicit WorkflowAppOpenSRA(QWidget *parent = 0);
    ~WorkflowAppOpenSRA();

    static WorkflowAppOpenSRA *theInstance;

    static WorkflowAppOpenSRA *getInstance(void);

    void initialize(void);

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

    void onRunButtonClicked();
    void onRemoteRunButtonClicked();
    void onRemoteGetButtonClicked();
    void onExitButtonClicked();
    int getMaxNumParallelTasks();
    
    GeneralInformationWidget *getGeneralInformationWidget() const;
    QGISVisualizationWidget *getVisualizationWidget() const;

    QJsonObject getMethodsAndParamsObj() const;

    QMap<QString, QString> getMethodsAndParamsMap() const;

    WidgetFactory* getTheWidgetFactory() const;

    void setTheWidgetFactory(WidgetFactory* value);

signals:
    void setUpForApplicationRunDone(QString &tmpDirectory, QString &inputFile);
    void sendLoadFile(QString filename);

public slots:  
    void clear(void);

    void clearWorkDir(void);

    void setUpForApplicationRun(QString &, QString &);
    void processResults(QString doesNothing1, QString doesNothing2, QString doesNothing3);

    int loadFile(QString filename);
    void replyFinished(QNetworkReply*);

    // Load examples
    void loadExamples();

    // Load cached result
    void loadResults(void);

    void statusMessage(QString message);
    void infoMessage(QString message);
    void errorMessage(QString message);
    void fatalMessage(QString message);

private:

    std::unique_ptr<WidgetFactory> theWidgetFactory;

    // sidebar container selection
    SimCenterComponentSelection *theComponentSelection;

    // objects that go in sidebar
    GeneralInformationWidget* theGenInfoWidget = nullptr;
    UncertaintyQuantificationWidget* theUQWidget = nullptr;
    PipelineNetworkWidget* thePipelineNetworkWidget = nullptr;
    IntensityMeasureWidget* theIntensityMeasureWidget = nullptr;
    DecisionVariableWidget* theDecisionVariableWidget = nullptr;
    DamageMeasureWidget* theDamageMeasureWidget = nullptr;
    EngDemandParamWidget* theEDPWidget = nullptr;
    CustomVisualizationWidget* theCustomVisualizationWidget = nullptr;
    QGISVisualizationWidget* theVisualizationWidget = nullptr;
    ResultsWidget* theResultsWidget = nullptr;

    // objects for running the workflow and obtaining results
    RunWidget* theRunWidget;
    Application* localApp;

    QJsonObject* jsonObjOrig;

    LoadResultsDialog* resultsDialog;

    QJsonObject methodsAndParamsObj;
    QMap<QString, QString> methodsParamsMap;
};

#endif // WORKFLOW_APP_EE_UQ_H
