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

#include "JsonGroupBoxWidget.h"
#include "WorkflowAppOpenSRA.h"

JsonGroupBoxWidget::JsonGroupBoxWidget(QWidget* parent, const QJsonObject& obj, const QString parentKey) : QGroupBox(parent)
{
    auto theWidgetFactory = WorkflowAppOpenSRA::getInstance()->getTheWidgetFactory();

    auto params = obj["Params"].toObject();

    auto layout = theWidgetFactory->getLayout(params, parentKey, parent);

    this->setLayout(layout);
}


JsonGroupBoxWidget::JsonGroupBoxWidget(QWidget* parent) : QGroupBox(parent)
{

}


bool JsonGroupBoxWidget::outputToJSON(QJsonObject &jsonObject)
{

    auto objName = this->objectName();

    QJsonObject thisGroup;

    auto subWidgetList = this->findChildren<QWidget*>(QRegularExpression(), Qt::FindDirectChildrenOnly);
    for(auto&& subWidget : subWidgetList)
    {
        auto key = subWidget->objectName();

        if(key.isEmpty() == true || key.compare("NULL") == 0)
            continue;

        auto asJson = dynamic_cast<JsonSerializable*>(subWidget);
        if(asJson != nullptr)
        {
            QJsonObject obj;
            asJson->outputToJSON(obj);
            thisGroup[key] = obj;
        }
    }

    jsonObject[objName] = thisGroup;

    return true;
}


bool JsonGroupBoxWidget::inputFromJSON(QJsonObject &jsonObject)
{

    auto childWidgetList = this->findChildren<QWidget*>(QRegularExpression(), Qt::FindDirectChildrenOnly);
    QStringList childList;
    for(auto&& child : childWidgetList)
        childList<<child->objectName();

    auto name = this->objectName();

    auto thisObject = jsonObject.value(name).toObject();

    auto keys = thisObject.keys();

    auto res = false;

    for(int i = 0; i < keys.size(); ++i)
    {
        // Get the key
        auto key = keys.at(i);

        // Find the child and send it the data
        auto child = this->findChild<QWidget*>(key, Qt::FindDirectChildrenOnly);

        // Cast to a json object
        auto asJson = dynamic_cast<JsonSerializable*>(child);
        if(asJson != nullptr)
        {
            //            auto childName = child->objectName();

            // Send it the data
            res = asJson->inputFromJSON(thisObject);

            if(res == false)
                return res;

        }
    }

    return res;
}


void JsonGroupBoxWidget::reset(void)
{
    auto childWidgetList = this->findChildren<QWidget*>(QRegularExpression(), Qt::FindDirectChildrenOnly);

    for(auto&& child : childWidgetList)
    {

        auto asJson = dynamic_cast<JsonSerializable*>(child);
        if(asJson != nullptr)
            asJson->reset();

    }
}

