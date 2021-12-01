#ifndef JsonGroupBoxWidget_H
#define JsonGroupBoxWidget_H

#include "UI/JsonSerializable.h"

#include <QGroupBox>
#include <QJsonObject>

class JsonGroupBoxWidget : public QGroupBox, public JsonSerializable
{
public:
    JsonGroupBoxWidget(QWidget* parent, const QJsonObject& obj, const QString parentKey);
    JsonGroupBoxWidget(QWidget* parent);

    bool outputToJSON(QJsonObject &jsonObject);

    bool inputFromJSON(QJsonObject &jsonObject);

    void reset(void);

private:
};

#endif // JsonGroupBoxWidget_H
