/**
 * @file monitor_adapter.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief MonitorAdapter implementation.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor wires UdpClient::stateReceived -> MonitorAdapter::stateUpdated
 *   2. Constructor wires UdpClient::logReceived   -> MonitorAdapter::logEntry
 */

#include "monitor_adapter.h"
#include "../network/udp_client.h"

///////////////////////////////////////////////////////////////////////////////

/** @brief Connects to client's signals if client is not nullptr. */
MonitorAdapter::MonitorAdapter(UdpClient* client, QObject *parent) : QObject(parent)
{
    if (client) {
        // Forward state snapshots to any connected monitor widget
        connect(client, &UdpClient::stateReceived, this, &MonitorAdapter::stateUpdated);
        // Forward log events to any connected log view
        connect(client, &UdpClient::logReceived,   this, &MonitorAdapter::logEntry);
    }
}
