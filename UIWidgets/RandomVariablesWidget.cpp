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
#include "WorkflowAppOpenSRA.h"
#include "PipelineNetworkWidget.h"
#include "LineAssetInputWidget.h"

#include "RVTableView.h"
#include "RVTableModel.h"
#include "MixedDelegate.h"
#include "LineEditDelegate.h"
#include "LabelDelegate.h"
#include "ComboBoxDelegate.h"
#include "ButtonDelegate.h"
#include "StringListDelegate.h"

#include <QComboBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <sectiontitle.h>
#include <QLineEdit>
#include <QFileDialog>

RandomVariablesWidget::RandomVariablesWidget(QWidget *parent) : SimCenterWidget(parent)
{
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setMargin(2);
    verticalLayout->setSpacing(2);

    RVTableHeaders = QStringList({"Name","Description","From Model","Source","Distribution Type","Mean or Median","Sigma","CoV","Distribution Min","Distribution Max"});
    constantTableHeaders = QStringList({"Name","Description","From Model","Source","Value"});

    this->makeRVWidget();
}


RandomVariablesWidget::~RandomVariablesWidget()
{

}


bool RandomVariablesWidget::removeRandomVariable(const QString &uuid, const QString &fromModel)
{
    auto res = theRVTableView->getTableModel()->removeRandomVariable(uuid,fromModel);

    if(res == false)
        this->errorMessage("Failed to remove random variable "+uuid);

    return res;
}


bool RandomVariablesWidget::removeConstant(const QString &uuid, const QString &fromModel)
{
    auto res = theConstantTableView->getTableModel()->removeRandomVariable(uuid,fromModel);

    if(res == false)
        this->errorMessage("Failed to remove the constant "+uuid);

    return res;
}


void RandomVariablesWidget::makeRVWidget(void)
{
    // title & add button
    QHBoxLayout *titleLayout = new QHBoxLayout();
    //titleLayout->setMargin(10);

    SectionTitle *title=new SectionTitle();
    title->setText(tr("Input Variables"));
    title->setMinimumWidth(250);

    titleLayout->addWidget(title);

    verticalLayout->addLayout(titleLayout);

    QLabel* rvLabel = new QLabel("Random Variables");
    rvLabel->setStyleSheet("font-weight: bold; color: black");
    verticalLayout->addWidget(rvLabel,0,Qt::AlignHCenter);

    theRVTableView = new RVTableView();

    theRVTableView->setWordWrap(true);
    theRVTableView->setTextElideMode(Qt::ElideNone);

    RVTableModel* tableModel = theRVTableView->getTableModel();
    tableModel->setHeaderStringList(RVTableHeaders);

    verticalLayout->addWidget(theRVTableView);

    // Source type
    sourceComboDelegate = new ComboBoxDelegate(this);
    QStringList sourceTypes = {"Preferred","From Infrastructure Table or Enter Value","From user-provided GIS maps"};
    sourceComboDelegate->setItems(sourceTypes);

    connect(sourceComboDelegate,&ComboBoxDelegate::currentIndexChanged,this,&RandomVariablesWidget::handleSourceChanged);

    // Column selection/data type delegate
    colDataComboDelegate = new MixedDelegate(this);
    QStringList colTypes = {"No infrastructure information available"};
    colDataComboDelegate->setItems(colTypes);
    colDataComboDelegate->setIsEditable(true);

    gisMapsComboDelegate = new MixedDelegate(this);
    QStringList gisMaps = {"No GIS maps loaded"};
    gisMapsComboDelegate->setItems(gisMaps);
    gisMapsComboDelegate->setIsEditable(false);

    // Distribution type
    distTypeComboDelegate = new MixedDelegate(this);
    QStringList distTypes = {"","Lognormal","Normal"};
    distTypeComboDelegate->setItems(distTypes);
    distTypeComboDelegate->setIsEditable(false);

    connect(theRVTableView->getTableModel(),SIGNAL(handleCellChanged(int,int)),this,SLOT(handleCellChanged(int,int)));

    LabelDelegate* labDelegate = new LabelDelegate(this);

    LEDelegate = new LineEditDelegate(this);

    //    StringListDelegate* SLDelegate = new StringListDelegate(this);

    // "Name","Description","From Model","Source","Distribution Type","Mean or Median","Sigma","CoV","Distribution Min","Distribution Max"

    // Name
    theRVTableView->setItemDelegateForColumn(0, labDelegate);

    // Description
    theRVTableView->setItemDelegateForColumn(1, labDelegate);

    // From Model
    theRVTableView->setItemDelegateForColumn(2, labDelegate);

    // Source
    theRVTableView->setItemDelegateForColumn(3, sourceComboDelegate);

    // Distribution Type
    theRVTableView->setItemDelegateForColumn(4, distTypeComboDelegate);

    // Mean or Median
    theRVTableView->setItemDelegateForColumn(5, colDataComboDelegate);

    // Sigma
    theRVTableView->setItemDelegateForColumn(6, colDataComboDelegate);

    // Cov
    theRVTableView->setItemDelegateForColumn(7, colDataComboDelegate);

    // Min/max distribution
    theRVTableView->setItemDelegateForColumn(8, LEDelegate);
    theRVTableView->setItemDelegateForColumn(9, LEDelegate);

    theRVTableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    theRVTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);


    // The constant table
    QLabel* constLabel = new QLabel("Constant or Fixed Variables");
    constLabel->setStyleSheet("font-weight: bold; color: black");
    verticalLayout->addWidget(constLabel,0,Qt::AlignHCenter);

    theConstantTableView = new RVTableView();

    theConstantTableView->setWordWrap(true);
    theConstantTableView->setTextElideMode(Qt::ElideNone);

    RVTableModel* constTableModel = theConstantTableView->getTableModel();
    constTableModel->setHeaderStringList(constantTableHeaders);

    // "Name","Description","From Model","Source","Value"
    // Name
    theConstantTableView->setItemDelegateForColumn(0, labDelegate);

    // Description
    theConstantTableView->setItemDelegateForColumn(1, labDelegate);

    // From Model
    theConstantTableView->setItemDelegateForColumn(2, labDelegate);

    // Source
    theConstantTableView->setItemDelegateForColumn(3, sourceComboDelegate);

    // "Value"
    theConstantTableView->setItemDelegateForColumn(4, colDataComboDelegate);


    verticalLayout->addWidget(theConstantTableView);

    QLabel* instructionsLabel = new QLabel("Instructions for using this tab");

    verticalLayout->addWidget(instructionsLabel);

    verticalLayout->addStretch(1);
}


