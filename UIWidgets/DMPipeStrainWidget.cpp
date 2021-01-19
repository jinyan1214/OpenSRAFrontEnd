#include "DMPipeStrainWidget.h"
#include "CustomListWidget.h"

#include <QCheckBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QVBoxLayout>

DMPipeStrainWidget::DMPipeStrainWidget(QWidget* parent) : SimCenterAppWidget(parent)
{

    QSplitter *splitter = new QSplitter(this);

    listWidget = new CustomListWidget(this, "List of Cases to Run");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    splitter->addWidget(this->getWidgetBox());
    splitter->addWidget(listWidget);

    mainLayout->addWidget(splitter);
}


QGroupBox* DMPipeStrainWidget::getWidgetBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Pipe Strain");
    groupBox->setFlat(true);

    auto smallVSpacer = new QSpacerItem(0,20);

    toAssessCheckBox = new QCheckBox("Include in analysis",this);

    auto ModelLabel = new QLabel("Model:", this);
    modelSelectCombo = new QComboBox(this);
    modelSelectCombo->addItem("Median model (preferred, reference)");
    modelSelectCombo->setCurrentIndex(0);
    modelSelectCombo->setMinimumWidth(300);
    modelSelectCombo->setMaximumWidth(450);

    auto ModelParam1Label = new QLabel("Model Parameter 1:",this);
    auto ModelParam1LineEdit = new QLineEdit(this);
    ModelParam1LineEdit->setText("100");
    ModelParam1LineEdit->setMaximumWidth(100);
    auto param1UnitLabel = new QLabel("Unit",this);

    auto ModelParam2Label = new QLabel("Model Parameter 2:",this);
    auto ModelParam2LineEdit = new QLineEdit(this);
    ModelParam2LineEdit->setText("100");
    ModelParam2LineEdit->setMaximumWidth(100);
    auto param2UnitLabel = new QLabel("Unit",this);

    auto ModelParamNLabel = new QLabel("Model Parameter N:",this);
    auto ModelParamNLineEdit = new QLineEdit(this);
    ModelParamNLineEdit->setText("100");
    ModelParamNLineEdit->setMaximumWidth(100);
    auto paramNUnitLabel = new QLabel("Unit",this);

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

    connect(addRunListButton,&QPushButton::clicked, this, &DMPipeStrainWidget::handleAddButtonPressed);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum);

    QGridLayout* gridLayout = new QGridLayout(groupBox);

    gridLayout->addWidget(toAssessCheckBox,0,0);
    gridLayout->addItem(smallVSpacer,0,1);
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

    return groupBox;
}


void DMPipeStrainWidget::handleAddButtonPressed(void)
{
    QString item = modelSelectCombo->currentText();
    QString model = modelSelectCombo->currentData().toString();
    double weight = weightLineEdit->text().toDouble();

    listWidget->addItem(item, model, weight);
}


bool DMPipeStrainWidget::outputToJSON(QJsonObject &jsonObj)
{
    QJsonObject outputObj;

    outputObj.insert("ToAssess", toAssessCheckBox->isChecked());

    auto modelsList = listWidget->getListOfModels();
    auto weightsList = listWidget->getListOfWeights();

    QJsonArray methods = QJsonArray::fromVariantList(modelsList);
    QJsonArray weights = QJsonArray::fromVariantList(weightsList);

    outputObj.insert("ListOfMethods", methods);
    outputObj.insert("ListOfWeights", weights);

    QJsonObject otherParamsObj;

    outputObj.insert("OtherParameters",otherParamsObj);

    QJsonObject episUncertaintyObj;

    outputObj.insert("EpistemicUncertainty",episUncertaintyObj);

    jsonObj.insert("TransientPipeStrain",outputObj);

    return true;
}


bool DMPipeStrainWidget::inputFromJSON(QJsonObject &/*jsonObject*/)
{

    return false;
}

