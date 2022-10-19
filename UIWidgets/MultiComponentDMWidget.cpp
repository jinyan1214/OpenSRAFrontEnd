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

#include "MultiComponentDMWidget.h"
#include "EDPLandslideWidget.h"
#include "SimCenterJsonWidget.h"
#include "WorkflowAppOpenSRA.h"
#include "DamageMeasureWidget.h"

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

MultiComponentDMWidget::MultiComponentDMWidget(QWidget *parent) : MultiComponentR2D("DamageMeasure",parent)
{
    theMainLayout->setMargin(0);
    theMainLayout->setSpacing(0);
    theMainLayout->setContentsMargins(5,0,0,0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Damage Measure (DM)"));

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,0);
    theHeaderLayout->addItem(spacer);

    theHeaderLayout->addStretch(1);

    theMainLayout->insertLayout(0,theHeaderLayout);

    auto mainObj = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    auto belowGroundObj = mainObj.value("BelowGround").toObject();

    belowGround = new DamageMeasureWidget(belowGroundObj,this);

    auto aboveGroundObj = mainObj.value("AboveGround").toObject();

    aboveGround = new DamageMeasureWidget(aboveGroundObj,this);

    auto wellsCaprocksObj = mainObj.value("WellsCaprocks").toObject();

    wellAndCaprocks = new DamageMeasureWidget(wellsCaprocksObj,this);

    this->addComponent("Pipelines", belowGround);
    this->addComponent("Wells and Caprocks",  wellAndCaprocks);
    this->addComponent("Above Ground \nGas Infrastructure", aboveGround);

    this->hideSelectionWidget();

}


MultiComponentDMWidget::~MultiComponentDMWidget()
{

}


void MultiComponentDMWidget::handleWidgetSelected(const QString& name)
{

    auto res = this->show(name);

    if(!res)
        this->errorMessage("Error, could not display the widget with the name "+name);

}


bool MultiComponentDMWidget::outputToJSON(QJsonObject &jsonObject)
{    
    auto currComponent = this->getCurrentComponent();

    if(currComponent == nullptr)
    {
        this->errorMessage("Could not get the current component in "+QString(__FUNCTION__)+", contact the developer");
        return false;
    }

    return currComponent->outputToJSON(jsonObject);
}


bool MultiComponentDMWidget::inputFromJSON(QJsonObject &jsonObject)
{  
    if(jsonObject.isEmpty())
        return false;

    auto currComponent = this->getCurrentComponent();

    if(currComponent == nullptr)
    {
        this->errorMessage("Could not get the current component in "+QString(__FUNCTION__)+", contact the developer");
        return false;
    }

    return currComponent->inputFromJSON(jsonObject);
}


bool MultiComponentDMWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    Q_UNUSED(jsonObject);
    return true;
}


bool MultiComponentDMWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    Q_UNUSED(jsonObject);
    return true;
}


bool MultiComponentDMWidget::copyFiles(QString &destDir)
{
    Q_UNUSED(destDir);

    return true;
}


void MultiComponentDMWidget::clear(void)
{ 
    belowGround->clear();
    aboveGround->clear();
    wellAndCaprocks->clear();
}


