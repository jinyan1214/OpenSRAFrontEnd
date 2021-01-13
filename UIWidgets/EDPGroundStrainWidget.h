#ifndef EDPGroundStrainWidget_H
#define EDPGroundStrainWidget_H

#include "SimCenterAppWidget.h"

#include <QGroupBox>

class CustomListWidget;

class QCheckBox;
class QComboBox;
class QLineEdit;

class EDPGroundStrainWidget : public SimCenterAppWidget
{
public:
    EDPGroundStrainWidget(QWidget* parent = nullptr);

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

#endif // EDPGroundStrainWidget_H
