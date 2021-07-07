#include "DVRepairRateWidget.h"
#include "CustomListWidget.h"
#include "TreeItem.h"
#include "WorkflowAppOpenSRA.h"
#include "JsonDefinedWidget.h"
#include "JsonGroupBoxWidget.h"
#include "JsonWidget.h"
#include "WidgetFactory.h"

#include <QCheckBox>
#include <QSplitter>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>


DVRepairRateWidget::DVRepairRateWidget(QWidget* parent) : SimCenterAppWidget(parent)
{
    this->setObjectName("RepairRate");

    QSplitter *splitter = new QSplitter(this);

    listWidget = new CustomListWidget("List of Cases to Run", this);

    connect(listWidget,&QAbstractItemView::clicked,this,&DVRepairRateWidget::handleListItemSelected);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    auto mainWidget = this->getWidgetBox();

    if(mainWidget)
        splitter->addWidget(mainWidget);

    splitter->addWidget(listWidget);

    mainLayout->addWidget(splitter);

}


QGroupBox* DVRepairRateWidget::getWidgetBox(void)
{
    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams["DecisionVariable"].toObject()[this->objectName()].toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in DVRepairRateWidget");
        return nullptr;
    }

    QGroupBox* groupBox = new QGroupBox(this->objectName());
    groupBox->setContentsMargins(0,0,0,0);
    groupBox->setFlat(true);

    auto smallVSpacer = new QSpacerItem(0,20);

    QDoubleValidator* validator = new QDoubleValidator(this);
    validator->setRange(0.0,1.0,5);

    auto weightLabel = new QLabel("Model Weight:");
    weightLineEdit = new QLineEdit(this);
    weightLineEdit->setText("1.0");
    weightLineEdit->setValidator(validator);
    weightLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    QPushButton *addRunListButton = new QPushButton(this);
    addRunListButton->setText(tr("Add run to list"));
    addRunListButton->setMinimumWidth(250);

    connect(addRunListButton,&QPushButton::clicked, this, &DVRepairRateWidget::handleAddButtonPressed);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    QHBoxLayout* weightLayout = new QHBoxLayout();
    weightLayout->setMargin(0);
    weightLayout->addWidget(weightLabel,Qt::AlignLeft);
    weightLayout->addWidget(weightLineEdit);
    weightLayout->setStretch(0,0);
    weightLayout->setStretch(1,1);

    // Remove the demand and component properties widget before creating the main widget, this will be treated as a special case
    auto paramObj = thisObj.value("Params").toObject();

    paramObj.remove("ComponentProperties");
    paramObj.remove("Demand");

    thisObj["Params"] = paramObj;

    methodWidget = new JsonDefinedWidget(this, thisObj, this->objectName());
    methodWidget->setObjectName("MethodWidget");
    demandWidget = this->getDemandWidget();
    componentPropertiesWidget = this->getComponentPropertiesWidget();
    componentPropertiesWidget->setObjectName("ComponentProperties");

    QVBoxLayout* mainLayout = new QVBoxLayout(groupBox);

    QVBoxLayout* methodLayout = new QVBoxLayout();
    methodLayout->setMargin(0);
    methodLayout->addWidget(methodWidget);
    methodLayout->addWidget(demandWidget);
    methodLayout->addStretch();

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setMargin(0);
    inputLayout->addLayout(methodLayout);
    inputLayout->addWidget(componentPropertiesWidget);
    inputLayout->setStretch(0,1);
    inputLayout->setStretch(1,1);

    mainLayout->addLayout(inputLayout);

    mainLayout->addItem(smallVSpacer);

    mainLayout->addLayout(weightLayout);

    mainLayout->addWidget(addRunListButton,Qt::AlignCenter);
    mainLayout->addItem(vspacer);

    return groupBox;
}


