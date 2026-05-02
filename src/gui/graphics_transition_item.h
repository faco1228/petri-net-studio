/**
 * @file graphics_transition_item.h
 * @brief GraphicsTransitionItem - visual representation of a transition (rectangle with name).
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QGraphicsRectItem>
#include <QString>

class GraphicsEditor;

static constexpr double TRANSITION_W = 60.0;
static constexpr double TRANSITION_H = 30.0;

class GraphicsTransitionItem : public QGraphicsRectItem
{
public:
    GraphicsTransitionItem(int transitionId, const QString &name,
                           QPointF center, GraphicsEditor *editor);

    int     transitionId()   const;
    QString transitionName() const;

    void setTransitionName(const QString &name);

    // Color feedback during monitor mode
    void setEnabled(bool enabled);     // green = can fire
    void setPending(bool pending);     // orange = timer running

    QPointF centerPos() const;

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int     m_transitionId;
    QString m_name;
    bool    m_enabled;
    bool    m_pending;

    GraphicsEditor *m_editor;
};
