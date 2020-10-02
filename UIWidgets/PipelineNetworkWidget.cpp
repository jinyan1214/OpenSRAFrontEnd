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
// Latest revision: 09.30.2020

#include "PipelineNetworkWidget.h"
#include "sectiontitle.h"
#include "SimCenterComponentSelection.h"

// GIS headers
#include "Basemap.h"
#include "Map.h"
#include "MapGraphicsView.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QTableWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>

using namespace Esri::ArcGISRuntime;

PipelineNetworkWidget::PipelineNetworkWidget(QWidget *parent)
    : SimCenterAppWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Pipeline Network"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);

    theHeaderLayout->addStretch(1);
    mainLayout->addLayout(theHeaderLayout);

    auto theComponentSelection = new SimCenterComponentSelection();
    mainLayout->addWidget(theComponentSelection);

    theComponentSelection->setMaxWidth(120);

    QGroupBox* pipelineInfoBox = this->getInputWidget();

    QGroupBox* visualizationBox = this->getVisualizationWidget();

    theComponentSelection->addComponent("Input",pipelineInfoBox);
    theComponentSelection->addComponent("Visualization",visualizationBox);

    theComponentSelection->displayComponent("Input");

    this->setLayout(mainLayout);
}


PipelineNetworkWidget::~PipelineNetworkWidget()
{

}


bool PipelineNetworkWidget::outputToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool PipelineNetworkWidget::inputFromJSON(QJsonObject &jsonObject)
{
    return false;
}


bool PipelineNetworkWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool PipelineNetworkWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    return false;
}


bool PipelineNetworkWidget::copyFiles(QString &destDir)
{
  return false;
}


QGroupBox* PipelineNetworkWidget::getInputWidget(void)
{
    QGroupBox* groupBox = new QGroupBox("Enter Pipeline Information");
    QGridLayout* gridLayout = new QGridLayout(this);
    groupBox->setLayout(gridLayout);
    groupBox->setFlat(true);

    auto smallVSpacer = new QSpacerItem(0,10);

    QLabel* topText = new QLabel();
    topText->setText("Load pipeline information from CSV File (headers in CSV file must match those shown in the table below)");

    QLabel* pathText = new QLabel();
    pathText->setText("Import Path:");

    auto workingDirectoryLineEdit = new QLineEdit(this);
    workingDirectoryLineEdit->setMaximumWidth(750);
    workingDirectoryLineEdit->setMinimumWidth(400);
    workingDirectoryLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);

    QPushButton *loadFileButton = new QPushButton();
    loadFileButton->setText(tr("Load CSV File"));
    loadFileButton->setMaximumWidth(150);

    // Add a horizontal spacer after the browse and load buttons
    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Text label that says pipeline data
    QLabel* pipeLineDataText = new QLabel("Pipeline Data");
    pipeLineDataText->setStyleSheet("font-weight: bold; color: black");


    // Create the table
    // Headings for the table
    QStringList headings;
    headings << tr("Latitude") << tr("Longitude") << tr("Length")<< tr("Material")<< tr("Placeholder_1")<< tr("Placeholder_2");

    auto tableWidget = new QTableWidget(10, headings.size(), this);
    tableWidget->setHorizontalHeaderLabels(headings);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addItem(smallVSpacer,0,0,1,5);
    gridLayout->addWidget(topText,1,0,1,5);
    gridLayout->addWidget(pathText,2,0);
    gridLayout->addWidget(workingDirectoryLineEdit,2,1);
    gridLayout->addWidget(browseFileButton,2,2);
    gridLayout->addWidget(loadFileButton,2,3);
    gridLayout->addItem(hspacer, 2, 4);
    gridLayout->addItem(smallVSpacer,3,0,1,5);
    gridLayout->addWidget(pipeLineDataText,4,0,1,5,Qt::AlignCenter);
    gridLayout->addWidget(tableWidget, 5, 0, 1, 5);
    gridLayout->addItem(vspacer, 6, 0);

    return groupBox;
}


QGroupBox* PipelineNetworkWidget::getVisualizationWidget(void)
{
    QGroupBox* groupBox = new QGroupBox("Visualize Pipeline Network");
    QGridLayout* layout = new QGridLayout(this);
    groupBox->setLayout(layout);
    groupBox->setFlat(true);

    auto smallVSpacer = new QSpacerItem(0,10);

    QLabel* topText = new QLabel();
    topText->setText("Use the following shapes\nto select a subset of\nsegments to analyze");
    topText->setStyleSheet("font-weight: bold; color: black; text-align: center");

    QPushButton *circleButton = new QPushButton();
    circleButton->setText(tr("Circle"));
    circleButton->setMaximumWidth(150);

    QPushButton *rectangleButton = new QPushButton();
    rectangleButton->setText(tr("Rectangle"));
    rectangleButton->setMaximumWidth(150);

    QLabel* bottomText = new QLabel();
    bottomText->setText("Click the “Apply” button to\nuse the subset of segments");
    bottomText->setStyleSheet("font-weight: bold; color: black; text-align: center");

    QPushButton *applyButton = new QPushButton();
    applyButton->setText(tr("Apply"));
    applyButton->setMaximumWidth(150);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    // Create the Widget view
    mapViewWidget = new MapGraphicsView(this);

    // Create a map using the topographic Basemap
    mapObject = new Map(Basemap::topographic(this), this);

    mapViewWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    // Set map to map view
    mapViewWidget->setMap(mapObject);

    layout->addItem(smallVSpacer,0,0,1,2);
    layout->addWidget(topText,1,0);
    layout->addWidget(circleButton,2,0);
    layout->addWidget(rectangleButton,3,0);
    layout->addWidget(bottomText,4,0);
    layout->addWidget(applyButton,5,0);
    layout->addItem(vspacer,6,0,1,1);

    layout->addWidget(mapViewWidget,0,1,8,2);

    return groupBox;
}


