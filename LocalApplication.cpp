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


// Written: fmckenna
// Modified by: Stevan Gavrilovic

// Purpose: a widget for managing submiited jobs by uqFEM tool
//  - allow for refresh of status, deletion of submitted jobs, and download of results from finished job

#include "LocalApplication.h"
#include "OpenSRAPreferences.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonObject>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QStringList>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QProcessEnvironment>
#include <QCoreApplication>

LocalApplication::LocalApplication(QString workflowScriptName, QWidget *parent)
{

    proc = new QProcess(this);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &LocalApplication::handleProcessFinished);
    connect(proc, &QProcess::readyReadStandardOutput, this, &LocalApplication::handleProcessTextOutput);
    connect(proc, &QProcess::started, this, &LocalApplication::handleProcessStarted);
    
    this->workflowScript = workflowScriptName;
}

bool
LocalApplication::outputToJSON(QJsonObject &jsonObject)
{
    //    jsonObject["localAppDir"]=appDirName->text();
    //    jsonObject["remoteAppDir"]=appDirName->text();
    //    jsonObject["workingDir"]=workingDirName->text();
    jsonObject["localAppDir"]=OpenSRAPreferences::getInstance()->getAppDir();
    jsonObject["remoteAppDir"]=OpenSRAPreferences::getInstance()->getAppDir();
    jsonObject["workingDir"]=OpenSRAPreferences::getInstance()->getLocalWorkDir();

    jsonObject["runType"]=QString("runningLocal");

    return true;
}

bool
LocalApplication::inputFromJSON(QJsonObject &dataObject) {

    Q_UNUSED(dataObject);
    return true;
}




void
LocalApplication::onRunButtonPressed(void)
{
    statusMessage("Setting up temporary directory");

    QString workingDir = OpenSRAPreferences::getInstance()->getLocalWorkDir();
    QDir dirWork(workingDir);
    if (!dirWork.exists())
        if (!dirWork.mkpath(workingDir)) {
            QString errMsg = QString("Could not create Working Dir: ") + workingDir + QString(". Change the Local Jobs Directory location in preferences.");
            errorMessage(errMsg);

            return;
        }
    

    //   QString appDir = appDirName->text();
    QString appDir = OpenSRAPreferences::getInstance()->getAppDir();
    QDir dirApp(appDir);
    if (!dirApp.exists()) {
        QString errMsg = QString("The application directory, ") + appDir +QString(" specified does not exist!. Check Local Application Directory im Preferences");
        errorMessage(errMsg);;
        return;
    }

    statusMessage("Gathering Files to local workdir");
    emit setupForRun(workingDir, workingDir);
}


//
// now use the applications Workflow Application EE-UQ.py  to run dakota and produce output files:
//    dakota.in dakota.out dakotaTab.out dakota.err
//

