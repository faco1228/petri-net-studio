/**
 * @file transition_dialog.h
 * @brief TransitionDialog - dialog for editing transition properties.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QPlainTextEdit;

class TransitionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TransitionDialog(const QString &name,
                              const QString &eventName,
                              const QString &guard,
                              const QString &delayExpr,
                              const QString &action,
                              QWidget *parent = nullptr);

    QString name()       const;
    QString eventName()  const;
    QString guard()      const;
    QString delayExpr()  const;
    QString action()     const;

private:
    QLineEdit     *m_nameEdit;
    QLineEdit     *m_eventEdit;
    QLineEdit     *m_guardEdit;
    QLineEdit     *m_delayEdit;
    QPlainTextEdit *m_actionEdit;
};
