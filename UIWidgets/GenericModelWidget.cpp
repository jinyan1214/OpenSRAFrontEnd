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

// Created by: Dr. Stevan Gavrilovic, UC Berkeley

#include "GenericModelWidget.h"
#include "RVTableView.h"
#include "RVTableModel.h"

#include "ComboBoxDelegate.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <sectiontitle.h>
#include <QLineEdit>
#include <QFileDialog>

GenericModelWidget::GenericModelWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setMargin(0);
    verticalLayout->setSpacing(0);
    this->makeRVWidget();
}


GenericModelWidget::~GenericModelWidget()
{

}


void GenericModelWidget::makeRVWidget(void)
{
    // title & add button
    QHBoxLayout *titleLayout = new QHBoxLayout();
    //titleLayout->setMargin(10);

    SectionTitle *title=new SectionTitle();
    title->setText(tr("Generic Model Definition"));
    title->setMinimumWidth(250);
    //    QSpacerItem *spacer1 = new QSpacerItem(50,10);
    //    QSpacerItem *spacer2 = new QSpacerItem(20,10);
    //    QSpacerItem *spacer3 = new QSpacerItem(20,10);
    //    QSpacerItem *spacer4 = new QSpacerItem(50,10);
    //    QSpacerItem *spacer5 = new QSpacerItem(20,10);


    //    QPushButton *addRV = new QPushButton();
    //    addRV->setMinimumWidth(75);
    //    addRV->setMaximumWidth(75);
    //    addRV->setText(tr("Add"));
    //    connect(addRV,SIGNAL(clicked()),this,SLOT(addRandomVariable()));


    //    QPushButton *removeRV = new QPushButton();
    //    removeRV->setMinimumWidth(75);
    //    removeRV->setMaximumWidth(75);
    //    removeRV->setText(tr("Remove"));
    //    connect(removeRV,SIGNAL(clicked()),this,SLOT(removeRandomVariable()));


    //    QPushButton *RVsFromJson = new QPushButton();
    //    RVsFromJson->setMinimumWidth(75);
    //    RVsFromJson->setMaximumWidth(75);
    //    RVsFromJson->setText(tr("Import"));
    //    RVsFromJson->setStyleSheet("background-color: dodgerblue;border-color:dodgerblue");
    //    connect(RVsFromJson,SIGNAL(clicked()),this,SLOT(loadRVsFromJson()));


    //    QPushButton *RVsToJson = new QPushButton();
    //    RVsToJson->setMinimumWidth(75);
    //    RVsToJson->setMaximumWidth(75);
    //    RVsToJson->setText(tr("Export"));
    //    RVsToJson->setStyleSheet("background-color: dodgerblue;border-color:dodgerblue");
    //    connect(RVsToJson,SIGNAL(clicked()),this,SLOT(saveRVsToJson()));

    titleLayout->addWidget(title);
    //    titleLayout->addItem(spacer1);
    //    titleLayout->addWidget(addRV);
    //    titleLayout->addItem(spacer2);
    //    titleLayout->addWidget(removeRV);
    //    titleLayout->addItem(spacer3);

    //    //titleLayout->addWidget(addCorrelation,0,Qt::AlignTop);
    //    QString appName = QApplication::applicationName();
    //    if (appName == "quoFEM") {
    //        titleLayout->addWidget(addCorrelation);
    //        titleLayout->addItem(spacer4);
    //    }

    //    titleLayout->addWidget(RVsToJson);
    //    titleLayout->addItem(spacer5);
    //    titleLayout->addWidget(RVsFromJson);
    //    titleLayout->addStretch();

    verticalLayout->addLayout(titleLayout);
    theRVTableView = new RVTableView();

    verticalLayout->addWidget(theRVTableView);

    RVTableModel* tableModel = theRVTableView->getTableModel();
    QStringList headers = {"Level","Coeff. Mean", "Coeff. Sigma","RV Label", "Apply Ln", "Power"};
    tableModel->setHeaderStringList(headers);
    theRVTableView->show();


    // Level
    levelComboDelegate = new ComboBoxDelegate(this);
    QStringList levelTypes = {"1","2","3"};
    levelComboDelegate->setItems(levelTypes);
    theRVTableView->setItemDelegateForColumn(0, levelComboDelegate);

    // Ln type
    applyLnComboDelegate = new ComboBoxDelegate(this);
    QStringList distTypes = {"true","false"};
    applyLnComboDelegate->setItems(distTypes);
    theRVTableView->setItemDelegateForColumn(4, applyLnComboDelegate);

    // Power
    powerComboDelegate = new ComboBoxDelegate(this);
    QStringList powerTypes = {"0","1","2"};
    powerComboDelegate->setItems(powerTypes);
    theRVTableView->setItemDelegateForColumn(5, powerComboDelegate);

//    theRVTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);

    // Test
    QVector<QVector<QVariant>> data;
    for(int i = 0; i<5; ++i)
    {

        // "Level","Coeff. Mean", "Coeff. Sigma","RV Label", "Apply Ln", "Power"
        QVector<QVariant> row(6);

        row[0] = rand() % 3 + 1;
        row[1] = QVariant(rand() % 5 + 1);
        row[2] = QVariant(0.5);
        row[3] = QVariant("RV"+QString::number(i));
        row[4] = rand() % 10 < 5 ? true : false;
        row[5] = QVariant(rand() % 2);

        // auto itObj = paramsObject.value(it).toObject();
        data.push_back(row);
    }

    tableModel->populateData(data);

    verticalLayout->addStretch(1);
}


void GenericModelWidget::clear(void) {

    theRVTableView->clear();

}


bool GenericModelWidget::outputToJSON(QJsonObject &rvObject) {

    bool result = true;
    //    QJsonArray rvArray;
    //    for (int i = 0; i <theRandomVariables.size(); ++i) {
    //        QJsonObject rv;
    //        if (theRandomVariables.at(i)->outputToJSON(rv)) {
    //            rvArray.append(rv);
    //        } else {
    //            qDebug() << "OUTPUT FAILED" << theRandomVariables.at(i)->variableName->text();
    //            result = false;
    //        }
    //    }

    //    rvObject["randomVariables"]=rvArray;

    return result;
}






void GenericModelWidget::copyFiles(QString fileDir)
{


}


bool GenericModelWidget::inputFromJSON(QJsonObject &rvObject)
{
    bool result = true;

    return result;
}

