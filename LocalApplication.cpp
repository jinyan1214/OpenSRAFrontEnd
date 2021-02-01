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
#include "PythonProgressDialog.h"
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
    progressDialog = new PythonProgressDialog(parent,proc);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), progressDialog, &PythonProgressDialog::handleProcessFinished);
    connect(proc, &QProcess::readyReadStandardOutput, progressDialog, &PythonProgressDialog::handleProcessTextOutput);
    connect(proc, &QProcess::started, progressDialog, &PythonProgressDialog::handleProcessStarted);
    
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &LocalApplication::handleProcessFinished);

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
    progressDialog->appendText("Setting up temporary directory");

    QString workingDir = OpenSRAPreferences::getInstance()->getLocalWorkDir();
    QDir dirWork(workingDir);
    if (!dirWork.exists())
        if (!dirWork.mkpath(workingDir)) {
            QString errorMessage = QString("Could not create Working Dir: ") + workingDir
                    + QString(". Change the Local Jobs Directory location in preferences.");
            progressDialog->appendText(errorMessage);
            emit sendErrorMessage(errorMessage);;

            return;
        }
    

    //   QString appDir = appDirName->text();
    QString appDir = OpenSRAPreferences::getInstance()->getAppDir();
    QDir dirApp(appDir);
    if (!dirApp.exists()) {
        QString errorMessage = QString("The application directory, ") + appDir +QString(" specified does not exist!. Check Local Application Directory im Preferences");
        progressDialog->appendText(errorMessage);
        emit sendErrorMessage(errorMessage);;
        return;
    }

    progressDialog->appendText("Gathering files to local workdir");
    emit sendStatusMessage("Gathering Files to local workdir");
    emit setupForRun(workingDir, workingDir);
}


//
// now use the applications Workflow Application EE-UQ.py  to run dakota and produce output files:
//    dakota.in dakota.out dakotaTab.out dakota.err
//

bool
LocalApplication::setupDoneRunApplication(QString &/*tmpDirectory*/, QString &inputFile)
{

    progressDialog->clear();
    progressDialog->showDialog(true);

    QString appDir = OpenSRAPreferences::getInstance()->getAppDir();

    QString pySCRIPT;

    QDir scriptDir(appDir);
    scriptDir.cd("OpenSRABackend");
    pySCRIPT = scriptDir.absoluteFilePath(workflowScript);

    QFileInfo check_script(pySCRIPT);
    // check if file exists and if yes: Is it really a file and no directory?
    if (!check_script.exists() || !check_script.isFile())
    {
        progressDialog->appendText("Cannot find the Python script: " + pySCRIPT);

        emit sendErrorMessage(QString("NO SCRIPT FILE: ") + pySCRIPT);
        return false;
    }


    {
        progressDialog->appendText("Looking for Java");
        QProcess program;
        QString commandToStart= "java -version";
        program.start(commandToStart);
        bool started = program.waitForStarted();
        if (!program.waitForFinished(30000)) // 3 Second timeout
            program.kill();

        if(!started)
        {
            progressDialog->appendText("Could not start Java on the system");
            return false; // Not found or which does not work
        }

        QString stdError = QString::fromLocal8Bit(program.readAllStandardError());
        stdError.trimmed();
        progressDialog->appendText(stdError);
    }


    progressDialog->appendText("Running script: " + pySCRIPT);

    proc->setProcessChannelMode(QProcess::SeparateChannels);
    auto procEnv = QProcessEnvironment();

    QString python;

    // Check if a python path is given in common
    QSettings settings("SimCenter", "Common"); //These names will need to be constants to be shared
    QVariant  pythonLocationVariant = settings.value("pythonExePath");
    if (pythonLocationVariant.isValid()) {
        python = pythonLocationVariant.toString();
    }

    if(python.isEmpty())
    {
        // If not look for the venv
        QDir pythonExeDir(appDir);
        pythonExeDir.cd("PythonEnv");

#ifdef Q_OS_WIN
        python = pythonExeDir.absoluteFilePath("python.exe");
#else
        python = appDir+QDir::separator()+"PythonEnv"+QDir::separator()+"bin"+QDir::separator()+"python";
#endif

        QFileInfo checkPython(python);

        // check if exe exists and if yes: Is it really a exe and no directory?
        if (!checkPython.exists() || !checkPython.isExecutable()) {
            progressDialog->appendText("NO PYTHON FOUND");
            emit sendErrorMessage(QString("NO PYTHON FOUND"));
            return false;
        }
    }

    QString msg = "Starting the analysis... this may take a while!";
    progressDialog->appendText(msg);
    emit sendStatusMessage(msg);

    procEnv.insert("PATH", python);
    procEnv.insert("PYTHONPATH", python);
    proc->setProcessEnvironment(procEnv);

    qDebug() << "PATH: " << python;

#ifdef Q_OS_WIN

    QStringList args{pySCRIPT,"-i",inputFile};

    python = QString("\"") + python + QString("\"");

    qDebug() << python;
    qDebug() << args;

    proc->start(python,args);

    bool failed = false;
    if (!proc->waitForStarted(-1))
    {
        qDebug() << "Failed to start the workflow!!! exit code returned: " << proc->exitCode();
        qDebug() << proc->errorString().split('\n');

        progressDialog->appendText("Failed to start the workflow!!!");
        progressDialog->appendText( proc->errorString());

        failed = true;
    }

    if(!proc->waitForFinished(-1))
    {
        qDebug() << "Failed to finish running the workflow!!! exit code returned: " << proc->exitCode();
        qDebug() << proc->errorString();

        progressDialog->appendText("Failed to finish running the workflow!!! exit code returned: " + QString::number(proc->exitCode()));
        progressDialog->appendText(proc->errorString());

        emit sendStatusMessage("Failed to finish running the workflow!!!");
        failed = true;
    }


    if(0 != proc->exitCode())
    {
        qDebug() << "Failed to run the workflow!!! exit code returned: " << proc->exitCode();
        qDebug() << proc->errorString();
        emit sendStatusMessage("Failed to run the workflow!!!");
        failed = true;
    }

    if(failed)
    {
        qDebug().noquote() << proc->readAllStandardOutput();
        qDebug().noquote() << proc->readAllStandardError();
        return false;
    }

#else

    QString command;

    command = python + " " + pySCRIPT  + " -i " + inputFile;

    qDebug() << "PYTHON COMMAND" << command;

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
    if(exitStatus == QProcess::ExitStatus::CrashExit || exitCode != 0)
    {
        return;
    }

    //
    // process the results
    //

    emit processResults("NA");
}


