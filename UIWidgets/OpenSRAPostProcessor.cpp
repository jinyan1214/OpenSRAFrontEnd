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
    totalTreeItem = nullptr;
    defaultItem = nullptr;
//    thePipelineDb = nullptr;
    // The first n columns is information about the component and not results
    numInfoCols = 8;

    //QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create a view menu for the dockable windows
    mainWidget = new QSplitter();
    mainWidget->setOrientation(Qt::Horizontal);
    mainWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
//    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    listWidget = new MutuallyExclusiveListWidget(this, "Results");

    connect(listWidget, &MutuallyExclusiveListWidget::itemChecked, this, &OpenSRAPostProcessor::handleListSelection);
//    connect(listWidget, &MutuallyExclusiveListWidget::clearAll, this, &OpenSRAPostProcessor::clearAll);
//    connect(theVisualizationWidget,&VisualizationWidget::emitScreenshot,this,&OpenSRAPostProcessor::assemblePDF);

    listWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);


    // Get the map view widget
    auto mapView = theVisualizationWidget->getMapViewWidget("ResultsWidget");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);

    // Enable the selection tool
    mapViewSubWidget->enableSelectionTool();

    // Popup stuff
    // Once map is set, connect to MapQuickView mouse clicked signal
    // connect(mapViewSubWidget.get(), &MapViewSubWidget::mouseClick, theVisualizationWidget, &VisualizationWidget::onMouseClickedGlobal);

    mainLayout->addWidget(mapViewSubWidget.get());

    // right hand side widget
    QWidget* rightHandWidget = new QWidget();
    QVBoxLayout* rightHandLayout = new QVBoxLayout(rightHandWidget);
    rightHandLayout->setMargin(0);
    rightHandLayout->setContentsMargins(0, 0, 0, 0);

    rightHandLayout->addWidget(listWidget);

    QPushButton* modifyLegendButton = new QPushButton("Modify Legend",this);
    connect(modifyLegendButton, &QPushButton::clicked ,this, &OpenSRAPostProcessor::handleModifyLegend);

    rightHandLayout->addWidget(modifyLegendButton);

    mainWidget->addWidget(rightHandWidget);

    mainLayout->addWidget(mainWidget);

    // The number of header rows in the Pelicun results file
    numHeaderRows = 1;

    mainWidget->setStretchFactor(0,2);

    // Now add the splitter handle
    // Note: index 0 handle is always hidden, index 1 is between the two widgets
    QSplitterHandle *handle = mainWidget->handle(1);

    if(handle == nullptr)
    {
        qDebug()<<"Error getting the handle";
        return;
    }

    auto buttonHandle = new QToolButton(handle);
    QVBoxLayout *layout = new QVBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    mainWidget->setHandleWidth(15);

    buttonHandle->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    buttonHandle->setDown(false);
    buttonHandle->setAutoRaise(false);
    buttonHandle->setCheckable(false);
    buttonHandle->setArrowType(Qt::LeftArrow);
    buttonHandle->setStyleSheet("QToolButton{border:0px solid}; QToolButton:pressed {border:0px solid}");
    buttonHandle->setIconSize(buttonHandle->size());
    layout->addWidget(buttonHandle);

}

//int OpenSRAPostProcessor::importDVresults(const QString& pathToResults)
//{

//    QDir resultsDir(pathToResults);

//    QString errMsg;

//    // Get the existing files in the folder
//    QStringList acceptableFileExtensions = {"*.csv"};
//    QStringList existingCSVFiles = resultsDir.entryList(acceptableFileExtensions, QDir::Files);

//    if(existingCSVFiles.empty())
//    {
//        errMsg = "The results folder is empty. Did you include DV's in the analysis?";
//        throw errMsg;
//    }

//    QString PGDResultsSheet;
//    QString PGVResultsSheet;
//    QString AllResultsSheet;

//    for(auto&& it : existingCSVFiles)
//    {
//        if(it.startsWith("RepairRatePGD"))
//            PGDResultsSheet = it;
//        else if(it.startsWith("RepairRatePGV"))
//            PGVResultsSheet = it;
//        else if(it.startsWith("AllResults"))
//            AllResultsSheet = it;
//    }