void DVRepairRateWidget::handleAddButtonPressed(void)
{

    // Get the human or name to display
    auto methodsAndParamsMap = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsMap();

    double weight = weightLineEdit->text().toDouble();

    QJsonObject methodObj;
    methodWidget->outputToJSON(methodObj);

    if(methodObj.isEmpty())
    {
        this->errorMessage("Error in getting the method in DVRepairRateWidget");
        return;
    }

    // Add the yield acceleration parameters
    QJsonObject compPropObj;
    componentPropertiesWidget->outputToJSON(compPropObj);

    if(compPropObj.isEmpty())
    {
        this->errorMessage("Error in getting the component properties in DVRepairRateWidget");
        return;
    }

    // First get the type of demand
    QJsonObject typeObj;
    demandWidget->outputToJSON(typeObj);

    auto keys = typeObj.keys();

    for(int i = 0; i<keys.size(); ++i)
    {
        QJsonObject type;

        auto key = keys.at(i);
        auto val = typeObj.value(key).toBool();

        type["ToInclude"] = val;

        if(val == true)
        {
            QJsonObject typeMethodObj;

            typeMethodObj["ModelWeight"] = weight;

            QJsonObject::const_iterator prop;

            for (prop = compPropObj.begin(); prop != compPropObj.end(); ++prop)
            {
                auto propKey = prop.key();
                auto propObj = prop.value().toObject();

                if(!propObj.isEmpty())
                    typeMethodObj[propKey] = propObj;
                else
                    typeMethodObj[propKey] =  prop.value().toString();

            }

            auto methodType= methodObj.value("Method").toString();

            QJsonObject finalMethodObj;
            finalMethodObj[methodType] = typeMethodObj;

            type["Method"] = finalMethodObj;

            auto textType = methodsAndParamsMap.value(key, QString());
            auto typeTreeItem = listWidget->addItem(textType);

            QJsonObject finalObj;

            finalObj[key] = type;

            finalObj["Key"] = key;

            finalObj["MethodType"] = methodType;

            finalObj["ModelWeight"] = weight;

            auto methodName = methodsAndParamsMap.value(methodType, QString());
            finalObj["ModelName"] = methodName;

            listWidget->addItem(finalObj, typeTreeItem);
        }
    }
}


bool DVRepairRateWidget::outputToJSON(QJsonObject &jsonObj)
{

    auto modelsMap = listWidget->getModelsMap();

    if(modelsMap.empty())
    {
        return false;
    }

    for(auto&& e : modelsMap)
    {
        auto key = e["Key"].toString();

        auto existingTypeObj = jsonObj.value(key).toObject();

        if(existingTypeObj.isEmpty())
        {
            QJsonObject typeObj = e.value(key).toObject();

            jsonObj.insert(key, typeObj);
        }
        else
        {
            auto methodType = e.value("MethodType").toString();

            QJsonObject methodObj = e.value(key).toObject().value("Method").toObject().value(methodType).toObject();

            auto typeObj = jsonObj[key].toObject()["Method"].toObject();
            typeObj.insert(methodType, methodObj);

            existingTypeObj.insert("Method", typeObj);

            jsonObj.insert(key, existingTypeObj);
        }

    }

    return true;
}


bool DVRepairRateWidget::inputFromJSON(QJsonObject &jsonObject)
{

    auto methodsAndParamsMap = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsMap();

    auto keys = jsonObject.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        QJsonObject type;

        auto key = keys.at(i);

        auto name = methodsAndParamsMap[key];

        auto typesObj = jsonObject.value(key).toObject();

        if(name.isEmpty())
            name = key;

        type["ToInclude"] = typesObj.value("ToInclude").toBool();

        auto methodObj = typesObj.value("Method").toObject();

        auto methodKeys = methodObj.keys();

        for(int j = 0; j < methodKeys.size(); ++j)
        {

            auto methodType = methodKeys.at(j);

            auto typeMethodObj = methodObj.value(methodType).toObject();

            auto weight = typeMethodObj.value("ModelWeight").toDouble();

            auto textType = methodsAndParamsMap.value(key, QString());
            auto typeTreeItem = listWidget->addItem(textType);

            QJsonObject finalMethodObj;
            finalMethodObj[methodType] = typeMethodObj;

            type["Method"] = finalMethodObj;

            QJsonObject finalObj;

            finalObj[key] = type;

            finalObj["Key"] = key;

            finalObj["MethodType"] = methodType;

            finalObj["ModelWeight"] = weight;

            auto methodName = methodsAndParamsMap.value(methodType, QString());
            finalObj["ModelName"] = methodName;

            listWidget->addItem(finalObj, typeTreeItem);
        }
    }

    listWidget->expandRecursively(listWidget->rootIndex());

    // Select the last row
    auto numItems = listWidget->getNumberOfItems();
    listWidget->selectRow(numItems-1);

    return true;
}


void DVRepairRateWidget::clear()
{
    listWidget->clear();
    weightLineEdit->clear();
}


