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

#include "CustomizedItemModel.h"
#include "DamageMeasureWidget.h"
#include "DecisionVariableWidget.h"
#include "EngDemandParamWidget.h"
#include "UIWidgets/GeneralInformationWidget.h"
#include "IntensityMeasureWidget.h"
#include "LocalApplication.h"
#include "PipelineNetworkWidget.h"
#include "RunLocalWidget.h"
#include "RunWidget.h"
#include "UIWidgets/ResultsWidget.h"
#include "SimCenterComponentSelection.h"
#include "UIWidgets/CustomVisualizationWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppOpenSRA.h"
#include "UncertaintyQuantificationWidget.h"
#include "MainWindowWorkflowApp.h"
#include "OpenSRAPreferences.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QUuid>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

// static pointer for global procedure set in constructor
static WorkflowAppOpenSRA *theApp = nullptr;

// global procedure
int getNumParallelTasks()
{
    return theApp->getMaxNumParallelTasks();
}


WorkflowAppOpenSRA* WorkflowAppOpenSRA::getInstance()
{
    return theInstance;
}


WorkflowAppOpenSRA *WorkflowAppOpenSRA::theInstance = nullptr;

WorkflowAppOpenSRA::WorkflowAppOpenSRA(QWidget *parent) : WorkflowAppWidget(parent)
{
    // set static pointer for global procedure
    theApp = this;

    theInstance = this;
}


WorkflowAppOpenSRA::~WorkflowAppOpenSRA()
{

}


void WorkflowAppOpenSRA::initialize(void)
{
    // Load the examples
    auto pathToExamplesJson = QCoreApplication::applicationDirPath() + QDir::separator() + "Examples" + QDir::separator() + "Examples.json";
    // QString pathToExamplesJson = "/Users/steve/Desktop/SimCenter/RDT/RDT/Examples/";

    QFile jsonFile(pathToExamplesJson);
    jsonFile.open(QFile::ReadOnly);
    QJsonDocument exDoc = QJsonDocument::fromJson(jsonFile.readAll());

    auto docObj = exDoc.object();

    auto exContainerObj = docObj.value("Examples").toObject();

    auto numEx = exContainerObj.count();

    if(numEx > 0)
    {

        QMenu *exampleMenu = theMainWindow->menuBar()->addMenu(tr("&Examples"));

        for(auto it = exContainerObj.begin(); it!=exContainerObj.end(); ++it)
        {
            auto name = it.key();

            auto exObj = exContainerObj.value(name).toObject();

            auto inputFile = exObj.value("InputFile").toString();

            // Set the path to the input file
            auto action = exampleMenu->addAction(name, this, &WorkflowAppOpenSRA::loadExamples);
            action->setProperty("InputFile",inputFile);
        }
    }

    // Clear action
    QMenu *editMenu = theMainWindow->menuBar()->addMenu(tr("&Edit"));
    // Set the path to the input file
    editMenu->addAction("Clear Inputs", this, &WorkflowAppOpenSRA::clear);
    editMenu->addAction("Clear Working Directory", this, &WorkflowAppOpenSRA::clearWorkDir);

    // Help menu
    theMainWindow->createHelpMenu();

    theVisualizationWidget = new VisualizationWidget(this);

    // Create the various widgets
    theGenInfoWidget = new GeneralInformationWidget(this);
    theUQWidget = new UncertaintyQuantificationWidget(this);
    theIntensityMeasureWidget = new IntensityMeasureWidget(theVisualizationWidget, this);
    theDamageMeasureWidget = new DamageMeasureWidget(this);
    thePipelineNetworkWidget = new PipelineNetworkWidget(this,theVisualizationWidget);
    theEDPWidget = new EngDemandParamWidget(this);
    theCustomVisualizationWidget = new CustomVisualizationWidget(this,theVisualizationWidget);
    theDecisionVariableWidget = new DecisionVariableWidget(this);
    theResultsWidget = new ResultsWidget(this,theVisualizationWidget);

    SimCenterWidget *theWidgets[1];

    localApp = new LocalApplication("OpenSRA.py",theMainWindow);
    theRunWidget = new RunWidget(localApp, theWidgets, 0);

    connect(theRunWidget,SIGNAL(sendErrorMessage(QString)), this,SLOT(errorMessage(QString)));
    connect(theRunWidget,SIGNAL(sendStatusMessage(QString)), this,SLOT(statusMessage(QString)));
    connect(theRunWidget,SIGNAL(sendFatalMessage(QString)), this,SLOT(fatalMessage(QString)));

    connect(localApp,SIGNAL(sendErrorMessage(QString)), this,SLOT(errorMessage(QString)));
    connect(localApp,SIGNAL(sendStatusMessage(QString)), this,SLOT(statusMessage(QString)));
    connect(localApp,SIGNAL(sendFatalMessage(QString)), this,SLOT(fatalMessage(QString)));
    connect(localApp,SIGNAL(setupForRun(QString &,QString &)), this, SLOT(setUpForApplicationRun(QString &,QString &)));
    connect(this,SIGNAL(setUpForApplicationRunDone(QString&, QString &)), theRunWidget, SLOT(setupForRunApplicationDone(QString&, QString &)));
    connect(localApp,SIGNAL(processResults(QString)), this, SLOT(processResults(QString)));

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    this->setLayout(horizontalLayout);
    this->setContentsMargins(0,5,0,5);
    horizontalLayout->setMargin(0);

    // Create the component selection & add the components to it
    theComponentSelection = new SimCenterComponentSelection();
    horizontalLayout->addWidget(theComponentSelection);

    theComponentSelection->addComponent(QString("Visualization"), theCustomVisualizationWidget);
    theComponentSelection->addComponent(QString("General\nInformation"), theGenInfoWidget);
    theComponentSelection->addComponent(QString("Uncertainty\nQuantification"), theUQWidget);
    theComponentSelection->addComponent(QString("Infrastructure"), thePipelineNetworkWidget);
    theComponentSelection->addComponent(QString("Intensity\nMeasure"), theIntensityMeasureWidget);
    theComponentSelection->addComponent(QString("Engineering\nDemand\nParameter"), theEDPWidget);
    theComponentSelection->addComponent(QString("Damage\nMeasure"), theDamageMeasureWidget);
    theComponentSelection->addComponent(QString("Decision\nVariable"), theDecisionVariableWidget);
    theComponentSelection->addComponent(QString("Results"), theResultsWidget);
    theComponentSelection->setWidth(120);
    theComponentSelection->setItemWidthHeight(120,70);

    theComponentSelection->displayComponent("Visualization");
}



