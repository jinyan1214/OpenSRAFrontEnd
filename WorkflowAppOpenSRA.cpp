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
#include "UIWidgets/IntensityMeasureWidget.h"
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
#include "LoadResultsDialog.h"
#include "Utils/PythonProgressDialog.h"

#include <QApplication>
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

    resultsDialog = nullptr;
}


WorkflowAppOpenSRA::~WorkflowAppOpenSRA()
{

}


void WorkflowAppOpenSRA::initialize(void)
{

    // Load the methods and params json file
    QString fileName = QCoreApplication::applicationDirPath() + QDir::separator() + "OpenSRABackEnd" + QDir::separator() + "MethodsAndParams.json";

    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists()){
        this->errorMessage(QString("The methods and params file does not exist! ") + fileName);
    }

    QString dirPath = fileInfo.absoluteDir().absolutePath();
    QDir::setCurrent(dirPath);

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        this->errorMessage(QString("Could Not Open File: ") + fileName);
    }

    //
    // place contents of file into json object
    //

    QString val;
    val=file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    methodsAndParamsObj = doc.object();

    // close file
    file.close();

    if(methodsAndParamsObj.isEmpty())
    {
        this->errorMessage("The methods and parameters file is empty");
        return;
    }

    // Get the human read-able names of the json objects
    std::function<void(const QJsonObject&)> recursiveObj = [&](const QJsonObject& objects){

        QJsonObject::const_iterator objIt;
        for (objIt = objects.begin(); objIt != objects.end(); ++objIt)
        {
            auto obj = objIt.value().toObject();

            auto name = obj["NameToDisplay"].toString();
            if(!name.isEmpty())
            {
                auto key = objIt.key();
                methodsParamsMap.insert(key,name);
            }

            if(!obj.isEmpty())
                recursiveObj(objIt.value().toObject());
        }
    };

    recursiveObj(methodsAndParamsObj);

    // Create the edit menu with the clear action
    QMenu *editMenu = theMainWindow->menuBar()->addMenu(tr("&Edit"));
    // Set the path to the input file
    editMenu->addAction("Clear Inputs", this, &WorkflowAppOpenSRA::clear);
    editMenu->addAction("Clear Working Directory", this, &WorkflowAppOpenSRA::clearWorkDir);

    // Load the examples
    auto pathToExamplesJson = QCoreApplication::applicationDirPath() + QDir::separator() + "OpenSRABackEnd" + QDir::separator() + "examples" + QDir::separator() + "Examples.json";

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

    // Edit menu for the clear action
    QMenu *resultsMenu = new QMenu(tr("&Results"),theMainWindow->menuBar());

    // Set the path to the input file
    resultsMenu->addAction("&Load Results", this, &WorkflowAppOpenSRA::loadResults);
    theMainWindow->menuBar()->addMenu(resultsMenu);


    // Show progress dialog
    QMenu *viewMenu = theMainWindow->menuBar()->addMenu(tr("&View"));
    viewMenu->addAction("Show Status Dialog", this, &WorkflowAppWidget::showOutputDialog);
    viewMenu->addSeparator();

    // Help menu
    theMainWindow->createHelpMenu();

    theVisualizationWidget = new VisualizationWidget(this);

    // Create the various widgets
    theGenInfoWidget = new GeneralInformationWidget(this);
    theUQWidget = new UncertaintyQuantificationWidget(this);
    thePipelineNetworkWidget = new PipelineNetworkWidget(this,theVisualizationWidget);
    theIntensityMeasureWidget = new IntensityMeasureWidget(theVisualizationWidget, this);
    theDamageMeasureWidget = new DamageMeasureWidget(this);
    theEDPWidget = new EngDemandParamWidget(this);
    theCustomVisualizationWidget = new CustomVisualizationWidget(this,theVisualizationWidget);
    theDecisionVariableWidget = new DecisionVariableWidget(this);
    theResultsWidget = new ResultsWidget(this,theVisualizationWidget);


    SimCenterWidget *theWidgets[1];

    localApp = new LocalApplication("OpenSRA.py",theMainWindow);
    theRunWidget = new RunWidget(localApp, theWidgets, 0);


    connect(this,SIGNAL(sendInfoMessage(QString)),this,SLOT(infoMessage(QString)));

    connect(localApp,SIGNAL(setupForRun(QString &,QString &)), this, SLOT(setUpForApplicationRun(QString &,QString &)));
    connect(this,SIGNAL(setUpForApplicationRunDone(QString&, QString &)), theRunWidget, SLOT(setupForRunApplicationDone(QString&, QString &)));
    connect(localApp,SIGNAL(processResults(QString,QString,QString)), this, SLOT(processResults(QString, QString, QString)));

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    this->setLayout(horizontalLayout);
    horizontalLayout->setSpacing(0);
    this->setContentsMargins(0,0,0,0);
    horizontalLayout->setMargin(0);

    // Create the component selection & add the components to it
    theComponentSelection = new SimCenterComponentSelection();
    theComponentSelection->setContentsMargins(0,0,0,0);
    horizontalLayout->addWidget(theComponentSelection);

    theComponentSelection->addComponent(QString("Visualization"), theCustomVisualizationWidget);
    theComponentSelection->addComponent(QString("General\nInformation"), theGenInfoWidget);
    theComponentSelection->addComponent(QString("Sampling\nMethod"), theUQWidget);
    theComponentSelection->addComponent(QString("Infrastructure"), thePipelineNetworkWidget);
    theComponentSelection->addComponent(QString("Intensity\nMeasure"), theIntensityMeasureWidget);
    theComponentSelection->addComponent(QString("Engineering\nDemand\nParameter"), theEDPWidget);
    theComponentSelection->addComponent(QString("Damage\nMeasure"), theDamageMeasureWidget);
    theComponentSelection->addComponent(QString("Decision\nVariable"), theDecisionVariableWidget);
    theComponentSelection->addComponent(QString("Results"), theResultsWidget);
    theComponentSelection->setWidth(120);
    theComponentSelection->setItemWidthHeight(120,70);

    theComponentSelection->displayComponent("Visualization");

