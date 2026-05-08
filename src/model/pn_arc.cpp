/**
 * @file pn_arc.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of the Arc class.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Initialises all Arc members via the constructor initializer list
 *   2. Provides trivial accessor and mutator implementations
 */

#include "pn_arc.h"

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs an Arc and initialises all fields. */
Arc::Arc(int id, ArcType type, int placeId, int transitionId, int weight)
    : id_(id)
    , type_(type)
    , place_id_(placeId)
    , transition_id_(transitionId)
    , weight_(weight)
{}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the unique arc ID. */
int Arc::id() const { return id_; }

/** @brief Returns the arc direction (INPUT or OUTPUT). */
ArcType Arc::type() const { return type_; }

/** @brief Returns the ID of the connected place. */
int Arc::place_id() const { return place_id_; }

/** @brief Returns the ID of the connected transition. */
int Arc::transition_id() const { return transition_id_; }

/** @brief Returns the arc weight. */
int Arc::weight() const { return weight_; }

/** @brief Sets the arc weight. */
void Arc::set_weight(int weight) { weight_ = weight; }

/** @brief Returns the intermediate waypoints (empty = straight line). */
const std::vector<QPointF>& Arc::waypoints() const { return waypoints_; }

/** @brief Sets the intermediate waypoints. */
void Arc::set_waypoints(const std::vector<QPointF> &points) { waypoints_ = points; }
