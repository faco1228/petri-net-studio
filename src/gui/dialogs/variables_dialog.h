/**
 * @file variables_dialog.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief VariablesDialog — dialog for managing net inputs, outputs and variables.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Three-tab dialog: Inputs / Outputs / Variables
 *   2. Each tab shows a list/table pre-populated from the current PnNet
 *   3. +/- buttons add or remove entries; table cells are editable in-place
 *   4. On OK the caller reads inputs() / outputs() / variables() and writes back to model
 */

#ifndef VARIABLES_DIALOG_H
#define VARIABLES_DIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QTableWidget>
#include <vector>
#include <string>
#include "pn_model.h"
#include "pn_net.h"

class PnNet;

/**
 * @brief Modal dialog for editing net-level inputs, outputs and C variables.
 */
class VariablesDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief constructs the dialog pre-populated from the given net
     * @param net    source net (may be nullptr — dialog opens empty)
     * @param parent owning widget
     */
    explicit VariablesDialog(const PnNet* net, QWidget *parent = nullptr);

    /** @brief returns the list of input names after OK */
    std::vector<std::string> inputs()    const;

    /** @brief returns the list of output names after OK */
    std::vector<std::string> outputs()   const;

    /** @brief returns the list of Variables (type/name/value) after OK */
    std::vector<Variable>    variables() const;

private slots:
    void addInput();       ///< prompts for a name and appends it to m_inputList
    void removeInput();    ///< removes the currently selected input
    void addOutput();      ///< prompts for a name and appends it to m_outputList
    void removeOutput();   ///< removes the currently selected output
    void addVariable();    ///< inserts a new row in m_varTable and starts editing
    void removeVariable(); ///< removes the currently selected variable row

private:
    QListWidget*  m_inputList  = nullptr; ///< list of input event names
    QListWidget*  m_outputList = nullptr; ///< list of output value names
    QTableWidget* m_varTable   = nullptr; ///< type / name / initial-value table
};

#endif // VARIABLES_DIALOG_H