MixedDelegate *RandomVariablesWidget::getColDataComboDelegate() const
{
    return colDataComboDelegate;
}


MixedDelegate *RandomVariablesWidget::getGisMapsComboDelegate() const
{
    return gisMapsComboDelegate;
}


bool RandomVariablesWidget::addNewModelToExistingParameter(const RV& rv, const QStringList& fromModel, RVTableView* database)
{

    auto name = rv.getName();

    auto model = database->getTableModel();

    auto& RVs = model->getRandomVariables();

    for(auto& it: RVs)
    {
        if(it.getName().compare(name) == 0)
        {
            it.addModelToList(fromModel);
            database->updateTable();
            return true;
        }
    }

    return false;
}



bool RandomVariablesWidget::addRandomVariable(const RV& newRV)
{

    auto rvName = newRV.getName();

    auto fromModel = newRV.getFromModelList();

    if(!this->checkIfRVExists(rvName).isEmpty())
    {
        return this->addNewModelToExistingParameter(newRV,fromModel,theRVTableView);
    }

    auto RVtableModel = theRVTableView->getTableModel();

    if(RVtableModel == nullptr)
        return false;

    RVtableModel->addRandomVariable(newRV);

    // Set the default source value to "Preferred"
    auto rowRV = RVtableModel->rowCount() - 1;

    // Source column
    auto RvIndex = RVtableModel->index(rowRV,3);
    RVtableModel->setData(RvIndex,QVariant("Preferred"));

    // Dist type column
    auto distTypeIndex = RVtableModel->index(rowRV,4);
    RVtableModel->setData(distTypeIndex,QVariant("Normal"));

    return true;
}