//    // Create the CSV reader/writer tool
//    CSVReaderWriter csvTool;

//    // Vector to hold the attributes
//    QVector< QgsAttributes > fieldAttributes;
//    QStringList fieldNames;

//    if(!PGVResultsSheet.isEmpty())
//    {
//        RepairRatePGV = csvTool.parseCSVFile(pathToResults + QDir::separator() + PGVResultsSheet,errMsg);
//        if(!errMsg.isEmpty())
//            throw errMsg;

//        if(!RepairRatePGV.empty())
//            this->processPGVResults(RepairRatePGV,fieldNames,fieldAttributes);
//        else
//        {
//            errMsg = "The PGV results are empty";
//            throw errMsg;
//        }
//    }

//    if(!PGDResultsSheet.isEmpty())
//    {
//        RepairRatePGD = csvTool.parseCSVFile(pathToResults + QDir::separator() + PGDResultsSheet,errMsg);
//        if(!errMsg.isEmpty())
//            throw errMsg;

//        if(!RepairRatePGD.empty())
//            this->processPGDResults(RepairRatePGD,fieldNames,fieldAttributes);
//        else
//        {
//            errMsg = "The PGD results are empty";
//            throw errMsg;
//        }
//    }

//    if(!AllResultsSheet.isEmpty())
//    {
//        RepairRateAll = csvTool.parseCSVFile(pathToResults + QDir::separator() + AllResultsSheet,errMsg);
//        if(!errMsg.isEmpty())
//            throw errMsg;

//        if(!RepairRateAll.empty())
//            this->processTotalResults(RepairRateAll,fieldNames,fieldAttributes);
//        else
//        {
//            errMsg = "The total results are empty";
//            throw errMsg;
//        }
//    }

//    // Get the pipelines database

//    auto thePipelineDB = ComponentDatabaseManager::getInstance()->getAssetDb("GasNetworkPipelines");

//    if(thePipelineDB == nullptr)
//    {
//        QString msg = "Error getting the pipeline database from the input widget!";
//        throw msg;
//    }

//    if(thePipelineDB->isEmpty())
//    {
//        QString msg = "Pipeline database is empty";
//        throw msg;
//    }

//    auto selFeatLayer = thePipelineDB->getSelectedLayer();
//    mapViewSubWidget->setCurrentLayer(selFeatLayer);

////    mapViewSubWidget->addLayerToLegend(selFeatLayer);

//    // Starting editing
//    thePipelineDB->startEditing();

//    auto res = thePipelineDB->addNewComponentAttributes(fieldNames,fieldAttributes,errMsg);
//    if(!res)
//        throw errMsg;

//    // Commit the changes
//    thePipelineDB->commitChanges();

//    defaultItem->setState(2);

//    listWidget->expandAll();

//    return 0;
//}


//int OpenSRAPostProcessor::processTotalResults(const QVector<QStringList>& DVResults, QStringList& fieldNames, QVector<QgsAttributes>& fieldAttributes)
//{

//    auto numRows = DVResults.size();

//    // Check if there is data in the results, and not just the header rows
//    if(numRows < numHeaderRows)
//    {
//        QString msg = "No results to import!";
//        throw msg;
//    }

//    auto numHeaderColumns = DVResults.at(0).size();

//    if(numHeaderColumns < numInfoCols)
//    {
//        QString msg = "No results to import!";
//        throw msg;
//    }

//    if(totalTreeItem == nullptr)
//    {
//        totalTreeItem = listWidget->addItem("Total Repair Rates");
//        totalTreeItem->setIsCheckable(false);
//    }

//    QStringList tableHeadings = DVResults.at(0);

//    QString headerStr = "TotalRepairRateForAllDemands";

//    auto indexOfTotals = tableHeadings.indexOf(headerStr,-1);

//    if(indexOfTotals == -1)
//    {
//        QString msg = "Error getting index to total repairs";
//        throw msg;
//    }

//    QString itemStr = "All demands";

//    defaultItem = listWidget->addItem(itemStr,totalTreeItem);

//    // Set the header string as a property so I can find the header value later
//    defaultItem->setProperty("HeaderString",headerStr);

