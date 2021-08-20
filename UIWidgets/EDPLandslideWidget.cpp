#include "EDPLandslideWidget.h"
#include "CustomListWidget.h"
#include "TreeItem.h"
#include "WorkflowAppOpenSRA.h"
#include "JsonDefinedWidget.h"
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


EDPLandslideWidget::EDPLandslideWidget(QWidget* parent) : SimCenterAppWidget(parent)
{
    this->setObjectName("Landslide");

    QSplitter *splitter = new QSplitter(this);

    listWidget = new CustomListWidget("List of Cases to Run", this);

    connect(listWidget,&QAbstractItemView::clicked,this,&EDPLandslideWidget::handleListItemSelected);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    auto mainWidget = this->getWidgetBox();

    if(mainWidget)
        splitter->addWidget(mainWidget);

    splitter->addWidget(listWidget);

    mainLayout->addWidget(splitter);
}


QGroupBox* EDPLandslideWidget::getWidgetBox(void)
{
    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams["EngineeringDemandParameter"].toObject()[this->objectName()].toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in EDPLandslideWidget");
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

    connect(addRunListButton,&QPushButton::clicked, this, &EDPLandslideWidget::handleAddButtonPressed);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    QHBoxLayout* weightLayout = new QHBoxLayout();
    weightLayout->setMargin(0);
    weightLayout->addWidget(weightLabel,Qt::AlignLeft);
    weightLayout->addWidget(weightLineEdit);
    weightLayout->setStretch(0,0);
    weightLayout->setStretch(1,1);

    // Remove the yield acceleration widget before creating the main widget, this will be treated as a special case
    auto paramObj = thisObj.value("Params").toObject();

    paramObj.remove("YieldAcceleration");

    thisObj["Params"] = paramObj;

    methodWidget = new JsonDefinedWidget(this, thisObj, this->objectName());
    methodWidget->setObjectName("MethodWidget");
    yieldAccMethodWidget = this->getYieldMethodWidget();
    yieldAccParametersWidget = this->getYieldAccWidget();
    yieldAccParametersWidget->setObjectName("YieldAccWidget");

    QVBoxLayout* mainLayout = new QVBoxLayout(groupBox);

    QHBoxLayout* yieldAccLayout = new QHBoxLayout();
    yieldAccLayout->setMargin(0);
    yieldAccLayout->addWidget(yieldAccMethodWidget);
    yieldAccLayout->addWidget(yieldAccParametersWidget);
    yieldAccLayout->setStretch(0,1);
    yieldAccLayout->setStretch(1,1);

    QVBoxLayout* inputLayout = new QVBoxLayout();
    inputLayout->setMargin(0);
    inputLayout->addWidget(methodWidget);
    inputLayout->addLayout(yieldAccLayout);
    inputLayout->addStretch();

    mainLayout->addLayout(inputLayout);

    mainLayout->addStretch(1);

    mainLayout->addItem(smallVSpacer);

    mainLayout->addLayout(weightLayout);

    mainLayout->addWidget(addRunListButton,Qt::AlignCenter);
    mainLayout->addItem(vspacer);

    return groupBox;
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

    // Add the model weight
    double weight = weightLineEdit->text().toDouble();
    methodObj["ModelWeight"] = weight;

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
    finalObj["ModelWeight"] = weight;

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
    listWidget->clear();
    weightLineEdit->clear();
}


JsonWidget* EDPLandslideWidget::getYieldMethodWidget()
{
    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams["EngineeringDemandParameter"].toObject()[this->objectName()].toObject()["Params"].toObject()["YieldAcceleration"].toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in EDPLandslideWidget");
        return nullptr;
    }

    auto theWidgetFactory = std::make_unique<WidgetFactory>(this);

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


JsonWidget* EDPLandslideWidget::getYieldAccWidget()
{

    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams["EngineeringDemandParameter"].toObject()[this->objectName()].toObject()["Params"].toObject()["YieldAcceleration"].toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in EDPLandslideWidget");
        return nullptr;
    }

    auto theWidgetFactory = std::make_unique<WidgetFactory>(this);

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

    auto modelWeight = methodObj.value(itemKey).toObject().value("ModelWeight").toDouble();

    weightLineEdit->setText(QString::number(modelWeight));

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

