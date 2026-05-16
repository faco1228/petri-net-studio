/**
 * @file properties_panel.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief PropertiesPanel — inline dock editor for selected Petri net elements.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. QStackedWidget with 4 pages: empty / place / transition / arc
 *   2. showPlace/showTransition/showArc slots populate and switch to the right page
 *   3. Apply buttons call AppController::updatePlace/Transition/ArcWeight
 */

#ifndef PROPERTIES_PANEL_H
#define PROPERTIES_PANEL_H

#include <QWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPlainTextEdit>
#include <QLabel>

class AppController;

/**
 * @brief Inline editor shown in the right dock when a net element is selected.
 *
 * Displays editable fields for the selected place, transition or arc and
 * writes changes back to the model via AppController on Apply.
 * Connected to GraphicsEditor selection signals by MainWindow.
 */
class PropertiesPanel : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the panel with access to the controller for model writes.
     * @param ctrl   application controller (provides net access and update methods)
     * @param parent optional Qt parent
     */
    explicit PropertiesPanel(AppController *ctrl, QWidget *parent = nullptr);

public slots:
    /** @brief Loads and displays properties of the given place. */
    void showPlace(int placeId);

    /** @brief Loads and displays properties of the given transition. */
    void showTransition(int transitionId);

    /** @brief Loads and displays weight and direction of the given arc. */
    void showArc(int arcId);

    /** @brief Switches to the "no selection" page. */
    void showEmpty();

signals:
    /** @brief Emitted after a place edit is applied so the canvas item can refresh. */
    void placeApplied(int placeId);
    /** @brief Emitted after a transition edit is applied so the canvas item can refresh. */
    void transitionApplied(int transitionId);
    /** @brief Emitted after an arc edit is applied so the canvas item can refresh. */
    void arcApplied(int arcId);

private slots:
    /** @brief Reads the place form fields and writes them back to the model. */
    void applyPlace();
    /** @brief Reads the transition form fields and writes them back to the model. */
    void applyTransition();
    /** @brief Reads the arc form fields and writes them back to the model. */
    void applyArc();

private:
    AppController  *m_ctrl  = nullptr;
    QStackedWidget *m_stack = nullptr;

    int m_currentPlaceId      = -1;
    int m_currentTransitionId = -1;
    int m_currentArcId        = -1;

    // Place page widgets
    QLineEdit      *m_placeName   = nullptr;
    QSpinBox       *m_placeTokens = nullptr;
    QPlainTextEdit *m_placeAction = nullptr;

    // Transition page widgets
    QLineEdit      *m_transName   = nullptr;
    QLineEdit      *m_transEvent  = nullptr;
    QLineEdit      *m_transGuard  = nullptr;
    QLineEdit      *m_transDelay  = nullptr;
    QPlainTextEdit *m_transAction = nullptr;

    // Arc page widgets
    QLabel   *m_arcInfo   = nullptr;
    QSpinBox *m_arcWeight = nullptr;
};

#endif // PROPERTIES_PANEL_H
