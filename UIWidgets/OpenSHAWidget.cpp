#include "OpenSHAWidget.h"

#include <QComboBox>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QCheckBox>

OpenSHAWidget::OpenSHAWidget(QWidget* parent) : SimCenterAppWidget(parent)
{   
    QGridLayout* theMainLayout = new QGridLayout(this);

    auto GMCharacLabel = new QLabel("Seismic Source and Ground Motion Characterization", this);
    GMCharacLabel->setStyleSheet("font-weight: bold; color: black");

    auto ModelLabel = new QLabel("Ground Motion Model:", this);
    modelSelectCombo = new QComboBox(this);
    modelSelectCombo->addItem("Abrahamson, Silva & Kamai (2014)","Abrahamson, Silva & Kamai (2014)");
    modelSelectCombo->addItem("Boore, Stewart, Seyhan & Atkinson (2014)","Boore, Stewart, Seyhan & Atkinson (2014)");
    modelSelectCombo->addItem("Campbell & Bozorgnia (2014)","Campbell & Bozorgnia (2014)");
    modelSelectCombo->addItem("Chiou & Youngs (2014)","Chiou & Youngs (2014)");
    modelSelectCombo->addItem("NGAWest2 2014 Averaged Attenuation Relationship","NGAWest2 2014 Averaged Attenuation Relationship");
    modelSelectCombo->addItem("NGAWest2 2014 Averaged No Idriss (Preferred)","NGAWest2 2014 Averaged No Idriss");
    modelSelectCombo->setCurrentText("NGAWest2 2014 Averaged No Idriss (Preferred)");

    auto seismicSourceLabel = new QLabel("Seismic Source Model:");
    seismicSourceCombo = new QComboBox(this);
    seismicSourceCombo->addItem("Mean UCERF3","Mean UCERF3");
    seismicSourceCombo->addItem("Mean UCERF3 FM3.1 (Preferred)","Mean UCERF3 FM3.1");
    seismicSourceCombo->addItem("Mean UCERF3 FM3.2","Mean UCERF3 FM3.2");
    seismicSourceCombo->setCurrentText("Mean UCERF3 FM3.1 (Preferred)");

    auto vs30Label = new QLabel("Source for Vs30:");
    vs30Combo = new QComboBox(this);
    vs30Combo->addItem("User-Defined (Preferred)", "UserDefined");
    vs30Combo->addItem("Infer from CGS/Wills VS30 Map (2015) through OpenSHA", "Inferred");

    connect(vs30Combo, QOverload<int>::of(&QComboBox::currentIndexChanged),this,&OpenSHAWidget::handleVS30Change);

    vs30InfoLabel = new QLabel("NOTE: Vs30 must be specified in the SiteData input file if the \"User Defined\" option is selected");
    vs30InfoLabel->setStyleSheet("font-weight: bold; color: red");

    QGroupBox* filterGroupBox = new QGroupBox("Filter",this);
    auto filterGridLayout = new QGridLayout(filterGroupBox);

    RpCheckBox = new QCheckBox("Return Period [years]", this);
    RpCheckBox->setChecked(true);
    RpCheckBox->setStyleSheet("font-weight: bold; color: black");
    auto RpLabel = new QLabel("Maximum:", this);
    RpLineEdit = new QLineEdit(this);
    RpLineEdit->setText("1000");

    distCheckBox = new QCheckBox("Distance [km]", this);
    distCheckBox->setChecked(true);
    distCheckBox->setStyleSheet("font-weight: bold; color: black");
    auto distLabelMax = new QLabel("Maximum:", this);
    distLineEdit = new QLineEdit(this);
    distLineEdit->setText("100");

    magCheckBox = new QCheckBox("Magnitude", this);
    magCheckBox->setChecked(true);
    magCheckBox->setStyleSheet("font-weight: bold; color: black");
    auto magLabelMin = new QLabel("Minimum:", this);
    auto magLabelMax = new QLabel("Maximum:", this);
    magLineEditMin = new QLineEdit(this);
    magLineEditMax = new QLineEdit(this);
    magLineEditMin->setText("5");
    magLineEditMax->setText("8");

    pointSourceCheckBox = new QCheckBox("Exclude Point Source?", this);
    pointSourceCheckBox->setChecked(true);
    pointSourceCheckBox->setStyleSheet("font-weight: bold; color: black");

    filterGridLayout->addWidget(RpCheckBox,0,0,1,4);
    filterGridLayout->addWidget(RpLabel,1,0,1,2);
    filterGridLayout->addWidget(RpLineEdit,1,1,1,3);

    filterGridLayout->addWidget(distCheckBox,2,0,1,4);
    filterGridLayout->addWidget(distLabelMax,3,0,1,2);
    filterGridLayout->addWidget(distLineEdit,3,1,1,3);

    filterGridLayout->addWidget(magCheckBox,4,0,1,4);
    filterGridLayout->addWidget(magLabelMin,5,0);
    filterGridLayout->addWidget(magLineEditMin,5,1);
    filterGridLayout->addWidget(magLabelMax,5,2);
    filterGridLayout->addWidget(magLineEditMax,5,3);

    filterGridLayout->addWidget(pointSourceCheckBox,6,0,1,4);

    //    auto weightLabel = new QLabel("Weight:");
    //    auto weightLineEdit = new QLineEdit();
    //    weightLineEdit->setText("1");
    //    weightLineEdit->setMaximumWidth(100);

    //    QPushButton *addRunListButton = new QPushButton();
    //    addRunListButton->setText(tr("Add run to list"));
    //    addRunListButton->setMinimumWidth(250);

    theMainLayout->addWidget(GMCharacLabel,0,0,1,2);
    theMainLayout->addWidget(ModelLabel,1,0);
    theMainLayout->addWidget(modelSelectCombo,1,1);
    theMainLayout->addWidget(seismicSourceLabel,2,0);
    theMainLayout->addWidget(seismicSourceCombo,2,1);
    theMainLayout->addWidget(vs30Label,3,0);
    theMainLayout->addWidget(vs30Combo,3,1);
    theMainLayout->addWidget(vs30InfoLabel,4,0,1,2,Qt::AlignCenter);
    theMainLayout->addWidget(filterGroupBox,5,0,1,2);

}