void WorkflowAppOpenSRA::loadExamples()
{
    auto pathToExample = QCoreApplication::applicationDirPath() + QDir::separator() + "Examples" + QDir::separator();
    pathToExample += QObject::sender()->property("InputFile").toString();

    if(pathToExample.isNull())
    {
        qDebug()<<"Error loading examples";
        return;
    }

    this->loadFile(pathToExample);
}


void WorkflowAppOpenSRA::replyFinished(QNetworkReply */*pReply*/)
{
    return;
}

VisualizationWidget *WorkflowAppOpenSRA::getVisualizationWidget() const
{
    return theVisualizationWidget;
}


GeneralInformationWidget *WorkflowAppOpenSRA::getGeneralInformationWidget() const
{
    return theGenInfoWidget;
}



bool WorkflowAppOpenSRA::outputToJSON(QJsonObject &jsonObjectTop)
{
    // Get each of the main widgets to output themselves
    theGenInfoWidget->outputToJSON(jsonObjectTop);
    theUQWidget->outputToJSON(jsonObjectTop);
    thePipelineNetworkWidget->outputToJSON(jsonObjectTop);
    theIntensityMeasureWidget->outputToJSON(jsonObjectTop);
    theDecisionVariableWidget->outputToJSON(jsonObjectTop);
    theDamageMeasureWidget->outputToJSON(jsonObjectTop);
    theEDPWidget->outputToJSON(jsonObjectTop);

    return true;
}


void WorkflowAppOpenSRA::processResults(QString /*pathToResults*/)
{
    theResultsWidget->processResults();
    theRunWidget->hide();
    theComponentSelection->displayComponent("Results");

    statusMessage("Analysis complete");
}


void WorkflowAppOpenSRA::clear(void)
{
    theGenInfoWidget->clear();
    theUQWidget->clear();
    thePipelineNetworkWidget->clear();
    theIntensityMeasureWidget->clear();
    theDecisionVariableWidget->clear();
    theDamageMeasureWidget->clear();
    theEDPWidget->clear();
    theCustomVisualizationWidget->clear();
    theResultsWidget->clear();
}

