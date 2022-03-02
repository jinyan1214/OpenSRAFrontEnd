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

#include "RandomVariablesWidget.h"

#include "RandomVariable.h"
#include "ConstantDistribution.h"
#include "NormalDistribution.h"
#include "UniformDistribution.h"
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

RandomVariablesWidget::RandomVariablesWidget(QWidget *parent) : SimCenterWidget(parent) //, correlationDialog(NULL), correlationMatrix(NULL), checkbox(NULL)
{
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setMargin(0);
    verticalLayout->setSpacing(0);
    this->makeRVWidget();
}


void RandomVariablesWidget::addRVsWithValues(QStringList &varNamesAndValues)
{
    if (addRVsType == 0)
        this->addConstantRVs(varNamesAndValues);
    else if (addRVsType == 1)
        this->addUniformRVs(varNamesAndValues);
    else
        this->addNormalRVs(varNamesAndValues);
}


void RandomVariablesWidget::addConstantRVs(QStringList &varNamesAndValues)
{
    int numVar = varNamesAndValues.count();
    for (int i=0; i<numVar; i+= 2) {

        QString varName = varNamesAndValues.at(i);
        QString value = varNamesAndValues.at(i+1);

        double dValue = value.toDouble();
        ConstantDistribution *theDistribution = new ConstantDistribution(dValue, 0);
        RandomVariable *theRV = new RandomVariable("Uncertain", varName, *theDistribution, "OpenSRA");

        this->addRandomVariable(theRV);
    }
}


void RandomVariablesWidget::addNormalRVs(QStringList &varNamesAndValues)
{
    int numVar = varNamesAndValues.count();
    for (int i=0; i<numVar; i+= 2) {

        QString varName = varNamesAndValues.at(i);
        QString value = varNamesAndValues.at(i+1);

        double dValue = value.toDouble();
        NormalDistribution *theDistribution = new NormalDistribution(dValue, 0);
        RandomVariable *theRV = new RandomVariable("Uncertain", varName, *theDistribution, "OpenSRA");

        this->addRandomVariable(theRV);
    }
}


void RandomVariablesWidget::addUniformRVs(QStringList &varNamesAndValues)
{

    int numVar = varNamesAndValues.count();
    for (int i=0; i<numVar; i+= 2)
    {
        QString varName = varNamesAndValues.at(i);
        QString value = varNamesAndValues.at(i+1);

        double dValue = value.toDouble();
        ConstantDistribution *theDistribution = new ConstantDistribution(dValue);
        RandomVariable *theRV = new RandomVariable("Uncertain", varName, *theDistribution, "OpenSRA");
        theRV->fixToUniform(dValue);

        this->addRandomVariable(theRV);
    }
}


void RandomVariablesWidget::copyRVs(RandomVariablesWidget *oldRVcontainers)
{

    QVector<RandomVariable *> tmp_dists = oldRVcontainers->getRVdists();
    for(int i = 0; i < tmp_dists.size(); ++i)
    {
        tmp_dists.at(i)->uqEngineChanged("OpenSRA");
        this->addRandomVariable(tmp_dists.at(i));
    }
}


QVector<RandomVariable *> RandomVariablesWidget::getRVdists()
{
    return theRandomVariables;
}


void RandomVariablesWidget::addRandomVariable(QString &varName) {

    NormalDistribution *theDistribution = new NormalDistribution();
    RandomVariable *theRV = new RandomVariable("Uncertain", varName, *theDistribution, "OpenSRA");

    this->addRandomVariable(theRV);
}


void RandomVariablesWidget::removeRandomVariable(QString &varName)
{
    //
    // find the RV, if refCout > 1 decrement refCount otherwise remove and delete the RV
    //

    int numRandomVariables = theRandomVariables.size();
    for (int j =0; j < numRandomVariables; j++) {
        RandomVariable *theRV = theRandomVariables.at(j);
        if (theRV->variableName->text() == varName) {
            if (theRV->refCount > 1) {
                theRV->refCount = theRV->refCount-1;
            } else {
                theRV->close();
                //rvLayout->removeWidget(theRV);
                theRandomVariables.remove(j);
                theRV->setParent(0);
                delete theRV;

                // remove name from List
                randomVariableNames.removeAt(j);
            }
            j=numRandomVariables; // get out of loop if foud
        }
    }
}


void RandomVariablesWidget::removeRandomVariables(QStringList &varNames)
{
    //
    // just loop over list, get varName & invoke removeRandomVariable with varName
    //

    int numVar = varNames.count();
    for (int i=0; i<numVar; i++) {
        QString varName = varNames.at(i);
        this->removeRandomVariable(varName);
    }
}


RandomVariablesWidget::~RandomVariablesWidget()
{

}


