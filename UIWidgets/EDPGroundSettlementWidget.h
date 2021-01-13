#ifndef EDPGroundSettlementWidget_H
#define EDPGroundSettlementWidget_H

#include "SimCenterAppWidget.h"

#include <QGroupBox>

class CustomListWidget;

class QCheckBox;
class QComboBox;
class QLineEdit;

class EDPGroundSettlementWidget : public SimCenterAppWidget
{
public:
    EDPGroundSettlementWidget(QWidget* parent = nullptr);

    QGroupBox* getWidgetBox(void);

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

public slots:

    void handleAddButtonPressed(void);

private:

    CustomListWidget *listWidget ;

    QCheckBox* toAssessCheckBox;
    QComboBox* modelSelectCombo;
    QLineEdit* weightLineEdit;
};

#endif // EDPGroundSettlementWidget_H
