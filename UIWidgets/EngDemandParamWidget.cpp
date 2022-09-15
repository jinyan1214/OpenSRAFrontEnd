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

// Written by: Stevan Gavrilovic, SimCenter @ UC Berkeley

#include "EngDemandParamWidget.h"
#include "EDPLandslideWidget.h"
#include "SimCenterJsonWidget.h"

#include "sectiontitle.h"
#include "SimCenterComponentSelection.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QStackedWidget>
#include <QComboBox>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>

EngDemandParamWidget::EngDemandParamWidget(QJsonObject mainObj, QWidget *parent) : SimCenterAppWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(5,0,0,0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Engineering Demand Parameter (EDP)"));

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,0);
    theHeaderLayout->addItem(spacer);

    theHeaderLayout->addStretch(1);
    mainLayout->addLayout(theHeaderLayout);

    auto theComponentSelection = new SimCenterComponentSelection();

    auto obj = mainObj.value("EngineeringDemandParameter").toObject();

    auto landslideObj = obj.value("Landslide").toObject();

    if(!landslideObj.isEmpty() && landslideObj.value("ToDisplay").toBool(false) == true)
        EDPLandslide = new EDPLandslideWidget(landslideObj,this);

    auto liqObj = obj.value("Liquefaction").toObject();

    if(!liqObj.isEmpty() && liqObj.value("ToDisplay").toBool(false) == true)
        EDPLiquefaction = new SimCenterJsonWidget("Liquefaction",liqObj, this);

    auto latSpreadObj = obj.value("LateralSpread").toObject();

    if(!latSpreadObj.isEmpty() && latSpreadObj.value("ToDisplay").toBool(false) == true)
        EDPLatSpread = new SimCenterJsonWidget("LateralSpread",latSpreadObj,this);

    auto groundSettlementObj = obj.value("Settlement").toObject();

    if(!groundSettlementObj.isEmpty() && groundSettlementObj.value("ToDisplay").toBool(false) == true)
        EDPGroundSettlement = new SimCenterJsonWidget("Settlement",groundSettlementObj,this);

    auto surfFaultObj = obj.value("SurfaceFaultRupture").toObject();

    if(!surfFaultObj.isEmpty() && surfFaultObj.value("ToDisplay").toBool(false) == true)
        EDPSurfFaultRup = new SimCenterJsonWidget("SurfaceFaultRupture",surfFaultObj,this);

    auto groundStrainObj = obj.value("TransientGroundStrain").toObject();

    if(!groundStrainObj.isEmpty() && groundStrainObj.value("ToDisplay").toBool(false) == true)
        EDPGroundStrain = new SimCenterJsonWidget("TransientGroundStrain",groundStrainObj,this);

    auto ssFaultRupObj = obj.value("SubsurfaceFaultRupture").toObject();

    if(!ssFaultRupObj.isEmpty() && ssFaultRupObj.value("ToDisplay").toBool(false) == true)
        EDPSubSurfFaultRup = new SimCenterJsonWidget("SubsurfaceFaultRupture",ssFaultRupObj,this);

    theComponentSelection->setWidth(120);
    theComponentSelection->setItemWidthHeight(120,70);

    theComponentSelection->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    if(EDPLiquefaction)
        theComponentSelection->addComponent("Liquefaction",EDPLiquefaction);
    if(EDPLatSpread)
        theComponentSelection->addComponent("Lateral\nSpreading",EDPLatSpread);
    if(EDPGroundSettlement)
        theComponentSelection->addComponent("Ground\nSettlement",EDPGroundSettlement);
    if(EDPLandslide)
        theComponentSelection->addComponent("Landslide",EDPLandslide);
    if(EDPSurfFaultRup)
        theComponentSelection->addComponent("Surface\nFault Rupture",EDPSurfFaultRup);
    if(EDPSubSurfFaultRup)
        theComponentSelection->addComponent("Subsurface\nFault Rupture",EDPSubSurfFaultRup);
    if(EDPGroundStrain)
        theComponentSelection->addComponent("Transient\nGround\nStrain",EDPGroundStrain);

    if(EDPLiquefaction)
        theComponentSelection->displayComponent("Liquefaction");

    mainLayout->addWidget(theComponentSelection);
}

EngDemandParamWidget::~EngDemandParamWidget()
{

}


bool EngDemandParamWidget::outputToJSON(QJsonObject &jsonObject)
{

    QJsonObject outputObj;

    QJsonObject typeObj;

    EDPLandslide->outputToJSON(typeObj);
    EDPLiquefaction->outputToJSON(typeObj);
    EDPLatSpread->outputToJSON(typeObj);
    EDPGroundSettlement->outputToJSON(typeObj);
    EDPGroundStrain->outputToJSON(typeObj);
    EDPSurfFaultRup->outputToJSON(typeObj);
    EDPSubSurfFaultRup->outputToJSON(typeObj);

    outputObj.insert("Type",typeObj);

    jsonObject.insert("EngineeringDemandParameter",outputObj);

    return true;
}


bool EngDemandParamWidget::inputFromJSON(QJsonObject &jsonObject)
{  
    auto typeObj = jsonObject["Type"].toObject();

    if(typeObj.isEmpty())
        return false;

    auto liquefactionObj = typeObj.value("Liquefaction").toObject();
    if(!liquefactionObj.isEmpty())
        EDPLiquefaction->inputFromJSON(liquefactionObj);

    auto landslideObj = typeObj.value("Landslide").toObject();
    if(!landslideObj.isEmpty())
        EDPLandslide->inputFromJSON(landslideObj);

    auto latSpreadObj = typeObj.value("LateralSpread").toObject();
    if(!latSpreadObj.isEmpty())
        EDPLatSpread->inputFromJSON(latSpreadObj);

    auto groundSpreadObj = typeObj.value("Settlement").toObject();
    if(!groundSpreadObj.isEmpty())
        EDPGroundSettlement->inputFromJSON(groundSpreadObj);

    auto surfFaultObj = typeObj.value("SurfaceFaultRupture").toObject();
    if(!surfFaultObj.isEmpty())
        EDPSurfFaultRup->inputFromJSON(surfFaultObj);

    auto subSurfObj = typeObj.value("SubsurfaceFaultRupture").toObject();
    if(!subSurfObj.isEmpty())
        EDPSubSurfFaultRup->inputFromJSON(subSurfObj);

    auto groundStrainObj = typeObj.value("TransientGroundStrain").toObject();
    if(!groundStrainObj.isEmpty())
        EDPGroundStrain->inputFromJSON(groundStrainObj);

    return true;
}


bool EngDemandParamWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool EngDemandParamWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    return true;
}


bool EngDemandParamWidget::copyFiles(QString &destDir)
{

    return false;
}

void EngDemandParamWidget::clear(void)
{
    EDPLandslide->clear();
    EDPLiquefaction->clear();
    EDPLatSpread->clear();
    EDPGroundSettlement->clear();
    EDPGroundStrain->clear();
    EDPSurfFaultRup->clear();
    EDPSubSurfFaultRup->clear();
}


