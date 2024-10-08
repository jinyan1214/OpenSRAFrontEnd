#include "StateWidePipelineWidget.h"
#include "QGISVisualizationWidget.h"
#include "ComponentDatabaseManager.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"
#include "AssetFilterDelegate.h"
#include "AssetInputDelegate.h"
#include "PointAssetInputWidget.h"
#include "OpenSRAPreferences.h"
#include "CRSSelectionWidget.h"

#ifdef OpenSRA
#include "WorkflowAppOpenSRA.h"
#include "WidgetFactory.h"
#include "JsonGroupBoxWidget.h"
#endif

#include <qgsfeature.h>
#include <qgslinesymbol.h>

#include <QFileInfo>
#include <QDir>


// Constructor
StateWidePipelineWidget::StateWidePipelineWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType, QString appType) : GISAssetInputWidget(parent, visWidget, assetType, appType)
{
#ifdef OpenSRA
    StateWidePipelineWidget::createComponentsBox();
#endif

    componentFileLineEdit->setEnabled(false);
    componentFileLineEdit->setStyleSheet("{color: #000000; background-color: #D3D3D3;}");

    componentFileLineEdit->setPlaceholderText("Click on the \"Load Network\" button to load the statewide natural gas network for California");

    browseFileButton->setText("Load Network:");

    // Disconnect the old slot and connect the new one
    disconnect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseComponentInfoFileDialog()));
    connect(browseFileButton,&QPushButton::clicked,this,&StateWidePipelineWidget::handleLoadStateData);

    label1->setText("Use the prepackaged statewide natural gas network for California");

    // Hide the CRS
    crsSelectorWidget->hide();

    // Hide the filter
    this->setFilterVisibility(false);

}


// Destructor
StateWidePipelineWidget::~StateWidePipelineWidget()
{

}


//int StateWidePipelineWidget::loadAssetVisualization(void)
//{

//    auto headers = this->getTableHorizontalHeadings();

//    auto indexLatStart = headers.indexOf("LAT_BEGIN");
//    auto indexLonStart = headers.indexOf("LONG_BEGIN");
//    auto indexLatEnd = headers.indexOf("LAT_END");
//    auto indexLonEnd = headers.indexOf("LONG_END");

//    if(indexLatStart == -1 || indexLonStart == -1 || indexLatEnd == -1 || indexLonEnd == -1)
//    {
//        //Check if nodes are provided in lieu of lat and lon coordinates for the start and end
//        // If they are, the pipelines visualization should be handled separately
//        if(headers.indexOf("node1") != -1 && headers.indexOf("node2") != -1)
//        {
//            if(theNodesWidget != nullptr)
//            {
//                auto numAssets = theNodesWidget->getNumberOfAseets();

//                if(numAssets == 0)
//                {
//                    this->errorMessage("The header columns 'node1' and 'node2' were found in the pipeline input file, but no nodes are found. Please input the nodes before the pipelines.");
//                    this->clear();
//                    return -1;
//                }

//                return 0;
//            }

//            return -1;
//        }

//        this->errorMessage("Could not find the required lat./lon. header labels in the input file. Alternatively, provide start and end nodes. In the pipeline input file, supply the node IDs under the headers 'node1' and 'node2'");
//        return -1;
//    }

//    QgsFields featFields;
//    featFields.append(QgsField("ID", QVariant::Int));
//    featFields.append(QgsField("AssetType", QVariant::String));
//    featFields.append(QgsField("TabName", QVariant::String));

//    // Set the table headers as fields in the table
//    for(int i = 1; i<componentTableWidget->columnCount(); ++i)
//    {
//        auto fieldText = componentTableWidget->horizontalHeaderItemVariant(i);
//        featFields.append(QgsField(fieldText.toString(),fieldText.type()));
//    }

//    auto attribFields = featFields.toList();

//    // Create the pipelines layer
//    mainLayer = theVisualizationWidget->addVectorLayer("linestring","All Pipelines");

//    if(mainLayer == nullptr)
//    {
//        this->errorMessage("Error adding a vector layer");
//        return -1;
//    }

