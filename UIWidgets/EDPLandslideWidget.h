#ifndef EDPLANDSLIDEWIDGET_H
#define EDPLANDSLIDEWIDGET_H

#include "SimCenterAppWidget.h"

#include <QGroupBox>

class CustomListWidget;

class QComboBox;
class QLineEdit;

class EDPLandslideWidget : public SimCenterAppWidget
{
public:
    EDPLandslideWidget(QWidget* parent = nullptr);

    QGroupBox* getLandSlideBox(void);

public slots:

    void handleAddButtonPressed(void);

private:

    CustomListWidget *listWidget ;

    QComboBox* modelSelectCombo;
    QLineEdit* weightLineEdit;
};

#endif // EDPLANDSLIDEWIDGET_H
