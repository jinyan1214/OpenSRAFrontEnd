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


#include "AddToRunListWidget.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QJsonObject>

AddToRunListWidget::AddToRunListWidget(QWidget* parent) : QWidget(parent)
{

    // The model weight
    QDoubleValidator* validator1 = new QDoubleValidator(this);
    validator1->setRange(0.0,1.0,5);

    auto weightLabel = new QLabel("Model Weight:");
    weightLineEdit = new QLineEdit(this);
    weightLineEdit->setText("1.0");
    weightLineEdit->setValidator(validator1);
    weightLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    // Aleatory uncertainty
    auto aleatLabel = new QLabel("Aleatory Variability:");
    aleatoryLE = new QLineEdit();
    aleatoryLE->setPlaceholderText("Preferred");
    aleatoryLE->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    QDoubleValidator* validator2 = new QDoubleValidator(this);
    aleatoryLE->setValidator(validator2);
    aleatoryLE->setEnabled(true);

    // Epistemic uncertainty
    auto episLabel = new QLabel("Epistemic Uncertainty:");
    episLE = new QLineEdit();
    episLE->setPlaceholderText("Preferred");
    episLE->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    QDoubleValidator* validator3 = new QDoubleValidator(this);
    episLE->setValidator(validator3);
    episLE->setEnabled(true);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setMargin(0);
    inputLayout->addWidget(weightLabel);
    inputLayout->addWidget(weightLineEdit);

    inputLayout->addWidget(aleatLabel);
    inputLayout->addWidget(aleatoryLE);

    inputLayout->addWidget(episLabel);
    inputLayout->addWidget(episLE);

    auto smallVSpacer = new QSpacerItem(0,20);

    QPushButton *addRunListButton = new QPushButton(this);
    addRunListButton->setText(tr("Add run to list"));
    addRunListButton->setMinimumWidth(250);

    connect(addRunListButton,&QPushButton::clicked, this, [=](){emit addToRunListButtonPressed();});

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    mainLayout->addItem(smallVSpacer);

    mainLayout->addLayout(inputLayout,Qt::AlignCenter);

    mainLayout->addWidget(addRunListButton,Qt::AlignCenter);

    mainLayout->addStretch();

}


bool AddToRunListWidget::outputToJSON(QJsonObject &jsonObj)
{
    double weight = this->getWeight();
    jsonObj["ModelWeight"] = weight;

    if(aleatoryLE->text().isEmpty())
    {
        jsonObj["Aleatory"] = aleatoryLE->placeholderText();
    }
    else
    {
        double aleatory = this->getAleatoryVariability();
        jsonObj["Aleatory"] = aleatory;
    }

    // Return preferred if the user did not enter anything
    if(episLE->text().isEmpty())
    {
        jsonObj["Epistemic"] = episLE->placeholderText();
    }
    else
    {
        double epistemic = this->getEpistemicUncertainty();
        jsonObj["Epistemic"] = epistemic;
    }

    return true;
}


void AddToRunListWidget::clear()
{
    aleatoryLE->clear();
    episLE->clear();
    weightLineEdit->clear();

    weightLineEdit->setText("1.0");
}


bool AddToRunListWidget::inputFromJSON(QJsonObject &jsonObj)
{
    auto modWeight = jsonObj.value("ModelWeight").toDouble();
    weightLineEdit->setText(QString::number(modWeight));

    if (jsonObj.contains("Aleatory"))
    {
        auto modAV = jsonObj.value("Aleatory");
        if(modAV.type() == QJsonValue::Double)
            aleatoryLE->setText(QString::number(modAV.toDouble()));
        else if (modAV.type() == QJsonValue::String)
            aleatoryLE->setText(modAV.toString());
    }
    else
        aleatoryLE->setPlaceholderText("Preferred");


    if (jsonObj.contains("Epistemic"))
    {
        auto modEpis= jsonObj.value("Epistemic");
        if(modEpis.type() == QJsonValue::Double)
            episLE->setText(QString::number(modEpis.toDouble()));
        else if (modEpis.type() == QJsonValue::String)
            episLE->setText(modEpis.toString());
    }
    else
        episLE->setPlaceholderText("Preferred");

    return true;
}


double AddToRunListWidget::getWeight(void)
{
    return weightLineEdit->text().toDouble();
}


double AddToRunListWidget::getAleatoryVariability(void)
{
    return aleatoryLE->text().toDouble();
}


double AddToRunListWidget::getEpistemicUncertainty(void)
{
    return episLE->text().toDouble();
}


void AddToRunListWidget::setWeight(QString& val)
{

    weightLineEdit->setText(val);
}


void AddToRunListWidget::setAleatoryVariability(QString& val)
{
    aleatoryLE->setText(val);
}


void AddToRunListWidget::setEpistemicUncertainty(QString& val)
{
    episLE->setText(val);
}
