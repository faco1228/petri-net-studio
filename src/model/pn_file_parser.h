/**
 * @file pn_file_parser.h
 * @brief PnFileParser - loads a .pn text file into a PnNet object.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <string>
#include <memory>
#include "pn_net.h"

class PnFileParser
{
public:
    // Loads a .pn file and returns a populated PnNet.
    // Returns nullptr on failure and fills errorMsg with the first error found.
    static std::unique_ptr<PnNet> load(const std::string &path, std::string &errorMsg);

private:
    // Parser state - one instance per load() call
    PnFileParser();

    std::unique_ptr<PnNet> parse(const std::string &path, std::string &errorMsg);

    // Section parsers
    bool parseName(std::string &errorMsg);
    bool parseComment(std::string &errorMsg);
    bool parseInputs(std::string &errorMsg);
    bool parseOutputs(std::string &errorMsg);
    bool parseVariables(std::string &errorMsg);
    bool parsePlaces(std::string &errorMsg);
    bool parseTransitions(std::string &errorMsg);

    // Low-level helpers
    std::string currentLine() const;
    bool advance();           // move to next non-empty, non-comment line
    bool atEnd() const;
    std::string stripComment(const std::string &line) const;
    std::string trim(const std::string &s) const;

    // Reads a { ... } block, handling multiline and nested braces
    bool readBlock(std::string &block, std::string &errorMsg);

    // Parses "pos: x,y" and returns true if found, sets x/y
    bool tryParsePos(const std::string &token, double &x, double &y) const;

    // Auto-layout: assigns grid positions to elements without explicit pos
    void applyAutoLayout();

    std::vector<std::string> m_lines;
    int m_lineIdx;
    std::unique_ptr<PnNet> m_net;
};
