#include "MainWindowWorkflowApp.h"
#include "sectiontitle.h"
#include "WorkflowAppWidget.h"
#include "HeaderWidget.h"
#include "SimCenterPreferences.h"
#include "Utils/RelativePathResolver.h"
#include "Utils/dialogabout.h"

#include <QTreeView>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QDebug>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMenuBar>
#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSettings>

MainWindowWorkflowApp::MainWindowWorkflowApp(QString appName, WorkflowAppWidget *theApp, QWidget *parent)
    : QMainWindow(parent), inputWidget(theApp)
{
    //
    // create a layout & widget for central area of this QMainWidget
    //  to this widget we will add a header, selection, button and footer widgets
    //

    QWidget *centralWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    centralWidget->setLayout(layout);
    centralWidget->setContentsMargins(0,0,0,0);

    //
    // resize to primary screen
    //
    /*************************** keep around
    QSize availableSize = qApp->desktop()->availableGeometry().size();
    int availWidth = availableSize.width();
    int availHeight = availableSize.height();
    QSize newSize( availWidth*.85, availHeight*.65 );

    setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                    Qt::AlignCenter,
                    newSize,
                    qApp->desktop()->availableGeometry()
                     )
        );
        ********************************************************/

    QRect rec = QGuiApplication::primaryScreen()->geometry();
    int height = this->height()<int(0.75*rec.height())?int(0.75*rec.height()):this->height();
    int width  = this->width()<int(0.75*rec.width())?int(0.75*rec.width()):this->width();
    this->resize(width, height);


    //
    // add SimCenter Header
    //

    HeaderWidget *header = new HeaderWidget();
    header->setHeadingText(appName);
    layout->addWidget(header);

    // place a location for messages;
    QHBoxLayout *layoutMessages = new QHBoxLayout();
    errorLabel = new QLabel();
    layoutMessages->addWidget(errorLabel);
    header->appendLayout(layoutMessages);

    // place login info
    QHBoxLayout *layoutLogin = new QHBoxLayout();
    //    QLabel *name = new QLabel();
    //    //name->setText("");
    //    loginButton = new QPushButton();
    //    loginButton->setText("Login");
    //    layoutLogin->addWidget(name);
    //    layoutLogin->addWidget(loginButton);
    layoutLogin->setAlignment(Qt::AlignLeft);
    header->appendLayout(layoutLogin);

    layout->addWidget(inputWidget);

    //
    // add run, run-DesignSafe and exit buttons into a new widget for buttons
    //

    // create the buttons widget and a layout for it
    QHBoxLayout *pushButtonLayout = new QHBoxLayout();

    // Create run and exit buttons
    QPushButton *runButton = new QPushButton();
    runButton->setText(tr("RUN"));
    pushButtonLayout->addWidget(runButton);

    QPushButton *exitButton = new QPushButton();
    exitButton->setText(tr("Exit"));
    pushButtonLayout->addWidget(exitButton);


    // connect some signals and slots

    // allow remote interface to send error and status messages
    connect(inputWidget,SIGNAL(sendErrorMessage(QString)),this,SLOT(errorMessage(QString)));
    connect(inputWidget,SIGNAL(sendStatusMessage(QString)),this,SLOT(statusMessage(QString)));
    connect(inputWidget,SIGNAL(sendFatalMessage(QString)),this,SLOT(fatalMessage(QString)));


    // connect(runButton, SIGNAL(clicked(bool)),this,SLOT(onRunButtonClicked()));
    // connect job manager
    connect(runButton, SIGNAL(clicked(bool)),this,SLOT(onRunButtonClicked()));
    connect(exitButton, SIGNAL(clicked(bool)),this,SLOT(onExitButtonClicked()));

    // add button widget to layout
    //layout->addWidget(buttonWidget);
    pushButtonLayout->setSpacing(10);
    layout->addLayout(pushButtonLayout);

    //
    // add SimCenter footer
    //

    //FooterWidget *footer = new FooterWidget();
    //layout->addWidget(footer);
    layout->setSpacing(0);

    this->setCentralWidget(centralWidget);

    this->createActions();

    inputWidget->setMainWindow(this);


    //
    // if have save login and passowrd fill in lineedits
    //


    //
    // strings needed in about menu, use set methods to override
    //

    manualURL = QString("");
    feedbackURL = QString("https://docs.google.com/forms/d/e/1FAIpQLSfh20kBxDmvmHgz9uFwhkospGLCeazZzL770A2GuYZ2KgBZBA/viewform");
    //    featureRequestURL = QString("https://docs.google.com/forms/d/e/1FAIpQLScTLkSwDjPNzH8wx8KxkyhoIT7AI9KZ16Wg9TuW1GOhSYFOag/viewform");
    versionText = QString("");
    citeText = QString("");
    aboutText = QString(tr("This is a SimCenter Workflow Applicatios"));

    aboutTitle = "About this Application"; // this is the title displayed in the on About dialog
    aboutSource = ":/Resources/docs/textAbout.html";  // this is an HTML file stored under resources

    copyrightText = QString("\
                            <p>\
                            Copyright text. \
                            <p>\
                            ");
}


