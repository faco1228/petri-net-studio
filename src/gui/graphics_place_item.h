/**
 * @file graphics_place_item.h
 * @brief GraphicsPlaceItem - visual representation of a place (circle with name and token count).
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QGraphicsEllipseItem>
#include <QString>

class GraphicsEditor;

static constexpr double PLACE_RADIUS = 25.0;

class GraphicsPlaceItem : public QGraphicsEllipseItem
{
public:
    GraphicsPlaceItem(int placeId, const QString &name, int tokens,
                      QPointF center, GraphicsEditor *editor);

    int     placeId()    const;
    QString placeName()  const;
    int     tokenCount() const;

    void setTokenCount(int tokens);
    void setPlaceName(const QString &name);

    // Highlight during monitor mode (shows live token count)
    void setMonitorHighlight(bool active, int liveTokens = 0);

    QPointF centerPos() const;

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int     m_placeId;
    QString m_name;
    int     m_tokens;
    bool    m_monitorActive;
    int     m_liveTokens;

    GraphicsEditor *m_editor;
};
