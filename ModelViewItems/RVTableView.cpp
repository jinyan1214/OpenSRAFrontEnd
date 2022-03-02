/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
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

// Written by: Dr. Stevan Gavrilovic, UC Berkeley

#include "RVTableModel.h"
#include "RVTableView.h"
#include "VisualizationWidget.h"

#include <QDebug>
#include <QMenu>
#include <QVariant>
#include <QHeaderView>

RVTableView::RVTableView(QWidget *parent) : QTableView(parent)
{
    tableModel = new RVTableModel();
    this->setModel(tableModel);
    this->hide();
    this->setToolTip("Specify the random variables");
    this->verticalHeader()->setVisible(false);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    this->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    this->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    this->setEditTriggers(EditTrigger::DoubleClicked);
    this->setSelectionMode(SelectionMode::SingleSelection);
}


void RVTableView::clear(void)
{
    tableModel->clear();
}


int RVTableView::columnCount(void)
{
    return tableModel->columnCount();
}


int RVTableView::rowCount(void)
{
    return tableModel->rowCount();
}


QString RVTableView::horizontalHeaderItem(int section)
{
    return this->horizontalHeaderItemVariant(section).toString();
}


QVariant RVTableView::horizontalHeaderItemVariant(int section)
{
    auto headerData = tableModel->headerData(section, Qt::Horizontal);

    return headerData;
}


RVTableModel *RVTableView::getTableModel() const
{
    return tableModel;
}


QVariant RVTableView::item(int row, int col)
{
    return tableModel->item(row,col);
}
