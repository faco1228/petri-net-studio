/**
 * @file pn_arc.h
 * @brief Arc class - oriented edge between a place and a transition.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <vector>
#include <QPointF>
#include "../inc/pn_model.h"

class Arc
{
public:
    Arc(int id, ArcType type, int placeId, int transitionId, int weight = 1);

    int getId() const;
    ArcType getType() const;

    int getPlaceId() const;
    int getTransitionId() const;

    int getWeight() const;
    void setWeight(int weight);

    // Intermediate control points for non-straight edges. Empty = straight line.
    const std::vector<QPointF>& getWaypoints() const;
    void setWaypoints(const std::vector<QPointF> &points);

private:
    int m_id;
    ArcType m_type;
    int m_placeId;
    int m_transitionId;
    int m_weight;
    std::vector<QPointF> m_waypoints;
};