//    auto pr = mainLayer->dataProvider();

//    mainLayer->startEditing();

//    auto res = pr->addAttributes(attribFields);

//    if(!res)
//        this->errorMessage("Error adding attributes to the layer" + mainLayer->name());

//    mainLayer->updateFields(); // tell the vector layer to fetch changes from the provider

//    theComponentDb->setMainLayer(mainLayer);

//    filterDelegateWidget  = new AssetFilterDelegate(mainLayer);

//    // Get the number of rows
//    auto nRows = componentTableWidget->rowCount();

//    auto numAtrb = attribFields.size();

//    for(int i = 0; i<nRows; ++i)
//    {

//        // create the feature attributes
//        QgsAttributes featureAttributes(numAtrb);

//        // Create a new pipeline
//        QString pipelineIDStr = componentTableWidget->item(i,0).toString();

//        int pipelineID =  pipelineIDStr.toInt();

//        // Create a unique ID for the building
////        auto uid = theVisualizationWidget->createUniqueID();

//        // "ID"
//        // "AssetType"
//        // "Tabname"

//        featureAttributes[0] = QVariant(pipelineID);
//        featureAttributes[1] = QVariant(QString(assetType).remove(" "));
//        featureAttributes[2] = QVariant("ID: "+QString::number(pipelineID));

//        // The feature attributes are the columns from the table
//        for(int j = 1; j<componentTableWidget->columnCount(); ++j)
//        {
//            auto attrbVal = componentTableWidget->item(i,j);
//            featureAttributes[2+j] = attrbVal;
//        }

//        QgsFeature feature;
//        feature.setFields(featFields);

//        auto latitudeStart = componentTableWidget->item(i,indexLatStart).toDouble();
//        auto longitudeStart = componentTableWidget->item(i,indexLonStart).toDouble();

//        auto latitudeEnd = componentTableWidget->item(i,indexLatEnd).toDouble();
//        auto longitudeEnd = componentTableWidget->item(i,indexLonEnd).toDouble();

//        // Start and end point of the pipe
//        QgsPointXY point1(longitudeStart,latitudeStart);
//        QgsPointXY point2(longitudeEnd,latitudeEnd);

//        QgsPolylineXY pipeSegment(2);
//        pipeSegment[0]=point1;
//        pipeSegment[1]=point2;

//        feature.setGeometry(QgsGeometry::fromPolylineXY(pipeSegment));

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

//    markerSymbol->setWidth(0.8);
//    markerSymbol->setColor(Qt::darkGray);
//    theVisualizationWidget->createSimpleRenderer(markerSymbol,mainLayer);

////    auto numFeat = mainLayer->featureCount();

//    theVisualizationWidget->zoomToLayer(mainLayer);

//    auto layerId = mainLayer->id();

//    theVisualizationWidget->registerLayerForSelection(layerId,this);

//    // Create the selected building layer
//    selectedFeaturesLayer = theVisualizationWidget->addVectorLayer("linestring","Selected "+assetType);

//    if(selectedFeaturesLayer == nullptr)
//    {
//        this->errorMessage("Error adding the selected assets vector layer");
//        return -1;
//    }

//    QgsLineSymbol* selectedLayerMarkerSymbol = new QgsLineSymbol();

//    selectedLayerMarkerSymbol->setWidth(2.0);
//    selectedLayerMarkerSymbol->setColor(Qt::darkBlue);
//    theVisualizationWidget->createSimpleRenderer(selectedLayerMarkerSymbol,selectedFeaturesLayer);

//    auto pr2 = selectedFeaturesLayer->dataProvider();

//    auto res2 = pr2->addAttributes(attribFields);

//    if(!res2)
//        this->errorMessage("Error adding attributes to the layer");

//    selectedFeaturesLayer->updateFields(); // tell the vector layer to fetch changes from the provider

//    theComponentDb->setSelectedLayer(selectedFeaturesLayer);

//    QVector<QgsMapLayer*> mapLayers;
//    mapLayers.push_back(selectedFeaturesLayer);
//    mapLayers.push_back(mainLayer);

