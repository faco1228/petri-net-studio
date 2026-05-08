/**
 * @file pn_file_parser.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief PnFileParser — loads a .pn text file into a PnNet object.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Provides a single static entry point PnFileParser::load()
 *   2. The parser reads the .pn format line-by-line through ordered sections
 *   3. Each section (Name, Comment, Inputs, Outputs, Variables, Places, Transitions)
 *      is handled by a dedicated private method
 *   4. Positions missing from the file are filled in with a grid auto-layout
 */

#ifndef PN_FILE_PARSER_H
#define PN_FILE_PARSER_H

#include <string>
#include <memory>
#include "pn_net.h"

/**
 * @brief Parses a .pn file and constructs the corresponding PnNet.
 *
 * Use the static load() method — the class is instantiated internally
 * and is not intended to be constructed by callers.
 */
class PnFileParser
{
public:
    /**
     * @brief Loads a .pn file and returns a fully populated PnNet.
     *
     * @param path     filesystem path to the .pn file
     * @param errorMsg filled with a human-readable message on failure
     * @return unique_ptr to the parsed PnNet, or nullptr on error
     */
    static std::unique_ptr<PnNet> load(const std::string &path, std::string &errorMsg);

private:
    /** @brief Constructs a PnFileParser with a fresh net and line index of 0. */
    PnFileParser();

    /**
     * @brief Reads the file at path into lines_ and drives all section parsers.
     *
     * @param path     filesystem path
     * @param errorMsg error description on failure
     * @return populated PnNet or nullptr
     */
    std::unique_ptr<PnNet> parse(const std::string &path, std::string &errorMsg);

    // ---- Section parsers ----

    /** @brief Parses the "Jméno sítě:" / "Meno siete:" section and sets net name. */
    bool parseName(std::string &errorMsg);

    /** @brief Parses the "Komentář:" / "Komentár:" section and sets net comment. */
    bool parseComment(std::string &errorMsg);

    /** @brief Parses the "Vstupy:" section and adds input names. */
    bool parseInputs(std::string &errorMsg);

    /** @brief Parses the "Výstupy:" section and adds output names. */
    bool parseOutputs(std::string &errorMsg);

    /** @brief Parses the "Proměnné:" / "Premenné:" section and adds variables. */
    bool parseVariables(std::string &errorMsg);

    /** @brief Parses the "Místa" / "Miesta" section and creates Place objects. */
    bool parsePlaces(std::string &errorMsg);

    /** @brief Parses the "Přechody" / "Prechody" section and creates Transition/Arc objects. */
    bool parseTransitions(std::string &errorMsg);

    // ---- Low-level helpers ----

    /** @brief Returns the current line without advancing. */
    std::string currentLine() const;

    /**
     * @brief Advances to the next line.
     * @return false when end-of-file is reached
     */
    bool advance();

    /** @brief Returns true when all lines have been consumed. */
    bool atEnd() const;

    /**
     * @brief Strips a trailing '#' comment from a line.
     *
     * Respects double-quoted strings so '#' inside a string is preserved.
     *
     * @param line raw input line
     * @return line with comment suffix removed
     */
    std::string stripComment(const std::string &line) const;

    /**
     * @brief Trims leading and trailing whitespace from a string.
     *
     * @param s input string
     * @return trimmed copy
     */
    std::string trim(const std::string &s) const;

    /**
     * @brief Reads a balanced { ... } block from the current position.
     *
     * Handles multi-line blocks and nested braces; advances past the
     * closing brace.
     *
     * @param block     receives the block contents (without outer braces)
     * @param errorMsg  set on failure (unterminated block)
     * @return true on success
     */
    bool readBlock(std::string &block, std::string &errorMsg);

    /**
     * @brief Tries to parse "pos: x,y" from a token string.
     *
     * @param token  string that may contain a "pos:" prefix
     * @param x      set to the parsed x coordinate on success
     * @param y      set to the parsed y coordinate on success
     * @return true if a valid "pos:" was found and parsed
     */
    bool tryParsePos(const std::string &token, double &x, double &y) const;

    /**
     * @brief Assigns grid positions to all elements that have no explicit position.
     *
     * Elements with pos == (-1, -1) receive evenly spaced positions on a grid.
     */
    void applyAutoLayout();

    std::vector<std::string>  lines_;    ///< all lines read from the file
    int                       line_idx_; ///< current read position
    std::unique_ptr<PnNet>    net_;      ///< net being constructed
};


#endif // PN_FILE_PARSER_H
