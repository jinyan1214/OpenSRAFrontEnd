#include "JsonDefinedWidget.h"


JsonDefinedWidget::JsonDefinedWidget(QWidget* parent, const QJsonObject& obj, const QString parentKey) : JsonWidget(parent)
{
    theWidgetFactory = std::make_unique<WidgetFactory>(this);

    auto params = obj["Params"].toObject();

    auto layout = theWidgetFactory->getLayout(params, parentKey, parent);

    this->setLayout(layout);
}
