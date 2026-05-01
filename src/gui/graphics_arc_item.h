/**
 * @file graphics_arc_item.h
 * @brief GraphicsArcItem - visual arc with arrowhead between place and transition.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QGraphicsLineItem>

class GraphicsArcItem : public QGraphicsLineItem
{
public:
    explicit GraphicsArcItem(QGraphicsItem *parent = nullptr);
};
