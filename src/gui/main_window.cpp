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
    , m_editor(nullptr)
    , m_monitorPanel(nullptr)
    , m_modeGroup(nullptr)
    , m_actSelect(nullptr)
    , m_actAddPlace(nullptr)
    , m_actAddTransition(nullptr)
    , m_actAddArc(nullptr)
    , m_actRun(nullptr)
    , m_actStop(nullptr)
    , m_actStep(nullptr)
    , m_actAuto(nullptr)
    , m_autoTimer(nullptr)
    , m_propertiesDock(nullptr)
    , m_propertiesPanel(nullptr)
    , m_logDock(nullptr)
    , m_eventLog(nullptr)
    , m_injectPanel(nullptr)
    , m_controller(nullptr)
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

/** @brief Sets GraphicsEditor as the central widget and wires all controller signals. */
void MainWindow::setupCentralWidget()
{
    m_editor       = new GraphicsEditor(m_controller, this);
    m_monitorPanel = new MonitorPanel(m_controller, this);

    // Editor is the only central widget — always visible
    setCentralWidget(m_editor);

    // Reload the editor scene whenever a net is opened from disk
    connect(m_controller, &AppController::netLoaded,   this, &MainWindow::onNetLoaded);

    // Live token/transition colours in diagram
    connect(m_controller->udpClient(), &UdpClient::stateReceived,
            m_editor, &GraphicsEditor::onStateUpdated);

    // Clear highlight when interpreter stops
    connect(m_controller, &AppController::interpreterStopped,
            m_editor, &GraphicsEditor::clearMonitorHighlight);

    // Toolbar button state
    connect(m_controller, &AppController::interpreterStarted,
            this, &MainWindow::onInterpreterStarted);
    connect(m_controller, &AppController::interpreterStopped,
            this, &MainWindow::onInterpreterStopped);

    // ANNOUNCE reconnect
    connect(m_controller->udpClient(), &UdpClient::announceReceived,
            this, &MainWindow::onAnnounceReceived);
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

    // Inject input dock (left side — shown only when interpreter is running)
    m_injectPanel = new InjectPanel(m_controller, this);
    m_injectDock  = new QDockWidget("Inject Input", this);
    m_injectDock->setObjectName("InjectInputDock");
    m_injectDock->setWidget(m_injectPanel);
    m_injectDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_injectDock);
    m_injectDock->hide();

    // Monitor dock (right side — shown only when interpreter is running)
    m_monitorDock = new QDockWidget("Monitor", this);
    m_monitorDock->setObjectName("MonitorDock");
    m_monitorDock->setWidget(m_monitorPanel);
    m_monitorDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_monitorDock);
    m_monitorDock->hide();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Builds the File, Run, View and Help menus. */
void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New",        QKeySequence::New,  this, &MainWindow::onNewNet);
    fileMenu->addAction("&Open...",    QKeySequence::Open, this, &MainWindow::onOpenNet);
    fileMenu->addSeparator();
    fileMenu->addAction("&Save",              QKeySequence::Save, this, &MainWindow::onSaveNet);
    fileMenu->addAction("Save &As...",        this, &MainWindow::onSaveNetAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Net &Properties...", this, &MainWindow::onNetProperties);
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit",              QKeySequence::Quit, this, &QWidget::close);

    QMenu *runMenu = menuBar()->addMenu("&Run");
    runMenu->addAction("&Run",  QKeySequence("F5"), this, &MainWindow::onGenerateAndRun);
    runMenu->addAction("&Stop", QKeySequence("F6"), this, &MainWindow::onStopInterpreter);
    runMenu->addAction("S&tep", QKeySequence("F7"), this, &MainWindow::onStepInterpreter);

    // View menu lets the user toggle dock visibility
    QMenu *viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(m_propertiesDock->toggleViewAction());
    viewMenu->addAction(m_monitorDock->toggleViewAction());
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
    tb->addAction("New",  this, &MainWindow::onNewNet);
    tb->addAction("Open", this, &MainWindow::onOpenNet);
    tb->addAction("Save", this, &MainWindow::onSaveNet);
    tb->addSeparator();

    // Interpreter control buttons
    m_actRun  = new QAction("▶ Run",  this);
    m_actStop = new QAction("■ Stop", this);
    m_actStep = new QAction("⏭ Step", this);
    m_actAuto = new QAction("▶▶ Auto", this);
    m_actAuto->setCheckable(true);
    m_actStop->setEnabled(false);
    m_actStep->setEnabled(false);
    m_actAuto->setEnabled(false);
    connect(m_actRun,  &QAction::triggered, this, &MainWindow::onGenerateAndRun);
    connect(m_actStop, &QAction::triggered, this, &MainWindow::onStopInterpreter);
    connect(m_actStep, &QAction::triggered, this, &MainWindow::onStepInterpreter);
    connect(m_actAuto, &QAction::triggered, this, &MainWindow::onToggleAuto);
    tb->addAction(m_actRun);
    tb->addAction(m_actStop);
    tb->addAction(m_actStep);
    tb->addAction(m_actAuto);

    m_autoTimer = new QTimer(this);
    m_autoTimer->setInterval(200);
    connect(m_autoTimer, &QTimer::timeout, this, &MainWindow::onStepInterpreter);
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

/** @brief Triggers code generation and compilation; diagram stays visible. */
void MainWindow::onGenerateAndRun()
{
    m_controller->generateAndRun();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Stops the running interpreter. */
void MainWindow::onStopInterpreter()
{
    m_controller->stopInterpreter();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Fires one step in the interpreter. */
void MainWindow::onStepInterpreter()
{
    m_controller->stepInterpreter();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Shows monitor/inject docks and updates button states when interpreter starts. */
void MainWindow::onInterpreterStarted()
{
    m_actRun->setEnabled(false);
    m_actStop->setEnabled(true);
    m_actStep->setEnabled(true);
    m_actAuto->setEnabled(true);
    m_monitorDock->show();
    m_injectDock->show();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Hides monitor/inject docks and updates button states when interpreter stops. */
void MainWindow::onInterpreterStopped()
{
    m_autoTimer->stop();
    m_actAuto->setChecked(false);
    m_actRun->setEnabled(true);
    m_actStop->setEnabled(false);
    m_actStep->setEnabled(false);
    m_actAuto->setEnabled(false);
    m_monitorDock->hide();
    m_injectDock->hide();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Toggles continuous auto-stepping on/off. */
void MainWindow::onToggleAuto()
{
    if (m_actAuto->isChecked()) {
        m_actStep->setEnabled(false);
        m_autoTimer->start();
    } else {
        m_autoTimer->stop();
        m_actStep->setEnabled(true);
    }
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
    m_propertiesDock->show();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Reloads the editor scene from the newly loaded net. */
void MainWindow::onNetLoaded()
{
    m_editor->reloadFromNet(m_controller->net());
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

    // If the matching net is already loaded, just show a status message
    if (m_controller->net() && m_controller->net()->name() == msg.net_name) {
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
