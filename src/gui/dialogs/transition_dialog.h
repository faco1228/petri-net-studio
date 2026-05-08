/**
 * @file transition_dialog.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief TransitionDialog — dialog for editing transition properties.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Pre-filled with existing transition values passed via constructor
 *   2. Exposes name / event / guard / delayExpr / action fields
 *   3. On OK the caller reads the accessors and updates the model
 */

#ifndef TRANSITION_DIALOG_H
#define TRANSITION_DIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;
class QPlainTextEdit;

/**
 * @brief Modal dialog for editing a Petri net transition.
 */
class TransitionDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief constructs the dialog pre-filled with existing transition values
     * @param name       current transition name
     * @param eventName  input event name (empty = spontaneous)
     * @param guard      C guard expression
     * @param delayExpr  delay in ms or variable name
     * @param action     C action code
     * @param parent     owning widget
     */
    explicit TransitionDialog(const QString &name,
                              const QString &eventName,
                              const QString &guard,
                              const QString &delayExpr,
                              const QString &action,
                              QWidget *parent = nullptr);

    /** @brief returns the trimmed transition name */
    QString name()       const;

    /** @brief returns the trimmed input event name (empty = spontaneous) */
    QString eventName()  const;

    /** @brief returns the trimmed C guard expression */
    QString guard()      const;

    /** @brief returns the trimmed delay expression (ms or variable) */
    QString delayExpr()  const;

    /** @brief returns the trimmed C action code */
    QString action()     const;

private:
    QLineEdit      *m_nameEdit;   ///< transition name field
    QLineEdit      *m_eventEdit;  ///< input event name field
    QLineEdit      *m_guardEdit;  ///< C guard expression field
    QLineEdit      *m_delayEdit;  ///< delay expression field
    QPlainTextEdit *m_actionEdit; ///< C action code editor
};

#endif // TRANSITION_DIALOG_H
