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

 protected:

 signals:
    void attemptLogin(QString, QString);
    void logout();

  public slots:
    // for menu items
    void newFile();
    void open();
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
    void onExitButtonClicked();

    // for error messages
    void statusMessage(QString message);
    void errorMessage(QString message);
    void fatalMessage(QString message);


 private:
    void setCurrentFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void loadFile(const QString &fileName);

    void createActions();

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *selectSubMenu;
    QMenu *toolsMenu;
    QMenu *optionsMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;


    QString currentFile;
    WorkflowAppWidget *inputWidget;

    QPushButton *loginButton;
    QLabel *errorLabel;

    QString versionText;
    QString aboutText;
    QString aboutTitle;
    QString aboutSource;
    QString copyrightText;
    QString manualURL;
    QString feedbackURL;
    QString citeText;

    OpenSRAPreferences *thePreferences;
};

#endif // MAIN_WINDOW_WORKFLOW_APP_H
