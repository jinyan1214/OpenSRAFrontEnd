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
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

// Written by: Dr. Stevan Gavrilovic, UC Berkeley

#include "SimCenterJsonWidget.h"
#include "CustomListWidget.h"
#include "TreeItem.h"
#include "JsonDefinedWidget.h"
#include "JsonWidget.h"
#include "WidgetFactory.h"
#include "AddToRunListWidget.h"
#include "WorkflowAppOpenSRA.h"
#include "RandomVariablesWidget.h"

#include <QCheckBox>
#include <QSplitter>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>


SimCenterJsonWidget::SimCenterJsonWidget(QString methodName, QJsonObject jsonObj, QWidget* parent) : SimCenterAppWidget(parent)
{
    this->setObjectName(methodName);

    theInputParamsWidget = WorkflowAppOpenSRA::getInstance()->getTheRandomVariableWidget();

    assert(theInputParamsWidget != nullptr);

    methodKey = methodName;

    QSplitter *splitter = new QSplitter(this);

    listWidget = new CustomListWidget("List of Cases to Run",this);

    connect(listWidget,&QAbstractItemView::clicked,this,&SimCenterJsonWidget::handleListItemSelected);
    connect(listWidget,&CustomListWidget::itemRemoved,this,&SimCenterJsonWidget::handleItemRemoved);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setMargin(0);

    auto mainWidget = this->getWidgetBox(jsonObj);

    if(mainWidget)
        splitter->addWidget(mainWidget);

    splitter->addWidget(listWidget);

    mainLayout->addWidget(splitter);
}


QGroupBox* SimCenterJsonWidget::getWidgetBox(const QJsonObject jsonObj)
{
    if(jsonObj.isEmpty())
    {
        this->errorMessage("Json object is empty in SimCenterJsonWidget");
        return nullptr;
    }

    QGroupBox* groupBox = new QGroupBox(this->objectName());
    groupBox->setContentsMargins(0,0,0,0);
    groupBox->setFlat(true);

    methodWidget = new JsonDefinedWidget(this, jsonObj, this->objectName());
    methodWidget->setObjectName("MethodWidget");

    QVBoxLayout* mainLayout = new QVBoxLayout(groupBox);
    mainLayout->setContentsMargins(3,10,0,0);

    QVBoxLayout* methodLayout = new QVBoxLayout();
    methodLayout->setMargin(0);
    methodLayout->addWidget(methodWidget);
    methodLayout->addStretch();

    // Add the weight and add to run list button at the bottom
    addRunListWidget = new AddToRunListWidget();

    connect(addRunListWidget,&AddToRunListWidget::addToRunListButtonPressed, this, &SimCenterJsonWidget::handleAddButtonPressed);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setMargin(0);
    inputLayout->setContentsMargins(0,0,0,0);
    inputLayout->addLayout(methodLayout);

    mainLayout->addLayout(inputLayout);

    mainLayout->addWidget(addRunListWidget,Qt::AlignCenter);

    mainLayout->setStretch(0,1);
    mainLayout->setStretch(1,0);

    // Add a vertical spacer at the bottom to push everything up
    //    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    //    mainLayout->addItem(vspacer);


    return groupBox;
}


void SimCenterJsonWidget::handleAddButtonPressed(void)
{
    QJsonObject methodObj;
    methodWidget->outputToJSON(methodObj);

    auto passedObj = methodWidget->getJsonObj();

    auto isObj = methodObj.value("Method").toObject();

    QStringList keys;

    if(!isObj.isEmpty())
    {
        methodObj = isObj;
        keys = methodObj.keys();
    }
    else
    {
        keys.push_back(methodObj.value("Method").toString());
    }

    if(methodObj.isEmpty() || keys.size() != 1)
    {
        this->errorMessage("Error in getting the method in SimCenterJsonWidget");
        return;
    }

    auto key = keys.front();

    methodObj = methodObj.value(key).toObject();

    // Object to store the variable types
    QJsonObject typesObj;

    QJsonObject uuidObj;

    auto res = this->getVarTypes(methodObj,passedObj,key,typesObj);
    if(res != 0)
    {
        this->errorMessage("Error, could not get the var types in SimCenterJsonWidget");
        return;
    }

    auto paramsKeys = methodObj.keys();
    for(auto&& it : paramsKeys)
    {
        // Create a unique id to identify the specific instance of these parameters
        auto uid = QUuid::createUuid().toString();

        uuidObj[it]=uid;

        auto fromModel = methodKey+"-"+key;

        auto varType = typesObj[it].toString();

        if(varType.isEmpty())
        {
            this->errorMessage("Error getting the variable type for "+theInputParamsWidget->objectName());
            return;
        }

        auto res = theInputParamsWidget->addNewInputParameter(it,fromModel,uid,varType);

        if(!res)
        {
            this->errorMessage("Error adding a random variable to "+theInputParamsWidget->objectName());
            return;
        }
    }

    // Add the model weight, epistemic uncertainty, aleatory variability to json obj for this method
    addRunListWidget->outputToJSON(methodObj);

    // Get the human readable text or name to display
    auto methodsAndParamsMap = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsMap();
    auto name = methodsAndParamsMap.value(key,QString());

    if(name.isEmpty())
        name = key;

    QJsonObject finalObj;

    finalObj[key] = methodObj;

    finalObj["Key"] = key;
    finalObj["ModelName"] = name;
    finalObj["VarTypes"] = typesObj;

    finalObj["Uuids"] = uuidObj;

    addRunListWidget->outputToJSON(finalObj);

    // qDebug()<<finalObj;

    listWidget->addItem(finalObj);
}


