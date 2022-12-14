/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
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

#include "CSVReaderWriter.h"
#include "AssetInputWidget.h"
#include "GeneralInformationWidget.h"
#include "MainWindowWorkflowApp.h"
#include "OpenSRAPostProcessor.h"
#include "REmpiricalProbabilityDistribution.h"
#include "TablePrinter.h"
#include "TreeItem.h"
#include "QGISVisualizationWidget.h"
#include "ComponentDatabaseManager.h"
#include "WorkflowAppOpenSRA.h"
#include "EmbeddedMapViewWidget.h"
#include "MutuallyExclusiveListWidget.h"

#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QChartView>
#include <QComboBox>
#include <QDir>
#include <QDockWidget>
#include <QFileInfo>
#include <QFontMetrics>
#include <QGraphicsLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineSeries>
#include <QPixmap>
#include <QListView>
#include <QPrinter>
#include <QSplitter>
#include <QStackedBarSeries>
#include <QStringList>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextCursor>
#include <QTextTable>
#include <QValueAxis>
#include <QToolButton>
#include <QPushButton>

// GIS headers
#include <qgslinesymbol.h>
#include <qgsmapcanvas.h>
#include <qgsrenderer.h>
#include <qgsgraduatedsymbolrenderer.h>
#include <qgslayertreeview.h>

using namespace QtCharts;

OpenSRAPostProcessor::OpenSRAPostProcessor(QWidget *parent, QGISVisualizationWidget* visWidget) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create a view menu for the dockable windows
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    // Get the map view widget
    auto mapView = theVisualizationWidget->getMapViewWidget("ResultsWidget");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);

    // Enable the selection tool
    mapViewSubWidget->enableSelectionTool();

    // Popup stuff
    // Once map is set, connect to MapQuickView mouse clicked signal
    // connect(mapViewSubWidget.get(), &MapViewSubWidget::mouseClick, theVisualizationWidget, &VisualizationWidget::onMouseClickedGlobal);

    mainLayout->addWidget(mapViewSubWidget.get());

}


void OpenSRAPostProcessor::showEvent(QShowEvent *e)
{
    auto mainCanvas = mapViewSubWidget->getMainCanvas();

    auto mainExtent = mainCanvas->extent();

    mapViewSubWidget->mapCanvas()->zoomToFeatureExtent(mainExtent);
    QWidget::showEvent(e);
}


int OpenSRAPostProcessor::importResultVisuals(const QString& pathToResults)
{

    QDir resultsDir(pathToResults);

    // Get the existing files in the folder
    QStringList acceptableFileExtensions = {"*.csv"};
    QStringList existingCSVFiles = resultsDir.entryList(acceptableFileExtensions, QDir::Files);

    if(existingCSVFiles.empty())
    {
        errorMessage("The results folder is empty. Did you include DV's in the analysis?");
        return -1;
    }

    for(auto&& it : existingCSVFiles)
    {
        QString pathToFile = pathToResults+ QDir::separator() + it;

        if(it.startsWith("ScenarioTraces"))
        {
            this->importScenarioTraces(pathToFile);
        }
        else if(it.startsWith("FaultCrossings"))
        {
            this->importFaultCrossings(pathToFile);
        }
    }

    return 0;
}


int OpenSRAPostProcessor::importScenarioTraces(const QString& pathToFile)
{

    if(pathToFile.isEmpty())
        return 0;

    CSVReaderWriter csvTool;

    QString errMsg;

    auto traces = csvTool.parseCSVFile(pathToFile, errMsg);
    if(!errMsg.isEmpty())
    {
        errorMessage(errMsg);
        return -1;
    }

    if(traces.size() < 2)
    {
        statusMessage("No fault traces available.");
        return 0;
    }

    QgsFields featFields;
    featFields.append(QgsField("AssetType", QVariant::String));
    featFields.append(QgsField("TabName", QVariant::String));
    featFields.append(QgsField("SourceIndex", QVariant::String));

    // Create the pipelines layer
    auto mainLayer = theVisualizationWidget->addVectorLayer("linestring","Scenario Faults");

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
        this->errorMessage("Error adding attributes to the layer" + mainLayer->name());

    mainLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    auto headers = traces.front();

    auto indexListofTraces = headers.indexOf("ListOfTraces");

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

        featureAttributes[0] = QVariant("SCENARIO_TRACES");
        featureAttributes[1] = QVariant("Scenario Traces");
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


int OpenSRAPostProcessor::importFaultCrossings(const QString& pathToFile)
{

    return 0;
}


void OpenSRAPostProcessor::importResults(const QString& pathToResults)
{
    qDebug() << "OpenSRAPostProcessor: " << pathToResults;

    QString errMsg;

    // Get the pipelines database
    thePipelineDb = ComponentDatabaseManager::getInstance()->getAssetDb("GasNetworkPipelines");

    if(thePipelineDb == nullptr)
    {
        errMsg = "Could not get the pipeline database";
        throw errMsg;
    }

    QString pathToScenarioTraces = pathToResults + QDir::separator() + "IM" + QDir::separator() + "SeismicSource";
    this->importResultVisuals(pathToScenarioTraces);

}


void OpenSRAPostProcessor::clear(void)
{

    if(thePipelineDb)
        thePipelineDb->clear();

    mapViewSubWidget->clear();
}

