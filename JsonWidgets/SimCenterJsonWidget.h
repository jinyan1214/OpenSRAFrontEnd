#ifndef SimCenterJsonWidget_H
#define SimCenterJsonWidget_H

#include "SimCenterAppWidget.h"


class CustomListWidget;
class JsonDefinedWidget;
class JsonWidget;

class QGroupBox;
class QLineEdit;

class SimCenterJsonWidget : public SimCenterAppWidget
{
public:
    SimCenterJsonWidget(QString methodName, QString type, QWidget* parent = nullptr);

    QGroupBox* getWidgetBox(const QString& type);

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

    void clear();

public slots:

    void handleAddButtonPressed(void);

    void handleListItemSelected(const QModelIndex &index);

private:

    QString methodKey;

    JsonDefinedWidget* methodWidget;

    CustomListWidget* listWidget ;

    QLineEdit* weightLineEdit;
};

#endif // SimCenterJsonWidget_H
