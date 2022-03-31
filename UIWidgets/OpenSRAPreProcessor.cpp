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
#include "QGISGasPipelineInputWidget.h"
#include "OpenSRAPreProcessor.h"
#include "ComponentDatabaseManager.h"

#include <qgsvectorlayer.h>
#include <qgsgeometryengine.h>
#include <qgsfeature.h>
#include <qgsrasterlayer.h>
#include <qgsmapcanvas.h>
#include <qgsfillsymbol.h>

#include <QDir>
#include <QVariant>

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


int OpenSRAPreProcessor::run()
{

    // Get the pipeline network widget
    // Get the pipelines database
    auto thePipelineDB = ComponentDatabaseManager::getInstance()->getPipelineComponentDb();

    if(thePipelineDB == nullptr)
    {        
        this->errorMessage("Error getting the pipeline database from the input widget.");
        return -1;
    }

    if(thePipelineDB->isEmpty())
    {
        this->errorMessage("Pipeline database is empty. No pipelines to preprocess.");
        return -1;
    }

    // Check if folder exists
    auto externalDataFolder = pathToBackend + QDir::separator() + "lib" + QDir::separator() + "OtherData";

    if(!QDir(externalDataFolder).exists())
    {
        this->errorMessage("Error, could not find the folder containing backend data: "+externalDataFolder);
        return -1;
    }

    // Get the layer containing the features that are selected for analysis
    auto selFeatLayer = thePipelineDB->getSelectedLayer();
    if(selFeatLayer == nullptr)
    {
        this->errorMessage("Error, could not get the selected features layer");
        return -1;
    }

    auto numFeat = selFeatLayer->featureCount();

    if(numFeat == 0)
    {
        this->errorMessage("Error, the number of features selected for analysis is zero");
        return -1;
    }

    // Layer group to hold all of the layers
    QVector<QgsMapLayer*> layerGroup;

    // Need to return the features with ascending ids
    QgsFeatureRequest featRequest;

    QgsFeatureRequest::OrderByClause orderByClause(QString("id"),true);
    QList<QgsFeatureRequest::OrderByClause> obcList = {orderByClause};
    QgsFeatureRequest::OrderBy orderBy(obcList);
    featRequest.setOrderBy(orderBy);
    auto pipeFeatures = selFeatLayer->getFeatures(featRequest);

    // First pull in the geologic soil parameters
    auto pathGeoMap = externalDataFolder + QDir::separator() + "CGS_CA_Geologic_Map_2010"+ QDir::separator() +"shapefiles"+ QDir::separator() +"GMC_geo_poly.shp";

    auto geoMapLayer = theVisualizationWidget->addVectorLayer(pathGeoMap, "CGS CA Geologic Map 2010", "ogr");
    if(geoMapLayer == nullptr)
    {
        this->errorMessage("Error, could not load the geologic map layer");
        return -1;
    }

    layerGroup.push_back(geoMapLayer);

    // Set the crs for the provided layer so that it appears where it should in the map
    geoMapLayer->setCrs(QgsCoordinateReferenceSystem(QStringLiteral("EPSG:3310")));

    QgsFillSymbol* fillSymbol = new QgsFillSymbol();

    theVisualizationWidget->createCategoryRenderer("PTYPE",geoMapLayer,fillSymbol);

    // Test to remove start
    auto start = high_resolution_clock::now();
    // Test to remove end

    // Coordinate transform to transform into the selected layers coordinate system
    QgsCoordinateTransform ct(geoMapLayer->crs(), selFeatLayer->crs(), QgsProject::instance());

    // The geology layer
    std::vector<QgsFeature> geoFeatVec;
    geoFeatVec.reserve(geoMapLayer->featureCount());
    auto geoMapFeatures = geoMapLayer->getFeatures();
    QgsFeature geoMapFeat;
    while (geoMapFeatures.nextFeature(geoMapFeat))
    {
        auto geom = geoMapFeat.geometry();
        geom.get()->transform(ct);
        geoMapFeat.setGeometry(geom);
        geoFeatVec.push_back(geoMapFeat);
    }


    auto pathSlopeDegrees = externalDataFolder + QDir::separator() + "Slope_Interpretted_From_CA_DEM" + QDir::separator() +"CA_Slope_Degrees_UTM_clip.tif";

    auto slopeDegRasterLayer = this->loadRaster(pathSlopeDegrees, "CA Slope Degrees");
    if(slopeDegRasterLayer == nullptr)
    {
        this->errorMessage("Error, could not load the slope raster map layer");
        return -1;
    }

    layerGroup.push_back(slopeDegRasterLayer);

    slopeDegRasterLayer->setCrs(QgsCoordinateReferenceSystem(QStringLiteral("EPSG:4326")));

    // this->statusMessage("Number of geo features="+QString::number(numgeoFeat));

    QStringList fieldNames = {"PTYPE","Slope Degrees"};
    QVector< QgsAttributes > fieldAttributes(numFeat, QgsAttributes(fieldNames.size()));


    QgsFeature pipeFeat;
    int count = 0;
    while (pipeFeatures.nextFeature(pipeFeat))
    {
        // Get the pipe geometry
        auto pipeGeom = pipeFeat.geometry();

        if(pipeGeom.isNull())
        {
            this->errorMessage("Error: the pipe feature geometry is null "+QString::number(pipeFeat.id()));
            return -1;
        }

        // Use the midpoint of the pipe
        auto pipeMidPoint = pipeGeom.centroid();

        auto pipeMidPointXY = pipeMidPoint.asPoint();

        // if(count%10 == 0)
        //     statusMessage("For point "+QString::number(count)+" pipe coords are "+pipeMidPoint.asWkt());

        // if(pipeMidPoint.isGeosValid())
        // {
        //     this->errorMessage("Error: the pipe feature geometry is not valid "+QString::number(pipeFeat.id()));
        //     return -1;
        // }

        std::unique_ptr< QgsGeometryEngine > pipeGeometryEngine(QgsGeometry::createGeometryEngine(pipeMidPoint.constGet()));
        pipeGeometryEngine->prepareGeometry();

        if(!pipeGeometryEngine->isValid())
        {
            this->errorMessage("Error: the geometry engine is not valid for feature "+QString::number(pipeFeat.id()));
            return -1;
        }

        // Did we find the information
        bool found = false;

        for(auto&& it:geoFeatVec)
        {
            auto g = it.geometry();

            // Do initial bounding box check which is very fast to exclude points that are far away
            auto bb = g.boundingBox();

            // If bounding box contains the point then do a deeper check
            if(bb.contains(pipeMidPointXY))
            {
                QString errMsg;
                const bool intersects = pipeGeometryEngine->intersects(g.constGet(), &errMsg);

                if(!errMsg.isEmpty())
                {
                    this->errorMessage("Error: "+errMsg+" for feature "+QString::number(pipeFeat.id()));
                    return -1;
                }

                if(intersects)
                {
                    //auto gMidPoint = g.centroid();
                    //statusMessage("For geom "+QString::number(it.id())+" coords are "+gMidPoint.asWkt());

                    //this->statusMessage("Pipe "+QString::number(pipeFeat.id())+" intersects with "+QString::number(it.id()));
                    QVariant ptype = it.attribute("PTYPE");

                    if(!ptype.isValid())
                        this->infoMessage("Warning: could not find the attribute PTYPE for feature "+QString::number(pipeFeat.id()));
                    else
                        fieldAttributes[count][0] = ptype;

                    found = true;
                    break;
                }
            }
        }

        if(!found)
            this->infoMessage("Warning: could not find data from the CGS_CA_Geologic_Map_2010 for feature: "+QString::number(pipeFeat.id()));

        found = false;

        auto midPointX = pipeMidPointXY.x();
        auto midPointY = pipeMidPointXY.y();

        auto val = theVisualizationWidget->sampleRaster(midPointX,midPointY,slopeDegRasterLayer,1);

        if (isnan(val))
            this->infoMessage("Warning: failed to sample raster layer for feature: "+QString::number(pipeFeat.id()));

        fieldAttributes[count][1] = QVariant(val);

        ++count;
    }

    // Starting editing
    thePipelineDB->startEditing();

    QString errMsg;
    auto res = thePipelineDB->addNewComponentAttributes(fieldNames,fieldAttributes,errMsg);
    if(!res)
        this->errorMessage(errMsg);

    // Commit the changes
    thePipelineDB->commitChanges();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    this->statusMessage("Done preprocessing in "+QString::number(duration.count())+ " seconds");

    theVisualizationWidget->createLayerGroup(layerGroup,"Data Layers");

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
