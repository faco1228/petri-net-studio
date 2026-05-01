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
    tb->addAction("New",              this, &MainWindow::onNewNet);
    tb->addAction("Open",             this, &MainWindow::onOpenNet);
    tb->addAction("Save",             this, &MainWindow::onSaveNet);
    tb->addSeparator();
    tb->addAction("Generate && Run",  this, &MainWindow::onGenerateAndRun);
    tb->addAction("Stop",             this, &MainWindow::onStopInterpreter);
}

void MainWindow::onNewNet()
{
    // TODO: open NewNetDialog, create empty PnNet via controller
}

void MainWindow::onOpenNet()
{
    QString path = QFileDialog::getOpenFileName(this, "Open Petri Net", "", "Petri Net Files (*.pn);;All Files (*)");
    if (path.isEmpty()) return;
    // TODO: m_controller->loadNet(path);
}

void MainWindow::onSaveNet()
{
    // TODO: m_controller->saveNet();
}

void MainWindow::onSaveNetAs()
{
    QString path = QFileDialog::getSaveFileName(this, "Save Petri Net", "", "Petri Net Files (*.pn);;All Files (*)");
    if (path.isEmpty()) return;
    // TODO: m_controller->saveNetAs(path);
}

void MainWindow::onGenerateAndRun()
{
    // TODO: m_controller->generateAndRun();
    m_tabs->setCurrentIndex(1);
}

void MainWindow::onStopInterpreter()
{
    // TODO: m_controller->stopInterpreter();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About", "ICP Petri Net Editor\nxfacka00, xlogin02\n2026");
}

void MainWindow::onTabChanged(int index)
{
    // Hide properties panel in monitor mode - no point editing while running
    m_propertiesDock->setVisible(index == 0);
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
    if (settings.contains("windowState"))
        restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveWindowGeometry();
    event->accept();
}
