#include "JsonDefinedWidget.h"
#include "WorkflowAppOpenSRA.h"

#include <QJsonArray>
#include <QVariantList>

JsonDefinedWidget::JsonDefinedWidget(QWidget* parent, const QJsonObject& obj, const QString parentKey) : JsonWidget(parent)
{
    auto theWidgetFactory = WorkflowAppOpenSRA::getInstance()->getTheWidgetFactory();

    auto params = obj["Params"].toObject();

    auto displayOrderArray = obj["DisplayOrder"].toArray();

    auto displayOrderVarList = displayOrderArray.toVariantList();

    QStringList displayOrder;

    for(auto&& varnt : displayOrderVarList)
        displayOrder.append(varnt.toString());

    auto layout = theWidgetFactory->getLayout(params, parentKey, parent, displayOrder);

    this->setLayout(layout);
}
