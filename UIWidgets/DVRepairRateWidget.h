#ifndef DVRepairRateWidget_H
#define DVRepairRateWidget_H

#include "SimCenterAppWidget.h"

#include <QGroupBox>

class CustomListWidget;
class JsonDefinedWidget;
class JsonGroupBoxWidget;
class TreeItem;

class QCheckBox;
class QComboBox;
class QLineEdit;

class DVRepairRateWidget : public SimCenterAppWidget
{
public:
    DVRepairRateWidget(QWidget* parent = nullptr);

    QGroupBox* getWidgetBox(void);

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

    void clear();

public slots:

    void handleAddButtonPressed(void);

    void handleListItemSelected(const QModelIndex &index);

private:

    JsonGroupBoxWidget* getDemandWidget();
    JsonGroupBoxWidget* getComponentPropertiesWidget();

    JsonDefinedWidget* methodWidget;
    JsonGroupBoxWidget* demandWidget;
    JsonGroupBoxWidget* componentPropertiesWidget;

    CustomListWidget *listWidget ;

    QLineEdit* weightLineEdit;
};

#endif // DVRepairRateWidget_H
