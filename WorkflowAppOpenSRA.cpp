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
// Latest revision: 09.29.2020

#include "WorkflowAppOpenSRA.h"
#include "SimCenterComponentSelection.h"
#include "GeneralInformationWidget.h"
#include "SourceCharacterizationWidget.h"
#include "DamageMeasureWidget.h"
#include "DecisionVariableWidget.h"
#include "PipelineNetworkWidget.h"
#include "LocalApplication.h"
#include "RunWidget.h"
#include "UIWidgets/ResultsWidget.h"
#include "IntensityMeasureWidget.h"
#include "RunLocalWidget.h"
#include "EngDemandParamWidget.h"
#include "CustomizedItemModel.h"
#include "VisualizationWidget.h"

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QUuid>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QHostInfo>
#include <QPushButton>
#include <QScrollArea>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QLabel>
#include <QDebug>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QProcess>
#include <QCoreApplication>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QHostInfo>
#include <QTreeView>
#include <QUuid>

// static pointer for global procedure set in constructor
static WorkflowAppOpenSRA *theApp = nullptr;

WorkflowAppOpenSRA::WorkflowAppOpenSRA(QWidget *parent) : WorkflowAppWidget(parent)
{
    // set static pointer for global procedure
    theApp = this;

    theVisualizationWidget = new VisualizationWidget(this);

    // create the various widgets
    theGenInfoWidget = GeneralInformationWidget::getInstance();
    theSourceCharacterizationWidget = new SourceCharacterizationWidget();
    theIntensityMeasureWidget = new IntensityMeasureWidget(theGenInfoWidget);
    theDamageMeasureWidget = new DamageMeasureWidget();
    thePipelineNetworkWidget = new PipelineNetworkWidget(this,theVisualizationWidget);
    theEDPWidget = new EngDemandParamWidget();
    theResultsWidget = new ResultsWidget(this,theVisualizationWidget);
    theDecisionVariableWidget = new DecisionVariableWidget();

    SimCenterWidget *theWidgets[1];
    theRunWidget = new RunWidget(localApp, theWidgets, 0);

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    this->setLayout(horizontalLayout);
    this->setContentsMargins(0,5,0,5);
    horizontalLayout->setMargin(0);

    // Create the component selection & add the components to it

    theComponentSelection = new SimCenterComponentSelection();
    horizontalLayout->addWidget(theComponentSelection);

    theComponentSelection->addComponent(QString("General\nInformation"), theGenInfoWidget);
    theComponentSelection->addComponent(QString("Infrastructure"), thePipelineNetworkWidget);
    theComponentSelection->addComponent(QString("Source\nCharacterization"), theSourceCharacterizationWidget);
    theComponentSelection->addComponent(QString("Intensity\nMeasure"), theIntensityMeasureWidget);
    theComponentSelection->addComponent(QString("Engineering\nDemand Parameter"), theEDPWidget);
    theComponentSelection->addComponent(QString("Damage\nMeasure"), theDamageMeasureWidget);
    theComponentSelection->addComponent(QString("Decision\nVariable"), theDecisionVariableWidget);
    theComponentSelection->addComponent(QString("Visualization"), theResultsWidget);
    theComponentSelection->setMaxWidth(160);

    theComponentSelection->displayComponent("General\nInformation");

    // access a web page which will increment the usage count for this tool
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("http://opensees.berkeley.edu/OpenSees/developer/eeuq/use.php")));
}

WorkflowAppOpenSRA::~WorkflowAppOpenSRA()
{

}


void WorkflowAppOpenSRA::replyFinished(QNetworkReply *pReply)
{
    return;
}

GeneralInformationWidget *WorkflowAppOpenSRA::getTheGI() const
{
    return theGenInfoWidget;
}

void WorkflowAppOpenSRA::setTheGI(GeneralInformationWidget *value)
{
    theGenInfoWidget = value;
}


bool WorkflowAppOpenSRA::outputToJSON(QJsonObject &jsonObjectTop)
{

    return true;
}


void WorkflowAppOpenSRA::processResults(QString dakotaOut, QString dakotaTab, QString inputFile)
{


}


void WorkflowAppOpenSRA::clear(void)
{
    theGenInfoWidget->clear();
}


bool WorkflowAppOpenSRA::inputFromJSON(QJsonObject &jsonObject)
{
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
    errorMessage("");

    //
    // create temporary directory in working dir
    // and copy all files needed to this directory by invoking copyFiles() on app widgets
    //

    // designsafe will need a unique name
    /* *********************************************
    will let ParallelApplication rename dir
    QUuid uniqueName = QUuid::createUuid();
    QString strUnique = uniqueName.toString();
    strUnique = strUnique.mid(1,36);
    QString tmpDirName = QString("tmp.SimCenter") + strUnique;
    *********************************************** */

    QString tmpDirName = QString("tmp.SimCenter");
    qDebug() << "TMP_DIR: " << tmpDirName;
    QDir workDir(workingDir);

    QString tmpDirectory = workDir.absoluteFilePath(tmpDirName);
    QDir destinationDirectory(tmpDirectory);

    if(destinationDirectory.exists()) {
        destinationDirectory.removeRecursively();
    } else
        destinationDirectory.mkpath(tmpDirectory);

    QString templateDirectory  = destinationDirectory.absoluteFilePath(subDir);
    destinationDirectory.mkpath(templateDirectory);

    // copyPath(path, tmpDirectory, false);
    theSourceCharacterizationWidget->copyFiles(templateDirectory);
    theIntensityMeasureWidget->copyFiles(templateDirectory);
    theDamageMeasureWidget->copyFiles(templateDirectory);
    thePipelineNetworkWidget->copyFiles(templateDirectory);
    theEDPWidget->copyFiles(templateDirectory);

    //
    // in new templatedir dir save the UI data into dakota.json file (same result as using saveAs)
    // NOTE: we append object workingDir to this which points to template dir
    //

    QString inputFile = templateDirectory + QDir::separator() + tr("dakota.json");

    QFile file(inputFile);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        //errorMessage();
        return;
    }
    QJsonObject json;
    this->outputToJSON(json);

    json["runDir"]=tmpDirectory;
    json["WorkflowType"]="Building Simulation";

    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();

    statusMessage("SetUp Done .. Now starting application");

    emit setUpForApplicationRunDone(tmpDirectory, inputFile);
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

