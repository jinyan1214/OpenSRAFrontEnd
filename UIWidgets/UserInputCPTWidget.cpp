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


UserInputCPTWidget::UserInputCPTWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    eventFile = "";
    motionDir = "";

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(this->getUserInputCPTWidget());
    layout->addStretch();
    this->setLayout(layout);

    // Test Start
    eventFile = "/Users/steve/Desktop/SimCenter/Examples/CPTs/CPTSites.csv";
    motionDir = "/Users/steve/Desktop/SimCenter/Examples/CPTs/CPTs";
    CPTSitesFileLineEdit->setText(eventFile);
    CPTDirLineEdit->setText(motionDir);
    this->loadUserCPTData();
    // Test end
}


UserInputCPTWidget::~UserInputCPTWidget()
{

}


bool UserInputCPTWidget::outputAppDataToJSON(QJsonObject &jsonObject) {

    emit eventTypeChangedSignal("Earthquake");

    jsonObject["Application"] = "UserInputGM";

    QJsonObject appData;

    jsonObject["ApplicationData"]=appData;

    return true;
}


bool UserInputCPTWidget::outputToJSON(QJsonObject &jsonObj)
{
    QFileInfo theFile(eventFile);
    if (theFile.exists()) {
        jsonObj["eventFile"]=theFile.fileName();
        jsonObj["eventFilePath"]=theFile.path();
    } else {
        jsonObj["eventFile"]=eventFile; // may be valid on others computer
        jsonObj["eventFilePath"]=QString("");
    }

    // output motionDir if not same as eventFile's path
    QString eventFilePath = QFileInfo(eventFile).absolutePath();
    if (eventFilePath != motionDir) {

        QFileInfo theDir(motionDir);
        if (theDir.exists()) {
            jsonObj["motionDir"]=theDir.absoluteFilePath();
        } else {
            jsonObj["motionDir"]=QString("None");
        }
    }

    auto res = unitsWidget->outputToJSON(jsonObj);

    return res;
}


bool UserInputCPTWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    return true;
}


bool UserInputCPTWidget::inputFromJSON(QJsonObject &jsonObject)
{
    QString fileName;
    QString pathToFile;

    if (jsonObject.contains("eventFile"))
        fileName = jsonObject["eventFile"].toString();
    if (jsonObject.contains("eventFilePath"))
        pathToFile = jsonObject["eventFilePath"].toString();
    else
        pathToFile=QDir::currentPath();

    QString fullFilePath= pathToFile + QDir::separator() + fileName;

    // adam .. adam .. adam .. ADAM .. ADAM
    if (!QFileInfo::exists(fullFilePath)){

        fullFilePath = pathToFile + QDir::separator()
                + "input_data" + QDir::separator() + fileName;

        if (!QFile::exists(fullFilePath)) {
            this->errorMessage("UserInputGM - could not find event file");
            return false;
        }
    }

    CPTSitesFileLineEdit->setText(fullFilePath);
    eventFile = fullFilePath;

    if (jsonObject.contains("motionDir")) {
        motionDir = jsonObject["motionDir"].toString();

        QDir motionD(motionDir);

        if (!motionD.exists()){

            QString trialDir = QDir::currentPath() +
                    QDir::separator() + "input_data" + motionDir;
            if (motionD.exists(trialDir)) {
                motionDir = trialDir;
                CPTDirLineEdit->setText(trialDir);
            } else {
                this->errorMessage("UserInputGM - could not find motion dir" + motionDir + " " + trialDir);
                return false;
            }
        }
    } else {
        motionDir = QFileInfo(fullFilePath).absolutePath();
    }

    // set the line dit
    CPTDirLineEdit->setText(motionDir);


    // load the motions
    this->loadUserCPTData();

    // read in the units
    bool res = unitsWidget->inputFromJSON(jsonObject);

    // If setting of units failed, provide default units and issue a warning
    if(!res)
    {
        auto paramNames = unitsWidget->getParameterNames();

        this->infoMessage("Warning \\!/: Failed to find/import the units in 'User Specified Ground Motion' widget. Setting default units for the following parameters:");

        for(auto&& it : paramNames)
        {
            auto res = unitsWidget->setUnit(it,"g");

            if(res == 0)
                this->infoMessage("For parameter "+it+" setting default unit as: g");
            else
                this->errorMessage("Failed to set default units for parameter "+it);
        }

        this->infoMessage("Warning \\!/: Check if the units are correct!");
    }

    return res;
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
    fileInputWidget->setLayout(fileLayout);


    QLabel* selectComponentsText = new QLabel("File Containing Locations of CPT Test Sites");
    CPTSitesFileLineEdit = new QLineEdit();
    QPushButton *browseFileButton = new QPushButton("Browse");

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    fileLayout->addWidget(selectComponentsText, 0,0);
    fileLayout->addWidget(CPTSitesFileLineEdit,    0,1);
    fileLayout->addWidget(browseFileButton,     0,2);

    QLabel* selectFolderText = new QLabel("Folder Containing CPT Site Data");
    CPTDirLineEdit = new QLineEdit();
    QPushButton *browseFolderButton = new QPushButton("Browse");

    connect(browseFolderButton,SIGNAL(clicked()),this,SLOT(chooseMotionDirDialog()));
    
    unitsWidget = new SimCenterUnitsWidget("CPT Units");

    fileLayout->addWidget(selectFolderText,   1,0);
    fileLayout->addWidget(CPTDirLineEdit, 1,1);
    fileLayout->addWidget(browseFolderButton, 1,2);

    fileLayout->addWidget(unitsWidget,2,0,1,3);

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

    fileLayout->addWidget(tableSplitter,3,0,1,3);
    fileLayout->setRowStretch(4,1);

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
    CPTSitesFileLineEdit->setText(eventFile);

    // check if file in dir with all motions, if so set motionDir
    // Pop off the row that contains the header information
    data.pop_front();
    auto numRows = data.size();
    int count = 0;
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
        motionDir = fileDir.path();
        CPTDirLineEdit->setText(fileDir.path());
        this->loadUserCPTData();
    } else {
        QDir motionDirDir(motionDir);
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


void UserInputCPTWidget::chooseMotionDirDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);
    QString newPath = dialog.getExistingDirectory(this, tr("Dir containing specified motions"));
    dialog.close();

    // Return if the user cancels or enters same dir
    if(newPath.isEmpty() || newPath == motionDir)
    {
        return;
    }

    motionDir = newPath;
    // motionDirLineEdit->setText(motionDir);

    // check if dir contains EventGrid.csv file, if it does set the file
    QFileInfo eventFileInfo(newPath, "EventGrid.csv");
    if (eventFileInfo.exists()) {
        eventFile = newPath + "/EventGrid.csv";
        CPTSitesFileLineEdit->setText(eventFile);
    }

    // could check files exist if eventFile set, but need something to give an error if not all there
    this->loadUserCPTData();

    return;
}


