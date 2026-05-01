/**
 * @file variables_dialog.h
 * @brief VariablesDialog - dialog for managing net inputs, outputs and internal variables.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QDialog>

class VariablesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit VariablesDialog(QWidget *parent = nullptr);
};
