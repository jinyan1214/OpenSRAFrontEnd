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
#include "GenericModelWidget.h"

#include <QScrollArea>
#include <QCheckBox>
#include <QSplitter>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QFileInfo>
#include <QFileDialog>
#include <QSettings>
#include <QSpacerItem>



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
    //QGroupBox* groupBox = new QGroupBox(this->objectName());
    QGroupBox* groupBox = new QGroupBox(jsonObj.value("NameToDisplay").toString());
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

    //
    //-----
    // widget for additional landslide parameters for deformation polygons to use
    nameToDisplay = jsonObj.value("NameToDisplay").toString();
    if(nameToDisplay == "Landslide")
    {

        QVBoxLayout* inputLayout = new QVBoxLayout();
        inputLayout->setMargin(0);

        // widget for additional landslide parameters for deformation polygons to use
        defPolyLineEdit = new QLineEdit();
        QHBoxLayout *defPolyLayout = new QHBoxLayout();
        defPolyCheckBox = new QCheckBox("Leave as \"CA_LandslideInventory_WGS84\" or browse and locate the shapefile:");
        defPolyCheckBox->setChecked(true);
        defPolyLineEdit->setEnabled(true);
        defPolyLineEdit->setText("CA_LandslideInventory_WGS84");
        defPolyLayout->setEnabled(false);


        QPushButton *defPolyButton = new QPushButton();
        defPolyButton->setText("Browse");
        //defPolyButton->setToolTip(tr("Select path to shapefile with landslide deformation polygon"));

        defPolyLayout->addWidget(defPolyCheckBox);
        defPolyLayout->addWidget(defPolyLineEdit);
        defPolyLayout->addWidget(defPolyButton);
        defPolyLayout->setAlignment(Qt::AlignLeft);

        // connect the pushbutton with code to open file selection and update path to def polygon
        connect(defPolyButton, &QPushButton::clicked, this, [this](){
            QSettings settings("SimCenter", QCoreApplication::applicationName()); //These names will need to be constants to be shared
            QVariant defPolyPathVariant = settings.value("appDir");
            QString existingDir = QCoreApplication::applicationDirPath();
            if (defPolyPathVariant.isValid()) {
                QString existingF = defPolyPathVariant.toString();
                QFileInfo existingFile(existingF);
                if (existingFile.exists())
                    existingDir = existingFile.absolutePath();
            }

            QString selectedFile = QFileDialog::getOpenFileName(this,
                                                                tr("Select path to shapefile with landslide deformation polygon"),
                                                                existingDir,
                                                                "All files (*.*)");

            if(!selectedFile.isEmpty()) {
                defPolyLineEdit->setText(selectedFile);
            }
        }
        );

        connect(defPolyCheckBox, &QCheckBox::toggled, this, [this, defPolyButton](bool checked)
        {
            this->defPolyLineEdit->setEnabled(checked);
            defPolyButton->setEnabled(checked);
            defPolyButton->setFlat(!checked);
//            this->defPolyLineEdit->setText(this->getDefPolyDir());
            this->defPolyLineEdit->setText("CA_LandslideInventory_WGS84");
        });

        auto defPolyLabel = new QLabel("Use a shapefile with deformation polygons (checked = yes)? Default: use the California Landslide Inventory");

        inputLayout->addWidget(defPolyLabel);
        inputLayout->addLayout(defPolyLayout);
        groupBoxLayout->addLayout(inputLayout,Qt::AlignCenter);

    }
    //

    groupBoxLayout->addWidget(addRunListWidget,Qt::AlignCenter);

    groupBoxLayout->setStretch(0,1); // scroll widget
    groupBoxLayout->setStretch(1,0); // def poly widget if landslide or add to list widget
    groupBoxLayout->setStretch(2,0); // add to list widget if landslide or null

    // Add a vertical spacer at the bottom to push everything up
    //    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    //    mainLayout->addItem(vspacer);


    return groupBox;
}


