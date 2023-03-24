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

#include "EDPLandslideWidget.h"
#include "CustomListWidget.h"
#include "TreeItem.h"
#include "WorkflowAppOpenSRA.h"
#include "JsonDefinedWidget.h"
#include "JsonWidget.h"
#include "WidgetFactory.h"
#include "AddToRunListWidget.h"

#include <QCheckBox>
#include <QSplitter>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSettings>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QCoreApplication>


EDPLandslideWidget::EDPLandslideWidget(QJsonObject obj, QWidget* parent) : SimCenterAppWidget(parent)
{
    this->setObjectName("Landslide");

    QSplitter *splitter = new QSplitter(this);
    splitter->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    listWidget = new CustomListWidget("List of Cases to Run", this);

    connect(listWidget,&QAbstractItemView::clicked,this,&EDPLandslideWidget::handleListItemSelected);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(0,0,0,0);

    QWidget* mainWidget = new QWidget();
    mainWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    QVBoxLayout* inputLayout = new QVBoxLayout(mainWidget);
    inputLayout->setMargin(0);
    inputLayout->setContentsMargins(5,0,0,0);

    auto boxWidget = this->getWidgetBox(obj);

    if(boxWidget == nullptr)
    {
        this->errorMessage("Failed to create the main widget in EDPLandslideWidget");
        return;
    }

    boxWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    inputLayout->addWidget(boxWidget);

    //-----
    // widget for additional landslide parameters for deformation polygons to use
    defPolyLineEdit = new QLineEdit();
    QHBoxLayout *defPolyLayout = new QHBoxLayout();
    defPolyLayout->addWidget(defPolyLineEdit);
    QPushButton *defPolyButton = new QPushButton();
    defPolyButton->setText("Browse");
    defPolyButton->setToolTip(tr("Select path to shapefile with landslide deformation polygon"));
    defPolyLayout->addWidget(defPolyButton);


    defPolyCheckBox = new QCheckBox("Use any deformation polygon:");
    defPolyCheckBox->setChecked(false);
    defPolyLineEdit->setEnabled(false);
    defPolyLayout->setEnabled(false);


    inputLayout->addLayout(defPolyLayout);
//    inputLayout->addRow(defPolyCheckBox, defPolyLayout);
//    inputLayout->setAlignment(Qt::AlignLeft);
//    inputLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
//    inputLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);

    // connect the pushbutton with code to open file selection and update python preferences with selected file
//    connect(defPolyButton, &QPushButton::clicked, this, [this](){
//        QSettings settings("SimCenter", QCoreApplication::applicationName()); //These names will need to be constants to be shared
//        QVariant  defPolyPathVariant = settings.value("appDir");
//        QString existingDir = QCoreApplication::applicationDirPath();
//        if (pythonPathVariant.isValid()) {
//            QString existingF = pythonPathVariant.toString();
//        }

//        QString selectedFile = QFileDialog::getOpenFileName(this,
//                                                            tr("Select Python Interpreter"),
//                                                            existingDir,
//                                                            "All files (*.*)");

//        if(!selectedFile.isEmpty()) {
//            customPythonLineEdit->setText(selectedFile);
//        }
//    }
//    );

//    connect(defPolyCheckBox, &QCheckBox::toggled, this, [this, defPolyButton](bool checked)
//    {
//        this->defPolyLineEdit->setEnabled(checked);
//        defPolyButton->setEnabled(checked);
//        defPolyButton->setFlat(!checked);
//        this->customAppDirLineEdit->setText(this->getAppDir());
//    });
    //-----


    // Add the weight and add to run list button at the bottom
    addRunListWidget = new AddToRunListWidget();

//    // Add a vertical spacer at the bottom to push everything up
//    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
//    inputLayout->addItem(vspacer);

    connect(addRunListWidget,&AddToRunListWidget::addToRunListButtonPressed, this, &EDPLandslideWidget::handleAddButtonPressed);
    inputLayout->addWidget(addRunListWidget,Qt::AlignBottom);
    inputLayout->setStretch(0,1);
    inputLayout->setStretch(1,0);

    if(mainWidget)
        splitter->addWidget(mainWidget);

    splitter->addWidget(listWidget);

    auto idx = splitter->indexOf(listWidget);
    splitter->setCollapsible(idx,false);

    mainLayout->addWidget(splitter);
}


