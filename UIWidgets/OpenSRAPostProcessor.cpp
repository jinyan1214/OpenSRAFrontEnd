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

// Written by: Stevan Gavrilovic

#include "OpenSRAPostProcessor.h"
#include "TreeItem.h"
#include "QGISVisualizationWidget.h"
#include "MutuallyExclusiveListWidget.h"


#include <QVBoxLayout>
#include <QHeaderView>


// GIS headers
#include <qgslinesymbol.h>
#include <qgsmapcanvas.h>
#include <qgsrenderer.h>
#include <qgsgraduatedsymbolrenderer.h>
#include <qgslayertreeview.h>
#include <qgsdataprovider.h>
#include <qgsvectorlayer.h>
#include <qgsfields.h>


OpenSRAPostProcessor::OpenSRAPostProcessor(QWidget *parent, QGISVisualizationWidget* visWidget) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{

    const char *resultHeaderString =
            "Results:\n"
            "- Showing the mean annual rate of occurrence of the loss metric\n"
            "  (i.e., number of occurrences per year across all events)\n";
//            "- The \"Modify Legend\" widget below is being updated;\n"
//            "  for now, modify the legend by right-clicking on the layer\n"
//            "  and choosing \"Properties...\"";
//            "- If the annual rates of events are set to 1 (e.g., ShakeMaps),\n"
//            "  then the results correspond to probability.";
//            "Mean annual rate of occurrence\n"
//            "= (prob(decision metric) x annual rate of event)\n"
//            "  summed over all events";
    listWidget = new MutuallyExclusiveListWidget(this, resultHeaderString);
    listWidget->setWordWrap(true);

    // initialize IM Source type for graduated renderer bin limits
    this->IMSourceType = "Not ShakeMap";
    this->infraType = "below_ground";
    this->initIMSourceType = "Not ShakeMap";
    this->initInfraType = "below_ground";

    connect(listWidget, &MutuallyExclusiveListWidget::itemChecked, this, &OpenSRAPostProcessor::handleListSelection);

    listWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    listWidget->header()->resizeSections(QHeaderView::ResizeToContents);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(listWidget);

}


void OpenSRAPostProcessor::handleModifyLegend(void)
{
    auto res_layer = results_layers.at(0);

    auto layerId = res_layer->id();

    theVisualizationWidget->handleLegendChange(layerId);
}


int OpenSRAPostProcessor::importResultVisuals(const QString& pathToResults)
{

    QFileInfo resultsDir(pathToResults);

    if (!resultsDir.exists())
        throw QString("Error, could not find the results file 'analysis_summary.gpkg' in the folder: "+pathToResults);

    results_layers = theVisualizationWidget->addVectorInGroup(pathToResults,"Results", "ogr");

    auto mean_layer = results_layers.value(0);

    auto data_provider = dynamic_cast<QgsVectorDataProvider*>(mean_layer->dataProvider());

    if(data_provider == nullptr)
    {
        this->errorMessage("Developer error: could not cast to vector data provider");
        return -1;
    }

    auto fieldsList = data_provider->fields();

    if (fieldsList.isEmpty())
    {
        this->errorMessage("Empty field list given by the data provider");
        return -1;
    }

    QList<QString> finalFieldsList;
    for(auto&& it : fieldsList)
    {
        auto displayName = it.displayName();

        if(!displayName.contains("case"))
            continue;

        finalFieldsList.push_back(displayName);
    }


    for(auto&& it : finalFieldsList)
    {

        auto field_idx = fieldsList.indexOf(it);
        auto field = fieldsList.at(field_idx);

        QString displayName = field.displayName();

        displayName.replace("_"," ");

        auto treeItem = listWidget->addItem(displayName);
        treeItem->setIsCheckable(true);

        treeItem->setProperty("HeaderString", field.name());
    }

    // get IM source type for determining graduated renderer bin limits
    auto res = this->getSetupConfigParams(pathToResults);
    if (res == -1)
    {
        this->errorMessage("Cannot find parameter(s) in SetupConfig.json to use in Results widget.");
        return res;
    }

    // Default check/select the first item
    listWidget->checkItem(0);
    listWidget->selectItem(0);

    listWidget->resizeColumnToContents(0);

    theVisualizationWidget->zoomToLayer(results_layers.value(0));
    QString name;
    for(auto&& it: results_layers)
    {
        name = it->name();
        if (name == "deformation_polygons_crossed")
            it->setOpacity(0.7);
        if (name == "caprocks_with_crossings")
            it->setOpacity(0.7);
    }

    return 0;
}


