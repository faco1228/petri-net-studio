/**
 * @file graphics_arc_item.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief GraphicsArcItem — directed arc between a place and a transition with arrowhead.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Subclasses QGraphicsPathItem to draw a straight line with a filled arrowhead
 *   2. updateGeometry() recalculates endpoint positions on the item borders using
 *      connectionPoint() so the line starts/ends on the visible shape boundary
 *   3. Weight labels are drawn at the midpoint when weight > 1
 *   4. Double-click opens ArcDialog via GraphicsEditor::openArcDialog()
 */

#ifndef GRAPHICS_ARC_ITEM_H
#define GRAPHICS_ARC_ITEM_H

#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include "../inc/pn_model.h"

class GraphicsPlaceItem;
class GraphicsTransitionItem;
class GraphicsEditor;

/**
 * @brief QGraphicsItem that draws a directed arc between a place and a transition.
 *
 * The arc path consists of a straight line from the source item's border to the
 * target item's border, plus a filled arrowhead triangle at the target end.
 */
class GraphicsArcItem : public QGraphicsPathItem
{
public:
    /**
     * @brief Constructs the arc item and computes the initial path.
     *
     * @param arcId         ID matching the corresponding Arc in the model
     * @param type          INPUT (place->transition) or OUTPUT (transition->place)
     * @param placeItem     visual place endpoint
     * @param transitionItem visual transition endpoint
     * @param editor        owning editor (used for callbacks)
     */
    GraphicsArcItem(int arcId, ArcType type,
                    GraphicsPlaceItem *placeItem,
                    GraphicsTransitionItem *transitionItem,
                    GraphicsEditor *editor = nullptr);

    /**
     * @brief Returns the arc ID.
     * @return model arc ID
     */
    int arcId() const;

    /**
     * @brief Returns the arc direction.
     * @return ArcType::INPUT or ArcType::OUTPUT
     */
    ArcType arcType() const;

    /**
     * @brief Returns the arc weight.
     * @return positive integer
     */
    int weight() const;

    /**
     * @brief Sets the arc weight and triggers a repaint.
     * @param w new weight (>= 1)
     */
    void setWeight(int w);

    /**
     * @brief Recomputes the line path from the current positions of the connected items.
     *
     * Must be called after either endpoint moves.
     */
    void updateGeometry();

protected:
    /** @brief Draws the line, arrowhead and optional weight label. */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    /** @brief Opens the arc properties dialog on double-click. */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    /**
     * @brief Returns the border intersection point of item's shape in the direction of target.
     *
     * Handles both circular places and rectangular transitions.
     *
     * @param item   the place or transition item
     * @param target target point in scene coordinates
     * @return intersection point on the item's visible boundary
     */
    static QPointF connectionPoint(QGraphicsItem *item, QPointF target);

    /**
     * @brief Builds a filled arrowhead path pointing toward tip in direction dir.
     *
     * @param tip scene-coordinate tip of the arrowhead
     * @param dir unit direction vector (from shaft toward tip)
     * @return QPainterPath of the arrowhead triangle
     */
    static QPainterPath buildArrowHead(QPointF tip, QPointF dir);

    int     m_arcId;   ///< model arc ID
    ArcType m_type;    ///< INPUT or OUTPUT
    int     m_weight;  ///< tokens consumed/produced per firing

    GraphicsPlaceItem      *m_placeItem;      ///< visual place endpoint
    GraphicsTransitionItem *m_transitionItem; ///< visual transition endpoint
    GraphicsEditor         *m_editor;         ///< owning editor for callbacks
};

#endif // GRAPHICS_ARC_ITEM_H
