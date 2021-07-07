#include "JsonGroupBoxWidget.h"

JsonGroupBoxWidget::JsonGroupBoxWidget(QWidget* parent, const QJsonObject& obj, const QString parentKey) : QGroupBox(parent)
{
    theWidgetFactory = std::make_unique<WidgetFactory>(this);

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

