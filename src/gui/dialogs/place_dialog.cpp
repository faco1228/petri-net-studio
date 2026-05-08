/**
 * @file place_dialog.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief PlaceDialog implementation.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor builds a form with name / initial-tokens / action fields
 *   2. All fields are pre-filled from the constructor arguments
 *   3. name() / tokens() / action() return the current field values
 */

#include "place_dialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPlainTextEdit>
#include <QLabel>

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief constructs the dialog pre-filled with existing place values
 */
PlaceDialog::PlaceDialog(const QString &name, int tokens,
                         const QString &action, QWidget *parent)
    : QDialog(parent)
    , m_nameEdit(new QLineEdit(this))
    , m_tokensSpin(new QSpinBox(this))
    , m_actionEdit(new QPlainTextEdit(this))
{
    setWindowTitle("Place Properties");
    setMinimumWidth(400);

    m_nameEdit->setText(name);

    m_tokensSpin->setMinimum(0);
    m_tokensSpin->setMaximum(9999);
    m_tokensSpin->setValue(tokens);

    m_actionEdit->setPlainText(action);
    m_actionEdit->setFixedHeight(100);
    m_actionEdit->setPlaceholderText(
        "C code executed when a token is added, e.g.:\n"
        "  output(\"out\", tokens(\"P1\"));");

    auto *form = new QFormLayout;
    form->addRow("Name:", m_nameEdit);
    form->addRow("Initial tokens:", m_tokensSpin);
    form->addRow("Action (C code):", m_actionEdit);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief returns the trimmed place name */
QString PlaceDialog::name()   const { return m_nameEdit->text().trimmed(); }

/** @brief returns the initial token count */
int     PlaceDialog::tokens() const { return m_tokensSpin->value(); }

/** @brief returns the trimmed C action code */
QString PlaceDialog::action() const { return m_actionEdit->toPlainText().trimmed(); }
