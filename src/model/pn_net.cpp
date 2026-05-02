/**
 * @file pn_net.cpp
 * @brief Implementation of the PnNet class.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "pn_net.h"
#include <algorithm>

PnNet::PnNet()
    : m_nextPlaceId(1)
    , m_nextTransitionId(1)
    , m_nextArcId(1)
{}

const std::string& PnNet::getName() const { return m_name; }
void PnNet::setName(const std::string &name) { m_name = name; }

const std::string& PnNet::getComment() const { return m_comment; }
void PnNet::setComment(const std::string &comment) { m_comment = comment; }

// Inputs

const std::vector<std::string>& PnNet::getInputs() const { return m_inputs; }

void PnNet::addInput(const std::string &name) {
    if (std::find(m_inputs.begin(), m_inputs.end(), name) == m_inputs.end())
        m_inputs.push_back(name);
}

void PnNet::removeInput(const std::string &name) {
    m_inputs.erase(std::remove(m_inputs.begin(), m_inputs.end(), name), m_inputs.end());
}

// Outputs

const std::vector<std::string>& PnNet::getOutputs() const { return m_outputs; }

void PnNet::addOutput(const std::string &name) {
    if (std::find(m_outputs.begin(), m_outputs.end(), name) == m_outputs.end())
        m_outputs.push_back(name);
}

void PnNet::removeOutput(const std::string &name) {
    m_outputs.erase(std::remove(m_outputs.begin(), m_outputs.end(), name), m_outputs.end());
}

// Variables

const std::vector<Variable>& PnNet::getVariables() const { return m_variables; }

void PnNet::addVariable(const Variable &var) {
    m_variables.push_back(var);
}

void PnNet::removeVariable(const std::string &name) {
    m_variables.erase(
        std::remove_if(m_variables.begin(), m_variables.end(),
                       [&](const Variable &v) { return v.name == name; }),
        m_variables.end());
}

Variable* PnNet::findVariable(const std::string &name) {
    for (auto &v : m_variables)
        if (v.name == name) return &v;
    return nullptr;
}

// Places

Place* PnNet::addPlace(const std::string &name, int initialTokens, QPointF pos, const std::string &action) {
    m_places.push_back(std::make_unique<Place>(m_nextPlaceId++, name, initialTokens, pos, action));
    return m_places.back().get();
}

void PnNet::removePlace(int id) {
    m_places.erase(std::remove_if(m_places.begin(), m_places.end(), [id](const std::unique_ptr<Place> &p) { return p->getId() == id; }), m_places.end());
    // Also remove arcs connected to this place
    m_arcs.erase(std::remove_if(m_arcs.begin(), m_arcs.end(), [id](const std::unique_ptr<Arc> &a) { return a->getPlaceId() == id; }), m_arcs.end());
}

Place* PnNet::findPlaceById(int id) {
    for (auto &p : m_places)
        if (p->getId() == id) return p.get();
    return nullptr;
}

Place* PnNet::findPlaceByName(const std::string &name) {
    for (auto &p : m_places)
        if (p->getName() == name) return p.get();
    return nullptr;
}

const Place* PnNet::findPlaceById(int id) const {
    for (const auto &p : m_places)
        if (p->getId() == id) return p.get();
    return nullptr;
}

const Place* PnNet::findPlaceByName(const std::string &name) const {
    for (const auto &p : m_places)
        if (p->getName() == name) return p.get();
    return nullptr;
}

const std::vector<std::unique_ptr<Place>>& PnNet::getPlaces() const { return m_places; }

// Transitions

Transition* PnNet::addTransition(const std::string &name, QPointF pos) {
    m_transitions.push_back(std::make_unique<Transition>(m_nextTransitionId++, name, pos));
    return m_transitions.back().get();
}

void PnNet::removeTransition(int id) {
    m_transitions.erase(std::remove_if(m_transitions.begin(), m_transitions.end(), [id](const std::unique_ptr<Transition> &t) { return t->getId() == id; }), m_transitions.end());
    m_arcs.erase(std::remove_if(m_arcs.begin(), m_arcs.end(), [id](const std::unique_ptr<Arc> &a) { return a->getTransitionId() == id; }), m_arcs.end());
}

Transition* PnNet::findTransitionById(int id) {
    for (auto &t : m_transitions)
        if (t->getId() == id) return t.get();
    return nullptr;
}

Transition* PnNet::findTransitionByName(const std::string &name) {
    for (auto &t : m_transitions)
        if (t->getName() == name) return t.get();
    return nullptr;
}

const std::vector<std::unique_ptr<Transition>>& PnNet::getTransitions() const { return m_transitions; }

const Transition* PnNet::findTransitionById(int id) const {
    for (const auto &t : m_transitions)
        if (t->getId() == id) return t.get();
    return nullptr;
}

const Transition* PnNet::findTransitionByName(const std::string &name) const {
    for (const auto &t : m_transitions)
        if (t->getName() == name) return t.get();
    return nullptr;
}

// Arcs

Arc* PnNet::addArc(ArcType type, int placeId, int transitionId, int weight) {
    m_arcs.push_back(std::make_unique<Arc>(m_nextArcId++, type, placeId, transitionId, weight));
    return m_arcs.back().get();
}

void PnNet::removeArc(int id) {
    m_arcs.erase(std::remove_if(m_arcs.begin(), m_arcs.end(), [id](const std::unique_ptr<Arc> &a) { return a->getId() == id; }), m_arcs.end());
}

Arc* PnNet::findArcById(int id) {
    for (auto &a : m_arcs)
        if (a->getId() == id) return a.get();
    return nullptr;
}

std::vector<Arc*> PnNet::getArcsForPlace(int placeId) const {
    std::vector<Arc*> result;
    for (auto &a : m_arcs)
        if (a->getPlaceId() == placeId) result.push_back(a.get());
    return result;
}

std::vector<Arc*> PnNet::getArcsForTransition(int transitionId) const {
    std::vector<Arc*> result;
    for (auto &a : m_arcs)
        if (a->getTransitionId() == transitionId) result.push_back(a.get());
    return result;
}

const std::vector<std::unique_ptr<Arc>>& PnNet::getArcs() const { return m_arcs; }

// Resets everything

void PnNet::clear() {
    m_places.clear();
    m_transitions.clear();
    m_arcs.clear();
    m_inputs.clear();
    m_outputs.clear();
    m_variables.clear();
    m_name.clear();
    m_comment.clear();
    m_nextPlaceId = 1;
    m_nextTransitionId = 1;
    m_nextArcId = 1;
}
