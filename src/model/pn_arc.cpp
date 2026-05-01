/**
 * @file pn_arc.cpp
 * @brief Implementation of the Arc class.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "pn_arc.h"

Arc::Arc(int id, ArcType type, int placeId, int transitionId, int weight)
    : m_id(id)
    , m_type(type)
    , m_placeId(placeId)
    , m_transitionId(transitionId)
    , m_weight(weight)
{}

int Arc::getId() const { return m_id; }
ArcType Arc::getType() const { return m_type; }

int Arc::getPlaceId() const { return m_placeId; }
int Arc::getTransitionId() const { return m_transitionId; }

int Arc::getWeight() const { return m_weight; }
void Arc::setWeight(int weight) { m_weight = weight; }

const std::vector<QPointF>& Arc::getWaypoints() const { return m_waypoints; }
void Arc::setWaypoints(const std::vector<QPointF> &points) { m_waypoints = points; }
