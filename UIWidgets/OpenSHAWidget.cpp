#include "OpenSHAWidget.h"

#include "WorkflowAppOpenSRA.h"
#include "JsonDefinedWidget.h"

#include <QVBoxLayout>

OpenSHAWidget::OpenSHAWidget(QWidget* parent) : SimCenterAppWidget(parent)
{   
    this->setObjectName("UCERF");

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);

    auto mWidget = this->getMainWidget();

    if(mWidget == nullptr)
        return;

    mWidget->setContentsMargins(0,0,0,0);

    layout->addWidget(mWidget);

    layout->addStretch(1);
}


bool OpenSHAWidget::outputToJSON(QJsonObject &jsonObj)
{
    return mainWidget->outputToJSON(jsonObj);
}


void OpenSHAWidget::clear()
{
    mainWidget->reset();
}


bool OpenSHAWidget::inputFromJSON(QJsonObject &jsonObject)
{
    auto thisObj = jsonObject.value(this->objectName()).toObject();

    // Nothing to change, use default
    if(thisObj.empty())
        return true;

    return mainWidget->inputFromJSON(thisObj);
}


QWidget* OpenSHAWidget::getMainWidget(void)
{
    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams.value("IntensityMeasure").toObject().value(this->objectName()).toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in SimCenterJsonWidget");
        return nullptr;
    }

    mainWidget = new JsonDefinedWidget(this, thisObj, this->objectName());

    return mainWidget;
}

