#ifndef EDPLiquefactionWidget_H
#define EDPLiquefactionWidget_H

#include "SimCenterAppWidget.h"

#include <QGroupBox>

class CustomListWidget;

class QComboBox;
class QLineEdit;

class EDPLiquefactionWidget : public SimCenterAppWidget
{
public:
    EDPLiquefactionWidget(QWidget* parent = nullptr);

    QGroupBox* getLandSlideBox(void);

public slots:

    void handleAddButtonPressed(void);

private:

    CustomListWidget *listWidget ;

    QComboBox* modelSelectCombo;
    QLineEdit* weightLineEdit;
};

#endif // EDPLiquefactionWidget_H
