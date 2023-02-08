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
#include "CSVReaderWriter.h"
#include "OpenSRAPreferences.h"

#include "ComboBoxDelegate.h"
#include "sectiontitle.h"

#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QLineEdit>
#include <QFileDialog>

GenericModelWidget::GenericModelWidget(QString parName, QJsonObject &methodObj, QWidget *parent) : SimCenterAppWidget(parent), parentName(parName)
{
    this->setObjectName("Generic Model Widget of "+parName);
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setMargin(2);
    verticalLayout->setSpacing(2);
    this->makeRVWidget(methodObj);

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}


GenericModelWidget::~GenericModelWidget()
{

}


void GenericModelWidget::makeRVWidget(QJsonObject &methodObj)
{
    // instructions section
    QGroupBox* instructionsGB = new QGroupBox("Instructions");
    QHBoxLayout *instructionsLayout = new QHBoxLayout(instructionsGB);
    auto instructionsLabel = new QLabel(
        "- Create equations by adding one term at a time using the following table."
        "\n\t- The \"Add\" button adds the term to the equation. The term you added should show up under the \"Generic Equation\" section below."
        "\n\t- The \"Remove\" button removes the term in the bottom row of the table."
        "\n\t- To generate constants, set \"Power\" to \"0\" (the value under \"Variable Label\" will not be used when \"Power\" is set to \"0\")."
        "\n- The three levels correspond to the levels of analysis users can create models for. OpenSRA decides the level to use based on data availability."
        "\n\t- Ideally, the lower levels should contain the variables used by the upper levels (e.g., the level 2 model should contain the variables used by level 1)."
    );
    instructionsLayout->addWidget(instructionsLabel);
    verticalLayout->addWidget(instructionsGB);

    // model distribution type
    QHBoxLayout *eqnTypeLayout = new QHBoxLayout();
    QLabel* eqTypeLabel = new QLabel("Select model distribution type:");

    eqTypeCombo = new QComboBox();
    eqTypeCombo->addItems(QStringList({"lognormal","normal"}));
    eqTypeCombo->setMinimumWidth(100);
    eqTypeCombo->setMinimumHeight(25);

    connect(eqTypeCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(handleTypeChanged(int)));

    eqnTypeLayout->addWidget(eqTypeLabel);
    eqnTypeLayout->addWidget(eqTypeCombo);
    eqnTypeLayout->addStretch();
    verticalLayout->addLayout(eqnTypeLayout);

    // return param
    QHBoxLayout *returnParamLayout = new QHBoxLayout();
    QLabel* returnParamLabel = new QLabel("Enter label for return parameter (limited to one for now):");

    returnParamLineEdit = new QLineEdit();
    returnParamLineEdit->setText("pgdef");
    returnParamLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    returnParamLayout->addWidget(returnParamLabel);
    returnParamLayout->addWidget(returnParamLineEdit);
    returnParamLayout->setStretch(1,0);
    verticalLayout->addLayout(returnParamLayout);

    // upstream category
    QHBoxLayout *upstreamCatLayout = new QHBoxLayout();
    QLabel* upstreamCatLabel = new QLabel("Select upstream dependency:");

//    auto genModMethodParamObj = this->getMethodAndParamJsonObj();
    auto genModParams = methodObj["Params"].toObject();
    auto genModCatObj = genModParams.value("ReturnCategory").toObject();
    QString genModCat = genModCatObj.value("DefaultValue").toString();
    if (genModCat == "EngineeringDemandParameter")
        upstreamCatList.append("IM");
    else if (genModCat == "DamageMeasure")
    {
        upstreamCatList.append("IM");
        upstreamCatList.append("EDP");
    }
    else if (genModCat == "DecisionVariable")
    {
        upstreamCatList.append("IM");
        upstreamCatList.append("EDP");
        upstreamCatList.append("DM");
    }

    upstreamCatCombo = new QComboBox();
    upstreamCatCombo->addItems(upstreamCatList);
    upstreamCatCombo->setMinimumWidth(100);
    upstreamCatCombo->setMinimumHeight(25);

    upstreamCatLayout->addWidget(upstreamCatLabel);
    upstreamCatLayout->addWidget(upstreamCatCombo);
    upstreamCatLayout->addStretch();
    verticalLayout->addLayout(upstreamCatLayout);

    // upstream params
    QHBoxLayout *upstreamParamLayout = new QHBoxLayout();
    QLabel* upstreamParamLabel = new QLabel("Enter list of upstream parameters (leave empty if none):");

    upstreamParamLineEdit = new QLineEdit();
    upstreamParamLineEdit->setText("pga, mag");
    upstreamParamLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    upstreamParamLayout->addWidget(upstreamParamLabel);
    upstreamParamLayout->addWidget(upstreamParamLineEdit);
    upstreamParamLayout->setStretch(1,0);
    verticalLayout->addLayout(upstreamParamLayout);

    // model definition table
    // title & add button
    QHBoxLayout *titleLayout = new QHBoxLayout();
    //titleLayout->setMargin(10);

    SectionTitle *title=new SectionTitle();
    title->setText(tr("Generic Model Definition"));
    title->setMinimumWidth(250);

    QPushButton *addParam = new QPushButton();
    addParam->setMinimumWidth(75);
    addParam->setMaximumWidth(75);
    addParam->setText(tr("Add"));
    connect(addParam,SIGNAL(clicked()),this,SLOT(addParam()));

    QPushButton *removeParam= new QPushButton();
    removeParam->setMinimumWidth(75);
    removeParam->setMaximumWidth(75);
    removeParam->setText(tr("Remove"));
    connect(removeParam,SIGNAL(clicked()),this,SLOT(removeParam()));

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(addParam);
    buttonLayout->addWidget(removeParam);
    buttonLayout->addStretch();

    titleLayout->addWidget(title);

    verticalLayout->addLayout(titleLayout);
    theRVTableView = new RVTableView();
    theRVTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    verticalLayout->addWidget(theRVTableView);
    verticalLayout->addLayout(buttonLayout);

    QGroupBox* eqnGB = new QGroupBox("Generic Equation");

    QVBoxLayout *eqnLayout = new QVBoxLayout(eqnGB);

    eqnLabelLevel1 = new QLabel("");
    eqnLabelLevel2 = new QLabel("");
    eqnLabelLevel3 = new QLabel("");

    QLabel* level1Label = new QLabel("Level 1:");
    QLabel* level2Label = new QLabel("Level 2:");
    QLabel* level3Label = new QLabel("Level 3:");

    level1Label->setStyleSheet("font-weight: bold; color: black");
    level2Label->setStyleSheet("font-weight: bold; color: black");
    level3Label->setStyleSheet("font-weight: bold; color: black");

    eqnLayout->addWidget(level1Label);
    eqnLayout->addWidget(eqnLabelLevel1);

    eqnLayout->addWidget(level2Label);
    eqnLayout->addWidget(eqnLabelLevel2);

    eqnLayout->addWidget(level3Label);
    eqnLayout->addWidget(eqnLabelLevel3);

    eqnLayout->addStretch();

    verticalLayout->addWidget(eqnGB);

    RVTableModel* tableModel = theRVTableView->getTableModel();
    headers.append("Variable");
    headers.append("Level");
    headers.append("Coefficient");
    headers.append("Apply Ln");
    headers.append("Power");
//    {"Variable Label","Level","Coefficient", "Apply Ln", "Power"};
    tableModel->setHeaderStringList(headers);
    theRVTableView->show();

    // Level
    levelComboDelegate = new ComboBoxDelegate(this);
    QStringList levelTypes = {"1","2","3"};
    levelComboDelegate->setItems(levelTypes);
    theRVTableView->setItemDelegateForColumn(1, levelComboDelegate);

    connect(theRVTableView->getTableModel(),SIGNAL(handleCellChanged(int,int)),this,SLOT(handleCellChanged(int,int)));

    // Ln type
    applyLnComboDelegate = new ComboBoxDelegate(this);
    QStringList distTypes = {"true","false"};
    applyLnComboDelegate->setItems(distTypes);
    theRVTableView->setItemDelegateForColumn(3, applyLnComboDelegate);

    // Power
    powerComboDelegate = new ComboBoxDelegate(this);
    QStringList powerTypes = {"0","1","2"};
    powerComboDelegate->setItems(powerTypes);
    theRVTableView->setItemDelegateForColumn(4, powerComboDelegate);

    //    theRVTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);

    // Add five random parameters to start with
    for(int i = 0; i<5; ++i)
        this->addParam();

    this->generateEquation();

//    verticalLayout->addStretch(1);
}


