/**
 * @file pn_place.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of the Place class.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Initialises all Place members via the constructor initializer list
 *   2. Provides trivial accessor and mutator implementations
 */

#include "pn_place.h"

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs a Place, forwarding all arguments to member fields. */
Place::Place(int id, const std::string &name, int initialTokens, QPointF pos, const std::string &action)
    : id_(id)
    , name_(name)
    , initial_tokens_(initialTokens)
    , action_(action)
    , pos_(pos)
{}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the unique place ID. */
int Place::id() const { return id_; }

/** @brief Returns the place name. */
const std::string& Place::name() const { return name_; }

/** @brief Sets the place name. */
void Place::set_name(const std::string &name) { name_ = name; }

/** @brief Returns the initial token count. */
int Place::initial_tokens() const { return initial_tokens_; }

/** @brief Sets the initial token count. */
void Place::set_initial_tokens(int tokens) { initial_tokens_ = tokens; }

/** @brief Returns the C-code action string (empty = no action). */
const std::string& Place::action() const { return action_; }

/** @brief Sets the C-code action string. */
void Place::set_action(const std::string &action) { action_ = action; }

/** @brief Returns the canvas centre position. */
QPointF Place::pos() const { return pos_; }

/** @brief Sets the canvas centre position. */
void Place::set_pos(QPointF pos) { pos_ = pos; }
