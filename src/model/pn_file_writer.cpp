/**
 * @file pn_file_writer.cpp
 * @brief Implementation of PnFileWriter.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "pn_file_writer.h"

#include <fstream>
#include <sstream>

bool PnFileWriter::save(const PnNet &net, const std::string &path, std::string &errorMsg)
{
    std::ofstream file(path);
    if (!file.is_open()) {
        errorMsg = "Cannot open file for writing: " + path;
        return false;
    }

    // Name
    file << "Jméno sítě:\n";
    file << "    " << net.getName() << "\n\n";

    // Comment
    if (!net.getComment().empty()) {
        file << "Komentář:\n";
        // Indent each line of the comment
        std::istringstream ss(net.getComment());
        std::string line;
        while (std::getline(ss, line))
            file << "    " << line << "\n";
        file << "\n";
    }

    // Inputs
    if (!net.getInputs().empty()) {
        file << "Vstupy:\n";
        for (const auto &in : net.getInputs())
            file << "    " << in << "\n";
        file << "\n";
    }

    // Outputs
    if (!net.getOutputs().empty()) {
        file << "Výstupy:\n";
        for (const auto &out : net.getOutputs())
            file << "    " << out << "\n";
        file << "\n";
    }

    // Variables
    if (!net.getVariables().empty()) {
        file << "Premenné:\n";
        for (const auto &v : net.getVariables()) {
            file << "    " << v.type << " " << v.name << " = " << v.value;
            if (!v.comment.empty())
                file << "  # " << v.comment;
            file << "\n";
        }
        file << "\n";
    }

    // Places
    file << "Miesta (počiatočné tokeny, voliteľne akcie):\n";
    for (const auto &p : net.getPlaces()) {
        file << "    " << p->getName()
             << " (" << p->getInitialTokens() << ")"
             << " pos: " << p->getPos().x() << "," << p->getPos().y();

        if (!p->getAction().empty())
            file << " : { " << p->getAction() << " }";

        file << "\n";
    }
    file << "\n";

    // Transitions
    file << "Prechody a ich podmienky:\n";
    for (const auto &t : net.getTransitions()) {
        file << t->getName()
             << " pos: " << t->getPos().x() << "," << t->getPos().y()
             << " :\n";

        // Input arcs
        auto arcs = net.getArcsForTransition(t->getId());
        std::string inArcs, outArcs;
        for (Arc *a : arcs) {
            const Place *p = net.findPlaceById(a->getPlaceId());
            if (!p) continue;
            std::string entry = p->getName() + "*" + std::to_string(a->getWeight());
            if (a->getType() == ArcType::INPUT)
                inArcs  += (inArcs.empty()  ? "" : ", ") + entry;
            else
                outArcs += (outArcs.empty() ? "" : ", ") + entry;
        }

        file << "    in:  " << inArcs  << "\n";
        file << "    out: " << outArcs << "\n";

        // Firing condition
        std::string when;
        if (!t->getEventName().empty()) when += t->getEventName();
        if (!t->getGuard().empty())     when += " [ " + t->getGuard() + " ]";
        if (!t->getDelayExpr().empty()) when += " @ " + t->getDelayExpr();
        file << "    when: " << when << "\n";

        // Action
        file << "    do: { " << t->getAction() << " }\n";
        file << "\n";
    }

    return true;
}
