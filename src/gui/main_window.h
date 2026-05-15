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
#include <QTimer>
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
    void onNewNet();
    void onOpenNet();
    void onSaveNet();
    void onSaveNetAs();
    void onGenerateAndRun();
    void onStopInterpreter();
    void onStepInterpreter();
    void onToggleAuto();
    void onAbout();
    void onNetProperties();
    void onModeActionTriggered(QAction *action);
    void onNetLoaded();
    void onAnnounceReceived(const AnnounceMsg &msg);
    /** @brief Updates Run/Stop/Step button states when interpreter starts or stops. */
    void onInterpreterStarted();
    void onInterpreterStopped();

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

    GraphicsEditor *m_editor;      ///< canvas-based Petri net editor (central widget)
    MonitorPanel   *m_monitorPanel; ///< live token/transition state sidebar

    QActionGroup *m_modeGroup;         ///< exclusive group for editor mode buttons
    QAction      *m_actSelect;         ///< Select mode toolbar button
    QAction      *m_actAddPlace;       ///< Add Place mode toolbar button
    QAction      *m_actAddTransition;  ///< Add Transition mode toolbar button
    QAction      *m_actAddArc;         ///< Add Arc mode toolbar button
    QAction      *m_actRun;            ///< Run toolbar button
    QAction      *m_actStop;           ///< Stop toolbar button
    QAction      *m_actStep;           ///< Step toolbar button
    QAction      *m_actAuto;           ///< Auto (continuous) toolbar button
    QTimer       *m_autoTimer;         ///< fires stepInterpreter() repeatedly in Auto mode

    QDockWidget    *m_propertiesDock;  ///< right-side properties dock
    PropertiesPanel *m_propertiesPanel; ///< content of the properties dock

    QDockWidget    *m_monitorDock = nullptr; ///< right-side monitor dock (shown when running)

    QDockWidget  *m_logDock;    ///< bottom event log dock
    EventLogView *m_eventLog;   ///< content of the log dock

    QDockWidget *m_injectDock  = nullptr; ///< left-side inject dock
    InjectPanel *m_injectPanel = nullptr; ///< content of the inject dock

    AppController *m_controller; ///< central application controller
};

#endif // MAIN_WINDOW_H
