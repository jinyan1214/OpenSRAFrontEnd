#include "LosAngelesPipelineWidget.h"
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
LosAngelesPipelineWidget::LosAngelesPipelineWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType, QString appType) : GISAssetInputWidget(parent, visWidget, assetType, appType)
{
#ifdef OpenSRA
    LosAngelesPipelineWidget::createComponentsBox();
#endif

    componentFileLineEdit->setEnabled(false);
    componentFileLineEdit->setStyleSheet("{color: #000000; background-color: #D3D3D3;}");

    componentFileLineEdit->setPlaceholderText("Click on the \"Load Network\" button to load the Los Angeles natural gas network (clipped from the statewide network for California)");

    browseFileButton->setText("Load Network:");

    // Disconnect the old slot and connect the new one
    disconnect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseComponentInfoFileDialog()));
    connect(browseFileButton,&QPushButton::clicked,this,&LosAngelesPipelineWidget::handleLoadData);

    label1->setText("Use the prepackaged Los Angeles natural gas network (clipped from the statewide network for California)");

    // Hide the CRS
    crsSelectorWidget->hide();

    // Hide the filter
    this->setFilterVisibility(false);

}


// Destructor
LosAngelesPipelineWidget::~LosAngelesPipelineWidget()
{

}


#ifdef OpenSRA
bool LosAngelesPipelineWidget::outputToJSON(QJsonObject &rvObject)
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

    rvObject.insert("DataType", "Region_Network");

    return true;
}


bool LosAngelesPipelineWidget::inputFromJSON(QJsonObject &rvObject)
{
    // load data as part of call for input
    this->handleLoadData();
    return true;
}


void LosAngelesPipelineWidget::createComponentsBox(void)
{

    QWidget* regionLoadWidget = new QWidget();
//    QHBoxLayout* regionSitesLayout = new QHBoxLayout(regionLoadWidget);

    // Insert the widget three rows from the bottom
    insertWidgetIntoLayout(regionLoadWidget,3);

}


#endif


void LosAngelesPipelineWidget::clear()
{
    GISAssetInputWidget::clear();
    isLoaded = false;
}


void LosAngelesPipelineWidget::setTheNodesWidget(PointAssetInputWidget *newTheNodesWidget)
{
    theNodesWidget = newTheNodesWidget;
}


void LosAngelesPipelineWidget::handleLoadData(void)
{

    if(isLoaded)
        return;

    // Get the path to the application data
    auto prefs = OpenSRAPreferences::getInstance();
    auto pathToOSRABackend = prefs->getAppDir();

    // Assemble the path for the
    auto path = pathToOSRABackend + QDir::separator() + "lib" +
            QDir::separator()+"OtherData"+
            QDir::separator()+"Preprocessed"+
            QDir::separator()+"Los_Angeles_Pipeline_Network_Clipped_From_Statewide"+
            QDir::separator() + "Los_Angeles_Pipeline_Network_Clipped_From_Statewide.gpkg";


    if(!QFile::exists(path))
    {
        this->errorMessage("The path "+path+" does not exist ");
    }

    pathToComponentInputFile = path;
    componentFileLineEdit->setText(path);

    this->statusMessage("Loading the Los Angeles network at : "+path);

    this->loadAssetData();

}
