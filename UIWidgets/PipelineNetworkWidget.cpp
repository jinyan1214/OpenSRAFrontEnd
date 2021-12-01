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

#include "PipelineNetworkWidget.h"
#include "sectiontitle.h"
#include "SimCenterComponentSelection.h"
#include "ComponentTableView.h"
#include "QGISGasPipelineInputWidget.h"
#include "VisualizationWidget.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QTableWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>

PipelineNetworkWidget::PipelineNetworkWidget(QWidget *parent, VisualizationWidget* visWidget)
    : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    this->setContentsMargins(0,0,0,0);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(5,0,0,0);
    mainLayout->setSpacing(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    theHeaderLayout->setContentsMargins(0,0,0,0);
    theHeaderLayout->setMargin(0);
    theHeaderLayout->setSpacing(0);
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Infrastructure (Limited to a maximum of 1,000 sites)"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);

    theHeaderLayout->addStretch(1);
    mainLayout->addLayout(theHeaderLayout);

    theComponentInputWidget = new QGISGasPipelineInputWidget(this, theVisualizationWidget, "Gas Pipelines","Gas Network");
    theComponentInputWidget->setGroupBoxText("Enter Component Locations and Characteristics");

    theComponentInputWidget->setLabel1("Load information from CSV File (headers in CSV file must match those shown in the table below)");
    theComponentInputWidget->setLabel3("Locations and Characteristics of the Components to the Infrastructure");

    mainLayout->addWidget(theComponentInputWidget);

    mainLayout->addStretch();

}


PipelineNetworkWidget::~PipelineNetworkWidget()
{

}


bool PipelineNetworkWidget::outputToJSON(QJsonObject &jsonObject)
{

    auto numComponents = theComponentInputWidget->getTableWidget()->rowCount();

    if(numComponents == 0)
    {
        errorMessage("No pipelines loaded");
        return false;
    }

    QJsonObject infrastructureObj;

    auto siteDataPath = theComponentInputWidget->getPathToComponentFile();

    QFileInfo file(siteDataPath);

    infrastructureObj.insert("SiteDataFile",file.absoluteFilePath());

    QString filterString = theComponentInputWidget->getFilterString();
    if(filterString.isEmpty())
    {
        statusMessage("Selecting all components for analysis");

        theComponentInputWidget->selectAllComponents();

        filterString = theComponentInputWidget->getFilterString();
    }

    infrastructureObj.insert("filter",filterString);

    QJsonObject locationParams;
    theComponentInputWidget->outputToJSON(locationParams);

    QJsonObject siteParamsObj;

    QJsonObject::const_iterator locationObj;
    for (locationObj = locationParams.begin(); locationObj != locationParams.end(); ++locationObj)
    {
        auto locObj = locationObj.value().toObject();

        QJsonObject::const_iterator locationParamObj;
        for (locationParamObj = locObj.begin(); locationParamObj != locObj.end(); ++locationParamObj)
        {
            auto key = locationParamObj.key();

            auto val = locationParamObj.value().toObject().value(key).toString();

            if(val.compare("N/A") == 0)
                siteParamsObj[key] = QJsonValue::Null;
            else
                siteParamsObj[key] = locationParamObj.value().toObject().value(key).toString();
        }
    }

    infrastructureObj["SiteLocationParams"] = siteParamsObj;

    jsonObject.insert("Infrastructure",infrastructureObj);

    return true;
}


bool PipelineNetworkWidget::inputFromJSON(QJsonObject &jsonObject)
{
    auto fileName = jsonObject["SiteDataFile"].toString();

    if(fileName.isEmpty())
    {
        errorMessage("Cannot find the pipeline data 'SiteDataFile' in the .json input file");
        return false;
    }

    if(!theComponentInputWidget->loadFileFromPath(fileName))
    {
        errorMessage("Failed to load the 'SiteDataFile': "+fileName);
        return false;
    }

    auto filter = jsonObject["filter"].toString();

    if(!filter.isEmpty())
        theComponentInputWidget->setFilterString(filter);

    return theComponentInputWidget->inputFromJSON(jsonObject);
}


bool PipelineNetworkWidget::copyFiles(QString &destDir)
{    
    theComponentInputWidget->copyFiles(destDir);

    return false;
}


void PipelineNetworkWidget::clear(void)
{
    theComponentInputWidget->clear();
}
