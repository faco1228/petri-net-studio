/**
 * @file new_net_dialog.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief NewNetDialog implementation.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor builds a QFormLayout with a name QLineEdit and a comment QTextEdit
 *   2. Standard Ok/Cancel QDialogButtonBox is wired to accept/reject
 *   3. netName() / netComment() trim and return the widget text
 */

#include "new_net_dialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QTextEdit>

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief constructs the dialog with empty fields
 */
NewNetDialog::NewNetDialog(QWidget *parent)
    : QDialog(parent)
    , m_nameEdit(new QLineEdit(this))
    , m_commentEdit(new QTextEdit(this))
{
    setWindowTitle("New Petri Net");
    setMinimumWidth(360);

    m_nameEdit->setPlaceholderText("e.g. MyNet");
    m_commentEdit->setPlaceholderText("Optional description...");
    m_commentEdit->setFixedHeight(80);

    auto *form = new QFormLayout;
    form->addRow("Name:", m_nameEdit);
    form->addRow("Comment:", m_commentEdit);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief returns the trimmed net name */
QString NewNetDialog::netName()    const { return m_nameEdit->text().trimmed(); }

/** @brief returns the trimmed comment (may be empty) */
QString NewNetDialog::netComment() const { return m_commentEdit->toPlainText().trimmed(); }
