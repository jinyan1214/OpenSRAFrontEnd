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

#include <QScrollArea>
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

    QSplitter *splitter = new QSplitter();
    splitter->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    listWidget = new CustomListWidget("List of Cases to Run");

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

    //    QGroupBox* groupBox = new QGroupBox("test GB Name");
    QGroupBox* groupBox = new QGroupBox(this->objectName());
    groupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    groupBox->setContentsMargins(0,0,0,0);
    groupBox->setFlat(true);

    methodWidget = new JsonDefinedWidget(this, jsonObj, this->objectName());
    methodWidget->setObjectName("MethodWidget");

    QScrollArea *scrollWidget = new QScrollArea;
    scrollWidget->setWidgetResizable(true);
    scrollWidget->setLineWidth(0);
    scrollWidget->setWidget(methodWidget);
    scrollWidget->setFrameShape(QFrame::NoFrame);
    scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* groupBoxLayout = new QVBoxLayout(groupBox);
    groupBoxLayout->setContentsMargins(3,10,0,0);

    // Add the weight and add to run list button at the bottom
    addRunListWidget = new AddToRunListWidget();

    connect(addRunListWidget,&AddToRunListWidget::addToRunListButtonPressed, this, &SimCenterJsonWidget::handleAddButtonPressed);

    groupBoxLayout->addWidget(scrollWidget);

    groupBoxLayout->addWidget(addRunListWidget,Qt::AlignCenter);

    groupBoxLayout->setStretch(0,1);
    groupBoxLayout->setStretch(1,0);

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

    // Object to store the variable types
    QJsonObject typesObj;

    auto key = keys.front();

    // Handle the special case of a user defined model
    if(key.compare("UserdefinedModel") == 0)
    {
        auto paramObj = methodObj.value(key).toObject();

        QJsonObject newMethodObj;

        auto keys = paramObj.keys();

        foreach(auto&& key, keys)
        {
            newMethodObj.insert(key,"User-created generic model parameter");

            typesObj.insert(key,"random");
        }

        methodObj = newMethodObj;
    }
    else
    {
        methodObj = methodObj.value(key).toObject();

        auto res = this->getVarTypes(methodObj,passedObj,key,typesObj);
        if(res != 0)
        {
            this->errorMessage("Error, could not get the var types in SimCenterJsonWidget");
            return;
        }
    }

    // Object to store the uuid's of the parameters, where the key is the parameter name
    QJsonObject uuidObj;

    auto paramsKeys = methodObj.keys();
    for(auto&& it : paramsKeys)
    {
        // If the parameter already exists, get its UID
        bool ok = true;
        auto uid = theInputParamsWidget->checkIfParameterExists(it,ok);

        if(!ok)
        {
            this->errorMessage("Error adding a new parameter in "+theInputParamsWidget->objectName());
            return;
        }

        // If uid is empty, create a new parameter as it does not exist
        if (uid.isEmpty())
        {
            // Create a unique id to identify the specific instance of these parameters
            uid = QUuid::createUuid().toString();
        }

        // Save the parameter uid
        uuidObj[it]=uid;

        // Generate the from model string
        auto fromModel = methodKey+"-"+key;

        // Get the type of variable, e.g., random or constant
        auto varType = typesObj[it].toString();

        if(varType.isEmpty())
        {
            this->errorMessage("Error getting the variable type for "+theInputParamsWidget->objectName());
            return;
        }

        // Get the description of the input variable
        QString desc = methodObj.value(it).toString();

        auto res = theInputParamsWidget->addNewParameter(it,fromModel,desc,uid,varType);

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

    auto key = removedObj["Key"].toString();

    if(key.isEmpty())
    {
        this->errorMessage("Error, the object is empty");
        return;
    }

    auto fromModel = methodKey+"-"+key;

    for(auto&& it : uuids)
    {
        auto res = theInputParamsWidget->removeParameter(it.toString(),fromModel);

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
