/**
 * @file transition_dialog.h
 * @brief TransitionDialog - dialog for editing transition properties (name, event, guard, delay, action).
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QDialog>

class TransitionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TransitionDialog(QWidget *parent = nullptr);
};
