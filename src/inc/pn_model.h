/**
 * @file pn_model.h
 * @brief Common forward declarations, typedefs and shared types for the Petri net model.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <string>
#include <map>

// Forward declarations
class Place;
class Transition;
class Arc;
class PnNet;

using TokenCount = int;
using ArcWeight  = int;
using Marking    = std::map<std::string, TokenCount>;

enum class ArcType {
    INPUT,   // place -> transition
    OUTPUT   // transition -> place
};
