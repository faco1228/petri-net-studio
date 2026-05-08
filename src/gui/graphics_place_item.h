/**
 * @file graphics_place_item.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief GraphicsPlaceItem — visual representation of a place (circle with name and token count).
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Subclasses QGraphicsEllipseItem to draw a circle with the token count inside
 *      and the place name below
 *   2. ItemSendsGeometryChanges notifies GraphicsEditor when the item is moved
 *   3. Double-click opens PlaceDialog via GraphicsEditor::openPlaceDialog()
 *   4. Monitor highlight mode shows green (tokens > 0) or red (tokens == 0)
 */

#ifndef GRAPHICS_PLACE_ITEM_H
#define GRAPHICS_PLACE_ITEM_H

#include <QGraphicsEllipseItem>
#include <QString>

class GraphicsEditor;

/** @brief Radius of the place circle in scene pixels. */
static constexpr double PLACE_RADIUS = 25.0;

/**
 * @brief QGraphicsItem that draws a Petri net place as a labelled circle.
 *
 * The circle diameter is 2 * PLACE_RADIUS.  The token count is centred
 * inside the circle; the place name appears below it.
 */
class GraphicsPlaceItem : public QGraphicsEllipseItem
{
public:
    /**
     * @brief Constructs the place item and positions it at center.
     *
     * @param placeId ID matching the corresponding Place in the model
     * @param name    initial label
     * @param tokens  initial token count
     * @param center  scene-coordinate centre position
     * @param editor  owning editor (used for callbacks)
     */
    GraphicsPlaceItem(int placeId, const QString &name, int tokens,
                      QPointF center, GraphicsEditor *editor);

    /**
     * @brief Returns the place ID.
     * @return model place ID
     */
    int placeId() const;

    /**
     * @brief Returns the place label.
     * @return display name
     */
    QString placeName() const;

    /**
     * @brief Returns the initial token count shown when not in monitor mode.
     * @return token count
     */
    int tokenCount() const;

    /**
     * @brief Sets the displayed token count and triggers a repaint.
     * @param tokens new count
     */
    void setTokenCount(int tokens);

    /**
     * @brief Sets the place label and triggers a repaint.
     * @param name new label
     */
    void setPlaceName(const QString &name);

    /**
     * @brief Activates or deactivates monitor highlight mode.
     *
     * When active, the fill colour reflects the live token count from the
     * running interpreter rather than the model's initial token count.
     *
     * @param active     true to enable monitor colouring
     * @param liveTokens current live token count (ignored when active is false)
     */
    void setMonitorHighlight(bool active, int liveTokens = 0);

    /**
     * @brief Returns the centre of the item in scene coordinates.
     * @return scenePos() of this item
     */
    QPointF centerPos() const;

protected:
    /**
     * @brief Returns the bounding rect extended below the circle for the name label.
     */
    QRectF boundingRect() const override;

    /** @brief Paints the circle, token count and name label. */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    /** @brief Notifies the editor when the item position changes. */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    /** @brief Opens the place properties dialog on double-click. */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int            m_placeId;      ///< model place ID
    QString        m_name;         ///< display label
    int            m_tokens;       ///< initial token count (shown outside monitor mode)
    bool           m_monitorActive; ///< true when showing live token counts
    int            m_liveTokens;   ///< live token count from interpreter

    GraphicsEditor *m_editor; ///< owning editor for callbacks
};

#endif // GRAPHICS_PLACE_ITEM_H
