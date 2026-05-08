/**
 * @file event_log_view.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief EventLogView implementation.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor builds the layout: monospace QListWidget + Clear button
 *   2. connectController() subscribes to compiler output and UDP log events
 *   3. onLogEntry() formats the LogMsg timestamp and appends a coloured entry
 *   4. Old entries beyond 500 are pruned on each addition
 */

#include "event_log_view.h"
#include "app_controller.h"
#include "monitor_adapter.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QDateTime>

///////////////////////////////////////////////////////////////////////////////

/** @brief Builds the list widget and Clear button. */
EventLogView::EventLogView(QWidget *parent) : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    m_list = new QListWidget(this);
    m_list->setFont(QFont("Monospace", 9)); // fixed-width for alignment
    layout->addWidget(m_list);

    auto* btnClear = new QPushButton("Clear", this);
    connect(btnClear, &QPushButton::clicked, this, &EventLogView::clear);
    layout->addWidget(btnClear);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Subscribes to the controller's UDP log events and compiler output. */
void EventLogView::connectController(AppController* ctrl)
{
    if (!ctrl) return;
    // Use a MonitorAdapter for typed UDP log events
    auto* adapter = new MonitorAdapter(ctrl->udpClient(), this);
    connect(adapter, &MonitorAdapter::logEntry,  this, &EventLogView::onLogEntry);
    // Also show compiler/interpreter startup messages
    connect(ctrl,    &AppController::compileOutput, this, &EventLogView::appendText);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Formats the log message with a timestamp and appends it to the list. */
void EventLogView::onLogEntry(const LogMsg& msg)
{
    // Format: "[hh:mm:ss.zzz] EVENT_TYPE details"
    QString ts   = QDateTime::fromMSecsSinceEpoch(msg.timestamp_ms).toString("hh:mm:ss.zzz");
    QString line = "[" + ts + "] " + QString::fromStdString(msg.event_type)
                 + " " + QString::fromStdString(msg.details);
    m_list->addItem(line);
    m_list->scrollToBottom();
    // Prune oldest entries to keep memory bounded
    while (m_list->count() > 500) delete m_list->takeItem(0);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Appends a plain-text line and scrolls to the bottom. */
void EventLogView::appendText(const QString& text)
{
    m_list->addItem(text);
    m_list->scrollToBottom();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Removes all entries from the list widget. */
void EventLogView::clear()
{
    m_list->clear();
}
