/**
 * @file pn_file_writer.h
 * @brief PnFileWriter - serializes a PnNet object back to a .pn text file.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <string>
#include "pn_net.h"

class PnFileWriter
{
public:
    // Writes net to path. Returns false and fills errorMsg on failure.
    static bool save(const PnNet &net, const std::string &path, std::string &errorMsg);
};
