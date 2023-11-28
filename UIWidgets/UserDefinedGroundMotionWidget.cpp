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

// Written by: Barry Zheng (Modified from Stevan Gavrilovic's script)

#include "CSVReaderWriter.h"
#include "LayerTreeView.h"
#include "UserDefinedGroundMotionWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"
#include "SimCenterUnitsWidget.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"
#include "AssetInputDelegate.h"
#include "ClickableLabel.h"
#include "ComponentDatabaseManager.h"

#include <QApplication>
#include <QHeaderView>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QJsonObject>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QComboBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QDir>
#include <QString>

#include "QGISVisualizationWidget.h"
#include <qgsvectorlayer.h>


UserDefinedGroundMotionWidget::UserDefinedGroundMotionWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    theComponentDb = ComponentDatabaseManager::getInstance()->createAssetDb("SiteCPTData");

    theComponentDb->setOffset(1);

    eventFile = "";
    gmDataDir = "";

    QVBoxLayout *layout = new QVBoxLayout(this);
    QStackedWidget* userDefinedGMWidget = this->getUserDefinedGroundMotionWidget();
    layout->addWidget(userDefinedGMWidget);
    layout->addStretch();
}


UserDefinedGroundMotionWidget::~UserDefinedGroundMotionWidget()
{

}


bool UserDefinedGroundMotionWidget::outputAppDataToJSON(QJsonObject &/*jsonObject*/) {

    return true;
}


bool UserDefinedGroundMotionWidget::outputToJSON(QJsonObject &jsonObj)
{
    QJsonObject outputObj;

    QFileInfo pathToSummaryTXT(summaryFileLineEdit->text());
    QFileInfo pathToGMDataFolder(GMDirLineEdit->text());

    jsonObj.insert("pathToSummaryTXTTXT",pathToSummaryTXT.absoluteFilePath());
    jsonObj.insert("PathToGMDataFolder",pathToGMDataFolder.absoluteFilePath());

    return true;
}


bool UserDefinedGroundMotionWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    Q_UNUSED(jsonObj);
    return true;
}


bool UserDefinedGroundMotionWidget::inputFromJSON(QJsonObject &jsonObject)
{
    // set the line
    if (jsonObject.contains("pathToSummaryTXT"))
    {
        auto sum_path = jsonObject["pathToSummaryTXT"].toString();

        // if string is not empty, then continue, else don't load into CPT widget tab
        if (sum_path.length() > 0)
        {
            // Check for relative/absolute paths
            QFileInfo pathToSummaryTXT(sum_path);
            if (!pathToSummaryTXT.exists())
                sum_path=QDir::currentPath() + QDir::separator() + sum_path;
            pathToSummaryTXT.setFile(sum_path);

            summaryFileLineEdit->setText(pathToSummaryTXT.absoluteFilePath());
            eventFile = sum_path;

            // set the line
            if (jsonObject.contains("pathToGMDataFolder"))
            {
                auto cpt_dir = jsonObject["pathToGMDataFolder"].toString();

                // Check for relative/absolute paths
                QFileInfo pathToGMDataFolder(cpt_dir);
                if (!pathToGMDataFolder.exists())
                    cpt_dir=QDir::currentPath() + QDir::separator() + cpt_dir;
                pathToGMDataFolder.setFile(cpt_dir);

                GMDirLineEdit->setText(pathToGMDataFolder.absoluteFilePath());
                gmDataDir = cpt_dir;
            }

            // load the events
            this->loadUserGMData();
        }
    }

    return true;
}


