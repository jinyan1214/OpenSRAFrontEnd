// Written: Stevan Gavrilovic
// Latest Revision: 09/20

#include "MainWindowWorkflowApp.h"
#include "WorkflowAppOpenSRA.h"

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
#include <QDir>
#include <QStatusBar>

static QString logFilePath;
static bool logToFile = false;


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

    // Setting Core Application Name, Organization, Version and Google Analytics Tracking Id
    QCoreApplication::setApplicationName("OpenSRA");
    QCoreApplication::setOrganizationName("SimCenter");
    QCoreApplication::setApplicationVersion("1.0.0");

    // set up logging of output messages for user debugging
    logFilePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            + QDir::separator() + QCoreApplication::applicationName();

    // make sure tool dir exists in Documentss folder
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

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);

    // create the main window

    WorkflowAppWidget *theInputApp = new WorkflowAppOpenSRA();
    MainWindowWorkflowApp w(QString("OpenSRA: Open-Source Seismic Risk Assessment Tool"), theInputApp);

    QString textAboutOpenSRA = "\
            <p> \
            This is the OpenSRA application. More to come here\
            <p>\  ";

            w.setAbout(textAboutOpenSRA);

    // This is the title displayed in the on About dialog
    QString aboutTitle = "About the OpenSRA Application";

    QString aboutSource = "About OpenSRA";
    w.setAbout(aboutTitle, aboutSource);

    QString version("Version 1.0.0");
    w.setVersion(version);

    QString citeText("Cite text here");
    w.setCite(citeText);

    QString manualURL("Link to user manual here");
    w.setDocumentationURL(manualURL);

    QString messageBoardURL("Link to message");
    w.setFeedbackURL(messageBoardURL);

    //
    // show the main window, set styles & start the event loop
    //

    w.show();
    w.statusBar()->showMessage("Ready", 5000);

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
