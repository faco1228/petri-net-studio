/**
 * @file graphics_place_item.h
 * @brief GraphicsPlaceItem - visual representation of a place on the canvas.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QGraphicsEllipseItem>

class GraphicsPlaceItem : public QGraphicsEllipseItem
{
public:
    explicit GraphicsPlaceItem(QGraphicsItem *parent = nullptr);
};