QStackedWidget* UserDefinedGroundMotionWidget::getUserDefinedGroundMotionWidget(void)
{
    if (theStackedWidget)
        return theStackedWidget;

    theStackedWidget = new QStackedWidget();

    //
    // file and dir input
    //
    fileInputWidget = new QWidget();

//    QGridLayout *fileLayout = new QGridLayout(fileInputWidget);
    QVBoxLayout *fileLayout = new QVBoxLayout(fileInputWidget);

    int count = 0;

    // File main message
    QLabel* mainText = new QLabel("This tab allows you to provide the user-defined ground motions in gridded format. ");
    QFont fontMainText = mainText->font();
    fontMainText.setWeight(QFont::Bold);
    mainText->setFont(fontMainText);
//    fileLayout->addWidget(mainText, count,1, Qt::AlignLeft);
//    count ++;

//    QHBoxLayout* headerTextBox = new QHBoxLayout();
//    headerTextBox->addWidget(mainText,0,Qt::AlignLeft);
//    headerTextBox->addStretch(1);
//    fileLayout->addLayout(headerTextBox);
    fileLayout->addWidget(mainText);
    count ++;

    // skip some space vertically
//    QLabel* emptyText = new QLabel("");
//    fileLayout->addWidget(emptyText);
//    count ++;

    // Summary file
    QHBoxLayout* summaryLayout = new QHBoxLayout();
    QLabel* selectSummaryText = new QLabel("1. Event summary file in TXT format:");
//    QFont fontSummaryText = selectSummaryText->font();
//    fontSummaryText.setWeight(QFont::Bold);
//    selectSummaryText->setFont(fontSummaryText);
    summaryFileLineEdit = new QLineEdit();
    QPushButton *browseSummaryButton = new QPushButton("Browse");

    connect(browseSummaryButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    QLabel* summaryNoteText = new QLabel(
        "\tExpected format for event summary file (table for scenarios start on row 11):\n"
        "\t\tRow  1| # Run control file for scenario maps\n"
        "\t\tRow  2| # Missing value\n"
        "\t\tRow  3| -1\n"
        "\t\tRow  4| # Bounds. Lower left corner reference; increasing longitude then latitude\n"
        "\t\tRow  5| 37.0 -122.5 38.0 -121.5\n"
        "\t\tRow  6| # Grid spacing\n"
        "\t\tRow  7| 0.00500\n"
        "\t\tRow  8| # Number of scenario maps\n"
        "\t\tRow  9| 20\n"
        "\t\tRow 10| # ID, Magnitude, and file name of scenarios\n"
        "\t\tRow 11| 0 7.5 4.6626e-05 gm_data-0000.txt\n"
        "\t\tRow 12| 1 7.25 4.6626e-05 gm_data-0001.txt\n"
        "\tNote: 1. The keywords \"Missing value\", \"Bounds\", and \"Number of scenario maps\" are used locate the respective information in the header block."
        "The keywords and values should be placed in the consecutive rows, but their line positions can change.\n"
        "\t      2. Table for scenarios is expected to start on row 11, is space-delimited, and in the order of: \"EventID\" \"Magnitude\" \"Annual Rate\" \"File Name\""
    );

    // Grid layout is specified with row/col integers
    summaryLayout->addWidget(selectSummaryText);
    summaryLayout->addWidget(summaryFileLineEdit);
    summaryLayout->addWidget(browseSummaryButton);
    fileLayout->addLayout(summaryLayout);
    fileLayout->addWidget(summaryNoteText);
    count ++;

    // folder with GM data
    QHBoxLayout* gmDirLayout = new QHBoxLayout();
    QLabel* selectFolderText = new QLabel("2. Folder containing ground motion files in TXT format (filenames must match those in the event summary file):");
//    QFont fontFolderText = selectFolderText->font();
//    fontFolderText.setWeight(QFont::Bold);
//    selectFolderText->setFont(fontFolderText);
    GMDirLineEdit = new QLineEdit();
    QPushButton *browseFolderButton = new QPushButton("Browse");

    connect(browseFolderButton,SIGNAL(clicked()),this,SLOT(chooseGMDirDialog()));

    QLabel* gmDirNoteText = new QLabel(
        "\tExpected format for event summary file (table header expected to start on row 5, and data expected to start on row 6):\n"
        "\t\tRow  1| # Header line 1\n"
        "\t\tRow  2| # Header line 2\n"
        "\t\tRow  3| # Header line 3\n"
        "\t\tRow  4| # Header line 4\n"
        "\t\tRow  5| # Latitude (N), Longitude (E), PGA (g), PGV (cm/s), SA(T=0.3s) (g), SA(T=1.0s) (g), SA(T=3.0s) (g) \n"
        "\t\tRow  6| 37.000,-122.500,-1.0,-1.0,-1.0,-1.0,-1.0\n"
        "\t\tRow  7| 37.000,-122.495,-1.0,-1.0,-1.0,-1.0,-1.0\n"
        "\t\tRow  8| 37.000,-122.490,-1.0,-1.0,-1.0,-1.0,-1.0\n"
        "\tNote: 1. Metadata for each scenario is based on the event summary file. Information placed in the header block is not used at this time.\n"
        "\t      2. The header for the data table is expected to start on row 5. The data table is expected to start on row 6."
    );

    gmDirLayout->addWidget(selectFolderText);
    gmDirLayout->addWidget(GMDirLineEdit);
    gmDirLayout->addWidget(browseFolderButton);
    fileLayout->addLayout(gmDirLayout);
    fileLayout->addWidget(gmDirNoteText);
    count ++;

    siteListTableWidget = new ComponentTableView();
    siteListTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
//    siteListTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//    siteListTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    siteListTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    siteListTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    siteListTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
//    siteListTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
//    siteListTableWidget->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Maximum);

//    connect(siteListTableWidget,&QAbstractItemView::clicked,this,&UserDefinedGroundMotionWidget::handleRowSelect);

    // skip some space vertically
    QLabel* emptyText = new QLabel("");
    fileLayout->addWidget(emptyText);

    tableHeader = new QLabel("Event Summary Table");
    QFont tableHeaderText = tableHeader->font();
    tableHeaderText.setWeight(QFont::Bold);
    tableHeader->setFont(tableHeaderText);
    tableHeader->hide();

    fileLayout->addWidget(tableHeader,0,Qt::AlignCenter);
    count ++;

//    fileLayout->addWidget(tableSplitter,count,0,1,3);
    fileLayout->addWidget(siteListTableWidget,0,Qt::AlignCenter);
    count ++;
    fileLayout->addStretch(1);
    count ++;

    //
    // progress bar
    //

    progressBarWidget = new QWidget();
    auto progressBarLayout = new QVBoxLayout(progressBarWidget);
    progressBarWidget->setLayout(progressBarLayout);

    auto progressText = new QLabel("Loading user ground motion data. This may take a while.");
    progressLabel =  new QLabel(" ");
    progressBar = new QProgressBar();

    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto vspacer2 = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addWidget(progressText,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressLabel,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressBar);
    progressBarLayout->addItem(vspacer2);
    progressBarLayout->addStretch(1);

    //
    // add file and progress widgets to stacked widgets, then set defaults
    //

    theStackedWidget->addWidget(fileInputWidget);
    theStackedWidget->addWidget(progressBarWidget);

    theStackedWidget->setCurrentWidget(fileInputWidget);

    theStackedWidget->setWindowTitle("Select folder containing earthquake ground motions");

    return theStackedWidget;
}


void UserDefinedGroundMotionWidget::showUserGMSelectDialog(void)
{

    if (!theStackedWidget)
    {
        this->getUserDefinedGroundMotionWidget();
    }

    theStackedWidget->show();
    theStackedWidget->raise();
    theStackedWidget->activateWindow();
}


void UserDefinedGroundMotionWidget::chooseEventFileDialog(void)
{

    QFileDialog dialog(this);
    QString newEventFile = QFileDialog::getOpenFileName(this,tr("Event Grid File"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(newEventFile.isEmpty() || newEventFile == eventFile)
    {
        return;
    }

    QString err;
//    QVector<QStringList> data = csvTool.parseCSVFile(newEventFile, err);
    int skiprows = 10;
    QString delimiter = " ";
    QVector<QStringList> data = this->parseTXTFile(newEventFile, skiprows, delimiter, err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return;
    }

    if(data.empty())
        return;

    eventFile = newEventFile;
    summaryFileLineEdit->setText(eventFile);

    // load event table
    this->loadUserGMData();

    return;
}

QVector<QStringList> UserDefinedGroundMotionWidget::parseTXTFile(const QString &pathToFile, int skiprows, QString &delimiter, QString& err)
{
    QVector<QStringList> returnVec;

    QFile eventFile(pathToFile);

    if (!eventFile.open(QIODevice::ReadOnly))
    {
        err = "Cannot find the summary file: " + pathToFile + "\nCheck the path and try again.";
        return returnVec;
    }

    QStringList rowLines;
    while (!eventFile.atEnd())
    {
        QString line = eventFile.readLine();

        rowLines << line;
    }

    auto numRows = rowLines.size();
    if(numRows == 0)
    {
        err = "Error in parsing the .txt file " + pathToFile + " in UserDefinedGroundMotionWidget::parseTXTFile";
        return returnVec;
    }

    returnVec.reserve(numRows);

    int line_count = 0;
    for(auto&& it: rowLines)
    {
        if (line_count >= skiprows) {
            QStringList lineStr;
            // append default header
            //                auto lineStr = this->parseLineTXT(it);
            if (line_count == skiprows)
            {
                lineStr.append("ID");
                lineStr.append("Magnitude");
                lineStr.append("AnnualRate");
                lineStr.append("FileName");
                returnVec.push_back(lineStr);
                lineStr.clear();
            }
            it = it.trimmed();
            lineStr = it.split(delimiter);
            returnVec.push_back(lineStr);
        }
        line_count = line_count + 1;
    }

    return returnVec;
}


void UserDefinedGroundMotionWidget::chooseGMDirDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);
    QString newPath = dialog.getExistingDirectory(this, tr("Directory containing ground motion files"));
    dialog.close();

    // Return if the user cancels or enters same dir
    if(newPath.isEmpty() || newPath == gmDataDir)
    {
        return;
    }

    gmDataDir = newPath;
    GMDirLineEdit->setText(gmDataDir);

    return;
}


void UserDefinedGroundMotionWidget::clear(void)
{
    eventFile.clear();
    gmDataDir.clear();

    theComponentDb->clear();
    selectComponentsLineEdit->clear();

    siteListTableWidget->clear();
//    siteDataTableWidget->clear();

    siteListTableWidget->hide();
//    siteDataTableWidget->hide();
    tableHeader->hide();

    summaryFileLineEdit->clear();
    GMDirLineEdit->clear();
    stationMap.clear();
}


void UserDefinedGroundMotionWidget::loadUserGMData(void)
{
    auto qgisVizWidget = static_cast<QGISVisualizationWidget*>(theVisualizationWidget);

    if(qgisVizWidget == nullptr)
    {
        qDebug()<<"Failed to cast to ArcGISVisualizationWidget";
        return;
    }

    QString err;
//    QVector<QStringList> data = csvTool.parseCSVFile(newEventFile, err);
    int skiprows = 10;
    QString delimiter = " ";
    QVector<QStringList> data = this->parseTXTFile(eventFile, skiprows, delimiter, err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return;
    }

    if(data.size() < 2)
        return;

    this->showProgressBar();

    QApplication::processEvents();

    //progressBar->setRange(0,inputFiles.size());
    progressBar->setRange(0, data.count());
    progressBar->setValue(0);

    // Get the headings from the list before you pop it off
    auto listHeadings = data.first();

    // Pop off the row that contains the header information
    data.pop_front();

    // Populate the table view with the data
    siteListTableWidget->getTableModel()->populateData(data, listHeadings);
    siteListTableWidget->show();
    siteListTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);

    tableHeader->show();

    progressLabel->setVisible(false);

    // Reset the widget back to the input pane and close
    this->hideProgressBar();

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    return;
}


