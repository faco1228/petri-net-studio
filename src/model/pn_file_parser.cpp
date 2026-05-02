/**
 * @file pn_file_parser.cpp
 * @brief Implementation of PnFileParser.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "pn_file_parser.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

// ---- Public static entry point ----

std::unique_ptr<PnNet> PnFileParser::load(const std::string &path, std::string &errorMsg)
{
    PnFileParser p;
    return p.parse(path, errorMsg);
}

// ---- Constructor ----

PnFileParser::PnFileParser()
    : m_lineIdx(0)
    , m_net(std::make_unique<PnNet>())
{}

// ---- Main parse routine ----

std::unique_ptr<PnNet> PnFileParser::parse(const std::string &path, std::string &errorMsg)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        errorMsg = "Cannot open file: " + path;
        return nullptr;
    }

    std::string line;
    while (std::getline(file, line))
        m_lines.push_back(line);
    file.close();

    m_lineIdx = 0;

    // Sections must appear in order but are otherwise flexible.
    // Each section lists alternative header prefixes (Czech and Slovak variants).
    struct Section {
        std::vector<std::string> headers; // accepted alternatives
        bool (PnFileParser::*parser)(std::string &);
        bool required;
    };

    std::vector<Section> sections = {
        { {"Jméno sítě:", "Meno siete:"},   &PnFileParser::parseName,       true  },
        { {"Komentář:",   "Komentár:"},      &PnFileParser::parseComment,    false },
        { {"Vstupy:"},                        &PnFileParser::parseInputs,     false },
        { {"Výstupy:"},                       &PnFileParser::parseOutputs,    false },
        { {"Proměnné:",   "Premenné:"},      &PnFileParser::parseVariables,  false },
        { {"Místa",       "Miesta"},          &PnFileParser::parsePlaces,     true  },
        { {"Přechody",    "Prechody"},        &PnFileParser::parseTransitions,true  },
    };

    for (auto &sec : sections) {
        // Seek to a line that starts with any of the accepted headers
        bool found = false;
        while (!atEnd()) {
            std::string stripped = trim(stripComment(currentLine()));
            for (const auto &hdr : sec.headers) {
                if (stripped.find(hdr) == 0) {
                    found = true;
                    break;
                }
            }
            if (found) { advance(); break; }
            advance();
        }

        if (!found) {
            if (sec.required) {
                errorMsg = "Missing required section: " + sec.headers[0];
                return nullptr;
            }
            continue;
        }

        if (!(this->*sec.parser)(errorMsg))
            return nullptr;
    }

    applyAutoLayout();
    return std::move(m_net);
}

// ---- Section parsers ----

bool PnFileParser::parseName(std::string &errorMsg)
{
    while (!atEnd()) {
        std::string line = trim(stripComment(currentLine()));
        if (!line.empty()) {
            m_net->setName(line);
            advance();
            return true;
        }
        advance();
    }
    errorMsg = "Empty network name";
    return false;
}

bool PnFileParser::parseComment(std::string &/*errorMsg*/)
{
    std::string comment;
    while (!atEnd()) {
        std::string raw = stripComment(currentLine());
        std::string line = trim(raw);

        // Stop at next section header (no leading whitespace and ends with ':')
        if (!line.empty() && raw.front() != ' ' && raw.front() != '\t' && line.back() == ':')
            break;

        if (!comment.empty()) comment += '\n';
        comment += line;
        advance();
    }
    m_net->setComment(trim(comment));
    return true;
}

bool PnFileParser::parseInputs(std::string &/*errorMsg*/)
{
    while (!atEnd()) {
        std::string raw = stripComment(currentLine());
        std::string line = trim(raw);

        if (line.empty()) { advance(); continue; }

        // Stop at next section (unindented line ending with ':')
        if (raw.front() != ' ' && raw.front() != '\t' && line.back() == ':')
            break;

        m_net->addInput(line);
        advance();
    }
    return true;
}

bool PnFileParser::parseOutputs(std::string &/*errorMsg*/)
{
    while (!atEnd()) {
        std::string raw = stripComment(currentLine());
        std::string line = trim(raw);

        if (line.empty()) { advance(); continue; }

        if (raw.front() != ' ' && raw.front() != '\t' && line.back() == ':')
            break;

        m_net->addOutput(line);
        advance();
    }
    return true;
}

