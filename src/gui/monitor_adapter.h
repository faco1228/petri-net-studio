/**
 * @file monitor_adapter.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief MonitorAdapter — bridges UdpClient signals to GUI monitor widgets.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Connects to the UdpClient passed in the constructor
 *   2. Re-emits stateReceived as stateUpdated and logReceived as logEntry
 *      so multiple widgets can independently connect without coupling to UdpClient
 */

#ifndef MONITOR_ADAPTER_H
#define MONITOR_ADAPTER_H

#include <QObject>
#include "../inc/udp_protocol.h"

class UdpClient;

/**
 * @brief Thin adapter that forwards UdpClient signals to monitor widgets.
 *
 * Decouples MonitorPanel and EventLogView from UdpClient by providing
 * clean stateUpdated / logEntry signals.
 */
class MonitorAdapter : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the adapter and connects to client's signals.
     *
     * If client is nullptr the adapter is inert (no connections are made).
     *
     * @param client UdpClient whose signals are forwarded
     * @param parent optional Qt parent
     */
    explicit MonitorAdapter(UdpClient* client, QObject *parent = nullptr);

signals:
    /** @brief Forwarded from UdpClient::stateReceived. */
    void stateUpdated(const StateMsg& msg);

    /** @brief Forwarded from UdpClient::logReceived. */
    void logEntry(const LogMsg& msg);
};

#endif // MONITOR_ADAPTER_H
