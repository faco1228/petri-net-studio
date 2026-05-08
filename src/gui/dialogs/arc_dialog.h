/**
 * @file arc_dialog.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief ArcDialog — dialog for editing the weight of an arc.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Shows a spin box pre-filled with the arc's current weight
 *   2. On accept, weight() returns the new value to the caller
 */

#pragma once

#include <QDialog>

class QSpinBox;

/**
 * @brief Modal dialog for editing an arc's token weight.
 *
 * Accepts a weight in the range [1, 999].
 */
class ArcDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the dialog pre-filled with currentWeight.
     *
     * @param currentWeight initial value shown in the spin box
     * @param parent        optional Qt parent
     */
    explicit ArcDialog(int currentWeight, QWidget *parent = nullptr);

    /**
     * @brief Returns the weight entered by the user.
     * @return positive integer in [1, 999]
     */
    int weight() const;

private:
    QSpinBox *m_weightSpin; ///< spin box for the arc weight
};
