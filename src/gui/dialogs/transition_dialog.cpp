/**
 * @file transition_dialog.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief TransitionDialog implementation.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor builds a form with name / event / guard / delay / action fields
 *   2. All fields are pre-filled from the constructor arguments
 *   3. A hint label explains spontaneous firing and timed transitions
 *   4. Accessors trim and return the field values
 */

#include "transition_dialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QLabel>

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief constructs the dialog pre-filled with existing transition values
 */
TransitionDialog::TransitionDialog(const QString &name,
                                   const QString &eventName,
                                   const QString &guard,
                                   const QString &delayExpr,
                                   const QString &action,
                                   QWidget *parent)
    : QDialog(parent)
    , m_nameEdit(new QLineEdit(this))
    , m_eventEdit(new QLineEdit(this))
    , m_guardEdit(new QLineEdit(this))
    , m_delayEdit(new QLineEdit(this))
    , m_actionEdit(new QPlainTextEdit(this))
{
    setWindowTitle("Transition Properties");
    setMinimumWidth(440);

    m_nameEdit->setText(name);

    m_eventEdit->setText(eventName);
    m_eventEdit->setPlaceholderText("input name, e.g. in");

    m_guardEdit->setText(guard);
    m_guardEdit->setPlaceholderText("C expression, e.g. atoi(valueof(\"in\")) == 1");

    m_delayEdit->setText(delayExpr);
    m_delayEdit->setPlaceholderText("milliseconds or variable, e.g. 5000 or timeout");

    m_actionEdit->setPlainText(action);
    m_actionEdit->setFixedHeight(100);
    m_actionEdit->setPlaceholderText("C code, e.g.:\n  output(\"out\", 1);");

    auto *form = new QFormLayout;
    form->addRow("Name:", m_nameEdit);
    form->addRow("Event (input):", m_eventEdit);
    form->addRow("Guard [C expr]:", m_guardEdit);
    form->addRow("Delay (ms / var):", m_delayEdit);
    form->addRow("Action (C code):", m_actionEdit);

    // Short help label about spontaneous firing and timed transitions
    auto *hint = new QLabel(
        "<small>Leave Event empty for spontaneous firing. "
        "Use @ prefix in .pn for timed transitions.</small>", this);
    hint->setWordWrap(true);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(hint);
    layout->addWidget(buttons);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief returns the trimmed transition name */
QString TransitionDialog::name()      const { return m_nameEdit->text().trimmed(); }

/** @brief returns the trimmed input event name */
QString TransitionDialog::eventName() const { return m_eventEdit->text().trimmed(); }

/** @brief returns the trimmed C guard expression */
QString TransitionDialog::guard()     const { return m_guardEdit->text().trimmed(); }

/** @brief returns the trimmed delay expression */
QString TransitionDialog::delayExpr() const { return m_delayEdit->text().trimmed(); }

/** @brief returns the trimmed C action code */
QString TransitionDialog::action()    const { return m_actionEdit->toPlainText().trimmed(); }
