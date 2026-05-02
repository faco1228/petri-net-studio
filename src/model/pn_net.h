/**
 * @file pn_net.h
 * @brief PnNet class - aggregate of all Petri net elements.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include "pn_place.h"
#include "pn_transition.h"
#include "pn_arc.h"

struct Variable {
    std::string type;
    std::string name;
    std::string value;
    std::string comment; // optional, for readability in the .pn file
};

class PnNet
{
public:
    PnNet();

    // Net metadata
    const std::string& getName() const;
    void setName(const std::string &name);

    const std::string& getComment() const;
    void setComment(const std::string &comment);

    // Inputs / outputs (just names, values come at runtime via UDP)
    const std::vector<std::string>& getInputs() const;
    void addInput(const std::string &name);
    void removeInput(const std::string &name);

    const std::vector<std::string>& getOutputs() const;
    void addOutput(const std::string &name);
    void removeOutput(const std::string &name);

    // Internal variables
    const std::vector<Variable>& getVariables() const;
    void addVariable(const Variable &var);
    void removeVariable(const std::string &name);
    Variable* findVariable(const std::string &name);

    // Places
    Place* addPlace(const std::string &name, int initialTokens = 0, QPointF pos = {0.0, 0.0}, const std::string &action = "");
    void removePlace(int id);
    Place* findPlaceById(int id);
    const Place* findPlaceById(int id) const;
    Place* findPlaceByName(const std::string &name);
    const Place* findPlaceByName(const std::string &name) const;
    const std::vector<std::unique_ptr<Place>>& getPlaces() const;

    // Transitions
    Transition* addTransition(const std::string &name, QPointF pos = {0.0, 0.0});
    void removeTransition(int id);
    Transition* findTransitionById(int id);
    const Transition* findTransitionById(int id) const;
    Transition* findTransitionByName(const std::string &name);
    const Transition* findTransitionByName(const std::string &name) const;
    const std::vector<std::unique_ptr<Transition>>& getTransitions() const;

    // Arcs
    Arc* addArc(ArcType type, int placeId, int transitionId, int weight = 1);
    void removeArc(int id);
    Arc* findArcById(int id);
    // Returns all arcs connected to a given place or transition
    std::vector<Arc*> getArcsForPlace(int placeId) const;
    std::vector<Arc*> getArcsForTransition(int transitionId) const;
    const std::vector<std::unique_ptr<Arc>>& getArcs() const;

    // Removes all elements and resets ID counters
    void clear();

private:
    std::string m_name;
    std::string m_comment;

    std::vector<std::string> m_inputs;
    std::vector<std::string> m_outputs;
    std::vector<Variable> m_variables;

    std::vector<std::unique_ptr<Place>> m_places;
    std::vector<std::unique_ptr<Transition>> m_transitions;
    std::vector<std::unique_ptr<Arc>> m_arcs;

    int m_nextPlaceId;
    int m_nextTransitionId;
    int m_nextArcId;
};
