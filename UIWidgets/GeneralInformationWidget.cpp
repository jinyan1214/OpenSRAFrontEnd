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

// Written by: Dr. Stevan Gavrilovic, UC Berkeley

#include "UIWidgets/GeneralInformationWidget.h"
#include "OpenSRAPreferences.h"
#include "sectiontitle.h"
#include "ClickableLabel.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QJsonArray>
#include <QPushButton>
#include <QRadioButton>
#include <QJsonObject>
#include <QComboBox>
#include <QDebug>
#include <QList>
#include <QLabel>
#include <QLineEdit>
#include <QMetaEnum>
#include <QStandardPaths>
#include <QApplication>
#include <QScrollArea>

GeneralInformationWidget::GeneralInformationWidget(QWidget *parent) : SimCenterAppWidget(parent)
{

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(5,0,0,0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("General Information"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,0);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);

    auto infoLayout = this->getInfoLayout();
    infoLayout->setSpacing(3);
    infoLayout->setContentsMargins(5,0,0,0);

    auto infoWidget = new QWidget();
    infoWidget->setLayout(infoLayout);

    QScrollArea *scrollWidget = new QScrollArea();
    scrollWidget->setWidgetResizable(true);
    scrollWidget->setLineWidth(0);
    scrollWidget->setWidget(infoWidget);
    scrollWidget->setFrameShape(QFrame::NoFrame);
    scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addWidget(scrollWidget);
//    mainLayout->addStretch();

    this->setLayout(mainLayout);

}


GeneralInformationWidget::~GeneralInformationWidget()
{

}


bool GeneralInformationWidget::outputToJSON(QJsonObject &jsonObj)
{

    QJsonObject outputObj;
    QJsonObject directoryObj;

    QDir workDir(workingDirectoryLineEdit->text());
    directoryObj.insert("Working",workDir.absolutePath());
    directoryObj.insert("OpenSRAData",OpenSRAPreferences::getInstance()->getAppDataDir());
    directoryObj.insert("NDAData",OpenSRAPreferences::getInstance()->getNDADataDir());

    outputObj.insert("AnalysisID",analysisLineEdit->text());
//    outputObj.insert("UnitSystem",unitsCombo->currentText());
    outputObj.insert("Directory", directoryObj);
//    outputObj.insert("OutputFileType", "csv");

    jsonObj.insert("General",outputObj);

    return true;
}


bool GeneralInformationWidget::inputFromJSON(QJsonObject &jsonObject)
{
    auto dirObj = jsonObject["Directory"].toObject();

    auto workingDir = dirObj["Working"].toString();

    if(!workingDir.isEmpty())
    {
        QDir workDir(workingDir);

        if(workDir.exists())
        {
            workingDirectoryLineEdit->setText(workDir.absolutePath());

            // Set the file path to the preferences
            OpenSRAPreferences::getInstance()->setLocalWorkDir(workDir.absolutePath());
        }
    }

    auto analysisID = jsonObject["AnalysisID"].toString();
    analysisLineEdit->setText(analysisID);

//    if(dirObj.contains("OpenSRAData"))
//    {
//        auto appDataDir = dirObj["OpenSRAData"].toString();
//        if(!appDataDir.isEmpty())
//        {
//            QDir dataDir(appDataDir);

//            if(dataDir.exists())
//            {
//                // Set the file path to the preferences
//                OpenSRAPreferences::getInstance()->setAppDataDir(dataDir.absolutePath());
//            }
//        }

//    }

//    auto unitsSystem = jsonObject["UnitSystem"].toString();
//    unitsCombo->setCurrentText(unitsSystem);

    return true;
}


void GeneralInformationWidget::clear(void)
{
    analysisLineEdit->clear();
    auto loclWorkDir = OpenSRAPreferences::getInstance()->getLocalWorkDir();
    workingDirectoryLineEdit->setText(loclWorkDir);
}


