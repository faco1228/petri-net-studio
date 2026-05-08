/**
 * @file pn_place.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Place class — represents a place in the Petri net (data model only).
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Declares the Place class with its unique ID, name, token count and canvas position
 *   2. Exposes an optional action string (C code executed when a token enters the place)
 */

#ifndef PN_PLACE_H
#define PN_PLACE_H

#include <string>
#include <QPointF>

/**
 * @brief Represents a single place node in a Petri net.
 *
 * A place stores an initial token count and an optional C-code action that is
 * executed by the generated interpreter each time a token is deposited.
 */
class Place
{
public:
    /**
     * @brief Constructs a place with the given attributes.
     *
     * @param id           unique identifier assigned by PnNet
     * @param name         human-readable place name
     * @param initialTokens number of tokens at t=0
     * @param pos          canvas centre position (used by the editor)
     * @param action       optional C code executed on token arrival
     */
    Place(int id, const std::string &name, int initialTokens = 0,
          QPointF pos = {0.0, 0.0}, const std::string &action = "");

    /**
     * @brief Returns the unique place identifier.
     * @return integer ID
     */
    int id() const;

    /**
     * @brief Returns the place name.
     * @return const reference to name string
     */
    const std::string& name() const;

    /**
     * @brief Sets the place name.
     * @param name new name
     */
    void set_name(const std::string &name);

    /**
     * @brief Returns the initial token count.
     * @return non-negative integer
     */
    int initial_tokens() const;

    /**
     * @brief Sets the initial token count.
     * @param tokens new initial count (must be >= 0)
     */
    void set_initial_tokens(int tokens);

    /**
     * @brief Returns the place action code.
     *
     * An empty string means no action is attached.
     *
     * @return const reference to action string
     */
    const std::string& action() const;

    /**
     * @brief Sets the place action code.
     * @param action C++ code fragment; empty string to clear
     */
    void set_action(const std::string &action);

    /**
     * @brief Returns the centre position of this place on the canvas.
     * @return QPointF in scene coordinates
     */
    QPointF pos() const;

    /**
     * @brief Sets the canvas centre position.
     * @param pos new scene-coordinate position
     */
    void set_pos(QPointF pos);

private:
    int         id_;             ///< unique place ID
    std::string name_;           ///< place label shown in the editor
    int         initial_tokens_; ///< token count at simulation start
    std::string action_;         ///< C code executed on token arrival (may be empty)
    QPointF     pos_;            ///< canvas centre position
};

#endif // PN_PLACE_H