void RandomVariablesWidget::makeRVWidget(void)
{
    // title & add button
    QHBoxLayout *titleLayout = new QHBoxLayout();
    //titleLayout->setMargin(10);

    SectionTitle *title=new SectionTitle();
    title->setText(tr("Random Variables"));
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

    //    // padhye, adding the button for correlation matrix, we need to add a condition here
    //    // that whether the uqMehod selected is that of Dakota and sampling type? only then we need correlation matrix

    //    /* FMK */
    //    addCorrelation = new QPushButton(tr("Correlation Matrix"));
    //    connect(addCorrelation,SIGNAL(clicked()),this,SLOT(addCorrelationMatrix()));

    //    flag_for_correlationMatrix=1;

    //    /********************* moving to sampling method input ***************************
    //    QCheckBox *checkbox =new QCheckBox("Sobolev Index", this);
    //    connect(checkbox,SIGNAL(clicked(bool)),this,SLOT(addSobolevIndices(bool)));
    //    flag_for_sobolev_indices=0;
    //    ******************************************************************************** */

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
    QStringList headers = {"Name","Source", "Mean", "Sigma","CoV", "Distribution Min", "Distribution Max","Distribution Type"};
    tableModel->setHeaderStringList(headers);

    // Distribution type
    distTypeComboDelegate = new ComboBoxDelegate(this);
    QStringList distTypes = {"Lognormal","Normal","N/A"};
    distTypeComboDelegate->setItems(distTypes);
    theRVTableView->setItemDelegateForColumn(7, distTypeComboDelegate);

    // Source type
    sourceComboDelegate = new ComboBoxDelegate(this);
    QStringList sourceTypes = {"Preferred","User-defined","Distribution"};
    sourceComboDelegate->setItems(sourceTypes);
    theRVTableView->setItemDelegateForColumn(1, sourceComboDelegate);

    theRVTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);

    //    QScrollArea *sa = new QScrollArea;
    //    sa->setWidgetResizable(true);
    //    sa->setLineWidth(0);
    //    sa->setFrameShape(QFrame::NoFrame);

    //    //rv = new QGroupBox(tr(""));
    //    rv = new QWidget;

    //    rvLayout = new QVBoxLayout;
    //    rvLayout->addStretch();
    //    rv->setLayout(rvLayout);

    //   // this->addRandomVariable();
    //     sa->setWidget(rv);
    //     verticalLayout->addWidget(sa);

    verticalLayout->addStretch(1);
}


void RandomVariablesWidget::addRandomVariable(void) {

    RandomVariable *theRV = new RandomVariable("Uncertain", "OpenSRA");
    theRandomVariables.append(theRV);

    // rvLayout->insertWidget(rvLayout->count()-1, theRV);

}


void RandomVariablesWidget::removeRandomVariable(void)
{
    // find the ones selected & remove them
    int numRandomVariables = theRandomVariables.size();
    int *index_selected_to_remove;int size_selected_to_remove=0;

    index_selected_to_remove = (int *)malloc(numRandomVariables*sizeof(int));

    for (int i = numRandomVariables-1; i >= 0; i--) {
        qDebug()<<"\n the value of i is     "<<i;
        RandomVariable *theRV = theRandomVariables.at(i);
        if (theRV->isSelectedForRemoval()) {
            theRV->close();

            //rvLayout->removeWidget(theRV);

            theRandomVariables.remove(i);
            randomVariableNames.removeAt(i);

            theRV->setParent(0);
            delete theRV;
            index_selected_to_remove[size_selected_to_remove]=i;

            size_selected_to_remove=size_selected_to_remove+1;

        }
    }

    free(index_selected_to_remove);
}


void RandomVariablesWidget::loadRVsFromJson(void)
{
    QString RVsFileDir=QFileDialog::getOpenFileName(this,tr("Open File"),"", "JSON File (*.json)");

    QFile file(RVsFileDir);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QString message = QString("Error: could not open file") + RVsFileDir;
    }
    QString val;
    val=file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject rvObject = doc.object();
    if (!RVsFileDir.isEmpty())
        inputFromJSON(rvObject);
}


void RandomVariablesWidget::saveRVsToJson(void)
{
    QString RVsFileDir = QFileDialog::getSaveFileName(this,
                                                      tr("Save Data"), "RVs",
                                                      tr("JSON File (*.json)"));
    QFile file(RVsFileDir);
    if (file.open(QIODevice::WriteOnly))
    {
        QJsonObject rvObject;
        outputToJSON(rvObject);

        QJsonDocument doc(rvObject);
        file.write(doc.toJson());
        file.close();
    }

}


void RandomVariablesWidget::addRandomVariable(RandomVariable *theRV) {

    if (randomVariableNames.contains(theRV->variableName->text())) {

        //
        // if exists, get index and increment refCount of current RV, deletig new
        //
        auto aa =theRV->variableName->text();
        int index = randomVariableNames.indexOf(theRV->variableName->text());
        RandomVariable *theCurrentRV = theRandomVariables.at(index);
        theCurrentRV->refCount = theCurrentRV->refCount+1;
        delete theRV;

    } else {

        //
        // if does not exist we add it
        //    set refCount to 1, don;t allow others to edit it, set connections & finally and add at end
        //

        theRandomVariables.append(theRV);

        //rvLayout->insertWidget(rvLayout->count()-1, theRV);

        theRV->refCount = 1;
        theRV->variableName->setReadOnly(true);

        // connect(this,SLOT(randomVariableErrorMessage(QString)), theRV, SIGNAL(sendErrorMessage(QString)));
        connect(theRV->variableName, SIGNAL(textEdited(const QString &)), this, SLOT(variableNameChanged(const QString &)));

        randomVariableNames << theRV->variableName->text();
    }
}


