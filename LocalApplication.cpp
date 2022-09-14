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

LocalApplication::LocalApplication(QString workflowScriptName, QWidget *parent) : Application(parent)
{
    this->workflowScript = workflowScriptName;

    theProcessHandler = std::make_unique<PythonProcessHandler>();
}


bool LocalApplication::outputToJSON(QJsonObject &jsonObject)
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


void LocalApplication::setupTempDir()
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

}


void LocalApplication::onRunButtonPressed(QPushButton* button)
{

    //   QString appDir = appDirName->text();
    QString appDir = OpenSRAPreferences::getInstance()->getAppDir();
    QDir dirApp(appDir);
    if (!dirApp.exists()) {
        QString errMsg = QString("The application directory, ") + appDir +QString(" specified does not exist!. Check Local Application Directory im Preferences");
        errorMessage(errMsg);;
        return;
    }

    this->statusMessage("onRunButtonPressed "+ appDir);

    if(setUpForRun == false)
    {
        this->setupTempDir();

        this->runButton = button;

        statusMessage("Gathering Files to local workdir");

        emit setupForRun(workingDir, workingDir);

    }


    emit setupForRunDone(workingDir, workingDir);
}


void LocalApplication::onPreprocessButtonPressed(QPushButton* button)
{
    //   QString appDir = appDirName->text();
    QString appDir = OpenSRAPreferences::getInstance()->getAppDir();
    QDir dirApp(appDir);
    if (!dirApp.exists()) {
        QString errMsg = QString("The application directory, ") + appDir +QString(" specified does not exist!. Check Local Application Directory im Preferences");
        errorMessage(errMsg);;
        return;
    }


    auto pathToPreprocessScript = appDir + QDir::separator() + "Preprocess.py -w";

    this->statusMessage("onPreprocessButtonPressed "+ pathToPreprocessScript);

//    if(setUpForRun == false)
//    {
        this->setupTempDir();

        this->preProcessButton = button;

        statusMessage("Gathering Files to local workdir");

        emit setupForPreprocessing(workingDir, workingDir);

//        setUpForRun = true;

        return;
//    }

//    emit setupForPreProcessingDone(workingDir, workingDir);
}


void LocalApplication::clear()
{
    setUpForRun = false;
    workingDir.clear();
}


bool LocalApplication::setupDoneRunPreprocessing(QString &/*tmpDirectory*/, QString &inputFile)
{

    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss");
    QString msg1 = "Running preprocessing step on OpenSRA version " + QCoreApplication::applicationVersion() + " at " + currentTime;
    statusMessage(msg1);

    QString appDir = OpenSRAPreferences::getInstance()->getAppDir();

    QString pySCRIPT;

    auto pathToPreprocessScript = appDir + QDir::separator() + "Preprocess.py";

    QDir scriptDir(appDir);
    scriptDir.cd("OpenSRABackend");
    pySCRIPT = scriptDir.absoluteFilePath(pathToPreprocessScript);

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

    statusMessage("Running script: " + pySCRIPT);

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

    theProcessHandler->setProcessEnv(procEnv);

    qDebug() << "Env: "<< procEnv.toStringList();
    qDebug() << "PATH: " << python;

#ifdef Q_OS_WIN

    QStringList args{pySCRIPT,"-i",inputFile};

    python = QString("\"") + python + QString("\"");

    qDebug() << python;
    qDebug() << args;

    theProcessHandler->startProcess(python,args);

    //    bool failed = false;
    //    if (!proc->waitForStarted())
    //    {
    //        qDebug() << "Failed to start the workflow!!! exit code returned: " << proc->exitCode();
    //        qDebug() << proc->errorString().split('\n');

    //        errorMessage("Failed to start the workflow!!!");
    //        errorMessage(proc->errorString());

    //        failed = true;
    //    }

    //    //    if(!proc->waitForFinished(-1))
    //    //    {
    //    //        qDebug() << "Failed to finish running the workflow!!! exit code returned: " << proc->exitCode();
    //    //        qDebug() << proc->errorString();

    //    //        errorMessage("Failed to finish running the workflow!!! exit code returned: " + QString::number(proc->exitCode()));
    //    //        errorMessage(proc->errorString());

    //    //        failed = true;
    //    //    }

    //    if(0 != proc->exitCode())
    //    {
    //        qDebug() << "Failed to run the workflow!!! exit code returned: " << proc->exitCode();
    //        qDebug() << proc->errorString();

    //        errorMessage("Failed to run the workflow!!!");
    //        failed = true;
    //    }

    //    if(failed)
    //    {
    //        qDebug().noquote() << proc->readAll();
    //        qDebug().noquote() << proc->readAllStandardError();
    //        return false;
    //    }

#else

    QString command;

    command = python + " \"" + pySCRIPT + "\"" + " -w " +  "\"" + inputFile + "\"";

    // Clean up the command for the debug log
    auto commandClean = command;
    commandClean.remove('\"');

    QDebug debugHelper = qDebug();
    debugHelper.noquote();
    debugHelper << "PYTHON COMMAND: " << commandClean;

    theProcessHandler->startProcess("bash", QStringList() << "-c" <<  command, "preprocessing", runButton);
#endif

    return 0;
}


bool LocalApplication::setupDoneRunApplication(QString &tmpDirectory, QString &inputFile)
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss");
    QString msg1 = "Running OpenSRA backend version " + QCoreApplication::applicationVersion() + " at " + currentTime;
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


    statusMessage("Running script: " + pySCRIPT);


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

    theProcessHandler->setProcessEnv(procEnv);

    qDebug() << "Env: "<< procEnv.toStringList();
    qDebug() << "PATH: " << python;

#ifdef Q_OS_WIN

    QStringList args{pySCRIPT,"-i",inputFile};

    python = QString("\"") + python + QString("\"");

    qDebug() << python;
    qDebug() << args;

    theProcessHandler->startProcess(python,args);

    //    bool failed = false;
    //    if (!proc->waitForStarted())
    //    {
    //        qDebug() << "Failed to start the workflow!!! exit code returned: " << proc->exitCode();
    //        qDebug() << proc->errorString().split('\n');

    //        errorMessage("Failed to start the workflow!!!");
    //        errorMessage(proc->errorString());

    //        failed = true;
    //    }

    //    //    if(!proc->waitForFinished(-1))
    //    //    {
    //    //        qDebug() << "Failed to finish running the workflow!!! exit code returned: " << proc->exitCode();
    //    //        qDebug() << proc->errorString();

    //    //        errorMessage("Failed to finish running the workflow!!! exit code returned: " + QString::number(proc->exitCode()));
    //    //        errorMessage(proc->errorString());

    //    //        failed = true;
    //    //    }

    //    if(0 != proc->exitCode())
    //    {
    //        qDebug() << "Failed to run the workflow!!! exit code returned: " << proc->exitCode();
    //        qDebug() << proc->errorString();

    //        errorMessage("Failed to run the workflow!!!");
    //        failed = true;
    //    }

    //    if(failed)
    //    {
    //        qDebug().noquote() << proc->readAll();
    //        qDebug().noquote() << proc->readAllStandardError();
    //        return false;
    //    }

#else

    QString command;

    command = python + " \"" + pySCRIPT + "\"" + " -i " +  "\"" + inputFile + "\"";

    // Clean up the command for the debug log
    auto commandClean = command;
    commandClean.remove('\"');

    QDebug debugHelper = qDebug();
    debugHelper.noquote();
    debugHelper << "PYTHON COMMAND: " << commandClean;

    theProcessHandler->startProcess("bash", QStringList() << "-c" <<  command, "backend ", preProcessButton);
#endif
}








