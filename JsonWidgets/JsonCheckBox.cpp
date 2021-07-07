#include "JsonCheckBox.h"
#include "JsonWidget.h"


JsonCheckBox::JsonCheckBox(QWidget* parent) : QCheckBox(parent)
{
    defaultValue = false;
}


bool JsonCheckBox::outputToJSON(QJsonObject &jsonObject)
{
    auto key = this->objectName();

    QJsonObject outputObj;
    mainWidget->outputToJSON(outputObj);

    if(outputObj.isEmpty())
    {
        jsonObject[key] = this->isChecked();
    }
    else
    {
        jsonObject[key] = outputObj;
    }

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