QVBoxLayout* GeneralInformationWidget::getInfoLayout(void)
{
    // Get a rectangular message box on this window that could be help text box


    auto analysisLabel = new QLabel("Analysis ID:");
    analysisLineEdit = new QLineEdit();
    analysisLineEdit->setText("Analysis_ID");
    analysisLineEdit->setMaximumWidth(500);

    auto unitSystemLabel = new QLabel("Unit System:");
    unitsCombo = new QComboBox();
    unitsCombo->addItem("SI (km)");
    unitsCombo->setCurrentIndex(0);
    unitsCombo->setMaximumWidth(500);
    unitsCombo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    unitsCombo->setToolTip("Set the units here");

    QPushButton *loadFileButton = new QPushButton();
    loadFileButton->setText(tr("Browse"));
    loadFileButton->setMaximumWidth(150);
    auto workingDirectoryLabel = new QLabel("Working Directory:");
    workingDirectoryLineEdit = new QLineEdit();
    workingDirectoryLineEdit->setReadOnly(true);
    workingDirectoryLineEdit->setMaximumWidth(400);

    // Set the local work dir as default
    auto loclWorkDir = OpenSRAPreferences::getInstance()->getLocalWorkDir();
    workingDirectoryLineEdit->setText(loclWorkDir);

    connect(loadFileButton,&QPushButton::clicked, this, &GeneralInformationWidget::chooseDirectoryDialog);

    // overview
    auto assessmentLabel = new QLabel("General Comments:");
    assessmentLabel->setStyleSheet("font-weight: bold; color: black");

    const char *assessmentDetailString =
        "1. Refer back to this tab at any time for setup instructions.\n"

        "2. Note that the first time you click \"PREPROCESS\" and \"PERFORM ANALYSIS\" after installation will take some time for OpenSRA to perform some behind-the-scene tasks.\n"

        "3. If you are not familiar with OpenSRA, it is best to pick the example that best match your problem and adjust the analysis parameters from there (see \"Examples\" on the menu bar at the top).";
    auto assessmentDetailLabel = new QLabel(assessmentDetailString);

    // examples
    auto exampleLabel = new QLabel("Instructions for Examples:");
    exampleLabel->setStyleSheet("font-weight: bold; color: black");

    const char *exampleDetailString =
        "1. The built-in examples are separated by the currently available infrastructure types: (1) above ground components, (2) below ground pipeline, and (3) wells and caprocks.\n"
        "\t- For below ground pipelines, the examples are further divided by the type of geohazard: (1) lateral spread, (2) liquefaction-induced settlement, (3) landslide, and (4) surface fault rupture.\n"

        "2. To run an example:\n"
        "\t- Select an example from the dropdown menu at the top of the screen\n"
        "\t- Follow the pop-up prompts";
    auto exampleDetailLabel = new QLabel(exampleDetailString);

    // instructions
    auto setupLabel = new QLabel("Program Overview and Setup Details:");
    setupLabel->setStyleSheet("font-weight: bold; color: black");

    const char *setupDetailString =
        "1. \"General Information\":\n"
        "\t- Define analysis ID (optional) and select the path for the working directory\n"

        "2. \"Infrastructure\":\n"
        "\t- Click on the button with the type of infrastructure to run\n"
        "\t- From the dropdown menu, choose the file type to import\n"
        "\t- Fill out the inputs as requested\n"
        "\tNote: for \"Pipelines\", users can choose to use the prepackaged state pipeline network, which has been heavily preprocessed internally to reduce runtime; however, state-wide analysis can still take a long time to complete (over 1 hour)\n"

        "3. \"Decision Variable\":\n"
        "\t- Pick the decision metric to run from the secondary list of buttons\n"
        "\t- From the dropdown menu, pick the method to run\n"
        "\t- Review the descriptions of the outputs, inputs, and additional requirements from tabs on the left side-bar\n"
        "\t- Click \"Add to list of methods to run\" to the include this method in the run\n"
        "\tNote: The input parameters will be displayed under the \"Input Variables\" tab for users to edit\n"
        "\tWarning: While you can use more than one method, we advise to use one method per run as combinations of methods may not be appropriate for the current implemention of Polynomial Chaos\n"

        "4. \"Damage Measure\":\n"
        "\t- Only required if requested by one of the methods under the \"Decision Variable\" tab\n"
        "\t- Follow the same instructions as those under \"Decision Variable\"\n"

        "5. \"Engineering Demand Parameters\":\n"
        "\t- Only required if requested by one of the methods under the \"Decision Variable\" and/or the \"Damage Measure\" tab\n"
        "\t- Follow the same instructions as those under \"Decision Variable\"\n"

        "6. \"Intensity Measure\":\n"
        "\t- From the dropdown menu, choose the type of seismic source to use\n"
        "\t- Follow the on-screen instructions to fill out the inputs as requested\n"

        "7. \"GIS and CPT Data\":\n"
        "\t- \"User Provided GIS Data\"\n"\
        "\t\t- Browse for folder with datasets - the menu on the right should populate with the GIS maps found in the folder\n"
        "\t- \"Site Investigation Data (currently supporting CPTs only)\"\n"\
        "\t\t- Follow the on-screen instructions to fill out the inputs as requested\n"
        "\t\tNote: only used by below ground pipelines with lateral spread and settlement as geohazards\n"

        "8. \"Input Variables\":\n"
        "\t- This tab contains two tables:\n"
        "\t\t- (a) random variables that can sampled for epistemic uncertainty\n"
        "\t\t- (a) fixed variables that holds a singular value\n"
        "\t- Follow the on-screen instructions to edit the tables\n"

        "\nClick \"PREPROCESS\" once Steps 1 through 7 have been reviewed\n"

        "When prompted, click \"PERFORM ANALYSIS\" to perform the analysis\n\n"
        "\t- If the analysis is successful, a new layer named \"Results\" will be populated in the \"Visualization\" tab\n";

    auto setupDetailLabel = new QLabel(setupDetailString);


//    QRadioButton *button1 = new QRadioButton("Pre-configured setup for risk assessment (preferred)");
//    QRadioButton *button2 = new QRadioButton("Allow for user customization (e.g., source models, evaluation methods to use)");
//    button1->setChecked(true);

//    auto warningLabel = new QLabel();
//    warningLabel->setText("Warning: Only choose this option if you have read the user manual and are familiar with the program");
//    warningLabel->setStyleSheet("color: red");

    // Layout the UI components in a grid
    QVBoxLayout* layout = new QVBoxLayout();

    ClickableLabel* helpLabel1 = new ClickableLabel("   ?");
    helpLabel1->setToolTip("Enter a unique analysis name.");

    QHBoxLayout* analysisNameLayout = new QHBoxLayout();
    analysisNameLayout->addWidget(analysisLabel);
    analysisNameLayout->addWidget(analysisLineEdit);
    analysisNameLayout->addWidget(helpLabel1);
    analysisNameLayout->addStretch();

    ClickableLabel* helpLabel2 = new ClickableLabel("   ?");
    helpLabel2->setToolTip("Enter or select the path to the working directory,  i.e.,  where OpenSRA will store the analysis inputs and results.");

    QHBoxLayout* workingDirLayout = new QHBoxLayout();
    workingDirLayout->addWidget(workingDirectoryLabel);
    workingDirLayout->addWidget(workingDirectoryLineEdit);
    workingDirLayout->addWidget(loadFileButton);
    workingDirLayout->addWidget(helpLabel2);
    workingDirLayout->addStretch();

    ClickableLabel* helpLabel3 = new ClickableLabel("  ?");
    helpLabel3->setToolTip("Select the analysis unit system.");

    QHBoxLayout* unitsLayout = new QHBoxLayout();
    unitsLayout->addWidget(unitSystemLabel);
    unitsLayout->addWidget(unitsCombo);
    unitsLayout->addWidget(helpLabel3);
    unitsLayout->addStretch();

    layout->addLayout(analysisNameLayout);
    layout->addLayout(workingDirLayout);
    layout->addLayout(unitsLayout);

    // add spacer
    QSpacerItem *vSpacer = new QSpacerItem(0,20);
    layout->addItem(vSpacer);

    // general comment text
    layout->addWidget(assessmentLabel);
    layout->addWidget(assessmentDetailLabel);

    // example text
    QSpacerItem *vSpacer2 = new QSpacerItem(0,20);
    layout->addItem(vSpacer2);
    layout->addWidget(exampleLabel);
    layout->addWidget(exampleDetailLabel);

    // setup text
    QSpacerItem *vSpacer3 = new QSpacerItem(0,20);
    layout->addItem(vSpacer3);
    layout->addWidget(setupLabel);
    layout->addWidget(setupDetailLabel);

    layout->addStretch(1);

//    layout->addWidget(button1);
//    layout->addWidget(button2);

//    layout->addWidget(warningLabel);

    return layout;
}


void GeneralInformationWidget::chooseDirectoryDialog(void)
{
    auto prefs = OpenSRAPreferences::getInstance();

    auto pathToWorkingDirectoryFile = QFileDialog::getExistingDirectory(this,tr("Working Directory"), prefs->getLocalWorkDir());

    if(pathToWorkingDirectoryFile.isEmpty())
        return;

    // Return if the user cancels
    if(!QDir(pathToWorkingDirectoryFile).exists())
    {
        QString errMsg = "The given path " + pathToWorkingDirectoryFile + " is not a valid directory";
        errorMessage(errMsg);

        return;
    }

    // Set file name & entry in qLine edit
    workingDirectoryLineEdit->setText(pathToWorkingDirectoryFile);

    // Set the file path to the preferences
    prefs->setLocalWorkDir(pathToWorkingDirectoryFile);

    return;
}



