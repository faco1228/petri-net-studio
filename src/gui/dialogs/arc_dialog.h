/**
 * @file arc_dialog.h
 * @brief ArcDialog - dialog for editing arc weight.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QDialog>

class QSpinBox;

class ArcDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ArcDialog(int currentWeight, QWidget *parent = nullptr);

    int weight() const;

private:
    QSpinBox *m_weightSpin;
};
