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
        return true;

    if(dataType == QJsonValue::Type::Double)
        jsonObject[key] = data.toDouble();
    else if(dataType == QJsonValue::Type::String)
        jsonObject[key] = data;
    else if(dataType == QJsonValue::Type::Array)
        jsonObject[key] = getArrayFromText(data);
    else if(dataType == QJsonValue::Type::Null)
        jsonObject[key] = "null";
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

