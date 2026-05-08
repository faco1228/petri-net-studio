/**
 * @file graphics_arc_item.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of GraphicsArcItem.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor calls updateGeometry() to compute the initial path
 *   2. updateGeometry() calculates border-to-border line and arrowhead using
 *      connectionPoint() for each endpoint
 *   3. paint() draws the path and an optional weight label at the midpoint
 *   4. connectionPoint() intersects the line with the ellipse or rectangle border
 *   5. buildArrowHead() constructs a triangular arrowhead QPainterPath
 */

#include "graphics_arc_item.h"
#include "graphics_place_item.h"
#include "graphics_transition_item.h"
#include "graphics_editor.h"

#include <QPainter>
#include <QLineF>
#include <cmath>

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs the arc item and computes the initial path geometry. */
GraphicsArcItem::GraphicsArcItem(int arcId, ArcType type,
                                  GraphicsPlaceItem *placeItem,
                                  GraphicsTransitionItem *transitionItem,
                                  GraphicsEditor *editor)
    : m_arcId(arcId)
    , m_type(type)
    , m_weight(1)
    , m_placeItem(placeItem)
    , m_transitionItem(transitionItem)
    , m_editor(editor)
{
    setZValue(0); // draw below nodes
    setFlag(QGraphicsItem::ItemIsSelectable);
    updateGeometry();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Opens the arc weight dialog when the user double-clicks the arc. */
void GraphicsArcItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPathItem::mouseDoubleClickEvent(event);
    if (m_editor)
        m_editor->openArcDialog(m_arcId);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the model arc ID. */
int     GraphicsArcItem::arcId()   const { return m_arcId; }

/** @brief Returns the arc direction (INPUT or OUTPUT). */
ArcType GraphicsArcItem::arcType() const { return m_type; }

/** @brief Returns the arc weight. */
int     GraphicsArcItem::weight()  const { return m_weight; }

/** @brief Sets the arc weight and triggers a repaint. */
void    GraphicsArcItem::setWeight(int w) { m_weight = w; update(); }

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Recalculates the arc path from the current positions of the connected items.
 *
 * For INPUT arcs the direction is place -> transition.
 * For OUTPUT arcs the direction is transition -> place.
 */
void GraphicsArcItem::updateGeometry()
{
    QPointF from, to;

    if (m_type == ArcType::INPUT) {
        // Place -> Transition
        from = connectionPoint(m_placeItem,      m_transitionItem->centerPos());
        to   = connectionPoint(m_transitionItem, m_placeItem->centerPos());
    } else {
        // Transition -> Place
        from = connectionPoint(m_transitionItem, m_placeItem->centerPos());
        to   = connectionPoint(m_placeItem,      m_transitionItem->centerPos());
    }

    QLineF  line(from, to);
    QPointF dir = (line.length() > 0)
                  ? QPointF(line.dx() / line.length(), line.dy() / line.length())
                  : QPointF(1, 0); // fallback direction when endpoints overlap

    QPainterPath path;
    path.moveTo(from);
    path.lineTo(to);
    path.addPath(buildArrowHead(to, dir));

    setPath(path);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Draws the arc line, arrowhead and weight label. */
void GraphicsArcItem::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem */*option*/,
                             QWidget */*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing);
    QPen pen(isSelected() ? Qt::blue : Qt::black, 1.5);
    painter->setPen(pen);
    painter->setBrush(isSelected() ? Qt::blue : Qt::black);
    painter->drawPath(path());

    // Draw weight label near the midpoint (only when weight > 1)
    if (m_weight > 1) {
        QPointF mid = path().pointAtPercent(0.5);
        painter->setFont(QFont("Sans", 8));
        painter->drawText(mid + QPointF(4, -4), QString::number(m_weight));
    }
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Returns the point on the item's border in the direction of target.
 *
 * For a place (ellipse): calculates the point on the circle at radius PLACE_RADIUS.
 * For a transition (rectangle): intersects the line with each side of the rectangle.
 */
QPointF GraphicsArcItem::connectionPoint(QGraphicsItem *item, QPointF target)
{
    QPointF center = item->scenePos();
    QLineF  line(center, target);
    if (line.length() < 1.0) return center;

    // Ellipse (place): intersection at PLACE_RADIUS from the centre
    if (dynamic_cast<GraphicsPlaceItem*>(item)) {
        double r   = PLACE_RADIUS;
        double len = line.length();
        return center + QPointF(line.dx() / len * r, line.dy() / len * r);
    }

    // Rectangle (transition): intersect with each of the four sides
    if (auto *trans = dynamic_cast<GraphicsTransitionItem*>(item)) {
        Q_UNUSED(trans);
        QRectF rect(-TRANSITION_W / 2, -TRANSITION_H / 2, TRANSITION_W, TRANSITION_H);
        QLineF sides[4] = {
            QLineF(center + rect.topLeft(),     center + rect.topRight()),
            QLineF(center + rect.topRight(),    center + rect.bottomRight()),
            QLineF(center + rect.bottomRight(), center + rect.bottomLeft()),
            QLineF(center + rect.bottomLeft(),  center + rect.topLeft())
        };
        for (auto &side : sides) {
            QPointF pt;
            if (line.intersects(side, &pt) == QLineF::BoundedIntersection)
                return pt;
        }
        return center;
    }

    return center;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Builds a filled arrowhead triangle pointing at tip in direction dir.
 *
 * The arrowhead is ~10 px long with a ~25-degree half-angle.
 */
QPainterPath GraphicsArcItem::buildArrowHead(QPointF tip, QPointF dir)
{
    const double size  = 10.0;
    const double angle = 0.45; // radians (~25 degrees)

    double cos1 = std::cos(angle),  sin1 = std::sin(angle);
    double cos2 = std::cos(-angle), sin2 = std::sin(-angle);

    // Rotate dir by ±angle to get the two wing vectors
    QPointF left(dir.x() * cos1 - dir.y() * sin1,
                 dir.x() * sin1 + dir.y() * cos1);
    QPointF right(dir.x() * cos2 - dir.y() * sin2,
                  dir.x() * sin2 + dir.y() * cos2);

    QPainterPath arrow;
    arrow.moveTo(tip);
    arrow.lineTo(tip - left  * size);
    arrow.lineTo(tip - right * size);
    arrow.closeSubpath();
    return arrow;
}
