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

#include "WorkflowAppOpenSRA.h"
#include "PipelineNetworkWidget.h"
#include "QGISVisualizationWidget.h"
#include "LineAssetInputWidget.h"
#include "OpenSRAPreProcessor.h"
#include "ComponentDatabaseManager.h"
#include "CSVReaderWriter.h"

#include <qgsvectorlayer.h>
#include <qgsgeometryengine.h>
#include <qgsfeature.h>
#include <qgsrasterlayer.h>
#include <qgsmapcanvas.h>
#include <qgsfillsymbol.h>
#include <qgslinesymbol.h>

#include <QDir>
#include <QVariant>
#include <QPushButton>
#include <QMessageBox>

// Test to remove
#include <chrono>
using namespace std::chrono;

OpenSRAPreProcessor::OpenSRAPreProcessor(QString path, QWidget *parent) : SimCenterWidget(parent), pathToBackend(path)
{
    // Get the workflow app
    auto workFlowApp = WorkflowAppOpenSRA::getInstance();

    assert(workFlowApp);

    // Get the visualization widget
    theVisualizationWidget = workFlowApp->getVisualizationWidget();
}


int OpenSRAPreProcessor::loadPreprocessingResults(const QString path)
{
    this->statusMessage("Loading preprocessing results: "+path);

    auto processedInputPath = path + QDir::separator() + "Processed_Input";


    QDir dirWork(processedInputPath);
    if (!dirWork.exists())
    {
        QString errMsg = "Error, the preprocessing directory: " + processedInputPath + " does not exist" ;
        this->errorMessage(errMsg);
        return -1;
    }

    // Load the workflow json
    auto workFlowJson = processedInputPath + QDir::separator() + "workflow.json";

    QFile inFile(workFlowJson);
    inFile.open(QIODevice::ReadOnly|QIODevice::Text);

    QByteArray data = inFile.readAll();
    inFile.close();

    QJsonParseError errorPtr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &errorPtr);
    if (doc.isNull()) {
        this->errorMessage("Parsing to json file "+workFlowJson+" failed");
        return -1;
    }


    QString jsonString = doc.toJson(QJsonDocument::Indented);

//    this->statusMessage("Workflow :");
//    this->statusMessage(jsonString);

    // Now load the fault traces
//    auto faultPath = path + QDir::separator() + "IM" + QDir::separator() + "RUPTURE_METADATA.csv";

//    if(!QFile::exists(faultPath))
//    {
//        this->errorMessage("Error, the file containing the fault traces: "+faultPath+" does not exist");
//        return -1;
//    }

//    auto res = this->importFaultTraces(faultPath);

//    if(res != 0)
//    {
//        this->errorMessage("Error importing the fault traces");
//        return -1;
//    }

    QString msgText("Preprocess step complete. Click the \"PERFORM ANALYSIS\" button to perform the analysis.");
    this->infoMessage(msgText);

    QString msgText2("~~~~~~~~~~\n");
    this->statusMessage(msgText2);

    QMessageBox msgBox;
    msgBox.setText(msgText);
    msgBox.exec();

    return 0;
}


int OpenSRAPreProcessor::importFaultTraces(const QString& pathToFile)
{

    if(pathToFile.isEmpty())
        return 0;

    CSVReaderWriter csvTool;

    QString errMsg;

    auto traces = csvTool.parseCSVFile(pathToFile, errMsg);
    if(!errMsg.isEmpty())
    {
        this->errorMessage(errMsg);
        return -1;
    }

    if(traces.size() < 2)
    {
        this->statusMessage("No fault traces available.");
        return 0;
    }

    QgsFields featFields;
    featFields.append(QgsField("AssetType", QVariant::String));
    featFields.append(QgsField("TabName", QVariant::String));
    featFields.append(QgsField("SourceIndex", QVariant::String));

    // Create the pipelines layer
    auto mainLayer = theVisualizationWidget->addVectorLayer("linestring","Rupture Faults");

    if(mainLayer == nullptr)
    {
        this->errorMessage("Error adding a vector layer");
        return -1;
    }

    QList<QgsField> attribFields;
    for(int i = 0; i<featFields.size(); ++i)
        attribFields.push_back(featFields[i]);

    auto pr = mainLayer->dataProvider();

    mainLayer->startEditing();

    auto res = pr->addAttributes(attribFields);

    if(!res)
    {
        this->errorMessage("Error adding attributes to the layer" + mainLayer->name());
        return -1;
    }

    mainLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    auto headers = traces.front();

    auto indexListofTraces = headers.indexOf("fault_trace");

    if(indexListofTraces == -1)
    {
        this->errorMessage("Error could not find the required column header 'fault_trace' in the file "+pathToFile);
        return -1;
    }

    traces.pop_front();

    auto numAtrb = attribFields.size();

    for(auto&& it : traces)
    {
        auto coordString = it.at(indexListofTraces);

        auto geometry = theVisualizationWidget->getMultilineStringGeometryFromJson(coordString);

        if(geometry.isEmpty())
        {
            QString msg ="Error getting the feature geometry for scenario faults layer";
            this->errorMessage(msg);

            return -1;
        }

        // create the feature attributes
        QgsAttributes featureAttributes(numAtrb);

        featureAttributes[0] = QVariant("fault_trace");
        featureAttributes[1] = QVariant("Fault Traces");
        featureAttributes[2] = QVariant(it.at(0));


        QgsFeature feature;
        feature.setFields(featFields);

        feature.setGeometry(geometry);

        feature.setAttributes(featureAttributes);

        if(!feature.isValid())
            return -1;

        auto res = pr->addFeature(feature, QgsFeatureSink::FastInsert);
        if(!res)
        {
            this->errorMessage("Error adding the feature to the layer");
            return -1;
        }
    }

    mainLayer->commitChanges(true);
    mainLayer->updateExtents();

    QgsLineSymbol* markerSymbol = new QgsLineSymbol();

    QColor featureColor = QColor(0,0,0,200);
    auto weight = 1.0;

    markerSymbol->setWidth(weight);
    markerSymbol->setColor(featureColor);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,mainLayer);


    return 0;
}



QgsRasterLayer* OpenSRAPreProcessor::loadRaster(const QString& rasterFilePath, const QString& rasterName)
{
    this->statusMessage("Loading Raster Hazard Layer");

    auto rasterlayer = theVisualizationWidget->addRasterLayer(rasterFilePath, rasterName, "gdal");

    if(rasterlayer == nullptr)
    {
        this->errorMessage("Error adding the raster layer "+rasterName+" at path "+rasterFilePath);
        return nullptr;
    }

    rasterlayer->setOpacity(0.5);

    rasterlayer->dataProvider()->setZoomedInResamplingMethod(QgsRasterDataProvider::ResamplingMethod::Bilinear);

    rasterlayer->dataProvider()->setZoomedOutResamplingMethod(QgsRasterDataProvider::ResamplingMethod::Bilinear);

    rasterlayer->dataProvider()->enableProviderResampling(true);

    rasterlayer->setResamplingStage(Qgis::RasterResamplingStage::Provider);

    return rasterlayer;
}



