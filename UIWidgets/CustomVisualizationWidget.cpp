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

// Written by: Stevan Gavrilovic
// Latest revision: 11.03.2020

#include "CustomVisualizationWidget.h"
#include "VisualizationWidget.h"
#include "sectiontitle.h"
#include "ShakeMapWidget.h"

// GIS headers
#include "GroupLayer.h"
#include "LayerListModel.h"
#include "FeatureLayer.h"
#include "ArcGISMapImageLayer.h"
#include "RasterLayer.h"

#include <QListWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonObject>
#include <QCheckBox>
#include <QMessageBox>
#include <QDebug>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonArray>
#include <QTreeView>
#include <QPushButton>


using namespace Esri::ArcGISRuntime;

CustomVisualizationWidget::CustomVisualizationWidget(QWidget *parent,  VisualizationWidget* visWidget)
    : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    landslideLayer = nullptr;
    liquefactionLayer = nullptr;
    geologicMapLayer = nullptr;

    downloadJsonReply = nullptr;
    baseCGSURL = "https://gis.conservation.ca.gov/server/rest/services/CGS/Geologic_Map_of_California/MapServer";

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Visualization"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);

    QHBoxLayout *theVizLayout = new QHBoxLayout();

    auto visSelectBox = this->getVisSelectionGroupBox();

    theVizLayout->addWidget(visSelectBox);

    auto theVisWidget = theVisualizationWidget->getVisWidget();

    theVizLayout->addWidget(theVisWidget);

    // Export objects
    QHBoxLayout *theExportLayout = new QHBoxLayout();

    QLabel* exportLabel = new QLabel("Export folder:");

    auto exportPathLineEdit = new QLineEdit();
    exportPathLineEdit->setMaximumWidth(1000);
    exportPathLineEdit->setMinimumWidth(400);
    exportPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *exportBrowseFileButton = new QPushButton();
    exportBrowseFileButton->setText(tr("Browse"));
    exportBrowseFileButton->setMaximumWidth(150);

    QPushButton *exportFileButton = new QPushButton();
    exportFileButton->setText(tr("Export to CSV"));
    exportFileButton->setMaximumWidth(150);

    theExportLayout->addStretch();
    theExportLayout->addWidget(exportLabel);
    theExportLayout->addWidget(exportPathLineEdit);
    theExportLayout->addWidget(exportBrowseFileButton);
    theExportLayout->addWidget(exportFileButton);

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addLayout(theVizLayout);
    mainLayout->addLayout(theExportLayout);

    this->setLayout(mainLayout);
    this->setMinimumWidth(640);

    //    QString mapServerUrl =  "/Users/steve/Desktop/SimCenter/OpenSRA/Examples/CA_Precip_1981-2010_30m_WGS84_clip_mm.tif";

    //    auto shpLayer = theVisualizationWidget->createAndAddRasterLayer(mapServerUrl,"Precipitation Map",nullptr);

    //    if(shpLayer != nullptr)
    //        theVisualizationWidget->addLayerToMap(shpLayer);

}


CustomVisualizationWidget::~CustomVisualizationWidget()
{

}


bool CustomVisualizationWidget::outputToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool CustomVisualizationWidget::inputFromJSON(QJsonObject &jsonObject)
{
    return true;
}


int CustomVisualizationWidget::processResults(QString &filenameResults)
{

    return 0;
}


void CustomVisualizationWidget::setCurrentlyViewable(bool status)
{
    theVisualizationWidget->setCurrentlyViewable(status);
}


QGroupBox* CustomVisualizationWidget::getVisSelectionGroupBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Select Data to Visualize");
    groupBox->setMaximumWidth(250);
    //    groupBox->setStyleSheet("background-color: white;");

    auto layout = new QVBoxLayout();
    groupBox->setLayout(layout);

    auto mapDataLabel = new QLabel("Pre-packaged maps and data sets:");
    mapDataLabel->setStyleSheet("font-weight: bold; color: black");

    QCheckBox* CGS1Checkbox = new QCheckBox("CGS Geologic Map (Ref.)");
    QCheckBox* CGS2Checkbox = new QCheckBox("CGS Liquefaction Susceptibility\nMap (Ref.)");
    QCheckBox* CGS3Checkbox = new QCheckBox("CGS Landslide Susceptibility\nMap (Ref.)");

    connect(CGS1Checkbox,&QCheckBox::clicked,this,&CustomVisualizationWidget::showCGSGeologicMap);
    connect(CGS2Checkbox,&QCheckBox::clicked,this,&CustomVisualizationWidget::showCGSLiquefactionMap);
    connect(CGS3Checkbox,&QCheckBox::clicked,this,&CustomVisualizationWidget::showCGSLandslideMap);

    auto DVLabel = new QLabel("Decision Variables:");
    DVLabel->setStyleSheet("font-weight: bold; color: black");

    auto allRepairsLabel = new QLabel("Annual number of repairs \n(breaks and leaks)");
    allRepairsLabel->setStyleSheet("text-decoration: underline; color: black");

    QCheckBox* GMCheckbox = new QCheckBox("Ground Motion");
    QCheckBox* LatSpreadCheckbox = new QCheckBox("Lateral Spreading");
    QCheckBox* LandslideCheckbox = new QCheckBox("Landslide");
    GMCheckbox->setChecked(true);

    auto breaksLabel = new QLabel("Annual number of breaks");
    breaksLabel->setStyleSheet("text-decoration: underline; color: black");
    QCheckBox* GMCheckbox2 = new QCheckBox("Ground Motion");
    QCheckBox* LatSpreadCheckbox2 = new QCheckBox("Lateral Spreading");
    QCheckBox* LandslideCheckbox2 = new QCheckBox("Landslide");

    auto othersLabel = new QLabel("Others:");
    othersLabel->setStyleSheet("font-weight: bold; color: black");
    QCheckBox* shakeMapCheckbox = new QCheckBox("ShakeMap");
    connect(shakeMapCheckbox,&QCheckBox::clicked,this,&CustomVisualizationWidget::showShakeMapLayer);

    loadShakeMapButton = new QPushButton ("Add ShakeMap", this);
    loadShakeMapButton->setVisible(false);

    theShakeMapWidget = std::make_unique<ShakeMapWidget>(theVisualizationWidget);
    connect(loadShakeMapButton,&QPushButton::pressed,theShakeMapWidget.get(),&ShakeMapWidget::showLoadShakeMapDialog);

    layout->addWidget(mapDataLabel);
    layout->addWidget(CGS1Checkbox);
    layout->addWidget(CGS2Checkbox);
    layout->addWidget(CGS3Checkbox);
    layout->addWidget(DVLabel);
    layout->addWidget(allRepairsLabel);
    layout->addWidget(GMCheckbox);
    layout->addWidget(LatSpreadCheckbox);
    layout->addWidget(LandslideCheckbox);
    layout->addWidget(breaksLabel);
    layout->addWidget(GMCheckbox2);
    layout->addWidget(LatSpreadCheckbox2);
    layout->addWidget(LandslideCheckbox2);
    layout->addWidget(othersLabel);
    layout->addWidget(shakeMapCheckbox);
    layout->addWidget(loadShakeMapButton);

    layout->addStretch();

    return groupBox;
}


