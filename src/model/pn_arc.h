/**
 * @file pn_arc.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Arc class — oriented edge between a place and a transition.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Declares the Arc class connecting one place and one transition
 *   2. Stores arc direction (INPUT or OUTPUT), weight, and optional waypoints for curved edges
 */

#ifndef PN_ARC_H
#define PN_ARC_H

#include <vector>
#include <QPointF>
#include "../inc/pn_model.h"

/**
 * @brief Represents a directed edge in a Petri net.
 *
 * Each arc connects exactly one place and one transition.  The direction
 * (ArcType::INPUT or ArcType::OUTPUT) determines whether tokens are
 * consumed from or produced into the place when the transition fires.
 */
class Arc
{
public:
    /**
     * @brief Constructs an arc.
     *
     * @param id           unique identifier assigned by PnNet
     * @param type         INPUT (place->transition) or OUTPUT (transition->place)
     * @param placeId      ID of the connected place
     * @param transitionId ID of the connected transition
     * @param weight       number of tokens consumed/produced per firing
     */
    Arc(int id, ArcType type, int placeId, int transitionId, int weight = 1);

    /**
     * @brief Returns the unique arc identifier.
     * @return integer ID
     */
    int id() const;

    /**
     * @brief Returns the arc direction.
     * @return ArcType::INPUT or ArcType::OUTPUT
     */
    ArcType type() const;

    /**
     * @brief Returns the ID of the connected place.
     * @return place ID
     */
    int place_id() const;

    /**
     * @brief Returns the ID of the connected transition.
     * @return transition ID
     */
    int transition_id() const;

    /**
     * @brief Returns the arc weight (tokens consumed/produced per firing).
     * @return positive integer
     */
    int weight() const;

    /**
     * @brief Sets the arc weight.
     * @param weight new weight (must be >= 1)
     */
    void set_weight(int weight);

    /**
     * @brief Returns the list of intermediate waypoints for curved rendering.
     *
     * An empty list means the arc is drawn as a straight line.
     *
     * @return const reference to waypoint vector
     */
    const std::vector<QPointF>& waypoints() const;

    /**
     * @brief Sets the intermediate waypoints.
     * @param points ordered list of scene-coordinate control points
     */
    void set_waypoints(const std::vector<QPointF> &points);

private:
    int                  id_;            ///< unique arc ID
    ArcType              type_;          ///< INPUT or OUTPUT
    int                  place_id_;      ///< connected place
    int                  transition_id_; ///< connected transition
    int                  weight_;        ///< tokens consumed/produced
    std::vector<QPointF> waypoints_;     ///< optional intermediate control points
};

#endif // PN_ARC_H
