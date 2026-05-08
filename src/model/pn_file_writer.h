/**
 * @file pn_file_writer.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief PnFileWriter — serializes a PnNet object back to a .pn text file.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Declares a single static method save() that writes all sections of
 *      the .pn format in the canonical order expected by PnFileParser
 */

#ifndef PN_FILE_WRITER_H
#define PN_FILE_WRITER_H

#include <string>
#include "pn_net.h"

/**
 * @brief Serializes a PnNet to the .pn text format.
 *
 * This class provides a single static utility method and is not
 * intended to be instantiated.
 */
class PnFileWriter
{
public:
    /**
     * @brief Writes a PnNet to a .pn file.
     *
     * The output is a valid .pn file that can be round-tripped through
     * PnFileParser without loss of data.
     *
     * @param net      the Petri net to serialize
     * @param path     filesystem path of the output file
     * @param errorMsg filled with a human-readable message on failure
     * @return true on success, false on I/O error
     */
    static bool save(const PnNet &net, const std::string &path, std::string &errorMsg);
};


#endif // PN_FILE_WRITER_H