bool
LocalApplication::setupDoneRunApplication(QString &/*tmpDirectory*/, QString &inputFile)
{

    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss");
    QString msg1 = "Running analysis on OpenSRA version " + QCoreApplication::applicationVersion() + " at " + currentTime;
    statusMessage(msg1);

    QString appDir = OpenSRAPreferences::getInstance()->getAppDir();

    QString pySCRIPT;

    QDir scriptDir(appDir);
    scriptDir.cd("OpenSRABackend");
    pySCRIPT = scriptDir.absoluteFilePath(workflowScript);

    QFileInfo check_script(pySCRIPT);
    // check if file exists and if yes: Is it really a file and no directory?
    if (!check_script.exists() || !check_script.isFile())
    {
        QString errMsg = "Cannot find the Python script: " + pySCRIPT;
        errorMessage(errMsg);

        return false;
    }


    auto procEnv = QProcessEnvironment();
    procEnv.clear();

    statusMessage("Looking for Java on the system");

    // First check for JAVA_HOME
#ifdef Q_OS_WIN
    auto sysEnv = QProcessEnvironment::systemEnvironment();
    QString javaHomeVal = sysEnv.value("JAVA_HOME");

    statusMessage("JAVA_HOME: " + javaHomeVal);

    qDebug() << "JAVA_HOME: "<<javaHomeVal;

    if(javaHomeVal.isEmpty())
    {
        this->errorMessage("NO JAVA_HOME VARIABLE FOUND");
        qDebug() << "NO JAVA_HOME VARIABLE FOUND";
        return false;
    }

    procEnv.insert("JAVA_HOME", javaHomeVal);
#endif

    QString javaOutput;

    {
        QProcess javaProcess;
        QString commandToStart= "java -version";
        javaProcess.start(commandToStart);
        bool started = javaProcess.waitForStarted();
        if (!javaProcess.waitForFinished(30000)) // 3 Second timeout
            javaProcess.kill();

        if(!started)
        {
            statusMessage("Could not start Java on the system");
            return false; // Not found or which does not work
        }

        javaOutput = QString::fromLocal8Bit(javaProcess.readAllStandardError());
        javaOutput = javaOutput.trimmed();
        statusMessage(javaOutput);
    }

#ifdef Q_OS_WIN
    {
        QProcess javaInstallation;

        QString whereJava = "where java";
        javaInstallation.start(whereJava);
        bool started = javaInstallation.waitForStarted();
        if (!javaInstallation.waitForFinished(30000)) // 3 Second timeout
            javaInstallation.kill();

        javaLocation = QString::fromLocal8Bit(javaInstallation.readAll());

        if(javaLocation.isEmpty())
            javaLocation = "Not Found";

        statusMessage("Java location: "+ javaLocation);

        qDebug()<<"WHERE JAVA:"<<javaLocation;
    }

    // Check if Java Home does not contain the java location
    if(!javaLocation.contains(javaHomeVal))
        errorMessage("Warning, JAVA_HOME is not found the Java locations on your system. This may cause issues at runtime. Please check your JAVA_HOME value and make sure Java is added to the PATH variable in your system's environment variables.");
#endif

    statusMessage("Running script: " + pySCRIPT);

    proc->setProcessChannelMode(QProcess::SeparateChannels);

    QString python;

    // Check if a python path is given in the app
    QSettings settings("SimCenter", QCoreApplication::applicationName()); //These names will need to be constants to be shared
    QVariant  pythonLocationVariant = settings.value("pythonExePath");
    if (pythonLocationVariant.isValid()) {
        python = pythonLocationVariant.toString();

    }

    if(python.isEmpty())
    {

#ifdef Q_OS_WIN
        python = appDir+QDir::separator()+"PythonEnv"+QDir::separator()+"python.exe";
#else
        python = appDir+QDir::separator()+"PythonEnv"+QDir::separator()+"bin"+QDir::separator()+"python";
#endif

        QFileInfo checkPython(python);

        // check if exe exists and if yes: Is it really a exe and no directory?
        if (!checkPython.exists() || !checkPython.isExecutable()) {
            errorMessage(QString("NO PYTHON FOUND"));
            return false;
        }
    }

    statusMessage("Starting the analysis... this may take a while!");

    procEnv.insert("PATH", python);
    procEnv.insert("PYTHONPATH", python);
    proc->setProcessEnvironment(procEnv);

    qDebug() << "Env: "<< procEnv.toStringList();
    qDebug() << "PATH: " << python;

#ifdef Q_OS_WIN

    QStringList args{pySCRIPT,"-i",inputFile};

    python = QString("\"") + python + QString("\"");

    qDebug() << python;
    qDebug() << args;

    proc->start(python,args);

    bool failed = false;
    if (!proc->waitForStarted())
    {
        qDebug() << "Failed to start the workflow!!! exit code returned: " << proc->exitCode();
        qDebug() << proc->errorString().split('\n');

        errorMessage("Failed to start the workflow!!!");
        errorMessage(proc->errorString());

        failed = true;
    }

//    if(!proc->waitForFinished(-1))
//    {
//        qDebug() << "Failed to finish running the workflow!!! exit code returned: " << proc->exitCode();
//        qDebug() << proc->errorString();

//        errorMessage("Failed to finish running the workflow!!! exit code returned: " + QString::number(proc->exitCode()));
//        errorMessage(proc->errorString());

//        failed = true;
//    }

    if(0 != proc->exitCode())
    {
        qDebug() << "Failed to run the workflow!!! exit code returned: " << proc->exitCode();
        qDebug() << proc->errorString();

        errorMessage("Failed to run the workflow!!!");
        failed = true;
    }

    if(failed)
    {
        qDebug().noquote() << proc->readAll();
        qDebug().noquote() << proc->readAllStandardError();
        return false;
    }

#else

    QString command;

    command = python + " \"" + pySCRIPT + "\"" + " -i " +  "\"" + inputFile + "\"";

    // Clean up the command for the debug log
    auto commandClean = command;
    commandClean.remove('\"');

    QDebug debugHelper = qDebug();
    debugHelper.noquote();
    debugHelper << "PYTHON COMMAND: " << commandClean;

    //    proc->execute("bash", QStringList() << "-c" <<  command);
    proc->start("bash", QStringList() << "-c" <<  command);
    proc->waitForStarted();

#endif


    return 0;
}

void
LocalApplication::displayed(void){
    this->onRunButtonPressed();
}



void LocalApplication::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus == QProcess::ExitStatus::CrashExit)
    {
        QString errText("Error, the process running the hazard simulation script crashed");
        errorMessage(errText);

        // Output to console and to text edit
        qDebug()<<errText;

        return;
    }

    if(exitCode != 0)
    {
        QString errText("An error occurred in the Hazard Simulation script, the exit code is " + QString::number(exitCode));

        errorMessage(errText);

//        this->appendText(proc->errorString());

        // Output to console and to text edit
        qDebug()<<errText;

        return;
    }

    statusMessage("Analysis Complete\n");

    auto SCPrefs = OpenSRAPreferences::getInstance();
    auto resultsDirectory = SCPrefs->getLocalWorkDir();

    emit processResults(resultsDirectory, QString(), QString());
}


void LocalApplication::handleProcessStarted(void)
{
    statusMessage("Running OpenSRA script in the background.\n");
}


void LocalApplication::handleProcessTextOutput(void)
{
    QByteArray output =  proc->readAllStandardOutput();
    //    QByteArray output =  proc->readAllStandardError();

    statusMessage(QString(output));
}




