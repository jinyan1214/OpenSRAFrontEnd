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
    //samplingMethod->setMaximumWidth(800);
    //samplingMethod->setMinimumWidth(800);
    samplingMethod->addItem(tr("LHS"));
    samplingMethod->addItem(tr("Monte Carlo"));
    samplingMethod->addItem(tr("Polynomial Chaos Expansion"));

    methodLayout->addWidget(label1);
    methodLayout->addWidget(samplingMethod,2);
    methodLayout->addStretch(4);

    layout->addLayout(methodLayout);

    //
    // qstacked widget to hold all widgets
    //

    theStackedWidget = new QStackedWidget();

    theMC = new MonteCarloSamplingWidget();
    theStackedWidget->addWidget(theMC);

    layout->addWidget(theStackedWidget);

    connect(samplingMethod, SIGNAL(currentTextChanged(QString)), this, SLOT(onTextChanged(QString)));

}


UncertaintyQuantificationWidget::~UncertaintyQuantificationWidget()
{

}


void UncertaintyQuantificationWidget::onTextChanged(const QString &text)
{
    if (text=="LHS") {
        theStackedWidget->setCurrentIndex(0);
    }
    else if (text=="Monte Carlo") {
        theStackedWidget->setCurrentIndex(1);
    }
}


bool UncertaintyQuantificationWidget::outputToJSON(QJsonObject &jsonObject)
{
    bool result = true;

//    QJsonObject uq;
//    uq["method"]=samplingMethod->currentText();
//    theCurrentMethod->outputToJSON(uq);

//    jsonObject["samplingMethodData"]=uq;

    return result;
}


bool UncertaintyQuantificationWidget::inputFromJSON(QJsonObject &jsonObject)
{
    bool result = false;


    return result;
}







