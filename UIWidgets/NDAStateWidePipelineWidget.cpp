#include "NDAStateWidePipelineWidget.h"
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
NDAStateWidePipelineWidget::NDAStateWidePipelineWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType, QString appType) : GISAssetInputWidget(parent, visWidget, assetType, appType)
{
#ifdef OpenSRA
    NDAStateWidePipelineWidget::createComponentsBox();
#endif

    componentFileLineEdit->setEnabled(false);
    componentFileLineEdit->setStyleSheet("{color: #000000; background-color: #D3D3D3;}");

    componentFileLineEdit->setPlaceholderText("Click on the \"Load Network\" button to load the NDA natural gas network (statewide)");

    browseFileButton->setText("Load Network:");

    // Disconnect the old slot and connect the new one
    disconnect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseComponentInfoFileDialog()));
    connect(browseFileButton,&QPushButton::clicked,this,&NDAStateWidePipelineWidget::handleLoadData);

    label1->setText("Use the NDA natural gas network (statewide)");

    // Hide the CRS
    crsSelectorWidget->hide();

    // Hide the filter
    this->setFilterVisibility(false);

}


// Destructor
NDAStateWidePipelineWidget::~NDAStateWidePipelineWidget()
{

}



#ifdef OpenSRA
bool NDAStateWidePipelineWidget::outputToJSON(QJsonObject &rvObject)
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

    rvObject.insert("DataType", "NDA_Network");

    return true;
}


bool NDAStateWidePipelineWidget::inputFromJSON(QJsonObject &rvObject)
{
    // load data as part of call for input
    this->handleLoadData();
    return true;
}


void NDAStateWidePipelineWidget::createComponentsBox(void)
{

//    QWidget* loadWidget = new QWidget();
//    QHBoxLayout* sitesLayout = new QHBoxLayout(loadWidget);

//    // Insert the widget three rows from the bottom
//    insertWidgetIntoLayout(loadWidget,3);

}


#endif


void NDAStateWidePipelineWidget::clear()
{
    GISAssetInputWidget::clear();
    isLoaded = false;
    emit headingValuesChanged(QStringList{"N/A"});
}


void NDAStateWidePipelineWidget::setTheNodesWidget(PointAssetInputWidget *newTheNodesWidget)
{
    theNodesWidget = newTheNodesWidget;
}


void NDAStateWidePipelineWidget::handleLoadData(void)
{
    // clear previously loaded network
//    this->clear();
//    auto pipelinesMainLayer = this->getMainLayer();
//    if(pipelinesMainLayer != nullptr)
//        theVisualizationWidget->removeLayer(pipelinesMainLayer);

    emit clearExisting();

    if(isLoaded)
        return;

    // Get the path to the application data
    auto prefs = OpenSRAPreferences::getInstance();
    auto pathToNDAData = prefs->getNDADataDir();

    // Assemble the path for the
    auto path = pathToNDAData +
            QDir::separator() + "Pipeline_Network" +
            QDir::separator() + "Infra_Full"+
            QDir::separator() + "infra_full_preproc.shp";

    if(!QFile::exists(path))
    {
        this->errorMessage("The path "+path+" does not exist ");
    }

    pathToComponentInputFile = path;
    componentFileLineEdit->setText(path);

    this->statusMessage("Loading the NDA network at : "+path);

    this->loadAssetData();

    auto tableHeadings = this->getMainLayer()->fields().names();

    emit headingValuesChanged(tableHeadings);

    // zoom to layer
//    theVisualizationWidget->zoomToLayer(this->getMainLayer());

}
