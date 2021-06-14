/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written by: Stevan Gavrilovic

#include "IntensityMeasureWidget.h"
#include "sectiontitle.h"
#include "OpenSHAWidget.h"
#include "ShakeMapWidget.h"
#include "SourceCharacterizationWidget.h"
#include "WorkflowAppOpenSRA.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QStackedWidget>
#include <QComboBox>
#include <QListWidget>
#include <QSpacerItem>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>
#include <QStackedWidget>

IntensityMeasureWidget::IntensityMeasureWidget(VisualizationWidget* visWidget, QWidget *parent)
    : SimCenterAppWidget(parent)
{
    PGACheckbox = nullptr;
    PGVCheckbox = nullptr;

    IMSelectCombo = new QComboBox(this);
    IMSelectCombo->addItem("OpenSHA (Preferred)","OpenSHA");
    IMSelectCombo->addItem("ShakeMap","ShakeMap");
    //    IMSelectCombo->addItem("User-defined");
    IMSelectCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    connect(IMSelectCombo,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&IntensityMeasureWidget::IMSelectionChanged);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Intensity Measure (IM)"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addWidget(IMSelectCombo);

    auto IMBox = this->getIMBox();

    openSHA = new OpenSHAWidget(this);
    shakeMap = new ShakeMapWidget(visWidget, this);

    shakeMapStackedWidget = shakeMap->getShakeMapWidget();

    theSourceCharacterizationWidget = new SourceCharacterizationWidget(this);

    mainPanel = new QStackedWidget(this);
    mainPanel->addWidget(openSHA);
    mainPanel->addWidget(shakeMapStackedWidget);
    mainPanel->addWidget(theSourceCharacterizationWidget);

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addWidget(mainPanel);
    mainLayout->addWidget(IMBox);
    mainLayout->addStretch();

}


IntensityMeasureWidget::~IntensityMeasureWidget()
{

}


bool IntensityMeasureWidget::outputToJSON(QJsonObject &jsonObject)
{
    QJsonObject IMobj;

    auto index = IMSelectCombo->currentIndex();

    if(index == 0)
    {
        openSHA->outputToJSON(IMobj);
    }
    else if(index == 1)
    {
        shakeMap->outputToJSON(IMobj);
    }
    else
    {
        return false;
    }

    QJsonObject TypeObj;
    QJsonObject PGAObj;
    QJsonObject PGVObj;

    PGAObj.insert("ToAssess",PGACheckbox->isChecked());
    PGVObj.insert("ToAssess",PGVCheckbox->isChecked());

    TypeObj.insert("PGA",PGAObj);
    TypeObj.insert("PGV",PGVObj);

    IMobj.insert("Type",TypeObj);

    QJsonObject corrObj;
    QJsonObject spatCorrObj;
    QJsonObject specCorrObj;

    spatCorrObj.insert("ToInclude",spatialCorrCheckbox->isChecked());
    spatCorrObj.insert("Method",spatialCorrComboBox->currentData().toString());

    specCorrObj.insert("ToInclude",spectralCorrCheckbox->isChecked());
    specCorrObj.insert("Method",spectralCorrComboBox->currentData().toString());

    corrObj.insert("Spatial",spatCorrObj);
    corrObj.insert("Spectral",specCorrObj);

    IMobj.insert("Correlation",corrObj);

    jsonObject.insert("IntensityMeasure",IMobj);

    return true;
}


void IntensityMeasureWidget::clear()
{
    IMSelectCombo->setCurrentIndex(0);

    openSHA->clear();
    shakeMap->clear();
    theSourceCharacterizationWidget->clear();

    PGACheckbox->setChecked(false);
    PGVCheckbox->setChecked(false);
    spatialCorrCheckbox->setChecked(false);
    spectralCorrCheckbox->setChecked(false);

    spatialCorrComboBox->setCurrentIndex(0);
    spectralCorrComboBox->setCurrentIndex(0);
}