bool RandomVariablesWidget::addConstant(const RV& newConstant)
{

    auto paramName = newConstant.getName();
    auto fromModel = newConstant.getFromModelList();

    if(!this->checkIfConstantExists(paramName).isEmpty())
    {
        return this->addNewModelToExistingParameter(newConstant,fromModel,theConstantTableView);
    }

    auto tableModel = theConstantTableView->getTableModel();

    if(tableModel == nullptr)
        return false;

    tableModel->addRandomVariable(newConstant);

    auto constanTableModel = theConstantTableView->getTableModel();

    // Set the default source value to "Preferred"
    auto rowConst = constanTableModel->rowCount() - 1;

    // Source column
    auto constIndex = constanTableModel->index(rowConst,3);
    constanTableModel->setData(constIndex,QVariant("Preferred"));

    return true;
}


bool RandomVariablesWidget::addRandomVariable(const QString& name, const QString& fromModel, const QString& desc, const QString& uuid)
{
    RV newRV = this->createNewRV(name,fromModel,desc,uuid);

    return this->addRandomVariable(newRV);
}


bool RandomVariablesWidget::addConstant(const QString& name, const QString& fromModel, const QString& desc, const QString& uuid)
{
    RV newRV = this->createNewConstant(name,fromModel,desc,uuid);

    return this->addConstant(newRV);
}


void RandomVariablesWidget::clear(void)
{
    theConstantTableView->clear();
    theRVTableView->clear();

    theRVTableView->getTableModel()->setHeaderStringList(RVTableHeaders);
    theConstantTableView->getTableModel()->setHeaderStringList(constantTableHeaders);
}


bool RandomVariablesWidget::outputToJSON(QJsonObject &rvObject) {

    Q_UNUSED(rvObject);

    //    bool result = true;
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

    return true;
}


QStringList RandomVariablesWidget::getRandomVariableNames(void)
{

    auto RVs = theRVTableView->getTableModel()->getRandomVariables();

    QStringList results;

    for (int i = 0; i <RVs.size(); ++i) {
        results.append(RVs.at(i).getName());
    }

    return results;
}


void RandomVariablesWidget::handleCellChanged(int row, int col)
{
    RVTableModel* tableModel = theRVTableView->getTableModel();

    if(col == 6)
    {
        auto index = theRVTableView->getTableModel()->index(row,7);
        tableModel->blockSignals(true);
        tableModel->setData(index,QVariant(),Qt::EditRole);
        tableModel->blockSignals(false);
    }
    else if(col == 7)
    {
        auto index = theRVTableView->getTableModel()->index(row,6);
        tableModel->blockSignals(true);
        tableModel->setData(index,QVariant(),Qt::EditRole);
        tableModel->blockSignals(false);
    }
}


bool RandomVariablesWidget::inputFromJSON(QJsonObject &rvObject)
{

    Q_UNUSED(rvObject);

    bool result = true;

    //    theRVTableView->show();

    //    QJsonObject paramsObject = rvObject["EngineeringDemandParameter"].toObject()["Landslide"].toObject()["Params"].toObject()["YieldAcceleration"].toObject()["Params"].toObject();

    //    auto objKeys = paramsObject.keys();

    //    RVTableModel* tableModel = theRVTableView->getTableModel();

    //    QVector<RV> data;

    //    for(auto&& it : objKeys)
    //    {
    //        if(it == "MethodForKy")
    //            continue;

    //        RV row = this->createNewRV(it,"EngineeringDemandParameter-Landslide-YieldAcceleration");
    //        //        row[10] = QVariant();

    //        // auto itObj = paramsObject.value(it).toObject();
    //        data.push_back(row);
    //    }

    //    tableModel->populateData(data);

    return result;
}


QString RandomVariablesWidget::checkIfParameterExists(const QString& name, bool& OK)
{

    auto RVUUid = checkIfRVExists(name);
    auto constantUUid = checkIfConstantExists(name);

    if(!RVUUid.isEmpty() && !constantUUid.isEmpty())
    {
        this->errorMessage("Warning, a RV and a constant have the same name, this is not allowed.");
        OK = false;
        return QString();
    }

    if(!RVUUid.isEmpty())
        return RVUUid;
    else
        return constantUUid;

    return QString();
}


QString RandomVariablesWidget::checkIfRVExists(const QString& name)
{
    auto model = theRVTableView->getTableModel();

    auto RVs = model->getRandomVariables();

    for(auto&& it: RVs)
    {
        if(it.getName().compare(name) == 0)
            return it.getUuid();
    }

    return QString();
}


