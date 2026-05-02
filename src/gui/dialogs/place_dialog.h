/**
 * @file place_dialog.h
 * @brief PlaceDialog - dialog for editing place properties (name, tokens, action).
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QSpinBox;
class QPlainTextEdit;

class PlaceDialog : public QDialog
{
    Q_OBJECT
public:
    // Pre-fill the dialog with existing values
    explicit PlaceDialog(const QString &name, int tokens,
                         const QString &action, QWidget *parent = nullptr);

    QString name()   const;
    int     tokens() const;
    QString action() const;

private:
    QLineEdit     *m_nameEdit;
    QSpinBox      *m_tokensSpin;
    QPlainTextEdit *m_actionEdit;
};
