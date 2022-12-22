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
#include <QHeaderView>

// GIS headers
#include <qgslinesymbol.h>
#include <qgsmapcanvas.h>
#include <qgsrenderer.h>
#include <qgsgraduatedsymbolrenderer.h>
#include <qgslayertreeview.h>
#include <QgsDataProvider.h>
#include <qgsvectorlayer.h>
#include <QgsFields.h>

using namespace QtCharts;

OpenSRAPostProcessor::OpenSRAPostProcessor(QWidget *parent, QGISVisualizationWidget* visWidget) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{

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
    listWidget->header()->resizeSections(QHeaderView::ResizeToContents);

    // Get the map view widget
    auto mapView = theVisualizationWidget->getMapViewWidget("ResultsWidget");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);

    // Enable the selection tool
    mapViewSubWidget->enableSelectionTool();

    // Popup stuff
    // Once map is set, connect to MapQuickView mouse clicked signal
    // connect(mapViewSubWidget.get(), &MapViewSubWidget::mouseClick, theVisualizationWidget, &VisualizationWidget::onMouseClickedGlobal);

    mainWidget->addWidget(mapViewSubWidget.get());

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


    mainWidget->setStretchFactor(0,0);

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



void OpenSRAPostProcessor::handleModifyLegend(void)
{
    auto res_layer = results_layers.at(0);

    auto layerId = res_layer->id();

    theVisualizationWidget->handleLegendChange(layerId);
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

    results_layers = theVisualizationWidget->addVectorInGroup(pathToResults,"Results", "ogr");

    auto mean_layer = results_layers.value(0);

    auto data_provider = dynamic_cast<QgsVectorDataProvider*>(mean_layer->dataProvider());

    if(data_provider == nullptr)
    {
        this->errorMessage("Developer error: could not cast to vector data provider");
        return -1;
    }

    auto fieldsList = data_provider->fields();

    if (fieldsList.isEmpty())
    {
        this->errorMessage("Empty field list given by the data provider");
        return -1;
    }

    QList<QString> finalFieldsList;
    for(auto&& it : fieldsList)
    {
        auto displayName = it.displayName();

        if(!displayName.contains("case"))
            continue;

        finalFieldsList.push_back(displayName);
    }


    for(auto&& it : finalFieldsList)
    {

        auto field_idx = fieldsList.indexOf(it);
        auto field = fieldsList.at(field_idx);

        QString displayName = field.displayName();

        displayName.replace("_"," ");

        auto treeItem = listWidget->addItem(displayName);
        treeItem->setIsCheckable(true);

        treeItem->setProperty("HeaderString", field.name());
    }

    // Default check/select the first item
    listWidget->checkItem(0);
    listWidget->selectItem(0);

    listWidget->resizeColumnToContents(0);

    theVisualizationWidget->zoomToLayer(results_layers.value(0));

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

    results_layers.clear();
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

    auto res_layer = results_layers.at(0);

    if(res_layer == nullptr)
    {
        this->errorMessage("Dev error: Layer is a nullptr in "+QString(__FUNCTION__));
        return;
    }

    auto vector_layer = dynamic_cast<QgsVectorLayer*>(res_layer);

    if(vector_layer == nullptr)
    {
        this->errorMessage("Developer error: could not cast to vector layer in "+QString(__FUNCTION__));
        return;
    }

    auto data_provider = vector_layer->dataProvider();

    // Check to see if that field exists in the layer
    auto idx = data_provider->fieldNameIndex(headerString);

    if(idx == -1)
    {
        this->errorMessage("Could not find the field "+headerString+" in layer "+res_layer->name());
        return;
    }

    auto field = data_provider->fields().at(idx);

    auto layerRenderer = vector_layer->renderer();
    if(layerRenderer == nullptr)
    {
        this->errorMessage("No layer renderer available in layer "+res_layer->name());
        return;
    }

    if(field.isNumeric())
    {

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
            theVisualizationWidget->createCustomClassBreakRenderer(headerString,vector_layer,Qgis::SymbolType::Line,classBreaks,colors,QVector<QString>(),1.0);
        }
        else if(auto graduatedRender = dynamic_cast<QgsGraduatedSymbolRenderer*>(layerRenderer))
        {
            graduatedRender->setClassAttribute(headerString);
        }
        else
        {
            this->errorMessage("Unrecognized type of layer renderer available in layer "+res_layer->name());
            return;
        }
    }
    else
    {
        this->statusMessage("TODO: implement category renderer for non-numeric fields in "+QString(__FUNCTION__));
    }


    theVisualizationWidget->markDirty();
}