//    // Start at the row where headers end
//    for(int row = numHeaderRows, count = 0; row<numRows; ++row, ++count)
//    {
//        auto inputRow = DVResults.at(row);

//        // Add the result to the database
//        auto value = inputRow.at(indexOfTotals).toDouble();

//        fieldAttributes[count].push_back(QVariant(value));
//    }

//    fieldNames.append(headerStr);

//    return 0;

//}


void OpenSRAPostProcessor::handleModifyLegend(void)
{

//    auto pipelineInputWidget = theVisualizationWidget->getComponentWidget("GASPIPELINES");

//    if(pipelineInputWidget == nullptr)
//        return;

//    auto pipelineLayer = pipelineInputWidget->getSelectedFeatureLayer();

//    if(pipelineLayer == nullptr)
//        return;

//    auto layerId = pipelineLayer->layerId();

//    theVisualizationWidget->handlePlotColorChange(layerId);
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

    QFileInfo resultsDir(pathToResults);

    if (!resultsDir.exists())
        throw QString("Error, could not find the results file 'analysis_summary.gpkg' in the folder: "+pathToResults);

    auto results_layer = theVisualizationWidget->addVectorInGroup(pathToResults,"Results", "ogr");

    theVisualizationWidget->zoomToLayer(results_layer.value(0));

    return 0;
}


int OpenSRAPostProcessor::importScenarioTraces(const QString& pathToFile)
{

//    if(pathToFile.isEmpty())
//        return 0;

//    CSVReaderWriter csvTool;

//    QString errMsg;

//    auto traces = csvTool.parseCSVFile(pathToFile, errMsg);
//    if(!errMsg.isEmpty())
//    {
//        errorMessage(errMsg);
//        return -1;
//    }

//    if(traces.size() < 2)
//    {
//        statusMessage("No fault traces available.");
//        return 0;
//    }

//    QgsFields featFields;
//    featFields.append(QgsField("AssetType", QVariant::String));
//    featFields.append(QgsField("TabName", QVariant::String));
//    featFields.append(QgsField("SourceIndex", QVariant::String));

//    // Create the pipelines layer
//    auto mainLayer = theVisualizationWidget->addVectorLayer("linestring","Scenario Faults");

//    if(mainLayer == nullptr)
//    {
//        this->errorMessage("Error adding a vector layer");
//        return -1;
//    }

//    QList<QgsField> attribFields;
//    for(int i = 0; i<featFields.size(); ++i)
//        attribFields.push_back(featFields[i]);

//    auto pr = mainLayer->dataProvider();

//    mainLayer->startEditing();

//    auto res = pr->addAttributes(attribFields);

//    if(!res)
//        this->errorMessage("Error adding attributes to the layer" + mainLayer->name());

//    mainLayer->updateFields(); // tell the vector layer to fetch changes from the provider

//    auto headers = traces.front();

//    auto indexListofTraces = headers.indexOf("ListOfTraces");

//    traces.pop_front();

//    auto numAtrb = attribFields.size();

//    for(auto&& it : traces)
//    {
//        auto coordString = it.at(indexListofTraces);

//        auto geometry = theVisualizationWidget->getMultilineStringGeometryFromJson(coordString);

//        if(geometry.isEmpty())
//        {
//            QString msg ="Error getting the feature geometry for scenario faults layer";
//            this->errorMessage(msg);

//            return -1;
//        }

//        // create the feature attributes
//        QgsAttributes featureAttributes(numAtrb);

//        featureAttributes[0] = QVariant("SCENARIO_TRACES");
//        featureAttributes[1] = QVariant("Scenario Traces");
//        featureAttributes[2] = QVariant(it.at(0));


//        QgsFeature feature;
//        feature.setFields(featFields);

//        feature.setGeometry(geometry);

//        feature.setAttributes(featureAttributes);

//        if(!feature.isValid())
//            return -1;

//        auto res = pr->addFeature(feature, QgsFeatureSink::FastInsert);
//        if(!res)
//        {
//            this->errorMessage("Error adding the feature to the layer");
//            return -1;
//        }
//    }

//    mainLayer->commitChanges(true);
//    mainLayer->updateExtents();

//    QgsLineSymbol* markerSymbol = new QgsLineSymbol();

//    QColor featureColor = QColor(0,0,0,200);
//    auto weight = 1.0;

//    markerSymbol->setWidth(weight);
//    markerSymbol->setColor(featureColor);
//    theVisualizationWidget->createSimpleRenderer(markerSymbol,mainLayer);


    return 0;
}