//    loadFile("/Users/steve/Desktop/SimCenter/OpenSRABackEnd/examples/Ex1_OpenSHA_noEDP_noDV/Input/SetupConfig.json");
//    loadFile("/Users/steve/Desktop/SimCenter/OpenSRABackEnd/examples/Ex2_ShakeMap_noEDP_noDV/Input/SetupConfig.json");
//    loadFile("/Users/steve/Desktop/SimCenter/OpenSRABackEnd/examples/Ex3_IMCorr_RepairRatePGV/Input/SetupConfig.json");
//    loadFile("/Users/steve/Desktop/SimCenter/OpenSRABackEnd/examples/Ex4_EDPs_and_RepairRatePGD/Input/SetupConfig.json");
//    loadFile("/Users/steve/Desktop/SimCenter/OpenSRABackEnd/examples/Ex5_MultipleMethods/Input/SetupConfig.json");
    loadFile("/Users/steve/Desktop/SimCenter/OpenSRABackEnd/examples/Ex6_UserInputModelParams/Input/SetupConfig.json");

    theResultsWidget->processResults("/Users/steve/Desktop/ResToDelete/");
}


void WorkflowAppOpenSRA::loadExamples()
{
    QObject* senderObj = QObject::sender();

    if(senderObj == nullptr)
        return;

    QString pathToExample = QCoreApplication::applicationDirPath() + QDir::separator() + "OpenSRABackEnd" + QDir::separator() + "examples" + QDir::separator();

    pathToExample += QObject::sender()->property("InputFile").toString();

    if(pathToExample.isNull())
    {
        QString msg = "Error loading example "+pathToExample;
        emit sendErrorMessage(msg);
        return;
    }

    auto exampleName = senderObj->property("name").toString();
    emit sendStatusMessage("Loading example "+exampleName);

    auto description = senderObj->property("description").toString();

    if(!description.isEmpty())
        this->infoMessage(description);

    this->statusMessage("Loading Example file.  Wait until \"Done Loading\" appears before progressing.");

    auto progressDialog = this->getProgressDialog();

    progressDialog->showProgressBar();
    progressDialog->setProgressBarRange(0,0);
    progressDialog->setProgressBarValue(0);

    QApplication::processEvents();

    this->loadFile(pathToExample);
    progressDialog->hideProgressBar();

    this->statusMessage("Done loading.  Click on 'Run' button to run the analysis.");
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
    bool res = true;

    // Get each of the main widgets to output themselves
    res = theGenInfoWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    res = theUQWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    res = thePipelineNetworkWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    res = theIntensityMeasureWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    res = theDecisionVariableWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    res = theDamageMeasureWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    res = theEDPWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    return true;
}


