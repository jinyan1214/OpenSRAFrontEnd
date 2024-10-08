#include "MainWindowWorkflowApp.h"
#include "sectiontitle.h"
#include "WorkflowAppWidget.h"
#include "HeaderWidget.h"
#include "OpenSRAPreferences.h"
#include "Utils/RelativePathResolver.h"
#include "Utils/dialogabout.h"
#include "Utils/ProgramOutputDialog.h"
#include "Utils/EventFilter.h"

#include <QTreeView>
#include <QDockWidget>
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

    statusWidget = ProgramOutputDialog::getInstance();

    statusDockWidget = new QDockWidget(tr("Program Output"), this);
    statusDockWidget->setContentsMargins(0,0,0,0);

    statusDockWidget->setWidget(statusWidget);

    this->addDockWidget(Qt::BottomDockWidgetArea, statusDockWidget);

    resizeDocks({statusDockWidget}, {30}, Qt::Vertical);

    connect(statusWidget,&ProgramOutputDialog::showDialog,statusDockWidget,&QDockWidget::setVisible);

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

    //    QRect rec = QGuiApplication::primaryScreen()->geometry();
    //    int height = this->height()<int(rec.height())?int(rec.height()):this->height();
    //    int width  = this->width()<int(rec.width())?int(rec.width()):this->width();
    //    this->resize(width, height);


    //
    // add SimCenter Header
    //

    HeaderWidget *header = new HeaderWidget();
    header->setHeadingText(appName);
    layout->addWidget(header);


    // place login info
    layout->addWidget(inputWidget);

    //
    // add run, run-DesignSafe and exit buttons into a new widget for buttons
    //

    // create the buttons widget and a layout for it
    QHBoxLayout *pushButtonLayout = new QHBoxLayout();

    // Create the preprocess button
    preProcessButton = new QPushButton();
    preProcessButton->setText(tr("PREPROCESS"));
    pushButtonLayout->addWidget(preProcessButton);

    // Create run and exit buttons
    runButton = new QPushButton();
    runButton->setText(tr("PERFORM ANALYSIS"));
    pushButtonLayout->addWidget(runButton);

    QPushButton *exitButton = new QPushButton();
    exitButton->setText(tr("Exit"));
    pushButtonLayout->addWidget(exitButton);


    // connect some signals and slots
    connect(runButton, SIGNAL(clicked(bool)),this,SLOT(onRunButtonClicked()));
    connect(preProcessButton, SIGNAL(clicked(bool)),this,SLOT(onPreprocessButtonClicked()));

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
    aboutText = QString(tr("This is a SimCenter Workflow Application"));

    //    aboutTitle = "About this Application"; // this is the title displayed in the on About dialog
    //    aboutSource = ":/Resources/docs/textAbout.html";  // this is an HTML file stored under resources

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

    dialog.exec();

    auto fileToSave = dialog.selectedFiles().constFirst();

    // and save the file
    return saveFile(fileToSave);
}


void MainWindowWorkflowApp::openConfigJson()
{
    auto openDir = this->thePreferences->getLocalWorkDir() + QDir::separator() + "Input";
    QFileInfo openDirStatus(openDir);
    if (!openDirStatus.exists())
        openDir = this->thePreferences->getLocalWorkDir();
    QString fileName = QFileDialog::getOpenFileName(this, "Load SetupConfig.Jjson file (under analysisDir/Input)", openDir,  "Json files (*.json);;All files (*)");
    if (!fileName.isEmpty())
        loadFile(fileName);
}


void MainWindowWorkflowApp::openRunFolder()
{
    auto openDir = this->thePreferences->getLocalWorkDir();
    QDir runDir = QFileDialog::getExistingDirectory(this, "Load from an existing analysis (working) folder", openDir, QFileDialog::ShowDirsOnly);
    // find SetupConfig.json from runDir
    auto configJsonFile = runDir.path() + QDir::separator() + "Input" + QDir::separator() + "SetupConfig.json";
    if (!configJsonFile.isEmpty())
        loadFile(configJsonFile);
    else
        this->sendErrorMessage("Error, could not locate SetupConfig.json under analysisDir/Input; try another analysis folder");
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
    //    setCurrentFile(fileName);

    return true;
}


void MainWindowWorkflowApp::loadFile(const QString &fileName)
{
    //
    // Set current path to dir of fileName for relative pathing used in setup_config.json
    QFileInfo fileInfo(fileName);
    QString absPath = fileInfo.absoluteDir().absolutePath();
    QDir::setCurrent(absPath);
    //

    QString testPath = QDir::currentPath();

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
    //    QFileInfo fileInfo(fileName);
    SCUtils::ResolveAbsolutePaths(jsonObj, fileInfo.dir());

    // close file
    file.close();

    // given the json object, create the C++ objects
    if ( ! (currentFile.isNull() || currentFile.isEmpty()) ) {
        inputWidget->clear();
    }
    inputWidget->inputFromJSON(jsonObj);

    //    setCurrentFile(fileName);
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

    QAction *openRunFolderAction = new QAction(tr("&Open working directory"), this);
    openRunFolderAction->setShortcuts(QKeySequence::Open);
    openRunFolderAction->setStatusTip(tr("Load from an existing analysis folder"));
    connect(openRunFolderAction, &QAction::triggered, this, &MainWindowWorkflowApp::openRunFolder);
    fileMenu->addAction(openRunFolderAction);
    //fileToolBar->addAction(openAction);

    QAction *openConfigJsonAction = new QAction(tr("&Open SetupConfig.json"), this);
    openConfigJsonAction->setShortcuts(QKeySequence::Open);
    openConfigJsonAction->setStatusTip(tr("Open an existing SetupConfig.json file (under workDir/Input)"));
    connect(openConfigJsonAction, &QAction::triggered, this, &MainWindowWorkflowApp::openConfigJson);
    fileMenu->addAction(openConfigJsonAction);
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

    thePreferences = OpenSRAPreferences::getInstance(this);
    QAction *preferenceAction = new QAction(tr("&Preferences"), this);
    preferenceAction->setMenuRole(QAction::ApplicationSpecificRole);
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
}


QPushButton *MainWindowWorkflowApp::getRunButton() const
{
    return runButton;
}


QPushButton *MainWindowWorkflowApp::getPreProcessButton() const
{
    return preProcessButton;
}


void MainWindowWorkflowApp::createHelpMenu()
{
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&Version"), this, &MainWindowWorkflowApp::version);
    helpMenu->addAction(tr("&About"), this, &MainWindowWorkflowApp::about);
    helpMenu->addAction(tr("&Manual"), this, &MainWindowWorkflowApp::manual);
    helpMenu->addAction(tr("&Submit Bug/Feature Request"), this, &MainWindowWorkflowApp::submitFeedback);
    // QAction *submitFeature = helpMenu->addAction(tr("&Submit Bug/Feature Request"), this, &MainWindowWorkflowApp::submitFeatureRequest);
    helpMenu->addAction(tr("&How to Cite"), this, &MainWindowWorkflowApp::cite);
    helpMenu->addAction(tr("&License"), this, &MainWindowWorkflowApp::copyright);
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


void MainWindowWorkflowApp::setAbout(QString &/*newText*/)
{

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
    inputWidget->onRunButtonClicked(runButton);
}


void MainWindowWorkflowApp::onPreprocessButtonClicked()
{
    inputWidget->onPreprocessButtonClicked(preProcessButton);
}


void MainWindowWorkflowApp::onExitButtonClicked()
{
    inputWidget->onExitButtonClicked();
    QCoreApplication::exit(0);
}
