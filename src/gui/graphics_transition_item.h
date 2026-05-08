/**
 * @file graphics_transition_item.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief GraphicsTransitionItem — visual representation of a transition (rectangle with name).
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Subclasses QGraphicsRectItem to draw a rectangle with the transition name below
 *   2. Supports monitor mode colours: green = enabled, orange = timer pending
 *   3. ItemSendsGeometryChanges notifies GraphicsEditor when the item moves
 *   4. Double-click opens TransitionDialog via GraphicsEditor::openTransitionDialog()
 */

#ifndef GRAPHICS_TRANSITION_ITEM_H
#define GRAPHICS_TRANSITION_ITEM_H

#include <QGraphicsRectItem>
#include <QString>

class GraphicsEditor;

/** @brief Width of the transition rectangle in scene pixels. */
static constexpr double TRANSITION_W = 60.0;

/** @brief Height of the transition rectangle in scene pixels. */
static constexpr double TRANSITION_H = 30.0;

/**
 * @brief QGraphicsItem that draws a Petri net transition as a labelled rectangle.
 *
 * The rectangle is centred at the item's scene position.  The name label
 * appears below the rectangle.  Colour feedback during monitor mode:
 *   - Green:  transition is enabled (can fire)
 *   - Orange: a timed delay is pending
 */
class GraphicsTransitionItem : public QGraphicsRectItem
{
public:
    /**
     * @brief Constructs the transition item at the given centre position.
     *
     * @param transitionId ID matching the corresponding Transition in the model
     * @param name         initial label
     * @param center       scene-coordinate centre position
     * @param editor       owning editor (used for callbacks)
     */
    GraphicsTransitionItem(int transitionId, const QString &name,
                           QPointF center, GraphicsEditor *editor);

    /**
     * @brief Returns the transition ID.
     * @return model transition ID
     */
    int transitionId() const;

    /**
     * @brief Returns the transition label.
     * @return display name
     */
    QString transitionName() const;

    /**
     * @brief Sets the display label and triggers a repaint.
     * @param name new label
     */
    void setTransitionName(const QString &name);

    /**
     * @brief Activates green "enabled" colouring in monitor mode.
     * @param enabled true when this transition has all required input tokens
     */
    void setEnabled(bool enabled);

    /**
     * @brief Activates orange "pending timer" colouring in monitor mode.
     * @param pending true when a timed delay is counting down
     */
    void setPending(bool pending);

    /**
     * @brief Returns the centre of the item in scene coordinates.
     * @return scenePos() of this item
     */
    QPointF centerPos() const;

protected:
    /**
     * @brief Returns the bounding rect extended below the rectangle for the name label.
     */
    QRectF boundingRect() const override;

    /** @brief Paints the rectangle and name label with appropriate fill colour. */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    /** @brief Notifies the editor when the item position changes. */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    /** @brief Opens the transition properties dialog on double-click. */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int            m_transitionId; ///< model transition ID
    QString        m_name;         ///< display label
    bool           m_enabled;      ///< true = can fire (green)
    bool           m_pending;      ///< true = timer running (orange)

    GraphicsEditor *m_editor; ///< owning editor for callbacks
};

#endif // GRAPHICS_TRANSITION_ITEM_H
