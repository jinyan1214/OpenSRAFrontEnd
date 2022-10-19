#ifndef RV_H
#define RV_H
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

#include "SimCenterWidget.h"

#include <QVector>
#include <QVariant>

class RV
{

public:
    RV(int numParams, QString id, QString modelName, QString desc);
    ~RV();

    int size() const;

    QVariant& at(const int index);

    QVariant& operator[](const int index);

    const QVariant &operator[](const int index) const;

    const QString &getName() const;
    void setName(const QString &newName);

    const QStringList &getParamTags() const;
    bool setParamTags(const QStringList &newParams);

    bool updateValue(const QString paramTag, const QVariant value);
    QVariant getValue(const QString paramTag);

    const QString &getUuid() const;

    const QStringList &getFromModelList() const;

    void addModelToList(const QString& modelName);
    void addModelToList(const QStringList& modelNames);

    int removeModelFromList(const QString& modelName);

    const QString &getDescription(void) const;

    const QVector<QVariant> &getData(void) const;

    const QStringList getDataAsStringList(void) const;

private:

    QStringList parameterNames;
    QVector<QVariant> data;

    QString name;
    QString uuid;
    QStringList fromModelList;
    QString description;
};

#endif // RV_H
