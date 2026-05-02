/**
 * @file graphics_arc_item.cpp
 * @brief Implementation of GraphicsArcItem.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "graphics_arc_item.h"
#include "graphics_place_item.h"
#include "graphics_transition_item.h"
#include "graphics_editor.h"

#include <QPainter>
#include <QLineF>
#include <cmath>

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
    setZValue(0);
    setFlag(QGraphicsItem::ItemIsSelectable);
    updateGeometry();
}

void GraphicsArcItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPathItem::mouseDoubleClickEvent(event);
    if (m_editor)
        m_editor->openArcDialog(m_arcId);
}

int     GraphicsArcItem::arcId()   const { return m_arcId; }
ArcType GraphicsArcItem::arcType() const { return m_type; }
int     GraphicsArcItem::weight()  const { return m_weight; }
void    GraphicsArcItem::setWeight(int w) { m_weight = w; update(); }

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

    QLineF line(from, to);
    QPointF dir = (line.length() > 0)
                  ? QPointF(line.dx() / line.length(), line.dy() / line.length())
                  : QPointF(1, 0);

    QPainterPath path;
    path.moveTo(from);
    path.lineTo(to);
    path.addPath(buildArrowHead(to, dir));

    setPath(path);
}

void GraphicsArcItem::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem */*option*/,
                             QWidget */*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing);
    QPen pen(isSelected() ? Qt::blue : Qt::black, 1.5);
    painter->setPen(pen);
    painter->setBrush(isSelected() ? Qt::blue : Qt::black);
    painter->drawPath(path());

    // Weight label at the midpoint (only if > 1)
    if (m_weight > 1) {
        QPointF mid = path().pointAtPercent(0.5);
        painter->setFont(QFont("Sans", 8));
        painter->drawText(mid + QPointF(4, -4), QString::number(m_weight));
    }
}

QPointF GraphicsArcItem::connectionPoint(QGraphicsItem *item, QPointF target)
{
    QPointF center = item->scenePos();
    QLineF line(center, target);
    if (line.length() < 1.0) return center;

    // For ellipse (place): intersection with circle of radius PLACE_RADIUS
    if (dynamic_cast<GraphicsPlaceItem*>(item)) {
        double r = PLACE_RADIUS;
        double len = line.length();
        return center + QPointF(line.dx() / len * r, line.dy() / len * r);
    }

    // For rect (transition): intersection with rectangle border
    if (auto *trans = dynamic_cast<GraphicsTransitionItem*>(item)) {
        Q_UNUSED(trans);
        QRectF rect(-TRANSITION_W / 2, -TRANSITION_H / 2, TRANSITION_W, TRANSITION_H);
        // Check intersection with each side
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

QPainterPath GraphicsArcItem::buildArrowHead(QPointF tip, QPointF dir)
{
    const double size = 10.0;
    const double angle = 0.45; // radians (~25 degrees)

    double cos1 = std::cos(angle), sin1 = std::sin(angle);
    double cos2 = std::cos(-angle), sin2 = std::sin(-angle);

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