int OpenSRAPostProcessor::importFaultCrossings(const QString& pathToFile)
{

    return 0;
}


void OpenSRAPostProcessor::importResults(const QString& pathToResults)
{
    qDebug() << "OpenSRAPostProcessor: " << pathToResults;

    QString pathToGPKG = pathToResults + QDir::separator() + "analysis_summary.gpkg";
    this->importResultVisuals(pathToGPKG);

}


void OpenSRAPostProcessor::clear(void)
{

    if(thePipelineDb)
        thePipelineDb->clear();

    mapViewSubWidget->clear();
}


void OpenSRAPostProcessor::handleListSelection(const TreeItem* itemSelected)
{
    if(itemSelected == nullptr)
        return;

    auto headerString = itemSelected->property("HeaderString").toString();

    if(headerString.isEmpty())
    {
        this->errorMessage("Could not find the property "+headerString+" in item "+itemSelected->getName());
        return;
    }

    // Get the pipelines database
    auto thePipelineDB = ComponentDatabaseManager::getInstance()->getAssetDb("GasNetworkPipelines");

    if(thePipelineDB == nullptr)
    {
        this->errorMessage("Error getting the pipeline database from the input widget!");
        return;
    }

    if(thePipelineDB->isEmpty())
    {
        this->errorMessage("Pipeline database is empty");
        return;
    }

    auto selFeatLayer = thePipelineDB->getSelectedLayer();
    if(selFeatLayer == nullptr)
    {
        this->errorMessage("Layer is a nullptr in handleListSelection");
        return;
    }

    // Check to see if that field exists in the layer
    auto idx = selFeatLayer->dataProvider()->fieldNameIndex(headerString);

    if(idx == -1)
    {
        this->errorMessage("Could not find the field "+headerString+" in layer "+selFeatLayer->name());
        return;
    }

    auto layerRenderer = selFeatLayer->renderer();
    if(layerRenderer == nullptr)
    {
        this->errorMessage("No layer renderer available in layer "+selFeatLayer->name());
        return;
    }

    // Create a graduated renderer if one does not exist
    if(layerRenderer->type().compare("graduatedSymbol") != 0)
    {
        QVector<QPair<double,double>>classBreaks;
        QVector<QColor> colors;

        classBreaks.append(QPair<double,double>(0.0, 1E-03));
        classBreaks.append(QPair<double,double>(1.00E-03, 1.00E-02));
        classBreaks.append(QPair<double,double>(1.00E-02, 1.00E-01));
        classBreaks.append(QPair<double,double>(1.00E-01, 1.00E+00));
        classBreaks.append(QPair<double,double>(1.00E+00, 1.00E+01));
        classBreaks.append(QPair<double,double>(1.00E+01, 1.00E+10));

        colors.push_back(Qt::darkBlue);
        colors.push_back(QColor(255,255,178));
        colors.push_back(QColor(253,204,92));
        colors.push_back(QColor(253,141,60));
        colors.push_back(QColor(240,59,32));
        colors.push_back(QColor(189,0,38));

        // createCustomClassBreakRenderer(const QString attrName, const QVector<QPair<double,double>>& classBreaks, const QVector<QColor>& colors, QgsVectorLayer * vlayer)
        theVisualizationWidget->createCustomClassBreakRenderer(headerString,selFeatLayer,Qgis::SymbolType::Line,classBreaks,colors);
    }
    else if(auto graduatedRender = dynamic_cast<QgsGraduatedSymbolRenderer*>(layerRenderer))
    {
        graduatedRender->setClassAttribute(headerString);
    }
    else
    {
        this->errorMessage("Unrecognized type of layer renderer available in layer "+selFeatLayer->name());
        return;
    }


    theVisualizationWidget->markDirty();
}

