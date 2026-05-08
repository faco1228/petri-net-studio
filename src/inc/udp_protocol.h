/**
 * @file udp_protocol.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief UDP protocol definitions — ports, message types, serialization/deserialization.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Defines the port constants used by both the GUI and the interpreter
 *   2. Declares message-type structs (InputMsg, StateMsg, LogMsg, AnnounceMsg, QuitMsg)
 *   3. Provides inline serialize/deserialize helpers for tab-delimited UDP datagrams
 */

#ifndef UDP_PROTOCOL_H
#define UDP_PROTOCOL_H

#include <string>
#include <sstream>
#include <vector>

/** @brief UDP port the interpreter listens on for commands from the GUI. */
static constexpr int UDP_PORT_INTERPRETER = 7000;

/** @brief UDP port the GUI listens on for state/log messages from the interpreter. */
static constexpr int UDP_PORT_GUI = 7001;

/** @brief Maximum UDP payload size in bytes. */
static constexpr int UDP_MAX_PAYLOAD = 65507;

/**
 * @brief Discriminator for all datagram types exchanged over UDP.
 */
enum class MsgType {
    INPUT,    ///< GUI -> interpreter: inject an input value
    QUIT,     ///< GUI -> interpreter: request graceful shutdown
    STATE,    ///< interpreter -> GUI: current marking snapshot
    LOG,      ///< interpreter -> GUI: single event log entry
    ANNOUNCE, ///< interpreter -> GUI: hello / register with GUI
    UNKNOWN   ///< unrecognised or malformed datagram
};

/**
 * @brief Payload of an INPUT datagram sent by the GUI to inject a value.
 */
struct InputMsg {
    std::string net_name;   ///< target net identifier
    std::string input_name; ///< name of the input variable
    std::string value;      ///< string-encoded value to inject
};

/**
 * @brief Payload of a QUIT datagram sent by the GUI to stop the interpreter.
 */
struct QuitMsg {
    std::string net_name; ///< net whose interpreter should stop
};

/**
 * @brief Payload of a STATE datagram sent periodically by the interpreter.
 */
struct StateMsg {
    std::string net_name;      ///< source net identifier
    int64_t     timestamp_ms;  ///< steady-clock milliseconds since epoch
    std::string marking_json;  ///< JSON object mapping place name to token count, e.g. {"P1":2,"P2":0}
    std::string enabled_list;  ///< comma-separated list of currently enabled transitions
    std::string pending_list;  ///< comma-separated list of pending timed transitions with expiry
    std::string vars_json;     ///< JSON object of variable name-value pairs, e.g. {"timeout":5000}
};

/**
 * @brief Payload of a LOG datagram emitted by the interpreter on each event.
 */
struct LogMsg {
    std::string net_name;    ///< source net identifier
    int64_t     timestamp_ms; ///< event timestamp in milliseconds
    std::string event_type;  ///< e.g. FIRED, TIMEOUT_FIRED, INPUT_RECEIVED
    std::string details;     ///< human-readable event description
};

/**
 * @brief Payload of an ANNOUNCE datagram sent by the interpreter on startup.
 */
struct AnnounceMsg {
    std::string net_name;  ///< identifier of the running net
    int         listen_port; ///< port the interpreter is bound to
};

// ---- Serialization ----

/**
 * @brief Serializes an InputMsg to a tab-delimited UDP datagram string.
 *
 * @param m the message to serialize
 * @return formatted datagram ending with '\n'
 */
inline std::string serializeInput(const InputMsg &m) {
    return "INPUT\t" + m.net_name + "\t" + m.input_name + "\t" + m.value + "\n";
}

/**
 * @brief Serializes a QuitMsg to a tab-delimited UDP datagram string.
 *
 * @param m the message to serialize
 * @return formatted datagram ending with '\n'
 */
inline std::string serializeQuit(const QuitMsg &m) {
    return "QUIT\t" + m.net_name + "\n";
}

/**
 * @brief Serializes a StateMsg to a tab-delimited UDP datagram string.
 *
 * @param m the message to serialize
 * @return formatted datagram ending with '\n'
 */
