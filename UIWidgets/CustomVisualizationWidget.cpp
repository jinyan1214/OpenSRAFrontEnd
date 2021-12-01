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

#include "CustomVisualizationWidget.h"
#include "QGISVisualizationWidget.h"
#include "sectiontitle.h"
#include "ShakeMapWidget.h"

// GIS headers
#include <qgsvectorlayer.h>

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
#include <QSplitter>
#include <QToolButton>


CustomVisualizationWidget::CustomVisualizationWidget(QWidget *parent,  QGISVisualizationWidget* visWidget)
    : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    this->setContentsMargins(0,0,0,0);

    landslideLayer = nullptr;
    liquefactionLayer = nullptr;
    geologicMapLayer = nullptr;

    baseCGSURL = "https://gis.conservation.ca.gov/server/rest/services/CGS/Geologic_Map_of_California/MapServer";

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(5,0,0,0);
    mainLayout->setSpacing(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    theHeaderLayout->setContentsMargins(0,0,0,0);
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Visualization"));
//    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,0);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);

    QSplitter *theVizLayout = new QSplitter(this);
    visWidget->setContentsMargins(0,0,0,0);

    theVizLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto visSelectBox = this->getVisSelectionGroupBox();

    QWidget* theLeftHandWidget = new QWidget(this);

    theLeftHandWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Expanding);

    QVBoxLayout *theLeftHandLayout = new QVBoxLayout(theLeftHandWidget);
    theLeftHandLayout->setMargin(0);

    theLeftHandLayout->addWidget(visSelectBox);

    theVizLayout->addWidget(theLeftHandWidget);
    theVizLayout->addWidget(theVisualizationWidget);

    theVizLayout->setStretchFactor(1,1);

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addWidget(theVizLayout);

    this->setLayout(mainLayout);

    // Now add the splitter handle
    // Note: index 0 handle is always hidden, index 1 is between the two widgets
    QSplitterHandle *handle = theVizLayout->handle(1);

    if(handle == nullptr)
    {
        qDebug()<<"Error getting the handle";
        return;
    }

    auto buttonHandle = new QToolButton(handle);
    QVBoxLayout *layout = new QVBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    theVizLayout->setHandleWidth(15);

    buttonHandle->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    buttonHandle->setDown(false);
    buttonHandle->setAutoRaise(false);
    buttonHandle->setCheckable(false);
    buttonHandle->setArrowType(Qt::RightArrow);
    buttonHandle->setStyleSheet("QToolButton{border:0px solid}; QToolButton:pressed {border:0px solid}");
    buttonHandle->setIconSize(buttonHandle->size());
    layout->addWidget(buttonHandle);

    QList<int> sizes = {0,theVizLayout->width()};

    theVizLayout->setSizes(sizes);

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


QGroupBox* CustomVisualizationWidget::getVisSelectionGroupBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Select Data to Visualize");
    groupBox->setMaximumWidth(250);
    groupBox->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Minimum);
    //    groupBox->setStyleSheet("background-color: white;");

    auto layout = new QVBoxLayout();
    groupBox->setLayout(layout);

    auto mapDataLabel = new QLabel("Pre-packaged maps and\ndata sets:");
    mapDataLabel->setStyleSheet("font-weight: bold; color: black");

    CGS1Checkbox = new QCheckBox("CGS Geologic Map (Ref.)");
    CGS2Checkbox = new QCheckBox("CGS Liquefaction Susceptibility\nMap (Ref.)");
    CGS3Checkbox = new QCheckBox("CGS Landslide Susceptibility\nMap (Ref.)");

    connect(CGS1Checkbox,&QCheckBox::clicked,this,&CustomVisualizationWidget::showCGSGeologicMap);
    connect(CGS2Checkbox,&QCheckBox::clicked,this,&CustomVisualizationWidget::showCGSLiquefactionMap);
    connect(CGS3Checkbox,&QCheckBox::clicked,this,&CustomVisualizationWidget::showCGSLandslideMap);

    layout->addWidget(mapDataLabel);
    layout->addWidget(CGS1Checkbox);
    layout->addWidget(CGS2Checkbox);
    layout->addWidget(CGS3Checkbox);

    layout->addStretch();

    return groupBox;
}


void CustomVisualizationWidget::showCGSLandslideMap(bool state)
{
    if(state == false)
    {
        if(landslideLayer != nullptr)
        {
//            theVisualizationWidget->removeLayerFromMapAndTree(landslideLayer->layerId());
            delete landslideLayer;
            landslideLayer = nullptr;
        }

        return;
    }


    if(landslideLayer == nullptr)
    {
        QString mapServerUrl =  "https://gis.conservation.ca.gov/server/rest/services/CGS/MS58_LandslideSusceptibility_Classes/MapServer";
//        landslideLayer = theVisualizationWidget->createAndAddMapServerLayer(mapServerUrl,"Landslide Susceptibility Map",nullptr);
    }

}


void CustomVisualizationWidget::showCGSGeologicMap(bool state)
{
    if(state == false)
    {
        if(geologicMapLayer != nullptr)
        {
            theVisualizationWidget->removeLayer(geologicMapLayer);
            geologicMapLayer = nullptr;
        }

        return;
    }

//    if(geologicMapLayer == nullptr)
//        geologicMapLayer = theVisualizationWidget->createAndAddMapServerLayer(baseCGSURL,"California Geological Map",nullptr);

}


void CustomVisualizationWidget::showCGSLiquefactionMap(bool state)
{
    if(state == false)
    {
        if(liquefactionLayer != nullptr)
        {
            theVisualizationWidget->removeLayer(liquefactionLayer);
            liquefactionLayer = nullptr;
        }

        return;
    }

    if(liquefactionLayer == nullptr)
    {
        QString mapServerUrl =  "https://gis.conservation.ca.gov/server/rest/services/CGS_Earthquake_Hazard_Zones/SHP_Liquefaction_Zones/MapServer";
//        liquefactionLayer = theVisualizationWidget->createAndAddMapServerLayer(mapServerUrl,"Liquefaction Zones Map",nullptr);
    }

}



void CustomVisualizationWidget::clear()
{
    theVisualizationWidget->clear();
    CGS1Checkbox->setChecked(false);
    CGS2Checkbox->setChecked(false);
    CGS3Checkbox->setChecked(false);
}

