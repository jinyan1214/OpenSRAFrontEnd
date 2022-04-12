#include "JsonWidget.h"

JsonWidget::JsonWidget(QWidget* parent) : QWidget(parent)
{

}


bool JsonWidget::outputToJSON(QJsonObject &jsonObject)
{
    auto childWidgetList = this->findChildren<QWidget*>(QRegularExpression(), Qt::FindDirectChildrenOnly);
    QStringList childList;
    for(auto&& child : childWidgetList)
        childList<<child->objectName();

    // auto thisObj = this->objectName();

    // qDebug()<<thisObj;

    auto subWidgetList = this->findChildren<QWidget*>(QRegularExpression(), Qt::FindDirectChildrenOnly);
    for(auto&& subWidget : subWidgetList)
    {
        auto key = subWidget->objectName();

        if(key.isEmpty() == true || key.compare("NULL") == 0)
            continue;

        auto asJson = dynamic_cast<JsonSerializable*>(subWidget);
        if(asJson != nullptr)
        {
            asJson->outputToJSON(jsonObject);
        }
    }


    // Works with ex 6,

//    auto childWidgetList = this->findChildren<QWidget*>(QRegularExpression(), Qt::FindDirectChildrenOnly);
//    QStringList childList;
//    for(auto&& child : childWidgetList)
//        childList<<child->objectName();

//    auto thisObj = this->objectName();

//    // qDebug()<<thisObj;

//    auto subWidgetList = this->findChildren<QWidget*>(QRegularExpression(), Qt::FindDirectChildrenOnly);
//    for(auto&& subWidget : subWidgetList)
//    {
//        auto key = subWidget->objectName();

//        if(key.isEmpty() == true || key.compare("NULL") == 0)
//            continue;

//        auto asJson = dynamic_cast<JsonSerializable*>(subWidget);
//        if(asJson != nullptr)
//        {
//            asJson->outputToJSON(jsonObject);
//        }
//    }


    return true;
}


bool JsonWidget::inputFromJSON(QJsonObject &jsonObject)
{

    auto childWidgetList = this->findChildren<QWidget*>(QRegularExpression(), Qt::FindDirectChildrenOnly);
    QStringList childList;
    for(auto&& child : childWidgetList)
        childList<<child->objectName();


    //auto name = this->objectName();

    auto keys = jsonObject.keys();

    bool res = false;

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

            res = asJson->inputFromJSON(jsonObject);

            if(res == false)
                return res;
        }
    }

    return res;
}


void JsonWidget::reset(void)
{
    auto childWidgetList = this->findChildren<QWidget*>(QRegularExpression(), Qt::FindDirectChildrenOnly);

    for(auto&& child : childWidgetList)
    {
        auto asJson = dynamic_cast<JsonSerializable*>(child);
        if(asJson != nullptr)
            asJson->reset();
    }
}
