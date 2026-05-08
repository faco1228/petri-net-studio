/**
 * @file pn_file_writer.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of PnFileWriter.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Opens the output file for writing
 *   2. Writes each .pn section in the order expected by PnFileParser:
 *      Name, Comment, Inputs, Outputs, Variables, Places, Transitions
 *   3. Each place stores its canvas position (pos: x,y) for exact round-trip fidelity
 *   4. Each transition stores in:/out: arc lists and the full when:/do: condition
 */

#include "pn_file_writer.h"

#include <fstream>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////

/** @brief Writes all sections of net to the file at path. */
bool PnFileWriter::save(const PnNet &net, const std::string &path, std::string &errorMsg)
{
    std::ofstream file(path);
    if (!file.is_open()) {
        errorMsg = "Cannot open file for writing: " + path;
        return false;
    }

    // ---- Name section ----
    file << "Jméno sítě:\n";
    file << "    " << net.name() << "\n\n";

    // ---- Comment section (optional) ----
    if (!net.comment().empty()) {
        file << "Komentář:\n";
        // Indent every line of the multi-line comment
        std::istringstream ss(net.comment());
        std::string line;
        while (std::getline(ss, line))
            file << "    " << line << "\n";
        file << "\n";
    }

    // ---- Inputs section (optional) ----
    if (!net.inputs().empty()) {
        file << "Vstupy:\n";
        for (const auto &in : net.inputs())
            file << "    " << in << "\n";
        file << "\n";
    }

    // ---- Outputs section (optional) ----
    if (!net.outputs().empty()) {
        file << "Výstupy:\n";
        for (const auto &out : net.outputs())
            file << "    " << out << "\n";
        file << "\n";
    }

    // ---- Variables section (optional) ----
    if (!net.variables().empty()) {
        file << "Premenné:\n";
        for (const auto &v : net.variables()) {
            file << "    " << v.type << " " << v.name << " = " << v.value;
            if (!v.comment.empty())
                file << "  # " << v.comment;
            file << "\n";
        }
        file << "\n";
    }

    // ---- Places section ----
    file << "Miesta (počiatočné tokeny, voliteľne akcie):\n";
    for (const auto &p : net.places()) {
        // Write place with canvas position for round-trip fidelity
        file << "    " << p->name()
             << " (" << p->initial_tokens() << ")"
             << " pos: " << p->pos().x() << "," << p->pos().y();

        if (!p->action().empty())
            file << " : { " << p->action() << " }";

        file << "\n";
    }
    file << "\n";

    // ---- Transitions section ----
    file << "Prechody a ich podmienky:\n";
    for (const auto &t : net.transitions()) {
        file << t->name()
             << " pos: " << t->pos().x() << "," << t->pos().y()
             << " :\n";

        // Collect input and output arc strings for this transition
        auto arcs = net.arcs_for_transition(t->id());
        std::string inArcs, outArcs;
        for (Arc *a : arcs) {
            const Place *p = net.find_place_by_id(a->place_id());
            if (!p) continue;
            // Format as "PlaceName*weight"
            std::string entry = p->name() + "*" + std::to_string(a->weight());
            if (a->type() == ArcType::INPUT)
                inArcs  += (inArcs.empty()  ? "" : ", ") + entry;
            else
                outArcs += (outArcs.empty() ? "" : ", ") + entry;
        }

        file << "    in:  " << inArcs  << "\n";
        file << "    out: " << outArcs << "\n";

        // Build the "when:" clause from the three firing-condition fields
        std::string when;
        if (!t->event_name().empty()) when += t->event_name();
        if (!t->guard().empty())      when += " [ " + t->guard() + " ]";
        if (!t->delay_expr().empty()) when += " @ " + t->delay_expr();
        file << "    when: " << when << "\n";

        // Write the action block (always present, even if empty)
        file << "    do: { " << t->action() << " }\n";
        file << "\n";
    }

    return true;
}