void UserDefinedGroundMotionWidget::showProgressBar(void)
{
    theStackedWidget->setCurrentWidget(progressBarWidget);
    fileInputWidget->setVisible(false);
    progressBarWidget->setVisible(true);
}


bool UserDefinedGroundMotionWidget::copyFiles(QString &destDir)
{
    // create dir and copy motion files
    QDir destDIR(destDir);
    if (!destDIR.exists()) {
        qDebug() << "UserDefinedGroundMotionWidget::copyFiles dest dir does not exist: " << destDir;
        return false;
    }

    QFileInfo eventFileInfo(eventFile);
    if (eventFileInfo.exists()) {
        this->copyFile(eventFile, destDir);
    } else {
        qDebug() << "UserDefinedGroundMotionWidget::copyFiles eventFile does not exist: " << eventFile;
        return false;
    }

    QDir motionDirInfo(gmDataDir);
    if (motionDirInfo.exists()) {
        return this->copyPath(gmDataDir, destDir, false);
    } else {
        qDebug() << "UserDefinedGroundMotionWidget::copyFiles motionDir does not exist: " << gmDataDir;
        return false;
    }

    // should never get here
    return false;
}


void UserDefinedGroundMotionWidget::hideProgressBar(void)
{
    theStackedWidget->setCurrentWidget(fileInputWidget);
    progressBarWidget->setVisible(false);
    fileInputWidget->setVisible(true);
}


