/**
 * @file main_window.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of the main application window.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor assembles the full UI: central widget, docks, menu, toolbar
 *   2. File actions delegate to AppController (new/load/save)
 *   3. Run actions invoke AppController::generateAndRun() / stopInterpreter()
 *   4. Editor mode buttons are exclusive and stay in sync with GraphicsEditor::modeChanged
 *   5. Net Properties dialog updates inputs/outputs/variables in the model
 */

#include "main_window.h"
#include "app_controller.h"
#include "graphics_editor.h"
#include "monitor_panel.h"
#include "properties_panel.h"
#include "event_log_view.h"
#include "inject_panel.h"
#include "dialogs/new_net_dialog.h"
#include "dialogs/variables_dialog.h"

#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QTabWidget>
#include <QDockWidget>
#include <QCloseEvent>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QStatusBar>
#include "../network/udp_client.h"

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs and fully initialises the main window. */
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
    , m_injectPanel(nullptr)
    , m_modeGroup(nullptr)
    , m_actSelect(nullptr)
    , m_actAddPlace(nullptr)
    , m_actAddTransition(nullptr)
    , m_actAddArc(nullptr)
{
    setWindowTitle("ICP Petri Net Editor");
    resize(1280, 800);

    m_controller = new AppController(this);

    // Build UI in the required order (docks before menus so toggleViewAction() works)
    setupCentralWidget();
    setupDocks();
    setupMenuBar();
    setupToolBar();

    restoreWindowGeometry();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Destructor (trivial — Qt parent/child ownership cleans up). */
MainWindow::~MainWindow() {}

///////////////////////////////////////////////////////////////////////////////

/** @brief Creates the central QTabWidget containing the Editor and Monitor tabs. */
void MainWindow::setupCentralWidget()
{
    m_tabs = new QTabWidget(this);

    m_editor       = new GraphicsEditor(m_controller, this);
    m_monitorPanel = new MonitorPanel(m_controller, this);

    m_tabs->addTab(m_editor,       "Editor");
    m_tabs->addTab(m_monitorPanel, "Monitor");

    connect(m_tabs, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // Reload the editor scene whenever a net is opened from disk
    connect(m_controller, &AppController::netLoaded,
            this, &MainWindow::onNetLoaded);

    // Live token counts: forward STATE datagrams to place items in the editor
    connect(m_controller->udpClient(), &UdpClient::stateReceived,
            m_editor, &GraphicsEditor::onStateUpdated);

    // Clear monitor highlight when interpreter stops
    connect(m_controller, &AppController::interpreterStopped,
            m_editor, &GraphicsEditor::clearMonitorHighlight);

    // ANNOUNCE reconnect: wire UdpClient signal so GUI can discover a running net at startup
    connect(m_controller->udpClient(), &UdpClient::announceReceived,
            this, &MainWindow::onAnnounceReceived);

    setCentralWidget(m_tabs);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Creates the Properties, Event Log and Inject Input dock widgets. */
void MainWindow::setupDocks()
{
    // Properties dock (right side)
    m_propertiesPanel = new PropertiesPanel(m_controller, this);
    m_propertiesDock  = new QDockWidget("Properties", this);
    m_propertiesDock->setObjectName("PropertiesDock");
    m_propertiesDock->setWidget(m_propertiesPanel);
    m_propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);
    connect(m_editor, &GraphicsEditor::placeSelected,
            m_propertiesPanel, &PropertiesPanel::showPlace);
    connect(m_editor, &GraphicsEditor::transitionSelected,
            m_propertiesPanel, &PropertiesPanel::showTransition);
    connect(m_editor, &GraphicsEditor::arcSelected,
            m_propertiesPanel, &PropertiesPanel::showArc);
    connect(m_editor, &GraphicsEditor::selectionCleared,
            m_propertiesPanel, &PropertiesPanel::showEmpty);
    connect(m_propertiesPanel, &PropertiesPanel::placeApplied,
            m_editor, &GraphicsEditor::refreshPlaceItem);
    connect(m_propertiesPanel, &PropertiesPanel::transitionApplied,
            m_editor, &GraphicsEditor::refreshTransitionItem);
    connect(m_propertiesPanel, &PropertiesPanel::arcApplied,
            m_editor, &GraphicsEditor::refreshArcItem);

    // Event log dock (bottom)
    m_eventLog = new EventLogView(this);
    m_eventLog->connectController(m_controller);
    m_logDock  = new QDockWidget("Event Log", this);
    m_logDock->setObjectName("EventLogDock");
    m_logDock->setWidget(m_eventLog);
    m_logDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_logDock);

    // Inject input dock (left side, only shown in Monitor tab)
    m_injectPanel = new InjectPanel(m_controller, this);
    m_injectDock  = new QDockWidget("Inject Input", this);
    m_injectDock->setObjectName("InjectInputDock");
    m_injectDock->setWidget(m_injectPanel);
    m_injectDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_injectDock);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Builds the File, Run, View and Help menus. */
void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New",        this, &MainWindow::onNewNet,     QKeySequence::New);
    fileMenu->addAction("&Open...",    this, &MainWindow::onOpenNet,    QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("&Save",             this, &MainWindow::onSaveNet,        QKeySequence::Save);
    fileMenu->addAction("Save &As...",       this, &MainWindow::onSaveNetAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Net &Properties...", this, &MainWindow::onNetProperties);
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit",             this, &QWidget::close, QKeySequence::Quit);

    QMenu *runMenu = menuBar()->addMenu("&Run");
    runMenu->addAction("&Generate && Run", this, &MainWindow::onGenerateAndRun,  QKeySequence("F5"));
    runMenu->addAction("&Stop",            this, &MainWindow::onStopInterpreter, QKeySequence("F6"));

    // View menu lets the user toggle dock visibility
    QMenu *viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(m_propertiesDock->toggleViewAction());
    viewMenu->addAction(m_logDock->toggleViewAction());
    viewMenu->addAction(m_injectDock->toggleViewAction());

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About", this, &MainWindow::onAbout);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Builds the main toolbar with file shortcuts and editor mode buttons. */
void MainWindow::setupToolBar()
{
    QToolBar *tb = addToolBar("Main");
    tb->setObjectName("MainToolBar");
    tb->addAction("New",             this, &MainWindow::onNewNet);
    tb->addAction("Open",            this, &MainWindow::onOpenNet);
    tb->addAction("Save",            this, &MainWindow::onSaveNet);
    tb->addSeparator();
    tb->addAction("Generate && Run", this, &MainWindow::onGenerateAndRun);
    tb->addAction("Stop",            this, &MainWindow::onStopInterpreter);
    tb->addSeparator();

    // Exclusive editor mode buttons (behave like a radio group)
    m_modeGroup = new QActionGroup(this);
    m_modeGroup->setExclusive(true);

    m_actSelect        = new QAction("Select",     this);
    m_actAddPlace      = new QAction("Add Place",  this);
    m_actAddTransition = new QAction("Add Trans.", this);
    m_actAddArc        = new QAction("Add Arc",    this);

    for (QAction *a : {m_actSelect, m_actAddPlace, m_actAddTransition, m_actAddArc}) {
        a->setCheckable(true);
        m_modeGroup->addAction(a);
        tb->addAction(a);
    }
    m_actSelect->setChecked(true); // start in Select mode

    connect(m_modeGroup, &QActionGroup::triggered,
            this, &MainWindow::onModeActionTriggered);

    // Keep toolbar in sync when editor changes mode programmatically (e.g. after add)
    connect(m_editor, &GraphicsEditor::modeChanged, this, [this](EditorMode mode) {
        if      (mode == EditorMode::Select)        m_actSelect->setChecked(true);
        else if (mode == EditorMode::AddPlace)      m_actAddPlace->setChecked(true);
        else if (mode == EditorMode::AddTransition) m_actAddTransition->setChecked(true);
        else if (mode == EditorMode::AddArc)        m_actAddArc->setChecked(true);
    });
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Shows the New Net dialog; on accept, creates the net and clears the scene. */
void MainWindow::onNewNet()
{
    NewNetDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    QString name = dlg.netName().isEmpty() ? "NewNet" : dlg.netName();
    m_controller->newNet(name.toStdString());
    m_controller->net()->set_comment(dlg.netComment().toStdString());
    m_editor->clearScene();
    updateTitle();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Shows a file-open dialog; on accept, loads the .pn file. */
void MainWindow::onOpenNet()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Open Petri Net", "", "Petri Net Files (*.pn);;All Files (*)");
    if (path.isEmpty()) return;

    if (!m_controller->loadNet(path.toStdString())) {
        QMessageBox::critical(this, "Open failed",
            QString::fromStdString(m_controller->lastError()));
    }
    // onNetLoaded() handles the scene reload via signal
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Saves the net; falls back to Save As if no path is set. */
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

///////////////////////////////////////////////////////////////////////////////

/** @brief Shows a file-save dialog and saves to the chosen path. */
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

///////////////////////////////////////////////////////////////////////////////

/** @brief Triggers code generation and compilation, then switches to the Monitor tab. */
void MainWindow::onGenerateAndRun()
{
    m_controller->generateAndRun();
    m_tabs->setCurrentIndex(1); // switch to Monitor tab automatically
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Stops the running interpreter. */
void MainWindow::onStopInterpreter()
{
    m_controller->stopInterpreter();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Opens the Net Properties dialog and applies changes to the model. */
void MainWindow::onNetProperties()
{
    if (!m_controller->net()) return;
    VariablesDialog dlg(m_controller->net(), this);
    if (dlg.exec() != QDialog::Accepted) return;

    PnNet* net = m_controller->net();

    // Replace inputs (clear then re-add from dialog)
    for (const auto& s : net->inputs())  net->remove_input(s);
    for (const auto& s : dlg.inputs())   net->add_input(s);

    // Replace outputs
    for (const auto& s : net->outputs()) net->remove_output(s);
    for (const auto& s : dlg.outputs())  net->add_output(s);

    // Replace variables
    for (const auto& v : net->variables()) net->remove_variable(v.name);
    for (const auto& v : dlg.variables())  net->add_variable(v);

    // Refresh the inject combo so it reflects the updated input list
    if (m_injectPanel) m_injectPanel->refreshInputs();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Shows the About dialog with author and version info. */
void MainWindow::onAbout()
{
    QMessageBox::about(this, "About",
        "ICP Petri Net Editor\nxfackas00, xhanzea00\n2026");
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Shows/hides docks and toolbar buttons depending on the active tab. */
void MainWindow::onTabChanged(int index)
{
    // Properties dock is only useful in the editor tab
    m_propertiesDock->setVisible(index == 0);
    // Inject dock is only useful in the monitor tab
    m_injectDock->setVisible(index == 1);
    // Editor mode buttons make no sense in the monitor tab
    for (QAction *a : {m_actSelect, m_actAddPlace, m_actAddTransition, m_actAddArc})
        a->setEnabled(index == 0);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Translates the triggered toolbar action into an EditorMode change. */
void MainWindow::onModeActionTriggered(QAction *action)
{
    if (!m_editor) return;

    if      (action == m_actSelect)        m_editor->setMode(EditorMode::Select);
    else if (action == m_actAddPlace)      m_editor->setMode(EditorMode::AddPlace);
    else if (action == m_actAddTransition) m_editor->setMode(EditorMode::AddTransition);
    else if (action == m_actAddArc)        m_editor->setMode(EditorMode::AddArc);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Saves geometry and dock state to QSettings. */
void MainWindow::saveWindowGeometry()
{
    QSettings settings("ICP", "PetriNetEditor");
    settings.setValue("geometry",    saveGeometry());
    settings.setValue("windowState", saveState());
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Restores geometry from QSettings; ensures critical docks are in their default state. */
void MainWindow::restoreWindowGeometry()
{
    QSettings settings("ICP", "PetriNetEditor");
    if (settings.contains("geometry"))
        restoreGeometry(settings.value("geometry").toByteArray());
    // Always show properties dock and hide inject dock on startup
    m_propertiesDock->show();
    m_injectDock->hide(); // only visible in the Monitor tab
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Reloads the editor scene from the newly loaded net and switches to Editor tab. */
void MainWindow::onNetLoaded()
{
    m_editor->reloadFromNet(m_controller->net());
    m_tabs->setCurrentIndex(0); // bring the editor into view
    updateTitle();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Sets the window title to include the open file's base name. */
void MainWindow::updateTitle()
{
    const std::string &path = m_controller->currentPath();
    if (path.empty()) {
        setWindowTitle("ICP Petri Net Editor");
    } else {
        // Extract just the filename from the full path
        QString file = QString::fromStdString(path);
        int sep = file.lastIndexOf('/');
        if (sep < 0) sep = file.lastIndexOf('\\');
        setWindowTitle("ICP Petri Net Editor — " + file.mid(sep + 1));
    }
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Saves window geometry before the close event is accepted. */
void MainWindow::closeEvent(QCloseEvent *event)
{
    saveWindowGeometry();
    event->accept();
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Handles an ANNOUNCE datagram from a running interpreter.
 *
 * Tries to auto-load the matching .pn file in this order:
 *   1. Same directory as the currently open file (if any)
 *   2. Current working directory
 *   3. examples/ subdirectory relative to the working directory
 * If a matching file is found it is loaded and the Monitor tab is shown.
 * Otherwise a status-bar message prompts the user to open it manually.
 */
void MainWindow::onAnnounceReceived(const AnnounceMsg &msg)
{
    QString netName  = QString::fromStdString(msg.net_name);
    QString fileName = netName + ".pn";

    // If the matching net is already loaded, just switch to Monitor
    if (m_controller->net() && m_controller->net()->name() == msg.net_name) {
        m_tabs->setCurrentIndex(1);
        statusBar()->showMessage(tr("Connected to running net '%1'.").arg(netName), 4000);
        return;
    }

    // Build candidate paths to search for the .pn file
    QStringList candidates;
    if (!m_controller->currentPath().empty()) {
        QFileInfo fi(QString::fromStdString(m_controller->currentPath()));
        candidates << fi.absolutePath() + "/" + fileName;
    }
    candidates << QDir::currentPath() + "/" + fileName
               << QDir::currentPath() + "/examples/" + fileName;

    for (const QString &path : candidates) {
        if (QFile::exists(path)) {
            if (m_controller->loadNet(path.toStdString())) {
                m_tabs->setCurrentIndex(1);
                statusBar()->showMessage(
                    tr("Auto-connected to running net '%1'.").arg(netName), 4000);
            }
            return;
        }
    }

    // Could not locate the file automatically — prompt the user
    statusBar()->showMessage(
        tr("Running net '%1' detected — use File → Open to load it.").arg(netName), 6000);
}
