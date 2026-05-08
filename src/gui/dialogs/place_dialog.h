/**
 * @file place_dialog.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief PlaceDialog — dialog for editing place properties (name, tokens, action).
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Pre-filled with existing place values passed via constructor
 *   2. On OK the caller reads name() / tokens() / action() and updates the model
 */

#ifndef PLACE_DIALOG_H
#define PLACE_DIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;
class QSpinBox;
class QPlainTextEdit;

/**
 * @brief Modal dialog for editing a Petri net place.
 */
class PlaceDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief constructs the dialog pre-filled with existing place values
     * @param name         current place name
     * @param tokens       current initial token count
     * @param action       current C action code
     * @param parent       owning widget
     */
    explicit PlaceDialog(const QString &name, int tokens,
                         const QString &action, QWidget *parent = nullptr);

    /** @brief returns the trimmed place name */
    QString name()   const;

    /** @brief returns the initial token count */
    int     tokens() const;

    /** @brief returns the trimmed C action code */
    QString action() const;

private:
    QLineEdit      *m_nameEdit;   ///< place name field
    QSpinBox       *m_tokensSpin; ///< initial token count spinner
    QPlainTextEdit *m_actionEdit; ///< C action code editor
};

#endif // PLACE_DIALOG_H
