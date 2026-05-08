/**
 * @file pn_net.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of the PnNet class.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Manages three owned collections of Place, Transition and Arc objects
 *   2. Assigns monotonically increasing IDs via next_*_id_ counters
 *   3. Cascade-deletes connected arcs when a place or transition is removed
 */

#include "pn_net.h"
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs an empty net with all ID counters starting at 1. */
PnNet::PnNet()
    : next_place_id_(1)
    , next_transition_id_(1)
    , next_arc_id_(1)
{}

///////////////////////////////////////////////////////////////////////////////
// Net metadata

/** @brief Returns the net name. */
const std::string& PnNet::name() const { return name_; }

/** @brief Sets the net name. */
void PnNet::set_name(const std::string &name) { name_ = name; }

/** @brief Returns the net comment. */
const std::string& PnNet::comment() const { return comment_; }

/** @brief Sets the net comment. */
void PnNet::set_comment(const std::string &comment) { comment_ = comment; }

///////////////////////////////////////////////////////////////////////////////
// Inputs

/** @brief Returns the declared input names. */
const std::vector<std::string>& PnNet::inputs() const { return inputs_; }

/** @brief Adds an input name, ignoring duplicates. */
void PnNet::add_input(const std::string &name) {
    // Only insert if not already present
    if (std::find(inputs_.begin(), inputs_.end(), name) == inputs_.end())
        inputs_.push_back(name);
}

/** @brief Removes an input name (no-op if absent). */
void PnNet::remove_input(const std::string &name) {
    inputs_.erase(std::remove(inputs_.begin(), inputs_.end(), name), inputs_.end());
}

///////////////////////////////////////////////////////////////////////////////
// Outputs

/** @brief Returns the declared output names. */
const std::vector<std::string>& PnNet::outputs() const { return outputs_; }

/** @brief Adds an output name, ignoring duplicates. */
void PnNet::add_output(const std::string &name) {
    if (std::find(outputs_.begin(), outputs_.end(), name) == outputs_.end())
        outputs_.push_back(name);
}

/** @brief Removes an output name (no-op if absent). */
void PnNet::remove_output(const std::string &name) {
    outputs_.erase(std::remove(outputs_.begin(), outputs_.end(), name), outputs_.end());
}

///////////////////////////////////////////////////////////////////////////////
// Variables

/** @brief Returns the list of embedded C++ variables. */
const std::vector<Variable>& PnNet::variables() const { return variables_; }

/** @brief Appends a variable, ignoring duplicates (same name already present). */
void PnNet::add_variable(const Variable &var) {
    for (const auto &v : variables_)
        if (v.name == var.name) return; // skip duplicate
    variables_.push_back(var);
}

/** @brief Removes the variable with the given name. */
void PnNet::remove_variable(const std::string &name) {
    variables_.erase(
        std::remove_if(variables_.begin(), variables_.end(),
                       [&](const Variable &v) { return v.name == name; }),
        variables_.end());
}

