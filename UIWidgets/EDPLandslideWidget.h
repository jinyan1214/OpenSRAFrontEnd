#ifndef EDPLANDSLIDEWIDGET_H
#define EDPLANDSLIDEWIDGET_H

#include "SimCenterAppWidget.h"

#include <QGroupBox>

class CustomListWidget;
class JsonDefinedWidget;
class JsonWidget;

class QCheckBox;
class QComboBox;
class QLineEdit;

class EDPLandslideWidget : public SimCenterAppWidget
{
public:
    EDPLandslideWidget(QWidget* parent = nullptr);

    QGroupBox* getWidgetBox(void);

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

    void clear();

public slots:

    void handleAddButtonPressed(void);

    void handleListItemSelected(const QModelIndex &index);

private:

    JsonWidget* getYieldMethodWidget();
    JsonWidget* getYieldAccWidget();

    JsonDefinedWidget* methodWidget;

    JsonWidget* yieldAccParametersWidget;
    JsonWidget* yieldAccMethodWidget;

    CustomListWidget *listWidget ;

    QLineEdit* weightLineEdit;
};

#endif // EDPLANDSLIDEWIDGET_H
