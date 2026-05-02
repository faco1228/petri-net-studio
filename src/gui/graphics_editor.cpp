/**
 * @file graphics_editor.cpp
 * @brief Implementation of GraphicsEditor.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
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
    setSceneRect(-2000, -2000, 4000, 4000);
    setBackgroundBrush(QColor(245, 245, 245));
}

void GraphicsEditor::setMode(EditorMode mode)
{
    // Cancel any in-progress arc drawing
    if (m_tempLine) {
        m_scene->removeItem(m_tempLine);
        delete m_tempLine;
        m_tempLine  = nullptr;
        m_arcSource = nullptr;
    }

    m_mode = mode;

    // Items are only draggable in Select mode
    bool movable = (mode == EditorMode::Select);
    for (QGraphicsItem *item : m_scene->items()) {
        item->setFlag(QGraphicsItem::ItemIsMovable, movable);
    }

    setDragMode(mode == EditorMode::Select
                ? QGraphicsView::RubberBandDrag
                : QGraphicsView::NoDrag);

    emit modeChanged(mode);
}

EditorMode GraphicsEditor::mode() const { return m_mode; }

void GraphicsEditor::clearScene()
{
    m_tempLine  = nullptr;
    m_arcSource = nullptr;
    m_scene->clear();
}

void GraphicsEditor::reloadFromNet(PnNet *net)
{
    clearScene();
    if (!net) return;

    // Add places
    for (const auto &p : net->getPlaces()) {
        auto *item = new GraphicsPlaceItem(p->getId(),
                                           QString::fromStdString(p->getName()),
                                           p->getInitialTokens(),
                                           p->getPos(), this);
        m_scene->addItem(item);
    }

    // Add transitions
    for (const auto &t : net->getTransitions()) {
        auto *item = new GraphicsTransitionItem(t->getId(),
                                                QString::fromStdString(t->getName()),
                                                t->getPos(), this);
        m_scene->addItem(item);
    }

    // Add arcs
    for (const auto &a : net->getArcs()) {
        GraphicsPlaceItem      *pi = findPlaceItem(a->getPlaceId());
        GraphicsTransitionItem *ti = findTransitionItem(a->getTransitionId());
        if (!pi || !ti) continue;

        auto *arc = new GraphicsArcItem(a->getId(), a->getType(), pi, ti, this);
        arc->setWeight(a->getWeight());
        m_scene->addItem(arc);
    }
}

void GraphicsEditor::onItemMoved(int id, bool isPlace, QPointF newPos)
{
    // Update model position
    if (m_controller)
        m_controller->updateItemPos(id, isPlace, newPos);

    // Update all arcs connected to this item
    for (QGraphicsItem *item : m_scene->items()) {
        auto *arc = dynamic_cast<GraphicsArcItem*>(item);
        if (!arc) continue;

        // Update all arcs - cheap enough for typical net sizes
        arc->updateGeometry();
    }
}

// ---- Dialog helpers ----

void GraphicsEditor::openPlaceDialog(int placeId)
{
    if (!m_controller) return;
    Place *p = m_controller->net()->findPlaceById(placeId);
    if (!p) return;

    PlaceDialog dlg(QString::fromStdString(p->getName()),
                    p->getInitialTokens(),
                    QString::fromStdString(p->getAction()),
                    this);
    if (dlg.exec() != QDialog::Accepted) return;

    m_controller->updatePlace(placeId,
        dlg.name().toStdString(),
        dlg.tokens(),
        dlg.action().toStdString());

    // Refresh the visual item
    if (auto *item = findPlaceItem(placeId)) {
        item->setPlaceName(dlg.name());
        item->setTokenCount(dlg.tokens());
    }

    emit placeSelected(placeId);
}

void GraphicsEditor::openTransitionDialog(int transitionId)
{
    if (!m_controller) return;
    Transition *t = m_controller->net()->findTransitionById(transitionId);
    if (!t) return;

    TransitionDialog dlg(QString::fromStdString(t->getName()),
                         QString::fromStdString(t->getEventName()),
                         QString::fromStdString(t->getGuard()),
                         QString::fromStdString(t->getDelayExpr()),
                         QString::fromStdString(t->getAction()),
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

void GraphicsEditor::openArcDialog(int arcId)
{
    if (!m_controller) return;
    Arc *a = m_controller->net()->findArcById(arcId);
    if (!a) return;

    ArcDialog dlg(a->getWeight(), this);
    if (dlg.exec() != QDialog::Accepted) return;

    m_controller->updateArcWeight(arcId, dlg.weight());

    // Find and refresh arc item
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

// ---- Mouse events ----

void GraphicsEditor::mousePressEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());

    if (event->button() == Qt::LeftButton) {
        switch (m_mode) {
        case EditorMode::AddPlace:
            if (!m_scene->itemAt(scenePos, transform()))
                addPlaceAt(scenePos);
            break;

        case EditorMode::AddTransition:
            if (!m_scene->itemAt(scenePos, transform()))
                addTransitionAt(scenePos);
            break;

        case EditorMode::AddArc: {
            QGraphicsItem *hit = itemAt(scenePos);
            if (hit && !m_arcSource) {
                // Start arc drag
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

void GraphicsEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mode == EditorMode::AddArc && m_tempLine && m_arcSource) {
        QPointF scenePos = mapToScene(event->pos());
        m_tempLine->setLine(QLineF(m_arcSource->scenePos(), scenePos));
    }
    QGraphicsView::mouseMoveEvent(event);
}

void GraphicsEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_mode == EditorMode::AddArc && m_arcSource && event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());
        QGraphicsItem *target = itemAt(scenePos);

        if (target && target != m_arcSource)
            tryFinishArc(target);

        // Remove temp line regardless
        if (m_tempLine) {
            m_scene->removeItem(m_tempLine);
            delete m_tempLine;
            m_tempLine = nullptr;
        }
        m_arcSource = nullptr;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
        deleteSelected();
    else
        QGraphicsView::keyPressEvent(event);
}

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
        menu.addAction("Add Place",      [this, scenePos]() { addPlaceAt(scenePos); });
        menu.addAction("Add Transition", [this, scenePos]() { addTransitionAt(scenePos); });
    }

    menu.exec(event->globalPos());
}

// ---- Private helpers ----

void GraphicsEditor::addPlaceAt(QPointF pos)
{
    if (!m_controller) return;
    int id = m_controller->addPlace(pos);
    if (id < 0) return;

    PnNet *net = m_controller->net();
    Place *p   = net->findPlaceById(id);
    if (!p) return;

    auto *item = new GraphicsPlaceItem(id,
                                       QString::fromStdString(p->getName()),
                                       p->getInitialTokens(), pos, this);
    m_scene->addItem(item);

    // Switch back to Select so a double-click doesn't add two items
    setMode(EditorMode::Select);
}

void GraphicsEditor::addTransitionAt(QPointF pos)
{
    if (!m_controller) return;
    int id = m_controller->addTransition(pos);
    if (id < 0) return;

    PnNet *net      = m_controller->net();
    Transition *t   = net->findTransitionById(id);
    if (!t) return;

    auto *item = new GraphicsTransitionItem(id,
                                            QString::fromStdString(t->getName()),
                                            pos, this);
    m_scene->addItem(item);

    // Switch back to Select so a double-click doesn't add two items
    setMode(EditorMode::Select);
}

void GraphicsEditor::tryFinishArc(QGraphicsItem *target)
{
    if (!m_controller) return;

    auto *srcPlace  = dynamic_cast<GraphicsPlaceItem*>(m_arcSource);
    auto *srcTrans  = dynamic_cast<GraphicsTransitionItem*>(m_arcSource);
    auto *dstPlace  = dynamic_cast<GraphicsPlaceItem*>(target);
    auto *dstTrans  = dynamic_cast<GraphicsTransitionItem*>(target);

    ArcType type;
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
        return; // place->place or transition->transition — not valid
    }

    int arcId = m_controller->addArc(type, pi->placeId(), ti->transitionId());
    if (arcId < 0) return;

    auto *arc = new GraphicsArcItem(arcId, type, pi, ti, this);
    m_scene->addItem(arc);
}

void GraphicsEditor::deleteSelected()
{
    // Collect arc IDs that will be cascade-deleted by the model
    // so we can remove their scene items too
    QSet<int> arcIdsToRemove;

    for (QGraphicsItem *item : m_scene->selectedItems()) {
        if (auto *pi = dynamic_cast<GraphicsPlaceItem*>(item)) {
            // Collect arcs connected to this place before removing from model
            for (QGraphicsItem *si : m_scene->items()) {
                if (auto *ai = dynamic_cast<GraphicsArcItem*>(si)) {
                    Arc *a = m_controller->net()->findArcById(ai->arcId());
                    if (a && a->getPlaceId() == pi->placeId())
                        arcIdsToRemove.insert(ai->arcId());
                }
            }
            m_controller->removePlace(pi->placeId());
        } else if (auto *ti = dynamic_cast<GraphicsTransitionItem*>(item)) {
            for (QGraphicsItem *si : m_scene->items()) {
                if (auto *ai = dynamic_cast<GraphicsArcItem*>(si)) {
                    Arc *a = m_controller->net()->findArcById(ai->arcId());
                    if (a && a->getTransitionId() == ti->transitionId())
                        arcIdsToRemove.insert(ai->arcId());
                }
            }
            m_controller->removeTransition(ti->transitionId());
        } else if (auto *ai = dynamic_cast<GraphicsArcItem*>(item)) {
            m_controller->removeArc(ai->arcId());
        }
    }

    // Remove orphaned arc items from the scene
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto *ai = dynamic_cast<GraphicsArcItem*>(item)) {
            if (arcIdsToRemove.contains(ai->arcId())) {
                m_scene->removeItem(ai);
                delete ai;
            }
        }
    }

    // Remove the originally selected items
    for (QGraphicsItem *item : m_scene->selectedItems()) {
        m_scene->removeItem(item);
        delete item;
    }
}

QGraphicsItem* GraphicsEditor::itemAt(QPointF scenePos) const
{
    for (QGraphicsItem *item : m_scene->items(scenePos)) {
        if (dynamic_cast<GraphicsPlaceItem*>(item))      return item;
        if (dynamic_cast<GraphicsTransitionItem*>(item)) return item;
    }
    return nullptr;
}

GraphicsPlaceItem* GraphicsEditor::findPlaceItem(int id) const
{
    for (QGraphicsItem *item : m_scene->items()) {
        auto *pi = dynamic_cast<GraphicsPlaceItem*>(item);
        if (pi && pi->placeId() == id) return pi;
    }
    return nullptr;
}

GraphicsTransitionItem* GraphicsEditor::findTransitionItem(int id) const
{
    for (QGraphicsItem *item : m_scene->items()) {
        auto *ti = dynamic_cast<GraphicsTransitionItem*>(item);
        if (ti && ti->transitionId() == id) return ti;
    }
    return nullptr;
}
