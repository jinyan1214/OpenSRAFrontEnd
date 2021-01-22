#ifndef EDPLatSpreadWidget_H
#define EDPLatSpreadWidget_H

#include "SimCenterAppWidget.h"

#include <QGroupBox>

class CustomListWidget;

class QCheckBox;
class QComboBox;
class QLineEdit;

class EDPLatSpreadWidget : public SimCenterAppWidget
{
public:
    EDPLatSpreadWidget(QWidget* parent = nullptr);

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

#endif // EDPLatSpreadWidget_H
