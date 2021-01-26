#ifndef DVNumRepairsWidget_H
#define DVNumRepairsWidget_H

#include "SimCenterAppWidget.h"

#include <QGroupBox>
#include <QMap>

class CustomListWidget;
class TreeItem;

class QCheckBox;
class QComboBox;
class QLineEdit;

class DVNumRepairsWidget : public SimCenterAppWidget
{
public:
    DVNumRepairsWidget(QWidget* parent = nullptr);

    QGroupBox* getWidgetBox(void);

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

    void clear();

public slots:

    void handleAddButtonPressed(void);

private:

    CustomListWidget *listWidget ;

    QCheckBox* PGVCheckBox;
    QCheckBox* PGDCheckBox;

    TreeItem* PGVTreeItem;
    TreeItem* PGDTreeItem;

    QComboBox* modelSelectCombo;
    QLineEdit* weightLineEdit;

};

#endif // DVNumRepairsWidget_H