bool IntensityMeasureWidget::inputFromJSON(QJsonObject &jsonObject)
{

    auto IMSource = jsonObject["SourceForIM"].toString();

    auto sourceParamObj = jsonObject["SourceParameters"].toObject();

    if(sourceParamObj.isEmpty() || IMSource.isEmpty())
        return false;

    int index = IMSelectCombo->findData(IMSource);
    if (index != -1)
    {
       IMSelectCombo->setCurrentIndex(index);
    }

    if(IMSource == "OpenSHA")
    {
        openSHA->inputFromJSON(sourceParamObj);
    }
    else if(IMSource == "ShakeMap")
    {
        shakeMap->inputFromJSON(sourceParamObj);
    }
    else
        return false;

    auto typeObj = jsonObject["Type"].toObject();

    if(typeObj.isEmpty())
        return false;

    auto PGAObj = typeObj["PGA"].toObject();
    auto PGVObj = typeObj["PGV"].toObject();

    if(PGAObj.isEmpty() || PGVObj.isEmpty())
        return false;

    auto PGAChecked = PGAObj["ToAssess"].toBool();
    PGACheckbox->setChecked(PGAChecked);

    auto PGVChecked = PGVObj["ToAssess"].toBool();
    PGVCheckbox->setChecked(PGVChecked);

    QJsonObject corrObj = jsonObject["Correlation"].toObject();
    if(corrObj.isEmpty())
        return false;

    QJsonObject spatCorrObj = corrObj["Spatial"].toObject();
    QJsonObject specCorrObj = corrObj["Spectral"].toObject();

    auto includeSpatCorr = spatCorrObj["ToInclude"].toBool();
    spatialCorrCheckbox->setChecked(includeSpatCorr);

    auto spatCorrType = spatCorrObj["Method"].toString();

    int index2 = spatialCorrComboBox->findData(spatCorrType);
    if (index2 != -1)
    {
       spatialCorrComboBox->setCurrentIndex(index2);
    }

    auto includeSpecCorr = specCorrObj["ToInclude"].toBool();
    spectralCorrCheckbox->setChecked(includeSpecCorr);

    auto specCorrType = specCorrObj["Method"].toString();

    int index3 = spectralCorrComboBox->findData(specCorrType);
    if (index3 != -1)
    {
       spectralCorrComboBox->setCurrentIndex(index3);
    }

    return true;
}


void IntensityMeasureWidget::IMSelectionChanged(int index)
{
    if(index == 0)
    {
        mainPanel->setCurrentWidget(openSHA);
    }
    else if(index == 1)
    {
        mainPanel->setCurrentWidget(shakeMapStackedWidget);
    }
    else if(index == 2)
    {
        mainPanel->setCurrentWidget(theSourceCharacterizationWidget);
    }
}


bool IntensityMeasureWidget::copyFiles(QString &destDir)
{
    auto numMapsLoaded = shakeMap->getNumShakeMapsLoaded();
    if(numMapsLoaded != 0)
        shakeMap->copyFiles(destDir);

    return true;
}


QGroupBox* IntensityMeasureWidget::getIMBox(void)
{
    auto smallVSpacer = new QSpacerItem(0,10);

    QGroupBox* IMGroupBox = new QGroupBox(this);
    IMGroupBox->setFlat(true);

    auto IMLabel = new QLabel("Intensity Measures");
    IMLabel->setStyleSheet("font-weight: bold; color: black");

    PGACheckbox = new QCheckBox("Peak Ground Acceleration (PGA)");
    PGVCheckbox = new QCheckBox("Peak Ground Velocity (PGV)");

    PGACheckbox->setChecked(true);
    PGVCheckbox->setChecked(true);

    auto correlationsLabel = new QLabel("Correlations");
    correlationsLabel->setStyleSheet("font-weight: bold; color: black");

    spatialCorrCheckbox = new QCheckBox("Spatial Correlation");
    spectralCorrCheckbox = new QCheckBox("Spectral (Cross) Correlation");

    spatialCorrCheckbox->setChecked(true);
    spectralCorrCheckbox->setChecked(true);

    spatialCorrComboBox = new QComboBox(this);
    spatialCorrComboBox->addItem("Jayaram & Baker (2009)","JayaramBaker2009");

    spectralCorrComboBox = new QComboBox(this);
    spectralCorrComboBox->addItem("Baker & Jayaram (2008)","BakerJayaram2008");

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    QGridLayout* gridLayout = new QGridLayout(IMGroupBox);

    gridLayout->addWidget(IMLabel,0,0);

    gridLayout->addWidget(PGACheckbox,1,0);
    gridLayout->addWidget(PGVCheckbox,1,1);

    gridLayout->addItem(smallVSpacer,2,0);

    gridLayout->addWidget(correlationsLabel,3,0);

    gridLayout->addWidget(spatialCorrCheckbox,4,0);
    gridLayout->addWidget(spatialCorrComboBox,4,1);

    gridLayout->addWidget(spectralCorrCheckbox,5,0);
    gridLayout->addWidget(spectralCorrComboBox,5,1);

    gridLayout->addItem(vspacer, 6, 0);

    return IMGroupBox;
}
