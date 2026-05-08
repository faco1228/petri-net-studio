/**
 * @file code_generator.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief CodeGenerator — generates a standalone C++ interpreter from a PnNet.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Declares a single static generate() method that produces a self-contained
 *      C++ source file for the given Petri net
 *   2. The generated file includes the runtime, static tables of places/transitions/arcs,
 *      user action code and a main() loop
 */

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <string>

class PnNet;

/**
 * @brief Produces a standalone C++ interpreter source file for a PnNet.
 *
 * The generated file is named net_<NetName>.cpp and is placed in outputDir.
 * It can be compiled with g++ -std=c++17 and run independently; it communicates
 * with the GUI via the UDP protocol defined in udp_protocol.h.
 */
class CodeGenerator
{
public:
    /**
     * @brief Generates a C++ interpreter source file for the given net.
     *
     * @param net       the Petri net to translate
     * @param outputDir directory where the generated .cpp file is written
     * @param errorMsg  filled with a human-readable message on failure
     * @return true on success, false if the file could not be written
     */
    static bool generate(const PnNet &net, const std::string &outputDir, std::string &errorMsg);
};

#endif // CODE_GENERATOR_H