QWidget* EDPLandslideWidget::getWidgetBox(QJsonObject& obj)
{    
    if(obj.isEmpty())
    {
        this->errorMessage("Json object is empty in EDPLandslideWidget");
        return nullptr;
    }

    QGroupBox* groupBox = new QGroupBox(this->objectName());
    groupBox->setContentsMargins(0,0,0,0);
    groupBox->setFlat(true);

    // Create a backup of the original object
    auto origObj = obj;

    // Remove the yield acceleration widget before creating the main widget, this will be treated as a special case
    auto paramObj = obj.value("Params").toObject();

    paramObj.remove("YieldAcceleration");

    obj["Params"] = paramObj;

    methodWidget = new JsonDefinedWidget(this, obj, this->objectName());
    methodWidget->setObjectName("MethodWidget");

    QScrollArea *sa = new QScrollArea;
    sa->setWidgetResizable(true);
    sa->setLineWidth(0);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setWidget(groupBox);
    sa->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    yieldAccMethodWidget = this->getYieldMethodWidget(origObj);
    yieldAccParametersWidget = this->getYieldAccWidget(origObj);
    yieldAccParametersWidget->setObjectName("YieldAccWidget");

    QVBoxLayout* mainLayout = new QVBoxLayout(groupBox);

    QWidget* yieldAccWidget = new QWidget();

    QHBoxLayout* yieldAccLayout = new QHBoxLayout(yieldAccWidget);
    yieldAccLayout->setMargin(0);
    yieldAccLayout->addWidget(yieldAccMethodWidget);
    yieldAccLayout->addWidget(yieldAccParametersWidget);
    yieldAccLayout->setStretch(0,1);
    yieldAccLayout->setStretch(1,1);

    QVBoxLayout* inputLayout = new QVBoxLayout();
    inputLayout->setMargin(0);
    inputLayout->addWidget(methodWidget);
    inputLayout->addWidget(yieldAccWidget);
//    inputLayout->setStretch(0,1);
//    inputLayout->setStretch(1,1);

    mainLayout->addLayout(inputLayout);

    return sa;
}


void EDPLandslideWidget::handleAddButtonPressed(void)
{
    QJsonObject methodObj;
    methodWidget->outputToJSON(methodObj);

    methodObj = methodObj.value("Method").toObject();

    if(methodObj.isEmpty())
    {
        this->errorMessage("Error in getting the method in EDPLandslideWidget");
        return;
    }

    auto keys = methodObj.keys();

    if(keys.size() != 1)
    {
        this->errorMessage("Error in getting the method in EDPLandslideWidget");
        return;
    }

    auto key = keys.front();

    methodObj = methodObj.value(key).toObject();

    // Add the model weight, epistemic uncertainty, aleatory variability to json obj
    addRunListWidget->outputToJSON(methodObj);

    // Add the epistemic variabli

    // Add the yield acceleration method
    yieldAccMethodWidget->outputToJSON(methodObj);

    // Add the yield acceleration parameters
    yieldAccParametersWidget->outputToJSON(methodObj);

    // Get the human or name to display
    auto methodsAndParamsMap = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsMap();
    auto name = methodsAndParamsMap.value(key,QString());

    if(name.isEmpty())
        name = key;

    QJsonObject finalObj;

    finalObj[key] = methodObj;

    finalObj["Key"] = key;
    finalObj["ModelName"] = name;

    addRunListWidget->outputToJSON(finalObj);

    // qDebug()<<finalObj;

    listWidget->addItem(finalObj);
}


bool EDPLandslideWidget::outputToJSON(QJsonObject &jsonObj)
{
    QJsonObject outputObj;

    auto methodsObj = listWidget->getMethods();

    if(methodsObj.empty())
    {
        outputObj.insert("ToInclude", false);
        jsonObj.insert(this->objectName(),outputObj);
        return true;
    }

    outputObj.insert("ToInclude", true);

    outputObj["Method"] = methodsObj;

    jsonObj.insert(this->objectName(),outputObj);

    return true;
}


