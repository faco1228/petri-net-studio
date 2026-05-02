/**
 * @file graphics_place_item.cpp
 * @brief Implementation of GraphicsPlaceItem.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "graphics_place_item.h"
#include "graphics_editor.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

GraphicsPlaceItem::GraphicsPlaceItem(int placeId, const QString &name, int tokens,
                                     QPointF center, GraphicsEditor *editor)
    : QGraphicsEllipseItem(-PLACE_RADIUS, -PLACE_RADIUS, PLACE_RADIUS * 2, PLACE_RADIUS * 2)
    , m_placeId(placeId)
    , m_name(name)
    , m_tokens(tokens)
    , m_monitorActive(false)
    , m_liveTokens(0)
    , m_editor(editor)
{
    setPos(center);
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable
             | QGraphicsItem::ItemSendsGeometryChanges);
    setZValue(1);
}

int     GraphicsPlaceItem::placeId()   const { return m_placeId; }
QString GraphicsPlaceItem::placeName() const { return m_name; }
int     GraphicsPlaceItem::tokenCount() const { return m_tokens; }

void GraphicsPlaceItem::setTokenCount(int tokens) { m_tokens = tokens; update(); }
void GraphicsPlaceItem::setPlaceName(const QString &name) { m_name = name; update(); }

void GraphicsPlaceItem::setMonitorHighlight(bool active, int liveTokens)
{
    m_monitorActive = active;
    m_liveTokens    = liveTokens;
    update();
}

QPointF GraphicsPlaceItem::centerPos() const
{
    return scenePos();
}

QRectF GraphicsPlaceItem::boundingRect() const
{
    // Extend below the circle to include the name label
    return QRectF(-PLACE_RADIUS, -PLACE_RADIUS,
                  PLACE_RADIUS * 2, PLACE_RADIUS * 2 + 20);
}

void GraphicsPlaceItem::paint(QPainter *painter,
                               const QStyleOptionGraphicsItem */*option*/,
                               QWidget */*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing);

    // Fill color based on state
    QColor fill = Qt::white;
    if (m_monitorActive)
        fill = (m_liveTokens > 0) ? QColor(180, 230, 180) : QColor(230, 180, 180);

    painter->setBrush(fill);
    painter->setPen(isSelected() ? QPen(Qt::blue, 2) : QPen(Qt::black, 1.5));
    painter->drawEllipse(QRectF(-PLACE_RADIUS, -PLACE_RADIUS, PLACE_RADIUS * 2, PLACE_RADIUS * 2));

    // Token count in the center
    int displayTokens = m_monitorActive ? m_liveTokens : m_tokens;
    painter->setPen(Qt::black);
    painter->setFont(QFont("Sans", 10, QFont::Bold));
    painter->drawText(QRectF(-PLACE_RADIUS, -PLACE_RADIUS, PLACE_RADIUS * 2, PLACE_RADIUS * 2),
                      Qt::AlignCenter, QString::number(displayTokens));

    // Name below the circle
    painter->setFont(QFont("Sans", 9));
    painter->drawText(QRectF(-40, PLACE_RADIUS + 2, 80, 16),
                      Qt::AlignCenter, m_name);
}

QVariant GraphicsPlaceItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && m_editor)
        m_editor->onItemMoved(m_placeId, true, value.toPointF());

    return QGraphicsEllipseItem::itemChange(change, value);
}

void GraphicsPlaceItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsEllipseItem::mouseDoubleClickEvent(event);
    if (m_editor)
        m_editor->openPlaceDialog(m_placeId);
}
