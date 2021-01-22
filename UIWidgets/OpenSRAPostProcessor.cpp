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

// Written by: Stevan Gavrilovic

#include "CSVReaderWriter.h"
#include "GeneralInformationWidget.h"
#include "MainWindowWorkflowApp.h"
#include "OpenSRAPostProcessor.h"
#include "REmpiricalProbabilityDistribution.h"
#include "TablePrinter.h"
#include "TreeItem.h"
#include "VisualizationWidget.h"
#include "WorkflowAppOpenSRA.h"
#include "ResultsMapViewWidget.h"
#include "MutuallyExclusiveListWidget.h"

#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QChartView>
#include <QComboBox>
#include <QDir>
#include <QDockWidget>
#include <QFileInfo>
#include <QFontMetrics>
#include <QGraphicsLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineSeries>
#include <QMenuBar>
#include <QPixmap>
#include <QPrinter>
#include <QSplitter>
#include <QStackedBarSeries>
#include <QStringList>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextCursor>
#include <QTextTable>
#include <QValueAxis>

// GIS headers
#include "Basemap.h"
#include "FeatureTable.h"
#include "Map.h"
#include "MapGraphicsView.h"

using namespace QtCharts;

OpenSRAPostProcessor::OpenSRAPostProcessor(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Create a view menu for the dockable windows
    mainWidget = new QSplitter(this);
    mainWidget->setOrientation(Qt::Horizontal);
    mainWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    listWidget = new MutuallyExclusiveListWidget(this, "Results");

    PGVTreeItem = listWidget->addItem("Shaking Induced");
    PGDTreeItem = listWidget->addItem("Deformation Induced");

    listWidget->addItem("PGV (Low)",PGVTreeItem);
    listWidget->addItem("PGV (Med)",PGVTreeItem);
    listWidget->addItem("PGV (High)",PGVTreeItem);

    listWidget->addItem("Lateral Spreading (Low)",PGDTreeItem);
    listWidget->addItem("Lateral Spreading (Med)",PGDTreeItem);
    listWidget->addItem("Lateral Spreading (High)",PGDTreeItem);

    listWidget->addItem("Liquefaction (Low)",PGDTreeItem);
    listWidget->addItem("Liquefaction (Med)",PGDTreeItem);
    listWidget->addItem("Liquefaction (High)",PGDTreeItem);

    PGVTreeItem->setIsCheckable(false);
    PGDTreeItem->setIsCheckable(false);

    connect(theVisualizationWidget,&VisualizationWidget::emitScreenshot,this,&OpenSRAPostProcessor::assemblePDF);

    // Create the table that will show the Component information
    tableWidget = new QWidget(this);

    PGVResultsTableWidget = new QTableWidget(this);
    PGVResultsTableWidget->verticalHeader()->setVisible(false);
    PGVResultsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    PGVResultsTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    PGVResultsTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    PGVResultsTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    PGVResultsTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    PGVResultsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    PGVResultsTableWidget->setVisible(false);

    // Create a map view that will be used for selecting the grid points
    mapViewMainWidget = theVisualizationWidget->getMapViewWidget();

    mapViewSubWidget = std::make_unique<ResultsMapViewWidget>(nullptr);

    // Popup stuff
    // Once map is set, connect to MapQuickView mouse clicked signal
    // connect(mapViewSubWidget.get(), &ResultsMapViewWidget::mouseClick, theVisualizationWidget, &VisualizationWidget::onMouseClickedGlobal);

    mapViewSubWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    mainWidget->addWidget(mapViewSubWidget.get());
    mainWidget->addWidget(listWidget);

    mainLayout->addWidget(mainWidget);

    // The number of header rows in the Pelicun results file
    numHeaderRows = 1;

    this->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
}


