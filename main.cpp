// Written: Stevan Gavrilovic

#include "MainWindowWorkflowApp.h"
#include "WorkflowAppOpenSRA.h"
#include "OpenSRAUserPass.h"
#include "OpenSRAPreferences.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QObject>
#include <QCoreApplication>
#include <QApplication>
#include <QFile>
#include <QTime>
#include <QTextStream>
#include <QOpenGLWidget>
#include <QStandardPaths>
#include <QSettings>
#include <QDir>
#include <QStatusBar>

static bool logToFile = false;

#include "qgsapplication.h"

// customMessgaeOutput code taken from web:
// https://stackoverflow.com/questions/4954140/how-to-redirect-qdebug-qwarning-qcritical-etc-output

void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "Debug"}, {QtInfoMsg, "Info"}, {QtWarningMsg, "Warning"}, {QtCriticalMsg, "Critical"}, {QtFatalMsg, "Fatal"}});
    QByteArray localMsg = msg.toLocal8Bit();
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss.zzz");
    QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
    QString logLevelName = msgLevelHash[type];
    QByteArray logLevelMsg = logLevelName.toLocal8Bit();

    if (logToFile) {
        QString txt = QString("%1 %2: %3 (%4)").arg(formattedTime, logLevelName, msg,  context.file);
        QFile outFile(logFilePath);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt << endl;
        outFile.close();
    } else {
        fprintf(stderr, "%s %s: %s (%s:%u, %s)\n", formattedTimeMsg.constData(), logLevelMsg.constData(), localMsg.constData(), context.file, context.line, context.function);
        fflush(stderr);
    }

    if (type == QtFatalMsg)
        abort();
}


int main(int argc, char *argv[])
{

    QString OpenSRAVersion = APP_VERSION;

    // Setting Core Application Name, Organization, Version
    QCoreApplication::setApplicationName("OpenSRA");
    QCoreApplication::setOrganizationName("SimCenter");
    QCoreApplication::setApplicationVersion(OpenSRAVersion);


#ifdef Q_OS_WIN
//    QGuiApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

#ifdef Q_OS_MACOS
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

#ifdef Q_OS_LINUX
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    // Set up the application
    // Start the Application

    QgsApplication a( argc, argv, true );


    auto prefs = OpenSRAPreferences::getInstance();

    // Check if the app version has changed
    QSettings settings("SimCenter", QCoreApplication::applicationName());
    QVariant appVers = settings.value("AppVersion");
    if (appVers.isValid())
    {
        auto prevAppVers = appVers.toString();
        // If the app was updated, reset the preferences to pick up any potential updates
        if(OpenSRAVersion != prevAppVers)
        {
            prefs->resetPreferences(true);
            prefs->savePreferences(true);

            settings.setValue("AppVersion", OpenSRAVersion);
        }
    }
    else
    {
        // Set for the first time
        settings.setValue("AppVersion", OpenSRAVersion);
    }


    // set up logging of output messages for user debugging
    logFilePath = prefs->getLocalWorkDir();

    // make sure tool dir exists in Documents folder
    QDir dirWork(logFilePath);
    if (!dirWork.exists())
        if (!dirWork.mkpath(logFilePath)) {
            qDebug() << QString("Could not create Working Dir: ") << logFilePath;
        }

    // full path to debug.log file
    logFilePath = logFilePath + QDir::separator() + QString("debug.log");

    // remove old log file
    QFile debugFile(logFilePath);
    debugFile.remove();

    QByteArray envVar = qgetenv("QTDIR");

    if (envVar.isEmpty())
        logToFile = true;

    qInstallMessageHandler(customMessageOutput);

    // Set the key for the ArcGIS interface

    // create the main window
    WorkflowAppOpenSRA *theInputApp = new WorkflowAppOpenSRA();
    MainWindowWorkflowApp mainWindowApp(QString("OpenSRA: Open-Source Seismic Risk Assessment Tool"), theInputApp, nullptr);

    // Create the  menu bar and actions to run the examples
    theInputApp->initialize();

    QString textAboutOpenSRA = "\
            <p> \
            This is the OpenSRA application. More to come here\
            <p>\  ";

            mainWindowApp.setAbout(textAboutOpenSRA);

    // This is the title displayed in the on About dialog
    QString aboutTitle = "About the OpenSRA Application";

    QString aboutSource = "Open-source Seismic Risk Assessment (OpenSRA) Tool";
    mainWindowApp.setAbout(aboutTitle, aboutSource);

    QString version("Version "+ OpenSRAVersion);
    mainWindowApp.setVersion(version);

    QString citeText("OpenSRA");
    mainWindowApp.setCite(citeText);

    QString manualURL("https://docs.google.com/document/d/1zwgICunkn6PVedDiErWoyXhVvpM-vZqyZ7emsUKSxeQ/edit?usp=sharing");
    mainWindowApp.setDocumentationURL(manualURL);

    QString messageBoardURL("Link to message board");
    mainWindowApp.setFeedbackURL(messageBoardURL);

    //
    // show the main window, set styles & start the event loop
    //

    mainWindowApp.setWindowState((mainWindowApp.windowState() & ~(Qt::WindowMinimized | Qt::WindowFullScreen))| Qt::WindowMaximized);
    mainWindowApp.show();
    // mainWindowApp.statusBar()->showMessage("Ready", 5000);

#ifdef Q_OS_WIN
    QFile file(":/styles/stylesheetWIN.qss");
#endif

#ifdef Q_OS_MACOS
    QFile file(":/styles/stylesheetMAC.qss");
#endif

#ifdef Q_OS_LINUX
    QFile file(":/styles/stylesheetMAC.qss");
#endif


    if(file.open(QFile::ReadOnly)) {
        a.setStyleSheet(file.readAll());
        file.close();
    } else {
        qDebug() << "could not open stylesheet";
    }

    int res = a.exec();

    return res;
}
