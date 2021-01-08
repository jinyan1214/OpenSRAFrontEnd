#include "EDPLandslideWidget.h"
#include "CustomListWidget.h"

#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QVBoxLayout>

EDPLandslideWidget::EDPLandslideWidget(QWidget* parent) : SimCenterAppWidget(parent)
{

    QSplitter *splitter = new QSplitter(this);

    listWidget = new CustomListWidget(this, "List of Cases to Run");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    splitter->addWidget(this->getLandSlideBox());
    splitter->addWidget(listWidget);

    mainLayout->addWidget(splitter);
}


QGroupBox* EDPLandslideWidget::getLandSlideBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Landslide");
    groupBox->setFlat(true);

    auto smallVSpacer = new QSpacerItem(0,10);

    auto ModelLabel = new QLabel("Model:");
    modelSelectCombo = new QComboBox();
    modelSelectCombo->addItem("Bray and Macedo (2019)","BrayMacedo2019");
    modelSelectCombo->addItem("Jibson (2007)","Jibson2007");
    modelSelectCombo->setCurrentIndex(0);
    modelSelectCombo->setMinimumWidth(400);


    auto weightLabel = new QLabel("Weight:");
    weightLineEdit = new QLineEdit();
    weightLineEdit->setText("1");
    weightLineEdit->setMaximumWidth(100);

    QPushButton *addRunListButton = new QPushButton();
    addRunListButton->setText(tr("Add run to list"));
    addRunListButton->setMinimumWidth(250);

    connect(addRunListButton,&QPushButton::clicked, this, &EDPLandslideWidget::handleAddButtonPressed);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum);

    QGridLayout* gridLayout = new QGridLayout();

    gridLayout->addItem(smallVSpacer,0,0);
    gridLayout->addWidget(ModelLabel,1,0);
    gridLayout->addWidget(modelSelectCombo,1,1);

    gridLayout->addWidget(weightLabel,2,0);
    gridLayout->addWidget(weightLineEdit,2,1);

    gridLayout->addWidget(addRunListButton,3,0,1,2,Qt::AlignCenter);

    gridLayout->addItem(vspacer, 7, 0);
    groupBox->setLayout(gridLayout);

    return groupBox;
}


void EDPLandslideWidget::handleAddButtonPressed(void)
{
    QString newItem = QString::number(listWidget->getNumberOfItems()+1) + ". " + modelSelectCombo->currentText() + " - weight="+weightLineEdit->text();

    listWidget->addItem(newItem);
}
