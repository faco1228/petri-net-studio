/**
 * @file monitor_panel.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief MonitorPanel — live view of net state during interpreter run.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Shows a table of place names and their live token counts
 *   2. Shows a table of variable names and their current values
 *   3. Shows a label listing currently enabled transitions
 *   4. Subscribes to AppController::interpreterStarted/Stopped for status display
 *   5. Uses a MonitorAdapter to receive StateMsg updates from UdpClient
 */

#ifndef MONITOR_PANEL_H
#define MONITOR_PANEL_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include "../inc/udp_protocol.h"

class AppController;
class MonitorAdapter;

/**
 * @brief Widget that displays a live marking snapshot from the running interpreter.
 *
 * Place token counts are refreshed on every incoming StateMsg.  Non-zero
 * token counts are highlighted in green for quick visual inspection.
 * Variable values are shown in a second table below the place table.
 */
class MonitorPanel : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the MonitorPanel and wires it to the controller.
     *
     * @param controller provides the UdpClient and interpreter lifecycle signals
     * @param parent     optional Qt parent
     */
    explicit MonitorPanel(AppController *controller, QWidget *parent = nullptr);

public slots:
    /**
     * @brief Repopulates the place table from the latest state snapshot.
     *
     * Called automatically when the MonitorAdapter forwards a StateMsg.
     *
     * @param msg the state snapshot received from the interpreter
     */
    void onStateUpdated(const StateMsg& msg);

private:
    QTableWidget*   m_placeTable   = nullptr; ///< two-column table: place name | token count
    QTableWidget*   m_varsTable    = nullptr; ///< two-column table: variable name | current value
    QLabel*         m_enabledLabel = nullptr; ///< displays currently enabled transitions
    QLabel*         m_statusLabel  = nullptr; ///< shows "Running" / "Stopped"
    MonitorAdapter* m_adapter      = nullptr; ///< forwards UdpClient signals
};

#endif // MONITOR_PANEL_H
