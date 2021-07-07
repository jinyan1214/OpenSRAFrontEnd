#include "WidgetFactory.h"
#include "JsonDefinedWidget.h"
#include "JsonStackedWidget.h"
#include "JsonCheckBox.h"
#include "JsonComboBox.h"
#include "JsonLineEdit.h"
#include "JsonWidget.h"
#include "JsonGroupBoxWidget.h"
#include "ComponentInputWidget.h"

#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonArray>
#include <QString>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>

WidgetFactory::WidgetFactory(QWidget *parent) : SimCenterWidget(parent)
{
    this->setObjectName("NULL");
}


QLayout * WidgetFactory::getLayout(const QJsonObject& obj, const QString& parentKey, QWidget* parent)
{
    return this->getLayoutFromParams(obj, parentKey, parent);
}


QWidget* WidgetFactory::getWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent)
{
    auto widgetType = obj["WidgetType"].toString();

    if(widgetType.isNull())
    {
        this->errorMessage("No support for widget type "+ widgetType);
        return nullptr;
    }

    if(widgetType.compare("QComboBox") == 0)
        return getComboBoxWidget(obj,parentKey,parent);
    else if(widgetType.compare("QLineEdit") == 0)
        return getLineEditWidget(obj,parentKey,parent);
    else if(widgetType.compare("QCheckBox") == 0)
        return getCheckBoxWidget(obj,parentKey,parent);
    else if(widgetType.compare("QWidget") == 0)
        return getBoxWidget(obj,parentKey,parent);
    else
        this->errorMessage("No support for widget type "+ widgetType);

    return nullptr;
}


QWidget* WidgetFactory::getComboBoxWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent)
{
    auto comboItems = obj["Options"].toObject();

    if(comboItems.empty() == true || obj["ToDisplay"].toBool() == false)
        return nullptr;

    JsonWidget* mainWidget = new JsonWidget(parent);
    mainWidget->setContentsMargins(0,0,0,0);
    mainWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    mainWidget->setObjectName(parentKey);

    QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    JsonComboBox* comboWidget = new JsonComboBox(mainWidget);
    comboWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    comboWidget->setObjectName(parentKey);
    mainLayout->addWidget(comboWidget);

    JsonStackedWidget* comboStackedWidget = new JsonStackedWidget(mainWidget);
    comboStackedWidget->setObjectName("NULL");
    comboStackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    comboStackedWidget->setContentsMargins(0,0,0,0);
    comboWidget->setStackedWidget(comboStackedWidget);
    mainLayout->addWidget(comboStackedWidget);
    connect(comboWidget,SIGNAL(currentIndexChanged(int)),comboStackedWidget,SLOT(setCurrentIndex(int)));

    auto listKeys = comboItems.keys();

    // Special case where the combo box is the column headers
    if(listKeys.contains("ColumnHeaders"))
    {
        // This is being created as the program loads so header values must be set later
        auto compInputWidget = ComponentInputWidget::getInstance();

        connect(compInputWidget, &ComponentInputWidget::headingValuesChanged, comboWidget, &JsonComboBox::updateComboBoxValues);
    }
    else
    {
        for(int i = 0; i<listKeys.size(); ++i)
        {
            auto key = listKeys[i];

            auto itemObj = comboItems.value(key).toObject();

            auto toDisplay = itemObj["ToDisplay"].toBool();

            if(itemObj.isEmpty() == true || toDisplay == false)
                continue;

            auto itemText = itemObj["NameToDisplay"].toString();

            auto params = itemObj["Params"].toObject();

            if(!params.isEmpty())
            {
                JsonDefinedWidget* newWidget = new JsonDefinedWidget(mainWidget, itemObj, key);
                newWidget->setObjectName(key);
                comboStackedWidget->addWidget(newWidget);
            }
            else
            {
                auto emptyWidget = new QWidget(mainWidget);
                emptyWidget->setContentsMargins(0,0,0,0);
                emptyWidget->setObjectName("NULL");
                comboStackedWidget->addWidget(emptyWidget);
            }

            comboWidget->addItem(itemText, key);
        }
    }

    if(comboStackedWidget->count() != comboWidget->count())
    {
        this->errorMessage("Error inconsistent sizes between combo box and widgets");
        return nullptr;
    }

    auto defValue = obj["DefaultValue"].toString();

    if(!obj["DefaultValue"].isNull())
    {
        int index = comboWidget->findData(defValue);
        if (index != -1)
        {
            comboWidget->setCurrentIndex(index);
            comboWidget->setDefaultIndex(index);
        }
        else
        {
            this->errorMessage("Error, could not find the item " + defValue);
        }
    }

    return mainWidget;
}


