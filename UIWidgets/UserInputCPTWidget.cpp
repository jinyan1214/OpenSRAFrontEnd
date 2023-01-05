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

// Written by: Stevan Gavrilovic

#include "CSVReaderWriter.h"
#include "LayerTreeView.h"
#include "UserInputCPTWidget.h"
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


UserInputCPTWidget::UserInputCPTWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    theComponentDb = ComponentDatabaseManager::getInstance()->createAssetDb("SiteCPTData");

    theComponentDb->setOffset(1);

    eventFile = "";
    cptDataDir = "";
    freefaceDir = "";

    QVBoxLayout *layout = new QVBoxLayout(this);
    QStackedWidget* CPTWidget = this->getUserInputCPTWidget();
    layout->addWidget(CPTWidget);
    layout->addStretch();

    // Test Start
//    eventFile = "/Users/steve/Desktop/SimCenter/Examples/CPTs/CPTSites.csv";
//    cptDataDir = "/Users/steve/Desktop/SimCenter/Examples/CPTs/CPTs";
//    CPTSummaryFileLineEdit->setText(eventFile);
//    CPTDirLineEdit->setText(cptDataDir);
//    this->loadUserCPTData();
    // Test end
}


UserInputCPTWidget::~UserInputCPTWidget()
{

}


bool UserInputCPTWidget::outputAppDataToJSON(QJsonObject &/*jsonObject*/) {

    return true;
}


bool UserInputCPTWidget::outputToJSON(QJsonObject &jsonObj)
{
    QJsonObject outputObj;

    QFileInfo pathToCPTSummaryCSV(CPTSummaryFileLineEdit->text());
    QFileInfo pathToCPTDataFolder(CPTDirLineEdit->text());
    QFileInfo pathToFreefaceDir(FreefaceDirLineEdit->text());

    outputObj.insert("PathToCPTSummaryCSV",pathToCPTSummaryCSV.absoluteFilePath());
    outputObj.insert("PathToCPTDataFolder",pathToCPTDataFolder.absoluteFilePath());
    outputObj.insert("ColumnInCPTSummaryWithGWTable",gwtMeanOption->currentText());
    outputObj.insert("WeightRobertson09",weightR09LineEdit->text().toDouble());
    outputObj.insert("WeightZhang04",weightZ04LineEdit->text().toDouble());
    outputObj.insert("PathToFreefaceDir",pathToFreefaceDir.absoluteFilePath());

    jsonObj.insert("CPTParameters",outputObj);

    return true;
}


bool UserInputCPTWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    Q_UNUSED(jsonObj);
    return true;
}


