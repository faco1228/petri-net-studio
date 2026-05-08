/**
 * @file arc_dialog.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief Implementation of ArcDialog.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor builds a form layout with a single weight spin box
 *   2. Ok/Cancel buttons are connected to accept()/reject()
 */

#include "arc_dialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSpinBox>

///////////////////////////////////////////////////////////////////////////////

/** @brief Builds the dialog layout with the weight spin box pre-filled. */
ArcDialog::ArcDialog(int currentWeight, QWidget *parent)
    : QDialog(parent)
    , m_weightSpin(new QSpinBox(this))
{
    setWindowTitle("Arc Properties");

    m_weightSpin->setMinimum(1);
    m_weightSpin->setMaximum(999);
    m_weightSpin->setValue(currentWeight);

    auto *form = new QFormLayout;
    form->addRow("Weight:", m_weightSpin);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Returns the weight currently shown in the spin box. */
int ArcDialog::weight() const { return m_weightSpin->value(); }
