#ifndef JSONLINEEDIT_H
#define JSONLINEEDIT_H

#include "UI/JsonSerializable.h"

#include <QLineEdit>
#include <QJsonArray>

class JsonLineEdit : public QLineEdit, public JsonSerializable
{
public:
    JsonLineEdit(QWidget* parent);

    bool outputToJSON(QJsonObject &jsonObject);

    bool inputFromJSON(QJsonObject &jsonObject);

    void setDefaultValue(const QJsonValue& obj);

    void reset(void);

    QString getStringValueJson(const QJsonValue& obj);

    QJsonArray getArrayFromText(const QString& text);
    QString getTextFromArray(const QJsonArray& array);

private:

    QJsonValue::Type dataType;

    QString defaultValue;
};

#endif // JSONLINEEDIT_H
