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

// Written: fmckenna

#include <OpenSRAComponentSelection.h>
#include <QHBoxLayout>
#include <QTreeView>
#include <QTreeView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <CustomizedItemModel.h>
#include <QModelIndex>
#include <QStackedWidget>
#include <QDebug>
#include <SimCenterAppWidget.h>

OpenSRAComponentSelection::OpenSRAComponentSelection(QWidget *parent)
    :QSplitter(parent)
{

//  this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  this->setContentsMargins(0,5,0,5);

  // Create a TreeView widget
  treeView = new QTreeView();
  standardModel = new CustomizedItemModel;
  rootNode = standardModel->invisibleRootItem();

  infoItemIdx = rootNode->index();

  // Register the model
  treeView->setModel(standardModel);
  treeView->expandAll();
  treeView->setHeaderHidden(true);
  treeView->setEditTriggers(QTreeView::EditTrigger::NoEditTriggers); // Disable Edit
  treeView->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Expanding);

//  treeView->setMinimumWidth(120);
//  treeView->setMaximumWidth(500);


  // Customize the apperance of the menu on the left
  treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff ); // hide the horizontal scroll bar               
  treeView->setObjectName("treeViewOnTheLeft");
  treeView->setIndentation(0);
  treeView->setWordWrap(true);

  QItemSelectionModel *selectionModel= treeView->selectionModel();
  connect(selectionModel,
	  SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	  this,
	  SLOT(selectionChangedSlot(const QItemSelection &, const QItemSelection &)));

  theStackedWidget = new QStackedWidget();
  theStackedWidget->setContentsMargins(0,5,0,5);
  theStackedWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);


  // Add the TreeView  and stacked widget to the layout
  this->addWidget(treeView);
  this->addWidget(theStackedWidget);

  this->setStretchFactor(0, 1);
  this->setStretchFactor(1, 0);

  int index = this->indexOf(treeView);
  this->setCollapsible(index, false);

}

OpenSRAComponentSelection::~OpenSRAComponentSelection()
{
}


bool
OpenSRAComponentSelection::addComponent(QString text, QWidget *theWidget)
{
    if (textIndices.indexOf(text) == -1) {
        QStandardItem *theItem = new QStandardItem(text);
        rootNode->appendRow(theItem);
        QModelIndex modelIndex = theItem->index();
        theStackedWidget->addWidget(theWidget);
        textIndices.append(text);
        modelIndices.append(modelIndex);
        theWidget->setObjectName(text);
        return true;
    } else
        qDebug() << "ComponentSelection: text: " << text << " option already exists";

    return false;
}

void
OpenSRAComponentSelection::selectionChangedSlot(const QItemSelection &, const QItemSelection &)
{
    //
    // get the text of the selected item
    //

    const QModelIndex index = treeView->selectionModel()->currentIndex();
    QString selectedText = index.data(Qt::DisplayRole).toString();

    //
    // find text in list
    //

    int stackIndex = textIndices.lastIndexOf(selectedText);

    //
    // get stacked widget to display current if of course it exists
    //

    if (stackIndex != -1) {

        QWidget *theCurrentWidget = theStackedWidget->currentWidget();
        if (theCurrentWidget != 0) {
            SimCenterAppWidget *simCenterWidget = dynamic_cast<SimCenterAppWidget*>(theCurrentWidget);
            if (simCenterWidget)
                simCenterWidget->setCurrentlyViewable(false);
        }

        theStackedWidget->setCurrentIndex(stackIndex);

        theCurrentWidget = theStackedWidget->currentWidget();
        if (theCurrentWidget != 0) {
            SimCenterAppWidget *simCenterWidget = dynamic_cast<SimCenterAppWidget*>(theCurrentWidget);
            if (simCenterWidget)
                simCenterWidget->setCurrentlyViewable(true);
        }

    }
}

QWidget *
OpenSRAComponentSelection::swapComponent(QString text, QWidget *theWidget)
{
    QWidget *theRes = NULL;

    //
    // find text iin list
    //

    int index = textIndices.indexOf(text);
    //
    // get stacked widget to display current if of course it exists
    //

    if (index != -1) {
        theRes=theStackedWidget->widget(index);
        if (theRes != NULL) {
            theStackedWidget->removeWidget(theRes);
        }
       theStackedWidget->insertWidget(index, theWidget);
    }
    return theRes;
}


QString
OpenSRAComponentSelection::selectedComponentText(void)
{
    auto currIndex = theStackedWidget->currentIndex();

    if (currIndex < 0 || currIndex > textIndices.size() -1)
        return QString();

    return textIndices.at(currIndex);
}


bool
OpenSRAComponentSelection::displayComponent(QString text)
{
    //
    // find index of text in list and display corresponding widget if index found
    //

    int index = textIndices.indexOf(text);

    if (index != -1) {

        QModelIndex index1 = modelIndices.at(index);
        treeView->setCurrentIndex(index1);
        theStackedWidget->setCurrentIndex(index);
        return true;
    }

    return false;
}

bool
OpenSRAComponentSelection::displayComponent(int index)
{
    //
    // find index of text in list and display corresponding widget if index found
    //

    if (index >= 0 && index <= modelIndices.size()-1) {

        QModelIndex index1 = modelIndices.at(index);
        treeView->setCurrentIndex(index1);
        theStackedWidget->setCurrentIndex(index);
        return true;
    }

    return false;
}

void
OpenSRAComponentSelection::setMinWidth(const int width)
{
    treeView->setMinimumWidth(width);
}


void
OpenSRAComponentSelection::setMaxWidth(const int width)
{
    treeView->setMaximumWidth(width);
}


void
OpenSRAComponentSelection::setItemWidthHeight(const int width, const int height)
{
    auto customModel = dynamic_cast<CustomizedItemModel*>(standardModel);

    if(customModel)
    {
        customModel->setItemWidthHeight(width,height);
    }
}
