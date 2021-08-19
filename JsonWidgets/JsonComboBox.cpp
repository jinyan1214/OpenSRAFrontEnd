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
    this->reset();
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


