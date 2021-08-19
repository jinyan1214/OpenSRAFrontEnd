#ifndef JsonCheckBox_H
#define JsonCheckBox_H

#include "UI/JsonSerializable.h"

#include <QCheckBox>

class JsonWidget;

class JsonCheckBox : public QCheckBox, public JsonSerializable
{
public:
    JsonCheckBox(QWidget* parent);

    bool outputToJSON(QJsonObject &jsonObject);

    bool inputFromJSON(QJsonObject &jsonObject);

    void reset(void);

    void setDefaultValue(bool value);

    void setMainWidget(JsonWidget *value);

    void setSubWidget(JsonWidget *value);

private:

    bool defaultValue;

    JsonWidget* mainWidget;

    JsonWidget* subWidget;

};

#endif // JsonCheckBox_H
