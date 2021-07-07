#ifndef JSONCOMBOBOX_H
#define JSONCOMBOBOX_H

#include <QComboBox>
#include "UI/JsonSerializable.h"

class JsonStackedWidget;

class JsonComboBox : public QComboBox, public JsonSerializable
{
public:
    JsonComboBox(QWidget* parent);

    bool outputToJSON(QJsonObject &jsonObject);
    bool inputFromJSON(QJsonObject &jsonObject);

    void setStackedWidget(JsonStackedWidget *value);

    void reset(void);

    void setDefaultIndex(int value);

public slots:

    void updateComboBoxValues(const QStringList& vals);

private:

    JsonStackedWidget* boxStackedWidget;

    int defaultIndex;
};

#endif // JSONCOMBOBOX_H
