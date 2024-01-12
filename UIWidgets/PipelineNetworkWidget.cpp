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

#include "StateWidePipelineWidget.h"
#include "BayAreaPipelineWidget.h"
#include "LosAngelesPipelineWidget.h"
#include "LineAssetInputWidget.h"
#include "PointAssetInputWidget.h"
#include "CSVWellsCaprocksInputWidget.h"
#include "GISWellsCaprocksInputWidget.h"
#include "CSVAboveGroundGasComponentInputWidget.h"
#include "GISAboveGroundGasComponentInputWidget.h"
#include "NDAStateWidePipelineWidget.h"
#include "NDABayAreaPipelineWidget.h"

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

PipelineNetworkWidget::PipelineNetworkWidget(VisualizationWidget* visWidget, QWidget *parent)
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
    label->setText(QString("Infrastructure"));
    label->setMinimumWidth(150);
    theHeaderLayout->addWidget(label);
    theHeaderLayout->addStretch(1);

    theMainLayout->insertLayout(0,theHeaderLayout);

    // Gas pipelines
    gasPipelineWidget = new SimCenterAppSelection(QString("Pipeline Network"), QString("Assets"), QString("NaturalGasPipelines"), QString(), this);

    // CSV pipelines
    csvBelowGroundInputWidget = new LineAssetInputWidget(this, theVisualizationWidget, "Gas Pipelines","Gas Network");
    csvBelowGroundInputWidget->setGroupBoxText("Enter Component Locations and Characteristics");

    csvBelowGroundInputWidget->setLabel1("Load segment information from a CSV File and pick the columns with the start, mid, and end points of segments");
    csvBelowGroundInputWidget->setLabel3("Locations and Characteristics of the Components to the Infrastructure");

    // Statewide
    statewideBelowGroundInputWidget = new StateWidePipelineWidget(this, theVisualizationWidget, "Pipeline Network","Pipeline Network");
    bayareaBelowGroundInputWidget = new BayAreaPipelineWidget(this, theVisualizationWidget, "Pipeline Network","Pipeline Network");
    losangelesBelowGroundInputWidget = new LosAngelesPipelineWidget(this, theVisualizationWidget, "Pipeline Network","Pipeline Network");
    //    statewideBelowGroundInputWidget->setGroupBoxText("Enter Component Locations and Characteristics");
    ndaStatewideBelowGroundInputWidget = new NDAStateWidePipelineWidget(this, theVisualizationWidget, "Pipeline Network","Pipeline Network");
    ndaBayareaBelowGroundInputWidget = new NDABayAreaPipelineWidget(this, theVisualizationWidget, "Pipeline Network","Pipeline Network");

    //    statewideBelowGroundInputWidget->setLabel1("Load information from CSV File (headers in CSV file must match those shown in the table below)");
    statewideBelowGroundInputWidget->setLabel3("Locations and Characteristics of the Components to the Infrastructure");
    bayareaBelowGroundInputWidget->setLabel3("Locations and Characteristics of the Components to the Infrastructure");
    losangelesBelowGroundInputWidget->setLabel3("Locations and Characteristics of the Components to the Infrastructure");
    ndaStatewideBelowGroundInputWidget->setLabel3("Locations and Characteristics of the Components to the Infrastructure");
    ndaBayareaBelowGroundInputWidget->setLabel3("Locations and Characteristics of the Components to the Infrastructure");


    // GIS pipelines
    GISGasNetworkInputWidget *gisGasNetworkInventory = new GISGasNetworkInputWidget(this, theVisualizationWidget);


    // Add the pipeline widgets to the selection widget
    gasPipelineWidget->addComponent(QString("CSV to Pipeline"), QString("CSV_to_PIPELINE"), csvBelowGroundInputWidget);
    gasPipelineWidget->addComponent(QString("GIS to Pipeline"), QString("GIS_to_PIPELINE"), gisGasNetworkInventory);
    gasPipelineWidget->addComponent(QString("Use Prepackaged State Pipeline Network"), QString("STATE_PIPELINE"), statewideBelowGroundInputWidget);
    gasPipelineWidget->addComponent(QString("Use Prepackaged Bay Area Pipeline Network"), QString("BAY_AREA_PIPELINE"), bayareaBelowGroundInputWidget);
    gasPipelineWidget->addComponent(QString("Use Prepackaged Los Angeles Pipeline Network"), QString("LOS_ANGELES_PIPELINE"), losangelesBelowGroundInputWidget);
    gasPipelineWidget->addComponent(QString("Use NDA Pipeline Network - Statewide"), QString("NDA_PIPELINE_STATE"), ndaStatewideBelowGroundInputWidget);
    gasPipelineWidget->addComponent(QString("Use NDA Pipeline Network - Cropped to Bay Area"), QString("NDA_PIPELINE_BAY_AREA"), ndaBayareaBelowGroundInputWidget);


    // Above ground widget
    theAboveGroundInfWidget = new SimCenterAppSelection(QString("Above Ground Gas Infrastructure"), QString("Assets"), QString("Above ground infrastructure"), QString(), this);

    // auto csvAboveGroundInventory = new PointAssetInputWidget(this, theVisualizationWidget, "Above Ground Gas Infrastructures","Above ground infrastructure");
    auto csvAboveGroundInventory = new CSVAboveGroundGasComponentInputWidget(this, theVisualizationWidget, "Above Ground Gas Infrastructures","Above ground infrastructure");
    theAboveGroundInfWidget->addComponent(QString("CSV to Above Ground Infrastructure"), QString("CSV_to_ABOVE_GROUND"), csvAboveGroundInventory);
    auto gisAboveGroundInventory = new GISAboveGroundGasComponentInputWidget(this, theVisualizationWidget);
    theAboveGroundInfWidget->addComponent(QString("GIS to Above Ground Infrastructure"), QString("GIS_to_ABOVE_GROUND"), gisAboveGroundInventory);

    // Wells and caprocks
    theWellsCaprocksWidget = new SimCenterAppSelection(QString("Wells and Caprocks"), QString("Assets"), QString("Wells and Caprocks"), QString(), this);

    auto csvWellsCaprocksWidgetInventory = new CSVWellsCaprocksInputWidget(this, theVisualizationWidget, "Wells and Caprocks","Wells and Caprocks");
    theWellsCaprocksWidget->addComponent(QString("CSV to Wells and Caprocks"), QString("CSV_to_WELLS_CAPROCKS"), csvWellsCaprocksWidgetInventory);
    auto gisWellsCaprocksWidgetInventory = new GISWellsCaprocksInputWidget(this, theVisualizationWidget);
    theWellsCaprocksWidget->addComponent(QString("GIS to Wells and Caprocks"), QString("GIS_to_WELLS_CAPROCKS"), gisWellsCaprocksWidgetInventory);


    this->addComponent("Pipelines", gasPipelineWidget);
    this->addComponent("Wells and Caprocks", theWellsCaprocksWidget);
    this->addComponent("Above Ground\nGas Infrastructure", theAboveGroundInfWidget);

    auto testInputWidget = new LineAssetInputWidget(this, theVisualizationWidget, "Test","Test");
    this->addComponent("Future Infrastructure\n(Not Functional)", testInputWidget);

    //    vectorOfComponents.append(gasPipelineWidget);
    //    vectorOfComponents.append(theWellsCaprocksWidget);
    //    vectorOfComponents.append(theAboveGroundInfWidget);
    //    vectorOfComponents.append(testInputWidget);

    this->show("Pipelines");

    auto colDelegate = WorkflowAppOpenSRA::getInstance()->getTheRandomVariableWidget()->getColDataComboDelegate();

    assert(colDelegate);

    connect(csvBelowGroundInputWidget, &AssetInputWidget::headingValuesChanged, colDelegate, &MixedDelegate::updateComboBoxValues);
    connect(gisGasNetworkInventory, &GISGasNetworkInputWidget::headingValuesChanged, colDelegate, &MixedDelegate::updateComboBoxValues);
    connect(statewideBelowGroundInputWidget, &StateWidePipelineWidget::headingValuesChanged, colDelegate, &MixedDelegate::updateComboBoxValues);
    connect(bayareaBelowGroundInputWidget, &BayAreaPipelineWidget::headingValuesChanged, colDelegate, &MixedDelegate::updateComboBoxValues);
    connect(losangelesBelowGroundInputWidget, &LosAngelesPipelineWidget::headingValuesChanged, colDelegate, &MixedDelegate::updateComboBoxValues);
    connect(ndaStatewideBelowGroundInputWidget, &NDAStateWidePipelineWidget::headingValuesChanged, colDelegate, &MixedDelegate::updateComboBoxValues);
    connect(ndaBayareaBelowGroundInputWidget, &NDABayAreaPipelineWidget::headingValuesChanged, colDelegate, &MixedDelegate::updateComboBoxValues);

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
    else if(typeOfInf.compare("Above Ground\nGas Infrastructure") == 0)
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
        osraType = "Above Ground\nGas Infrastructure";
    else if(typeOfInf.compare("wells_caprocks") == 0)
        osraType = "Wells and Caprocks";

    auto res = this->show(osraType);

    if(res == false)
    {
        this->errorMessage("OpenSRA does not support the infrastructure type "+ typeOfInf);
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
        else if(typeOfFile.compare("State_Network") == 0)
            app = "Use Prepackaged State Pipeline Network";
        else if(typeOfFile.compare("Region_Network") == 0)
        {
            if (fileName.contains("Bay_Area"))
                app = "Use Prepackaged Bay Area Pipeline Network";
            else if (fileName.contains("Los_Angeles"))
                app = "Use Prepackaged Los Angeles Pipeline Network";
        }
        else if (typeOfFile.compare("NDA_Network") == 0)
        {
            if (fileName.contains("full"))
                app = "Use NDA Pipeline Network - Statewide";
            else if (fileName.contains("bayarea"))
                app = "Use NDA Pipeline Network - Bay Area";
        }

    }
    else if(typeOfInf.compare("above_ground") == 0)
    {
        if(typeOfFile.compare("Shapefile") == 0)
            app = "GIS to Above Ground Infrastructure";
        else if(typeOfFile.compare("CSV") == 0)
            app = "CSV to Above Ground Infrastructure";
    }
    else if(typeOfInf.compare("wells_caprocks") == 0)
    {
        if(typeOfFile.compare("Shapefile") == 0)
            app = "GIS to Wells and Caprocks";
        else if(typeOfFile.compare("CSV") == 0)
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

    /*
    - when loading setup from an analysis that was performed previously, the program loads in the assets that are already in the input folder
      this leads to error when trying to write files over themselves when the files are being occupied by the program
    -> this is a problem embedded in SimCenter commons. For OpenSRA, turn off copy of files for component and have program always refer to original source
    */

    return true;

//    auto theCurrInputWidget = this->getCurrentComponent();

//    if(theCurrInputWidget == nullptr)
//        return false;

//    return theCurrInputWidget->copyFiles(destDir);
}


void PipelineNetworkWidget::clear(void)
{
    gasPipelineWidget->clear();
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

