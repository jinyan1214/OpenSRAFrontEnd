#ifndef RANDOM_VARIABLES_CONTAINER_H
#define RANDOM_VARIABLES_CONTAINER_H

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

#include "RV.h"

#include <SimCenterWidget.h>

class RVTableView;
class ComboBoxDelegate;
class MixedDelegate;
class LineEditDelegate;

class QVBoxLayout;
class QDialog;
class QCheckBox;

class RandomVariablesWidget : public SimCenterWidget
{
    Q_OBJECT
public:
    explicit RandomVariablesWidget(QWidget *parent = 0);
    ~RandomVariablesWidget();

    bool inputFromJSON(QJsonObject &jsonObject);
    bool outputToJSON(QJsonObject &jsonObject);

    void addRVs(QVector<RV>& RVs);

    QStringList getRandomVariableNames(void);

    bool outputToCsv(const QString& path);

    MixedDelegate *getGisMapsComboDelegate() const;
    MixedDelegate *getColDataComboDelegate() const;

public slots:

    bool addNewParameter(const QString& name, const QString& fromModel, const QString& desc, const QString& uuid, const QString& type);
    bool removeParameter(const QString& uuid, const QString& fromModel);

    // To handle when sigma and cov are changed
    void handleCellChanged(int row, int col);

    // When the source is changed from preferred, from infrastructure table, etc.
    void handleSourceChanged(int val);

    void clear(void);

    // Function to check if an input parameter exists, returns a UUID string if exists and empty string if it does not    
    QString checkIfParameterExists(const QString& name, bool& OK);

private:

    bool addRandomVariable(const QString& name, const QString& fromModel, const QString& desc, const QString& uuid);
    bool addRandomVariable(RV& newRV);

    bool addNewModelToExistingParameter(const RV& rv, const QStringList& fromModel, RVTableView* database);

    bool addConstant(RV& newConstant);
    bool addConstant(const QString& name, const QString& fromModel, const QString& desc, const QString& uuid);

    bool removeRandomVariable(const QString &uuid, const QString &fromModel);
    bool removeConstant(const QString &uuid, const QString &fromModel);

    bool handleLoadVars(const QString& filePath, RVTableView* parameterTable);

    int addRVsType;
    void makeRVWidget(void);
    QVBoxLayout *verticalLayout;

    ComboBoxDelegate* sourceComboDelegate = nullptr;
    MixedDelegate* distTypeComboDelegate = nullptr;
    MixedDelegate* colDataComboDelegate = nullptr;
    MixedDelegate* gisMapsComboDelegate = nullptr;
    LineEditDelegate* LEDelegate = nullptr;

    RVTableView* theRVTableView = nullptr;
    RVTableView* theConstantTableView = nullptr;

    QCheckBox *checkbox;

    QStringList constantTableHeaders;
    QStringList RVTableHeaders;

    // Function to check if an RV exists
    QString checkIfRVExists(const QString& name);
    QString checkIfConstantExists(const QString& name);

    bool checkIfRVuuidExists(const QString& name);
    bool checkIfConstantuuidExists(const QString& name);

    RV createNewRV(const QString& name, const QString& fromModel, const QString& desc, const QString& uuid);
    RV createNewConstant(const QString& name, const QString& fromModel, const QString& desc, const QString& uuid);

    QString pathToRvFile;
    QString pathToFixedFile;

};

#endif // RANDOM_VARIABLES_CONTAINER_H
