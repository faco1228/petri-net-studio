/**
 * @file main_window.cpp
 * @brief Implementation of the main application window.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "main_window.h"
#include "app_controller.h"
#include "graphics_editor.h"
#include "monitor_panel.h"
#include "properties_panel.h"
#include "event_log_view.h"
#include "dialogs/new_net_dialog.h"

#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QTabWidget>
#include <QDockWidget>
#include <QCloseEvent>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tabs(nullptr)
    , m_editor(nullptr)
    , m_monitorPanel(nullptr)
    , m_propertiesDock(nullptr)
    , m_propertiesPanel(nullptr)
    , m_logDock(nullptr)
    , m_eventLog(nullptr)
    , m_controller(nullptr)
    , m_modeGroup(nullptr)
    , m_actSelect(nullptr)
    , m_actAddPlace(nullptr)
    , m_actAddTransition(nullptr)
    , m_actAddArc(nullptr)
{
    setWindowTitle("ICP Petri Net Editor");
    resize(1280, 800);

    m_controller = new AppController(this);

    setupCentralWidget();
    setupDocks();
    setupMenuBar();
    setupToolBar();

    restoreWindowGeometry();
}

MainWindow::~MainWindow() {}

void MainWindow::setupCentralWidget()
{
    m_tabs = new QTabWidget(this);

    m_editor       = new GraphicsEditor(m_controller, this);
    m_monitorPanel = new MonitorPanel(m_controller, this);

    m_tabs->addTab(m_editor,       "Editor");
    m_tabs->addTab(m_monitorPanel, "Monitor");

    connect(m_tabs, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // Reload editor scene whenever a net file is loaded
    connect(m_controller, &AppController::netLoaded,
            this, &MainWindow::onNetLoaded);

    setCentralWidget(m_tabs);
}

void MainWindow::setupDocks()
{
    m_propertiesPanel = new PropertiesPanel(this);
    m_propertiesDock  = new QDockWidget("Properties", this);
    m_propertiesDock->setWidget(m_propertiesPanel);
    m_propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);

    m_eventLog = new EventLogView(this);
    m_logDock  = new QDockWidget("Event Log", this);
    m_logDock->setWidget(m_eventLog);
    m_logDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_logDock);
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New",        this, &MainWindow::onNewNet,     QKeySequence::New);
    fileMenu->addAction("&Open...",    this, &MainWindow::onOpenNet,    QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("&Save",       this, &MainWindow::onSaveNet,    QKeySequence::Save);
    fileMenu->addAction("Save &As...", this, &MainWindow::onSaveNetAs);
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit",       this, &QWidget::close,           QKeySequence::Quit);

    QMenu *runMenu = menuBar()->addMenu("&Run");
    runMenu->addAction("&Generate && Run", this, &MainWindow::onGenerateAndRun,   QKeySequence("F5"));
    runMenu->addAction("&Stop",            this, &MainWindow::onStopInterpreter,  QKeySequence("F6"));

    QMenu *viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(m_propertiesDock->toggleViewAction());
    viewMenu->addAction(m_logDock->toggleViewAction());

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About", this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar()
{
    QToolBar *tb = addToolBar("Main");
    tb->addAction("New",             this, &MainWindow::onNewNet);
    tb->addAction("Open",            this, &MainWindow::onOpenNet);
    tb->addAction("Save",            this, &MainWindow::onSaveNet);
    tb->addSeparator();
    tb->addAction("Generate && Run", this, &MainWindow::onGenerateAndRun);
    tb->addAction("Stop",            this, &MainWindow::onStopInterpreter);
    tb->addSeparator();

    // Editor mode buttons - exclusive, like a radio group
    m_modeGroup = new QActionGroup(this);
    m_modeGroup->setExclusive(true);

    m_actSelect          = new QAction("Select",      this);
    m_actAddPlace        = new QAction("Add Place",   this);
    m_actAddTransition   = new QAction("Add Trans.",  this);
    m_actAddArc          = new QAction("Add Arc",     this);

    for (QAction *a : {m_actSelect, m_actAddPlace, m_actAddTransition, m_actAddArc}) {
        a->setCheckable(true);
        m_modeGroup->addAction(a);
        tb->addAction(a);
    }
    m_actSelect->setChecked(true);  // default mode

    connect(m_modeGroup, &QActionGroup::triggered,
            this, &MainWindow::onModeActionTriggered);

    // Sync toolbar buttons when editor changes mode programmatically
    connect(m_editor, &GraphicsEditor::modeChanged, this, [this](EditorMode mode) {
        if (mode == EditorMode::Select)           m_actSelect->setChecked(true);
        else if (mode == EditorMode::AddPlace)    m_actAddPlace->setChecked(true);
        else if (mode == EditorMode::AddTransition) m_actAddTransition->setChecked(true);
        else if (mode == EditorMode::AddArc)      m_actAddArc->setChecked(true);
    });
}

void MainWindow::onNewNet()
{
    NewNetDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    QString name = dlg.netName().isEmpty() ? "NewNet" : dlg.netName();
    m_controller->newNet(name.toStdString());
    m_controller->net()->setComment(dlg.netComment().toStdString());
    m_editor->clearScene();
    updateTitle();
}

void MainWindow::onOpenNet()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Open Petri Net", "", "Petri Net Files (*.pn);;All Files (*)");
    if (path.isEmpty()) return;

    if (!m_controller->loadNet(path.toStdString())) {
        QMessageBox::critical(this, "Open failed",
            QString::fromStdString(m_controller->lastError()));
    }
    // onNetLoaded() will reload the editor scene via signal
}

void MainWindow::onSaveNet()
{
    if (m_controller->currentPath().empty()) {
        onSaveNetAs();
        return;
    }
    if (!m_controller->saveNet()) {
        QMessageBox::critical(this, "Save failed",
            QString::fromStdString(m_controller->lastError()));
    }
}

void MainWindow::onSaveNetAs()
{
    QString path = QFileDialog::getSaveFileName(
        this, "Save Petri Net", "", "Petri Net Files (*.pn);;All Files (*)");
    if (path.isEmpty()) return;

    if (!m_controller->saveNetAs(path.toStdString())) {
        QMessageBox::critical(this, "Save failed",
            QString::fromStdString(m_controller->lastError()));
    } else {
        updateTitle();
    }
}

void MainWindow::onGenerateAndRun()
{
    m_controller->generateAndRun();
    m_tabs->setCurrentIndex(1);
}

void MainWindow::onStopInterpreter()
{
    m_controller->stopInterpreter();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About", "ICP Petri Net Editor\nxfacka00, xlogin02\n2026");
}

void MainWindow::onTabChanged(int index)
{
    // Hide properties panel and mode toolbar in monitor mode
    m_propertiesDock->setVisible(index == 0);
    for (QAction *a : {m_actSelect, m_actAddPlace, m_actAddTransition, m_actAddArc})
        a->setEnabled(index == 0);
}

void MainWindow::onModeActionTriggered(QAction *action)
{
    if (!m_editor) return;

    if (action == m_actSelect)
        m_editor->setMode(EditorMode::Select);
    else if (action == m_actAddPlace)
        m_editor->setMode(EditorMode::AddPlace);
    else if (action == m_actAddTransition)
        m_editor->setMode(EditorMode::AddTransition);
    else if (action == m_actAddArc)
        m_editor->setMode(EditorMode::AddArc);
}

void MainWindow::saveWindowGeometry()
{
    QSettings settings("ICP", "PetriNetEditor");
    settings.setValue("geometry",    saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindow::restoreWindowGeometry()
{
    QSettings settings("ICP", "PetriNetEditor");
    if (settings.contains("geometry"))
        restoreGeometry(settings.value("geometry").toByteArray());
    // Don't restore dock state — it can hide the Properties panel on first run
    m_propertiesDock->show();
}

void MainWindow::onNetLoaded()
{
    m_editor->reloadFromNet(m_controller->net());
    m_tabs->setCurrentIndex(0); // switch to editor tab
    updateTitle();
}

void MainWindow::updateTitle()
{
    const std::string &path = m_controller->currentPath();
    if (path.empty()) {
        setWindowTitle("ICP Petri Net Editor");
    } else {
        // Show just the filename in the title bar
        QString file = QString::fromStdString(path);
        int sep = file.lastIndexOf('/');
        if (sep < 0) sep = file.lastIndexOf('\\');
        setWindowTitle("ICP Petri Net Editor — " + file.mid(sep + 1));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveWindowGeometry();
    event->accept();
}
