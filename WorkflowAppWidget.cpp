#include "WorkflowAppWidget.h"
#include "Utils/PythonProgressDialog.h"

#include <QWidget>
#include <QDebug>

PythonProgressDialog *WorkflowAppWidget::progressDialog = nullptr;

WorkflowAppWidget::WorkflowAppWidget(QWidget *parent)
    :QWidget(parent)
{
    this->setContentsMargins(0,0,0,0);
    progressDialog = PythonProgressDialog::getInstance(this);
}


WorkflowAppWidget::~WorkflowAppWidget()
{

}


void WorkflowAppWidget::setMainWindow(MainWindowWorkflowApp* window)
{
    theMainWindow = window;
}


MainWindowWorkflowApp *WorkflowAppWidget::getTheMainWindow() const
{
    return theMainWindow;
}


void WorkflowAppWidget::statusMessage(const QString msg)
{
    qDebug() << "WorkflowAppWidget::statusMessage" << msg;
    emit sendStatusMessage(msg);
}


void WorkflowAppWidget::errorMessage(const QString msg)
{
    qDebug() << "WorkflowAppWidget::errorMessage" << msg;
    emit sendErrorMessage(msg);
}


void WorkflowAppWidget::fatalMessage(const QString msg)
{
    emit sendFatalMessage(msg);
}


void WorkflowAppWidget::showOutputDialog(void)
{
    progressDialog->setVisibility(true);
}


PythonProgressDialog *WorkflowAppWidget::getProgressDialog()
{
    return progressDialog;
}



