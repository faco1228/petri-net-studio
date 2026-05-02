/**
 * @file new_net_dialog.cpp
 * @brief Implementation of NewNetDialog.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "new_net_dialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QTextEdit>

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

QString NewNetDialog::netName()    const { return m_nameEdit->text().trimmed(); }
QString NewNetDialog::netComment() const { return m_commentEdit->toPlainText().trimmed(); }
