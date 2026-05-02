/**
 * @file arc_dialog.cpp
 * @brief Implementation of ArcDialog.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "arc_dialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSpinBox>

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

int ArcDialog::weight() const { return m_weightSpin->value(); }