void GenericModelWidget::sortData(void)
{

    if(data.size()<2)
        return;

    // Sorting function to sort according to level
    auto sortFxn = [](RV& rowA, RV& rowB) -> bool
    {
        auto a = rowA[1].toInt();
        auto b = rowB[1].toInt();

        return a < b ? true : false;
    };

    std::sort(data.begin(),data.end(),sortFxn);

}

void GenericModelWidget::clear(void) {

//    theRVTableView->clear();
    auto tableModel = theRVTableView->getTableModel();
    tableModel->clear();
    tableModel->setHeaderStringList(headers);
    data.clear();
    eqnLabelLevel1->clear();
    eqnLabelLevel2->clear();
    eqnLabelLevel3->clear();

    eqTypeCombo->setCurrentIndex(0);
    upstreamCatCombo->setCurrentIndex(0);
    upstreamParamLineEdit->setText("pga, mag");
    returnParamLineEdit->setText("pgdef");
}


bool GenericModelWidget::outputToJSON(QJsonObject &jsonObj) {

    // Here we get the random variables and constants
    // At the same time we need to populate the methods json object to provide some more information about the type of RV

    // get method params for generic model
    auto genModMethodParamObj = this->getMethodAndParamJsonObj();
    auto genModParams = genModMethodParamObj["Params"].toObject();
    auto genModCatObj = genModParams.value("ReturnCategory").toObject();
    QString genModCat = genModCatObj.value("DefaultValue").toString();
    auto genModHazObj = genModParams.value("ReturnHazard").toObject();
    QString genModHaz = genModHazObj.value("DefaultValue").toString();

    QString genModCatAbbr;
    if (genModCat == "EngineeringDemandParameter")
        genModCatAbbr = "EDP";
    else if (genModCat == "DamageMeasure")
        genModCatAbbr = "DM";
    else if (genModCat == "DecisionVariable")
        genModCatAbbr = "DV";

    // build path and file name for CSV export
    QString fileDir =
        OpenSRAPreferences::getInstance()->getLocalWorkDir() +
        QDir::separator() + "analysis" +
        QDir::separator() + "Input";
    QFileInfo fileDirInfo(fileDir);
    if (!fileDirInfo.exists())
    {
        fileDir =
            OpenSRAPreferences::getInstance()->getLocalWorkDir() +
            QDir::separator() + "Input";
    }
    fileDirInfo.setFile(fileDir);
    if (!fileDirInfo.exists())
    {
        SimCenterAppWidget::errorMessage("Error: In \"GenericModel.cpp\" - cannot determine path to \"Input\" folder in working dir");
        return false;
    }
    QString fileName = "genmod_" + genModCatAbbr + "_" + genModHaz + ".csv";
    QString filePath = fileDir + QDir::separator() + fileName;
    auto res = this->outputToCsv(filePath);
    if (!res)
        SimCenterAppWidget::errorMessage("Error: In \"GenericModel.cpp\" - cannot export table to " + filePath);

    // make jsonObj for return
    jsonObj["DistType"] = eqTypeCombo->currentText();
    jsonObj["ReturnParams"] = returnParamLineEdit->text();
    jsonObj["UpstreamCategory"] = upstreamCatCombo->currentText();
    jsonObj["UpstreamParams"] = upstreamParamLineEdit->text();
    jsonObj["PathToModelInfo"] = filePath;

    return true;
}