void OpenSRAPostProcessor::importResults(const QString& pathToResults)
{
    qDebug() << "OpenSRAPostProcessor: " << pathToResults;

    // Remove old csv files in the output pathToResults
    QDir resultsDir(pathToResults);

    const QFileInfo existingFilesInfo(pathToResults);

    // Get the existing files in the folder to see if we already have the record
    QStringList acceptableFileExtensions = {"*.csv"};
    QStringList existingCSVFiles = resultsDir.entryList(acceptableFileExtensions, QDir::Files);

    QString errMsg;

    if(existingCSVFiles.empty())
    {
        QStringList acceptableFileExtensions = {"*.*"};
        QStringList existingFiles = existingFilesInfo.dir().entryList(acceptableFileExtensions, QDir::Files);
        qDebug() << "FILES IN FOLDER: " << existingFiles;
        errMsg = "The results folder is empty";
        throw errMsg;
    }

    QString PGDResultsSheet;
    QString PGVResultsSheet;

    for(auto&& it : existingCSVFiles)
    {
        if(it.startsWith("RepairRatePGD"))
            PGDResultsSheet = it;
        else if(it.startsWith("RepairRatePGV"))
            PGVResultsSheet = it;
    }

    CSVReaderWriter csvTool;

    if(!PGVResultsSheet.isEmpty())
    {
        RepairRatePGV = csvTool.parseCSVFile(pathToResults + QDir::separator() + PGVResultsSheet,errMsg);
        if(!errMsg.isEmpty())
            throw errMsg;

        if(!RepairRatePGV.empty())
            this->processPGVResults(RepairRatePGV);
        else
        {
            errMsg = "The DV results are empty";
            throw errMsg;
        }
    }

    if(!PGDResultsSheet.isEmpty())
    {
        RepairRatePGD = csvTool.parseCSVFile(pathToResults + QDir::separator() + PGDResultsSheet,errMsg);
        if(!errMsg.isEmpty())
            throw errMsg;
    }


}


int OpenSRAPostProcessor::processPGVResults(const QVector<QStringList>& DVResults)
{
    if(DVResults.size() < numHeaderRows)
    {
        QString msg = "No results to import!";
        throw msg;
    }

    auto numHeaderColumns = DVResults.at(0).size();

    QStringList tableHeadings;

    for(int i = 0; i<numHeaderColumns; ++i)
    {
        QString headerStr =  DVResults.at(0).at(i);

        tableHeadings<<headerStr;
    }

    PGVResultsTableWidget->setColumnCount(tableHeadings.size());
    PGVResultsTableWidget->setHorizontalHeaderLabels(tableHeadings);
    PGVResultsTableWidget->setRowCount(DVResults.size()-numHeaderRows);

    // Get the buildings database
    auto thePipelineDB = theVisualizationWidget->getPipelineDatabase();

    // Start at the row where headers end
    for(int i = numHeaderRows, count = 0; i<DVResults.size(); ++i, ++count)
    {
        auto inputRow = DVResults.at(i);

        auto pipelineID = objectToInt(inputRow.at(0));

        auto pipeline = thePipelineDB->getComponent(pipelineID);

        if(pipeline.ID == -1)
            throw QString("Could not find the pipeline ID " + QString::number(pipelineID) + " in the database");

        for(int j = 1; j<numHeaderColumns; ++j)
        {
            pipeline.ResultsValues.insert(tableHeadings.at(j),inputRow.at(j).toDouble());
        }

        pipeline.ID = pipelineID;

        pipelinesVec.push_back(pipeline);


        // Populate the table
        for(int j = 0; j<numHeaderColumns; ++j)
        {
            auto tableVal = QString(inputRow.at(j));

            auto tableItem = new QTableWidgetItem(tableVal);

            PGVResultsTableWidget->setItem(count,j, tableItem);
        }

        auto arcGISFeature = pipeline.ComponentFeature;

        if(arcGISFeature == nullptr)
            throw QString("ArcGIS feature is a null pointer for component ID " + QString::number(pipelineID));

        arcGISFeature->attributes()->replaceAttribute("RepairRate",0.0);

        arcGISFeature->featureTable()->updateFeature(arcGISFeature);
    }


    return 0;
}


int OpenSRAPostProcessor::printToPDF(const QString& outputPath)
{
    outputFilePath = outputPath;

    theVisualizationWidget->takeScreenShot();

    return 0;
}


