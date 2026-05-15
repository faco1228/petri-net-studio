/**
 * @file app_controller.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief AppController — orchestrates net editing, code generation and interpreter lifecycle.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Owns the current PnNet and exposes editor operations (add/remove/update)
 *   2. Drives PnFileParser and PnFileWriter for load/save
 *   3. Invokes CodeGenerator and launches the compiled interpreter as a QProcess
 *   4. Owns a UdpClient for runtime communication with the interpreter
 */

#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <QObject>
#include <QPointF>
#include <QProcess>
#include <memory>
#include "../inc/pn_model.h"
#include "../model/pn_net.h"

class UdpClient;

/**
 * @brief Central controller that connects the GUI, the model and the interpreter.
 *
 * All net mutations go through AppController so that the model stays consistent
 * and the netChanged() signal is always emitted after a change.
 */
class AppController : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the AppController.
     *
     * Creates a fresh empty PnNet, a UdpClient and a QProcess.
     *
     * @param parent optional Qt parent
     */
    explicit AppController(QObject *parent = nullptr);

    /**
     * @brief Destructor — stops the interpreter if it is still running.
     */
    ~AppController();

    // ---- Net access ----

    /**
     * @brief Returns a non-owning pointer to the current net.
     * @return PnNet* (never nullptr after construction)
     */
    PnNet* net() const;

    // ---- Net lifecycle ----

    /**
     * @brief Replaces the current net with a new empty one.
     *
     * Emits netChanged() after the replacement.
     *
     * @param name name for the new net
     */
    void newNet(const std::string &name);

    /**
     * @brief Loads a .pn file and replaces the current net.
     *
     * Emits netLoaded() on success.
     *
     * @param path filesystem path to the .pn file
     * @return true on success; on failure lastError() is set
     */
    bool loadNet(const std::string &path);

    /**
     * @brief Saves the current net to its current path.
     *
     * @return false if no path is set or write fails
     */
    bool saveNet();

    /**
     * @brief Saves the current net to a new path.
     *
     * Updates the current path on success.
     *
     * @param path target filesystem path
     * @return true on success; on failure lastError() is set
     */
    bool saveNetAs(const std::string &path);

    // ---- Editor operations ----

    /**
     * @brief Adds a new place at the given canvas position.
     *
     * @param pos scene-coordinate centre of the new place
     * @return ID of the created place, or -1 on failure
     */
    int addPlace(QPointF pos);

    /**
     * @brief Adds a new transition at the given canvas position.
     *
     * @param pos scene-coordinate centre of the new transition
     * @return ID of the created transition, or -1 on failure
     */
    int addTransition(QPointF pos);

    /**
     * @brief Adds an arc between an existing place and transition.
     *
     * @param type         INPUT or OUTPUT
     * @param placeId      ID of the connected place
     * @param transitionId ID of the connected transition
     * @param weight       arc weight (default 1)
     * @return ID of the created arc, or -1 on failure
     */
    int addArc(ArcType type, int placeId, int transitionId, int weight = 1);

    /**
     * @brief Removes the place with the given ID (cascade-deletes its arcs).
     * @param id place ID
     */
    void removePlace(int id);

    /**
     * @brief Removes the transition with the given ID (cascade-deletes its arcs).
     * @param id transition ID
     */
    void removeTransition(int id);

    /**
     * @brief Removes the arc with the given ID.
     * @param id arc ID
     */
    void removeArc(int id);

    /**
     * @brief Updates the canvas position of a place or transition.
     *
     * @param id      element ID
     * @param isPlace true for a place, false for a transition
     * @param pos     new scene-coordinate position
     */
    void updateItemPos(int id, bool isPlace, QPointF pos);

    // ---- Property editing ----

    /**
     * @brief Updates the properties of an existing place.
     *
     * @param id      place ID
     * @param name    new label
     * @param tokens  new initial token count
     * @param action  new C code action (empty to clear)
     */
    void updatePlace(int id, const std::string &name, int tokens, const std::string &action);

    /**
     * @brief Updates the properties of an existing transition.
     *
     * @param id      transition ID
     * @param name    new label
     * @param event   new triggering event name (empty to clear)
     * @param guard   new C++ guard expression (empty to clear)
     * @param delay   new delay expression (empty to clear)
     * @param action  new C++ action code (empty to clear)
     */
    void updateTransition(int id, const std::string &name, const std::string &event,
                          const std::string &guard, const std::string &delay,
                          const std::string &action);

    /**
     * @brief Updates the weight of an existing arc.
     *
     * @param id     arc ID
     * @param weight new weight (>= 1)
     */
    void updateArcWeight(int id, int weight);

    // ---- Interpreter lifecycle ----

    /**
     * @brief Generates the C++ interpreter, compiles it and launches the binary.
     *
     * Progress and error messages are emitted via compileOutput().
     * Emits interpreterStarted() on successful launch.
     */
    void generateAndRun();

    /**
     * @brief Sends a QUIT UDP message and kills the interpreter process if needed.
     *
     * Emits interpreterStopped().
     */
    void stopInterpreter();

    /**
     * @brief Sends a STEP UDP message to fire one maximal transition set.
     *
     * Only has effect when the interpreter is running.
     */
    void stepInterpreter();

    // ---- Accessors ----

    /**
     * @brief Returns the UDP client owned by this controller.
     * @return non-owning pointer to UdpClient
     */
    UdpClient* udpClient() const { return m_udpClient; }

    /**
     * @brief Returns the filesystem path of the currently open file.
     * @return empty string if no file has been saved/loaded
     */
    const std::string& currentPath() const;

    /**
     * @brief Returns the most recent error message.
     * @return human-readable error string (empty if no error)
     */
    const std::string& lastError() const;

    /**
     * @brief Returns true if the interpreter process is currently running.
     */
    bool isInterpreterRunning() const;

signals:
    /** @brief Emitted after any structural change to the net (add/remove/update). */
    void netChanged();

    /** @brief Emitted after a net is successfully loaded from disk. */
    void netLoaded();

    /** @brief Emitted when the interpreter process has been launched. */
    void interpreterStarted();

    /** @brief Emitted when the interpreter process has exited or been stopped. */
    void interpreterStopped();

    /** @brief Emitted with stdout/stderr lines from the compiler or interpreter. */
    void compileOutput(const QString& line);

private slots:
    /** @brief Called when the interpreter process exits. */
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

    /** @brief Reads stdout/stderr from the interpreter and emits compileOutput(). */
    void onProcessOutput();

private:
    /** @brief Synchronises m_placeCounter and m_transitionCounter from the loaded net. */
    void syncCounters();

    std::unique_ptr<PnNet> m_net;          ///< currently active Petri net
    std::string            m_currentPath;  ///< path of the last saved/loaded file
    std::string            m_lastError;    ///< most recent error description
    int                    m_placeCounter;      ///< next auto-name counter for places
    int                    m_transitionCounter; ///< next auto-name counter for transitions

    QProcess*  m_process   = nullptr; ///< interpreter child process
    UdpClient* m_udpClient = nullptr; ///< UDP communication channel
    QString    m_genDir;              ///< directory where generated files are written
    QString    m_binaryPath;          ///< path to the compiled interpreter binary
    bool       m_handledStop = false; ///< true while stopInterpreter() owns the stop sequence
};

#endif // APP_CONTROLLER_H