void WorkflowAppOpenSRA::processResults(QString resultsDirectory, QString /*doesNothing2*/, QString /*doesNothing3*/)
{
    theResultsWidget->processResults(resultsDirectory);
    theRunWidget->hide();
    theComponentSelection->displayComponent("Results");
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

    // Create a new work dir
    if (!workDirectory.mkpath(workDirectoryPath)) {
        QString errorMessage = QString("Could not create Working Dir: ") + workDirectoryPath
                + QString(". Change the directory location in general information.");

        qDebug()<<errorMessage;

        return;
    }
}


bool WorkflowAppOpenSRA::inputFromJSON(QJsonObject &jsonObject)
{
    bool res = true;

    auto genJsonObj = jsonObject.value("General").toObject();
    res = theGenInfoWidget->inputFromJSON(genJsonObj);

    if(res == false)
    {
        errorMessage("Error loading .json input file at " + theGenInfoWidget->objectName() + " panel");
        return false;
    }

    auto UQJsonObj = jsonObject.value("SamplingMethod").toObject();
    res = theUQWidget->inputFromJSON(UQJsonObj);

    if(res == false)
    {
        errorMessage("Error loading .json input file at " + theUQWidget->objectName() + " panel");
        return false;
    }

    auto InfraJsonObj = jsonObject.value("Infrastructure").toObject();
    res = thePipelineNetworkWidget->inputFromJSON(InfraJsonObj);

    if(res == false)
    {
        errorMessage("Error loading .json input file at " + thePipelineNetworkWidget->objectName() + " panel");
        return false;
    }

    auto IntensityMeasObj = jsonObject.value("IntensityMeasure").toObject();
    res = theIntensityMeasureWidget->inputFromJSON(IntensityMeasObj);

    if(res == false)
    {
        errorMessage("Error loading .json input file at " + theIntensityMeasureWidget->objectName() + " panel");
        return false;
    }

    auto EDPObj = jsonObject.value("EngineeringDemandParameter").toObject();
    res = theEDPWidget->inputFromJSON(EDPObj);

    if(res == false)
    {
        errorMessage("Error loading .json input file at " + theEDPWidget->objectName() + " panel");
        return false;
    }

    auto DamageMeasureObj = jsonObject.value("DamageMeasure").toObject();
    res = theDamageMeasureWidget->inputFromJSON(DamageMeasureObj);

    if(res == false)
    {
        errorMessage("Error loading .json input file at " + theDamageMeasureWidget->objectName() + " panel");
        return false;
    }

    auto DecisionVarObj = jsonObject.value("DecisionVariable").toObject();
    res = theDecisionVariableWidget->inputFromJSON(DecisionVarObj);

    if(res == false)
    {
        errorMessage("Error loading .json input file at " + theDecisionVariableWidget->objectName() + " panel");
        return false;
    }

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
    auto res = this->outputToJSON(json);
    if(!res)
    {
        errorMessage("Error setting up the analysis.  Analysis did not run.");
        return;
    }

    json["runDir"]=tmpDirectory;
    json["WorkflowType"]="OpenSRA Simulation";

    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();

    statusMessage("Set up Done.  Now starting the analysis.");

    QString inputDirectory = tmpDirectory + QDir::separator();

    emit setUpForApplicationRunDone(tmpDirectory, inputDirectory);
}


void WorkflowAppOpenSRA::loadResults(void)
{
    if(resultsDialog == nullptr)
        resultsDialog= new LoadResultsDialog(this);

    resultsDialog->show();
}



int WorkflowAppOpenSRA::loadFile(const QString fileName)
{

    //
    // open file
    //

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        this->errorMessage(QString("Could Not Open File: ") + fileName);
        return -1;
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
    this->clearWorkDir();

    auto res = this->inputFromJSON(jsonObj);

    if(res == false)
        return -1;

    return 0;
}


void WorkflowAppOpenSRA::statusMessage(QString message)
{
    progressDialog->appendText(message);
}


void WorkflowAppOpenSRA::infoMessage(QString message)
{
    progressDialog->appendInfoMessage(message);
}


void WorkflowAppOpenSRA::errorMessage(QString message)
{
    progressDialog->appendErrorMessage(message);
}


void WorkflowAppOpenSRA::fatalMessage(QString message)
{
    progressDialog->appendErrorMessage(message);
}

QJsonObject WorkflowAppOpenSRA::getMethodsAndParamsObj() const
{
    return methodsAndParamsObj;
}

QMap<QString, QString> WorkflowAppOpenSRA::getMethodsAndParamsMap() const
{
    return methodsParamsMap;
}
