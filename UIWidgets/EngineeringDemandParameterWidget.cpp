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

// Written by: Stevan Gavrilovic, SimCenter @ UC Berkeley

#include "EngineeringDemandParameterWidget.h"
#include "SimCenterJsonWidget.h"
#include "sectiontitle.h"
#include "SimCenterComponentSelection.h"
#include "WorkflowAppOpenSRA.h"

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

EngineeringDemandParameterWidget::EngineeringDemandParameterWidget(QJsonObject mainObj, QWidget *parent): SimCenterAppWidget(parent)
{
    this->setObjectName("EngineeringDemandParameter");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(5,0,0,0);

    theComponentSelection = new SimCenterComponentSelection(this);

    auto thisObj = mainObj.value("EngineeringDemandParameter").toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Could not find the 'EngineeringDemandParameter' key in the methods and params file");
        return;
    }

    auto keys = thisObj.keys();

    // Iterate through the objects to create the widgets
    for(auto&& key : keys)
    {
        auto currObj = thisObj[key].toObject();

        if(currObj.value("ToDisplay").toBool(false) == false)
            continue;

        auto nameToDisplay = currObj.value("NameToDisplay").toString();

        auto newWidget = new SimCenterJsonWidget(key, currObj, this);

        if(newWidget == nullptr)
        {
            this->errorMessage("Error creating the widget "+nameToDisplay);
            continue;
        }

        theComponentSelection->addComponent(nameToDisplay,newWidget);

        newWidget->setObjectName(key);

        vecWidgets.append(newWidget);
    }

    theComponentSelection->setWidth(120);
    theComponentSelection->setItemWidthHeight(120,70);

    theComponentSelection->displayComponent(0);

    mainLayout->addWidget(theComponentSelection);

}


EngineeringDemandParameterWidget::~EngineeringDemandParameterWidget()
{

}


bool EngineeringDemandParameterWidget::outputToJSON(QJsonObject &jsonObject)
{
    QJsonObject outputObj;

    QJsonObject typeObj;

    for(auto&& component : vecWidgets)
    {
        if(!component->outputToJSON(typeObj))
        {
            this->errorMessage("Error output to json for EDP "+component->objectName());
            return false;
        }
    }

    outputObj.insert("Type",typeObj);

    jsonObject.insert("EngineeringDemandParameter",outputObj);

    return true;
}


bool EngineeringDemandParameterWidget::inputFromJSON(QJsonObject &jsonObject)
{
    auto typeObj = jsonObject.value("Type").toObject();

    if(typeObj.isEmpty())
    {
        this->errorMessage("Could not find the 'Type' keyword in "+QString(__FUNCTION__));
        return false;
    }

    auto keys = typeObj.keys();

    for(auto&& key : keys)
    {
        auto obj = typeObj[key].toObject();

        auto thisComponent = this->getComponentFromName(key);

        if(!thisComponent)
        {
            this->errorMessage("Error could not find the component of the type "+key);
            return false;
        }

        auto res = thisComponent->inputFromJSON(obj);

        if(!res )
            return false;

    }

    return true;
}

SimCenterAppWidget* EngineeringDemandParameterWidget::getComponentFromName(const QString& name)
{
    for(auto&& it : vecWidgets)
    {
        auto objName = it->objectName();

        if(objName.compare(name) == 0)
            return it;
    }
    return nullptr;
}


bool EngineeringDemandParameterWidget::copyFiles(QString &destDir)
{
    Q_UNUSED(destDir);
    return true;
}


void EngineeringDemandParameterWidget::clear(void)
{
    for(auto&& it : vecWidgets)
        it->clear();
}