void GenericModelWidget::reset(void)
{
//    this->clear();
}


void GenericModelWidget::addParam()
{
    auto i = data.size();

    // Create a unique id to identify the specific instance of these parameters
    auto uid = QUuid::createUuid().toString();

    // From model
    auto fromModel = parentName;
    auto desc = "User-created parameter from the generic model widget";

    auto name = "var_"+QString::number(i);

    RV newRV(headers.length(),uid,fromModel,desc);

//    int level = 1;

//    if(i>0)
//        level = data.back().at(0).toInt() + 1;

//    // maximum level is 3
//    if(level > 3)
//        level = 3;

    if (i == 0)
    {
        newRV[0] = QString("");
        newRV[1] = QString("1");
        newRV[2] = QString("2");
        newRV[3] = QString("false");
        newRV[4] = QString("0");
    }
    else if (i == 1)
    {
        newRV[0] = QString("pga");
        newRV[1] = QString("1");
        newRV[2] = QString("0.5");
        newRV[3] = QString("true");
        newRV[4] = QString("1");
    }
    else if (i == 2)
    {
        newRV[0] = QString("mag");
        newRV[1] = QString("2");
        newRV[2] = QString("0.5");
        newRV[3] = QString("false");
        newRV[4] = QString("2");
    }
    else
    {
        newRV[0] = QVariant(name);
//        newRV[1] = QVariant(rand() % 2 + 1);
//        newRV[1] = QVariant(qMin(rand() % 2 + 2,3));
        newRV[1] = QVariant(3);
        newRV[2] = QVariant(QString(rand() % 10 < 5 ? "-" : "+") + QString::number(rand() % 5 + 1));
//        newRV[3] = rand() % 10 < 5 ? true : false;
//        newRV[4] = QVariant(rand() % 2);
        newRV[3] = QString("false");
        newRV[4] = QVariant("1");
    }

    data.append(newRV);
    this->sortData();
    RVTableModel* tableModel = theRVTableView->getTableModel();
    tableModel->populateData(data);

    this->generateEquation();

    theRVTableView->resizeEvent(nullptr);
}


