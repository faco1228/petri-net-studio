/**
 * @file graphics_editor.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief GraphicsEditor — QGraphicsView-based editor for drawing Petri net diagrams.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Manages a QGraphicsScene populated with GraphicsPlaceItem, GraphicsTransitionItem
 *      and GraphicsArcItem objects
 *   2. Handles mouse events to add/move/delete items and draw arcs
 *   3. Dispatches double-click events to the appropriate properties dialog
 *   4. Implements four editor modes: Select, AddPlace, AddTransition, AddArc
 */

#ifndef GRAPHICS_EDITOR_H
#define GRAPHICS_EDITOR_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsLineItem>

#include "../inc/udp_protocol.h"

class AppController;
class PnNet;
class GraphicsPlaceItem;
class GraphicsTransitionItem;
class GraphicsArcItem;

/**
 * @brief Editing mode for the GraphicsEditor canvas.
 */
enum class EditorMode {
    Select,        ///< rubber-band selection and drag-to-move
    AddPlace,      ///< click empty space to add a place
    AddTransition, ///< click empty space to add a transition
    AddArc         ///< drag from one node to another to create an arc
};

/**
 * @brief Interactive canvas for editing Petri net diagrams.
 *
 * Keeps its QGraphicsScene in sync with the PnNet owned by AppController.
 * All structural changes go through AppController so the model remains
 * the single source of truth.
 */
class GraphicsEditor : public QGraphicsView
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the editor and wires it to the controller.
     *
     * @param controller provides access to the net and editor operations
     * @param parent     optional Qt parent
     */
    explicit GraphicsEditor(AppController *controller, QWidget *parent = nullptr);

    /**
     * @brief Sets the active editing mode.
     *
     * Cancels any in-progress arc drag and updates item movability.
     * Emits modeChanged().
     *
     * @param mode the new editor mode
     */
    void setMode(EditorMode mode);

    /**
     * @brief Returns the current editing mode.
     * @return current EditorMode
     */
    EditorMode mode() const;

    /**
     * @brief Rebuilds the scene from the net provided by AppController.
     *
     * Clears all existing items and re-creates them from the model.
     *
     * @param net the net to visualise (may be nullptr to clear the scene)
     */
    void reloadFromNet(PnNet *net);

    /**
     * @brief Removes all items from the scene.
     */
    void clearScene();

    /**
     * @brief Called by item subclasses when an item is moved on the canvas.
     *
     * Updates the model position and recalculates connected arc geometry.
     *
     * @param id      element ID
     * @param isPlace true for a place, false for a transition
     * @param newPos  new scene-coordinate position
     */
    void onItemMoved(int id, bool isPlace, QPointF newPos);

    /**
     * @brief Opens the PlaceDialog for the specified place.
     *
     * Called from GraphicsPlaceItem::mouseDoubleClickEvent.
     *
     * @param placeId ID of the place to edit
     */
    void openPlaceDialog(int placeId);

    /**
     * @brief Opens the TransitionDialog for the specified transition.
     *
     * Called from GraphicsTransitionItem::mouseDoubleClickEvent.
     *
     * @param transitionId ID of the transition to edit
     */
    void openTransitionDialog(int transitionId);

    /**
     * @brief Opens the ArcDialog for the specified arc.
     *
     * Called from GraphicsArcItem::mouseDoubleClickEvent.
     *
     * @param arcId ID of the arc to edit
     */
    void openArcDialog(int arcId);

public slots:
    /**
     * @brief Updates live token counts on all place items from a StateMsg.
     *
     * Called when the monitor receives a STATE datagram from the interpreter.
     * Sets each GraphicsPlaceItem into monitor-highlight mode with the live count.
     *
     * @param msg the state snapshot from the interpreter
     */
    void onStateUpdated(const StateMsg &msg);

    /**
     * @brief Clears monitor highlight on all place items.
     *
     * Called when the interpreter stops so items revert to showing initial tokens.
     */
    void clearMonitorHighlight();

signals:
    /** @brief Emitted when a place item is selected (e.g. for the properties dock). */
    void placeSelected(int placeId);

    /** @brief Emitted when a transition item is selected. */
    void transitionSelected(int transitionId);

    /** @brief Emitted when the selection is cleared. */
    void selectionCleared();

    /** @brief Emitted after setMode() so the toolbar can stay in sync. */
    void modeChanged(EditorMode mode);

protected:
    /** @brief Handles left-click actions according to the current mode. */
    void mousePressEvent(QMouseEvent *event) override;

    /** @brief Updates the temporary arc preview line while dragging. */
    void mouseMoveEvent(QMouseEvent *event) override;

    /** @brief Completes an arc drag or cancels it. */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /** @brief Handles Delete/Backspace to remove selected items. */
    void keyPressEvent(QKeyEvent *event) override;

    /** @brief Shows a context menu with Add/Delete/Properties actions. */
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    /** @brief Creates a place at pos and adds its item to the scene. */
    void addPlaceAt(QPointF scenePos);

    /** @brief Creates a transition at pos and adds its item to the scene. */
    void addTransitionAt(QPointF scenePos);

    /**
     * @brief Attempts to complete an arc from m_arcSource to target.
     *
     * Only valid if one endpoint is a place and the other is a transition.
     *
     * @param target the item the user released the mouse over
     */
    void tryFinishArc(QGraphicsItem *target);

    /** @brief Removes all selected items and their connected arcs from model and scene. */
    void deleteSelected();

    /**
     * @brief Returns the topmost place or transition item under scenePos.
     *
     * Other item types (arcs, text labels) are skipped.
     *
     * @param scenePos query position in scene coordinates
     * @return matching item or nullptr
     */
    QGraphicsItem* itemAt(QPointF scenePos) const;

    /**
     * @brief Finds the GraphicsPlaceItem with the given place ID.
     * @param id place ID
     * @return pointer to item or nullptr
     */
    GraphicsPlaceItem* findPlaceItem(int id) const;

    /**
     * @brief Finds the GraphicsTransitionItem with the given transition ID.
     * @param id transition ID
     * @return pointer to item or nullptr
     */
    GraphicsTransitionItem* findTransitionItem(int id) const;

    QGraphicsScene *m_scene;      ///< the scene that holds all visual items
    AppController  *m_controller; ///< the application controller
    EditorMode      m_mode;       ///< current editing mode

    QGraphicsItem     *m_arcSource; ///< source item when drawing an arc (nullptr if idle)
    QGraphicsLineItem *m_tempLine;  ///< dashed preview line shown during arc drag
};

#endif // GRAPHICS_EDITOR_H
