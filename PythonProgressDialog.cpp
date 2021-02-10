#include "PythonProgressDialog.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QPlainTextEdit>

PythonProgressDialog::PythonProgressDialog(QWidget* parent, QProcess* process) : QDialog(parent), proc(process)
{

    this->setWindowModality(Qt::ApplicationModal);
    this->setWindowTitle("Python Output");
    this->setAutoFillBackground(true);

    auto progressLayout = new QVBoxLayout(this);

    progressTextEdit = new QPlainTextEdit(this);
    progressTextEdit->setWordWrapMode(QTextOption::WrapMode::WordWrap);
    progressTextEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    progressTextEdit->setReadOnly(true);

    // give it some dimension
    int nWidth = 800;
    int nHeight = 500;
    if (parent != NULL)
        setGeometry(parent->x() + parent->width()/2,
                    parent->y() + parent->height()/2 - nHeight/2,
                    nWidth, nHeight);
    else
        resize(nWidth, nHeight);


    progressLayout->addWidget(progressTextEdit);
}


void PythonProgressDialog::appendText(const QString text)
{
    progressTextEdit->appendPlainText(text+ "\n");
}


void PythonProgressDialog::clear(void)
{
    progressTextEdit->clear();
}


void PythonProgressDialog::showDialog(bool visible)
{
    if(visible)
    {
        this->show();
        this->raise();
        this->activateWindow();
    }
    else
    {
        this->hide();
    }

}


void PythonProgressDialog::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus == QProcess::ExitStatus::CrashExit)
    {
        QString errText("Error, the process running the hazard simulation script crashed");
        this->appendErrorMessage(errText);

        // Output to console and to text edit
        qDebug()<<errText;

        return;
    }

    if(exitCode != 0)
    {
        QString errText("An error occurred in the Hazard Simulation script, the exit code is " + QString::number(exitCode));

        this->appendErrorMessage(errText);

//        this->appendText(proc->errorString());

        // Output to console and to text edit
        qDebug()<<errText;

        return;
    }

    progressTextEdit->appendPlainText("Analysis Complete\n");
}


void PythonProgressDialog::handleProcessStarted(void)
{
    progressTextEdit->appendPlainText("Running OpenSRA script in the background.\n");
}


void PythonProgressDialog::handleProcessTextOutput(void)
{
    QByteArray output =  proc->readAllStandardOutput();
    //    QByteArray output =  proc->readAllStandardError();

    progressTextEdit->appendPlainText(QString(output));
}


void PythonProgressDialog::appendErrorMessage(const QString text)
{
    progressTextEdit->appendPlainText("\n");

    auto msgStr = QString("<font color=%1>").arg("red") + text + QString("</font>") + QString("<font color=%1>").arg("black") + QString("&nbsp;") + QString("</font>");

    // Output to console and to text edit
    progressTextEdit->appendHtml(msgStr);

    progressTextEdit->appendPlainText("\n");
}
