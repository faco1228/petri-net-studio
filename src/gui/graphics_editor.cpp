/**
 * @file graphics_editor.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of GraphicsEditor.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. setMode() switches the editing mode and updates item flags
 *   2. reloadFromNet() clears the scene and recreates all visual items from the model
 *   3. Mouse events implement place/transition creation and arc drag-drawing
 *   4. deleteSelected() cascades arc item removal when a node is deleted
 *   5. openPlaceDialog/openTransitionDialog/openArcDialog show property dialogs
 */

#include "graphics_editor.h"
#include "graphics_place_item.h"
#include "graphics_transition_item.h"
#include "graphics_arc_item.h"
#include "app_controller.h"
#include "dialogs/place_dialog.h"
#include "dialogs/transition_dialog.h"
#include "dialogs/arc_dialog.h"

#include "../model/pn_net.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QPen>
#include <QSet>
#include <map>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs the editor, creates the scene and sets rendering options. */
GraphicsEditor::GraphicsEditor(AppController *controller, QWidget *parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_controller(controller)
    , m_mode(EditorMode::Select)
    , m_arcSource(nullptr)
    , m_tempLine(nullptr)
{
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);
    setSceneRect(-2000, -2000, 4000, 4000); // large canvas
    setBackgroundBrush(QColor(245, 245, 245));
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Switches mode, cancels any arc drag, and updates item movability flags. */
void GraphicsEditor::setMode(EditorMode mode)
{
    // Cancel any arc drawing in progress
    if (m_tempLine) {
        m_scene->removeItem(m_tempLine);
        delete m_tempLine;
        m_tempLine  = nullptr;
        m_arcSource = nullptr;
    }

    m_mode = mode;

    // Items are only draggable in Select mode to avoid accidental moves
    bool movable = (mode == EditorMode::Select);
    for (QGraphicsItem *item : m_scene->items())
        item->setFlag(QGraphicsItem::ItemIsMovable, movable);

    setDragMode(mode == EditorMode::Select
                ? QGraphicsView::RubberBandDrag
                : QGraphicsView::NoDrag);

    emit modeChanged(mode);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the current editing mode. */
EditorMode GraphicsEditor::mode() const { return m_mode; }

///////////////////////////////////////////////////////////////////////////////

/** @brief Removes all scene items and resets the arc drag state. */
void GraphicsEditor::clearScene()
{
    m_tempLine  = nullptr;
    m_arcSource = nullptr;
    m_scene->clear();
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Clears the scene and recreates all items from the model.
 *
 * Places and transitions are created first, then arcs are added using
 * the already-created node items as endpoints.
 */
void GraphicsEditor::reloadFromNet(PnNet *net)
{
    clearScene();
    if (!net) return;

    // Create place items
    for (const auto &p : net->places()) {
        auto *item = new GraphicsPlaceItem(p->id(),
                                           QString::fromStdString(p->name()),
                                           p->initial_tokens(),
                                           p->pos(), this);
        m_scene->addItem(item);
    }

    // Create transition items
    for (const auto &t : net->transitions()) {
        auto *item = new GraphicsTransitionItem(t->id(),
                                                QString::fromStdString(t->name()),
                                                t->pos(), this);
        m_scene->addItem(item);
    }

    // Create arc items (requires place and transition items to exist)
    for (const auto &a : net->arcs()) {
        GraphicsPlaceItem      *pi = findPlaceItem(a->place_id());
        GraphicsTransitionItem *ti = findTransitionItem(a->transition_id());
        if (!pi || !ti) continue;

        auto *arc = new GraphicsArcItem(a->id(), a->type(), pi, ti, this);
        arc->setWeight(a->weight());
        m_scene->addItem(arc);
    }
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Propagates a model position update and refreshes all arc geometries. */
void GraphicsEditor::onItemMoved(int id, bool isPlace, QPointF newPos)
{
    // Persist the new position in the model
    if (m_controller)
        m_controller->updateItemPos(id, isPlace, newPos);

    // Recalculate all arc paths (net sizes are small enough for a full scan)
    for (QGraphicsItem *item : m_scene->items()) {
        auto *arc = dynamic_cast<GraphicsArcItem*>(item);
        if (!arc) continue;
        arc->updateGeometry();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Dialog helpers

/** @brief Opens PlaceDialog pre-filled with the place's current values. */
void GraphicsEditor::openPlaceDialog(int placeId)
{
    if (!m_controller) return;
    Place *p = m_controller->net()->find_place_by_id(placeId);
    if (!p) return;

    PlaceDialog dlg(QString::fromStdString(p->name()),
                    p->initial_tokens(),
                    QString::fromStdString(p->action()),
                    this);
    if (dlg.exec() != QDialog::Accepted) return;

    // Apply changes to the model
    m_controller->updatePlace(placeId,
        dlg.name().toStdString(),
        dlg.tokens(),
        dlg.action().toStdString());

    // Refresh the scene item so it reflects the new name/tokens
    if (auto *item = findPlaceItem(placeId)) {
        item->setPlaceName(dlg.name());
        item->setTokenCount(dlg.tokens());
    }

    emit placeSelected(placeId);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Opens TransitionDialog pre-filled with the transition's current values. */
void GraphicsEditor::openTransitionDialog(int transitionId)
{
    if (!m_controller) return;
    Transition *t = m_controller->net()->find_transition_by_id(transitionId);
    if (!t) return;

    TransitionDialog dlg(QString::fromStdString(t->name()),
                         QString::fromStdString(t->event_name()),
                         QString::fromStdString(t->guard()),
                         QString::fromStdString(t->delay_expr()),
                         QString::fromStdString(t->action()),
                         this);
    if (dlg.exec() != QDialog::Accepted) return;

    m_controller->updateTransition(transitionId,
        dlg.name().toStdString(),
        dlg.eventName().toStdString(),
        dlg.guard().toStdString(),
        dlg.delayExpr().toStdString(),
        dlg.action().toStdString());

    if (auto *item = findTransitionItem(transitionId))
        item->setTransitionName(dlg.name());

    emit transitionSelected(transitionId);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Opens ArcDialog pre-filled with the arc's current weight. */
void GraphicsEditor::openArcDialog(int arcId)
{
    if (!m_controller) return;
    Arc *a = m_controller->net()->find_arc_by_id(arcId);
    if (!a) return;

    ArcDialog dlg(a->weight(), this);
    if (dlg.exec() != QDialog::Accepted) return;

    m_controller->updateArcWeight(arcId, dlg.weight());

    // Find the arc item in the scene and update its visual weight label
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto *ai = dynamic_cast<GraphicsArcItem*>(item)) {
            if (ai->arcId() == arcId) {
                ai->setWeight(dlg.weight());
                ai->updateGeometry();
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Mouse events

/** @brief Dispatches left-click to the appropriate mode handler. */
void GraphicsEditor::mousePressEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());

    if (event->button() == Qt::LeftButton) {
        switch (m_mode) {
        case EditorMode::AddPlace:
            // Only add if the click is on empty space
            if (!m_scene->itemAt(scenePos, transform()))
                addPlaceAt(scenePos);
            break;

        case EditorMode::AddTransition:
            if (!m_scene->itemAt(scenePos, transform()))
                addTransitionAt(scenePos);
            break;

        case EditorMode::AddArc: {
            // Start the arc drag from the clicked node
            QGraphicsItem *hit = itemAt(scenePos);
            if (hit && !m_arcSource) {
                m_arcSource = hit;
                m_tempLine  = new QGraphicsLineItem(
                    QLineF(hit->scenePos(), scenePos));
                QPen pen(Qt::darkGray, 1.5, Qt::DashLine);
                m_tempLine->setPen(pen);
                m_scene->addItem(m_tempLine);
            }
            break;
        }

        case EditorMode::Select:
        default:
            QGraphicsView::mousePressEvent(event);
            break;
        }
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Moves the temporary arc line endpoint to follow the mouse. */
void GraphicsEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mode == EditorMode::AddArc && m_tempLine && m_arcSource) {
        QPointF scenePos = mapToScene(event->pos());
        m_tempLine->setLine(QLineF(m_arcSource->scenePos(), scenePos));
    }
    QGraphicsView::mouseMoveEvent(event);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Completes or cancels the arc drag on mouse release. */
void GraphicsEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_mode == EditorMode::AddArc && m_arcSource && event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());
        QGraphicsItem *target = itemAt(scenePos);

        // Complete the arc only if released over a different valid node
        if (target && target != m_arcSource)
            tryFinishArc(target);

        // Always remove the temporary preview line
        if (m_tempLine) {
            m_scene->removeItem(m_tempLine);
            delete m_tempLine;
            m_tempLine = nullptr;
        }
        m_arcSource = nullptr;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Handles Delete/Backspace to remove the currently selected items. */
void GraphicsEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
        deleteSelected();
    else
        QGraphicsView::keyPressEvent(event);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Shows a context menu with Delete, Properties and Add actions. */
void GraphicsEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    QGraphicsItem *hit = itemAt(scenePos);

    QMenu menu(this);

    if (hit) {
        menu.addAction("Delete", this, &GraphicsEditor::deleteSelected);
        if (dynamic_cast<GraphicsPlaceItem*>(hit))
            menu.addAction("Properties...", [this, hit]() {
                emit placeSelected(dynamic_cast<GraphicsPlaceItem*>(hit)->placeId());
            });
        else if (dynamic_cast<GraphicsTransitionItem*>(hit))
            menu.addAction("Properties...", [this, hit]() {
                emit transitionSelected(dynamic_cast<GraphicsTransitionItem*>(hit)->transitionId());
            });
    } else {
        // Empty canvas: offer to add elements directly
        menu.addAction("Add Place",      [this, scenePos]() { addPlaceAt(scenePos); });
        menu.addAction("Add Transition", [this, scenePos]() { addTransitionAt(scenePos); });
    }

    menu.exec(event->globalPos());
}

///////////////////////////////////////////////////////////////////////////////
// Private helpers

/** @brief Creates a place in the model and adds a visual item at pos. */
void GraphicsEditor::addPlaceAt(QPointF pos)
{
    if (!m_controller) return;
    int id = m_controller->addPlace(pos);
    if (id < 0) return;

    PnNet *net = m_controller->net();
    Place *p   = net->find_place_by_id(id);
    if (!p) return;

    auto *item = new GraphicsPlaceItem(id,
                                       QString::fromStdString(p->name()),
                                       p->initial_tokens(), pos, this);
    m_scene->addItem(item);

    // Return to Select mode so a follow-up click doesn't immediately add another item
    setMode(EditorMode::Select);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Creates a transition in the model and adds a visual item at pos. */
void GraphicsEditor::addTransitionAt(QPointF pos)
{
    if (!m_controller) return;
    int id = m_controller->addTransition(pos);
    if (id < 0) return;

    PnNet      *net = m_controller->net();
    Transition *t   = net->find_transition_by_id(id);
    if (!t) return;

    auto *item = new GraphicsTransitionItem(id,
                                            QString::fromStdString(t->name()),
                                            pos, this);
    m_scene->addItem(item);

    setMode(EditorMode::Select);
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Validates the arc direction and creates the arc in model and scene.
 *
 * Valid combinations: place->transition (INPUT) or transition->place (OUTPUT).
 * All other combinations (e.g. place->place) are silently ignored.
 */
void GraphicsEditor::tryFinishArc(QGraphicsItem *target)
{
    if (!m_controller) return;

    auto *srcPlace = dynamic_cast<GraphicsPlaceItem*>(m_arcSource);
    auto *srcTrans = dynamic_cast<GraphicsTransitionItem*>(m_arcSource);
    auto *dstPlace = dynamic_cast<GraphicsPlaceItem*>(target);
    auto *dstTrans = dynamic_cast<GraphicsTransitionItem*>(target);

    ArcType                type;
    GraphicsPlaceItem      *pi = nullptr;
    GraphicsTransitionItem *ti = nullptr;

    if (srcPlace && dstTrans) {
        type = ArcType::INPUT;
        pi   = srcPlace;
        ti   = dstTrans;
    } else if (srcTrans && dstPlace) {
        type = ArcType::OUTPUT;
        pi   = dstPlace;
        ti   = srcTrans;
    } else {
        return; // place->place or transition->transition — invalid
    }

    int arcId = m_controller->addArc(type, pi->placeId(), ti->transitionId());
    if (arcId < 0) return;

    auto *arc = new GraphicsArcItem(arcId, type, pi, ti, this);
    m_scene->addItem(arc);
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Removes selected items from the model and scene.
 *
 * When a place or transition is deleted, all arcs connected to it are
 * cascade-deleted by the model; this method also removes their scene items.
 */
void GraphicsEditor::deleteSelected()
{
    // Collect arc IDs that will be cascade-deleted by the model so we can
    // also remove their scene items
    QSet<int> arcIdsToRemove;

    for (QGraphicsItem *item : m_scene->selectedItems()) {
        if (auto *pi = dynamic_cast<GraphicsPlaceItem*>(item)) {
            // Record arcs connected to this place before removing it
            for (QGraphicsItem *si : m_scene->items()) {
                if (auto *ai = dynamic_cast<GraphicsArcItem*>(si)) {
                    Arc *a = m_controller->net()->find_arc_by_id(ai->arcId());
                    if (a && a->place_id() == pi->placeId())
                        arcIdsToRemove.insert(ai->arcId());
                }
            }
            m_controller->removePlace(pi->placeId());
        } else if (auto *ti = dynamic_cast<GraphicsTransitionItem*>(item)) {
            for (QGraphicsItem *si : m_scene->items()) {
                if (auto *ai = dynamic_cast<GraphicsArcItem*>(si)) {
                    Arc *a = m_controller->net()->find_arc_by_id(ai->arcId());
                    if (a && a->transition_id() == ti->transitionId())
                        arcIdsToRemove.insert(ai->arcId());
                }
            }
            m_controller->removeTransition(ti->transitionId());
        } else if (auto *ai = dynamic_cast<GraphicsArcItem*>(item)) {
            m_controller->removeArc(ai->arcId());
        }
    }

    // Remove orphaned arc scene items that were cascade-deleted by the model
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto *ai = dynamic_cast<GraphicsArcItem*>(item)) {
            if (arcIdsToRemove.contains(ai->arcId())) {
                m_scene->removeItem(ai);
                delete ai;
            }
        }
    }

    // Remove the originally selected node items
    for (QGraphicsItem *item : m_scene->selectedItems()) {
        m_scene->removeItem(item);
        delete item;
    }
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the topmost place or transition item at scenePos, skipping arcs. */
QGraphicsItem* GraphicsEditor::itemAt(QPointF scenePos) const
{
    for (QGraphicsItem *item : m_scene->items(scenePos)) {
        if (dynamic_cast<GraphicsPlaceItem*>(item))      return item;
        if (dynamic_cast<GraphicsTransitionItem*>(item)) return item;
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Scans the scene for the GraphicsPlaceItem with the given ID. */
GraphicsPlaceItem* GraphicsEditor::findPlaceItem(int id) const
{
    for (QGraphicsItem *item : m_scene->items()) {
        auto *pi = dynamic_cast<GraphicsPlaceItem*>(item);
        if (pi && pi->placeId() == id) return pi;
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Scans the scene for the GraphicsTransitionItem with the given ID. */
GraphicsTransitionItem* GraphicsEditor::findTransitionItem(int id) const
{
    for (QGraphicsItem *item : m_scene->items()) {
        auto *ti = dynamic_cast<GraphicsTransitionItem*>(item);
        if (ti && ti->transitionId() == id) return ti;
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Updates live token counts on all place items from a StateMsg.
 *
 * Iterates over all GraphicsPlaceItem objects in the scene and calls
 * setMonitorHighlight() with the token count from the state snapshot.
 * Place names that are not in the snapshot are left unchanged.
 */
void GraphicsEditor::onStateUpdated(const StateMsg &msg)
{
    // Parse marking_json: {"P1":2,"P2":0} into a name->count map
    std::map<std::string, int> tokenMap;
    std::string json = msg.marking_json;
    if (!json.empty() && json.front() == '{') json = json.substr(1);
    if (!json.empty() && json.back()  == '}') json.pop_back();

    std::istringstream ss(json);
    std::string pair;
    while (std::getline(ss, pair, ',')) {
        auto colon = pair.rfind(':');
        if (colon == std::string::npos) continue;
        std::string name = pair.substr(0, colon);
        // Strip surrounding quotes from the key
        if (name.size() >= 2 && name.front() == '"') name = name.substr(1, name.size() - 2);
        try { tokenMap[name] = std::stoi(pair.substr(colon + 1)); } catch (...) {}
    }

    for (QGraphicsItem *item : m_scene->items()) {
        auto *pi = dynamic_cast<GraphicsPlaceItem*>(item);
        if (!pi) continue;
        auto it = tokenMap.find(pi->placeName().toStdString());
        if (it != tokenMap.end())
            pi->setMonitorHighlight(true, it->second);
    }
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Clears monitor highlight on all place items.
 *
 * Called when the interpreter stops; reverts place items to showing their
 * initial token counts with no special colouring.
 */
void GraphicsEditor::clearMonitorHighlight()
{
    for (QGraphicsItem *item : m_scene->items()) {
        auto *pi = dynamic_cast<GraphicsPlaceItem*>(item);
        if (pi) pi->setMonitorHighlight(false);
    }
}
