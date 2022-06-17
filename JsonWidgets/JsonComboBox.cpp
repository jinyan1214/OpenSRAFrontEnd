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

#include "JsonComboBox.h"
#include "JsonStackedWidget.h"

JsonComboBox::JsonComboBox(QWidget* parent) : QComboBox(parent)
{
    boxStackedWidget = nullptr;
    defaultIndex = 0;
}


bool JsonComboBox::outputToJSON(QJsonObject &jsonObject)
{    

    auto key = this->objectName();
    QString data;

    // Try to get the data from the "current data" function
    data = this->currentData().toString();

    if(data.isEmpty())
        data = this->currentText();

    QJsonObject outputObj;
    boxStackedWidget->outputToJSON(outputObj);

    if(outputObj.isEmpty())
    {
        jsonObject[key] = data;
    }
    else
    {
        QJsonObject finalObj;

        finalObj[data] = outputObj;
        jsonObject[key] = finalObj;
    }

    return true;
}


bool JsonComboBox::inputFromJSON(QJsonObject &jsonObject)
{

    auto name = this->objectName();

    auto data = jsonObject.value(name).toObject();
    auto keys = data.keys();

    int index = -1;
    QString itemString;

    if(!keys.empty())
    {
        for(auto&& key : keys)
        {
            itemString = key;
            index = this->findData(itemString);

            // auto boxName = boxStackedWidget->objectName();

            // auto childs = boxStackedWidget->findChildren<QWidget*>();
            // QStringList childsList;
            // for(auto&& it : childs)
            //     childsList.append(it->objectName());

            // Find the child and send it the data - note that when you add widget to a stacked widget, ownership of that widget is passed to the stacked widget
            auto child = boxStackedWidget->findChild<QWidget*>(key, Qt::FindDirectChildrenOnly);

            // Cast to a json object
            auto asJson = dynamic_cast<JsonSerializable*>(child);
            if(asJson != nullptr)
            {
                // auto childName = child->objectName();

                QJsonObject widgetData = jsonObject.value(name).toObject().value(key).toObject();
                auto res = asJson->inputFromJSON(widgetData);

                if(res == false)
                    return res;
            }
        }
    }
    else
    {
        itemString = jsonObject.value(name).toString();
        index = this->findData(itemString);

        // Try finding the text
        if(index == -1)
        {
            index = this->findText(itemString);
        }
    }

    if(index != -1)
    {
        this->setCurrentIndex(index);
    }
    else
    {
        if(jsonObject.value(name).isNull())
            this->setCurrentIndex(0);
        else
            this->errorMessage("Error, could not find the item "+ itemString + " in " + name);

        return true;
    }

    return true;
}


void JsonComboBox::setStackedWidget(JsonStackedWidget *value)
{
    boxStackedWidget = value;
}


void JsonComboBox::updateComboBoxValues(const QStringList& vals)
{
    QComboBox::clear();
    this->addItems(vals);
}

void JsonComboBox::setDefaultIndex(int value)
{
    defaultIndex = value;
}


void JsonComboBox::reset(void)
{
    this->setCurrentIndex(defaultIndex);
}


