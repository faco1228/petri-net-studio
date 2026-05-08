/**
 * @file pn_model.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Common forward declarations, typedefs and shared types for the Petri net model.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Forward-declares all core model classes (Place, Transition, Arc, PnNet)
 *   2. Defines shared typedefs (TokenCount, ArcWeight, Marking)
 *   3. Defines the ArcType enum used by Arc and the parser/writer
 */

#ifndef PN_MODEL_H
#define PN_MODEL_H

#include <string>
#include <map>

// Forward declarations
class Place;
class Transition;
class Arc;
class PnNet;

/** @brief Number of tokens held by a place at any given time. */
using TokenCount = int;

/** @brief Weight assigned to an arc (how many tokens are consumed/produced). */
using ArcWeight = int;

/** @brief A complete marking: maps place name to its current token count. */
using Marking = std::map<std::string, TokenCount>;

/**
 * @brief Direction of an arc in the Petri net.
 *
 * INPUT  arcs run from a place to a transition (consume tokens).
 * OUTPUT arcs run from a transition to a place (produce tokens).
 */
enum class ArcType {
    INPUT,  ///< place -> transition (consumes tokens)
    OUTPUT  ///< transition -> place (produces tokens)
};

#endif // PN_MODEL_H