QWidget* WidgetFactory::getLineEditWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent)
{

    if(obj.value("ToDisplay").toBool() == false)
        return nullptr;

    auto defVal = obj.value("DefaultValue");

    JsonLineEdit* lineEditWidget = new JsonLineEdit(parent);
    lineEditWidget->setObjectName(parentKey);

    lineEditWidget->setDefaultValue(defVal);

    return lineEditWidget;
}


QWidget* WidgetFactory::getCheckBoxWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent)
{
    if(obj["ToDisplay"].toBool() == false)
        return nullptr;

    JsonWidget* mainWidget = new JsonWidget(parent);
    mainWidget->setContentsMargins(0,0,0,0);
    mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainWidget->setObjectName("NULL");

    JsonCheckBox* checkBoxWidget = new JsonCheckBox(mainWidget);
    checkBoxWidget->setMainWidget(mainWidget);
    checkBoxWidget->setObjectName(parentKey);

    QHBoxLayout* mainLayout = new QHBoxLayout(mainWidget);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    auto text = obj.value("NameToDisplay").toString();

    checkBoxWidget->setText(text);

    mainLayout->addWidget(checkBoxWidget);

    auto params = obj["Params"].toObject();

    auto defValue = obj.value("DefaultValue").toBool(false);

    checkBoxWidget->setChecked(defValue);
    checkBoxWidget->setDefaultValue(defValue);

    if(!params.isEmpty())
    {
        mainLayout->addSpacing(15);
        JsonDefinedWidget* newWidget = new JsonDefinedWidget(checkBoxWidget, obj, parentKey);
        newWidget->setObjectName("CheckBoxSubWidget");
        mainLayout->addWidget(newWidget);

        connect(checkBoxWidget,&QCheckBox::stateChanged,newWidget,&QWidget::setVisible);

        newWidget->setVisible(defValue);
    }

    return mainWidget;
}


QLayout* WidgetFactory::getLayoutFromParams(const QJsonObject& params, const QString& parentKey, QWidget* parent, Qt::Orientation orientation)
{
    QBoxLayout* mainLayout;

    if(orientation == Qt::Vertical)
        mainLayout = new QVBoxLayout();
    else
        mainLayout = new QHBoxLayout();

    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    QJsonObject::const_iterator param;

    for (param = params.begin(); param != params.end(); ++param)
    {
        auto key = param.key();

        auto paramObj = param.value().toObject();
        auto widget = this->getWidget(paramObj, key, parent);

        if(widget != nullptr)
        {
            widget->setObjectName(key);

            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

            auto widgetLabelText = paramObj.value("NameToDisplay").toString();

            if(widgetLabelText.isEmpty())
            {
                this->errorMessage("Could not find the *NameToDisplay* key in object json for " + parentKey);
                continue;
            }


            auto widgetType = paramObj.value("WidgetType").toString();

            if(isNestedComboBoxWidget(paramObj))
            {
                QLabel* widgetLabel = new QLabel(widgetLabelText,this->parentWidget());
                mainLayout->addWidget(widgetLabel);
                mainLayout->addWidget(widget);
            }
            else if(widgetType.compare("QWidget") == 0 || widgetType.compare("QCheckBox") == 0)
            {
                mainLayout->addWidget(widget);
            }
            else
            {
                QLabel* widgetLabel = new QLabel(widgetLabelText,this->parentWidget());
                QGridLayout* newHLayout = new QGridLayout();
                newHLayout->setMargin(0);
                newHLayout->setSpacing(0);

                newHLayout->addWidget(widgetLabel,0,0);
                newHLayout->addWidget(widget,0,1);

                mainLayout->addLayout(newHLayout);
            }
        }
    }

    return mainLayout;
}


bool WidgetFactory::isNestedComboBoxWidget(const QJsonObject& obj)
{
    if(obj.value("WidgetType").toString() != "QComboBox")
        return false;

    auto options = obj.value("Options").toObject();

    QJsonObject::const_iterator option;

    for (option = options.begin(); option != options.end(); ++option)
    {
        auto params = option.value()["Params"].toObject();

        if(params.size() != 0)
            return true;
    }

    return false;
}


QWidget* WidgetFactory::getBoxWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent)
{
    JsonGroupBoxWidget* newWidget = new JsonGroupBoxWidget(parent, obj, parentKey);
    newWidget->setObjectName(parentKey);

    auto text = obj.value("NameToDisplay").toString();

    newWidget->setTitle(text);

    return newWidget;
}
