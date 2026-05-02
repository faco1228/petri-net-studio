/**
 * @file graphics_arc_item.h
 * @brief GraphicsArcItem - directed arc between a place and a transition with arrowhead.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include "../inc/pn_model.h"

class GraphicsPlaceItem;
class GraphicsTransitionItem;
class GraphicsEditor;

class GraphicsArcItem : public QGraphicsPathItem
{
public:
    GraphicsArcItem(int arcId, ArcType type,
                    GraphicsPlaceItem *placeItem,
                    GraphicsTransitionItem *transitionItem,
                    GraphicsEditor *editor = nullptr);

    int     arcId()  const;
    ArcType arcType() const;
    int     weight() const;
    void    setWeight(int w);

    // Recalculates the path from current positions of the connected items.
    // Called after either endpoint moves.
    void updateGeometry();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    // Returns the border intersection point of item's bounding shape
    // in the direction of 'target'
    static QPointF connectionPoint(QGraphicsItem *item, QPointF target);
    static QPainterPath buildArrowHead(QPointF tip, QPointF dir);

    int     m_arcId;
    ArcType m_type;
    int     m_weight;

    GraphicsPlaceItem      *m_placeItem;
    GraphicsTransitionItem *m_transitionItem;
    GraphicsEditor         *m_editor;
};
