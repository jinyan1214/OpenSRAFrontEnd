#ifndef MAIN_WINDOW_WORKFLOW_APP_H
#define MAIN_WINDOW_WORKFLOW_APP_H

#include <QMainWindow>
#include <QItemSelection>
#include <QTreeView>
#include <QStandardItemModel>
#include <QJsonObject>

class WorkflowAppWidget;
class SimCenterWidget;
class QPushButton;
class QLabel;
class OpenSRAPreferences;
class ProgramOutputDialog;

class MainWindowWorkflowApp : public QMainWindow
{
  Q_OBJECT
    
public:
    explicit MainWindowWorkflowApp(QString appName, WorkflowAppWidget *theApp, QWidget *parent = 0);
    ~MainWindowWorkflowApp();
    void setCopyright(QString &copyright);
    void setVersion(QString &version);
    void setDocumentationURL(QString &version);
    void setAbout(QString &about);
    void setAbout(QString &, QString&);
    void setFeedbackURL(QString &feedback);
    void setCite(QString &cite);
    void createHelpMenu();

    QPushButton *getPreProcessButton() const;

    QPushButton *getRunButton() const;

protected:

 signals:
    void attemptLogin(QString, QString);
    void logout();

    void sendStatusMessage(QString message);
    void sendErrorMessage(QString message);
    void sendFatalMessage(QString message);
    void sendInfoMessage(QString message);

  public slots:
    // for menu items
    void newFile();
    void openConfigJson();
    void openRunFolder();
    void openFile(QString filename);
    bool save();
    bool saveAs();

    void about();
    void manual();
    void submitFeedback();
    void version();
    void preferences();
    void copyright();
    void cite();

    // for main actions
    void onRunButtonClicked();
    void onPreprocessButtonClicked();
    void onExitButtonClicked();

 private:
    void setCurrentFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void loadFile(const QString &fileName);

    void createActions();

    QMenu *fileMenu = nullptr;
    QMenu *editMenu = nullptr;
    QMenu *selectSubMenu = nullptr;
    QMenu *toolsMenu = nullptr;
    QMenu *optionsMenu = nullptr;
    QMenu *helpMenu = nullptr;
    QToolBar *fileToolBar = nullptr;
    QToolBar *editToolBar = nullptr;


    QString currentFile;
    WorkflowAppWidget *inputWidget = nullptr;

    QPushButton *preProcessButton = nullptr;
    QPushButton *runButton = nullptr;
    QPushButton *loginButton = nullptr;

    ProgramOutputDialog* statusWidget = nullptr;
    QDockWidget* statusDockWidget = nullptr;

    QString versionText;
    QString aboutText;
    QString aboutTitle;
    QString aboutSource;
    QString copyrightText;
    QString manualURL;
    QString feedbackURL;
    QString citeText;

    OpenSRAPreferences *thePreferences = nullptr;
};

#endif // MAIN_WINDOW_WORKFLOW_APP_H
