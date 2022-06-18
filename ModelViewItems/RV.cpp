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

RV::RV(int numParams, QString id, QString modelName) : uuid(id), fromModel(modelName)
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
        qDebug()<<"Could not find the parameter with the name "+paramName;

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

const QString &RV::getFromModel() const
{
    return fromModel;
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


void RV::setParamTags(const QStringList &newParams)
{
     parameterNames = newParams;
}


const QVariant& RV::operator[](const int index) const
{
    return data[index];
}


QVariant& RV::at(const int index)
{
    return data[index];
}