void UserInputCPTWidget::clear(void)
{
    eventFile.clear();
    motionDir.clear();

    CPTSitesFileLineEdit->clear();
    CPTDirLineEdit->clear();
    stationMap.clear();
    unitsWidget->clear();
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
    auto stationFilePath = motionDir + QDir::separator() + stationName;

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
    attribFields.push_back(QgsField("AssetType", QVariant::String));
    attribFields.push_back(QgsField("Station Name", QVariant::String));
    attribFields.push_back(QgsField("Latitude", QVariant::Double));
    attribFields.push_back(QgsField("Longitude", QVariant::Double));

    for(auto&& it : stationDataHeadings)
    {
        attribFields.push_back(QgsField(it, QVariant::String));
        unitsWidget->addNewUnitItem(it);
    }

    // Get the headings from the list before you pop it off
    auto listHeadings = data.first();

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

    int count = 0;

    auto maxToDisp = 20;

    QgsFeatureList featureList;
    // Get the data
    for(int i = 0; i<numRows; ++i)
    {
        auto rowStr = data.at(i);

        auto stationName = rowStr[0];

        // Path to station files, e.g., site0.csv
        auto stationPath = motionDir + QDir::separator() + stationName;

        bool ok;
        auto longitude = rowStr[1].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error longitude to a double, check the value";
            this->errorMessage(errMsg);

            this->hideProgressBar();

            return;
        }

        auto latitude = rowStr[2].toDouble(&ok);

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

        featAttributes[0] = "CPTDataPoint";     // "AssetType"
        featAttributes[1] = stationName;                 // "Station Name"
        featAttributes[2] = latitude;                    // "Latitude"
        featAttributes[3] = longitude;                   // "Longitude"

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

            featAttributes[4+j] = str;
        }

        // Create the feature
        QgsFeature feature;
        feature.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude)));
        feature.setAttributes(featAttributes);
        featureList.append(feature);

        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        QApplication::processEvents();
    }

    auto vectorLayer = qgisVizWidget->addVectorLayer("Point", "CPT Data");

    if(vectorLayer == nullptr)
    {
        this->errorMessage("Error creating a layer");
        this->hideProgressBar();
        return;
    }

    auto dProvider = vectorLayer->dataProvider();
    auto res = dProvider->addAttributes(attribFields);

    if(!res)
    {
        this->errorMessage("Error adding attribute fields to layer");
        qgisVizWidget->removeLayer(vectorLayer);
        this->hideProgressBar();
        return;
    }

    vectorLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    dProvider->addFeatures(featureList);
    vectorLayer->updateExtents();

    qgisVizWidget->createSymbolRenderer(QgsSimpleMarkerSymbolLayerBase::Cross,Qt::black,2.0,vectorLayer);

    // Populate the table view with the data
    siteListTableWidget->getTableModel()->populateData(data, listHeadings);
    siteListTableWidget->show();
    siteListTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);

    progressLabel->setVisible(false);

    // Reset the widget back to the input pane and close
    this->hideProgressBar();

    if(theStackedWidget->isModal())
        theStackedWidget->close();

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

    QDir motionDirInfo(motionDir);
    if (motionDirInfo.exists()) {
        return this->copyPath(motionDir, destDir, false);
    } else {
        qDebug() << "UserInputCPTWidget::copyFiles motionDir does not exist: " << motionDir;
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
        siteDataTableWidget->show();

    auto row = index.row();

    auto key = siteListTableWidget->getTableModel()->item(row,0).toString();

    auto val = stationMap.value(key);

    if(val.isEmpty())
        return;

    auto headings = val.first();
    val.pop_front();
    siteDataTableWidget->getTableModel()->populateData(val,headings);
}
