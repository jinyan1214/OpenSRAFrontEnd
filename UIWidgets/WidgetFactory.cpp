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

#include "WidgetFactory.h"

#include "JsonDefinedWidget.h"
#include "JsonStackedWidget.h"
#include "JsonCheckBox.h"
#include "JsonComboBox.h"
#include "JsonLineEdit.h"
#include "JsonLabel.h"
#include "JsonWidget.h"
#include "JsonGroupBoxWidget.h"
#include "AssetInputWidget.h"
#include "LineAssetInputWidget.h"
#include "WorkflowAppOpenSRA.h"
#include "GenericModelWidget.h"
#include "RandomVariablesWidget.h"

#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonArray>
#include <QString>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>

WidgetFactory::WidgetFactory(AssetInputWidget *parent) : SimCenterWidget(parent), parentInputWidget(parent)
{
    this->setObjectName("NULL");

}


QLayout * WidgetFactory::getLayout(const QJsonObject& obj, const QString& parentKey, QWidget* parent, QStringList widgetOrder)
{
    return this->getLayoutFromParams(obj, parentKey, parent, Qt::Vertical, widgetOrder);
}


QWidget* WidgetFactory::getWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent)
{
    auto widgetType = obj["WidgetType"].toString();

    if(widgetType.isNull())
    {
        this->errorMessage("No support for null widget type for "+ parentKey);
        return nullptr;
    }

    if(widgetType.compare("QComboBox") == 0)
        return getComboBoxWidget(obj,parentKey,parent);
    else if(widgetType.compare("QLineEdit") == 0)
        return getLineEditWidget(obj,parentKey,parent);
    else if(widgetType.compare("QCheckBox") == 0)
        return getCheckBoxWidget(obj,parentKey,parent);
    else if(widgetType.compare("QLabel") == 0)
        return getLabelWidget(obj,parentKey,parent);
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

    if(theRVWidget == nullptr)
        theRVWidget = WorkflowAppOpenSRA::getInstance()->getTheRandomVariableWidget();

    JsonWidget* mainWidget = new JsonWidget(parent);
    mainWidget->setContentsMargins(0,0,0,0);
    mainWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    mainWidget->setObjectName(parentKey);

    QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(1);

    JsonComboBox* comboWidget = new JsonComboBox(mainWidget);
    comboWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    comboWidget->setObjectName(parentKey);
    comboWidget->setMethodAndParamJsonObj(obj);
    mainLayout->addWidget(comboWidget);

    JsonStackedWidget* comboStackedWidget = new JsonStackedWidget(mainWidget);
    comboStackedWidget->setObjectName("NULL");
    comboStackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    comboStackedWidget->setContentsMargins(50,0,0,0);
    comboWidget->setStackedWidget(comboStackedWidget);

    mainLayout->addWidget(comboStackedWidget);
    connect(comboWidget,SIGNAL(currentIndexChanged(int)),comboStackedWidget,SLOT(setCurrentIndex(int)));

    auto listKeys = comboItems.keys();

    // Special case where the combo box is the column headers
    if(listKeys.contains("ColumnHeaders"))
    {
        connect(parentInputWidget, &AssetInputWidget::headingValuesChanged, comboWidget, &JsonComboBox::updateComboBoxValues);
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

            if(!params.isEmpty() && key.compare("GenericModel") != 0)
            {
                JsonDefinedWidget* newWidget = new JsonDefinedWidget(mainWidget, itemObj, key);
                newWidget->setObjectName(key);
                newWidget->setMethodAndParamJsonObj(itemObj);
                comboStackedWidget->addWidget(newWidget);
            }
            else
            {
                // Check for the special case if this is a user-defined model
                if(key.compare("GenericModel") == 0)
                {
                    auto parentName = parent->objectName();

                    auto genModelWidget = new GenericModelWidget(parentName, itemObj);
                    genModelWidget->setMethodAndParamJsonObj(itemObj);
                    comboStackedWidget->addWidget(genModelWidget);
                }
                else
                {
                    auto emptyWidget = new QWidget();
                    emptyWidget->setContentsMargins(0,0,0,0);
                    emptyWidget->setObjectName("NULL");
                    comboStackedWidget->addWidget(emptyWidget);
                }
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

    if(!obj["DefaultValue"].isNull() && !defValue.isEmpty())
    {
        int index = comboWidget->findData(defValue);
        if (index != -1)
        {
            comboWidget->setCurrentIndex(index);
            comboWidget->setDefaultIndex(index);
        }
        else
        {
            this->errorMessage("Error, could not find the item " + defValue + " in " + comboWidget->objectName());
        }

        //        for(int i = 0; i < comboWidget->count(); ++i)
        //        {
        //            auto itemStr = comboWidget->itemData(i).toString();
        //            qDebug()<<itemStr;
        //        }
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
    lineEditWidget->setMethodAndParamJsonObj(obj);
    lineEditWidget->setDefaultValue(defVal);

    // for method parameters
    if (obj.contains("DescToDisplay"))
    {
        auto descTxt = "\t" + obj.value("DescToDisplay").toString();
        // not using as intended - proxy attribute to store description text
        lineEditWidget->setToolTip(descTxt);
    }

    return lineEditWidget;
}


QWidget* WidgetFactory::getLabelWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent)
{

    if(obj.value("ToDisplay").toBool() == false)
        return nullptr;

    auto defVal = "\t" + obj.value("DescToDisplay").toString();

    JsonLabel* labelWidget = new JsonLabel(parent);
    labelWidget->setObjectName(parentKey);
    labelWidget->setMethodAndParamJsonObj(obj);
    labelWidget->setDefaultValue(defVal);

    return labelWidget;
}


QWidget* WidgetFactory::getCheckBoxWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent)
{
    if(obj["ToDisplay"].toBool() == false)
        return nullptr;

    JsonWidget* mainWidget = new JsonWidget(parent);
    mainWidget->setContentsMargins(5,0,0,0);
    mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainWidget->setObjectName("NULL");

    JsonCheckBox* checkBoxWidget = new JsonCheckBox(parent);
    checkBoxWidget->setMainWidget(mainWidget);
    checkBoxWidget->setObjectName(parentKey);
    checkBoxWidget->setMethodAndParamJsonObj(obj);

    QHBoxLayout* mainLayout = new QHBoxLayout(mainWidget);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(4);

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


QLayout* WidgetFactory::getLayoutFromParams(const QJsonObject& params, const QString& parentKey, QWidget* parent, Qt::Orientation orientation, QStringList widgetOrder)
{
    QBoxLayout* mainLayout;

    if(orientation == Qt::Vertical)
        mainLayout = new QVBoxLayout();
    else
        mainLayout = new QHBoxLayout();

    mainLayout->setMargin(4);
    mainLayout->setSpacing(2);

    // This will create the widgets in a certain order
    if(!widgetOrder.isEmpty())
    {
        for(auto&& key : widgetOrder)
        {
            auto paramObj = params.value(key).toObject();

            auto res = addWidgetToLayout(paramObj,key,parent,mainLayout);

            if(!res)
            {
                this->errorMessage("Could not create the widget " + parentKey);
                break;
            }
        }
    }
    else
    {
        QJsonObject::const_iterator param;
        for (param = params.begin(); param != params.end(); ++param)
        {
            auto key = param.key();

            auto paramObj = param.value().toObject();

            if(paramObj["ToDisplay"].toBool() == false)
                continue;

            auto res = addWidgetToLayout(paramObj,key,parent,mainLayout);

            if(!res)
            {
                this->errorMessage("Could not create the widget " + parentKey);
                break;
            }
        }
    }

    if(orientation == Qt::Vertical)
        mainLayout->addStretch();

    return mainLayout;
}


bool WidgetFactory::addWidgetToLayout(const QJsonObject& paramObj, const QString& key, QWidget* parent, QBoxLayout* mainLayout)
{
    auto widget = this->getWidget(paramObj, key, parent);

    if(widget == nullptr)
        return false;

    widget->setObjectName(key);

    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    auto widgetLabelText = paramObj.value("NameToDisplay").toString();

    if(widgetLabelText.isEmpty())
    {
        this->errorMessage("Could not find the *NameToDisplay* key in object json for " + key);
        return false;
    }

    auto widgetType = paramObj.value("WidgetType").toString();

    if(isNestedComboBoxWidget(paramObj))
    {
        QLabel* widgetLabel = new QLabel(widgetLabelText,this->parentWidget());

        mainLayout->addWidget(widgetLabel,Qt::AlignTop);
        mainLayout->addWidget(widget);
    }
    else if(widgetType.compare("QWidget") == 0 || widgetType.compare("QCheckBox") == 0)
    {
        mainLayout->addWidget(widget);
    }
    else if(widgetType.compare("QLabel") == 0)
    {
        auto numThings = mainLayout->count();

        QLabel* widgetLabel = new QLabel(QString::number(numThings+1)+".  "+widgetLabelText+":",this->parentWidget());

        widgetLabel->setStyleSheet("font-weight: bold; color: black");

        QGridLayout* newHLayout = new QGridLayout();
        newHLayout->setMargin(0);
        newHLayout->setSpacing(4);

        newHLayout->addWidget(widgetLabel,0,0);
        newHLayout->addWidget(widget,1,0);

        mainLayout->addLayout(newHLayout);
    }
    else
    {
        // for method params only
        if (paramObj.contains("DescToDisplay"))
        {
            auto numThings = mainLayout->count();
            QLabel* widgetLabel = new QLabel(QString::number(numThings+1)+".  "+widgetLabelText+":",this->parentWidget());

            widgetLabel->setStyleSheet("font-weight: bold; color: black");

            QVBoxLayout* newVLayout = new QVBoxLayout();

            QGridLayout* newHLayout = new QGridLayout();
            newHLayout->setMargin(0);
            newHLayout->setSpacing(4);

            newHLayout->addWidget(widgetLabel,0,0);
            widget->setMaximumWidth(200);
            newHLayout->addWidget(widget,0,1);
            newHLayout->setColumnStretch(2,1);

            // proxy way to get description text for line edit
            QLabel* widgetDesc = new QLabel(widget->toolTip());

            newVLayout->addLayout(newHLayout);
            newVLayout->addWidget(widgetDesc);

            mainLayout->addLayout(newVLayout);
        }
        else
        {
            QLabel* widgetLabel = new QLabel(widgetLabelText+":",this->parentWidget());

            widgetLabel->setStyleSheet("font-weight: bold; color: black");

            QGridLayout* newHLayout = new QGridLayout();
            newHLayout->setMargin(0);
            newHLayout->setSpacing(4);

            newHLayout->addWidget(widgetLabel,0,0);
            newHLayout->addWidget(widget,0,1);

            mainLayout->addLayout(newHLayout);
        }
    }

    return true;
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
    newWidget->setMethodAndParamJsonObj(obj);

    auto text = obj.value("NameToDisplay").toString();

    newWidget->setTitle(text);

    return newWidget;
}