void CustomVisualizationWidget::processNetworkReply(QNetworkReply* pReply)
{
    if(pReply->error() != QNetworkReply::NoError)
    {
        QString err = "Error in connecting to the server at: " + pReply->url().toString();
        this->userMessageDialog(err);
        return;
    }

    if(pReply == downloadJsonReply)
    {
        this->createGSGLayers();
    }

}


void CustomVisualizationWidget::createGSGLayers()
{
    //    auto eventLayer = new GroupLayer(QList<Layer*>{});
    //    eventLayer->setName("California Geo Map");

    //    auto data = downloadJsonReply->readAll();

    //    QJsonDocument doc = QJsonDocument::fromJson(data);

    //    auto jsonObject = doc.object();

    //    QJsonValue nameValue = jsonObject["layers"];

    //    auto layerArray = nameValue.toArray();

    //    if(layerArray.size() == 0)
    //        return;

    //    auto urlToLayer = baseCGSURL;


    //    for(auto&& it : layerArray)
    //    {
    //        auto ID = it.toObject()["id"];

    //        auto name = it.toObject()["name"];

    //        //      auto urlToLayer = baseCGSURL +"/" + QString::number(ID.toInt());

    //        //      auto shpLayer = theVisualizationWidget->createAndAddMapServerLayer(urlToLayer,name.toString(),eventItem);

    //        //      if(shpLayer != nullptr)
    //        //          theVisualizationWidget->addLayerToMap(shpLayer);
    //    }

}


void CustomVisualizationWidget::showCGSLandslideMap(bool state)
{
    if(state == false)
    {
        if(landslideLayer != nullptr)
        {
            theVisualizationWidget->removeLayerFromMap(landslideLayer);
        }

        return;
    }


    if(landslideLayer == nullptr)
    {
        QString mapServerUrl =  "https://gis.conservation.ca.gov/server/rest/services/CGS/MS58_LandslideSusceptibility_Classes/MapServer";
        landslideLayer = theVisualizationWidget->createAndAddMapServerLayer(mapServerUrl,"Landslide Susceptibility Map",nullptr);
    }

    if(landslideLayer != nullptr)
        theVisualizationWidget->addLayerToMap(landslideLayer);

}


void CustomVisualizationWidget::showCGSGeologicMap(bool state)
{
    if(state == false)
    {
        if(geologicMapLayer != nullptr)
            theVisualizationWidget->removeLayerFromMap(geologicMapLayer);

        return;
    }

    if(geologicMapLayer == nullptr)
        geologicMapLayer = theVisualizationWidget->createAndAddMapServerLayer(baseCGSURL,"California Geo. Map",nullptr);


    if(geologicMapLayer != nullptr)
        theVisualizationWidget->addLayerToMap(geologicMapLayer);


    //    QString addToJson = baseCGSURL + "?f=pjson";

    //    QUrl addressToJson(addToJson);

    //    QNetworkRequest request(addressToJson);

    //    downloadJsonReply = m_WebCtrl.get(request);

    //    connect(&m_WebCtrl, &QNetworkAccessManager::finished, this, &CustomVisualizationWidget::processNetworkReply);

}


void CustomVisualizationWidget::showCGSLiquefactionMap(bool state)
{
    if(state == false)
    {
        if(liquefactionLayer != nullptr)
            theVisualizationWidget->removeLayerFromMap(liquefactionLayer);

        return;
    }

    if(liquefactionLayer == nullptr)
    {
        QString mapServerUrl =  "https://gis.conservation.ca.gov/server/rest/services/CGS_Earthquake_Hazard_Zones/SHP_Liquefaction_Zones/MapServer";
        liquefactionLayer = theVisualizationWidget->createAndAddMapServerLayer(mapServerUrl,"Liquefaction Zones Map",nullptr);
    }

    if(liquefactionLayer != nullptr)
        theVisualizationWidget->addLayerToMap(liquefactionLayer);
}


void CustomVisualizationWidget::showShakeMapLayer(bool state)
{

    theShakeMapWidget->showShakeMapLayers(state);

    if(state == false)
    {
        loadShakeMapButton->setVisible(false);

        return;
    }

    loadShakeMapButton->setVisible(true);
}


