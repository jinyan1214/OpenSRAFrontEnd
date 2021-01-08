#ifndef OPENSHAWIDGET_H
#define OPENSHAWIDGET_H

#include "SimCenterAppWidget.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;

class OpenSHAWidget : public SimCenterAppWidget
{
public:
    OpenSHAWidget(QWidget* parent = nullptr);

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

public slots:
    void handleVS30Change(int index);

private:
    QComboBox* modelSelectCombo;
    QComboBox* seismicSourceCombo;
    QComboBox* vs30Combo;

    QCheckBox* RpCheckBox;
    QLineEdit* RpLineEdit;

    QCheckBox* distCheckBox;
    QLineEdit* distLineEdit;

    QCheckBox* magCheckBox;
    QLineEdit* magLineEditMin;
    QLineEdit* magLineEditMax;

    QCheckBox* pointSourceCheckBox;

    QLabel* vs30InfoLabel;
};

#endif // OPENSHAWIDGET_H
