/**
 * @file inject_panel.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief InjectPanel implementation.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor builds a horizontal layout: combo + value field + Send button
 *   2. refreshInputs() re-populates the combo from the net's declared inputs
 *   3. onSend() reads the combo and text field and calls UdpClient::sendInput()
 */

#include "inject_panel.h"
#include "app_controller.h"
#include "../model/pn_net.h"
#include "../network/udp_client.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs the panel layout and subscribes to net-change signals. */
InjectPanel::InjectPanel(AppController* controller, QWidget *parent)
    : QWidget(parent), m_controller(controller)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Inject Input Event:", this));

    auto* row    = new QHBoxLayout();
    m_nameCombo  = new QComboBox(this);
    m_valueEdit  = new QLineEdit("1", this); // default value 1
    auto* btn    = new QPushButton("Send", this);

    row->addWidget(m_nameCombo, 2);
    row->addWidget(m_valueEdit, 1);
    row->addWidget(btn);
    layout->addLayout(row);

    connect(btn, &QPushButton::clicked, this, &InjectPanel::onSend);

    if (controller) {
        // Keep combo in sync whenever the net changes
        connect(controller, &AppController::netLoaded,  this, &InjectPanel::refreshInputs);
        connect(controller, &AppController::netChanged, this, &InjectPanel::refreshInputs);
    }
    refreshInputs();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Clears the combo and refills it from the current net's input list. */
void InjectPanel::refreshInputs()
{
    m_nameCombo->clear();
    if (!m_controller || !m_controller->net()) return;
    for (const auto& inp : m_controller->net()->inputs())
        m_nameCombo->addItem(QString::fromStdString(inp));
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Reads the combo selection and value field, then sends an INPUT datagram. */
void InjectPanel::onSend()
{
    // Only send when the interpreter is actually running
    if (!m_controller || !m_controller->isInterpreterRunning()) return;
    std::string name  = m_nameCombo->currentText().toStdString();
    std::string value = m_valueEdit->text().toStdString();
    if (name.empty()) return;
    m_controller->udpClient()->sendInput(
        m_controller->net()->name(), name, value, 7000);
}
