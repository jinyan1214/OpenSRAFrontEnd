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

#include "JsonCheckBox.h"
#include "JsonWidget.h"

JsonCheckBox::JsonCheckBox(QWidget* parent) : QCheckBox(parent)
{
    defaultValue = false;
    mainWidget = nullptr;
    subWidget = nullptr;
}


bool JsonCheckBox::outputToJSON(QJsonObject &jsonObject)
{
    // auto key = this->objectName();

//    QJsonObject outputObj;
//    if(subWidget != nullptr)
//        subWidget->outputToJSON(outputObj);

//    if(outputObj.isEmpty())
//    {
        jsonObject["ToInclude"] = this->isChecked();
//    }
//    else
//    {
//        jsonObject[key] = outputObj;
//    }

    return true;
}


bool JsonCheckBox::inputFromJSON(QJsonObject &jsonObject)
{
    auto childWidgetList = mainWidget->findChildren<QWidget*>(QRegularExpression(), Qt::FindDirectChildrenOnly);
    QStringList childList;
    for(auto&& child : childWidgetList)
        childList<<child->objectName();

    QString name = this->objectName();

    auto data = jsonObject.value(name).toObject();

    if(!data.isEmpty())
    {

        bool checkState = data.value("ToInclude").toBool();

        this->setChecked(checkState);

        if(checkState == false)
            return true;

        // Find the child and send it the data - note that when you add widget to a stacked widget or layout, ownership of that widget is passed to the stacked widget or parent of the layout
        auto child = mainWidget->findChild<QWidget*>("CheckBoxSubWidget", Qt::FindDirectChildrenOnly);

        // Cast to a json object
        auto asJson = dynamic_cast<JsonSerializable*>(child);
        if(asJson != nullptr)
        {
            auto res = asJson->inputFromJSON(data);

            if(res == false)
                return res;
        }


        //        auto keys = data.keys();

        //        for(auto&& key : keys)
        //        {
        //            // Find the child and send it the data - note that when you add widget to a stacked widget, ownership of that widget is passed to the stacked widget
        //            auto child = mainWidget->findChild<QWidget*>(key, Qt::FindDirectChildrenOnly);

        //            // Cast to a json object
        //            auto asJson = dynamic_cast<JsonSerializable*>(child);
        //            if(asJson != nullptr)
        //            {
        //                // auto childName = child->objectName();

        //                QJsonObject widgetData = jsonObject.value(name).toObject().value(key).toObject();
        //                auto res = asJson->inputFromJSON(widgetData);

        //                if(res == false)
        //                    return res;
        //            }
        //        }
    }
    else
    {
        bool data = jsonObject.value(name).toBool();

        this->setChecked(data);
    }


    return true;
}


void JsonCheckBox::reset(void)
{
    this->setChecked(defaultValue);
}


void JsonCheckBox::setDefaultValue(bool value)
{
    defaultValue = value;
}

void JsonCheckBox::setMainWidget(JsonWidget *value)
{
    mainWidget = value;
}

void JsonCheckBox::setSubWidget(JsonWidget *value)
{
    subWidget = value;
}
