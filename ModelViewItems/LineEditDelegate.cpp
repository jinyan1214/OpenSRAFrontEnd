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

#include "LineEditDelegate.h"

#include <QLineEdit>
#include <QDoubleValidator>
#include <QModelIndex>
#include <QLineEdit>
#include <QPainter>

LineEditDelegate::LineEditDelegate(QObject *parent) : QItemDelegate(parent)
{

}


QWidget *LineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &index) const
{
    if(!this->isPreferred(index) && this->hasDistribution(index))
    {
        QLineEdit *editor = new QLineEdit(parent);
        editor->setFrame(false);
        editor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        editor->setVisible(true);
        editor->setValidator(new QDoubleValidator());
        editor->setEnabled(isEditable);

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


void LineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto value = index.model()->data(index, Qt::DisplayRole).toString();

    if(value.isEmpty())
        return;

    QLineEdit *LEBox = dynamic_cast<QLineEdit*>(editor);
    if(LEBox)
        LEBox->setText(value);
}


void LineEditDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(this->isPreferred(index) || !this->hasDistribution(index))
        painter->fillRect(option.rect, option.palette.mid());
    else
        QItemDelegate::paint(painter, option, index);
}


void LineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{

    QLineEdit *LEBox = dynamic_cast<QLineEdit*>(editor);

    if(LEBox)
    {
        auto value = LEBox->text();

        model->setData(index, value, Qt::EditRole);
    }

}


void LineEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}


void LineEditDelegate::setIsEditable(bool newIsEditable)
{
    isEditable = newIsEditable;
}


bool LineEditDelegate::isPreferred(const QModelIndex &index) const
{
    auto row = index.row();
    auto model = index.model();
    auto sIndex = model->index(row,sourceCol);
    auto str = model->data(sIndex).toString();

    if(str.compare("Preferred") == 0)
        return true;

    return false;
}

bool LineEditDelegate::hasDistribution(const QModelIndex &index) const
{
    auto row = index.row();
    auto model = index.model();
    auto sIndex = model->index(row,distCol);
    auto str = model->data(sIndex).toString();

    if(str.compare("") == 0)
        return false;

    return true;
}