bool EDPLandslideWidget::inputFromJSON(QJsonObject &jsonObject)
{
    auto methodsObj = jsonObject["Method"].toObject();

    auto methodsAndParamsMap = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsMap();

    auto keys = methodsObj.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        auto key = keys.at(i);

        auto name = methodsAndParamsMap[key];

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


void EDPLandslideWidget::clear()
{
    methodWidget->reset();
    yieldAccParametersWidget->reset();
    yieldAccMethodWidget->reset();
    listWidget->clear();
    addRunListWidget->clear();
}


JsonWidget* EDPLandslideWidget::getYieldMethodWidget(const QJsonObject& obj)
{
    QJsonObject thisObj = obj.value("Params").toObject()["YieldAcceleration"].toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in EDPLandslideWidget");
        return nullptr;
    }

    auto theWidgetFactory = WorkflowAppOpenSRA::getInstance()->getTheWidgetFactory();

    QJsonObject paramsObj = thisObj["Params"].toObject();

    // The string given in the Methods and params json file
    QString methodKyStr = "MethodForKy";

    // First create the method for Ky object
    QJsonObject methodKyObj = paramsObj[methodKyStr].toObject();

    auto widgetLabelText = methodKyObj["NameToDisplay"].toString();

    if(widgetLabelText.isEmpty())
    {
        this->errorMessage("Could not find the *NameToDisplay* key in object json for " + methodKyStr);
        return nullptr;
    }

    JsonWidget* yieldMethodWidget = new JsonWidget(this);
    yieldMethodWidget->setObjectName(methodKyStr);

    QLabel* widgetLabel = new QLabel(widgetLabelText, yieldMethodWidget);

    auto kyWidget = theWidgetFactory->getWidget(methodKyObj,methodKyStr,yieldMethodWidget);
    kyWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    kyWidget->setObjectName(methodKyStr);

    QVBoxLayout* kyWidgetLayout = new QVBoxLayout(yieldMethodWidget);
    kyWidgetLayout->setMargin(0);

    kyWidgetLayout->addWidget(widgetLabel);
    kyWidgetLayout->addWidget(kyWidget);

    kyWidgetLayout->addStretch(1);

    return yieldMethodWidget;
}


JsonWidget* EDPLandslideWidget::getYieldAccWidget(const QJsonObject& obj)
{

    QJsonObject thisObj = obj.value("Params").toObject()["YieldAcceleration"].toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in EDPLandslideWidget");
        return nullptr;
    }

    auto theWidgetFactory = WorkflowAppOpenSRA::getInstance()->getTheWidgetFactory();

    QJsonObject paramsObj = thisObj["Params"].toObject();

    // The string given in the Methods and params json file
    QString KyStr = "KyParams";

    paramsObj.remove("MethodForKy");

    auto widgetLabelText = thisObj["NameToDisplay"].toString();

    if(widgetLabelText.isEmpty())
    {
        this->errorMessage("Could not find the *NameToDisplay* key in object json for " + KyStr);
        return nullptr;
    }

    JsonWidget* yieldAccWidget = new JsonWidget(this);
    yieldAccWidget->setObjectName(KyStr);

    QLabel* widgetLabel = new QLabel(widgetLabelText, yieldAccWidget);

    auto dispOrderAray = thisObj["DisplayOrder"].toArray();

    QStringList displayOrder;

    for(auto&& varnt : dispOrderAray.toVariantList())
        displayOrder.append(varnt.toString());

    auto accWidgetInputLayout = theWidgetFactory->getLayoutFromParams(paramsObj,KyStr,yieldAccWidget,Qt::Vertical,displayOrder);

    QVBoxLayout* accWidgetLayout = new QVBoxLayout(yieldAccWidget);
    accWidgetLayout->addWidget(widgetLabel);
    accWidgetLayout->addLayout(accWidgetInputLayout);

    return yieldAccWidget;
}


void EDPLandslideWidget::handleListItemSelected(const QModelIndex& index)
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

    QJsonObject landslideObj;
    landslideObj["Method"] = methodObj;

    //    qDebug()<<"****";
    //    qDebug()<<landslideObj;

    auto res = methodWidget->inputFromJSON(landslideObj);

    if(res == false)
    {
        this->errorMessage("Error importing from Json in " + this->objectName());
        return;
    }

    QJsonObject yieldAccObj = itemObj.value(itemKey).toObject();
    res = yieldAccMethodWidget->inputFromJSON(yieldAccObj);

    if(res == false)
    {
        this->errorMessage("Error importing from Json in " + this->objectName());
        return;
    }

    res = yieldAccParametersWidget->inputFromJSON(yieldAccObj);

    if(res == false)
    {
        this->errorMessage("Error importing from Json in " + this->objectName());
        return;
    }

}
