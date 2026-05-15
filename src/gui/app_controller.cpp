/**
 * @file app_controller.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of AppController.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor wires QProcess and UdpClient signals
 *   2. Net lifecycle methods delegate to PnFileParser/PnFileWriter
 *   3. Editor operations mutate the net and emit netChanged()
 *   4. generateAndRun() invokes CodeGenerator, compiles with g++ and launches the binary
 *   5. stopInterpreter() sends QUIT via UDP then kills the process if needed
 */

#include "app_controller.h"
#include "../model/pn_net.h"
#include "../model/pn_file_parser.h"
#include "../model/pn_file_writer.h"
#include "../codegen/code_generator.h"
#include "../network/udp_client.h"

#include <QDir>
#include <QMessageBox>
#include <QApplication>

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs the controller, creates UdpClient and QProcess, and wires signals. */
AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_net(std::make_unique<PnNet>())
    , m_placeCounter(1)
    , m_transitionCounter(1)
{
    m_udpClient = new UdpClient(this);
    m_process   = new QProcess(this);

    // Start listening for UDP datagrams immediately so we can detect a
    // running interpreter even before the user clicks Generate & Run
    m_udpClient->start(7001);

    // Connect interpreter process signals
    connect(m_process, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AppController::onProcessFinished);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &AppController::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError,  this, &AppController::onProcessOutput);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Destructor — ensures the interpreter is stopped before destruction. */
