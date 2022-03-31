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
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

#include "MixedDelegate.h"

#include <QComboBox>
#include <QModelIndex>
#include <QLineEdit>
#include <QPainter>

MixedDelegate::MixedDelegate(QObject *parent) : QItemDelegate(parent)
{
    items = QStringList({"NONE"});
}


QWidget *MixedDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &index) const
{

    if(!this->isPreferred(index))
    {
        QComboBox *editor = new QComboBox(parent);
        editor->setFrame(false);
        editor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        editor->setVisible(true);
        editor->addItems(items);
        editor->setEditable(isEditable);

        return editor;
    }
    else
    {
        QWidget *editor = new QWidget(parent);
        editor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        editor->setVisible(true);
        editor->setDisabled(true);

        return editor;
    }

    return nullptr;
}


void MixedDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto value = index.model()->data(index, Qt::DisplayRole).toString();

    if(value.isEmpty())
        return;


    QComboBox *comboBox = dynamic_cast<QComboBox*>(editor);
    if(comboBox)
        comboBox->setCurrentText(value);
}

bool MixedDelegate::isPreferred(const QModelIndex &index) const
{
    auto row = index.row();
    auto model = index.model();
    auto sIndex = model->index(row,col);
    auto str = model->data(sIndex).toString();

    if(str.compare("Preferred") == 0)
        return true;

    return false;
}

void MixedDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(this->isPreferred(index))
        painter->fillRect(option.rect, option.palette.mid());
    else
        QItemDelegate::paint(painter, option, index);
}


void MixedDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{

    QComboBox *comboBox = dynamic_cast<QComboBox*>(editor);

    if(comboBox)
    {
        auto value = comboBox->currentText();

        model->setData(index, value, Qt::EditRole);
    }

}


void MixedDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}


void MixedDelegate::setItems(const QStringList &newItems)
{
    items = newItems;
}


void MixedDelegate::updateComboBoxValues(const QStringList& vals)
{
    items = vals;
}


void MixedDelegate::setIsEditable(bool newIsEditable)
{
    isEditable = newIsEditable;
}


void MixedDelegate::setIsComboBoxEditable(bool newIsEditable)
{
    isEditable = newIsEditable;
}


