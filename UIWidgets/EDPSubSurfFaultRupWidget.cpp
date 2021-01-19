#include "EDPSubSurfFaultRupWidget.h"
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

EDPSubSurfFaultRupWidget::EDPSubSurfFaultRupWidget(QWidget* parent) : SimCenterAppWidget(parent)
{

    QSplitter *splitter = new QSplitter(this);

    listWidget = new CustomListWidget(this, "List of Cases to Run");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    splitter->addWidget(this->getWidgetBox());
    splitter->addWidget(listWidget);

    mainLayout->addWidget(splitter);
}


QGroupBox* EDPSubSurfFaultRupWidget::getWidgetBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Subsurface Fault Rupture");
    groupBox->setFlat(true);

    auto smallVSpacer = new QSpacerItem(0,20);

    toAssessCheckBox = new QCheckBox("Include in analysis",this);

    auto ModelLabel = new QLabel("Model:", this);
    modelSelectCombo = new QComboBox(this);
    modelSelectCombo->addItem("Median model (preferred, reference)");
    modelSelectCombo->setCurrentIndex(0);

    auto notesLabel = new QLabel("");

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

    connect(addRunListButton,&QPushButton::clicked, this, &EDPSubSurfFaultRupWidget::handleAddButtonPressed);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum);

    QGridLayout* gridLayout = new QGridLayout(groupBox);

    gridLayout->addWidget(toAssessCheckBox,0,0,1,2);
    gridLayout->addItem(smallVSpacer,0,1);
    gridLayout->addWidget(ModelLabel,1,0);
    gridLayout->addWidget(modelSelectCombo,1,1);
    gridLayout->addWidget(notesLabel,2,0,1,2,Qt::AlignCenter);
    gridLayout->addWidget(weightLabel,3,0);
    gridLayout->addWidget(weightLineEdit,3,1);
    gridLayout->addWidget(addRunListButton,4,0,1,2,Qt::AlignCenter);
    gridLayout->addItem(vspacer, 5, 0);
    gridLayout->addItem(hspacer, 0, 1,6,1);

    return groupBox;
}


void EDPSubSurfFaultRupWidget::handleAddButtonPressed(void)
{
    QString item = modelSelectCombo->currentText();
    QString model = modelSelectCombo->currentData().toString();
    double weight = weightLineEdit->text().toDouble();

    listWidget->addItem(item, model, weight);
}


bool EDPSubSurfFaultRupWidget::outputToJSON(QJsonObject &jsonObj)
{
    QJsonObject outputObj;

    outputObj.insert("ToAssess", toAssessCheckBox->isChecked());

    auto modelsList = listWidget->getListOfModels();
    auto weightsList = listWidget->getListOfWeights();

    QJsonArray methods = QJsonArray::fromVariantList(modelsList);
    QJsonArray weights = QJsonArray::fromVariantList(weightsList);

    outputObj.insert("ListOfMethods",methods);
    outputObj.insert("ListOfWeights",weights);

    QJsonObject otherParamsObj;

    outputObj.insert("OtherParameters",otherParamsObj);

    jsonObj.insert("SubsurfaceFaultRupture",outputObj);

    return true;
}


bool EDPSubSurfFaultRupWidget::inputFromJSON(QJsonObject &/*jsonObject*/)
{

    return false;
}
