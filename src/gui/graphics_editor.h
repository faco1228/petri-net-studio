/**
 * @file graphics_editor.h
 * @brief GraphicsEditor - QGraphicsView-based editor for drawing Petri net diagrams.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsLineItem>

class AppController;
class PnNet;
class GraphicsPlaceItem;
class GraphicsTransitionItem;
class GraphicsArcItem;

enum class EditorMode {
    Select,
    AddPlace,
    AddTransition,
    AddArc
};

class GraphicsEditor : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsEditor(AppController *controller, QWidget *parent = nullptr);

    void setMode(EditorMode mode);
    EditorMode mode() const;

    // Rebuilds the scene from the current net in AppController
    void reloadFromNet(PnNet *net);
    void clearScene();

    // Called by GraphicsPlaceItem / GraphicsTransitionItem when moved
    void onItemMoved(int id, bool isPlace, QPointF newPos);

    // Open properties dialog for the given item (called from double-click handlers)
    void openPlaceDialog(int placeId);
    void openTransitionDialog(int transitionId);
    void openArcDialog(int arcId);

signals:
    void placeSelected(int placeId);
    void transitionSelected(int transitionId);
    void selectionCleared();
    void modeChanged(EditorMode mode);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void addPlaceAt(QPointF scenePos);
    void addTransitionAt(QPointF scenePos);
    void tryFinishArc(QGraphicsItem *target);
    void deleteSelected();

    // Returns the place or transition item under the given scene position, or nullptr
    QGraphicsItem* itemAt(QPointF scenePos) const;

    GraphicsPlaceItem*      findPlaceItem(int id)      const;
    GraphicsTransitionItem* findTransitionItem(int id) const;

    QGraphicsScene *m_scene;
    AppController  *m_controller;
    EditorMode      m_mode;

    // State during arc drawing
    QGraphicsItem     *m_arcSource;   // place or transition item where drag started
    QGraphicsLineItem *m_tempLine;    // dashed preview line while dragging
};
