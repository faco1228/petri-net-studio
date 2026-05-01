/**
 * @file pn_place.h
 * @brief Place class - represents a place in the Petri net (data model only).
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <string>
#include <QPointF>

class Place
{
public:
    Place(int id, const std::string &name, int initialTokens = 0,
          QPointF pos = {0.0, 0.0}, const std::string &action = "");

    int getId() const;

    const std::string& getName() const;
    void setName(const std::string &name);

    int getInitialTokens() const;
    void setInitialTokens(int tokens);

    // Empty string means no action
    const std::string& getAction() const;
    void setAction(const std::string &action);

    // Center position on the canvas
    QPointF getPos() const;
    void setPos(QPointF pos);

private:
    int m_id;
    std::string m_name;
    int m_initialTokens;
    std::string m_action;
    QPointF m_pos;
};