void OpenSHAWidget::handleVS30Change(int index)
{

    if(index == 0)
        vs30InfoLabel->show();
    else
        vs30InfoLabel->hide();

}


bool OpenSHAWidget::outputToJSON(QJsonObject &jsonObj)
{
    jsonObj.insert("SourceForIM","OpenSHA");

    QJsonObject sourceParamObj;

    sourceParamObj.insert("SeismicSourceModel",seismicSourceCombo->currentText());
    sourceParamObj.insert("Vs30",vs30Combo->currentText());

    QJsonObject filterObj;

    QJsonObject returnPeriodObj;
    returnPeriodObj.insert("ToInclude",RpCheckBox->isChecked());
    returnPeriodObj.insert("Maximum",RpLineEdit->text());

    filterObj.insert("ReturnPeriod",returnPeriodObj);

    QJsonObject distanceObj;
    distanceObj.insert("ToInclude",distCheckBox->isChecked());
    distanceObj.insert("Maximum",distLineEdit->text());

    filterObj.insert("Distance",distanceObj);

    QJsonObject magObj;
    magObj.insert("ToInclude",magCheckBox->isChecked());
    magObj.insert("Minimum",magLineEditMin->text());
    magObj.insert("Maximum",magLineEditMax->text());

    filterObj.insert("Magnitude",magObj);

    QJsonObject psObj;

    bool ToInclude = pointSourceCheckBox->isChecked() ? false : true;
    psObj.insert("ToInclude", ToInclude);

    filterObj.insert("PointSource",psObj);

    sourceParamObj.insert("Filter",filterObj);

    sourceParamObj.insert("GroundMotionModel",modelSelectCombo->currentText());

    jsonObj.insert("SourceParameters",sourceParamObj);

    return true;
}


bool OpenSHAWidget::inputFromJSON(QJsonObject &/*jsonObject*/){


    return false;
}