void OpenSRAPostProcessor::importResults(const QString& pathToResults)
{
    qDebug() << "OpenSRAPostProcessor: " << pathToResults;

    QString pathToGPKG = pathToResults + QDir::separator() + "analysis_summary.gpkg";
    this->importResultVisuals(pathToGPKG);

}


void OpenSRAPostProcessor::clear(void)
{
    listWidget->clear();
    results_layers.clear();

    this->IMSourceType = "Not UCERF";
    this->infraType = "below_ground";
    this->initIMSourceType = "Not UCERF";
    this->initInfraType = "below_ground";
}


void OpenSRAPostProcessor::handleListSelection(const TreeItem* itemSelected)
{
    if(itemSelected == nullptr)
        return;

    auto headerString = itemSelected->property("HeaderString").toString();

    if(headerString.isEmpty())
    {
        this->errorMessage("Could not find the property "+headerString+" in item "+itemSelected->getName());
        return;
    }

    auto res_layer = results_layers.at(0);

    if(res_layer == nullptr)
    {
        this->errorMessage("Dev error: Layer is a nullptr in "+QString(__FUNCTION__));
        return;
    }

    auto vector_layer = dynamic_cast<QgsVectorLayer*>(res_layer);

    if(vector_layer == nullptr)
    {
        this->errorMessage("Developer error: could not cast to vector layer in "+QString(__FUNCTION__));
        return;
    }

    auto data_provider = vector_layer->dataProvider();

    // Check to see if that field exists in the layer
    auto idx = data_provider->fieldNameIndex(headerString);

    if(idx == -1)
    {
        this->errorMessage("Could not find the field "+headerString+" in layer "+res_layer->name());
        return;
    }

    auto field = data_provider->fields().at(idx);

    auto layerRenderer = vector_layer->renderer();
    if(layerRenderer == nullptr)
    {
        this->errorMessage("No layer renderer available in layer "+res_layer->name());
        return;
    }

    if(field.isNumeric())
    {

        // Create a graduated renderer if one does not exist or if IMSourceType or infraType has been updated
        if(layerRenderer->type().compare("graduatedSymbol") != 0 || this->initIMSourceType != this->IMSourceType || this->initInfraType != this->infraType)
        {
            QVector<QPair<double,double>>classBreaks;
            QVector<QColor> colors;
            QVector<QString> labels;

            if (this->IMSourceType == "UCERF")
            {
                // rates can be on the order of 1e-6, -7, use log scale
//                classBreaks.append(QPair<double,double>(1.00E+01, 1.00E+03));
//                classBreaks.append(QPair<double,double>(1.00E-00, 1.00E+01));
//                classBreaks.append(QPair<double,double>(1.00E-01, 1.00E+00));
//                classBreaks.append(QPair<double,double>(1.00E-02, 1.00E-01));
//                classBreaks.append(QPair<double,double>(1.00E-03, 1.00E-02));
//                classBreaks.append(QPair<double,double>(1.00E-04, 1.00E-03));
//                classBreaks.append(QPair<double,double>(1.00E-05, 1.00E-04));
//                classBreaks.append(QPair<double,double>(1.00E-06, 1.00E-05));
//                classBreaks.append(QPair<double,double>(0.0     , 1.00E-06));

                classBreaks.append(QPair<double,double>(1.00E-01, 1.00E+05));
                classBreaks.append(QPair<double,double>(1.00E-02, 1.00E-01));
                classBreaks.append(QPair<double,double>(1.00E-03, 1.00E-02));
                classBreaks.append(QPair<double,double>(1.00E-04, 1.00E-03));
                classBreaks.append(QPair<double,double>(1.00E-05, 1.00E-04));
                classBreaks.append(QPair<double,double>(1.00E-06, 1.00E-05));
                classBreaks.append(QPair<double,double>(1.00E-07, 1.00E-06));
                classBreaks.append(QPair<double,double>(1.00E-08, 1.00E-07));
                classBreaks.append(QPair<double,double>(0.0     , 1.00E-08));

                colors.push_back(QColor(158,   1,  66));
                colors.push_back(QColor(220,  73,  75));
                colors.push_back(QColor(248, 142,  82));
                colors.push_back(QColor(253, 212, 129));
                colors.push_back(QColor(254, 254, 190));
                colors.push_back(QColor(213, 238, 155));
                colors.push_back(QColor(134, 206, 164));
                colors.push_back(QColor( 61, 148, 183));
                colors.push_back(QColor( 94,  79, 162));

                labels.append(">1e-1 failure per year");
                labels.append("1e-2 - 1e-1");
                labels.append("1e-3 - 1e-2");
                labels.append("1e-4 - 1e-3");
                labels.append("1e-5 - 1e-4");
                labels.append("1e-6 - 1e-5");
                labels.append("1e-7 - 1e-6");
                labels.append("1e-8 - 1e-7");
                labels.append("<1e-8 failure per year");
            }
            else
            {
                // generally showing just probabilities from 0 to 1 (rate = 1)
//                classBreaks.append(QPair<double,double>(1.0     , 1.00E+03));
                classBreaks.append(QPair<double,double>(0.8     , 1.00E+03));
                classBreaks.append(QPair<double,double>(0.6     , 0.8     ));
                classBreaks.append(QPair<double,double>(0.4     , 0.6     ));
                classBreaks.append(QPair<double,double>(0.2     , 0.4     ));
                classBreaks.append(QPair<double,double>(1.00E-02, 0.2     ));
                classBreaks.append(QPair<double,double>(0.0     , 1.00E-02));

                colors.push_back(QColor(158,   1,  66));
                colors.push_back(QColor(233,  92,  71));
                colors.push_back(QColor(253, 190, 110));
//                colors.push_back(QColor(254, 254, 190));
                colors.push_back(QColor(190, 229, 160));
                colors.push_back(QColor( 83, 173, 173));
                colors.push_back(QColor( 94,  79, 162));

                labels.append(">0.8");
                labels.append("0.6-0.8");
                labels.append("0.4-0.6");
                labels.append("0.2-0.4");
                labels.append("0.01-0.2");
                labels.append("<0.01");
            }

            // if below ground, use line as symbol, otherwise use marker as symbol (e.g., wells, above ground components)
            if (this->infraType == "below_ground")
                // createCustomClassBreakRenderer(const QString attrName, const QVector<QPair<double,double>>& classBreaks, const QVector<QColor>& colors, QgsVectorLayer * vlayer)
//                theVisualizationWidget->createCustomClassBreakRenderer(headerString,vector_layer,Qgis::SymbolType::Line,classBreaks,colors,QVector<QString>(),1.0);
                theVisualizationWidget->createCustomClassBreakRenderer(headerString,vector_layer,Qgis::SymbolType::Line,classBreaks,colors,labels,1.0);
            else
                theVisualizationWidget->createCustomClassBreakRenderer(headerString,vector_layer,Qgis::SymbolType::Marker,classBreaks,colors,QVector<QString>(),2.0);

            // update initIMSourceType and initInfraType
            this->initIMSourceType = this->IMSourceType;
            this->initInfraType = this->infraType;

        }
        else if(auto graduatedRender = dynamic_cast<QgsGraduatedSymbolRenderer*>(layerRenderer))
        {
            graduatedRender->setClassAttribute(headerString);
        }
        else
        {
            this->errorMessage("Unrecognized type of layer renderer available in layer "+res_layer->name());
            return;
        }
    }
    else
    {
        this->statusMessage("TODO: implement category renderer for non-numeric fields in "+QString(__FUNCTION__));
    }


    theVisualizationWidget->markDirty();
}


