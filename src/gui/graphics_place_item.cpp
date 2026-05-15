/**
 * @file graphics_place_item.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of GraphicsPlaceItem.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor positions the ellipse centred at 'center' and sets item flags
 *   2. paint() draws the filled circle with token count and name label
 *   3. itemChange() forwards position changes to GraphicsEditor::onItemMoved()
 *   4. mouseDoubleClickEvent() opens the place properties dialog
 */

#include "graphics_place_item.h"
#include "graphics_editor.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs the place item at the given centre position. */
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
    setZValue(1); // draw above arcs
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the model place ID. */
int     GraphicsPlaceItem::placeId()    const { return m_placeId; }

/** @brief Returns the display label. */
QString GraphicsPlaceItem::placeName()  const { return m_name; }

/** @brief Returns the initial token count. */
int     GraphicsPlaceItem::tokenCount() const { return m_tokens; }

/** @brief Sets the initial token count and triggers a repaint. */
void GraphicsPlaceItem::setTokenCount(int tokens) { m_tokens = tokens; update(); }

/** @brief Sets the display label and triggers a repaint. */
void GraphicsPlaceItem::setPlaceName(const QString &name) { m_name = name; update(); }

///////////////////////////////////////////////////////////////////////////////

/** @brief Enables monitor colouring with the given live token count. */
void GraphicsPlaceItem::setMonitorHighlight(bool active, int liveTokens)
{
    m_monitorActive = active;
    m_liveTokens    = liveTokens;
    update();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the scene-coordinate centre of the item. */
QPointF GraphicsPlaceItem::centerPos() const
{
    return scenePos();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the bounding rect extended to include the name label below the circle. */
QRectF GraphicsPlaceItem::boundingRect() const
{
    return QRectF(-PLACE_RADIUS, -PLACE_RADIUS,
                  PLACE_RADIUS * 2, PLACE_RADIUS * 2 + 20);
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Paints the circle, token count and name label.
 *
 * Fill colour:
 *   - White by default
 *   - Light green when monitor mode is active and tokens > 0
 *   - Light red  when monitor mode is active and tokens == 0
 */
void GraphicsPlaceItem::paint(QPainter *painter,
                               const QStyleOptionGraphicsItem */*option*/,
                               QWidget */*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing);

    // Choose fill colour based on monitor state
    QColor fill = Qt::white;
    if (m_monitorActive)
        fill = (m_liveTokens > 0) ? QColor(180, 230, 180) : QColor(230, 180, 180);

    painter->setBrush(fill);
    painter->setPen(isSelected() ? QPen(Qt::blue, 2) : QPen(Qt::black, 1.5));
    painter->drawEllipse(QRectF(-PLACE_RADIUS, -PLACE_RADIUS, PLACE_RADIUS * 2, PLACE_RADIUS * 2));

    // Show tokens: dots for 1-5, number for 6+, nothing for 0
    int displayTokens = m_monitorActive ? m_liveTokens : m_tokens;
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::black);
    if (displayTokens >= 1 && displayTokens <= 5) {
        // Dot layout: positions for 1..5 tokens inside the circle
        static const QPointF dots[5][5] = {
            {{ 0,  0}, {0,0}, {0,0}, {0,0}, {0,0}},           // 1
            {{-7,  0}, {7,0}, {0,0}, {0,0}, {0,0}},           // 2
            {{-7,  5}, {7,5}, {0,-7},{0,0}, {0,0}},           // 3
            {{-7, -6}, {7,-6},{-7,6},{7,6}, {0,0}},           // 4
            {{-7, -6}, {7,-6},{ 0,0},{-7,6},{7,6}},           // 5
        };
        for (int i = 0; i < displayTokens; i++)
            painter->drawEllipse(dots[displayTokens-1][i], 4.0, 4.0);
    } else if (displayTokens > 5) {
        painter->setPen(Qt::black);
        painter->setBrush(Qt::NoBrush);
        painter->setFont(QFont("Sans", 10, QFont::Bold));
        painter->drawText(QRectF(-PLACE_RADIUS, -PLACE_RADIUS, PLACE_RADIUS * 2, PLACE_RADIUS * 2),
                          Qt::AlignCenter, QString::number(displayTokens));
    }

    // Name label below the circle
    painter->setPen(Qt::black);
    painter->setFont(QFont("Sans", 9));
    painter->drawText(QRectF(-40, PLACE_RADIUS + 2, 80, 16),
                      Qt::AlignCenter, m_name);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Forwards position changes to the editor so the model and arcs are updated. */
QVariant GraphicsPlaceItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && m_editor)
        m_editor->onItemMoved(m_placeId, true, value.toPointF());

    return QGraphicsEllipseItem::itemChange(change, value);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Opens the place properties dialog when the user double-clicks. */
void GraphicsPlaceItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsEllipseItem::mouseDoubleClickEvent(event);
    if (m_editor)
        m_editor->openPlaceDialog(m_placeId);
}
