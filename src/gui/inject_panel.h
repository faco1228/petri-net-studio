/**
 * @file inject_panel.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief InjectPanel — UI for injecting input events into the running interpreter.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Shows a combo box populated with the net's declared input names
 *   2. Provides a value text field and a Send button
 *   3. refreshInputs() is called whenever the net changes to keep the combo in sync
 *   4. onSend() transmits an INPUT datagram via AppController::udpClient()
 */

#ifndef INJECT_PANEL_H
#define INJECT_PANEL_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>

class AppController;

/**
 * @brief Dock widget that lets the user inject input events during interpreter execution.
 *
 * The combo box is populated from the net's declared inputs.  Sending is only
 * active while the interpreter is running (isInterpreterRunning()).
 */
class InjectPanel : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the panel and wires it to the controller.
     *
     * @param controller provides access to the net inputs and UdpClient
     * @param parent     optional Qt parent
     */
    explicit InjectPanel(AppController* controller, QWidget *parent = nullptr);

    /**
     * @brief Clears and repopulates the input combo box from the current net.
     *
     * Should be called after any change to the net's input list.
     */
    void refreshInputs();

private slots:
    /**
     * @brief Sends the selected input name and value to the interpreter.
     *
     * No-op if the interpreter is not running or no input is selected.
     */
    void onSend();

private:
    AppController* m_controller = nullptr; ///< provides net inputs and UdpClient
    QComboBox*     m_nameCombo  = nullptr; ///< list of declared input names
    QLineEdit*     m_valueEdit  = nullptr; ///< value to send with the input event
};

#endif // INJECT_PANEL_H