//    theVisualizationWidget->createLayerGroup(mapLayers, assetType);

//    return 0;
//}


#ifdef OpenSRA
bool StateWidePipelineWidget::outputToJSON(QJsonObject &rvObject)
{
    QJsonObject outJson;

    auto res = this->outputAppDataToJSON(outJson);

    if(!res)
        return res;

    QJsonObject appData = outJson["ApplicationData"].toObject();

    if(!appData.contains("assetGISFile") || !appData.contains("pathToSource"))
    {
        this->errorMessage("Error, could not find the 'assetGISFile' or 'pathToSource' fields in 'ApplicationData' in "+this->objectName());
        return false;
    }

    auto assetFileName = appData["assetGISFile"].toString();

    auto assetFilePath = appData["pathToSource"].toString();

    QDir dirInfo(assetFilePath);
    auto assetDirName = dirInfo.dirName();
    auto pathToFile = assetDirName + QDir::separator() + assetFileName;

    //    auto pathToFile = assetFilePath + QDir::separator() + assetFileName;

    //    auto pathToFile = assetFileName;

    rvObject.insert("SiteDataFile", pathToFile);

    if(!appData.contains("CRS"))
    {
        this->errorMessage("Error, could not find the 'CRS' field in 'ApplicationData' in "+this->objectName());
        return false;
    }

    rvObject.insert("CRS",appData["CRS"]);

    rvObject.insert("DataType", "State_Network");

    return true;
}


bool StateWidePipelineWidget::inputFromJSON(QJsonObject &rvObject)
{
    // load state data as part of call for input
    this->handleLoadStateData();
    return true;
}


void StateWidePipelineWidget::createComponentsBox(void)
{

    //    QWidget* stateLoadWidget = new QWidget();
    //    QHBoxLayout* stateSitesLayout = new QHBoxLayout(stateLoadWidget);

    // Insert the widget three rows from the bottom
    //    insertWidgetIntoLayout(stateLoadWidget,3);


}


#endif


void StateWidePipelineWidget::clear()
{
    GISAssetInputWidget::clear();
    isLoaded = false;
    emit headingValuesChanged(QStringList{"N/A"});
}


void StateWidePipelineWidget::setTheNodesWidget(PointAssetInputWidget *newTheNodesWidget)
{
    theNodesWidget = newTheNodesWidget;
}


void StateWidePipelineWidget::handleLoadStateData(void)
{
    // clear previously loaded network
//    this->clear();
//    auto pipelinesMainLayer = this->getMainLayer();
//    if(pipelinesMainLayer != nullptr)
//        theVisualizationWidget->removeLayer(pipelinesMainLayer);

    if(isLoaded)
        return;

    // Get the path to the application data
    auto prefs = OpenSRAPreferences::getInstance();
    auto pathToOSRABackend = prefs->getAppDir();

    // Assemble the path for the
    auto path = pathToOSRABackend + QDir::separator() + "lib" +
            QDir::separator()+"OtherData"+
            QDir::separator()+"Preprocessed"+
            QDir::separator()+"CA_Natural_Gas_Pipeline_Segments_WGS84"+
            QDir::separator() + "CA_Natural_Gas_Pipeline_Segments_WGS84_Under100m.gpkg";

//    QDir::separator()+"CA_Natural_Gas_Pipeline_Segments_WGS84_SUBSET"+
//    QDir::separator() + "CA_Natural_Gas_Pipeline_Segments_WGS84_Under100m_SUBSET.gpkg";


    if(!QFile::exists(path))
    {
        this->errorMessage("The path "+path+" does not exist ");
    }

    pathToComponentInputFile = path;
    componentFileLineEdit->setText(path);

    this->statusMessage("Loading the statewide network at : "+path);

    this->loadAssetData();

    auto tableHeadings = this->getMainLayer()->fields().names();

    emit headingValuesChanged(tableHeadings);

    // zoom to layer
//    theVisualizationWidget->zoomToLayer(this->getMainLayer());

}
