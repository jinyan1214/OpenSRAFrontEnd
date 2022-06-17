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

#include "JsonLabel.h"

#include <QRegExpValidator>
#include <sstream>

JsonLabel::JsonLabel(QWidget* parent) : QLabel(parent)
{

}


bool JsonLabel::outputToJSON(QJsonObject &jsonObject)
{
    auto key = this->objectName();
    auto data = this->text();

    if(data.isEmpty())
    {
        jsonObject[key] = QJsonValue::Null;
        return true;
    }

    if(dataType == QJsonValue::Type::Double)
        jsonObject[key] = data.toDouble();
    else if(dataType == QJsonValue::Type::String)
        jsonObject[key] = data;
    else if(dataType == QJsonValue::Type::Null)
        jsonObject[key] = QJsonValue::Null; // "null"
    else
    {
        this->errorMessage("Error, type not supported in " + this->objectName());
        return false;
    }


    return true;
}


bool JsonLabel::inputFromJSON(QJsonObject &jsonObject)
{

    QString name = this->objectName();

    auto obj = jsonObject.value(name);

    QString data = this->getStringValueJson(obj);

    this->setText(data);

    return true;
}


void JsonLabel::setDefaultValue(const QJsonValue& obj)
{
    auto defValueStr = this->getStringValueJson(obj);
    this->setText(defValueStr);

    defaultValue = defValueStr;
}


void JsonLabel::reset(void)
{
    this->setText(defaultValue);
}


QString JsonLabel::getStringValueJson(const QJsonValue& obj)
{
    QString val;

    dataType = obj.type();

    if(dataType == QJsonValue::Type::Double)
    {
        val = QString::number(obj.toDouble());
    }
    else if(dataType == QJsonValue::Type::String)
    {
        val = obj.toString();
    }
    else if(dataType == QJsonValue::Type::Null)
    {
        val = "";
    }
    else
    {
        this->errorMessage("Error, type not supported in " + this->objectName());
    }

    return val;
}