void GenericModelWidget::removeParam(void)
{
    QItemSelectionModel *select = theRVTableView->selectionModel();

    //check if has selection
    if(!select->hasSelection())
        return;

    auto selectedRows = select->selectedIndexes(); // return selected items(s)
    //select->selectedColumns() // return selected column(s)

    for(auto&& it: selectedRows)
    {
        if(!it.isValid())
            continue;

        auto rowNum = it.row();

        if(rowNum<=data.size()-1)
            data.remove(rowNum);
    }

    this->sortData();
    RVTableModel* tableModel = theRVTableView->getTableModel();
    tableModel->populateData(data);

    theRVTableView->resizeEvent(nullptr);

    this->generateEquation();
}


bool GenericModelWidget::inputFromJSON(QJsonObject &jsonObj)
{
    // dist type
    if (jsonObj.contains("DistType"))
    {
        if (jsonObj["DistType"]=="lognormal")
            eqTypeCombo->setCurrentIndex(0);
        else if (jsonObj["DistType"]=="normal")
            eqTypeCombo->setCurrentIndex(1);
        else
        {
            SimCenterAppWidget::errorMessage(
                "Error in \"GenericModelWidget::inputFromJSON\" - distribution type \"" +
                jsonObj["DistType"].toString() + "\" is not supported"
            );
            return false;
        }
    }
    else
        eqTypeCombo->setCurrentIndex(0);

    // return param
    if (jsonObj.contains("ReturnParams"))
        returnParamLineEdit->setText(jsonObj["ReturnParams"].toString());
    else
        returnParamLineEdit->setText(jsonObj["ReturnParams"].toString());

    // upstream category
    if (jsonObj.contains("UpstreamCategory"))
    {
        if (upstreamCatList.contains(jsonObj["UpstreamCategory"].toString()))
            upstreamCatCombo->setCurrentText(jsonObj["UpstreamCategory"].toString());
        else
        {
            SimCenterAppWidget::errorMessage(
                "Error in \"GenericModelWidget::inputFromJSON\" - upstream category is not valid"
            );
            return false;
        }
    }

    // upstream params
    if (jsonObj.contains("UpstreamParams"))
        upstreamParamLineEdit->setText(jsonObj["UpstreamParams"].toString());
    else
        upstreamParamLineEdit->setText(jsonObj["UpstreamParams"].toString());

    // path to model definition table
    QString filePath;
    if (jsonObj.contains("UpstreamParams"))
        filePath = jsonObj["PathToModelInfo"].toString();
    else
    {
        SimCenterAppWidget::errorMessage(
            "Error in \"GenericModelWidget::inputFromJSON\" - cannot find attribute in setup_config with path to generic model CSV table"
        );
        return false;
    }
    QFileInfo filePathInfo(filePath);
    if (!filePathInfo.exists())
    {
        // first try using work_dir/Input as reference
        filePath =
            OpenSRAPreferences::getInstance()->getLocalWorkDir() +
            QDir::separator() + "analysis" +
            QDir::separator() + "Input" +
            filePath;
    }
    filePathInfo.setFile(filePath);
    if (!filePathInfo.exists())
    {
        // first try using work_dir/analysis/Input as reference
        filePath =
            OpenSRAPreferences::getInstance()->getLocalWorkDir() +
            QDir::separator() + "Input" +
            filePath;
    }
    filePathInfo.setFile(filePath);
    if (!filePathInfo.exists())
    {
        SimCenterAppWidget::errorMessage(
            "Error in \"GenericModelWidget::inputFromJSON\" - path to generic model CSV table is not valid: " +
            filePath
        );
        return false;
    }

    // load CSV table
    theRVTableView->clear();
    auto tableModel = theRVTableView->getTableModel();
    tableModel->clear();
    tableModel->setHeaderStringList(headers);
    data.clear();
    if(!this->handleLoadVars(filePath,theRVTableView))
    {
        SimCenterAppWidget::errorMessage("Error in \"GenericModelWidget::inputFromJSON\", in loading the variables table");
        return false;
    }

    // make equation display
    this->generateEquation();

    return true;
}


