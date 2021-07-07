#ifndef JsonStackedWidget_H
#define JsonStackedWidget_H

#include "UI/JsonSerializable.h"
#include <QStackedWidget>

class JsonStackedWidget : public QStackedWidget, public JsonSerializable
{
    Q_OBJECT
public:
    explicit JsonStackedWidget(QWidget *parent = nullptr);

    bool outputToJSON(QJsonObject &jsonObject);
    bool inputFromJSON(QJsonObject &jsonObject);

    void reset(void);

signals:

};

#endif // JsonStackedWidget_H
