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

#include "UncertaintyQuantificationWidget.h"
#include "WorkflowAppOpenSRA.h"
#include "JsonDefinedWidget.h"

#include "SimCenterJsonWidget.h"

#include "sectiontitle.h"


UncertaintyQuantificationWidget::UncertaintyQuantificationWidget(QWidget *parent)
    : SimCenterAppWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setContentsMargins(5,0,0,0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Sampling Method"));

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,0);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);

    layout->addLayout(theHeaderLayout);

    //
    // create layout for selection box for method type to layout
    //

    auto uncertaintyWidget = this->getMainWidget();

    if(uncertaintyWidget == nullptr)
    {
        this->errorMessage("Failed to create uncertainty widget");
        return;
    }

    uncertaintyWidget->setContentsMargins(5,0,0,0);

    layout->addWidget(uncertaintyWidget);

    layout->addStretch(1);

}


UncertaintyQuantificationWidget::~UncertaintyQuantificationWidget()
{

}


QWidget* UncertaintyQuantificationWidget::getMainWidget(void)
{
    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams.value("SamplingMethod").toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in SimCenterJsonWidget");
        return nullptr;
    }

    QJsonObject samplingObj;

    samplingObj["SamplingMethod"] = thisObj.value("Method").toObject();

    QJsonObject widgetObj;

    widgetObj["Params"] = samplingObj;

    mainWidget = new JsonDefinedWidget(this, widgetObj, this->objectName());

    return mainWidget;
}



void  UncertaintyQuantificationWidget::clear()
{
    mainWidget->reset();
}


bool UncertaintyQuantificationWidget::outputToJSON(QJsonObject &jsonObject)
{    

    QJsonObject outputObj;
    auto res = mainWidget->outputToJSON(outputObj);

    if(!res || outputObj.isEmpty())
    {
        this->errorMessage("Failed to export json in " + this->objectName());
        return false;
    }

    auto samplingObject = outputObj.value("SamplingMethod").toObject();

    QJsonObject methodObj;
    methodObj["Method"] = samplingObject;

    jsonObject["SamplingMethod"] = methodObj;

    return true;
}


bool UncertaintyQuantificationWidget::inputFromJSON(QJsonObject &jsonObject)
{ 

    QJsonObject methodObj;

    methodObj["SamplingMethod"] = jsonObject.value("Method").toObject();

    auto res = mainWidget->inputFromJSON(methodObj);

    if(!res)
    {
        this->errorMessage("Failed to import json in " + this->objectName());
        return false;
    }

    return true;
}







