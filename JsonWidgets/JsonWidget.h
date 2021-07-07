#ifndef JSONWIDGET_H
#define JSONWIDGET_H

#include "UI/JsonSerializable.h"

#include <QWidget>

class JsonWidget : public QWidget, public JsonSerializable
{
public:
    JsonWidget(QWidget* parent);

    bool outputToJSON(QJsonObject &jsonObject);

    bool inputFromJSON(QJsonObject &jsonObject);

    void reset(void);
};

#endif // JSONQWIDGET_H
