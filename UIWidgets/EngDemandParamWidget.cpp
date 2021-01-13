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

#include "EngDemandParamWidget.h"
#include "EDPLandslideWidget.h"
#include "EDPGroundSettlementWidget.h"
#include "EDPLiquefactionWidget.h"
#include "EDPLatSpreadWidget.h"
#include "EDPGroundStrainWidget.h"
#include "EDPSurfFaultRupWidget.h"
#include "EDPSubSurfFaultRupWidget.h"

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

EngDemandParamWidget::EngDemandParamWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);


    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Engineering Demand Parameter (EDP)"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);

    theHeaderLayout->addStretch(1);
    mainLayout->addLayout(theHeaderLayout);

    auto theComponentSelection = new SimCenterComponentSelection();

    EDPLandslide = new EDPLandslideWidget(this);
    EDPLiquefaction = new EDPLiquefactionWidget(this);
    EDPLatSpread = new EDPLatSpreadWidget(this);
    EDPGroundSettlement = new EDPGroundSettlementWidget(this);
    EDPGroundStrain = new EDPGroundStrainWidget(this);
    EDPSurfFaultRup = new EDPSurfFaultRupWidget(this);
    EDPSubSurfFaultRup = new EDPSubSurfFaultRupWidget(this);

    theComponentSelection->setWidth(120);

    theComponentSelection->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    theComponentSelection->addComponent("Liquefaction",EDPLiquefaction);
    theComponentSelection->addComponent("Lateral\nSpreading",EDPLatSpread);
    theComponentSelection->addComponent("Ground\nSettlement",EDPGroundSettlement);
    theComponentSelection->addComponent("Landslide",EDPLandslide);
    theComponentSelection->addComponent("Surface\nFault Rupture",EDPSurfFaultRup);
    theComponentSelection->addComponent("Subsurface\nFault Rupture",EDPSubSurfFaultRup);
    theComponentSelection->addComponent("Ground\nStrain",EDPGroundStrain);

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

    return false;
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


