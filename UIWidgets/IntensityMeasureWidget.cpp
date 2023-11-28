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

#include "IntensityMeasureWidget.h"
#include "sectiontitle.h"
#include "OpenSHAWidget.h"
#include "ShakeMapWidget.h"
#include "UserDefinedGroundMotionWidget.h"
#include "WorkflowAppOpenSRA.h"
#include "JsonDefinedWidget.h"
#include "UserInputFaultWidget.h"
#include "JsonGroupBoxWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QStackedWidget>
#include <QComboBox>
#include <QListWidget>
#include <QSpacerItem>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>
#include <QSplitter>

IntensityMeasureWidget::IntensityMeasureWidget(VisualizationWidget* visWidget, QWidget *parent)
    : SimCenterAppWidget(parent)
{
    this->setContentsMargins(0,0,0,0);
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    IMSelectCombo = new QComboBox(this);
    IMSelectCombo->addItem("UCERF3 (Preferred)","UCERF");
    IMSelectCombo->addItem("ShakeMap","ShakeMap");
    IMSelectCombo->addItem("User-defined Rupture","UserDefinedRupture");
    IMSelectCombo->addItem("User-defined Ground Motions","UserDefinedGM");

    //    IMSelectCombo->addItem("User-defined");
    IMSelectCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    connect(IMSelectCombo,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&IntensityMeasureWidget::IMSelectionChanged);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(5,0,0,0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    theHeaderLayout->setContentsMargins(0,0,0,0);
    theHeaderLayout->setMargin(0);
    theHeaderLayout->setSpacing(0);
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Intensity Measure (IM)"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,0);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addWidget(IMSelectCombo);

//    auto IMBox = this->getIMBox();

//    auto corrBox = this->getCorrelationWidget();

    openSHA = new OpenSHAWidget(this);
    shakeMap = new ShakeMapWidget(visWidget);

    shakeMapStackedWidget = shakeMap->getShakeMapWidget();

    userFaultWidget = new UserInputFaultWidget(visWidget, this);

    userGMWidget = new UserDefinedGroundMotionWidget(visWidget, this);

    mainPanel = new QStackedWidget();
    mainPanel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    mainPanel->setContentsMargins(5,0,0,0);
    mainPanel->layout()->setMargin(0);
    mainPanel->layout()->setSpacing(0);

    mainPanel->addWidget(openSHA);
    mainPanel->addWidget(shakeMapStackedWidget);
    mainPanel->addWidget(userFaultWidget);
    mainPanel->addWidget(userGMWidget);

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addWidget(mainPanel);
//    mainLayout->addWidget(IMBox);
//    mainLayout->addWidget(corrBox);
//    mainLayout->addStretch(1);

}


IntensityMeasureWidget::~IntensityMeasureWidget()
{

}


bool IntensityMeasureWidget::outputToJSON(QJsonObject &jsonObject)
{
    QJsonObject IMobj;

    auto index = IMSelectCombo->currentIndex();

    QJsonObject sourceIM;

    bool res = false;

    if(index == 0)
    {
        QJsonObject outObject;
        res = openSHA->outputToJSON(outObject);

        if(!res)
        {
            this->errorMessage("Error in the json output of OpenSHA");
            return false;
        }

        sourceIM["UCERF"] = outObject;
    }
    else if(index == 1)
    {
        QJsonObject outObject;
        res = shakeMap->outputToJSON(outObject);

        if(!res)
        {
            this->errorMessage("Error in the json output of ShakeMap");
            return false;
        }

        sourceIM["ShakeMap"] = outObject;
    }
    else if(index == 2)
    {
        QJsonObject outObject;
        res = userFaultWidget->outputToJSON(outObject);

        if(!res)
        {
            this->errorMessage("Error in the json output of UserInputFault widget");
            return false;
        }

        sourceIM["UserDefinedRupture"] = outObject;
    }
    else if(index == 3)
    {
        QJsonObject outObject;
        res = userGMWidget->outputToJSON(outObject);

        if(!res)
        {
            this->errorMessage("Error in the json output of UserDefinedGroundMtion widget");
            return false;
        }

        sourceIM["UserDefinedGM"] = outObject;
    }
    else
    {
        return false;
    }

    IMobj["SourceForIM"] = sourceIM;

//    QJsonObject typeObject;
//    res = typeWidget->outputToJSON(typeObject);

//    if(!res)
//    {
//        this->errorMessage("Error in the json output of intensity measure type");
//        return false;
//    }

//    QJsonObject::const_iterator typeObj;
//    for (typeObj = typeObject.begin(); typeObj != typeObject.end(); ++typeObj)
//    {
//        auto key = typeObj.key();
//        IMobj[key] = typeObj.value().toObject();
//    }

//    QJsonObject corrObject;
//    res = corrWidget->outputToJSON(corrObject);

//    if(!res)
//    {
//        this->errorMessage("Error in the json output of intensity measure correlation");
//        return false;
//    }

//    QJsonObject::const_iterator corrObj;
//    for (corrObj = corrObject.begin(); corrObj != corrObject.end(); ++corrObj)
//    {
//        auto key = corrObj.key();
//        IMobj[key] = corrObj.value().toObject();
//    }

    jsonObject["IntensityMeasure"] = IMobj;

    return true;
}


void IntensityMeasureWidget::clear()
{
    IMSelectCombo->setCurrentIndex(0);

    openSHA->clear();
    shakeMap->clear();
    userFaultWidget->clear();

//    typeWidget->reset();
//    corrWidget->reset();
}


bool IntensityMeasureWidget::inputFromJSON(QJsonObject &jsonObject)
{

    auto IMSourceObject = jsonObject.value("SourceForIM").toObject();

    if(IMSourceObject.isEmpty())
        return false;

    auto IMSourceTypes = IMSourceObject.keys();

    if(IMSourceTypes.isEmpty())
        return false;

    auto IMSourceType = IMSourceTypes.first();

    int index = IMSelectCombo->findData(IMSourceType);
    if (index != -1)
    {
        IMSelectCombo->setCurrentIndex(index);
    }

    if(IMSourceType == "UCERF")
    {
        auto res = openSHA->inputFromJSON(IMSourceObject);
        if(res == false)
            return false;

    }
    else if(IMSourceType == "ShakeMap")
    {
        auto res =  shakeMap->inputFromJSON(IMSourceObject);
        if(res == false)
            return false;
    }
    else if(IMSourceType == "UserDefinedRupture")
    {
        auto res =  userFaultWidget->inputFromJSON(IMSourceObject);
        if(res == false)
            return false;
    }
    else if(IMSourceType == "UserDefinedGM")
    {
        auto res =  userGMWidget->inputFromJSON(IMSourceObject);
        if(res == false)
            return false;
    }
    else
    {
        this->errorMessage("Cannot find the intensity measure object of type " + IMSourceType);
        return false;
    }

//    auto res = typeWidget->inputFromJSON(jsonObject);
//    if(res == false)
//        return false;

//    auto res2 = corrWidget->inputFromJSON(jsonObject);
//    if(res2 == false)
//        return false;

    return true;
}


void IntensityMeasureWidget::IMSelectionChanged(int index)
{
    if(index == 0)
    {
        mainPanel->setCurrentWidget(openSHA);
    }
    else if(index == 1)
    {
        mainPanel->setCurrentWidget(shakeMapStackedWidget);
    }
    else if(index == 2)
    {
        mainPanel->setCurrentWidget(userFaultWidget);
    }
    else if(index == 3)
    {
        mainPanel->setCurrentWidget(userGMWidget);
    }
}


bool IntensityMeasureWidget::copyFiles(QString &destDir)
{
    auto numMapsLoaded = shakeMap->getNumShakeMapsLoaded();
    if(numMapsLoaded != 0)
        shakeMap->copyFiles(destDir);

    return true;
}


QWidget* IntensityMeasureWidget::getIMBox(void)
{
    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams["IntensityMeasure"].toObject()["Type"].toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in EDPLandslideWidget");
        return nullptr;
    }

    QString nameToDisplay = thisObj.value("NameToDisplay").toString();

    typeWidget = new JsonGroupBoxWidget(this, thisObj, "Type");
    typeWidget->setObjectName("Type");
    typeWidget->setTitle(nameToDisplay);

    return typeWidget;
}


QWidget* IntensityMeasureWidget::getCorrelationWidget(void)
{
    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams["IntensityMeasure"].toObject()["Correlation"].toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in EDPLandslideWidget");
        return nullptr;
    }

    QString nameToDisplay = thisObj.value("NameToDisplay").toString();

    corrWidget = new JsonGroupBoxWidget(this, thisObj, "Correlation");
    corrWidget->setTitle(nameToDisplay);
    corrWidget->setObjectName("Correlation");

    return corrWidget;
}
