/**
 * @file new_net_dialog.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief NewNetDialog — modal dialog for creating a new Petri net.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. User enters a net name (required) and optional free-text comment
 *   2. On OK the caller reads netName() / netComment() and passes them to AppController
 */

#ifndef NEW_NET_DIALOG_H
#define NEW_NET_DIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;
class QTextEdit;

/**
 * @brief Modal dialog for entering a new net name and optional comment.
 */
class NewNetDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief constructs the dialog with empty fields
     * @param parent owning widget
     */
    explicit NewNetDialog(QWidget *parent = nullptr);

    /**
     * @brief returns the trimmed net name entered by the user
     */
    QString netName()    const;

    /**
     * @brief returns the trimmed comment entered by the user (may be empty)
     */
    QString netComment() const;

private:
    QLineEdit *m_nameEdit;    ///< single-line name field
    QTextEdit *m_commentEdit; ///< multi-line comment field
};

#endif // NEW_NET_DIALOG_H
