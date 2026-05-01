/**
 * @file udp_protocol.h
 * @brief UDP protocol definitions - ports, message types, serialization/deserialization.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <string>
#include <sstream>
#include <vector>

// GUI -> Interpreter
static constexpr int UDP_PORT_INTERPRETER = 7000;
// Interpreter -> GUI
static constexpr int UDP_PORT_GUI = 7001;

static constexpr int UDP_MAX_PAYLOAD = 65507;

enum class MsgType {
    INPUT,
    QUIT,
    STATE,
    LOG,
    ANNOUNCE,
    UNKNOWN
};

struct InputMsg {
    std::string net_name;
    std::string input_name;
    std::string value;
};

struct QuitMsg {
    std::string net_name;
};

struct StateMsg {
    std::string net_name;
    int64_t timestamp_ms;
    std::string marking_json;  // {"P1":2,"P2":0}
    std::string enabled_list;  // "T_on,T_off_start"
    std::string pending_list;  // "T_timeout_off:3842"
    std::string vars_json;     // {"timeout":5000}
};

struct LogMsg {
    std::string net_name;
    int64_t timestamp_ms;
    std::string event_type;  // FIRED, TIMEOUT_FIRED, ...
    std::string details;
};

struct AnnounceMsg {
    std::string net_name;
    int listen_port;
};

// Serialization

inline std::string serializeInput(const InputMsg &m) {
    return "INPUT\t" + m.net_name + "\t" + m.input_name + "\t" + m.value + "\n";
}

inline std::string serializeQuit(const QuitMsg &m) {
    return "QUIT\t" + m.net_name + "\n";
}

inline std::string serializeState(const StateMsg &m) {
    return "STATE\t" + m.net_name + "\t" + std::to_string(m.timestamp_ms)
           + "\t" + m.marking_json + "\t" + m.enabled_list
           + "\t" + m.pending_list + "\t" + m.vars_json + "\n";
}

inline std::string serializeLog(const LogMsg &m) {
    return "LOG\t" + m.net_name + "\t" + std::to_string(m.timestamp_ms)
           + "\t" + m.event_type + "\t" + m.details + "\n";
}

inline std::string serializeAnnounce(const AnnounceMsg &m) {
    return "ANNOUNCE\t" + m.net_name + "\t" + std::to_string(m.listen_port) + "\n";
}

// Deserialization helpers

inline std::vector<std::string> splitTabs(const std::string &s) {
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, '\t'))
        parts.push_back(token);
    return parts;
}

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

inline InputMsg parseInput(const std::vector<std::string> &p) {
    InputMsg m;
    if (p.size() > 1) m.net_name   = p[1];
    if (p.size() > 2) m.input_name = p[2];
    if (p.size() > 3) m.value      = p[3];
    return m;
}

inline StateMsg parseState(const std::vector<std::string> &p) {
    StateMsg m;
    if (p.size() > 1) m.net_name     = p[1];
    if (p.size() > 2) m.timestamp_ms = std::stoll(p[2]);
    if (p.size() > 3) m.marking_json = p[3];
    if (p.size() > 4) m.enabled_list = p[4];
    if (p.size() > 5) m.pending_list = p[5];
    if (p.size() > 6) m.vars_json    = p[6];
    return m;
}

inline LogMsg parseLog(const std::vector<std::string> &p) {
    LogMsg m;
    if (p.size() > 1) m.net_name     = p[1];
    if (p.size() > 2) m.timestamp_ms = std::stoll(p[2]);
    if (p.size() > 3) m.event_type   = p[3];
    if (p.size() > 4) m.details      = p[4];
    return m;
}

inline AnnounceMsg parseAnnounce(const std::vector<std::string> &p) {
    AnnounceMsg m;
    if (p.size() > 1) m.net_name    = p[1];
    if (p.size() > 2) m.listen_port = std::stoi(p[2]);
    return m;
}
