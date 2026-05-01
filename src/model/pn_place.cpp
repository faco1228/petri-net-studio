/**
 * @file pn_place.cpp
 * @brief Implementation of the Place class.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "pn_place.h"

Place::Place(int id, const std::string &name, int initialTokens,
             QPointF pos, const std::string &action)
    : m_id(id)
    , m_name(name)
    , m_initialTokens(initialTokens)
    , m_pos(pos)
    , m_action(action)
{}

int Place::getId() const { return m_id; }

const std::string& Place::getName() const { return m_name; }
void Place::setName(const std::string &name) { m_name = name; }

int Place::getInitialTokens() const { return m_initialTokens; }
void Place::setInitialTokens(int tokens) { m_initialTokens = tokens; }

const std::string& Place::getAction() const { return m_action; }
void Place::setAction(const std::string &action) { m_action = action; }

QPointF Place::getPos() const { return m_pos; }
void Place::setPos(QPointF pos) { m_pos = pos; }
