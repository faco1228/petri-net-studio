/**
 * @file pn_transition.cpp
 * @brief Implementation of the Transition class.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "pn_transition.h"

Transition::Transition(int id, const std::string &name, QPointF pos)
    : m_id(id)
    , m_name(name)
    , m_pos(pos)
{}

int Transition::getId() const { return m_id; }

const std::string& Transition::getName() const { return m_name; }
void Transition::setName(const std::string &name) { m_name = name; }

QPointF Transition::getPos() const { return m_pos; }
void Transition::setPos(QPointF pos) { m_pos = pos; }

const std::string& Transition::getEventName() const { return m_eventName; }
void Transition::setEventName(const std::string &event) { m_eventName = event; }

const std::string& Transition::getGuard() const { return m_guard; }
void Transition::setGuard(const std::string &guard) { m_guard = guard; }

const std::string& Transition::getDelayExpr() const { return m_delayExpr; }
void Transition::setDelayExpr(const std::string &expr) { m_delayExpr = expr; }

const std::string& Transition::getAction() const { return m_action; }
void Transition::setAction(const std::string &action) { m_action = action; }
