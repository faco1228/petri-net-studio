/**
 * @file pn_transition.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of the Transition class.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Initialises all Transition members via the constructor initializer list
 *   2. Provides trivial accessor and mutator implementations for all fields
 */

#include "pn_transition.h"

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs a Transition with the given ID, name and canvas position. */
Transition::Transition(int id, const std::string &name, QPointF pos)
    : id_(id)
    , name_(name)
    , pos_(pos)
{}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the unique transition ID. */
int Transition::id() const { return id_; }

/** @brief Returns the transition label. */
const std::string& Transition::name() const { return name_; }

/** @brief Sets the transition label. */
void Transition::set_name(const std::string &name) { name_ = name; }

/** @brief Returns the canvas centre position. */
QPointF Transition::pos() const { return pos_; }

/** @brief Sets the canvas centre position. */
void Transition::set_pos(QPointF pos) { pos_ = pos; }

/** @brief Returns the triggering event name (empty = spontaneous). */
const std::string& Transition::event_name() const { return event_name_; }

/** @brief Sets the triggering event name. */
void Transition::set_event_name(const std::string &event) { event_name_ = event; }

/** @brief Returns the guard expression (empty = no guard). */
const std::string& Transition::guard() const { return guard_; }

/** @brief Sets the guard expression. */
void Transition::set_guard(const std::string &guard) { guard_ = guard; }

/** @brief Returns the delay expression (empty = no delay). */
const std::string& Transition::delay_expr() const { return delay_expr_; }

/** @brief Sets the delay expression. */
void Transition::set_delay_expr(const std::string &expr) { delay_expr_ = expr; }

/** @brief Returns the action code (empty = no action). */
const std::string& Transition::action() const { return action_; }

/** @brief Sets the action code. */
void Transition::set_action(const std::string &action) { action_ = action; }
