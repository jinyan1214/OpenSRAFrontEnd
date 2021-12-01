#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include "SimCenterWidget.h"

#include <QJsonObject>

class ComponentInputWidget;

class QFormLayout;
class QBoxLayout;

class WidgetFactory : public SimCenterWidget
{
public:
    WidgetFactory(ComponentInputWidget* parent = nullptr);

    QLayout* getLayout(const QJsonObject& obj, const QString& parentKey, QWidget* parent, QStringList widgetOrder = QStringList());

    QLayout* getLayoutFromParams(const QJsonObject& params, const QString& parentKey, QWidget* parent, Qt::Orientation orientation = Qt::Vertical, QStringList widgetOrder = QStringList());

    QWidget* getWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent);

private:

    bool addWidgetToLayout(const QJsonObject& paramObj, const QString& key, QWidget* parent, QBoxLayout* mainLayout);

    QWidget* getComboBoxWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent);
    QWidget* getLineEditWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent);
    QWidget* getCheckBoxWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent);
    QWidget* getBoxWidget(const QJsonObject& obj, const QString& parentKey, QWidget* parent);

    bool isNestedComboBoxWidget(const QJsonObject& obj);

    ComponentInputWidget* parentInputWidget;
};

#endif // WIDGETFACTORY_H
