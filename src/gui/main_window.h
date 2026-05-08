/**
 * @file main_window.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Main application window — menu bar, toolbar and tabbed Editor/Monitor layout.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Creates and owns AppController, GraphicsEditor and MonitorPanel
 *   2. Sets up the menu bar (File, Run, View, Help) and the mode toolbar
 *   3. Manages three dock widgets: Properties (right), Event Log (bottom), Inject Input (left)
 *   4. Saves and restores window geometry via QSettings
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QDockWidget>
#include <QActionGroup>
#include "graphics_editor.h"
#include "../inc/udp_protocol.h"

class AppController;
class MonitorPanel;
class PropertiesPanel;
class EventLogView;
class InjectPanel;

/**
 * @brief The top-level application window.
 *
 * Hosts a QTabWidget with two tabs (Editor and Monitor) and three dockable
 * panels.  All user actions are delegated to AppController.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructs and fully initialises the main window.
     * @param parent optional Qt parent (usually nullptr)
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /** @brief Destructor. */
    ~MainWindow();

protected:
    /**
     * @brief Saves window geometry before the window closes.
     * @param event the close event
     */
    void closeEvent(QCloseEvent *event) override;

private slots:
    /** @brief Slot: opens the New Net dialog and creates a fresh net. */
    void onNewNet();

    /** @brief Slot: shows a file-open dialog and loads the chosen .pn file. */
    void onOpenNet();

    /** @brief Slot: saves the current net to its existing path. */
    void onSaveNet();

    /** @brief Slot: shows a file-save dialog and saves to a new path. */
    void onSaveNetAs();

    /** @brief Slot: triggers code generation, compilation and interpreter launch. */
    void onGenerateAndRun();

    /** @brief Slot: stops the running interpreter. */
    void onStopInterpreter();

    /** @brief Slot: shows the About dialog. */
    void onAbout();

    /** @brief Slot: opens the Net Properties (inputs/outputs/variables) dialog. */
    void onNetProperties();

    /** @brief Slot: updates dock visibility when the active tab changes. */
    void onTabChanged(int index);

    /** @brief Slot: synchronises the editor mode with the toolbar action group. */
    void onModeActionTriggered(QAction *action);

    /** @brief Slot: reloads the editor scene and switches to the Editor tab after load. */
    void onNetLoaded();

    /**
     * @brief Slot: called when a running interpreter sends an ANNOUNCE datagram.
     *
     * Tries to auto-load the matching .pn file and switch to Monitor mode.
     * If the file cannot be located, shows a status-bar notification.
     *
     * @param msg the ANNOUNCE message containing the net name and port
     */
    void onAnnounceReceived(const AnnounceMsg &msg);

private:
    /** @brief Builds the menu bar (File, Run, View, Help). */
    void setupMenuBar();

    /** @brief Builds the main toolbar with file actions and editor mode buttons. */
    void setupToolBar();

    /** @brief Creates the central QTabWidget with Editor and Monitor tabs. */
    void setupCentralWidget();

    /** @brief Creates the three dock widgets (Properties, Event Log, Inject Input). */
    void setupDocks();

    /** @brief Persists window geometry and dock state to QSettings. */
    void saveWindowGeometry();

    /** @brief Restores window geometry from QSettings. */
    void restoreWindowGeometry();

    /** @brief Updates the window title to reflect the current file path. */
    void updateTitle();

    QTabWidget    *m_tabs;         ///< central tab widget (Editor / Monitor)
    GraphicsEditor *m_editor;      ///< canvas-based Petri net editor
    MonitorPanel   *m_monitorPanel; ///< live token-count view

    QActionGroup *m_modeGroup;         ///< exclusive group for editor mode buttons
    QAction      *m_actSelect;         ///< Select mode toolbar button
    QAction      *m_actAddPlace;       ///< Add Place mode toolbar button
    QAction      *m_actAddTransition;  ///< Add Transition mode toolbar button
    QAction      *m_actAddArc;         ///< Add Arc mode toolbar button

    QDockWidget    *m_propertiesDock;  ///< right-side properties dock
    PropertiesPanel *m_propertiesPanel; ///< content of the properties dock

    QDockWidget  *m_logDock;    ///< bottom event log dock
    EventLogView *m_eventLog;   ///< content of the log dock

    QDockWidget *m_injectDock  = nullptr; ///< left-side inject dock (Monitor tab only)
    InjectPanel *m_injectPanel = nullptr; ///< content of the inject dock

    AppController *m_controller; ///< central application controller
};

#endif // MAIN_WINDOW_H
