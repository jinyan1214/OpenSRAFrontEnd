#ifndef GenericModelWidget_H
#define GenericModelWidget_H
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

#include "SimCenterAppWidget.h"
#include "JsonSerializable.h"
#include "RV.h"

#include <QVariant>

class RVTableView;
class ComboBoxDelegate;

class QLabel;
class QVBoxLayout;
class QDialog;
class QComboBox;
class QLineEdit;

class GenericModelWidget : public SimCenterAppWidget, public JsonSerializable
{
    Q_OBJECT
public:
    explicit GenericModelWidget(QString parName, QJsonObject &methodObj, QWidget *parent = 0);
    ~GenericModelWidget();

    bool inputFromJSON(QJsonObject &jsonObj) override;
    bool outputToJSON(QJsonObject &jsonObj) override;

    void reset(void) override;

    bool outputToCsv(const QString& path);

public slots:
    void clear(void) override;
    void addParam(void);
    void removeParam(void);
    void handleCellChanged(int row, int col);
    void handleTypeChanged(int type);


private:

    void sortData(void);

    void generateEquation(void);

    bool handleLoadVars(const QString& filePath, RVTableView* parameterTable);

    QVector<RV> data;
    void makeRVWidget(QJsonObject &methodObj);
    QVBoxLayout* verticalLayout = nullptr;

    QLabel* eqnLabelLevel1 = nullptr;
    QLabel* eqnLabelLevel2 = nullptr;
    QLabel* eqnLabelLevel3 = nullptr;

    ComboBoxDelegate* levelComboDelegate = nullptr;
    ComboBoxDelegate* applyLnComboDelegate = nullptr;
    ComboBoxDelegate* powerComboDelegate = nullptr;

    RVTableView* theRVTableView = nullptr;

    QComboBox* eqTypeCombo = nullptr;
    QLineEdit* returnParamLineEdit = nullptr;
    QComboBox* upstreamCatCombo = nullptr;
    QLineEdit* upstreamParamLineEdit = nullptr;
    QStringList upstreamCatList;

    QString parentName;
};

#endif // GenericModelWidget_H
