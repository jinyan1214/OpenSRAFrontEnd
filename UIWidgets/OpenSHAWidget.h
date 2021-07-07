#ifndef OPENSHAWIDGET_H
#define OPENSHAWIDGET_H

#include "SimCenterAppWidget.h"

class JsonDefinedWidget;

class OpenSHAWidget : public SimCenterAppWidget
{
public:
    OpenSHAWidget(QWidget* parent = nullptr);

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

    void clear();

public slots:

private:

    QWidget* getMainWidget(void);

    JsonDefinedWidget* mainWidget = nullptr;

};

#endif // OPENSHAWIDGET_H