void OpenSRAPostProcessor::processResultsSubset(const std::set<int>& selectedComponentIDs)
{

    if(selectedComponentIDs.empty())
        return;

    if(RepairRatePGD.size() < numHeaderRows)
    {
        QString msg = "No results to import!";
        throw msg;
    }

    if(RepairRatePGD.at(numHeaderRows).isEmpty() || RepairRatePGD.last().isEmpty())
    {
        QString msg = "No values in the cells";
        throw msg;
    }

    auto firstID = objectToInt(RepairRatePGD.at(numHeaderRows).at(0));

    auto lastID = objectToInt(RepairRatePGD.last().at(0));

    QVector<QStringList> DVsubset(&RepairRatePGD[0],&RepairRatePGD[numHeaderRows]);

    for(auto&& id : selectedComponentIDs)
    {
        // Check that the ID falls within the bounds of the data
        if(id<firstID || id>lastID)
        {
            QString msg = "ID " + QString::number(id) + " is out of bounds of the results";
            throw msg;
        }

        auto found = false;
        for(int i = numHeaderRows; i<RepairRatePGD.size(); ++i)
        {
            auto inputRow = RepairRatePGD.at(i);

            auto buildingID = objectToInt(inputRow.at(0));

            if(id == buildingID)
            {
                DVsubset << inputRow;
                found = true;
                break;
            }
        }

        if(!found)
        {
            QString msg = "ID " + QString::number(id) + " cannot be found in the results";
            throw msg;
        }
    }

    this->processPGVResults(DVsubset);
}


int OpenSRAPostProcessor::assemblePDF(QImage screenShot)
{
    // The printer
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::Letter);
    printer.setPageMargins(25.4, 25.4, 25.4, 25.4, QPrinter::Millimeter);
    printer.setFullPage(true);
    qreal leftMargin, topMargin;
    printer.getPageMargins(&leftMargin,&topMargin,nullptr,nullptr,QPrinter::Point);
    printer.setOutputFileName(outputFilePath);

    // Create a new document
    QTextDocument* document = new QTextDocument();
    QTextCursor cursor(document);
    document->setDocumentMargin(25.4);
    document->setDefaultFont(QFont("Helvetica"));

    // Define font styles
    QTextCharFormat normalFormat;
    normalFormat.setFontWeight(QFont::Normal);

    QTextCharFormat titleFormat;
    titleFormat.setFontWeight(QFont::Bold);
    titleFormat.setFontCapitalization(QFont::AllUppercase);
    titleFormat.setFontPointSize(normalFormat.fontPointSize() * 2.0);

    QTextCharFormat captionFormat;
    captionFormat.setFontWeight(QFont::Light);
    captionFormat.setFontPointSize(normalFormat.fontPointSize() / 2.0);
    captionFormat.setFontItalic(true);

    QTextCharFormat boldFormat;
    boldFormat.setFontWeight(QFont::Bold);

    QFontMetrics normMetrics(normalFormat.font());
    auto lineSpacing = normMetrics.lineSpacing();

    // Define alignment formats
    QTextBlockFormat alignCenter;
    alignCenter.setLineHeight(lineSpacing, QTextBlockFormat::LineDistanceHeight) ;
    alignCenter.setAlignment(Qt::AlignCenter);

    QTextBlockFormat alignLeft;
    alignLeft.setAlignment(Qt::AlignLeft);
    alignLeft.setLineHeight(lineSpacing, QTextBlockFormat::LineDistanceHeight) ;

    cursor.movePosition(QTextCursor::Start);

    cursor.insertBlock(alignCenter);

    cursor.insertText("\nOpenSRA Tool\n",titleFormat);

    cursor.insertText("Results Summary\n",boldFormat);

    cursor.setBlockFormat(alignLeft);

    TablePrinter prettyTablePrinter;
    prettyTablePrinter.printToTable(&cursor, PGVResultsTableWidget,"Asset Results");

    document->print(&printer);

    return 0;
}


void OpenSRAPostProcessor::sortTable(int index)
{
    if(index == 0)
        PGVResultsTableWidget->sortByColumn(index,Qt::AscendingOrder);
    else
        PGVResultsTableWidget->sortByColumn(index,Qt::DescendingOrder);

}


void OpenSRAPostProcessor::setCurrentlyViewable(bool status){

    if (status == true)
        mapViewSubWidget->setCurrentlyViewable(status);
}


void OpenSRAPostProcessor::clear(void)
{
    RepairRatePGV.clear();
    RepairRatePGD.clear();
    pipelinesVec.clear();

    outputFilePath.clear();

    PGVResultsTableWidget->clear();
}