bool UserInputCPTWidget::inputFromJSON(QJsonObject &jsonObject)
{
    // set the line
    if (jsonObject.contains("PathToCPTSummaryCSV"))
    {
        auto sum_path = jsonObject["PathToCPTSummaryCSV"].toString();

        // if string is not empty, then continue, else don't load into CPT widget tab
        if (sum_path.length() > 0)
        {
            // Check for relative/absolute paths
            QFileInfo pathToCPTSummaryCSV(sum_path);
            if (!pathToCPTSummaryCSV.exists())
                sum_path=QDir::currentPath() + QDir::separator() + sum_path;
            pathToCPTSummaryCSV.setFile(sum_path);

            CPTSummaryFileLineEdit->setText(pathToCPTSummaryCSV.absoluteFilePath());
            eventFile = sum_path;

            // set the line
            if (jsonObject.contains("PathToCPTDataFolder"))
            {
                auto cpt_dir = jsonObject["PathToCPTDataFolder"].toString();

                // Check for relative/absolute paths
                QFileInfo pathToCPTDataFolder(cpt_dir);
                if (!pathToCPTDataFolder.exists())
                    cpt_dir=QDir::currentPath() + QDir::separator() + cpt_dir;
                pathToCPTDataFolder.setFile(cpt_dir);

                CPTDirLineEdit->setText(pathToCPTDataFolder.absoluteFilePath());
                cptDataDir = cpt_dir;
            }

            // set the line
            if (jsonObject.contains("WeightRobertson09"))
            {
                auto wr09 = jsonObject["WeightRobertson09"].toDouble();
                weightR09LineEdit->setText(QString::number(wr09));
            }

            // set the line
            if (jsonObject.contains("WeightZhang04"))
            {
                auto wz04 = jsonObject["WeightZhang04"].toDouble();
                weightZ04LineEdit->setText(QString::number(wz04));
            }

            // set the line
            if (jsonObject.contains("PathToFreefaceDir"))
            {
                auto ff_dir = jsonObject["PathToFreefaceDir"].toString();

                if (ff_dir.length() == 0)
                    FreefaceDirLineEdit->setText(ff_dir);
                else
                {
                    // Check for relative/absolute paths
                    QFileInfo pathToFreeFaceDir(ff_dir);
                    if (!pathToFreeFaceDir.exists())
                        ff_dir=QDir::currentPath() + QDir::separator() + ff_dir;
                    pathToFreeFaceDir.setFile(ff_dir);

                    FreefaceDirLineEdit->setText(pathToFreeFaceDir.absoluteFilePath());
                    freefaceDir = ff_dir;
                }
            }

            // load the motions
            this->loadUserCPTData();

            // set the QComboBox, valid after table has been loaded
            if (jsonObject.contains("ColumnInCPTSummaryWithGWTable"))
            {
                auto gwt_input = jsonObject["ColumnInCPTSummaryWithGWTable"].toString();
                gwtMeanOption->setCurrentText(gwt_input);
            }
        }
    }

    return true;
}


