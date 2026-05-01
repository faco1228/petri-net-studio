/**
 * @file graphics_editor.h
 * @brief GraphicsEditor - QGraphicsView-based editor for drawing Petri net diagrams.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QWidget>

class AppController;

class GraphicsEditor : public QWidget
{
    Q_OBJECT
public:
    explicit GraphicsEditor(AppController *controller, QWidget *parent = nullptr);
};