void WorkflowAppOpenSRA::clearWorkDir(void)
{
    auto thePreferences = OpenSRAPreferences::getInstance(this);

    QString workDirectoryPath =  thePreferences->getLocalWorkDir();

    QDir workDirectory(workDirectoryPath);

    if(workDirectory.exists()) {
        workDirectory.removeRecursively();
    }
}


bool WorkflowAppOpenSRA::inputFromJSON(QJsonObject &jsonObject)
{
    auto genJsonObj = jsonObject["General"].toObject();
    theGenInfoWidget->inputFromJSON(genJsonObj);

    auto UQJsonObj = jsonObject["UncertaintyQuantification"].toObject();
    theUQWidget->inputFromJSON(UQJsonObj);

    auto InfraJsonObj = jsonObject["Infrastructure"].toObject();
    thePipelineNetworkWidget->inputFromJSON(InfraJsonObj);

    auto IntensityMeasObj = jsonObject["IntensityMeasure"].toObject();
    theIntensityMeasureWidget->inputFromJSON(IntensityMeasObj);

    auto EDPObj = jsonObject["EngineeringDemandParameter"].toObject();
    theEDPWidget->inputFromJSON(EDPObj);

    auto DamageMeasureObj = jsonObject["DamageMeasure"].toObject();
    theDamageMeasureWidget->inputFromJSON(DamageMeasureObj);

    auto DecisionVarObj = jsonObject["DecisionVariable"].toObject();
    theDecisionVariableWidget->inputFromJSON(DecisionVarObj);

    return true;
}


void WorkflowAppOpenSRA::onRunButtonClicked()
{
    theRunWidget->hide();
    theRunWidget->setMinimumWidth(this->width()*0.5);
    theRunWidget->showLocalApplication();
}


void WorkflowAppOpenSRA::onRemoteRunButtonClicked()
{

}


void WorkflowAppOpenSRA::onRemoteGetButtonClicked()
{


}


void WorkflowAppOpenSRA::onExitButtonClicked()
{

}


int WorkflowAppOpenSRA::getMaxNumParallelTasks()
{
    return 1;
}


void WorkflowAppOpenSRA::setUpForApplicationRun(QString &workingDir, QString &subDir)
{

    //
    // create temporary directory in working dir
    // and copy all files needed to this directory by invoking copyFiles() on app widgets
    //

    QString tmpDirName = QString("Input");
    qDebug() << "TMP_DIR: " << tmpDirName;
    QDir workDir(workingDir);

    QString tmpDirectory = workDir.absoluteFilePath(tmpDirName);
    QDir destinationDirectory(tmpDirectory);

    if(destinationDirectory.exists()) {
        destinationDirectory.removeRecursively();
    } else
        destinationDirectory.mkpath(tmpDirectory);

    destinationDirectory.mkpath(tmpDirectory);

    // copyPath(path, tmpDirectory, false);
    theIntensityMeasureWidget->copyFiles(tmpDirectory);
    theDamageMeasureWidget->copyFiles(tmpDirectory);
    thePipelineNetworkWidget->copyFiles(tmpDirectory);
    theEDPWidget->copyFiles(tmpDirectory);

    //
    // in new templatedir dir save the UI data into dakota.json file (same result as using saveAs)
    // NOTE: we append object workingDir to this which points to template dir
    //

    QString inputFile = tmpDirectory + QDir::separator() + tr("SetupConfig.json");

    QFile file(inputFile);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        //errorMessage();
        return;
    }
    QJsonObject json;
    this->outputToJSON(json);

    json["runDir"]=tmpDirectory;
    json["WorkflowType"]="OpenSRA Simulation";

    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();

    statusMessage("SetUp Done .. Now starting application");

    QString inputDirectory = tmpDirectory + QDir::separator();

    emit setUpForApplicationRunDone(tmpDirectory, inputDirectory);
}


void WorkflowAppOpenSRA::loadFile(const QString fileName)
{

    //
    // open file
    //

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        emit errorMessage(QString("Could Not Open File: ") + fileName);
        return;
    }

    //
    // place contents of file into json object
    //

    QString val;
    val=file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = doc.object();

    // close file
    file.close();

    //
    // clear current and input from new JSON
    //

    this->clear();
    this->inputFromJSON(jsonObj);
}

