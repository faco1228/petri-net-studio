/**
 * @file main_window.h
 * @brief Main application window - menu, toolbar, QTabWidget (Editor/Monitor).
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QDockWidget>
#include <QActionGroup>
#include "graphics_editor.h"

class AppController;
class MonitorPanel;
class PropertiesPanel;
class EventLogView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onNewNet();
    void onOpenNet();
    void onSaveNet();
    void onSaveNetAs();
    void onGenerateAndRun();
    void onStopInterpreter();
    void onAbout();

    void onTabChanged(int index);
    void onModeActionTriggered(QAction *action);
    void onNetLoaded();

private:
    void setupMenuBar();
    void setupToolBar();
    void setupCentralWidget();
    void setupDocks();
    void saveWindowGeometry();
    void restoreWindowGeometry();
    void updateTitle();

    QTabWidget *m_tabs;
    GraphicsEditor *m_editor;
    MonitorPanel *m_monitorPanel;

    // Editor mode toolbar actions (exclusive group)
    QActionGroup *m_modeGroup;
    QAction *m_actSelect;
    QAction *m_actAddPlace;
    QAction *m_actAddTransition;
    QAction *m_actAddArc;

    // Properties dock on the right side
    QDockWidget *m_propertiesDock;
    PropertiesPanel *m_propertiesPanel;

    // Event log dock at the bottom, visible from both tabs
    QDockWidget *m_logDock;
    EventLogView *m_eventLog;

    AppController *m_controller;
};
