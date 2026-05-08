/**
 * @file pn_transition.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Transition class — represents a transition in the Petri net (data model only).
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Declares the Transition class with ID, name, canvas position and firing condition fields
 *   2. A firing condition is the combination of event_name, guard expression and delay expression
 *   3. An optional action (C code) is executed each time the transition fires
 */

#ifndef PN_TRANSITION_H
#define PN_TRANSITION_H

#include <string>
#include <QPointF>

/**
 * @brief Represents a single transition node in a Petri net.
 *
 * A transition may fire when:
 *   - all connected input places have enough tokens,
 *   - the optional event_name has been received (INPUT datagram),
 *   - the optional guard expression evaluates to true, and
 *   - the optional delay has elapsed.
 */
class Transition
{
public:
    /**
     * @brief Constructs a transition.
     *
     * @param id   unique identifier assigned by PnNet
     * @param name human-readable transition label
     * @param pos  canvas centre position
     */
    Transition(int id, const std::string &name, QPointF pos = {0.0, 0.0});

    /**
     * @brief Returns the unique transition identifier.
     * @return integer ID
     */
    int id() const;

    /**
     * @brief Returns the transition name.
     * @return const reference to name string
     */
    const std::string& name() const;

    /**
     * @brief Sets the transition name.
     * @param name new label
     */
    void set_name(const std::string &name);

    /**
     * @brief Returns the canvas centre position.
     * @return QPointF in scene coordinates
     */
    QPointF pos() const;

    /**
     * @brief Sets the canvas centre position.
     * @param pos new scene-coordinate position
     */
    void set_pos(QPointF pos);

    /**
     * @brief Returns the event name that triggers this transition.
     *
     * An empty string means the transition fires spontaneously (no event required).
     *
     * @return const reference to event name string
     */
    const std::string& event_name() const;

    /**
     * @brief Sets the triggering event name.
     * @param event input name from the UDP INPUT datagram; empty to disable
     */
    void set_event_name(const std::string &event);

    /**
     * @brief Returns the guard expression (C++ boolean expression).
     *
     * Empty string means no guard (always passes).
     *
     * @return const reference to guard string
     */
    const std::string& guard() const;

    /**
     * @brief Sets the guard expression.
     * @param guard C++ expression that must be true for the transition to fire
     */
    void set_guard(const std::string &guard);

    /**
     * @brief Returns the delay expression.
     *
     * Can be a numeric literal ("5000" ms) or a variable name ("timeout").
     * Empty string means no timed delay.
     *
     * @return const reference to delay expression string
     */
    const std::string& delay_expr() const;

    /**
     * @brief Sets the delay expression.
     * @param expr millisecond value or variable name; empty to disable
     */
    void set_delay_expr(const std::string &expr);

    /**
     * @brief Returns the action code executed when this transition fires.
     * @return const reference to action string (C++ code fragment)
     */
    const std::string& action() const;

    /**
     * @brief Sets the action code.
     * @param action C++ code executed on each firing; empty to clear
     */
    void set_action(const std::string &action);

private:
    int         id_;         ///< unique transition ID
    std::string name_;       ///< transition label
    QPointF     pos_;        ///< canvas centre position

    std::string event_name_; ///< optional triggering input event name
    std::string guard_;      ///< optional C++ guard expression
    std::string delay_expr_; ///< optional delay in ms (literal or variable name)
    std::string action_;     ///< optional C++ code executed on firing
};

#endif // PN_TRANSITION_H
