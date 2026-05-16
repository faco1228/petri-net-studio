/**
 * @file pn_file_parser.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of PnFileParser.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. load() creates a parser instance and delegates to parse()
 *   2. parse() reads the file, identifies each section header, and dispatches
 *      to the appropriate section parser
 *   3. Section parsers consume indented content until the next unindented header
 *   4. readBlock() accumulates a balanced { ... } block potentially spanning
 *      multiple lines
 *   5. applyAutoLayout() fills in missing positions with a uniform grid
 */

#include "pn_file_parser.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

///////////////////////////////////////////////////////////////////////////////
// Public static entry point

/** @brief Creates a temporary parser instance and runs the full parse pipeline. */
std::unique_ptr<PnNet> PnFileParser::load(const std::string &path, std::string &errorMsg)
{
    PnFileParser p;
    return p.parse(path, errorMsg);
}

///////////////////////////////////////////////////////////////////////////////
// Constructor

/** @brief Initialises line_idx_ to 0 and creates a fresh PnNet. */
PnFileParser::PnFileParser()
    : line_idx_(0)
    , net_(std::make_unique<PnNet>())
{}

///////////////////////////////////////////////////////////////////////////////
// Main parse routine

/**
 * @brief Reads the file into lines_ then drives each section parser in order.
 *
 * Sections can appear with Czech or Slovak header variants.  Required sections
 * (Name, Places, Transitions) return nullptr if missing.
 */
std::unique_ptr<PnNet> PnFileParser::parse(const std::string &path, std::string &errorMsg)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        errorMsg = "Cannot open file: " + path;
        return nullptr;
    }

    // Read all lines upfront
    std::string line;
    while (std::getline(file, line))
        lines_.push_back(line);
    file.close();

    line_idx_ = 0;

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

    // Flat list of all known section headers used to detect fence lines
    std::vector<std::string> allHeaders;
    for (auto &s : sections)
        for (auto &h : s.headers)
            allHeaders.push_back(h);

    for (auto &sec : sections) {
        // Seek forward to a line that starts with any of the accepted headers.
        // Stop early if we hit any OTHER known section header (fence) so that
        // optional sections never consume lines belonging to later sections.
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
            // If this line is any other section header, stop seeking
            bool fence = false;
            for (const auto &h : allHeaders) {
                if (stripped.find(h) == 0) { fence = true; break; }
            }
            if (fence) break;
            advance();
        }

        if (!found) {
            if (sec.required) {
                errorMsg = "Missing required section: " + sec.headers[0];
                return nullptr;
            }
            continue; // optional section absent — skip
        }

        // Dispatch to the section parser
        if (!(this->*sec.parser)(errorMsg))
            return nullptr;
    }

    applyAutoLayout();
    return std::move(net_);
}

///////////////////////////////////////////////////////////////////////////////
// Section parsers

/** @brief Reads the first non-empty content line after the name header. */
bool PnFileParser::parseName(std::string &errorMsg)
{
    while (!atEnd()) {
        std::string line = trim(stripComment(currentLine()));
        if (!line.empty()) {
            net_->set_name(line);
            advance();
            return true;
        }
        advance();
    }
    errorMsg = "Empty network name";
    return false;
}

/**
 * @brief Reads indented lines as free-form comment text.
 *
 * Stops when an unindented line ending with ':' is encountered (next section).
 */
bool PnFileParser::parseComment(std::string &/*errorMsg*/)
{
    std::string comment;
    while (!atEnd()) {
        std::string raw  = stripComment(currentLine());
        std::string line = trim(raw);

        // Stop at next section header: unindented and ends with ':'
        if (!line.empty() && raw.front() != ' ' && raw.front() != '\t' && line.back() == ':')
            break;

        if (!comment.empty()) comment += '\n';
        comment += line;
        advance();
    }
    net_->set_comment(trim(comment));
    return true;
}

/**
 * @brief Reads one input name per indented line until the next section header.
 */
bool PnFileParser::parseInputs(std::string &/*errorMsg*/)
{
    while (!atEnd()) {
        std::string raw  = stripComment(currentLine());
        std::string line = trim(raw);

        if (line.empty()) { advance(); continue; }

        // Stop at next section (unindented line ending with ':')
        if (raw.front() != ' ' && raw.front() != '\t' && line.back() == ':')
            break;

        net_->add_input(line);
        advance();
    }
    return true;
}

/**
 * @brief Reads one output name per indented line until the next section header.
 */
bool PnFileParser::parseOutputs(std::string &/*errorMsg*/)
{
    while (!atEnd()) {
        std::string raw  = stripComment(currentLine());
        std::string line = trim(raw);

        if (line.empty()) { advance(); continue; }

        if (raw.front() != ' ' && raw.front() != '\t' && line.back() == ':')
            break;

        net_->add_output(line);
        advance();
    }
    return true;
}