QString RandomVariablesWidget::checkIfConstantExists(const QString& name)
{
    auto model = theConstantTableView->getTableModel();

    auto params = model->getRandomVariables();

    for(auto&& it: params)
    {
        if(it.getName().compare(name) == 0)
            return it.getUuid();
    }

    return QString();
}


bool  RandomVariablesWidget::checkIfConstantuuidExists(const QString& uuid)
{
    auto model = theConstantTableView->getTableModel();

    auto RVs = model->getRandomVariables();

    for(auto&& it: RVs)
    {
        if(it.getUuid().compare(uuid) == 0)
            return true;
    }

    return false;
}


bool RandomVariablesWidget::checkIfRVuuidExists(const QString& uuid)
{
    auto model = theRVTableView->getTableModel();

    auto RVs = model->getRandomVariables();

    for(auto&& it: RVs)
    {
        if(it.getUuid().compare(uuid) == 0)
            return true;
    }

    return false;
}


RV RandomVariablesWidget::createNewRV(const QString& name, const QString& fromModel, const QString& desc, const QString& uuid)
{
    auto numCols = RVTableHeaders.size();

    RV newRV(numCols,uuid,fromModel,desc);

    // "Name","Description","From Model","Source","Distribution Type","Mean or Median","Sigma","CoV","Distribution Min","Distribution Max"
    newRV[0] = name;
    newRV[1] = QVariant(desc);
    newRV[2] = QVariant(QStringList(fromModel));
    newRV[3] = QVariant();
    newRV[4] = QVariant();
    newRV[5] = QVariant();
    newRV[6] = QVariant();
    newRV[7] = QVariant();
    newRV[8] = QVariant();
    newRV[9] = QVariant();

    newRV.setName(name);

    return newRV;
}


RV RandomVariablesWidget::createNewConstant(const QString& name, const QString& fromModel, const QString& desc, const QString& uuid)
{
    auto numCols = constantTableHeaders.size();

    RV newRV(numCols,uuid,fromModel,desc);

    // "Name","Description","From Model","Source","Value"
    newRV[0] = name;
    newRV[1] = QVariant(desc);
    newRV[2] = QVariant(QStringList(fromModel));
    newRV[3] = QVariant();
    newRV[4] = QVariant();

    newRV.setName(name);

    return newRV;
}


bool RandomVariablesWidget::addNewParameter(const QString& name, const QString& fromModel, const QString& desc, const QString& uuid, const QString& type)
{
    if(type.compare("random") == 0)
        return addRandomVariable(name,fromModel,desc,uuid);
    else if(type.compare("fixed") == 0)
        return addConstant(name,fromModel,desc,uuid);
    else
        this->errorMessage("The type of parameter: "+type+ " is not recognized, for parameter: "+name+", from model: "+fromModel);

    return false;
}


bool RandomVariablesWidget::removeParameter(const QString& uuid, const QString& fromModel)
{
    // First try if it is a RV
    if(this->checkIfRVuuidExists(uuid))
        return removeRandomVariable(uuid,fromModel);
    else if(this->checkIfConstantuuidExists(uuid)) // Then check to see if it is an object
        return removeConstant(uuid,fromModel);
    else
        return false;
}


void RandomVariablesWidget::handleSourceChanged(int val)
{
    if(val == 0 || val == 1)
    {
        // "Name","Description","From Model","Source","Distribution Type","Mean or Median","Sigma","CoV","Distribution Min","Distribution Max"

        // "Mean or Median"
        theRVTableView->setItemDelegateForColumn(5, colDataComboDelegate);

        // "Sigma"
        theRVTableView->setItemDelegateForColumn(6, colDataComboDelegate);

        // "CoV"
        theRVTableView->setItemDelegateForColumn(7, colDataComboDelegate);

        // "Name","Description","From Model","Source","Value"
        theConstantTableView->setItemDelegateForColumn(4, colDataComboDelegate);
    }
    else if(val == 2)
    {
        theRVTableView->setItemDelegateForColumn(5, gisMapsComboDelegate);
        theRVTableView->setItemDelegateForColumn(6, LEDelegate);
        theRVTableView->setItemDelegateForColumn(7, LEDelegate);

        theConstantTableView->setItemDelegateForColumn(4, gisMapsComboDelegate);
    }
    else
    {
        this->errorMessage("Combobox input not handled in RandomVariablesWidget::handleSourceChanged. Please let the developers know");
    }
}