QStackedWidget* UserInputCPTWidget::getUserInputCPTWidget(void)
{
    if (theStackedWidget)
        return theStackedWidget;

    theStackedWidget = new QStackedWidget();

    //
    // file and dir input
    //
    fileInputWidget = new QWidget();

    QGridLayout *fileLayout = new QGridLayout(fileInputWidget);

    int count = 0;

    // File main message
    QLabel* mainText = new QLabel("This tab allows you to enter CPT-related data to be used in CPT-based methods under \"Liquefaction\", \"Lateral Spread\", and \"Settlement\" in the \"Engineering Demand Parameter\" tab. ");
    QFont fontMainText = mainText->font();
    fontMainText.setWeight(QFont::Bold);
    mainText->setFont(fontMainText);
//    fileLayout->addWidget(mainText, count,1, Qt::AlignLeft);
//    count ++;

    QHBoxLayout* headerTextBox = new QHBoxLayout();
    headerTextBox->addWidget(mainText,0,Qt::AlignLeft);
    headerTextBox->addStretch(1);
    fileLayout->addLayout(headerTextBox,count,0,1,3);
    count ++;

    // skip some space vertically
    QLabel* emptyText = new QLabel("");
    fileLayout->addWidget(emptyText, count,1, Qt::AlignCenter);
    count ++;


    // CPT Summary file
    QLabel* selectSummaryText = new QLabel("1. CPT Summary file in csv format - must contain the columns \"CPT_FileName\", \"Longitude\", and \"Latitude\":");
//    QFont fontSummaryText = selectSummaryText->font();
//    fontSummaryText.setWeight(QFont::Bold);
//    selectSummaryText->setFont(fontSummaryText);
    CPTSummaryFileLineEdit = new QLineEdit();
    QPushButton *browseSummaryButton = new QPushButton("Browse");

    connect(browseSummaryButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    // Grid layout is specified with row/col integers
    fileLayout->addWidget(selectSummaryText, count,0, Qt::AlignLeft);
    fileLayout->addWidget(CPTSummaryFileLineEdit, count,1);
    fileLayout->addWidget(browseSummaryButton, count,2);
    count ++;

    // CPT folder with data
    QLabel* selectFolderText = new QLabel("2. Folder containing CPT files in csv format - file names must match those in the \"CPT_FileName\" column above:");
//    QFont fontFolderText = selectFolderText->font();
//    fontFolderText.setWeight(QFont::Bold);
//    selectFolderText->setFont(fontFolderText);
    CPTDirLineEdit = new QLineEdit();
    QPushButton *browseFolderButton = new QPushButton("Browse");

    connect(browseFolderButton,SIGNAL(clicked()),this,SLOT(chooseCPTDirDialog()));

    fileLayout->addWidget(selectFolderText, count,0, Qt::AlignLeft);
    fileLayout->addWidget(CPTDirLineEdit, count,1);
    fileLayout->addWidget(browseFolderButton, count,2);
    count ++;

    // CPT data columns
    QGroupBox* unitsWidget = new QGroupBox("Note: each CPT datafile must contain the following columns");
    QGridLayout* vboxLayout = new QGridLayout(unitsWidget);

    QLabel* zLabel = new QLabel("Column 1 label: z");
    QFont fontZLabel = zLabel->font();
    fontZLabel.setWeight(QFont::Bold);
    zLabel->setFont(fontZLabel);
    QLabel* zDesc = new QLabel("Desc: depth (m)");
    //ClickableLabel* helpLabelz = new ClickableLabel("?");
    //helpLabelz->setToolTip("Description of z, units, etc.");

    QLabel* qtLabel = new QLabel("Column 2 label: qt");
    QFont fontQtLabel = qtLabel->font();
    fontQtLabel.setWeight(QFont::Bold);
    qtLabel->setFont(fontQtLabel);
    QLabel* qtDesc = new QLabel("Desc: tip resistance (MPa)");
//    ClickableLabel* helpLabelqt = new ClickableLabel("?");
//    helpLabelqt->setToolTip("Description of qt, units, etc.");

    QLabel* fsLabel = new QLabel("Column 3 label: fs");
    QFont fontFsLabel = fsLabel->font();
    fontFsLabel.setWeight(QFont::Bold);
    fsLabel->setFont(fontFsLabel);
    QLabel* fsDesc = new QLabel("Desc: sleeve friction (MPa)");
//    ClickableLabel* helpLabelfs = new ClickableLabel("?");
//    helpLabelfs->setToolTip("Description of fs, units, etc.");

    QLabel* u2Label = new QLabel("Column 4 label: u2");
    QFont fontU2Label = u2Label->font();
    fontU2Label.setWeight(QFont::Bold);
    u2Label->setFont(fontU2Label);
    QLabel* u2Desc = new QLabel("Desc: pore pressure (MPa)");
//    ClickableLabel* helpLabelu2 = new ClickableLabel("?");
//    helpLabelu2->setToolTip("Description of u2, units, etc.");

    vboxLayout->addWidget(zLabel,0,0,Qt::AlignCenter);
    vboxLayout->addWidget(zDesc,1,0,Qt::AlignCenter);
//    vboxLayout->addWidget(helpLabelz,0,1,Qt::AlignLeft);

    vboxLayout->addWidget(qtLabel,0,1,Qt::AlignCenter);
    vboxLayout->addWidget(qtDesc,1,1,Qt::AlignCenter);
//    vboxLayout->addWidget(helpLabelqt,0,3,Qt::AlignLeft);

    vboxLayout->addWidget(fsLabel,0,2,Qt::AlignCenter);
    vboxLayout->addWidget(fsDesc,1,2,Qt::AlignCenter);
//    vboxLayout->addWidget(helpLabelfs,1,1,Qt::AlignLeft);

    vboxLayout->addWidget(u2Label,0,3,Qt::AlignCenter);
    vboxLayout->addWidget(u2Desc,1,3,Qt::AlignCenter);
//    vboxLayout->addWidget(helpLabelu2,1,3,Qt::AlignLeft);

    fileLayout->addWidget(unitsWidget,count,0,1,3);
    count ++;

    // Component selection
    selectComponentsLineEdit = new AssetInputDelegate();
    connect(selectComponentsLineEdit,&AssetInputDelegate::componentSelectionComplete,this,&UserInputCPTWidget::handleComponentSelection);

    QPushButton *selectComponentsButton = new QPushButton();
    selectComponentsButton->setText(tr("Select"));
    selectComponentsButton->setMaximumWidth(150);

    connect(selectComponentsButton,SIGNAL(clicked()),this,SLOT(selectComponents()));

    QPushButton *clearSelectionButton = new QPushButton();
    clearSelectionButton->setText(tr("Clear Selection"));
    clearSelectionButton->setMaximumWidth(150);

    connect(clearSelectionButton,SIGNAL(clicked()),this,SLOT(clearComponentSelection()));

//    auto selectedLabel = new QLabel("(Currently unused) 3. Enter the IDs of one or more CPT sites to analyze.  Define a range of sites with a dash and separate multiple sites with a comma.");
//    QFont fontSelectedLabel = selectedLabel->font();
//    fontSelectedLabel.setWeight(QFont::Bold);
//    selectedLabel->setFont(fontSelectedLabel);

//    QHBoxLayout* selectComponentsLayout = new QHBoxLayout();
//    selectComponentsLayout->addWidget(selectedLabel);
//    selectComponentsLayout->addWidget(selectComponentsLineEdit);
//    selectComponentsLayout->addWidget(selectComponentsButton);
//    selectComponentsLayout->addWidget(clearSelectionButton);

//    fileLayout->addLayout(selectComponentsLayout,count,0,1,3);
    count ++;

    siteListTableWidget = new ComponentTableView();
    siteListTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    siteListTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    siteListTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    siteListTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    siteListTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    siteListTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    connect(siteListTableWidget,&QAbstractItemView::clicked,this,&UserInputCPTWidget::handleRowSelect);

    siteDataTableWidget = new ComponentTableView();
    siteDataTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    siteDataTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    siteDataTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    siteDataTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    siteDataTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    siteDataTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    QSplitter* tableSplitter = new QSplitter();
    tableSplitter->addWidget(siteListTableWidget);
    tableSplitter->addWidget(siteDataTableWidget);

    tableHeader = new QLabel("CPT Site List. Click on a row in the table to view the CPT values at that site.");
    tableHeader->hide();

    fileLayout->addWidget(tableHeader,count,0,1,3, Qt::AlignCenter);
    count ++;

    fileLayout->addWidget(tableSplitter,count,0,1,3);
    count ++;
    fileLayout->setRowStretch(count,1);
    count ++;

    // select column to use for mean groundwater table depth
    auto gwtMeanLabel = new QLabel("3. Column in summary file with mean groundwater depth in meters:");
//    QFont fontGwtMeanLabel = gwtMeanLabel->font();
//    fontGwtMeanLabel.setWeight(QFont::Bold);
//    gwtMeanLabel->setFont(fontGwtMeanLabel);
    gwtMeanOption = new QComboBox();
    gwtMeanOption->addItem("N/A");
    gwtMeanOption->setMaximumWidth(300);

    fileLayout->addWidget(gwtMeanLabel,count,0,1,1);
    fileLayout->addWidget(gwtMeanOption,count,1,1,2);
    count ++;

    // Weights for CPT liquefaction and lateral spread models
    auto methodLabel = new QLabel("4. Assign weights to use for \"liquefaction\" and \"lateral spread\" analysis of CPTs:");
//    QFont methodFont = methodLabel->font();
//    methodFont.setWeight(QFont::Bold);
//    methodLabel->setFont(methodFont);
    fileLayout->addWidget(methodLabel,count,0);
    count ++;

    // weight to use for WeightRobertson09 for liquefaction
    auto weightR09Label = new QLabel("Liquefaction: weight for Robertson (2009):");
    weightR09LineEdit = new QLineEdit();
    weightR09LineEdit->setText("0.5");
    weightR09LineEdit->setMaximumWidth(150);
    auto weightR09Desc = new QLabel(", weight for Boulanger & Idriss (2016) = 1 - weight for Robertson (2009)");

    QHBoxLayout* selectWeightR09 = new QHBoxLayout();
    selectWeightR09->addWidget(weightR09LineEdit);
    selectWeightR09->addWidget(weightR09Desc);

    fileLayout->addWidget(weightR09Label,count,0,1,1,Qt::AlignRight);
    fileLayout->addLayout(selectWeightR09,count,1,1,2);
    count ++;

    // weight to use for WeightRobertson09 for lateral spread
    auto weightZ04Label = new QLabel("Lateral spread: weight for Zhang et al. (2004):");
    weightZ04LineEdit = new QLineEdit();
    weightZ04LineEdit->setText("0.5");
    weightZ04LineEdit->setMaximumWidth(150);
    auto weightZ04Desc = new QLabel(", weight for Idriss & Boulanger (2008) = 1 - weight for Zhang et al. (2004)");

    QHBoxLayout* selectWeightZ04 = new QHBoxLayout();
    selectWeightZ04->addWidget(weightZ04LineEdit);
    selectWeightZ04->addWidget(weightZ04Desc);

    fileLayout->addWidget(weightZ04Label,count,0,1,1,Qt::AlignRight);
    fileLayout->addLayout(selectWeightZ04,count,1,1,2);
    count ++;

    // path to free face feature
    QLabel* selectFreefaceText = new QLabel("5. (Optional) Path to folder with shapefile for free-face feature (must contain the attribute \"Height_m\"):");
//    QFont fontFreefaceText = selectFreefaceText->font();
//    fontFreefaceText.setWeight(QFont::Bold);
//    selectFreefaceText->setFont(fontFreefaceText);
    FreefaceDirLineEdit = new QLineEdit();
    QPushButton *browseFreefaceButton = new QPushButton("Browse");

    connect(browseFreefaceButton,SIGNAL(clicked()),this,SLOT(chooseFreefaceDirDialog()));

//    fileLayout->addWidget(selectFolderHeader, count,0);
//    count ++;
    fileLayout->addWidget(selectFreefaceText, count,0, Qt::AlignLeft);
    fileLayout->addWidget(FreefaceDirLineEdit, count,1);
    fileLayout->addWidget(browseFreefaceButton, count,2);
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


void UserInputCPTWidget::showUserGMSelectDialog(void)
{

    if (!theStackedWidget)
    {
        this->getUserInputCPTWidget();
    }

    theStackedWidget->show();
    theStackedWidget->raise();
    theStackedWidget->activateWindow();
}


void UserInputCPTWidget::chooseEventFileDialog(void)
{

    QFileDialog dialog(this);
    QString newEventFile = QFileDialog::getOpenFileName(this,tr("Event Grid File"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(newEventFile.isEmpty() || newEventFile == eventFile)
    {
        return;
    }

    // Set file name & entry in qLine edit

    // if file
    //    check valid
    //    set motionDir if file in dir that contains all the motions
    //    invoke loadUserGMData

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(newEventFile, err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return;
    }

    if(data.empty())
        return;

    eventFile = newEventFile;
    CPTSummaryFileLineEdit->setText(eventFile);

    // check if file in dir with all CPTs
    // Pop off the row that contains the header information
    data.pop_front();
    auto numRows = data.size();

    QFileInfo eventFileInfo(eventFile);
    QDir fileDir(eventFileInfo.absolutePath());
    QStringList filesInDir = fileDir.entryList(QStringList() << "*", QDir::Files);

    // check all files are there
    bool allThere = true;
    for(int i = 0; i<numRows; ++i) {
        auto rowStr = data.at(i);
        auto stationName = rowStr[0];
        if (!filesInDir.contains(stationName)) {
            allThere = false;
            i=numRows;
        }
    }

    if (allThere == true) {
        cptDataDir = fileDir.path();
        CPTDirLineEdit->setText(fileDir.path());
        this->loadUserCPTData();
    } else {
        QDir motionDirDir(cptDataDir);
        if (motionDirDir.exists()) {
            QStringList filesInDir = motionDirDir.entryList(QStringList() << "*", QDir::Files);
            bool allThere = true;
            for(int i = 0; i<numRows; ++i) {
                auto rowStr = data.at(i);
                auto stationName = rowStr[0];
                if (!filesInDir.contains(stationName)) {
                    allThere = false;
                    i=numRows;
                }
            }
            if (allThere == true)
                this->loadUserCPTData();
        }
    }

    return;
}


void UserInputCPTWidget::chooseCPTDirDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);
    QString newPath = dialog.getExistingDirectory(this, tr("Dir containing CPT files"));
    dialog.close();

    // Return if the user cancels or enters same dir
    if(newPath.isEmpty() || newPath == cptDataDir)
    {
        return;
    }

    cptDataDir = newPath;
    CPTDirLineEdit->setText(cptDataDir);

//    // check if dir contains EventGrid.csv file, if it does set the file
//    QFileInfo eventFileInfo(newPath, "EventGrid.csv");
//    if (eventFileInfo.exists()) {
//        eventFile = newPath + "/EventGrid.csv";
//        CPTSummaryFileLineEdit->setText(eventFile);
//    }

    // could check files exist if eventFile set, but need something to give an error if not all there
    this->loadUserCPTData();

    return;
}


void UserInputCPTWidget::chooseFreefaceDirDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);
    QString newPath = dialog.getExistingDirectory(this, tr("Path to directory with freeface feature shapefile"));
    dialog.close();

    // Return if the user cancels or enters same dir
    if(newPath.isEmpty() || newPath == cptDataDir)
    {
        return;
    }

    cptDataDir = newPath;
    FreefaceDirLineEdit->setText(cptDataDir);

    return;
}