inline std::string serializeState(const StateMsg &m) {
    return "STATE\t" + m.net_name + "\t" + std::to_string(m.timestamp_ms) + "\t"
         + m.marking_json + "\t" + m.enabled_list + "\t"
         + m.pending_list + "\t" + m.vars_json + "\n";
}

/**
 * @brief Serializes a LogMsg to a tab-delimited UDP datagram string.
 *
 * @param m the message to serialize
 * @return formatted datagram ending with '\n'
 */
inline std::string serializeLog(const LogMsg &m) {
    return "LOG\t" + m.net_name + "\t" + std::to_string(m.timestamp_ms) + "\t"
         + m.event_type + "\t" + m.details + "\n";
}

/**
 * @brief Serializes an AnnounceMsg to a tab-delimited UDP datagram string.
 *
 * @param m the message to serialize
 * @return formatted datagram ending with '\n'
 */
inline std::string serializeAnnounce(const AnnounceMsg &m) {
    return "ANNOUNCE\t" + m.net_name + "\t" + std::to_string(m.listen_port) + "\n";
}

// ---- Deserialization helpers ----

/**
 * @brief Splits a raw datagram string on tab characters.
 *
 * @param s raw datagram
 * @return vector of fields in order
 */
inline std::vector<std::string> splitTabs(const std::string &s) {
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, '\t'))
        parts.push_back(token);
    return parts;
}

/**
 * @brief Identifies the MsgType of a raw datagram by inspecting the first field.
 *
 * @param raw complete datagram string (tabs as delimiters)
 * @return the corresponding MsgType, or MsgType::UNKNOWN
 */
inline MsgType parseMsgType(const std::string &raw) {
    auto parts = splitTabs(raw);
    if (parts.empty()) return MsgType::UNKNOWN;
    if (parts[0] == "INPUT")    return MsgType::INPUT;
    if (parts[0] == "QUIT")     return MsgType::QUIT;
    if (parts[0] == "STATE")    return MsgType::STATE;
    if (parts[0] == "LOG")      return MsgType::LOG;
    if (parts[0] == "ANNOUNCE") return MsgType::ANNOUNCE;
    return MsgType::UNKNOWN;
}

/**
 * @brief Deserializes an InputMsg from pre-split datagram fields.
 *
 * @param p fields produced by splitTabs()
 * @return populated InputMsg (missing fields left empty)
 */
inline InputMsg parseInput(const std::vector<std::string> &p) {
    InputMsg m;
    if (p.size() > 1) m.net_name   = p[1];
    if (p.size() > 2) m.input_name = p[2];
    if (p.size() > 3) m.value      = p[3];
    return m;
}

/**
 * @brief Deserializes a StateMsg from pre-split datagram fields.
 *
 * @param p fields produced by splitTabs()
 * @return populated StateMsg
 */
inline StateMsg parseState(const std::vector<std::string> &p) {
    StateMsg m;
    if (p.size() > 1) m.net_name     = p[1];
    if (p.size() > 2) try { m.timestamp_ms = std::stoll(p[2]); } catch (...) {}
    if (p.size() > 3) m.marking_json = p[3];
    if (p.size() > 4) m.enabled_list = p[4];
    if (p.size() > 5) m.pending_list = p[5];
    if (p.size() > 6) m.vars_json    = p[6];
    return m;
}

/**
 * @brief Deserializes a LogMsg from pre-split datagram fields.
 *
 * @param p fields produced by splitTabs()
 * @return populated LogMsg
 */
inline LogMsg parseLog(const std::vector<std::string> &p) {
    LogMsg m;
    if (p.size() > 1) m.net_name     = p[1];
    if (p.size() > 2) try { m.timestamp_ms = std::stoll(p[2]); } catch (...) {}
    if (p.size() > 3) m.event_type   = p[3];
    if (p.size() > 4) m.details      = p[4];
    return m;
}

/**
 * @brief Deserializes an AnnounceMsg from pre-split datagram fields.
 *
 * @param p fields produced by splitTabs()
 * @return populated AnnounceMsg
 */
inline AnnounceMsg parseAnnounce(const std::vector<std::string> &p) {
    AnnounceMsg m;
    if (p.size() > 1) m.net_name    = p[1];
    if (p.size() > 2) try { m.listen_port = std::stoi(p[2]); } catch (...) {}
    return m;
}

#endif // UDP_PROTOCOL_H