int OpenSRAPostProcessor::getSetupConfigParams(const QString& pathToResults)
{
    // get path to setup config json
    QFileInfo resultsPathInfo(pathToResults);
    auto resultsPath = resultsPathInfo.absoluteDir().absolutePath();
    QFileInfo workPathInfo(resultsPath);
    auto workPath = workPathInfo.absoluteDir().absolutePath();
    QString pathToSetupConfig = workPath +
            QDir::separator() + "Input" +
            QDir::separator() + "SetupConfig.json";

    // read from json file
    QFile jsonFile(pathToSetupConfig);
    jsonFile.open(QFile::ReadOnly);
    QJsonDocument inputDoc = QJsonDocument::fromJson(jsonFile.readAll());
    QJsonObject jsonObj = inputDoc.object();

    // get IM source type string
    auto IMObject = jsonObj.value("IntensityMeasure").toObject();
    auto IMSourceObject = IMObject.value("SourceForIM").toObject();
    auto IMSourceTypes = IMSourceObject.keys();
    if(IMSourceTypes.isEmpty())
    {
        this->errorMessage("Processing results... SourceForIM in SetupConfig.json is empty.");
        return -1;
    }
    this->IMSourceType = IMSourceTypes.first();

    // get infrastructure type
    auto infraObject = jsonObj.value("Infrastructure").toObject();
    if(infraObject.isEmpty())
    {
        this->errorMessage("Processing results... infraObject in SetupConfig.json is empty.");
        return -1;
    }
    this->infraType = infraObject.value("InfrastructureType").toString();

    return 0;
}