AppController::~AppController()
{
    stopInterpreter();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns a non-owning pointer to the current net. */
PnNet* AppController::net() const { return m_net.get(); }

/** @brief Returns the filesystem path of the currently open file. */
const std::string& AppController::currentPath() const { return m_currentPath; }

/** @brief Returns the most recent error message. */
const std::string& AppController::lastError() const { return m_lastError; }

/** @brief Returns true when the interpreter process is active. */
bool AppController::isInterpreterRunning() const {
    return m_process && m_process->state() != QProcess::NotRunning;
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Reads max place/transition ID from the net and updates the auto-increment counters. */
void AppController::syncCounters()
{
    int maxP = 0;
    for (const auto& p : m_net->places())
        if (p->id() > maxP) maxP = p->id();
    int maxT = 0;
    for (const auto& t : m_net->transitions())
        if (t->id() > maxT) maxT = t->id();
    m_placeCounter      = maxP + 1;
    m_transitionCounter = maxT + 1;
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Replaces the current net with a new empty one and emits netChanged(). */
void AppController::newNet(const std::string& name)
{
    m_net = std::make_unique<PnNet>();
    m_net->set_name(name);
    m_currentPath.clear();
    m_placeCounter = m_transitionCounter = 1;
    emit netChanged();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Parses path into a PnNet, replaces the current net, and emits netLoaded(). */
bool AppController::loadNet(const std::string& path)
{
    std::string err;
    auto loaded = PnFileParser::load(path, err);
    if (!loaded) { m_lastError = err; return false; }
    m_net = std::move(loaded);
    m_currentPath = path;
    m_lastError.clear();
    syncCounters(); // keep auto-increment counters ahead of loaded IDs
    emit netLoaded();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Saves the net to its current path (returns false if no path is set). */
bool AppController::saveNet()
{
    if (m_currentPath.empty()) return false;
    std::string err;
    bool ok = PnFileWriter::save(*m_net, m_currentPath, err);
    if (!ok) m_lastError = err;
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Saves the net to a new path and updates currentPath on success. */
bool AppController::saveNetAs(const std::string& path)
{
    std::string err;
    if (!PnFileWriter::save(*m_net, path, err)) { m_lastError = err; return false; }
    m_currentPath = path;
    m_lastError.clear();
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Editor operations

/** @brief Creates a place at pos with an auto-generated name. */
int AppController::addPlace(QPointF pos)
{
    Place* p = m_net->add_place("P" + std::to_string(m_placeCounter++), 0, pos);
    emit netChanged();
    return p ? p->id() : -1;
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Creates a transition at pos with an auto-generated name. */
int AppController::addTransition(QPointF pos)
{
    Transition* t = m_net->add_transition("T" + std::to_string(m_transitionCounter++), pos);
    emit netChanged();
    return t ? t->id() : -1;
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Creates an arc between placeId and transitionId. */
int AppController::addArc(ArcType type, int placeId, int transitionId, int weight)
{
    Arc* a = m_net->add_arc(type, placeId, transitionId, weight);
    emit netChanged();
    return a ? a->id() : -1;
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Removes the place (cascade-deletes connected arcs) and emits netChanged(). */
void AppController::removePlace(int id)      { m_net->remove_place(id);      emit netChanged(); }

/** @brief Removes the transition (cascade-deletes connected arcs) and emits netChanged(). */
void AppController::removeTransition(int id) { m_net->remove_transition(id); emit netChanged(); }

/** @brief Removes the arc and emits netChanged(). */
void AppController::removeArc(int id)        { m_net->remove_arc(id);        emit netChanged(); }

///////////////////////////////////////////////////////////////////////////////

/** @brief Updates the canvas position of a place or transition without emitting netChanged(). */
void AppController::updateItemPos(int id, bool isPlace, QPointF pos)
{
    if (isPlace) { if (Place* p = m_net->find_place_by_id(id)) p->set_pos(pos); }
    else         { if (Transition* t = m_net->find_transition_by_id(id)) t->set_pos(pos); }
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Updates all editable fields of a place and emits netChanged(). */
void AppController::updatePlace(int id, const std::string& name, int tokens, const std::string& action)
{
    if (Place* p = m_net->find_place_by_id(id)) {
        p->set_name(name); p->set_initial_tokens(tokens); p->set_action(action);
        emit netChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Updates all editable fields of a transition and emits netChanged(). */
void AppController::updateTransition(int id, const std::string& name, const std::string& event,
                                     const std::string& guard, const std::string& delay,
                                     const std::string& action)
{
    if (Transition* t = m_net->find_transition_by_id(id)) {
        t->set_name(name); t->set_event_name(event); t->set_guard(guard);
        t->set_delay_expr(delay); t->set_action(action);
        emit netChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Sets a new weight on the arc and emits netChanged(). */
void AppController::updateArcWeight(int id, int weight)
{
    if (Arc* a = m_net->find_arc_by_id(id)) { a->set_weight(weight); emit netChanged(); }
}

///////////////////////////////////////////////////////////////////////////////
// Interpreter lifecycle

/**
 * @brief Generates C++ source, compiles it synchronously and launches the interpreter.
 *
 * Steps:
 *   1. Stop any running interpreter
 *   2. Create a temp directory under /tmp
 *   3. Call CodeGenerator::generate()
 *   4. Compile with g++ -std=c++17 -O2 (waits up to 30 s)
 *   5. Start UdpClient listening on port 7001
 *   6. Launch the interpreter binary with port 7000 as argv[1]
 */
void AppController::generateAndRun()
{
    if (!m_net || m_net->name().empty()) {
        m_lastError = "No net loaded.";
        emit compileOutput("Error: No net loaded.");
        return;
    }

    stopInterpreter();

    // Create the output directory under /tmp
    QString netName = QString::fromStdString(m_net->name());
    m_genDir = "/tmp/icp_" + netName;
    QDir().mkpath(m_genDir);

    // Generate C++ source
    std::string err;
    emit compileOutput("Generating " + m_genDir + "/net_" + netName + ".cpp ...");
    if (!CodeGenerator::generate(*m_net, m_genDir.toStdString(), err)) {
        m_lastError = err;
        emit compileOutput("Generate error: " + QString::fromStdString(err));
        return;
    }

    // Compile synchronously (block up to 30 s)
    QString src  = m_genDir + "/net_" + netName + ".cpp";
    m_binaryPath = m_genDir + "/interpreter_" + netName;

    QStringList args = {"-std=c++17", "-O2", src, "-o", m_binaryPath};
    emit compileOutput("Compiling...");

    QProcess compile;
    compile.start("g++", args);
    compile.waitForFinished(30000);
    QString compOut = compile.readAllStandardOutput() + compile.readAllStandardError();
    if (!compOut.isEmpty()) emit compileOutput(compOut);
    if (compile.exitCode() != 0) {
        m_lastError = "Compile failed";
        emit compileOutput("Compile FAILED.");
        return;
    }
    emit compileOutput("Compile OK.");

    // Start listening for UDP messages before launching the interpreter
    m_udpClient->start(7001);

    // Launch the interpreter
    m_process->start(m_binaryPath, QStringList{"7000"});
    if (!m_process->waitForStarted(3000)) {
        m_lastError = "Failed to start interpreter";
        emit compileOutput("Failed to start interpreter.");
        return;
    }
    emit compileOutput("Interpreter started (PID " + QString::number(m_process->processId()) + ").");
    emit interpreterStarted();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Sends STEP via UDP so the interpreter fires one maximal transition set. */
void AppController::stepInterpreter()
{
    if (isInterpreterRunning())
        m_udpClient->sendStep(7000);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Sends QUIT via UDP, waits 1 s for graceful exit, then kills the process. */
void AppController::stopInterpreter()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        // Claim ownership of the stop sequence so onProcessFinished() skips its cleanup
        m_handledStop = true;
        m_udpClient->sendQuit(m_net ? m_net->name() : "", 7000);
        // waitForFinished processes Qt events — onProcessFinished may fire here
        m_process->waitForFinished(1000);
        if (m_process->state() != QProcess::NotRunning)
            m_process->kill();
    }
    m_udpClient->stop();
    m_handledStop = false;
    emit interpreterStopped();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Slot called when the interpreter process exits; stops the UDP client. */
void AppController::onProcessFinished(int exitCode, QProcess::ExitStatus)
{
    emit compileOutput("Interpreter exited (code " + QString::number(exitCode) + ").");
    // If stopInterpreter() owns this stop sequence it already called stop()/interpreterStopped()
    if (m_handledStop) return;
    m_udpClient->stop();
    emit interpreterStopped();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Reads stdout/stderr from the interpreter and forwards to compileOutput(). */
void AppController::onProcessOutput()
{
    QString out = m_process->readAllStandardOutput()
                + m_process->readAllStandardError();
    if (!out.isEmpty()) emit compileOutput(out.trimmed());
}
