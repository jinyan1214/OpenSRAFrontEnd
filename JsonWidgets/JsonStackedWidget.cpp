#include "JsonStackedWidget.h"

JsonStackedWidget::JsonStackedWidget(QWidget *parent) : QStackedWidget(parent)
{

}


bool JsonStackedWidget::outputToJSON(QJsonObject &jsonObject)
{
    auto currWidget = dynamic_cast<JsonSerializable *>(this->currentWidget());

    if(currWidget)
    {
        auto currWIdgetName = this->currentWidget()->objectName();

        currWidget->outputToJSON(jsonObject);
    }
    else
        return false;

    return true;
}


bool JsonStackedWidget::inputFromJSON(QJsonObject &jsonObject)
{
    return true;
}


void JsonStackedWidget::reset(void)
{
    auto numWidgets = this->count();
    for(int i = 0; i<numWidgets; ++i)
    {
        auto child = this->widget(i);

        auto asJson = dynamic_cast<JsonSerializable*>(child);
        if(asJson != nullptr)
            asJson->reset();
    }
}
