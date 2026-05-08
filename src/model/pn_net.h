/**
 * @file pn_net.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief PnNet class — aggregate of all Petri net elements.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Declares the Variable POD struct used for C++ variables embedded in the net
 *   2. Declares PnNet which owns collections of Place, Transition and Arc objects
 *   3. PnNet manages unique IDs for each element type and handles cascade deletion
 */

#ifndef PN_NET_H
#define PN_NET_H

#include <string>
#include <vector>
#include <memory>
#include "pn_place.h"
#include "pn_transition.h"
#include "pn_arc.h"

/**
 * @brief A typed C++ variable embedded inside the Petri net.
 *
 * Variables are available in guard expressions and transition actions
 * in the generated interpreter.
 */
struct Variable {
    std::string type;    ///< C++ type name, e.g. "int" or "double"
    std::string name;    ///< variable identifier
    std::string value;   ///< initial value expression
    std::string comment; ///< optional inline comment written to the .pn file
};

/**
 * @brief Owns and manages all elements of a single Petri net.
 *
 * PnNet acts as the single source of truth for the net structure.
 * It assigns monotonically increasing IDs to places, transitions and arcs,
 * and cascades deletions so that removing a place also removes all arcs
 * that were connected to it.
 */
class PnNet
{
public:
    /** @brief Constructs an empty net with ID counters starting at 1. */
    PnNet();

    // ---- Net metadata ----

    /**
     * @brief Returns the net name.
     * @return const reference to the name string
     */
    const std::string& name() const;

    /**
     * @brief Sets the net name.
     * @param name new name (used as file basename by the code generator)
     */
    void set_name(const std::string &name);

    /**
     * @brief Returns the net comment.
     * @return const reference to the comment string
     */
    const std::string& comment() const;

    /**
     * @brief Sets the net comment.
     * @param comment free-form description written to the .pn file
     */
    void set_comment(const std::string &comment);

    // ---- Inputs / outputs ----

    /**
     * @brief Returns the list of input names.
     *
     * Input names are event identifiers that the GUI can send to the interpreter.
     *
     * @return const reference to the input name vector
     */
    const std::vector<std::string>& inputs() const;

    /**
     * @brief Adds an input name (ignored if already present).
     * @param name input identifier
     */
    void add_input(const std::string &name);

    /**
     * @brief Removes an input name (no-op if not found).
     * @param name input identifier to remove
     */
    void remove_input(const std::string &name);

    /**
     * @brief Returns the list of output names.
     * @return const reference to the output name vector
     */
    const std::vector<std::string>& outputs() const;

    /**
     * @brief Adds an output name (ignored if already present).
     * @param name output identifier
     */
    void add_output(const std::string &name);

    /**
     * @brief Removes an output name (no-op if not found).
     * @param name output identifier to remove
     */
    void remove_output(const std::string &name);

    // ---- Variables ----

    /**
     * @brief Returns the list of embedded C++ variables.
     * @return const reference to the variable vector
     */
    const std::vector<Variable>& variables() const;

    /**
     * @brief Appends a variable to the list.
     * @param var variable to add (no uniqueness check performed)
     */
    void add_variable(const Variable &var);

    /**
     * @brief Removes the variable with the given name (no-op if not found).
     * @param name variable identifier
     */
    void remove_variable(const std::string &name);

    /**
     * @brief Finds a variable by name.
     * @param name variable identifier
     * @return pointer to the Variable, or nullptr if not found
     */
    Variable* find_variable(const std::string &name);

    // ---- Places ----

    /**
     * @brief Creates and adds a new place to the net.
     *
     * @param name          place label
     * @param initialTokens token count at simulation start
     * @param pos           canvas centre position
     * @param action        optional C code executed on token arrival
     * @return raw pointer to the newly created Place (owned by the net)
     */
    Place* add_place(const std::string &name, int initialTokens = 0,
                     QPointF pos = {0.0, 0.0}, const std::string &action = "");

    /**
     * @brief Removes a place and all arcs connected to it.
     * @param id place ID
     */
    void remove_place(int id);

    /**
     * @brief Finds a place by ID (mutable).
     * @param id place ID
     * @return pointer to Place or nullptr
     */
    Place* find_place_by_id(int id);

