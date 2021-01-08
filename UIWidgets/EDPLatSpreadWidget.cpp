#include "EDPLatSpreadWidget.h"
#include "CustomListWidget.h"

#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QVBoxLayout>

EDPLatSpreadWidget::EDPLatSpreadWidget(QWidget* parent) : SimCenterAppWidget(parent)
{

    QSplitter *splitter = new QSplitter(this);

    listWidget = new CustomListWidget(this, "List of Cases to Run");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    splitter->addWidget(this->getLandSlideBox());
    splitter->addWidget(listWidget);

    mainLayout->addWidget(splitter);
}


QGroupBox* EDPLatSpreadWidget::getLandSlideBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Lateral Spreading");
    groupBox->setFlat(true);

    auto smallVSpacer = new QSpacerItem(0,10);

    auto ModelLabel = new QLabel("Model:");
    modelSelectCombo = new QComboBox();
    modelSelectCombo->addItem("Median model (preferred, reference)");
    modelSelectCombo->setCurrentIndex(0);
    modelSelectCombo->setMinimumWidth(300);
    modelSelectCombo->setMaximumWidth(450);

    auto ModelParam1Label = new QLabel("Model Parameter 1:");
    auto ModelParam1LineEdit = new QLineEdit();
    ModelParam1LineEdit->setText("100");
    ModelParam1LineEdit->setMaximumWidth(100);
    auto param1UnitLabel = new QLabel("Unit");

    auto ModelParam2Label = new QLabel("Model Parameter 2:");
    auto ModelParam2LineEdit = new QLineEdit();
    ModelParam2LineEdit->setText("100");
    ModelParam2LineEdit->setMaximumWidth(100);
    auto param2UnitLabel = new QLabel("Unit");

    auto ModelParamNLabel = new QLabel("Model Parameter N:");
    auto ModelParamNLineEdit = new QLineEdit();
    ModelParamNLineEdit->setText("100");
    ModelParamNLineEdit->setMaximumWidth(100);
    auto paramNUnitLabel = new QLabel("Unit");

    auto weightLabel = new QLabel("Weight:");
    weightLineEdit = new QLineEdit();
    weightLineEdit->setText("1");
    weightLineEdit->setMaximumWidth(100);

    QPushButton *addRunListButton = new QPushButton();
    addRunListButton->setText(tr("Add run to list"));
    addRunListButton->setMinimumWidth(250);

    connect(addRunListButton,&QPushButton::clicked, this, &EDPLatSpreadWidget::handleAddButtonPressed);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum);

    QGridLayout* gridLayout = new QGridLayout();

    gridLayout->addItem(smallVSpacer,0,0);
    gridLayout->addWidget(ModelLabel,1,0);
    gridLayout->addWidget(modelSelectCombo,1,1,1,2);

    gridLayout->addWidget(ModelParam1Label,2,0);
    gridLayout->addWidget(ModelParam1LineEdit,2,1);
    gridLayout->addWidget(param1UnitLabel,2,2);

    gridLayout->addWidget(ModelParam2Label,3,0);
    gridLayout->addWidget(ModelParam2LineEdit,3,1);
    gridLayout->addWidget(param2UnitLabel,3,2);

    gridLayout->addWidget(ModelParamNLabel,4,0);
    gridLayout->addWidget(ModelParamNLineEdit,4,1);
    gridLayout->addWidget(paramNUnitLabel,4,2);

    gridLayout->addWidget(weightLabel,5,0);
    gridLayout->addWidget(weightLineEdit,5,1);

    gridLayout->addWidget(addRunListButton,6,0,1,3,Qt::AlignCenter);

    gridLayout->addItem(hspacer, 1, 4);
    gridLayout->addItem(vspacer, 7, 0);
    groupBox->setLayout(gridLayout);

    return groupBox;
}


void EDPLatSpreadWidget::handleAddButtonPressed(void)
{
    QString newItem = QString::number(listWidget->getNumberOfItems()+1) + ". " + modelSelectCombo->currentText() + " - weight="+weightLineEdit->text();

    listWidget->addItem(newItem);
}
