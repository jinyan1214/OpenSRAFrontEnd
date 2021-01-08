#ifndef EDPLatSpreadWidget_H
#define EDPLatSpreadWidget_H

#include "SimCenterAppWidget.h"

#include <QGroupBox>

class CustomListWidget;

class QComboBox;
class QLineEdit;

class EDPLatSpreadWidget : public SimCenterAppWidget
{
public:
    EDPLatSpreadWidget(QWidget* parent = nullptr);

    QGroupBox* getLandSlideBox(void);

public slots:

    void handleAddButtonPressed(void);

private:

    CustomListWidget *listWidget ;

    QComboBox* modelSelectCombo;
    QLineEdit* weightLineEdit;
};

#endif // EDPLatSpreadWidget_H
