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

#include "GeospatialDataWidget.h"
#include "GenericModelWidget.h"
#include "GISMapWidget.h"

#include "UserInputCPTWidget.h"

GeospatialDataWidget::GeospatialDataWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppSelection(QString("Geospatial Data"),QString("UserSpecifiedData"), parent), visualizationWidget(visWidget)
{
    cptInputWidget = new UserInputCPTWidget(visualizationWidget);

    GISMapInputWidget = new GISMapWidget(visualizationWidget);

    this->addComponent("User Provided GIS Data", "GISDatasets", GISMapInputWidget);
    this->addComponent("Site Investigation Data (currently supporting CPTs only)", "CPTParameters", cptInputWidget);
}


GeospatialDataWidget::~GeospatialDataWidget()
{

}


void GeospatialDataWidget::clear(void)
{
    cptInputWidget->clear();
    GISMapInputWidget->clear();
}


bool GeospatialDataWidget::inputFromJSON(QJsonObject &jsonObject)
{

    auto GISObject = jsonObject.value("GISDatasets").toObject();
    auto CPTObject = jsonObject.value("CPTParameters").toObject();

//    if(GISObject.isEmpty())
//        return false;

    // GIS input
    auto GISParams = GISObject.keys();

    if(!GISParams.isEmpty())
    {
        auto res =  GISMapInputWidget->inputFromJSON(GISObject);
        if(res == false)
            return false;

        this->selectApp("GISDatasets");
    }


    // CPT input
    auto CPTParams = CPTObject.keys();

    if(!CPTParams.isEmpty())
    {
        auto res =  cptInputWidget->inputFromJSON(CPTObject);
        if(res == false)
            return false;

        this->selectApp("CPTParameters");
    }

    return true;
}



bool GeospatialDataWidget::outputToJSON(QJsonObject &rvObject)
{

    QJsonObject GISCPTObj;
    auto res = cptInputWidget->outputToJSON(GISCPTObj);

    if(!res)
    {
        this->errorMessage("Error in json output for CPT widget");
        return false;
    }

    res = GISMapInputWidget->outputToJSON(GISCPTObj);

    if(!res)
    {
        this->errorMessage("Error in json output for GIS map widget");
        return false;
    }

    rvObject["UserSpecifiedData"] = GISCPTObj;

    return true;
}



