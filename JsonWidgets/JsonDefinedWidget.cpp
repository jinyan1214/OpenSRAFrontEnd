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
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

// Written by: Dr. Stevan Gavrilovic, UC Berkeley

#include "JsonDefinedWidget.h"
#include "WorkflowAppOpenSRA.h"

#include <QJsonArray>
#include <QLabel>
#include <QVariantList>
#include <QScrollArea>
#include <QVBoxLayout>

JsonDefinedWidget::JsonDefinedWidget(QWidget* parent, const QJsonObject& obj, const QString parentKey) : JsonWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setContentsMargins(0,0,0,0);

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto theWidgetFactory = WorkflowAppOpenSRA::getInstance()->getTheWidgetFactory();

    this->setMethodAndParamJsonObj(obj);

    auto params = obj["Params"].toObject();

    auto displayOrderArray = obj["DisplayOrder"].toArray();

    auto displayOrderVarList = displayOrderArray.toVariantList();

    QStringList displayOrder;

    for(auto&& varnt : displayOrderVarList)
        displayOrder.append(varnt.toString());

    auto paramsLayout = theWidgetFactory->getLayout(params, parentKey, parent, displayOrder);

    auto returnObj = obj["Return"].toObject();

    auto upstreamObj = obj["UpstreamDependency"].toObject();

    if(!returnObj.isEmpty())
    {
        auto returnStr = "\t"+returnObj.value("Description").toString();

        QLabel* returnLabel = new QLabel("Returns:");
        returnLabel->setStyleSheet("font-weight: bold; color: black");

        QLabel* returnVal = new QLabel(returnStr);

        layout->addWidget(returnLabel);
        layout->addWidget(returnVal);

        if(!upstreamObj.isEmpty())
        {
            QLabel* upstreamLabel = new QLabel("Upstream Dependencies:");
            upstreamLabel->setStyleSheet("font-weight: bold; color: black");

            auto upstreamStr = "\t"+upstreamObj.value("Description").toString();

            QLabel* upstreamVal = new QLabel(upstreamStr);

            layout->addWidget(upstreamLabel);
            layout->addWidget(upstreamVal);
        }

        QLabel* paramsLabel = new QLabel("Additional Input Parameters (see the \"Input Variables\" tab):");
        paramsLabel->setStyleSheet("font-weight: bold; color: black");

        layout->addWidget(paramsLabel);
    }

    layout->addLayout(paramsLayout);

    layout->addStretch(0);
//    layout->setStretch(0,1);
//    layout->setStretch(1,0);

}
