/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
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

#include "ComponentInputWidget.h"
#include "MonteCarloSamplingWidget.h"
#include "FixedResidualsSamplingWidget.h"
#include "NoneWidget.h"
#include "SecondaryComponentSelection.h"
#include "SimCenterAppSelection.h"
#include "UncertaintyQuantificationWidget.h"
#include "VisualizationWidget.h"
#include "sectiontitle.h"

// Qt headers
#include <QCheckBox>
#include <QComboBox>
#include <QColorTransform>
#include <QDebug>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>

UncertaintyQuantificationWidget::UncertaintyQuantificationWidget(QWidget *parent)
    : SimCenterAppWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Uncertainty Quantification"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);

    layout->addLayout(theHeaderLayout);

    //
    // create layout for selection box for method type to layout
    //

    QHBoxLayout *methodLayout= new QHBoxLayout;
    QLabel *label1 = new QLabel();
    label1->setText(QString("Method"));
    samplingMethod = new QComboBox();
    samplingMethod->addItem("Latin Hypercube Sampling","Latin Hypercube Sampling");
    samplingMethod->addItem("Monte Carlo Random Sampling","MonteCarlo");
    samplingMethod->addItem("Fixed Residuals","FixedResiduals");
    samplingMethod->addItem("Polynomial Chaos Expansion (Currently Disabled)");
    samplingMethod->setCurrentIndex(0);

    methodLayout->addWidget(label1);
    methodLayout->addWidget(samplingMethod,2);
    methodLayout->addStretch(4);

    layout->addLayout(methodLayout);

    //
    // qstacked widget to hold all widgets
    //

    theStackedWidget = new QStackedWidget();

    theMonteCarloWidget = new MonteCarloSamplingWidget();
    theFixedResidualsWidget = new FixedResidualsSamplingWidget();

    theStackedWidget->addWidget(theMonteCarloWidget);
    theStackedWidget->addWidget(theFixedResidualsWidget);

    layout->addWidget(theStackedWidget);

    connect(samplingMethod, SIGNAL(currentTextChanged(QString)), this, SLOT(onTextChanged(QString)));

}


UncertaintyQuantificationWidget::~UncertaintyQuantificationWidget()
{

}


void UncertaintyQuantificationWidget::onTextChanged(const QString &text)
{
    if (text=="Latin Hypercube Sampling") {
        theStackedWidget->setCurrentWidget(theMonteCarloWidget);
    }
    else if (text=="Monte Carlo Random Sampling" || text=="MonteCarlo") {
        theStackedWidget->setCurrentWidget(theMonteCarloWidget);
    }
    else if (text=="Fixed Residuals" || text=="FixedResiduals") {
        theStackedWidget->setCurrentWidget(theFixedResidualsWidget);
    }
}


void  UncertaintyQuantificationWidget::clear()
{
    theFixedResidualsWidget->clear();
    theMonteCarloWidget->clear();
    samplingMethod->setCurrentIndex(0);
}


bool UncertaintyQuantificationWidget::outputToJSON(QJsonObject &jsonObject)
{    
    auto uqType = samplingMethod->currentData().toString();

    QJsonObject uq;

    uq.insert("Type","MonteCarlo");

    uq.insert("Algorithm",uqType);

    if(uqType == "Latin Hypercube Sampling" || uqType == "MonteCarlo")
    {

        theMonteCarloWidget->outputToJSON(uq);
    }
    else if (uqType == "FixedResiduals")
    {
        theFixedResidualsWidget->outputToJSON(uq);
    }

    jsonObject.insert("UncertaintyQuantification",uq);

    return true;
}


bool UncertaintyQuantificationWidget::inputFromJSON(QJsonObject &jsonObject)
{ 
    auto uqType = jsonObject["Algorithm"].toString();

    int index = samplingMethod->findData(uqType);
    if ( index != -1 )
    {
       samplingMethod->setCurrentIndex(index);
    }

    if(uqType == "Latin Hypercube Sampling" || uqType == "MonteCarlo")
    {
        theMonteCarloWidget->inputFromJSON(jsonObject);
    }
    else if (uqType == "FixedResiduals")
    {
        theFixedResidualsWidget->inputFromJSON(jsonObject);
    }

    return true;
}







