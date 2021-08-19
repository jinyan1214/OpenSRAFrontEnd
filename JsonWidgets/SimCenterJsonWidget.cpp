#include "SimCenterJsonWidget.h"
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
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>


SimCenterJsonWidget::SimCenterJsonWidget(QString methodName, QString type, QWidget* parent) : SimCenterAppWidget(parent)
{
    this->setObjectName(methodName);

    methodKey = methodName;

    QSplitter *splitter = new QSplitter(this);

    listWidget = new CustomListWidget("List of Cases to Run",this);

    connect(listWidget,&QAbstractItemView::clicked,this,&SimCenterJsonWidget::handleListItemSelected);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    auto mainWidget = this->getWidgetBox(type);

    if(mainWidget)
        splitter->addWidget(mainWidget);

    splitter->addWidget(listWidget);

    mainLayout->addWidget(splitter);
}


QGroupBox* SimCenterJsonWidget::getWidgetBox(const QString& type)
{
    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams.value(type).toObject().value(this->objectName()).toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in SimCenterJsonWidget");
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

    connect(addRunListButton,&QPushButton::clicked, this, &SimCenterJsonWidget::handleAddButtonPressed);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    QHBoxLayout* weightLayout = new QHBoxLayout();
    weightLayout->setMargin(0);
    weightLayout->addWidget(weightLabel,Qt::AlignLeft);
    weightLayout->addWidget(weightLineEdit);
    weightLayout->setStretch(0,0);
    weightLayout->setStretch(1,1);

    methodWidget = new JsonDefinedWidget(this, thisObj, this->objectName());
    methodWidget->setObjectName("MethodWidget");

    QVBoxLayout* mainLayout = new QVBoxLayout(groupBox);

    QVBoxLayout* methodLayout = new QVBoxLayout();
    methodLayout->setMargin(0);
    methodLayout->addWidget(methodWidget);
    methodLayout->addStretch();

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setMargin(0);
    inputLayout->addLayout(methodLayout);
    inputLayout->setStretch(0,1);

    mainLayout->addLayout(inputLayout);

    mainLayout->addStretch(1);

    mainLayout->addItem(smallVSpacer);

    mainLayout->addLayout(weightLayout);

    mainLayout->addWidget(addRunListButton,Qt::AlignCenter);
    mainLayout->addItem(vspacer);

    return groupBox;
}


void SimCenterJsonWidget::handleAddButtonPressed(void)
{
    QJsonObject methodObj;
    methodWidget->outputToJSON(methodObj);

    methodObj = methodObj.value("Method").toObject();

    if(methodObj.isEmpty())
    {
        this->errorMessage("Error in getting the method in SimCenterJsonWidget");
        return;
    }

    auto keys = methodObj.keys();

    if(keys.size() != 1)
    {
        this->errorMessage("Error in getting the method in SimCenterJsonWidget");
        return;
    }

    auto key = keys.front();

    methodObj = methodObj.value(key).toObject();

    // Add the model weight
    double weight = weightLineEdit->text().toDouble();
    methodObj["ModelWeight"] = weight;

    // Get the human or name to display
    auto methodsAndParamsMap = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsMap();
    auto name = methodsAndParamsMap.value(key,QString());

    if(name.isEmpty())
        name = key;

    QJsonObject finalObj;

    finalObj[key] = methodObj;

    finalObj["Key"] = key;
    finalObj["ModelWeight"] = weight;
    finalObj["ModelName"] = name;

    // qDebug()<<finalObj;

    listWidget->addItem(finalObj);
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
    weightLineEdit->clear();
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

    auto modelWeight = methodObj.value(itemKey).toObject().value("ModelWeight").toDouble();

    weightLineEdit->setText(QString::number(modelWeight));

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

