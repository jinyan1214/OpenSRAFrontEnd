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
// Latest revision: 10.01.2020

#include "DamageMeasureWidget.h"
#include "DMPipeStrainWidget.h"
#include "sectiontitle.h"
#include "SimCenterComponentSelection.h"

#include <QListWidget>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QComboBox>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>

DamageMeasureWidget::DamageMeasureWidget(QWidget *parent): SimCenterAppWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Damage Measure (DM)"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);

    theHeaderLayout->addStretch(1);
    mainLayout->addLayout(theHeaderLayout);

    auto theComponentSelection = new SimCenterComponentSelection(this);

    DMPipeStrain = new DMPipeStrainWidget(this);

    theComponentSelection->addComponent("Pipe Strain",DMPipeStrain);
    theComponentSelection->addComponent("DM 2",this->getDM2Widget());

    theComponentSelection->displayComponent("Pipe Strain");

    theComponentSelection->setWidth(120);

    mainLayout->addWidget(theComponentSelection);

}


DamageMeasureWidget::~DamageMeasureWidget()
{

}


bool DamageMeasureWidget::outputToJSON(QJsonObject &jsonObject)
{
    QJsonObject outputObj;

    QJsonObject typeObj;

    DMPipeStrain->outputToJSON(typeObj);

    outputObj.insert("Type",typeObj);

    jsonObject.insert("DamageMeasure",outputObj);

    return true;
}


bool DamageMeasureWidget::inputFromJSON(QJsonObject &jsonObject)
{
    return false;
}


bool DamageMeasureWidget::copyFiles(QString &destDir)
{
    return false;
}


QGroupBox* DamageMeasureWidget::getDM2Widget(void)
{
    QGroupBox* groupBox = new QGroupBox("DM 2");
    groupBox->setFlat(true);

    return groupBox;
}

void DamageMeasureWidget::clear(void)
{
    DMPipeStrain->clear();
}

