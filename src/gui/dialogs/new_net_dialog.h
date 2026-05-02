/**
 * @file new_net_dialog.h
 * @brief NewNetDialog - dialog for creating a new Petri net (name + comment).
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QTextEdit;

class NewNetDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewNetDialog(QWidget *parent = nullptr);

    QString netName()    const;
    QString netComment() const;

private:
    QLineEdit *m_nameEdit;
    QTextEdit *m_commentEdit;
};
