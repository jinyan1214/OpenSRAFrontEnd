#include "WorkflowAppWidget.h"
#include "Utils/ProgramOutputDialog.h"

#include <QWidget>
#include <QDebug>

ProgramOutputDialog *WorkflowAppWidget::progressDialog = nullptr;

WorkflowAppWidget::WorkflowAppWidget(QWidget *parent)
    :QWidget(parent)
{
    this->setContentsMargins(0,0,0,0);
    progressDialog = ProgramOutputDialog::getInstance(this);
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


ProgramOutputDialog *WorkflowAppWidget::getProgressDialog()
{
    return progressDialog;
}