void GenericModelWidget::generateEquation(void)
{

    auto numRows = data.size();

    auto typeText = eqTypeCombo->currentText();

    QString eqnBase;

    if(typeText.compare("lognormal") == 0)
        eqnBase = "ln(Y) = ";
    else
        eqnBase = "Y = ";

    QString start = "<html><head/><body><p><span style=\" font-size:11pt;\ font-style:italic;\">"+eqnBase;

    QString level1Str = start;
    QString level2Str;
    QString level3Str;

    for(int i = 0; i<numRows; ++i)
    {
        auto rvName = data[i][0].toString();
        auto applyLn = data[i][3].toBool();
        auto pow = data[i][4].toInt();

        QString str;

        // The coefficient
        if(i==0)
//            str += "c";
            str += data[i][2].toString();
        else
//            str += "+ c";
            str += " + " + data[i][2].toString();

//        str += QString::number(i);

        if(pow > 0 && !rvName.isEmpty())
        {
            if(pow>=1)
//                str += "*(";
                str += "*";

            // If apply LN
            if(applyLn)
            {
                str += "ln("+rvName+")";
            }
            else
                str += rvName;

            if(pow>=1)
            {
//                str += ")";
                if(pow>1)
                    str += "<sup>"+QString::number(pow)+"</sup>";
            }
        }

        auto level = data[i][1].toInt();

        if(level == 1)
            level1Str += str;
        else if(level == 2)
            level2Str += str;
        else if(level == 3)
            level3Str += str;

    }

    auto end = "</span></p></body></html>";

    level2Str.prepend(level1Str);
    level3Str.prepend(level2Str);

    level2Str +=  end;
    level3Str +=  end;

    eqnLabelLevel1->setText(level1Str);
    eqnLabelLevel2->setText(level2Str);
    eqnLabelLevel3->setText(level3Str);

}


void GenericModelWidget::handleCellChanged(int row, int col)
{
    // Check bounds and return if out
    if(row > data.size()-1 || col > data[row].size()-1)
        return;

    RVTableModel* tableModel = theRVTableView->getTableModel();
    QVariant item = tableModel->item(row,col);

    data[row][col] = item;

    if(col == 0)
    {
//        this->sortData();
        tableModel->populateData(data);
    }

    this->generateEquation();
}