/**
 * @brief Parses "type name = value" variable declarations.
 *
 * Format: one declaration per indented line, optionally with a trailing '#' comment.
 */
bool PnFileParser::parseVariables(std::string &/*errorMsg*/)
{
    while (!atEnd()) {
        std::string raw  = stripComment(currentLine());
        std::string line = trim(raw);

        if (line.empty()) { advance(); continue; }

        if (raw.front() != ' ' && raw.front() != '\t' && line.back() == ':')
            break;

        // Split on '=' to separate lhs (type name) from rhs (value)
        auto eq = line.find('=');
        if (eq == std::string::npos) { advance(); continue; }

        std::string lhs = trim(line.substr(0, eq));
        std::string rhs = trim(line.substr(eq + 1));

        // lhs is "type name" — split on the last space
        auto space = lhs.rfind(' ');
        if (space == std::string::npos) { advance(); continue; }

        Variable v;
        v.type  = trim(lhs.substr(0, space));
        v.name  = trim(lhs.substr(space + 1));
        v.value = rhs;
        net_->add_variable(v);
        advance();
    }
    return true;
}

/**
 * @brief Parses place declarations: NAME (tokens) [pos: x,y] [: { action }].
 *
 * Stops at the transitions section header.
 */
bool PnFileParser::parsePlaces(std::string &errorMsg)
{
    // Match: identifier '(' digit+ ')' optional-rest
    static const std::regex placeRe(R"(^(\w+)\s*\((\d+)\)(.*)$)");

    while (!atEnd()) {
        std::string raw  = stripComment(currentLine());
        std::string line = trim(raw);

        if (line.empty()) { advance(); continue; }

        // Stop at the transitions section header
        if (line.find("Přechody") == 0 || line.find("Prechody") == 0)
            break;

        std::smatch m;
        if (!std::regex_match(line, m, placeRe)) {
            advance();
            continue;
        }

        std::string name   = m[1];
        int         tokens = std::stoi(m[2]);
        std::string rest   = trim(m[3].str());

        double px = -1, py = -1;
        std::string action;

        // Parse optional "pos: x,y"
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

        // Parse optional action block ": { ... }"
        if (!rest.empty() && rest.front() == ':') {
            rest = trim(rest.substr(1));
            if (!rest.empty() && rest.front() == '{') {
                // Reuse the current line buffer so readBlock can consume the '{'
                lines_[line_idx_] = rest;
                if (!readBlock(action, errorMsg))
                    return false;
            }
        } else {
            advance();
        }

        QPointF pos = (px >= 0 && py >= 0) ? QPointF(px, py) : QPointF(-1, -1);
        net_->add_place(name, tokens, pos, action);
    }
    return true;
}

/**
 * @brief Parses transition declarations including sub-lines in:, out:, when:, do:.
 *
 * Format per transition:
 *   NAME [pos: x,y] :\n
 *       in:  P1*w, P2\n
 *       out: P3*w\n
 *       when: [event] [[guard]] [@ delay]\n
 *       do: { action }\n
 */