void SimCenterJsonWidget::handleItemRemoved(QJsonObject removedObj)
{
    auto uuids = removedObj.value("Uuids").toObject();

    if(uuids.isEmpty())
    {
        this->errorMessage("Error, the object uuids are empty");
        return;
    }

    for(auto&& it : uuids)
    {
        auto res = theInputParamsWidget->removeInputParameter(it.toString());

        if(!res)
        {
            this->errorMessage("Error removing a random variable in "+theInputParamsWidget->objectName());
            return;
        }
    }

}


bool SimCenterJsonWidget::outputToJSON(QJsonObject &jsonObj)
{
    QJsonObject outputObj;

    auto methodsObj = listWidget->getMethods();

    if(methodsObj.empty())
    {
        outputObj.insert("ToInclude", false);
        jsonObj.insert(methodKey,outputObj);
        return true;
    }

    outputObj.insert("ToInclude", true);

    outputObj["Method"] = methodsObj;

    jsonObj.insert(methodKey,outputObj);

    return true;
}


bool SimCenterJsonWidget::inputFromJSON(QJsonObject &jsonObject)
{
    auto methodsObj = jsonObject["Method"].toObject();

    auto methodsAndParamsMap = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsMap();

    auto keys = methodsObj.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        auto key = keys.at(i);

        auto name = methodsAndParamsMap.value(key);

        if(name.isEmpty())
            name = key;

        auto methodObj = methodsObj.value(key).toObject();

        QJsonObject finalObj;

        finalObj[key] = methodObj;

        finalObj["Key"] = key;
        finalObj["ModelName"] = name;
        finalObj["ModelWeight"] = methodObj.value("ModelWeight").toDouble();

        listWidget->addItem(finalObj);
    }

    // Select the last row
    auto numItems = listWidget->getNumberOfItems();
    listWidget->selectRow(numItems-1);

    return true;
}


void SimCenterJsonWidget::clear()
{
    methodWidget->reset();
    listWidget->clear();
    addRunListWidget->clear();
}


void SimCenterJsonWidget::handleListItemSelected(const QModelIndex& index)
{
    auto treeItem = listWidget->getItem(index);

    if(treeItem == nullptr)
        return;

    auto itemID = treeItem->getItemID();

    auto itemObj = listWidget->getItemJsonObject(itemID);

    QJsonObject methodObj;

    auto itemKey = itemObj.value("Key").toString();

    methodObj[itemKey] = itemObj[itemKey];

    auto modelListObj = methodObj.value(itemKey).toObject();

    addRunListWidget->inputFromJSON(modelListObj);

    QJsonObject finalObj;
    finalObj["Method"] = methodObj;

    // qDebug()<<"****";
    // qDebug()<<finalObj;

    auto res = methodWidget->inputFromJSON(finalObj);

    if(res == false)
    {
        this->errorMessage("Error importing from Json in " + this->objectName());
        return;
    }
}


int SimCenterJsonWidget::getVarTypes(const QJsonObject& vars, const QJsonObject& origObj, const QString& key, QJsonObject& varTypes)
{

    auto paramsObj = origObj["Params"]["Method"]["Options"][key]["Params"].toObject();

    if(paramsObj.isEmpty())
    {
        this->errorMessage("Error, could not get the parameter types, the parameter object is empty in the original json object");
        return -1;
    }

    auto varNames = vars.keys();
    foreach(const QString &key, varNames)
    {
        auto thisParamObj = paramsObj.value(key).toObject();

        if(thisParamObj.isEmpty())
        {
            this->errorMessage("Error, could not find the parameter "+key+" in the original json object");
            return -1;
        }

        auto type = thisParamObj["VarType"].toString();

        if(type.isEmpty())
        {
            this->errorMessage("Error, could not find the variable type, i.e., VarType, for the parameter "+key);
            return -1;
        }

        varTypes.insert(key,type);
    }

    return 0;
}
