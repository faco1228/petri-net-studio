/**
 * @file code_generator.h
 * @brief CodeGenerator - generates a standalone C++ interpreter from a PnNet.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <string>

class PnNet;

class CodeGenerator
{
public:
    // Generates net_NAME.cpp into outputDir. Returns false on error.
    static bool generate(const PnNet &net, const std::string &outputDir, std::string &errorMsg);
};
