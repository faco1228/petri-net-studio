/**
 * @file pn_transition.h
 * @brief Transition class - represents a transition in the Petri net (data model only).
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <string>
#include <QPointF>

class Transition
{
public:
    Transition(int id, const std::string &name, QPointF pos = {0.0, 0.0});

    int getId() const;

    const std::string& getName() const;
    void setName(const std::string &name);

    // Center position on the canvas
    QPointF getPos() const;
    void setPos(QPointF pos);

    // Firing condition - all three parts are optional (empty string = not set)
    const std::string& getEventName() const;
    void setEventName(const std::string &event);

    const std::string& getGuard() const;
    void setGuard(const std::string &guard);

    // Can be a literal ("5000") or a variable name ("timeout")
    const std::string& getDelayExpr() const;
    void setDelayExpr(const std::string &expr);

    // Transition action - C++ code executed on fire
    const std::string& getAction() const;
    void setAction(const std::string &action);

private:
    int m_id;
    std::string m_name;
    QPointF m_pos;

    std::string m_eventName;
    std::string m_guard;
    std::string m_delayExpr;
    std::string m_action;
};