bool PnFileParser::parseVariables(std::string &/*errorMsg*/)
{
    // Format: "type name = value  # optional comment"
    while (!atEnd()) {
        std::string raw = stripComment(currentLine());
        std::string line = trim(raw);

        if (line.empty()) { advance(); continue; }

        if (raw.front() != ' ' && raw.front() != '\t' && line.back() == ':')
            break;

        // Split on '='
        auto eq = line.find('=');
        if (eq == std::string::npos) { advance(); continue; }

        std::string lhs = trim(line.substr(0, eq));
        std::string rhs = trim(line.substr(eq + 1));

        // lhs is "type name"
        auto space = lhs.rfind(' ');
        if (space == std::string::npos) { advance(); continue; }

        Variable v;
        v.type = trim(lhs.substr(0, space));
        v.name = trim(lhs.substr(space + 1));
        v.value = rhs;
        m_net->addVariable(v);
        advance();
    }
    return true;
}

bool PnFileParser::parsePlaces(std::string &errorMsg)
{
    // Format: NAME (tokens) [pos: x,y] [: { action }]
    static const std::regex placeRe(
        R"(^(\w+)\s*\((\d+)\)(.*)$)"
    );

    while (!atEnd()) {
        std::string raw = stripComment(currentLine());
        std::string line = trim(raw);

        if (line.empty()) { advance(); continue; }

        // Stop at transitions section (Czech or Slovak header)
        if (line.find("Přechody") == 0 || line.find("Prechody") == 0)
            break;

        std::smatch m;
        if (!std::regex_match(line, m, placeRe)) {
            advance();
            continue;
        }

        std::string name = m[1];
        int tokens = std::stoi(m[2]);
        std::string rest = trim(m[3].str());

        double px = -1, py = -1;
        std::string action;

        // Check for optional "pos: x,y"
        if (rest.find("pos:") == 0) {
            std::string posStr = rest.substr(4);
            auto comma = posStr.find(',');
            if (comma != std::string::npos) {
                try {
                    px = std::stod(trim(posStr.substr(0, comma)));
                    py = std::stod(trim(posStr.substr(comma + 1, posStr.find(':') - comma - 1)));
                } catch (...) {}
            }
            auto colon = rest.find(':', 4);
            if (colon != std::string::npos)
                rest = trim(rest.substr(colon + 1));
            else
                rest.clear();
        }

        // Check for action block ": { ... }"
        if (!rest.empty() && rest.front() == ':') {
            rest = trim(rest.substr(1));
            if (!rest.empty() && rest.front() == '{') {
                // Put this back for readBlock to consume
                m_lines[m_lineIdx] = rest;
                if (!readBlock(action, errorMsg))
                    return false;
            }
        } else {
            advance();
        }

        QPointF pos = (px >= 0 && py >= 0) ? QPointF(px, py) : QPointF(-1, -1);
        m_net->addPlace(name, tokens, pos, action);
    }
    return true;
}

bool PnFileParser::parseTransitions(std::string &errorMsg)
{
    // Format:
    // NAME [pos: x,y] :
    //     in:  P1*w, P2*w
    //     out: P3*w
    //     when: [event] [[guard]] [@ delay]
    //     do: { action }

    static const std::regex transRe(R"(^(\w+)(.*):\s*$)");
    static const std::regex arcRe(R"((\w+)(?:\*(\d+))?)");

    while (!atEnd()) {
        std::string raw = stripComment(currentLine());
        std::string line = trim(raw);

        if (line.empty()) { advance(); continue; }

        std::smatch m;
        if (!std::regex_match(line, m, transRe))  { advance(); continue; }

        std::string name = m[1];
        std::string opts = trim(m[2].str());

        double px = -1, py = -1;
        tryParsePos(opts, px, py);

        Transition *t = m_net->addTransition(name,
            (px >= 0 && py >= 0) ? QPointF(px, py) : QPointF(-1, -1));
        advance();

        // Read sub-lines: in, out, when, do
        // Use raw content of CURRENT line (not stale outer `raw`) to detect
        // when we've left the indented block and hit the next transition header.
        while (!atEnd()) {
            std::string rawSub = currentLine();
            std::string sub    = trim(stripComment(rawSub));

            // Empty line means end of this transition's block
            if (sub.empty()) { ++m_lineIdx; break; }

            // Unindented line = next transition header or section header
            if (!rawSub.empty() && rawSub[0] != ' ' && rawSub[0] != '\t')
                break;

            if (sub.find("in:") == 0) {
                std::string arcList = trim(sub.substr(3));
                std::sregex_iterator it(arcList.begin(), arcList.end(), arcRe), end;
                for (; it != end; ++it) {
                    std::string pName  = (*it)[1];
                    int weight = (*it)[2].matched ? std::stoi((*it)[2]) : 1;
                    Place *p = m_net->findPlaceByName(pName);
                    if (!p) { errorMsg = "Unknown place '" + pName + "' in transition '" + name + "'"; return false; }
                    m_net->addArc(ArcType::INPUT, p->getId(), t->getId(), weight);
                }
                advance();

            } else if (sub.find("out:") == 0) {
                std::string arcList = trim(sub.substr(4));
                std::sregex_iterator it(arcList.begin(), arcList.end(), arcRe), end;
                for (; it != end; ++it) {
                    std::string pName  = (*it)[1];
                    int weight = (*it)[2].matched ? std::stoi((*it)[2]) : 1;
                    Place *p = m_net->findPlaceByName(pName);
                    if (!p) { errorMsg = "Unknown place '" + pName + "' in transition '" + name + "'"; return false; }
                    m_net->addArc(ArcType::OUTPUT, p->getId(), t->getId(), weight);
                }
                advance();

            } else if (sub.find("when:") == 0) {
                std::string cond = trim(sub.substr(5));

                // Extract event name (identifier before '[' or '@')
                std::string event, guard, delay;
                size_t i = 0;
                while (i < cond.size() && cond[i] == ' ') ++i;

                size_t start = i;
                while (i < cond.size() && cond[i] != '[' && cond[i] != '@' && cond[i] != ' ')
                    ++i;
                event = trim(cond.substr(start, i - start));

                size_t lb = cond.find('[');
                if (lb != std::string::npos) {
                    size_t rb = cond.rfind(']');
                    if (rb != std::string::npos && rb > lb)
                        guard = trim(cond.substr(lb + 1, rb - lb - 1));
                }

                size_t at = cond.find('@');
                if (at != std::string::npos)
                    delay = trim(cond.substr(at + 1));

                t->setEventName(event);
                t->setGuard(guard);
                t->setDelayExpr(delay);
                advance();

            } else if (sub.find("do:") == 0) {
                std::string rest = trim(sub.substr(3));
                m_lines[m_lineIdx] = rest;
                std::string action;
                if (!readBlock(action, errorMsg)) return false;
                t->setAction(action);

            } else {
                advance();
            }
        }
    }
    return true;
}

