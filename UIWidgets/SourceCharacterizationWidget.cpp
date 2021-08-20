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


#include "SimCenterWidget.h"
#include "SourceCharacterizationWidget.h"

#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollArea>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

SourceCharacterizationWidget::SourceCharacterizationWidget(QWidget *parent)
    : SimCenterAppWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    auto sourceLayout = this->getSourceLayout();

    mainLayout->addLayout(sourceLayout);
    mainLayout->addStretch();

    this->setLayout(mainLayout);
//    this->setMinimumWidth(640);
    this->setMaximumWidth(1000);
}

SourceCharacterizationWidget::~SourceCharacterizationWidget()
{

}


bool SourceCharacterizationWidget::outputToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool SourceCharacterizationWidget::inputFromJSON(QJsonObject &jsonObject)
{
    return true;
}


bool SourceCharacterizationWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool SourceCharacterizationWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    return true;
}


bool SourceCharacterizationWidget::copyFiles(QString &destDir)
{
    return false;
}


QGridLayout* SourceCharacterizationWidget::getSourceLayout(void)
{
    auto smallVSpacer = new QSpacerItem(0,10);

    QLabel *headingLabel = new QLabel("Import rupture scenarios from CSV files (the CSV files should follow the headers shown in the summary tables below)", this);

    // Segments import objects
    QLabel* segmentsPathText = new QLabel("List of Segments:", this);

    auto segementsPathLineEdit = new QLineEdit();
    segementsPathLineEdit->setMaximumWidth(750);
    segementsPathLineEdit->setMinimumWidth(400);
    segementsPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *segmentsBrowseButton = new QPushButton(this);
    segmentsBrowseButton->setText(tr("Browse"));
    segmentsBrowseButton->setMaximumWidth(150);

    QPushButton *segmentsFileButton = new QPushButton(this);
    segmentsFileButton->setText(tr("Import"));
    segmentsFileButton->setMaximumWidth(150);

    // Ruptures import objects
    QLabel* RupturesPathText = new QLabel(this);
    RupturesPathText->setText("List of Ruptures:");

    auto rupturesPathLineEdit = new QLineEdit(this);
    rupturesPathLineEdit->setMaximumWidth(750);
    rupturesPathLineEdit->setMinimumWidth(400);
    rupturesPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *RupturesBrowseButton = new QPushButton(this);
    RupturesBrowseButton->setText(tr("Browse"));
    RupturesBrowseButton->setMaximumWidth(150);

    QPushButton *RupturesFileButton = new QPushButton(this);
    RupturesFileButton->setText(tr("Import"));
    RupturesFileButton->setMaximumWidth(150);

    // Add a horizontal spacer after the browse and load buttons
    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);

    QLabel* rupSegmentsLabel = new QLabel("List of Rupture Scenarios", this);
    rupSegmentsLabel->setStyleSheet("font-weight: bold; color: black");

    // Headings for the rupture segment table
    QStringList ruptureSegmentHeadings;
    ruptureSegmentHeadings << tr("Rupture ID") << tr("Mw") << tr("Rate")<< tr("No. of Segments")<< tr("Segment 1")<< tr("Segment 2")<< tr("Segment 3");

    // Create rupture segment table
    auto tableRuptureSegments = new QTableWidget(5, ruptureSegmentHeadings.size());
    tableRuptureSegments->setHorizontalHeaderLabels(ruptureSegmentHeadings);
    tableRuptureSegments->verticalHeader()->setVisible(false);
    tableRuptureSegments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QLabel* rupScenarioLabel = new QLabel("List of Rupture Segments", this);
    rupScenarioLabel->setStyleSheet("font-weight: bold; color: black");

    // Headings for the rupture scenario table
    QStringList ruptureScenarioHeadings;
    ruptureScenarioHeadings << tr("Segment ID") << tr("No. of Traces") << tr("Trace 1\nLon")<< tr("Trace 1\nLat")<< tr("Trace 1\nZ");

    // Create rupture scenario table
    auto tableRuptureScenario = new QTableWidget(4, ruptureScenarioHeadings.size());
    tableRuptureScenario->setHorizontalHeaderLabels(ruptureScenarioHeadings);
    tableRuptureScenario->verticalHeader()->setVisible(false);
    tableRuptureScenario->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Export objects
    QLabel* exportLabel = new QLabel("Export folder:", this);

    auto exportPathLineEdit = new QLineEdit(this);
    exportPathLineEdit->setMaximumWidth(750);
    exportPathLineEdit->setMinimumWidth(400);
    exportPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *exportBrowseFileButton = new QPushButton(this);
    exportBrowseFileButton->setText(tr("Browse"));
    exportBrowseFileButton->setMaximumWidth(150);

    QPushButton *exportFileButton = new QPushButton(this);
    exportFileButton->setText(tr("Export"));
    exportFileButton->setMaximumWidth(150);

    QGridLayout* gridLayout = new QGridLayout();

    gridLayout->addItem(smallVSpacer,0,0,1,5);
    gridLayout->addWidget(headingLabel,1,0,1,5);
    gridLayout->addWidget(segmentsPathText,2,0);
    gridLayout->addWidget(segementsPathLineEdit,2,1);
    gridLayout->addWidget(segmentsBrowseButton,2,2);
    gridLayout->addWidget(segmentsFileButton,2,3);
    gridLayout->addItem(hspacer, 2, 4);
    gridLayout->addWidget(RupturesPathText,3,0);
    gridLayout->addWidget(rupturesPathLineEdit,3,1);
    gridLayout->addWidget(RupturesBrowseButton,3,2);
    gridLayout->addWidget(RupturesFileButton,3,3);
    gridLayout->addItem(smallVSpacer,4,0,1,5);
    gridLayout->addWidget(rupSegmentsLabel,5,0,1,5,Qt::AlignCenter);
    gridLayout->addWidget(tableRuptureSegments,5,0,1,5);
    gridLayout->addItem(smallVSpacer,6,0,1,5);
    gridLayout->addWidget(rupScenarioLabel,7,0,1,5,Qt::AlignCenter);
    gridLayout->addWidget(tableRuptureScenario,8,0,1,5);
    gridLayout->addWidget(exportLabel,9,0);
    gridLayout->addWidget(exportPathLineEdit,9,1);
    gridLayout->addWidget(exportBrowseFileButton,9,2);
    gridLayout->addWidget(exportFileButton,9,3);

    return gridLayout;
}