JsonGroupBoxWidget* DVRepairRateWidget::getDemandWidget()
{
    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams["DecisionVariable"].toObject()[this->objectName()].toObject()["Params"].toObject()["Demand"].toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in DVRepairRateWidget");
        return nullptr;
    }

    auto theWidgetFactory = std::make_unique<WidgetFactory>(this);

    QJsonObject paramsObj = thisObj["Params"].toObject();

    // The string given in the Methods and params json file
    QString nameStr = "Demand";

    auto widgetLabelText = thisObj["NameToDisplay"].toString();

    if(widgetLabelText.isEmpty())
    {
        this->errorMessage("Could not find the *NameToDisplay* key in object json for " + nameStr);
        return nullptr;
    }

    JsonGroupBoxWidget* demandWidget = new JsonGroupBoxWidget(this);
    demandWidget->setObjectName(nameStr);

    demandWidget->setTitle(widgetLabelText);

    auto propInputLayout = theWidgetFactory->getLayoutFromParams(paramsObj,nameStr,demandWidget);

    QVBoxLayout* widgetLayout = new QVBoxLayout(demandWidget);
    widgetLayout->addLayout(propInputLayout);

    return demandWidget;
}


JsonGroupBoxWidget* DVRepairRateWidget::getComponentPropertiesWidget()
{

    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams["DecisionVariable"].toObject()[this->objectName()].toObject()["Params"].toObject()["ComponentProperties"].toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in DVRepairRateWidget");
        return nullptr;
    }

    auto theWidgetFactory = std::make_unique<WidgetFactory>(this);

    QJsonObject paramsObj = thisObj["Params"].toObject();

    // The string given in the Methods and params json file
    QString nameStr = "ComponentProperties";

    auto widgetLabelText = thisObj["NameToDisplay"].toString();

    if(widgetLabelText.isEmpty())
    {
        this->errorMessage("Could not find the *NameToDisplay* key in object json for " + nameStr);
        return nullptr;
    }

    JsonGroupBoxWidget* componentPropertiesWidget = new JsonGroupBoxWidget(this);
    componentPropertiesWidget->setObjectName(nameStr);

    componentPropertiesWidget->setTitle(widgetLabelText);

    auto propInputLayout = theWidgetFactory->getLayoutFromParams(paramsObj,nameStr,componentPropertiesWidget);

    QVBoxLayout* widgetLayout = new QVBoxLayout(componentPropertiesWidget);
    widgetLayout->addLayout(propInputLayout);

    return componentPropertiesWidget;
}


void DVRepairRateWidget::handleListItemSelected(const QModelIndex& index)
{
    methodWidget->reset();
    demandWidget->reset();
    componentPropertiesWidget->reset();

    auto treeItem = listWidget->getItem(index);

    if(treeItem == nullptr)
        return;

    // Only load if there is a model
    auto count = treeItem->childCount();
    if(count != 0)
        return;

    auto itemID = treeItem->getItemID();

    auto itemObj = listWidget->getItemJsonObject(itemID);

    auto methodType = itemObj.value("MethodType").toString();

    auto itemKey = itemObj.value("Key").toString();

    QJsonObject typeObj = itemObj.value(itemKey).toObject();

    QJsonObject methodObj = typeObj.value("Method").toObject().value(methodType).toObject();

    auto modelWeight = methodObj.value("ModelWeight").toDouble();

    weightLineEdit->setText(QString::number(modelWeight));

    auto res = methodWidget->inputFromJSON(typeObj);

    if(res == false)
    {
        this->errorMessage("Error importing from Json in " + QString(__PRETTY_FUNCTION__));
        return;
    }

    QJsonObject typeInputObj;
    typeInputObj[itemKey] = typeObj.value("ToInclude").toBool();

    QJsonObject demandObject;
    demandObject["Demand"] = typeInputObj;

    res = demandWidget->inputFromJSON(demandObject);

    if(res == false)
    {
        this->errorMessage("Error importing from Json in " + QString(__PRETTY_FUNCTION__));
        return;
    }


    QJsonObject componentObject;
    componentObject["ComponentProperties"] = methodObj;

    res = componentPropertiesWidget->inputFromJSON(componentObject);

    if(res == false)
    {
        this->errorMessage("Error importing from Json in " + QString(__PRETTY_FUNCTION__));
        return;
    }

}

