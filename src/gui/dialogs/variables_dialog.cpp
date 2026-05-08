/**
 * @file variables_dialog.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief VariablesDialog implementation.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor builds three tabs: Inputs (QListWidget), Outputs (QListWidget),
 *      Variables (QTableWidget with Type/Name/Initial-value columns)
 *   2. Each tab has +/- buttons wired to the corresponding add/remove slot
 *   3. inputs() / outputs() drain the list widgets into std::vector<std::string>
 *   4. variables() drains the table, skipping rows with empty name column
 *   5. addVariable() inserts a default row and immediately starts inline editing
 */

#include "variables_dialog.h"
#include "pn_net.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QLabel>

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief constructs the dialog pre-populated from the given net
 */
VariablesDialog::VariablesDialog(const PnNet* net, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Net Properties");
    setMinimumSize(480, 360);

    auto* mainLayout = new QVBoxLayout(this);
    auto* tabs = new QTabWidget(this);

    // ---- Tab: Inputs ----
    auto* inputWidget = new QWidget;
    auto* inputLayout = new QVBoxLayout(inputWidget);
    m_inputList = new QListWidget;
    if (net) for (const auto& s : net->inputs())
        m_inputList->addItem(QString::fromStdString(s));
    inputLayout->addWidget(new QLabel("Input names (events sent from GUI to interpreter):"));
    inputLayout->addWidget(m_inputList);
    auto* inBtns = new QHBoxLayout;
    auto* btnAddIn  = new QPushButton("+");
    auto* btnRemIn  = new QPushButton("−");
    inBtns->addWidget(btnAddIn); inBtns->addWidget(btnRemIn); inBtns->addStretch();
    inputLayout->addLayout(inBtns);
    connect(btnAddIn, &QPushButton::clicked, this, &VariablesDialog::addInput);
    connect(btnRemIn, &QPushButton::clicked, this, &VariablesDialog::removeInput);
    tabs->addTab(inputWidget, "Inputs");

    // ---- Tab: Outputs ----
    auto* outputWidget = new QWidget;
    auto* outputLayout = new QVBoxLayout(outputWidget);
    m_outputList = new QListWidget;
    if (net) for (const auto& s : net->outputs())
        m_outputList->addItem(QString::fromStdString(s));
    outputLayout->addWidget(new QLabel("Output names (values sent from interpreter to GUI):"));
    outputLayout->addWidget(m_outputList);
    auto* outBtns = new QHBoxLayout;
    auto* btnAddOut = new QPushButton("+");
    auto* btnRemOut = new QPushButton("−");
    outBtns->addWidget(btnAddOut); outBtns->addWidget(btnRemOut); outBtns->addStretch();
    outputLayout->addLayout(outBtns);
    connect(btnAddOut, &QPushButton::clicked, this, &VariablesDialog::addOutput);
    connect(btnRemOut, &QPushButton::clicked, this, &VariablesDialog::removeOutput);
    tabs->addTab(outputWidget, "Outputs");

    // ---- Tab: Variables ----
    auto* varWidget = new QWidget;
    auto* varLayout = new QVBoxLayout(varWidget);
    m_varTable = new QTableWidget(0, 3);
    m_varTable->setHorizontalHeaderLabels({"Type", "Name", "Initial value"});
    m_varTable->horizontalHeader()->setStretchLastSection(true);
    m_varTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    if (net) for (const auto& v : net->variables()) {
        int row = m_varTable->rowCount();
        m_varTable->insertRow(row);
        m_varTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(v.type)));
        m_varTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(v.name)));
        m_varTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(v.value)));
    }
    varLayout->addWidget(new QLabel("C++ variables available in guards and actions:"));
    varLayout->addWidget(m_varTable);
    auto* varBtns = new QHBoxLayout;
    auto* btnAddVar = new QPushButton("+");
    auto* btnRemVar = new QPushButton("−");
    varBtns->addWidget(btnAddVar); varBtns->addWidget(btnRemVar); varBtns->addStretch();
    varLayout->addLayout(varBtns);
    connect(btnAddVar, &QPushButton::clicked, this, &VariablesDialog::addVariable);
    connect(btnRemVar, &QPushButton::clicked, this, &VariablesDialog::removeVariable);
    tabs->addTab(varWidget, "Variables");

    mainLayout->addWidget(tabs);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);
}

///////////////////////////////////////////////////////////////////////////////
// Accessors
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief returns all input names from the list widget
 */
std::vector<std::string> VariablesDialog::inputs() const {
    std::vector<std::string> v;
    for (int i = 0; i < m_inputList->count(); i++)
        v.push_back(m_inputList->item(i)->text().toStdString());
    return v;
}

/**
 * @brief returns all output names from the list widget
 */
std::vector<std::string> VariablesDialog::outputs() const {
    std::vector<std::string> v;
    for (int i = 0; i < m_outputList->count(); i++)
        v.push_back(m_outputList->item(i)->text().toStdString());
    return v;
}

/**
 * @brief returns all variables from the table, skipping rows with empty name
 */
std::vector<Variable> VariablesDialog::variables() const {
    std::vector<Variable> v;
    for (int i = 0; i < m_varTable->rowCount(); i++) {
        Variable var;
        var.type  = m_varTable->item(i,0) ? m_varTable->item(i,0)->text().toStdString() : "int";
        var.name  = m_varTable->item(i,1) ? m_varTable->item(i,1)->text().toStdString() : "";
        var.value = m_varTable->item(i,2) ? m_varTable->item(i,2)->text().toStdString() : "0";
        if (!var.name.empty()) v.push_back(var);
    }
    return v;
}

///////////////////////////////////////////////////////////////////////////////
// Slots
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief prompts for a name and appends it to the inputs list
 */
void VariablesDialog::addInput() {
    bool ok;
    QString name = QInputDialog::getText(this, "Add Input", "Input name:", QLineEdit::Normal, "", &ok);
    if (ok && !name.trimmed().isEmpty())
        m_inputList->addItem(name.trimmed());
}

/** @brief removes the currently selected input */
void VariablesDialog::removeInput() {
    delete m_inputList->takeItem(m_inputList->currentRow());
}

/**
 * @brief prompts for a name and appends it to the outputs list
 */
void VariablesDialog::addOutput() {
    bool ok;
    QString name = QInputDialog::getText(this, "Add Output", "Output name:", QLineEdit::Normal, "", &ok);
    if (ok && !name.trimmed().isEmpty())
        m_outputList->addItem(name.trimmed());
}

/** @brief removes the currently selected output */
void VariablesDialog::removeOutput() {
    delete m_outputList->takeItem(m_outputList->currentRow());
}

/**
 * @brief inserts a default variable row and starts inline editing on the name cell
 */
void VariablesDialog::addVariable() {
    int row = m_varTable->rowCount();
    m_varTable->insertRow(row);
    m_varTable->setItem(row, 0, new QTableWidgetItem("int"));
    m_varTable->setItem(row, 1, new QTableWidgetItem("var_name"));
    m_varTable->setItem(row, 2, new QTableWidgetItem("0"));
    m_varTable->editItem(m_varTable->item(row, 1)); // focus name cell for editing
}

/** @brief removes the currently selected variable row */
void VariablesDialog::removeVariable() {
    int row = m_varTable->currentRow();
    if (row >= 0) m_varTable->removeRow(row);
}