void SimCenterJsonWidget::handleAddButtonPressed(void)
{
    QJsonObject variablesObj;
    methodWidget->outputToJSON(variablesObj);

    // The object that was originally given in the methods and params file
    auto passedObj = methodWidget->getMethodAndParamJsonObj();

    auto isObj = variablesObj.value("Method").toObject();

    QStringList keys;

    if(!isObj.isEmpty())
    {
        variablesObj = isObj;
        keys = variablesObj.keys();
    }
    else
    {
        keys.push_back(variablesObj.value("Method").toString());
    }

    if(variablesObj.isEmpty() || keys.size() != 1)
    {
        this->errorMessage("Error in getting the method in SimCenterJsonWidget");
        return;
    }

    // Object to store the variable types
    QJsonObject variableTypesObj;

    auto key = keys.front();

    // Handle the special case of a user defined model
    if(key.compare("GenericModel") == 0)
    {
        auto paramObj = variablesObj.value(key).toObject();

        QJsonObject newMethodObj;

        auto keys = paramObj.keys();

        foreach(auto&& key, keys)
        {
            newMethodObj.insert(key,"User-created generic model parameter");

            variableTypesObj.insert(key,"random");
        }

        variablesObj = newMethodObj;
    }
    else
    {
        variablesObj = variablesObj.value(key).toObject();

        if(!variablesObj.contains("N/A"))
        {
            auto res = this->getVarTypes(variablesObj,passedObj,key,variableTypesObj);
            if(res != 0)
            {
                this->errorMessage("Error, could not get the var types in SimCenterJsonWidget");
                return;
            }
        }
    }

    // Get the human readable text or name to display
    auto methodsAndParamsMap = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsMap();
    auto name = methodsAndParamsMap.value(key,QString());

    if(name.isEmpty())
        name = key;

    QJsonObject finalObj;


    finalObj["Key"] = key;
    finalObj["ModelName"] = name;


    if(!variablesObj.contains("N/A"))
    {

        // Object to store the uuid's of the parameters, where the key is the parameter name
        QJsonObject uuidObjs;

        auto res2 = this->addNewParametersToInputWidget(variablesObj,variableTypesObj, key, uuidObjs);

        if(!res2)
        {
            this->errorMessage("Error, adding the parameters to the input widget for the model "+name+" in "+methodKey);
            return;
        }

        // Add the model weight, epistemic uncertainty, aleatory variability to json obj for this method
        addRunListWidget->outputToJSON(variablesObj);

        finalObj[key] = variablesObj;

        finalObj["VarTypes"] = variableTypesObj;

        finalObj["Uuids"] = uuidObjs;

        addRunListWidget->outputToJSON(finalObj);
    }
    else
    {
        QJsonObject modObj;

        // Add the model weight, epistemic uncertainty, aleatory variability to json obj for this method
        addRunListWidget->outputToJSON(modObj);

        finalObj[key] = modObj;
    }

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

    // landslide only
    if(this->nameToDisplay == "Landslide")
    {
        QJsonObject defPolyObj;
        defPolyObj["UseDeformationGeometry"] = defPolyCheckBox->isChecked();
        defPolyObj["SourceForDeformationGeometry"] = defPolyLineEdit->text();
        outputObj["OtherParameters"] = defPolyObj;
    }

    jsonObj.insert(methodKey,outputObj);

    return true;
}