void UserInputCPTWidget::clear(void)
{
    eventFile.clear();
    cptDataDir.clear();
    freefaceDir.clear();

    theComponentDb->clear();
    selectComponentsLineEdit->clear();

    siteListTableWidget->clear();
    siteDataTableWidget->clear();

    siteListTableWidget->hide();
    siteDataTableWidget->hide();
    tableHeader->hide();

    CPTSummaryFileLineEdit->clear();
    CPTDirLineEdit->clear();
    stationMap.clear();

    gwtMeanOption->clear();
    gwtMeanOption->addItem("N/A");

    weightR09LineEdit->clear();
    weightR09LineEdit->setText("0.5");
    weightZ04LineEdit->clear();
    weightZ04LineEdit->setText("0.5");

    FreefaceDirLineEdit->clear();

}


void UserInputCPTWidget::loadUserCPTData(void)
{
    auto qgisVizWidget = static_cast<QGISVisualizationWidget*>(theVisualizationWidget);

    if(qgisVizWidget == nullptr)
    {
        qDebug()<<"Failed to cast to ArcGISVisualizationWidget";
        return;
    }

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(eventFile, err);

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

    // Get the headers in the first station file - assume that the rest will be the same
    auto rowStr = data.at(1);
    auto stationName = rowStr[0];

    // Path to station files, e.g., site0.csv
    auto stationFilePath = cptDataDir + QDir::separator() + stationName;

    QString err2;
    QVector<QStringList> sampleStationData = csvTool.parseCSVFile(stationFilePath,err);

    // Return if there is an error or the station data is empty
    if(!err2.isEmpty())
    {
        this->errorMessage("Could not parse the first station with the following error: "+err2);
        return;
    }

    if(sampleStationData.size() < 2)
    {
        this->errorMessage("The file " + stationFilePath + " is empty");
        return;
    }

    // Get the header file
    auto stationDataHeadings = sampleStationData.first();


    // Create the fields
    QList<QgsField> attribFields;
    attribFields.push_back(QgsField("ID", QVariant::Int));
    attribFields.push_back(QgsField("AssetType", QVariant::String));
    attribFields.push_back(QgsField("Station Name", QVariant::String));
    attribFields.push_back(QgsField("Latitude", QVariant::Double));
    attribFields.push_back(QgsField("Longitude", QVariant::Double));

    for(auto&& it : stationDataHeadings)
    {
        attribFields.push_back(QgsField(it, QVariant::String));
    }

    // Get the headings from the list before you pop it off
    auto listHeadings = data.first();

    // Add headers as options
//    gwtMeanOption->addItems(stationDataHeadings);
    gwtMeanOption->addItems(listHeadings);

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

    int count = 0;

    auto maxToDisp = 20;
    auto indexCPTName = listHeadings.indexOf("CPT_FileName");
    auto indexLon = listHeadings.indexOf("Longitude");
    auto indexLat = listHeadings.indexOf("Latitude");

    if (indexCPTName == -1) {
        this->errorMessage("Error: cannot find column with CPT file name in CPT summary file.");
        return;
    }

    if (indexLon == -1 || indexLat == -1) {
        this->errorMessage("Error: cannot find columns with longitude and/or latitutde in CPT summary file.");
        return;
    }

    QgsFeatureList featureList;
    // Get the data
    for(int i = 0; i<numRows; ++i)
    {
        QStringList& rowStr = data[i];

        auto stationName = rowStr[indexCPTName];

        // Path to station files, e.g., site0.csv
        auto stationPath = cptDataDir + QDir::separator() + stationName;

        bool ok;
        auto longitude = rowStr[indexLon].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error longitude to a double, check the value";
            this->errorMessage(errMsg);

            this->hideProgressBar();

            return;
        }

        auto latitude = rowStr[indexLat].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error latitude to a double, check the value";
            this->errorMessage(errMsg);

            this->hideProgressBar();

            return;
        }

        QString parseErr;
        QVector<QStringList> stationData = csvTool.parseCSVFile(stationPath,parseErr);
        if(!parseErr.isEmpty())
        {
            this->errorMessage("Could not parse the CPT site "+QString::number(i) + " with the following error: "+parseErr);
            return;
        }

        if(stationData.size() < 2)
        {
            this->errorMessage("The file " + stationFilePath + " is empty");
            return;
        }

        stationMap.insert(stationName,stationData);

        // create the feature attributes
        QgsAttributes featAttributes(attribFields.size());

        featAttributes[0] = QString::number(i);     // "AssetType"
        featAttributes[1] = "CPTDataPoint";     // "AssetType"
        featAttributes[2] = stationName;                 // "Station Name"
        featAttributes[3] = latitude;                    // "Latitude"
        featAttributes[4] = longitude;                   // "Longitude"

        // The number of headings in the file
        auto numParams = stationData.front().size();

        maxToDisp = (maxToDisp<stationData.size() ? maxToDisp : stationData.size());

        QVector<QString> dataStrs(numParams);

        for(int i = 0; i<maxToDisp-1; ++i)
        {
            auto stationParams = stationData[i];

            for(int j = 0; j<numParams; ++j)
            {
                dataStrs[j] += stationParams[j] + ", ";
            }
        }

        for(int j = 0; j<numParams; ++j)
        {
            auto str = dataStrs[j] ;
            str += stationData[maxToDisp-1][j];

            if(maxToDisp<stationData.size())
                str += "...";

            featAttributes[5+j] = str;
        }

        // Create the feature
        QgsFeature feature;
        feature.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude)));
        feature.setAttributes(featAttributes);
        featureList.append(feature);

        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        rowStr.prepend(QString::number(i));

        QApplication::processEvents();
    }

    mainLayer = qgisVizWidget->addVectorLayer("Point", "CPT Data");

    if(mainLayer == nullptr)
    {
        this->errorMessage("Error creating a layer");
        this->hideProgressBar();
        return;
    }

    auto dProvider = mainLayer->dataProvider();
    auto res = dProvider->addAttributes(attribFields);

    if(!res)
    {
        this->errorMessage("Error adding attribute fields to layer");
        qgisVizWidget->removeLayer(mainLayer);
        this->hideProgressBar();
        return;
    }

    mainLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    theComponentDb->setMainLayer(mainLayer);

    dProvider->addFeatures(featureList);
    mainLayer->updateExtents();

    qgisVizWidget->createSymbolRenderer(Qgis::MarkerShape::Circle,Qt::red,3.0,mainLayer);

    listHeadings.prepend("ID");

    // Populate the table view with the data
    siteListTableWidget->getTableModel()->populateData(data, listHeadings);
    siteListTableWidget->show();
    siteListTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);

    progressLabel->setVisible(false);

    // Reset the widget back to the input pane and close
    this->hideProgressBar();

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    auto layerId = mainLayer->id();

    theVisualizationWidget->registerLayerForSelection(layerId,this);

    // Create the selected building layer
    selectedFeaturesLayer = theVisualizationWidget->addVectorLayer("Point","Selected CPT Data");

    if(selectedFeaturesLayer == nullptr)
    {
        this->errorMessage("Error adding the selected assets vector layer");
        return;
    }

    qgisVizWidget->createSymbolRenderer(Qgis::MarkerShape::Circle,Qt::darkBlue,3.0,selectedFeaturesLayer);

    auto pr2 = selectedFeaturesLayer->dataProvider();

    auto res2 = pr2->addAttributes(attribFields);

    if(!res2)
        this->errorMessage("Error adding attributes to the layer");

    selectedFeaturesLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    theComponentDb->setSelectedLayer(selectedFeaturesLayer);

    QVector<QgsMapLayer*> mapLayers;
    mapLayers.push_back(selectedFeaturesLayer);
    mapLayers.push_back(mainLayer);

    theVisualizationWidget->createLayerGroup(mapLayers,"CPT Data");

    return;
}


