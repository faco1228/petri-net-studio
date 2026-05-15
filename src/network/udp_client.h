/**
 * @file udp_client.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief UdpClient — Qt UDP socket wrapper for GUI-to-interpreter communication.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Wraps QUdpSocket to listen for incoming STATE/LOG/ANNOUNCE datagrams
 *   2. Provides sendInput() and sendQuit() to send commands to the interpreter
 *   3. Emits typed Qt signals so other GUI components can react to net updates
 */

#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include <QObject>
#include <QUdpSocket>
#include "../inc/udp_protocol.h"

/**
 * @brief Qt-based UDP client used by the GUI to communicate with the interpreter process.
 *
 * Bind the socket with start(), then connect to the typed signals to receive
 * live updates from the running interpreter.
 */
class UdpClient : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the UdpClient.
     * @param parent optional Qt parent
     */
    explicit UdpClient(QObject *parent = nullptr);

    /**
     * @brief Binds the UDP socket to listenPort on localhost.
     *
     * Creates the QUdpSocket on first call.  Safe to call multiple times;
     * does nothing if the socket is already bound.
     *
     * @param listenPort port to listen on (typically UDP_PORT_GUI = 7001)
     */
    void start(quint16 listenPort);

    /**
     * @brief Closes the UDP socket.
     */
    void stop();

    /**
     * @brief Sends an INPUT datagram to the interpreter.
     *
     * @param netName         target net identifier
     * @param inputName       input variable name
     * @param value           string-encoded value to inject
     * @param interpreterPort destination port (default UDP_PORT_INTERPRETER = 7000)
     */
    void sendInput(const std::string& netName, const std::string& inputName,
                   const std::string& value, quint16 interpreterPort = 7000);

    /**
     * @brief Sends a QUIT datagram to the interpreter.
     *
     * @param netName         target net identifier
     * @param interpreterPort destination port (default UDP_PORT_INTERPRETER = 7000)
     */
    void sendQuit(const std::string& netName, quint16 interpreterPort = 7000);

    /**
     * @brief Sends a STEP datagram telling the interpreter to fire one transition set.
     *
     * @param interpreterPort destination port (default UDP_PORT_INTERPRETER = 7000)
     */
    void sendStep(quint16 interpreterPort = 7000);

signals:
    /** @brief Emitted when a STATE datagram is received from the interpreter. */
    void stateReceived(const StateMsg& msg);

    /** @brief Emitted when a LOG datagram is received from the interpreter. */
    void logReceived(const LogMsg& msg);

    /** @brief Emitted when an ANNOUNCE datagram is received from the interpreter. */
    void announceReceived(const AnnounceMsg& msg);

private slots:
    /** @brief Reads and dispatches all pending datagrams from the socket. */
    void onReadyRead();

private:
    QUdpSocket* m_socket = nullptr; ///< underlying Qt UDP socket
};

#endif // UDP_CLIENT_H