void UserDefinedGroundMotionWidget::insertSelectedAssets(QgsFeatureIds& featureIds)
{
    QVector<int> assetIds;
    assetIds.reserve(featureIds.size());

    for(auto&& it : featureIds)
        assetIds.push_back(it-1);

    selectComponentsLineEdit->insertSelectedComponents(assetIds);

    this->selectComponents();
}


void UserDefinedGroundMotionWidget::selectComponents(void)
{
    try
    {
        selectComponentsLineEdit->selectComponents();
    }
    catch (const QString msg)
    {
        this->errorMessage(msg);
    }
}


void UserDefinedGroundMotionWidget::handleComponentSelection(void)
{

    auto nRows = siteListTableWidget->rowCount();

    if(nRows == 0)
        return;

    // Get the ID of the first and last component
    bool OK;
    auto firstID = siteListTableWidget->item(0,0).toInt(&OK);

    if(!OK)
    {
        QString msg = "Error in getting the component ID in " + QString(__FUNCTION__);
        this->errorMessage(msg);
        return;
    }

    auto lastID = siteListTableWidget->item(nRows-1,0).toInt(&OK);

    if(!OK)
    {
        QString msg = "Error in getting the component ID in " + QString(__FUNCTION__);
        this->errorMessage(msg);
        return;
    }

    auto selectedComponentIDs = selectComponentsLineEdit->getSelectedComponentIDs();

    // First check that all of the selected IDs are within range
    for(auto&& it : selectedComponentIDs)
    {
        if(it<firstID || it>lastID)
        {
            QString msg = "The component ID " + QString::number(it) + " is out of range of the components provided";
            this->errorMessage(msg);
            selectComponentsLineEdit->clear();
            return;
        }
    }

    auto numAssets = selectedComponentIDs.size();
    QString msg = "A total of "+ QString::number(numAssets) + " CPT sites are selected for analysis";
    this->statusMessage(msg);

    theComponentDb->startEditing();

    // Test to remove
    //    auto start = high_resolution_clock::now();

    theComponentDb->addFeaturesToSelectedLayer(selectedComponentIDs);

    // Test to remove
    //    auto stop = high_resolution_clock::now();
    //    auto duration = duration_cast<milliseconds>(stop - start);
    //    this->statusMessage("Done ALL "+QString::number(duration.count()));

    theComponentDb->commitChanges();

    // Hide all of the rows that are not selecetd. Takes a long time!
    //    // Hide all rows in the table
    //    for(int i = 0; i<nRows; ++i)
    //        componentTableWidget->setRowHidden(i,true);

    //    // Unhide the selected rows
    //    for(auto&& it : selectedComponentIDs)
    //        componentTableWidget->setRowHidden(it - firstID,false);


}


void UserDefinedGroundMotionWidget::clearComponentSelection(void)
{
    // auto nRows = componentTableWidget->rowCount();

    // Hide all rows in the table
    //    for(int i = 0; i<nRows; ++i)
    //    {
    //        componentTableWidget->setRowHidden(i,false);
    //    }

    selectComponentsLineEdit->clear();

    theComponentDb->clearSelectedLayer();

    theComponentDb->getSelectedLayer()->updateExtents();
}


void UserDefinedGroundMotionWidget::clearSelectedAssets(void)
{
    this->clearComponentSelection();
}