bool SimCenterJsonWidget::addNewParametersToInputWidget(const QJsonObject& variablesObj, const QJsonObject& variablesTypeObj, const QString& key, QJsonObject& uuidObj)
{

    auto paramsKeys = variablesObj.keys();
    for(auto&& it : paramsKeys)
    {

        // Get the type of variable, e.g., random or constant
        auto varType = variablesTypeObj[it].toString();

        if(varType.isEmpty())
        {
            this->errorMessage("Error getting the variable type for "+theInputParamsWidget->objectName());
            return false;
        }

        if(varType.compare("UserInput") == 0)
            continue;

        // If the parameter already exists, get its UID
        bool ok = true;
        auto uid = theInputParamsWidget->checkIfParameterExists(it,ok);

        if(!ok)
        {
            this->errorMessage("Error adding a new parameter in "+theInputParamsWidget->objectName());
            return false;
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

        // Get the description of the input variable
        QString desc = variablesObj.value(it).toString();

        auto res = theInputParamsWidget->addNewParameter(it,fromModel,desc,uid,varType);

        if(!res)
        {
            this->errorMessage("Error adding a random variable to "+theInputParamsWidget->objectName());
            return false;
        }
    }

    return true;
}


bool SimCenterJsonWidget::inputFromJSON(QJsonObject &jsonObject)
{

    auto thisName = this->objectName();

    // If to include is false, skip it
    auto toInclude = jsonObject.value("ToInclude").toBool();
    if(!toInclude)
        return true;

    // Get the list of methods or models
    auto methodsObj = jsonObject.value("Method").toObject();
    if(methodsObj.isEmpty())
    {
        this->errorMessage("Error, the 'Method' object is empty in "+methodKey);
        return false;
    }

    auto methodsAndParamsMap = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsMap();

    // Get the original method object passed from the methods and params file, this is where the parameters are stored
    auto passedObj = methodWidget->getMethodAndParamJsonObj();

    if(passedObj.isEmpty())
    {
        this->errorMessage("Error, could not find the json object fromt the methods and params file for  "+thisName);
        return false;
    }

    auto keys = methodsObj.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        auto key = keys.at(i);

        // Check if the name is in the methods and params map
        auto name = methodsAndParamsMap.value(key);

        if(name.isEmpty())
            name = key;

        auto methodObj = methodsObj.value(key).toObject();

        // The final object that will be passed to the list widget
        QJsonObject finalObj;

        finalObj[key] = methodObj;

        finalObj["Key"] = key;
        finalObj["ModelName"] = name;
        finalObj["ModelWeight"] = methodObj.value("ModelWeight").toDouble();
        if (methodObj.contains("Aleatory"))
            finalObj["Aleatory"] = methodObj.value("Aleatory").toString();
        else
            finalObj["Aleatory"] = QString("Preferred");
        if (methodObj.contains("Epistemic"))
            finalObj["Epistemic"] = methodObj.value("Epistemic").toString();
        else
            finalObj["Epistemic"] = QString("Preferred");

        if (name == "GenericModel") {

//            this->wid

//            inputFromJSON(methodObj);
            auto temp = 1;
        }

        else {
            // Get the vars for the key
            auto variablesObj = this->getVars(passedObj,key);
            if(variablesObj.isEmpty())
            {
                this->errorMessage("Error, could not get the variable for method "+name+" in "+this->objectName());
                return false;
            }

            // There could be a model with no input parameters
            if(!variablesObj.contains("N/A"))
            {
                QJsonObject variableTypesObj;
                auto res = this->getVarTypes(variablesObj,passedObj,key,variableTypesObj);
                if(res != 0)
                {
                    this->errorMessage("Error, could not get the variable types for method "+name+" in "+this->objectName());
                    return false;
                }

                finalObj["VarTypes"] = variableTypesObj;

                // Object to store the uuid's of the parameters, where the key is the parameter name
                QJsonObject uuidObjs;

                auto res2 = this->addNewParametersToInputWidget(variablesObj,variableTypesObj, key, uuidObjs);

                if(!res2)
                {
                    this->errorMessage("Error, adding the parameters to the input widget for the model "+name+" in "+methodKey);
                    return false;
                }

                finalObj["Uuids"] = uuidObjs;
            }
        }

        auto item = listWidget->addItem(finalObj);

        if(item == nullptr)
        {
            this->errorMessage("Error, failed to add the model "+name+" in "+methodKey);
            return false;
        }
    }

    // landslide only
    if(this->nameToDisplay == "Landslide")
    {
        if (jsonObject.contains("OtherParameters"))
        {
            QJsonObject landslideParams = jsonObject["OtherParameters"].toObject();
            if (landslideParams.contains("UseDeformationGeometry"))
            {
                auto defPolyCheckState = landslideParams["UseDeformationGeometry"].toBool();
                defPolyCheckBox->setChecked(defPolyCheckState);
            }
            if (landslideParams.contains("SourceForDeformationGeometry"))
            {
                auto defPolyPath = landslideParams["SourceForDeformationGeometry"].toString();
                defPolyLineEdit->setText(defPolyPath);
            }
        }
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

    auto res = addRunListWidget->inputFromJSON(modelListObj);

    if(res == false)
    {
        this->errorMessage("Error in SimCenterJsonWidget::handleListItemSelected " + this->objectName());
        return;
    }

    QJsonObject finalObj;
//    finalObj["Method"] = methodObj;

    finalObj["Method"] = itemKey;

    // qDebug()<<"****";
    // qDebug()<<finalObj;

    res = methodWidget->inputFromJSON(finalObj);

    if(res == false)
    {
        this->errorMessage("Error in SimCenterJsonWidget::handleListItemSelected " + this->objectName());
        return;
    }
}




QJsonObject SimCenterJsonWidget::getVars(const QJsonObject& origObj, const QString& key)
{

    // Get the variables
    QJsonObject variablesObj = origObj["Params"]["Method"]["Options"][key]["Params"].toObject();

    if(variablesObj.isEmpty())
    {
        this->errorMessage("Error, could not get the variables for method "+key+" in "+this->objectName());
        return QJsonObject();
    }

    return variablesObj;

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
