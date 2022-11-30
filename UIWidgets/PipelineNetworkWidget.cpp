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
#include "RandomVariablesWidget.h"
#include "WorkflowAppOpenSRA.h"
#include "MixedDelegate.h"
#include "SimCenterAppSelection.h"
#include "GISGasNetworkInputWidget.h"

#include "LineAssetInputWidget.h"
#include "PointAssetInputWidget.h"
#include "QGISWellsCaprocksInputWidget.h"
#include "QGISAboveGroundGasNetworkInputWidget.h"

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
    : MultiComponentR2D("NaturalGasNetwork",parent), theVisualizationWidget(visWidget)
{
    this->setContentsMargins(0,0,0,0);

    theMainLayout->setMargin(0);
    theMainLayout->setContentsMargins(5,0,0,0);
    theMainLayout->setSpacing(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    theHeaderLayout->setContentsMargins(0,0,0,0);
    theHeaderLayout->setMargin(0);
    theHeaderLayout->setSpacing(0);
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Infrastructure (Limited to a maximum of 1,000 sites)"));
    label->setMinimumWidth(150);
    theHeaderLayout->addWidget(label);
    theHeaderLayout->addStretch(1);

    theMainLayout->insertLayout(0,theHeaderLayout);

    // Gas pipelines
    gasPipelineWidget = new SimCenterAppSelection(QString("Regional Gas Pipelines"), QString("Assets"), QString("NaturalGasPipelines"), QString(), this);

    csvBelowGroundInputWidget = new LineAssetInputWidget(this, theVisualizationWidget, "Gas Pipelines","Gas Network");
    csvBelowGroundInputWidget->setGroupBoxText("Enter Component Locations and Characteristics");

    csvBelowGroundInputWidget->setLabel1("Load information from CSV File (headers in CSV file must match those shown in the table below)");
    csvBelowGroundInputWidget->setLabel3("Locations and Characteristics of the Components to the Infrastructure");

    GISGasNetworkInputWidget *gisGasNetworkInventory = new GISGasNetworkInputWidget(this, theVisualizationWidget);

    gasPipelineWidget->addComponent(QString("CSV to Pipeline"), QString("CSV_to_PIPELINE"), csvBelowGroundInputWidget);
    gasPipelineWidget->addComponent(QString("GIS to Pipeline"), QString("GIS_to_PIPELINE"), gisGasNetworkInventory);


    // Above ground widget
    theAboveGroundInfWidget = new SimCenterAppSelection(QString("Above Ground Gas Infrastructure"), QString("Assets"), QString("Above ground infrastructure"), QString(), this);

    auto csvAboveGroundInventory = new PointAssetInputWidget(this, theVisualizationWidget, "Above Ground Gas Infrastructures","Above ground infrastructure");
    theAboveGroundInfWidget->addComponent(QString("CSV to Above Ground Infrastructure"), QString("CSV_to_ABOVE_GROUND"), csvAboveGroundInventory);


    // Wells and caprocks
    theWellsCaprocksWidget = new SimCenterAppSelection(QString("Wells and Caprocks"), QString("Assets"), QString("Wells and Caprocks"), QString(), this);

    auto csvWellsCaprocksWidgetInventory = new QGISWellsCaprocksInputWidget(this, theVisualizationWidget, "Wells and Caprocks","Wells and Caprocks");

    theWellsCaprocksWidget->addComponent(QString("CSV to Wells and Caprocks"), QString("CSV_to_WELLS_CAPROCKS"), csvWellsCaprocksWidgetInventory);

    this->addComponent("Pipelines", gasPipelineWidget);
    this->addComponent("Wells and Caprocks", theWellsCaprocksWidget);
    this->addComponent("Above Ground \nGas Infrastructure", theAboveGroundInfWidget);

    auto testInputWidget = new LineAssetInputWidget(this, theVisualizationWidget, "Test","Test");
    this->addComponent("Future Infrastructure", testInputWidget);

    //    vectorOfComponents.append(gasPipelineWidget);
    //    vectorOfComponents.append(theWellsCaprocksWidget);
    //    vectorOfComponents.append(theAboveGroundInfWidget);
    //    vectorOfComponents.append(testInputWidget);

    this->show("Pipelines");

    auto colDelegate = WorkflowAppOpenSRA::getInstance()->getTheRandomVariableWidget()->getColDataComboDelegate();

    assert(colDelegate);

    connect(csvBelowGroundInputWidget, &AssetInputWidget::headingValuesChanged, colDelegate, &MixedDelegate::updateComboBoxValues);
    connect(gisGasNetworkInventory, &GISGasNetworkInputWidget::headingValuesChanged, colDelegate, &MixedDelegate::updateComboBoxValues);

    connect(csvWellsCaprocksWidgetInventory, &AssetInputWidget::headingValuesChanged, colDelegate, &MixedDelegate::updateComboBoxValues);
    connect(csvAboveGroundInventory, &AssetInputWidget::headingValuesChanged, colDelegate, &MixedDelegate::updateComboBoxValues);

    // Test to remove start
    //    theAssetInputWidget->loadFileFromPath("/Users/steve/Downloads/10000_random_sites_in_ca.csv");
    //    theAssetInputWidget->selectAllComponents();

    //    theWellsCaprocksWidget->loadFileFromPath("/Users/steve/Desktop/ExWellCaprock.csv");

    // Test to remove end

}


PipelineNetworkWidget::~PipelineNetworkWidget()
{

}


bool PipelineNetworkWidget::outputToJSON(QJsonObject &jsonObject)
{

    auto currCompIndex = this->getCurrentIndex();

    if (currCompIndex < 0)
        return false;

    auto theCurrInputWidget = dynamic_cast<SimCenterAppSelection*>(this->getCurrentComponent());

    if(theCurrInputWidget == nullptr)
        return false;

    auto typeOfInf = theCurrInputWidget->property("ComponentText").toString();

    if(typeOfInf.isEmpty())
    {
        this->errorMessage("Error getting the type of infrastructure");
        return false;
    }

    QString infraType;

    if(typeOfInf.compare("Pipelines") == 0)
        infraType = "below_ground";
    else if(typeOfInf.compare("Above Ground \nGas Infrastructure") == 0)
        infraType = "above_ground";
    else if(typeOfInf.compare("Wells and Caprocks") == 0)
        infraType = "wells_caprocks";

    QJsonObject infrastructureObj;

    infrastructureObj.insert("InfrastructureType",infraType);

    QJsonObject compObj;

    theCurrInputWidget->outputToJSON(compObj);

    auto assetObj = compObj.value("Assets").toObject();

    foreach (auto&& key, assetObj.keys())
    {
        auto val = assetObj.value(key);

        infrastructureObj.insert(key,val);
    }

    jsonObject.insert("Infrastructure",infrastructureObj);


    return true;
}


bool PipelineNetworkWidget::inputFromJSON(QJsonObject &jsonObject)
{

    auto typeOfInf = jsonObject["InfrastructureType"].toString();

    QString osraType;

    if(typeOfInf.compare("below_ground") == 0)
        osraType = "Pipelines";
    else if(typeOfInf.compare("above_ground") == 0)
        osraType = "Above Ground \nGas Infrastructure";
    else if(typeOfInf.compare("wells_caprocks") == 0)
        osraType = "Wells and Caprocks";

    auto res = this->show(osraType);

    if(res == false)
    {
        this->errorMessage("OpenSRA does not support the infrastructrue type "+ typeOfInf);
        return false;

    }

    auto fileName = jsonObject["SiteDataFile"].toString();

    if(fileName.isEmpty())
    {
        errorMessage("Cannot find the pipeline data 'SiteDataFile' in the .json input file");
        return false;
    }

    if(!jsonObject.contains("DataType"))
    {
        errorMessage("The infrastructure .json input file does not contain the required field 'DataType'");
        return false;
    }

    auto typeOfFile = jsonObject["DataType"].toString();

    // Get the widget based on the app type
    auto theCurrInputWidget = this->getComponent(osraType);

    if(theCurrInputWidget == nullptr)
    {
        errorMessage("Error getting the input widget of the type"+osraType);
        return false;
    }


    auto appSelWidget = dynamic_cast<SimCenterAppSelection*>(theCurrInputWidget);

    if(appSelWidget == nullptr)
    {
        errorMessage("Error casting the input widget to SimCenterAppSelection contact dev team");
        return false;
    }


    QString app;

    if(typeOfInf.compare("below_ground") == 0)
    {
        if(typeOfFile.compare("Shapefile") == 0)
            app = "GIS to Pipeline";
        else if(typeOfFile.compare("CSV") == 0)
            app = "CSV to Pipeline";
    }
    else if(typeOfInf.compare("above_ground") == 0)
    {
        if(typeOfFile.compare("CSV") == 0)
            app = "CSV to Above Ground Infrastructure";
    }
    else if(typeOfInf.compare("wells_caprocks") == 0)
    {
        if(typeOfFile.compare("CSV") == 0)
            app = "CSV to Wells and Caprocks";
    }


    if(app.isEmpty())
    {
        errorMessage("The 'DataType' "+typeOfFile+ " is not supported");
        return false;
    }


    auto compWidget = appSelWidget->getComponent(app);

    if(compWidget == nullptr)
    {
        errorMessage("Error getting the application type "+app+" from the selected widget, contact the developers");
        return false;
    }


    if(!appSelWidget->selectComponent(app))
    {
        errorMessage("Error selecting the application type "+app+" from the widget "+ appSelWidget->objectName());
        return false;
    }


    return compWidget->inputFromJSON(jsonObject);
}


bool PipelineNetworkWidget::copyFiles(QString &destDir)
{    

    auto theCurrInputWidget = this->getCurrentComponent();

    if(theCurrInputWidget == nullptr)
        return false;

    return theCurrInputWidget->copyFiles(destDir);
}


void PipelineNetworkWidget::clear(void)
{
    csvBelowGroundInputWidget->clear();
    theWellsCaprocksWidget->clear();
    theAboveGroundInfWidget->clear();
}


//void PipelineNetworkWidget::handleComponentChanged(QString compName)
//{
//    auto currCompIndex = this->getIndexOfComponent(compName);

//    if (currCompIndex < 0)
//        emit componentChangedSignal(nullptr);

//    auto theCurrInputWidget = vectorOfComponents.at(currCompIndex);

//    emit componentChangedSignal(theCurrInputWidget);
//}


LineAssetInputWidget *PipelineNetworkWidget::getTheBelowGroundInputWidget() const
{
    return csvBelowGroundInputWidget;
}