/** @brief Finds a variable by name and returns a mutable pointer (nullptr if absent). */
Variable* PnNet::find_variable(const std::string &name) {
    for (auto &v : variables_)
        if (v.name == name) return &v;
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// Places

/** @brief Creates a place with an auto-assigned ID and appends it to the collection. */
Place* PnNet::add_place(const std::string &name, int initialTokens, QPointF pos, const std::string &action) {
    places_.push_back(std::make_unique<Place>(next_place_id_++, name, initialTokens, pos, action));
    return places_.back().get();
}

/** @brief Removes a place and cascade-deletes all arcs connected to it. */
void PnNet::remove_place(int id) {
    // Remove the place itself
    places_.erase(std::remove_if(places_.begin(), places_.end(),
        [id](const std::unique_ptr<Place> &p) { return p->id() == id; }), places_.end());
    // Also remove any arc that referenced this place
    arcs_.erase(std::remove_if(arcs_.begin(), arcs_.end(),
        [id](const std::unique_ptr<Arc> &a) { return a->place_id() == id; }), arcs_.end());
}

/** @brief Finds a place by ID (mutable version). */
Place* PnNet::find_place_by_id(int id) {
    for (auto &p : places_)
        if (p->id() == id) return p.get();
    return nullptr;
}

/** @brief Finds a place by name (mutable version). */
Place* PnNet::find_place_by_name(const std::string &name) {
    for (auto &p : places_)
        if (p->name() == name) return p.get();
    return nullptr;
}

/** @brief Finds a place by ID (const version). */
const Place* PnNet::find_place_by_id(int id) const {
    for (const auto &p : places_)
        if (p->id() == id) return p.get();
    return nullptr;
}

/** @brief Finds a place by name (const version). */
const Place* PnNet::find_place_by_name(const std::string &name) const {
    for (const auto &p : places_)
        if (p->name() == name) return p.get();
    return nullptr;
}

/** @brief Returns the full place collection. */
const std::vector<std::unique_ptr<Place>>& PnNet::places() const { return places_; }

///////////////////////////////////////////////////////////////////////////////
// Transitions

/** @brief Creates a transition with an auto-assigned ID. */
Transition* PnNet::add_transition(const std::string &name, QPointF pos) {
    transitions_.push_back(std::make_unique<Transition>(next_transition_id_++, name, pos));
    return transitions_.back().get();
}

/** @brief Removes a transition and cascade-deletes all arcs connected to it. */
void PnNet::remove_transition(int id) {
    transitions_.erase(std::remove_if(transitions_.begin(), transitions_.end(),
        [id](const std::unique_ptr<Transition> &t) { return t->id() == id; }), transitions_.end());
    arcs_.erase(std::remove_if(arcs_.begin(), arcs_.end(),
        [id](const std::unique_ptr<Arc> &a) { return a->transition_id() == id; }), arcs_.end());
}

/** @brief Finds a transition by ID (mutable version). */
Transition* PnNet::find_transition_by_id(int id) {
    for (auto &t : transitions_)
        if (t->id() == id) return t.get();
    return nullptr;
}

/** @brief Finds a transition by name (mutable version). */
Transition* PnNet::find_transition_by_name(const std::string &name) {
    for (auto &t : transitions_)
        if (t->name() == name) return t.get();
    return nullptr;
}

/** @brief Returns the full transition collection. */
const std::vector<std::unique_ptr<Transition>>& PnNet::transitions() const { return transitions_; }

/** @brief Finds a transition by ID (const version). */
const Transition* PnNet::find_transition_by_id(int id) const {
    for (const auto &t : transitions_)
        if (t->id() == id) return t.get();
    return nullptr;
}

/** @brief Finds a transition by name (const version). */
const Transition* PnNet::find_transition_by_name(const std::string &name) const {
    for (const auto &t : transitions_)
        if (t->name() == name) return t.get();
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// Arcs

/** @brief Creates an arc with an auto-assigned ID. */
Arc* PnNet::add_arc(ArcType type, int placeId, int transitionId, int weight) {
    arcs_.push_back(std::make_unique<Arc>(next_arc_id_++, type, placeId, transitionId, weight));
    return arcs_.back().get();
}

/** @brief Removes the arc with the given ID. */
void PnNet::remove_arc(int id) {
    arcs_.erase(std::remove_if(arcs_.begin(), arcs_.end(),
        [id](const std::unique_ptr<Arc> &a) { return a->id() == id; }), arcs_.end());
}

/** @brief Finds an arc by ID. */
Arc* PnNet::find_arc_by_id(int id) {
    for (auto &a : arcs_)
        if (a->id() == id) return a.get();
    return nullptr;
}

/** @brief Returns all arcs connected to the given place. */
std::vector<Arc*> PnNet::arcs_for_place(int placeId) const {
    std::vector<Arc*> result;
    for (auto &a : arcs_)
        if (a->place_id() == placeId) result.push_back(a.get());
    return result;
}

/** @brief Returns all arcs connected to the given transition. */
std::vector<Arc*> PnNet::arcs_for_transition(int transitionId) const {
    std::vector<Arc*> result;
    for (auto &a : arcs_)
        if (a->transition_id() == transitionId) result.push_back(a.get());
    return result;
}

/** @brief Returns the full arc collection. */
const std::vector<std::unique_ptr<Arc>>& PnNet::arcs() const { return arcs_; }

///////////////////////////////////////////////////////////////////////////////
// Reset

/** @brief Removes all elements and resets all ID counters to 1. */
void PnNet::clear() {
    places_.clear();
    transitions_.clear();
    arcs_.clear();
    inputs_.clear();
    outputs_.clear();
    variables_.clear();
    name_.clear();
    comment_.clear();
    next_place_id_      = 1;
    next_transition_id_ = 1;
    next_arc_id_        = 1;
}
