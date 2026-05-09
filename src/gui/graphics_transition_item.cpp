/**
 * @file graphics_transition_item.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of GraphicsTransitionItem.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor centres the rectangle at 'center' and sets item flags
 *   2. paint() draws the rectangle with monitor-mode colour coding and the name below
 *   3. itemChange() forwards position changes to GraphicsEditor::onItemMoved()
 *   4. mouseDoubleClickEvent() opens the transition properties dialog
 */

#include "graphics_transition_item.h"
#include "graphics_editor.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs the transition item centred at 'center'. */
GraphicsTransitionItem::GraphicsTransitionItem(int transitionId, const QString &name,
                                               QPointF center, GraphicsEditor *editor)
    : QGraphicsRectItem(-TRANSITION_W / 2, -TRANSITION_H / 2, TRANSITION_W, TRANSITION_H)
    , m_transitionId(transitionId)
    , m_name(name)
    , m_enabled(false)
    , m_pending(false)
    , m_editor(editor)
{
    setPos(center);
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable
             | QGraphicsItem::ItemSendsGeometryChanges);
    setZValue(1); // draw above arcs
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the model transition ID. */
int     GraphicsTransitionItem::transitionId()   const { return m_transitionId; }

/** @brief Returns the display label. */
QString GraphicsTransitionItem::transitionName() const { return m_name; }

/** @brief Sets the display label and triggers a repaint. */
void GraphicsTransitionItem::setTransitionName(const QString &name) { m_name = name; update(); }

/** @brief Sets the "enabled" (green) state. */
void GraphicsTransitionItem::setEnabled(bool enabled) { m_enabled = enabled; update(); }

/** @brief Sets the "pending timer" (orange) state. */
void GraphicsTransitionItem::setPending(bool pending) { m_pending = pending; update(); }

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the scene-coordinate centre of the item. */
QPointF GraphicsTransitionItem::centerPos() const
{
    return scenePos();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the bounding rect extended to include the name label. */
QRectF GraphicsTransitionItem::boundingRect() const
{
    // Label is drawn in a 80px-wide rect centred at x=0; ensure that's covered.
    const double halfW = std::max(TRANSITION_W / 2.0, 40.0);
    return QRectF(-halfW, -TRANSITION_H / 2, halfW * 2, TRANSITION_H + 20);
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Paints the rectangle with colour feedback and the name label below it.
 *
 * Colour priority (highest first):
 *   - Orange: timer pending (m_pending)
 *   - Green:  enabled (m_enabled)
 *   - White:  default / not in monitor mode
 */
void GraphicsTransitionItem::paint(QPainter *painter,
                                    const QStyleOptionGraphicsItem */*option*/,
                                    QWidget */*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing);

    QColor fill = Qt::white;
    if (m_enabled) fill = QColor(180, 230, 180); // green — can fire
    if (m_pending) fill = QColor(255, 200, 100); // orange — timer running (overrides green)

    painter->setBrush(fill);
    painter->setPen(isSelected() ? QPen(Qt::blue, 2) : QPen(Qt::black, 1.5));
    painter->drawRect(QRectF(-TRANSITION_W / 2, -TRANSITION_H / 2, TRANSITION_W, TRANSITION_H));

    // Name label below the rectangle
    painter->setPen(Qt::black);
    painter->setFont(QFont("Sans", 9));
    painter->drawText(QRectF(-40, TRANSITION_H / 2 + 2, 80, 16),
                      Qt::AlignCenter, m_name);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Forwards position changes to the editor so the model and arcs are updated. */
QVariant GraphicsTransitionItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && m_editor)
        m_editor->onItemMoved(m_transitionId, false, value.toPointF());

    return QGraphicsRectItem::itemChange(change, value);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Opens the transition properties dialog when the user double-clicks. */
void GraphicsTransitionItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsRectItem::mouseDoubleClickEvent(event);
    if (m_editor)
        m_editor->openTransitionDialog(m_transitionId);
}
