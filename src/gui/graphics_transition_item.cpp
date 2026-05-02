/**
 * @file graphics_transition_item.cpp
 * @brief Implementation of GraphicsTransitionItem.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "graphics_transition_item.h"
#include "graphics_editor.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

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
    setZValue(1);
}

int     GraphicsTransitionItem::transitionId()   const { return m_transitionId; }
QString GraphicsTransitionItem::transitionName() const { return m_name; }

void GraphicsTransitionItem::setTransitionName(const QString &name) { m_name = name; update(); }

void GraphicsTransitionItem::setEnabled(bool enabled) { m_enabled = enabled; update(); }
void GraphicsTransitionItem::setPending(bool pending) { m_pending = pending; update(); }

QPointF GraphicsTransitionItem::centerPos() const
{
    return scenePos();
}

QRectF GraphicsTransitionItem::boundingRect() const
{
    // Extend below the rectangle to include the name label
    return QRectF(-TRANSITION_W / 2, -TRANSITION_H / 2,
                  TRANSITION_W, TRANSITION_H + 20);
}

void GraphicsTransitionItem::paint(QPainter *painter,
                                    const QStyleOptionGraphicsItem */*option*/,
                                    QWidget */*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing);

    QColor fill = Qt::white;
    if (m_enabled) fill = QColor(180, 230, 180);      // green - can fire
    if (m_pending) fill = QColor(255, 200, 100);      // orange - timer running

    painter->setBrush(fill);
    painter->setPen(isSelected() ? QPen(Qt::blue, 2) : QPen(Qt::black, 1.5));
    painter->drawRect(QRectF(-TRANSITION_W / 2, -TRANSITION_H / 2, TRANSITION_W, TRANSITION_H));

    // Name below the rectangle
    painter->setPen(Qt::black);
    painter->setFont(QFont("Sans", 9));
    painter->drawText(QRectF(-40, TRANSITION_H / 2 + 2, 80, 16),
                      Qt::AlignCenter, m_name);
}

QVariant GraphicsTransitionItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && m_editor)
        m_editor->onItemMoved(m_transitionId, false, value.toPointF());

    return QGraphicsRectItem::itemChange(change, value);
}

void GraphicsTransitionItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsRectItem::mouseDoubleClickEvent(event);
    if (m_editor)
        m_editor->openTransitionDialog(m_transitionId);
}
