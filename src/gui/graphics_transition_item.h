/**
 * @file graphics_transition_item.h
 * @brief GraphicsTransitionItem - visual representation of a transition on the canvas.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QGraphicsRectItem>

class GraphicsTransitionItem : public QGraphicsRectItem
{
public:
    explicit GraphicsTransitionItem(QGraphicsItem *parent = nullptr);
};
