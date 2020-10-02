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


#include "sectiontitle.h"
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

SourceCharacterizationWidget::SourceCharacterizationWidget(QWidget *parent)
    : SimCenterAppWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Source Characterization"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(0);

    auto sourceLayout = this->getSourceLayout();

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addLayout(sourceLayout);
    mainLayout->addStretch();

    this->setLayout(mainLayout);
    this->setMinimumWidth(640);
    this->setMaximumWidth(1000);
}

SourceCharacterizationWidget::~SourceCharacterizationWidget()
{

}


bool SourceCharacterizationWidget::outputToJSON(QJsonObject &jsonObject)
{

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

    QRadioButton *button1 = new QRadioButton("Preferred model: UCERF 3.X (Reference)");
    QRadioButton *button2 = new QRadioButton("Manually define rupture scenario (available if customized run is selected under “General Information”)");
    QRadioButton *button3 = new QRadioButton("Import rupture scenarios from CSV files (the CSV files should follow the headers shown in the summary tables below)");
    button1->setChecked(true);

    // Segments import objects
    QLabel* segmentsPathText = new QLabel("List of Segments:");

    auto segementsPathLineEdit = new QLineEdit();
    segementsPathLineEdit->setMaximumWidth(750);
    segementsPathLineEdit->setMinimumWidth(400);
    segementsPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *segmentsBrowseButton = new QPushButton();
    segmentsBrowseButton->setText(tr("Browse"));
    segmentsBrowseButton->setMaximumWidth(150);

    QPushButton *segmentsFileButton = new QPushButton();
    segmentsFileButton->setText(tr("Import"));
    segmentsFileButton->setMaximumWidth(150);

    // Ruptures import objects
    QLabel* RupturesPathText = new QLabel();
    RupturesPathText->setText("List of Ruptures:");

    auto rupturesPathLineEdit = new QLineEdit();
    rupturesPathLineEdit->setMaximumWidth(750);
    rupturesPathLineEdit->setMinimumWidth(400);
    rupturesPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *RupturesBrowseButton = new QPushButton();
    RupturesBrowseButton->setText(tr("Browse"));
    RupturesBrowseButton->setMaximumWidth(150);

    QPushButton *RupturesFileButton = new QPushButton();
    RupturesFileButton->setText(tr("Import"));
    RupturesFileButton->setMaximumWidth(150);

    // Add a horizontal spacer after the browse and load buttons
    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);

    QLabel* rupSegmentsLabel = new QLabel("List of Rupture Segments");
    rupSegmentsLabel->setStyleSheet("font-weight: bold; color: black");

    // Headings for the rupture segment table
    QStringList ruptureSegmentHeadings;
    ruptureSegmentHeadings << tr("Rupture ID") << tr("Mw") << tr("Rate")<< tr("No. of Segments")<< tr("Segment 1")<< tr("Segment 2")<< tr("Segment 3");

    // Create rupture segment table
    auto tableRuptureSegments = new QTableWidget(5, ruptureSegmentHeadings.size());
    tableRuptureSegments->setHorizontalHeaderLabels(ruptureSegmentHeadings);
    tableRuptureSegments->verticalHeader()->setVisible(false);
    tableRuptureSegments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QLabel* rupScenarioLabel = new QLabel("List of Rupture Scenarios");
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
    QLabel* exportLabel = new QLabel("Export folder:");

    auto exportPathLineEdit = new QLineEdit();
    exportPathLineEdit->setMaximumWidth(750);
    exportPathLineEdit->setMinimumWidth(400);
    exportPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *exportBrowseFileButton = new QPushButton();
    exportBrowseFileButton->setText(tr("Browse"));
    exportBrowseFileButton->setMaximumWidth(150);

    QPushButton *exportFileButton = new QPushButton();
    exportFileButton->setText(tr("Export"));
    exportFileButton->setMaximumWidth(150);

    QGridLayout* gridLayout = new QGridLayout();

    gridLayout->addItem(smallVSpacer,0,0,1,5);
    gridLayout->addWidget(button1,1,0,1,5);
    gridLayout->addWidget(button2,2,0,1,5);
    gridLayout->addWidget(button3,3,0,1,5);
    gridLayout->addWidget(segmentsPathText,4,0);
    gridLayout->addWidget(segementsPathLineEdit,4,1);
    gridLayout->addWidget(segmentsBrowseButton,4,2);
    gridLayout->addWidget(segmentsFileButton,4,3);
    gridLayout->addItem(hspacer, 4, 4);
    gridLayout->addWidget(RupturesPathText,5,0);
    gridLayout->addWidget(rupturesPathLineEdit,5,1);
    gridLayout->addWidget(RupturesBrowseButton,5,2);
    gridLayout->addWidget(RupturesFileButton,5,3);
    gridLayout->addItem(smallVSpacer,6,0,1,5);
    gridLayout->addWidget(rupSegmentsLabel,7,0,1,5,Qt::AlignCenter);
    gridLayout->addWidget(tableRuptureSegments,8,0,1,5);
    gridLayout->addItem(smallVSpacer,9,0,1,5);
    gridLayout->addWidget(rupScenarioLabel,10,0,1,5,Qt::AlignCenter);
    gridLayout->addWidget(tableRuptureScenario,11,0,1,5);
    gridLayout->addWidget(exportLabel,12,0);
    gridLayout->addWidget(exportPathLineEdit,12,1);
    gridLayout->addWidget(exportBrowseFileButton,12,2);
    gridLayout->addWidget(exportFileButton,12,3);

    return gridLayout;
}





