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

#include "JsonLineEdit.h"
#include <QRegExpValidator>
#include <sstream>

JsonLineEdit::JsonLineEdit(QWidget* parent) : QLineEdit(parent)
{

}


bool JsonLineEdit::outputToJSON(QJsonObject &jsonObject)
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
    else if(dataType == QJsonValue::Type::Array)
        jsonObject[key] = getArrayFromText(data);
    else if(dataType == QJsonValue::Type::Null)
        jsonObject[key] = QJsonValue::Null; // "null"
    else
    {
        this->errorMessage("Error, type not supported in " + this->objectName());
        return false;
    }

    return true;
}


bool JsonLineEdit::inputFromJSON(QJsonObject &jsonObject)
{

    QString name = this->objectName();

    auto obj = jsonObject.value(name);

    QString data = this->getStringValueJson(obj);

    this->setText(data);

    return true;
}


void JsonLineEdit::setDefaultValue(const QJsonValue& obj)
{
    auto defValueStr = this->getStringValueJson(obj);
    this->setText(defValueStr);

    defaultValue = defValueStr;
}


void JsonLineEdit::reset(void)
{
    this->setText(defaultValue);
}


QString JsonLineEdit::getStringValueJson(const QJsonValue& obj)
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
    else if(dataType == QJsonValue::Type::Array)
    {
        val = getTextFromArray(obj.toArray());
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


QJsonArray JsonLineEdit::getArrayFromText(const QString& text)
{
    QString inputText = text;

    QJsonArray array;

    // Quick return if the input text is empty
    if(inputText.isEmpty())
        return array;

    // Remove any white space from the string
    inputText.remove(" ");

    // Split the incoming text into the parts delimited by commas
    std::vector<std::string> subStringVec;

    // Create string stream from the string
    std::stringstream s_stream(inputText.toStdString());

    // Split the input string to substrings at the comma
    while(s_stream.good()) {
        std:: string subString;
        getline(s_stream, subString, ',');

        if(!subString.empty())
            subStringVec.push_back(subString);
    }

    try
    {
        for(auto&& subStr : subStringVec)
        {
            auto val = std::stod(subStr);
            array.append(val);
        }
    }
    catch (std::exception& e)
    {
        qDebug()<<"Error getting array form string with message "<<e.what();
    }

    return array;
}


QString JsonLineEdit::getTextFromArray(const QJsonArray& array)
{
    QString text;

    auto valList = array.toVariantList();

    auto numVals = valList.size();

    for(int i = 0; i<numVals; ++i)
    {
        auto val = valList.at(i);

        text.append(val.toString());

        if(i != numVals-1)
            text.append(", ");
    }

    return text;
}