MainWindowWorkflowApp::~MainWindowWorkflowApp()
{

}


bool MainWindowWorkflowApp::save()
{
    if (currentFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(currentFile);
    }
}


bool MainWindowWorkflowApp::saveAs()
{
    //
    // get filename
    //

    QFileDialog dialog(this, "Save Simulation Model");
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList filters;
    filters << "Json files (*.json)"
            << "All files (*)";
    dialog.setNameFilters(filters);

    if (dialog.exec() != QDialog::Accepted)
        return false;

    // and save the file
    return saveFile(dialog.selectedFiles().first());
}


void MainWindowWorkflowApp::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Simulation Model", "",  "Json files (*.json);;All files (*)");
    if (!fileName.isEmpty())
        loadFile(fileName);
}


void MainWindowWorkflowApp::openFile(QString fileName)
{
    if (!fileName.isEmpty())
        loadFile(fileName);
}


void MainWindowWorkflowApp::newFile()
{
    // clear old
    inputWidget->clear();

    // set currentFile blank
    setCurrentFile(QString());
}


void MainWindowWorkflowApp::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    //  setWindowModified(false);

    QString shownName = currentFile;
    if (currentFile.isEmpty())
        shownName = "untitled.json";

    setWindowFilePath(shownName);
}


bool MainWindowWorkflowApp::saveFile(const QString &fileName)
{
    //
    // open file
    //

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }


    //
    // create a json object, fill it in & then use a QJsonDocument
    // to write the contents of the object to the file in JSON format
    //

    QJsonObject json;
    inputWidget->outputToJSON(json);

    //Resolve relative paths before saving
    QFileInfo fileInfo(fileName);
    SCUtils::ResolveRelativePaths(json, fileInfo.dir());

    QJsonDocument doc(json);
    file.write(doc.toJson());

    // close file
    file.close();

    // set current file
    setCurrentFile(fileName);

    return true;
}


void MainWindowWorkflowApp::loadFile(const QString &fileName)
{
    //
    // open file
    //

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    // validate the document
    // JsonValidator class already takes a model type param, add additional model types as required

    /*
    JsonValidator *jval = new JsonValidator();
    jval->validate(this, BIM, fileName);
*/

    // place contents of file into json object
    QString val;
    val=file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = doc.object();

    //
    QFileInfo fileInfo(fileName);
    SCUtils::ResolveAbsolutePaths(jsonObj, fileInfo.dir());

    // close file
    file.close();

    // given the json object, create the C++ objects
    if ( ! (currentFile.isNull() || currentFile.isEmpty()) ) {
        inputWidget->clear();
    }
    inputWidget->inputFromJSON(jsonObj);

    setCurrentFile(fileName);
}


