/**
 * @file udp_client.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief UdpClient implementation.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. start() creates the QUdpSocket and binds it to localhost:listenPort
 *   2. stop() closes the socket
 *   3. sendInput() / sendQuit() serialize and send command datagrams
 *   4. onReadyRead() is the Qt slot that reads all pending datagrams and emits signals
 */

#include "udp_client.h"
#include <QHostAddress>

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs the UdpClient; the socket is created lazily in start(). */
UdpClient::UdpClient(QObject *parent) : QObject(parent) {}

///////////////////////////////////////////////////////////////////////////////

/** @brief Binds the socket to localhost:listenPort, creating it if necessary. */
void UdpClient::start(quint16 listenPort)
{
    if (!m_socket) {
        // Create socket and wire the readyRead signal
        m_socket = new QUdpSocket(this);
        connect(m_socket, &QUdpSocket::readyRead, this, &UdpClient::onReadyRead);
    }
    if (m_socket->state() != QAbstractSocket::BoundState) {
        m_socket->bind(QHostAddress::LocalHost, listenPort);
    }
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Closes the bound socket so no further datagrams are received. */
void UdpClient::stop()
{
    if (m_socket && m_socket->state() == QAbstractSocket::BoundState)
        m_socket->close();
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Serializes and sends an INPUT datagram to the interpreter. */
void UdpClient::sendInput(const std::string& netName, const std::string& inputName,
                           const std::string& value, quint16 interpreterPort)
{
    // Create socket lazily for sending (may be called before start())
    if (!m_socket) { m_socket = new QUdpSocket(this); }
    InputMsg m; m.net_name = netName; m.input_name = inputName; m.value = value;
    std::string msg = serializeInput(m);
    m_socket->writeDatagram(msg.c_str(), (qint64)msg.size(),
                            QHostAddress::LocalHost, interpreterPort);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Serializes and sends a QUIT datagram to the interpreter. */
void UdpClient::sendQuit(const std::string& netName, quint16 interpreterPort)
{
    if (!m_socket) return;
    QuitMsg m; m.net_name = netName;
    std::string msg = serializeQuit(m);
    m_socket->writeDatagram(msg.c_str(), (qint64)msg.size(),
                            QHostAddress::LocalHost, interpreterPort);
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Reads all pending datagrams and emits the appropriate typed signal.
 *
 * Datagrams are dispatched by MsgType: STATE -> stateReceived,
 * LOG -> logReceived, ANNOUNCE -> announceReceived.
 */
void UdpClient::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize((int)m_socket->pendingDatagramSize());
        m_socket->readDatagram(data.data(), data.size());

        std::string raw   = data.toStdString();
        auto        parts = splitTabs(raw);
        MsgType     type  = parseMsgType(raw);

        // Dispatch to the correct signal based on message type
        switch (type) {
        case MsgType::STATE:
            emit stateReceived(parseState(parts));
            break;
        case MsgType::LOG:
            emit logReceived(parseLog(parts));
            break;
        case MsgType::ANNOUNCE:
            emit announceReceived(parseAnnounce(parts));
            break;
        default:
            break; // ignore INPUT, QUIT and UNKNOWN
        }
    }
}
