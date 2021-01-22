#ifndef EDPSubSurfFaultRupWidget_H
#define EDPSubSurfFaultRupWidget_H

#include "SimCenterAppWidget.h"

#include <QGroupBox>

class CustomListWidget;

class QCheckBox;
class QComboBox;
class QLineEdit;

class EDPSubSurfFaultRupWidget : public SimCenterAppWidget
{
public:
    EDPSubSurfFaultRupWidget(QWidget* parent = nullptr);

    QGroupBox* getWidgetBox(void);

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

    void clear();

public slots:

    void handleAddButtonPressed(void);

private:

    CustomListWidget *listWidget ;

    QCheckBox* toAssessCheckBox;
    QComboBox* modelSelectCombo;
    QLineEdit* weightLineEdit;
};

#endif // EDPSubSurfFaultRupWidget_H