void MainWindowWorkflowApp::createActions() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));


    //const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    //const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));

    //QToolBar *fileToolBar = addToolBar(tr("File"));

    //    QAction *newAction = new QAction(tr("&New"), this);
    //    newAction->setShortcuts(QKeySequence::New);
    //    newAction->setStatusTip(tr("Create a new file"));
    //    connect(newAction, &QAction::triggered, this, &MainWindowWorkflowApp::newFile);
    //    fileMenu->addAction(newAction);

    QAction *openAction = new QAction(tr("&Open"), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing file"));
    connect(openAction, &QAction::triggered, this, &MainWindowWorkflowApp::open);
    fileMenu->addAction(openAction);
    //fileToolBar->addAction(openAction);


    QAction *saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save the document to disk"));
    connect(saveAction, &QAction::triggered, this, &MainWindowWorkflowApp::save);
    fileMenu->addAction(saveAction);

    QAction *saveAsAction = new QAction(tr("&Save As"), this);
    saveAsAction->setStatusTip(tr("Save the document with new filename to disk"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindowWorkflowApp::saveAs);
    fileMenu->addAction(saveAsAction);

    thePreferences = SimCenterPreferences::getInstance(this);
    QAction *preferenceAction = new QAction(tr("&Preferences"), this);
    preferenceAction->setStatusTip(tr("Set application preferences"));
    connect(preferenceAction, &QAction::triggered, this, &MainWindowWorkflowApp::preferences);
    fileMenu->addAction(preferenceAction);

    // strangely, this does not appear in menu (at least on a mac)!! ..
    // does Qt not allow as in tool menu by default?
    // check for yourself by changing Quit to drivel and it works
    QAction *exitAction = new QAction(tr("&Quit"), this);
    connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    exitAction->setStatusTip(tr("Exit the application"));
    fileMenu->addAction(exitAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&Version"), this, &MainWindowWorkflowApp::version);
    helpMenu->addAction(tr("&About"), this, &MainWindowWorkflowApp::about);
    helpMenu->addAction(tr("&Manual"), this, &MainWindowWorkflowApp::manual);
    helpMenu->addAction(tr("&Submit Bug/Feature Request"), this, &MainWindowWorkflowApp::submitFeedback);
    // QAction *submitFeature = helpMenu->addAction(tr("&Submit Bug/Feature Request"), this, &MainWindowWorkflowApp::submitFeatureRequest);
    helpMenu->addAction(tr("&How to Cite"), this, &MainWindowWorkflowApp::cite);
    helpMenu->addAction(tr("&License"), this, &MainWindowWorkflowApp::copyright);
}


void MainWindowWorkflowApp::statusMessage(const QString msg){
    errorLabel->setText(msg);
    qDebug() << "STATUS MESSAGE" << msg;
    QApplication::processEvents();
}


void MainWindowWorkflowApp::errorMessage(const QString msg){
    errorLabel->setText(msg);
    qDebug() << "ERROR MESSAGE" << msg;
    QApplication::processEvents();
}


void MainWindowWorkflowApp::fatalMessage(const QString msg){
    errorLabel->setText(msg);
    qDebug() << "FATAL MESSAGE" << msg;
}


void MainWindowWorkflowApp::version()
{
    QMessageBox msgBox;
    QSpacerItem *theSpacer = new QSpacerItem(700, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    msgBox.setText(versionText);
    QGridLayout *layout = (QGridLayout*)msgBox.layout();
    layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();
}


void MainWindowWorkflowApp::cite()
{
    QMessageBox msgBox;
    QSpacerItem *theSpacer = new QSpacerItem(700, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    msgBox.setText(citeText);
    QGridLayout *layout = (QGridLayout*)msgBox.layout();
    layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();
}


void MainWindowWorkflowApp::about()
{

    DialogAbout *dlg = new DialogAbout();
    dlg->setTitle(aboutTitle);
    dlg->setTextSource(aboutSource);

    //
    // adjust size of application window to the available display
    //
    QRect rec = QApplication::desktop()->screenGeometry();
    int height = 0.50*rec.height();
    int width  = 0.50*rec.width();
    dlg->resize(width, height);

    dlg->exec();
    delete dlg;
}


void MainWindowWorkflowApp::preferences()
{
    thePreferences->show();
}


void MainWindowWorkflowApp::submitFeedback()
{
    QDesktopServices::openUrl(QUrl(feedbackURL, QUrl::TolerantMode));
    //QDesktopServices::openUrl(QUrl("https://www.designsafe-ci.org/help/new-ticket/", QUrl::TolerantMode));
}


void MainWindowWorkflowApp::manual()
{
    QDesktopServices::openUrl(QUrl(manualURL, QUrl::TolerantMode));
    //QDesktopServices::openUrl(QUrl("https://www.designsafe-ci.org/help/new-ticket/", QUrl::TolerantMode));
}


void MainWindowWorkflowApp::copyright()
{
    QMessageBox msgBox;
    QSpacerItem *theSpacer = new QSpacerItem(700, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    msgBox.setText(copyrightText);
    QGridLayout *layout = (QGridLayout*)msgBox.layout();
    layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();

}


void MainWindowWorkflowApp::setCopyright(QString &newText)
{
    copyrightText = newText;
}


void MainWindowWorkflowApp::setVersion(QString &newText)
{
    versionText = newText;
}


void MainWindowWorkflowApp::setAbout(QString &newText)
{
    aboutText = newText +QString("<p> This work is based on material supported by the National Science Foundation under grant 1612843<p>");
    qDebug() << "ABOUT: " << aboutText;
}


void MainWindowWorkflowApp::setAbout(QString &newTitle, QString &newTextSource)
{
    aboutTitle  = newTitle;
    aboutSource = newTextSource;
}


void MainWindowWorkflowApp::setDocumentationURL(QString &newText)
{
    manualURL = newText;
}


void MainWindowWorkflowApp::setFeedbackURL(QString &newText)
{
    feedbackURL = newText;
}


void MainWindowWorkflowApp::setCite(QString &newText)
{
    citeText = newText;
}


void MainWindowWorkflowApp::onRunButtonClicked()
{
    inputWidget->onRunButtonClicked();
}


void MainWindowWorkflowApp::onExitButtonClicked()
{
    inputWidget->onExitButtonClicked();
    QCoreApplication::exit(0);
}
