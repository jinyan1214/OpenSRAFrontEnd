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

#include "RV.h"

#include <qdebug.h>

RV::RV(int numParams, QString id, QString modelName, QString desc) : uuid(id), fromModelList(modelName), description(desc)
{
    data.resize(numParams);
}


RV::~RV()
{

}


QVariant RV::getValue(const QString paramName)
{
    auto idx = parameterNames.indexOf(paramName);

    if(idx == -1)
    {
        qDebug()<<"Developer warning, could with the name "+paramName;

        return QVariant();
    }

    // Check if the param tags and the data are the same size
    if(paramName.size() != data.size())
    {
        qDebug()<<"Developer warning, there is a mismatch between the RV values and parameter names "+paramName;

        return QVariant();
    }


    return data[idx];
}


const QString &RV::getUuid() const
{
    return uuid;
}


const QStringList &RV::getFromModelList() const
{
    return fromModelList;
}


const QString &RV::getDescription(void) const
{
    return description;
}

const QVector<QVariant> &RV::getData(void) const
{
    return data;
}


const QStringList RV::getDataAsStringList(void) const
{
    QStringList result;

    for(auto&& val : data)
    {
        if (val.type() == QVariant::Int)
            result.append(QString::number(val.toInt()));
        else if (val.type() == QVariant::Double)
            result.append(QString::number(val.toDouble()));
        else if (val.type() == QVariant::String)
             result.append(val.toString());
        else if (val.type() == QVariant::StringList)
             result.append(val.toStringList().join(","));
        else if (val.type() == QVariant::Invalid) // unknown
             result.append("");
        else
             qDebug()<<"Error no support for type "<<val.type()<< "in "<<__PRETTY_FUNCTION__;

    }

    return result;
}


int RV::size() const
{
    return data.size();
}


QVariant& RV::operator[](const int index)
{
    return data[index];
}


const QString &RV::getName() const
{
    return name;
}


void RV::setName(const QString &newName)
{
    name = newName;
}


const QStringList &RV::getParamTags() const
{
    return parameterNames;
}


bool RV::setParamTags(const QStringList &newParams)
{
     parameterNames = newParams;

     if(parameterNames.size() != data.size())
     {
         qDebug()<<"Developer warning, inconsistency between the number of parameter names and the number of values";
     }

     return true;
}


const QVariant& RV::operator[](const int index) const
{
    return data[index];
}


QVariant& RV::at(const int index)
{
    return data[index];
}


void RV::addModelToList(const QString& modelName)
{
    if(fromModelList.contains(modelName))
        return;

    fromModelList.append(modelName);
    data[2] = QVariant(fromModelList);
}


void RV::addModelToList(const QStringList& modelNames)
{
    fromModelList.append(modelNames);
    fromModelList.removeDuplicates();

    data[2] = QVariant(fromModelList.join(", "));
}


int RV::removeModelFromList(const QString& modelName)
{
    auto idx = fromModelList.indexOf(modelName);

    if(idx == -1)
        return -1;

    fromModelList.removeAt(idx);

    if(!fromModelList.isEmpty())
    {
        data[2] = QVariant(fromModelList.join(", "));
    }
    else
    {
        data[2] = QVariant("");
        return 1;
    }

    return 0;
}


bool RV::updateValue(const QString paramTag, const QVariant value)
{
    auto idx = parameterNames.indexOf(paramTag);

    if(idx == -1)
    {
        qDebug()<<"Developer warning, could not find the parameter with the name "+paramTag;

        return false;
    }

    if(value.isNull() || !value.isValid())
        return true;

    data[idx] = value;

    return true;
}

