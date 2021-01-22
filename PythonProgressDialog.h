#ifndef PYTHONPROGRESSDIALOG_H
#define PYTHONPROGRESSDIALOG_H

#include <QDialog>
#include <QProcess>

class QPlainTextEdit;

class PythonProgressDialog : public QDialog
{
    Q_OBJECT

public:
    PythonProgressDialog(QWidget* parent, QProcess* process);

    void appendText(const QString text);

    void appendErrorMessage(const QString text);

    // Handles the results when the user is finished
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    // Brings up the dialog and tells the user that the process has started
    void handleProcessStarted(void);

    // Displays the text output of the process in the dialog
    void handleProcessTextOutput(void);

    void clear(void);

    void showDialog(bool visible);

signals:
    void processResults(QString);

private:

    QPlainTextEdit* progressTextEdit;
    QProcess* proc;
};

#endif // PYTHONPROGRESSDIALOG_H
