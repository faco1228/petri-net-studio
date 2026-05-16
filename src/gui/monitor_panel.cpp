/**
 * @file monitor_panel.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief MonitorPanel implementation.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor builds the layout: status label, place table, vars table, enabled-transitions label
 *   2. Wires MonitorAdapter::stateUpdated to onStateUpdated()
 *   3. onStateUpdated() parses marking_json and rebuilds the place table rows
 *   4. onStateUpdated() parses vars_json and rebuilds the variables table rows
 *   5. Non-zero token counts are highlighted with a light-green background
 */

#include "monitor_panel.h"
#include "app_controller.h"
#include "monitor_adapter.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QStringList>

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs the widget layout and connects to the controller's signals. */
MonitorPanel::MonitorPanel(AppController *controller, QWidget *parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    // Status label shown above the tables
    m_statusLabel = new QLabel("Not running", this);
    layout->addWidget(m_statusLabel);

    // Two-column table: place name and current token count
    m_placeTable = new QTableWidget(0, 2, this);
    m_placeTable->setHorizontalHeaderLabels({"Place", "Tokens"});
    m_placeTable->horizontalHeader()->setStretchLastSection(true);
    m_placeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_placeTable);

    // Two-column table: variable name and current value
    m_varsTable = new QTableWidget(0, 2, this);
    m_varsTable->setHorizontalHeaderLabels({"Variable", "Value"});
    m_varsTable->horizontalHeader()->setStretchLastSection(true);
    m_varsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_varsTable);

    // Enabled transitions list below the tables
    m_enabledLabel = new QLabel("Enabled: —", this);
    m_enabledLabel->setWordWrap(true);
    layout->addWidget(m_enabledLabel);

    if (controller) {
        // Create adapter and connect for state updates
        m_adapter = new MonitorAdapter(controller->udpClient(), this);
        connect(m_adapter, &MonitorAdapter::stateUpdated, this, &MonitorPanel::onStateUpdated);

        // Update the status label when the interpreter starts or stops
        connect(controller, &AppController::interpreterStarted, this, [this](){
            m_statusLabel->setText("Running");
        });
        connect(controller, &AppController::interpreterStopped, this, [this](){
            m_statusLabel->setText("Stopped");
        });
    }
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Clears both tables and refills them from the incoming state snapshot.
 *
 * Parses marking_json ({"P1":2,"P2":0,...}) and vars_json ({"timeout":5000,...})
 * using the same quoted-key scan loop.  Non-zero token counts are highlighted
 * in green; variable rows have no special highlight.
 */
void MonitorPanel::onStateUpdated(const StateMsg& msg)
{
    // ---- Place token table ----
    m_placeTable->setRowCount(0);

    std::string json = msg.marking_json;
    size_t pos = 0;
    while (pos < json.size()) {
        // Find the opening quote of the next key
        size_t qs = json.find('"', pos);
        if (qs == std::string::npos) break;
        size_t qe = json.find('"', qs + 1);
        if (qe == std::string::npos) break;
        std::string key = json.substr(qs + 1, qe - qs - 1);

        // Find the ':' and extract the value up to ',' or '}'
        size_t colon = json.find(':', qe);
        if (colon == std::string::npos) break;
        size_t vstart = colon + 1;
        size_t vend   = json.find_first_of(",}", vstart);
        std::string val = json.substr(vstart, vend - vstart);

        // Strip leading whitespace
        while (!val.empty() && (val.front() == ' ' || val.front() == '\n'))
            val = val.substr(1);

        // Insert a new row
        int row = m_placeTable->rowCount();
        m_placeTable->insertRow(row);
        m_placeTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(key)));
        auto* tokItem = new QTableWidgetItem(QString::fromStdString(val));
        // Highlight non-zero token counts with light green
        if (val != "0") tokItem->setBackground(QColor(144, 238, 144));
        m_placeTable->setItem(row, 1, tokItem);

        pos = (vend == std::string::npos) ? json.size() : vend + 1;
    }

    // ---- Variables table ----
    m_varsTable->setRowCount(0);

    json = msg.vars_json;
    pos  = 0;
    while (pos < json.size()) {
        // Find the opening quote of the next key
        size_t qs = json.find('"', pos);
        if (qs == std::string::npos) break;
        size_t qe = json.find('"', qs + 1);
        if (qe == std::string::npos) break;
        std::string key = json.substr(qs + 1, qe - qs - 1);

        // Find the ':' and extract the value up to ',' or '}'
        size_t colon = json.find(':', qe);
        if (colon == std::string::npos) break;
        size_t vstart = colon + 1;
        size_t vend   = json.find_first_of(",}", vstart);
        std::string val = json.substr(vstart, vend - vstart);

        // Strip leading whitespace
        while (!val.empty() && (val.front() == ' ' || val.front() == '\n'))
            val = val.substr(1);

        // Insert a new row
        int row = m_varsTable->rowCount();
        m_varsTable->insertRow(row);
        m_varsTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(key)));
        m_varsTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(val)));

        pos = (vend == std::string::npos) ? json.size() : vend + 1;
    }

    // ---- Enabled-transitions label ----
    m_enabledLabel->setText("Enabled: " +
        (msg.enabled_list.empty() ? QString("—") : QString::fromStdString(msg.enabled_list)));
}