void RandomVariablesWidget::clear(void) {

    // loop over random variables, removing from layout & deleting
    for (int i = 0; i <theRandomVariables.size(); ++i) {
        RandomVariable *theRV = theRandomVariables.at(i);
        //rvLayout->removeWidget(theRV);
        delete theRV;
    }

    theRandomVariables.clear();
    randomVariableNames.clear();
    theRVTableView->clear();

}


bool RandomVariablesWidget::outputToJSON(QJsonObject &rvObject) {

    bool result = true;
    QJsonArray rvArray;
    for (int i = 0; i <theRandomVariables.size(); ++i) {
        QJsonObject rv;
        if (theRandomVariables.at(i)->outputToJSON(rv)) {
            rvArray.append(rv);
        } else {
            qDebug() << "OUTPUT FAILED" << theRandomVariables.at(i)->variableName->text();
            result = false;
        }
    }

    rvObject["randomVariables"]=rvArray;

    return result;
}


QStringList RandomVariablesWidget::getRandomVariableNames(void)
{
    QStringList results;
    for (int i = 0; i <theRandomVariables.size(); ++i) {
        results.append(theRandomVariables.at(i)->getVariableName());
    }
    return results;
}


int RandomVariablesWidget::getNumRandomVariables(void)
{
    return theRandomVariables.size();
}


void RandomVariablesWidget::copyFiles(QString fileDir)
{
    for (int i = 0; i <theRandomVariables.size(); ++i) {
        theRandomVariables.at(i)->copyFiles(fileDir);
    }
}


bool RandomVariablesWidget::inputFromJSON(QJsonObject &rvObject)
{
    bool result = true;

    //
    // go get randomvariables array from the JSON object
    // for each object in array:
    //    1)get it'is type,
    //    2)instantiate one
    //    4) get it to input itself
    //    5) finally add it to layout
    //


    // get randomVariables & add
    //  int numRandomVariables = 0;
    //  if (rvObject.contains("randomVariables")) {
    //      if (rvObject["randomVariables"].isArray()) {

    //          QJsonArray rvArray = rvObject["randomVariables"].toArray();

    //          // foreach object in array
    //          foreach (const QJsonValue &rvValue, rvArray) {

    //              QJsonObject rvObject = rvValue.toObject();

    //              if (rvObject.contains("variableClass")) {
    //                  QJsonValue typeRV = rvObject["variableClass"];
    //                  RandomVariable *theRV = 0;
    //                  QString classType = typeRV.toString();
    //                  theRV = new RandomVariable(classType,"OpenSRA");
    //                  connect(theRV->variableName, SIGNAL(textEdited(const QString &)), this, SLOT(variableNameChanged(const QString &)));

    //                  //connect(theRV,SIGNAL(sendErrorMessage(QString)),this,SLOT(errorMessage(QString)));

    //                  if (theRV->inputFromJSON(rvObject)) { // this method is where type is set
    //                      theRandomVariables.append(theRV);
    //                      randomVariableNames << theRV->variableName->text();
    //                      theRV->variableName->setReadOnly(true);

    //                      //rvLayout->insertWidget(rvLayout->count()-1, theRV);
    //                      numRandomVariables++;
    //                  } else {
    //                      result = false;
    //                  }
    //              } else {
    //                  result = false;
    //              }
    //          }
    //      }
    //  }

    theRVTableView->show();

    QJsonObject paramsObject = rvObject["EngineeringDemandParameter"].toObject()["Landslide"].toObject()["Params"].toObject()["YieldAcceleration"].toObject()["Params"].toObject();

    auto objKeys = paramsObject.keys();

    RVTableModel* tableModel = theRVTableView->getTableModel();

    QVector<QVector<QVariant>> data;

    for(auto&& it : objKeys)
    {
        if(it == "MethodForKy")
            continue;

        // "Name","Source", "Mean", "Sigma","cov", "dist. min", "dist. max","dist. type"
        QVector<QVariant> row(8);

        row[0] = it;
        row[1] = QVariant("Preferred");
        row[2] = QVariant(0.0);
        row[3] = QVariant(0.0);
        row[4] = QVariant(0.0);
        row[5] = QVariant(0.0);
        row[6] = QVariant(0.0);
        row[7] = QVariant("Lognormal");

        // auto itObj = paramsObject.value(it).toObject();
        data.push_back(row);
    }

    tableModel->populateData(data);

    return result;
}