// ---- Helpers ----

bool PnFileParser::readBlock(std::string &block, std::string &errorMsg)
{
    // Reads { ... } block, handles multiline and nested braces
    block.clear();
    int depth = 0;
    bool started = false;

    while (!atEnd()) {
        std::string line = currentLine();
        for (char c : line) {
            if (c == '{') {
                depth++;
                started = true;
                if (depth > 1) block += c;
            } else if (c == '}') {
                depth--;
                if (depth == 0) {
                    block = trim(block);
                    advance();
                    return true;
                }
                block += c;
            } else if (started) {
                block += c;
            }
        }
        if (started) block += '\n';
        advance();
    }

    errorMsg = "Unterminated block '{'";
    return false;
}

bool PnFileParser::tryParsePos(const std::string &s, double &x, double &y) const
{
    auto pos = s.find("pos:");
    if (pos == std::string::npos) return false;
    std::string rest = trim(s.substr(pos + 4));
    auto comma = rest.find(',');
    if (comma == std::string::npos) return false;
    try {
        x = std::stod(trim(rest.substr(0, comma)));
        y = std::stod(trim(rest.substr(comma + 1)));
        return true;
    } catch (...) {
        return false;
    }
}

void PnFileParser::applyAutoLayout()
{
    // Assign grid positions to elements that have no explicit pos (pos == -1,-1)
    const double stepX = 150.0;
    const double stepY = 120.0;
    const int cols = 4;

    int idx = 0;
    for (auto &p : m_net->getPlaces()) {
        if (p->getPos().x() < 0) {
            p->setPos(QPointF((idx % cols) * stepX + 80, (idx / cols) * stepY + 80));
            idx++;
        }
    }

    idx = 0;
    for (auto &t : m_net->getTransitions()) {
        if (t->getPos().x() < 0) {
            t->setPos(QPointF((idx % cols) * stepX + 80, (idx / cols) * stepY + 240));
            idx++;
        }
    }
}

std::string PnFileParser::currentLine() const
{
    return m_lines[m_lineIdx];
}

bool PnFileParser::advance()
{
    if (m_lineIdx < (int)m_lines.size())
        m_lineIdx++;
    return !atEnd();
}

bool PnFileParser::atEnd() const
{
    return m_lineIdx >= (int)m_lines.size();
}

std::string PnFileParser::stripComment(const std::string &line) const
{
    // Remove everything from '#' onward, but not inside strings
    bool inString = false;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == '"') inString = !inString;
        if (!inString && line[i] == '#')
            return line.substr(0, i);
    }
    return line;
}

std::string PnFileParser::trim(const std::string &s) const
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}
