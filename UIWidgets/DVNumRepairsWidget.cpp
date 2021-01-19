#include "DVNumRepairsWidget.h"
#include "CustomListWidget.h"

#include <QCheckBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QVBoxLayout>

DVNumRepairsWidget::DVNumRepairsWidget(QWidget* parent) : SimCenterAppWidget(parent)
{

    PGVTreeItem = nullptr;
    PGDTreeItem = nullptr;

    QSplitter *splitter = new QSplitter(this);

    listWidget = new CustomListWidget(this, "List of Cases to Run");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    splitter->addWidget(this->getWidgetBox());
    splitter->addWidget(listWidget);

    mainLayout->addWidget(splitter);
}


QGroupBox* DVNumRepairsWidget::getWidgetBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Repair Rates");
    groupBox->setFlat(true);

    auto smallVSpacer = new QSpacerItem(0,20);

    QGroupBox* demandBox = new QGroupBox("Demand",this);
    QVBoxLayout* demandBoxLayout = new QVBoxLayout(demandBox);

    PGVCheckBox = new QCheckBox("Shaking Induced",this);
    PGDCheckBox = new QCheckBox("Deformation Induced",this);

    demandBoxLayout->addWidget(PGVCheckBox);
    demandBoxLayout->addWidget(PGDCheckBox);

    toAssessCheckBox = new QCheckBox("Include in analysis",this);

    auto ModelLabel = new QLabel("Model:", this);
    modelSelectCombo = new QComboBox(this);
    modelSelectCombo->addItem("O'Rourke (2020)","ORourke2020");
    modelSelectCombo->addItem("ALA (2001)","ALA2001");
    modelSelectCombo->addItem("Hazus (FEMA, 2014)","Hazus2014");

    modelSelectCombo->setCurrentIndex(0);

    QDoubleValidator* validator = new QDoubleValidator(this);
    validator->setRange(0.0,1.0,5);

    auto weightLabel = new QLabel("Weight:");
    weightLineEdit = new QLineEdit(this);
    weightLineEdit->setText("1.0");
    weightLineEdit->setValidator(validator);
    weightLineEdit->setMaximumWidth(100);

    QPushButton *addRunListButton = new QPushButton(this);
    addRunListButton->setText(tr("Add run to list"));
    addRunListButton->setMinimumWidth(250);

    connect(addRunListButton,&QPushButton::clicked, this, &DVNumRepairsWidget::handleAddButtonPressed);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum);

    QGridLayout* gridLayout = new QGridLayout(groupBox);

    gridLayout->addWidget(toAssessCheckBox,0,0,1,2);
    gridLayout->addItem(smallVSpacer,0,1);
    gridLayout->addWidget(demandBox,1,0,1,2);
    gridLayout->addWidget(ModelLabel,2,0);
    gridLayout->addWidget(modelSelectCombo,2,1);

    gridLayout->addWidget(weightLabel,3,0);
    gridLayout->addWidget(weightLineEdit,3,1);
    gridLayout->addWidget(addRunListButton,4,0,1,2,Qt::AlignCenter);
    gridLayout->addItem(vspacer, 5, 0);
    gridLayout->addItem(hspacer, 0, 1,6,1);

    return groupBox;
}


void DVNumRepairsWidget::handleAddButtonPressed(void)
{
    QString item = modelSelectCombo->currentText();
    QString model = modelSelectCombo->currentData().toString();
    double weight = weightLineEdit->text().toDouble();

    if(PGVCheckBox->isChecked())
    {
        if(!PGVTreeItem)
            PGVTreeItem = listWidget->addItem("Shaking Induced");

        listWidget->addItem(item, model, weight, PGVTreeItem);

    }

    if(PGDCheckBox->isChecked())
    {
        if(!PGDTreeItem)
            PGDTreeItem = listWidget->addItem("Deformation Induced");

        listWidget->addItem(item, model, weight, PGDTreeItem);
    }

    if(!PGDCheckBox->isChecked() && !PGVCheckBox->isChecked())
    {
        QString msg = "Please check the \"Shaking Induced\" or \"Deformation Induced\" checkbox to add items to list to run";
        this->userMessageDialog(msg);
    }

}


bool DVNumRepairsWidget::outputToJSON(QJsonObject &jsonObj)
{

    QJsonObject PGVObj;

    PGVObj.insert("ToAssess", PGVCheckBox->isChecked());

    auto modelsListPGV = listWidget->getListOfModels();
    auto weightsListPGV = listWidget->getListOfWeights();

    QJsonArray methodsPGV = QJsonArray::fromVariantList(modelsListPGV);
    QJsonArray weightsPGV = QJsonArray::fromVariantList(weightsListPGV);

    PGVObj.insert("ListOfMethods",methodsPGV);
    PGVObj.insert("ListOfWeights",weightsPGV);

    QJsonObject otherParamsObjPGV;
    PGVObj.insert("OtherParameters",otherParamsObjPGV);

    QJsonObject PGDObj;

    PGDObj.insert("ToAssess", PGDCheckBox->isChecked());

    auto modelsListPGD = listWidget->getListOfModels();
    auto weightsListPGD = listWidget->getListOfWeights();

    QJsonArray methodsPGD = QJsonArray::fromVariantList(modelsListPGD);
    QJsonArray weightsPGD = QJsonArray::fromVariantList(weightsListPGD);

    PGDObj.insert("ListOfMethods",methodsPGD);
    PGDObj.insert("ListOfWeights",weightsPGD);

    QJsonObject otherParamsObjPGD;
    otherParamsObjPGD.insert("flag_rup_depend", true);
    otherParamsObjPGD.insert("pgd_cutoff", 10.16);
    PGDObj.insert("OtherParameters",otherParamsObjPGD);

    jsonObj.insert("RepairRatePGV",PGVObj);
    jsonObj.insert("RepairRatePGD",PGDObj);

    return true;
}


bool DVNumRepairsWidget::inputFromJSON(QJsonObject &/*jsonObject*/)
{

    return false;
}