void GenericModelWidget::handleTypeChanged(int type)
{
    Q_UNUSED(type);

    this->generateEquation();
}



bool GenericModelWidget::outputToCsv(const QString& path)
{

    // theRVTableView

    auto RVData = theRVTableView->getTableModel()->getTableData();

    auto RVHeaderValues = theRVTableView->getTableModel()->getHeaderStringList();

    RVData.push_front(RVHeaderValues);

    CSVReaderWriter csvTool;

//    auto finalRVPath = path + QDir::separator() + fileName;

    QString err;
    csvTool.saveCSVFile(RVData,path,err);

    if(!err.isEmpty())
        return false;

    return true;
}


bool GenericModelWidget::handleLoadVars(const QString& filePath, RVTableView* parameterTable)
{

    CSVReaderWriter csvTool;

    QString err;
    auto paramData = csvTool.parseCSVFile(filePath,err);

    if(paramData.isEmpty() || !err.isEmpty())
    {
        SimCenterAppWidget::errorMessage("Error in \"GenericModelWidget\", could not read the input file "+filePath);
        return false;
    }

    auto headers = paramData.front();

    auto indexOfName = headers.indexOf("Variable Label");

    if(indexOfName == -1)
    {
        SimCenterAppWidget::errorMessage("Error in \"GenericModelWidget\", could not find the required column 'Variable Label' in  provided input file "+filePath);
        return false;
    }

    // Pop the front
    paramData.pop_front();

    auto getMapFromVals = [=](const QStringList& vals){

        QMap<QString, QString> result;

        // Return an empty result if inconsistency in size
        if(vals.size() != headers.size())
            return result;

        for(int i = 0; i<vals.size(); ++i)
        {
            auto header = headers.at(i);
            auto val = vals.at(i);

            result.insert(header,val);
        }

        return result;
    };

//    auto numParams = parameterTable->rowCount();

//    if(paramData.size() != numParams)
//    {
//        this->errorMessage("Error, the number of parameters loaded " + QString::number(numParams) + " is not equal the number of parameters in the input file " + QString::number(paramData.size()));
//        return false;
//    }

//    QStringList rowData;

    for(auto&& param : paramData)
    {
        if(param.isEmpty())
        {
            SimCenterAppWidget::errorMessage("Error, an empty variable row in the input file "+filePath);
            return false;
        }

        this->addParamViaCSV(param);

//        auto name = param.at(indexOfName);

//        auto vals = getMapFromVals(param);

//        rowData.clear();
//        auto res = parameterTable->updateRV(name,vals);

//        if(!res)
//        {
//            SimCenterAppWidget::errorMessage("Error updating parameters "+name);
//            return false;
//        }
    }

    return true;
}


void GenericModelWidget::addParamViaCSV(const QStringList &rvData)
{
    auto i = data.size();

    // Create a unique id to identify the specific instance of these parameters
    auto uid = QUuid::createUuid().toString();

    // From model
    auto fromModel = parentName;
    auto desc = "User-created parameter from the generic model widget";

    auto name = "var_"+QString::number(i);

    RV newRV(headers.length(),uid,fromModel,desc);

    // get value by column
    newRV[0] = rvData[0]; // variable label: string
    newRV[1] = QString::number(rvData[1].toInt()); // level: int -> string
    newRV[2] = QString::number(rvData[2].toFloat()); // coefficient: float -> string
    newRV[3] = rvData[3].toLower(); // apply ln (true/false): boolean -> string
    newRV[4] = QString::number(rvData[4].toInt()); // power: int -> string
//    for (int j = 0; j < headers.length(); ++j)
//        newRV[j] = rvData[j];

    data.append(newRV);
    this->sortData();
    RVTableModel* tableModel = theRVTableView->getTableModel();
    tableModel->populateData(data);

    this->generateEquation();

    theRVTableView->resizeEvent(nullptr);
}