bool PnFileParser::parseTransitions(std::string &errorMsg)
{
    static const std::regex transRe(R"(^(\w+)(.*):\s*$)");
    static const std::regex arcRe(R"((\w+)(?:\*(\d+))?)");

    while (!atEnd()) {
        std::string raw  = stripComment(currentLine());
        std::string line = trim(raw);

        if (line.empty()) { advance(); continue; }

        std::smatch m;
        if (!std::regex_match(line, m, transRe)) { advance(); continue; }

        std::string name = m[1];
        std::string opts = trim(m[2].str());

        // Parse optional canvas position from the header line
        double px = -1, py = -1;
        tryParsePos(opts, px, py);

        Transition *t = net_->add_transition(name,
            (px >= 0 && py >= 0) ? QPointF(px, py) : QPointF(-1, -1));
        advance();

        // Read indented sub-lines: in, out, when, do
        while (!atEnd()) {
            std::string rawSub = currentLine();
            std::string sub    = trim(stripComment(rawSub));

            // Empty line terminates this transition's indented block
            if (sub.empty()) { ++line_idx_; break; }

            // Unindented line = start of next transition or section
            if (!rawSub.empty() && rawSub[0] != ' ' && rawSub[0] != '\t')
                break;

            if (sub.find("in:") == 0) {
                // Parse comma-separated arc list "P1*w, P2"
                std::string arcList = trim(sub.substr(3));
                std::sregex_iterator it(arcList.begin(), arcList.end(), arcRe), end;
                for (; it != end; ++it) {
                    std::string pName = (*it)[1];
                    int weight = (*it)[2].matched ? std::stoi((*it)[2]) : 1;
                    Place *p = net_->find_place_by_name(pName);
                    if (!p) { errorMsg = "Unknown place '" + pName + "' in transition '" + name + "'"; return false; }
                    net_->add_arc(ArcType::INPUT, p->id(), t->id(), weight);
                }
                advance();

            } else if (sub.find("out:") == 0) {
                std::string arcList = trim(sub.substr(4));
                std::sregex_iterator it(arcList.begin(), arcList.end(), arcRe), end;
                for (; it != end; ++it) {
                    std::string pName = (*it)[1];
                    int weight = (*it)[2].matched ? std::stoi((*it)[2]) : 1;
                    Place *p = net_->find_place_by_name(pName);
                    if (!p) { errorMsg = "Unknown place '" + pName + "' in transition '" + name + "'"; return false; }
                    net_->add_arc(ArcType::OUTPUT, p->id(), t->id(), weight);
                }
                advance();

            } else if (sub.find("when:") == 0) {
                // Parse "when: [event] [[guard]] [@ delay]"
                std::string cond = trim(sub.substr(5));
                std::string event, guard, delay;
                size_t i = 0;

                // Skip leading spaces
                while (i < cond.size() && cond[i] == ' ') ++i;

                // Collect identifier before '[' or '@'
                size_t start = i;
                while (i < cond.size() && cond[i] != '[' && cond[i] != '@' && cond[i] != ' ')
                    ++i;
                event = trim(cond.substr(start, i - start));

                // Extract guard: [ ... ]
                size_t lb = cond.find('[');
                if (lb != std::string::npos) {
                    size_t rb = cond.rfind(']');
                    if (rb != std::string::npos && rb > lb)
                        guard = trim(cond.substr(lb + 1, rb - lb - 1));
                }

                // Extract delay after '@'
                size_t at = cond.find('@');
                if (at != std::string::npos)
                    delay = trim(cond.substr(at + 1));

                t->set_event_name(event);
                t->set_guard(guard);
                t->set_delay_expr(delay);
                advance();

            } else if (sub.find("do:") == 0) {
                // Read the action block { ... }
                std::string rest = trim(sub.substr(3));
                lines_[line_idx_] = rest;
                std::string action;
                if (!readBlock(action, errorMsg)) return false;
                t->set_action(action);

            } else {
                advance();
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Helpers

/**
 * @brief Consumes a balanced { ... } block from the current read position.
 *
 * Handles multi-line blocks and nested braces.  Strips the outermost braces
 * and trims whitespace from the captured content.
 */
bool PnFileParser::readBlock(std::string &block, std::string &errorMsg)
{
    block.clear();
    int  depth   = 0;
    bool started = false;

    while (!atEnd()) {
        std::string line = currentLine();
        for (char c : line) {
            if (c == '{') {
                depth++;
                started = true;
                if (depth > 1) block += c; // keep nested braces
            } else if (c == '}') {
                depth--;
                if (depth == 0) {
                    // Closing brace of the outermost block — done
                    block = trim(block);
                    advance();
                    return true;
                }
                block += c;
            } else if (started) {
                block += c;
            }
        }
        if (started) block += '\n'; // preserve line breaks inside block
        advance();
    }

    errorMsg = "Unterminated block '{'";
    return false;
}

/**
 * @brief Looks for "pos: x,y" in the string s and parses the coordinates.
 */
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

/**
 * @brief Assigns grid positions to all places and transitions with pos == (-1, -1).
 *
 * Places are laid out first, then transitions, each on their own grid rows
 * with 4 columns and 150 px horizontal / 120 px vertical spacing.
 */
void PnFileParser::applyAutoLayout()
{
    const double stepX = 150.0;
    const double stepY = 120.0;
    const int    cols  = 4;

    int idx = 0;
    for (auto &p : net_->places()) {
        if (p->pos().x() < 0) {
            p->set_pos(QPointF((idx % cols) * stepX + 80, (idx / cols) * stepY + 80));
            idx++;
        }
    }

    idx = 0;
    for (auto &t : net_->transitions()) {
        if (t->pos().x() < 0) {
            t->set_pos(QPointF((idx % cols) * stepX + 80, (idx / cols) * stepY + 240));
            idx++;
        }
    }
}

/** @brief Returns the line at the current read position without advancing. */
std::string PnFileParser::currentLine() const
{
    return lines_[line_idx_];
}

/** @brief Increments line_idx_ and returns false when past the last line. */
bool PnFileParser::advance()
{
    if (line_idx_ < (int)lines_.size())
        line_idx_++;
    return !atEnd();
}

/** @brief Returns true when line_idx_ >= lines_.size(). */
bool PnFileParser::atEnd() const
{
    return line_idx_ >= (int)lines_.size();
}

/**
 * @brief Removes a trailing '#' comment from a line.
 *
 * A '#' inside a double-quoted string is not treated as a comment start.
 */
std::string PnFileParser::stripComment(const std::string &line) const
{
    bool inString = false;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == '"') inString = !inString;
        if (!inString && line[i] == '#')
            return line.substr(0, i);
    }
    return line;
}

/** @brief Removes leading and trailing ASCII whitespace from s. */
std::string PnFileParser::trim(const std::string &s) const
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}
