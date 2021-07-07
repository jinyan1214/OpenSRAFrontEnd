#ifndef JSONDEFINEDWIDGET_H
#define JSONDEFINEDWIDGET_H

#include "WidgetFactory.h"
#include "JsonWidget.h"

class JsonDefinedWidget : public JsonWidget
{
public:
    JsonDefinedWidget(QWidget* parent, const QJsonObject& obj, const QString parentKey);


private:
    std::unique_ptr<WidgetFactory> theWidgetFactory;
};

#endif // JSONDEFINEDWIDGET_H
