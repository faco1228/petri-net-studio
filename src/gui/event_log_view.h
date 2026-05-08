/**
 * @file event_log_view.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief EventLogView — scrollable log of interpreter events and compiler output.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Displays a QListWidget with timestamped event log entries
 *   2. connectController() wires AppController signals (compileOutput, logEntry)
 *   3. Limits the list to 500 items, removing oldest entries when full
 */

#ifndef EVENT_LOG_VIEW_H
#define EVENT_LOG_VIEW_H

#include <QWidget>
#include <QListWidget>
#include "../inc/udp_protocol.h"

class AppController;

/**
 * @brief A dock widget that shows interpreter log events and compiler messages.
 *
 * Entries are prepended with a wall-clock timestamp.  The list is capped
 * at 500 items to prevent unbounded memory growth during long runs.
 */
class EventLogView : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the log view with an empty list and a Clear button.
     * @param parent optional Qt parent
     */
    explicit EventLogView(QWidget *parent = nullptr);

    /**
     * @brief Wires this view to AppController signals.
     *
     * Creates a MonitorAdapter internally to receive log entries, and also
     * connects AppController::compileOutput for compiler/startup messages.
     *
     * @param ctrl the application controller to subscribe to
     */
    void connectController(AppController* ctrl);

public slots:
    /**
     * @brief Appends a formatted log entry received from the interpreter.
     *
     * @param msg the log message containing timestamp, event type and details
     */
    void onLogEntry(const LogMsg& msg);

    /**
     * @brief Appends a plain-text line (e.g. compiler output or status).
     *
     * @param text the text line to append
     */
    void appendText(const QString& text);

    /** @brief Clears all entries from the list. */
    void clear();

private:
    QListWidget* m_list = nullptr; ///< scrollable list of log entries
};

#endif // EVENT_LOG_VIEW_H