    /**
     * @brief Finds a place by ID (const).
     * @param id place ID
     * @return const pointer to Place or nullptr
     */
    const Place* find_place_by_id(int id) const;

    /**
     * @brief Finds a place by name (mutable).
     * @param name place label
     * @return pointer to Place or nullptr
     */
    Place* find_place_by_name(const std::string &name);

    /**
     * @brief Finds a place by name (const).
     * @param name place label
     * @return const pointer to Place or nullptr
     */
    const Place* find_place_by_name(const std::string &name) const;

    /**
     * @brief Returns the complete place collection.
     * @return const reference to the unique_ptr vector
     */
    const std::vector<std::unique_ptr<Place>>& places() const;

    // ---- Transitions ----

    /**
     * @brief Creates and adds a new transition to the net.
     *
     * @param name transition label
     * @param pos  canvas centre position
     * @return raw pointer to the newly created Transition (owned by the net)
     */
    Transition* add_transition(const std::string &name, QPointF pos = {0.0, 0.0});

    /**
     * @brief Removes a transition and all arcs connected to it.
     * @param id transition ID
     */
    void remove_transition(int id);

    /**
     * @brief Finds a transition by ID (mutable).
     * @param id transition ID
     * @return pointer to Transition or nullptr
     */
    Transition* find_transition_by_id(int id);

    /**
     * @brief Finds a transition by ID (const).
     * @param id transition ID
     * @return const pointer to Transition or nullptr
     */
    const Transition* find_transition_by_id(int id) const;

    /**
     * @brief Finds a transition by name (mutable).
     * @param name transition label
     * @return pointer to Transition or nullptr
     */
    Transition* find_transition_by_name(const std::string &name);

    /**
     * @brief Finds a transition by name (const).
     * @param name transition label
     * @return const pointer to Transition or nullptr
     */
    const Transition* find_transition_by_name(const std::string &name) const;

    /**
     * @brief Returns the complete transition collection.
     * @return const reference to the unique_ptr vector
     */
    const std::vector<std::unique_ptr<Transition>>& transitions() const;

    // ---- Arcs ----

    /**
     * @brief Creates and adds a new arc to the net.
     *
     * @param type         INPUT (place->transition) or OUTPUT (transition->place)
     * @param placeId      ID of the connected place
     * @param transitionId ID of the connected transition
     * @param weight       tokens consumed/produced per firing (default 1)
     * @return raw pointer to the newly created Arc (owned by the net)
     */
    Arc* add_arc(ArcType type, int placeId, int transitionId, int weight = 1);

    /**
     * @brief Removes the arc with the given ID.
     * @param id arc ID
     */
    void remove_arc(int id);

    /**
     * @brief Finds an arc by ID.
     * @param id arc ID
     * @return pointer to Arc or nullptr
     */
    Arc* find_arc_by_id(int id);

    /**
     * @brief Returns all arcs connected to the given place.
     * @param placeId place ID to query
     * @return vector of non-owning Arc pointers
     */
    std::vector<Arc*> arcs_for_place(int placeId) const;

    /**
     * @brief Returns all arcs connected to the given transition.
     * @param transitionId transition ID to query
     * @return vector of non-owning Arc pointers
     */
    std::vector<Arc*> arcs_for_transition(int transitionId) const;

    /**
     * @brief Returns the complete arc collection.
     * @return const reference to the unique_ptr vector
     */
    const std::vector<std::unique_ptr<Arc>>& arcs() const;

    /**
     * @brief Removes all elements and resets all ID counters to 1.
     */
    void clear();

private:
    std::string name_;    ///< net identifier
    std::string comment_; ///< optional description

    std::vector<std::string> inputs_;    ///< declared input event names
    std::vector<std::string> outputs_;   ///< declared output event names
    std::vector<Variable>    variables_; ///< embedded C++ variables

    std::vector<std::unique_ptr<Place>>      places_;      ///< owned place nodes
    std::vector<std::unique_ptr<Transition>> transitions_; ///< owned transition nodes
    std::vector<std::unique_ptr<Arc>>        arcs_;        ///< owned arc edges

    int next_place_id_;      ///< auto-increment counter for place IDs
    int next_transition_id_; ///< auto-increment counter for transition IDs
    int next_arc_id_;        ///< auto-increment counter for arc IDs
};

#endif // PN_NET_H