void UserInputCPTWidget::showProgressBar(void)
{
    theStackedWidget->setCurrentWidget(progressBarWidget);
    fileInputWidget->setVisible(false);
    progressBarWidget->setVisible(true);
}


bool UserInputCPTWidget::copyFiles(QString &destDir)
{
    // create dir and copy motion files
    QDir destDIR(destDir);
    if (!destDIR.exists()) {
        qDebug() << "UserInputCPTWidget::copyFiles dest dir does not exist: " << destDir;
        return false;
    }

    QFileInfo eventFileInfo(eventFile);
    if (eventFileInfo.exists()) {
        this->copyFile(eventFile, destDir);
    } else {
        qDebug() << "UserInputCPTWidget::copyFiles eventFile does not exist: " << eventFile;
        return false;
    }

    QDir motionDirInfo(cptDataDir);
    if (motionDirInfo.exists()) {
        return this->copyPath(cptDataDir, destDir, false);
    } else {
        qDebug() << "UserInputCPTWidget::copyFiles motionDir does not exist: " << cptDataDir;
        return false;
    }

    // should never get here
    return false;
}


void UserInputCPTWidget::hideProgressBar(void)
{
    theStackedWidget->setCurrentWidget(fileInputWidget);
    progressBarWidget->setVisible(false);
    fileInputWidget->setVisible(true);
}


void UserInputCPTWidget::handleRowSelect(const QModelIndex &index)
{
    if(siteDataTableWidget->isHidden())
    {
        tableHeader->show();
        siteDataTableWidget->show();
    }

    auto row = index.row();

    auto key = siteListTableWidget->getTableModel()->item(row,1).toString();

    auto val = stationMap.value(key);

    if(val.isEmpty())
        return;

    auto headings = val.first();
    val.pop_front();
    siteDataTableWidget->getTableModel()->populateData(val,headings);
}


void UserInputCPTWidget::insertSelectedAssets(QgsFeatureIds& featureIds)
{
    QVector<int> assetIds;
    assetIds.reserve(featureIds.size());

    for(auto&& it : featureIds)
        assetIds.push_back(it-1);

    selectComponentsLineEdit->insertSelectedComponents(assetIds);

    this->selectComponents();
}


void UserInputCPTWidget::selectComponents(void)
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


void UserInputCPTWidget::handleComponentSelection(void)
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


void UserInputCPTWidget::clearComponentSelection(void)
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


void UserInputCPTWidget::clearSelectedAssets(void)
{
    this->clearComponentSelection();
}
