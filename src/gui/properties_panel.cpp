/**
 * @file properties_panel.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief PropertiesPanel implementation.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Constructor builds a QStackedWidget with 4 pages (empty/place/transition/arc)
 *   2. showPlace/showTransition/showArc fetch model data and populate widget fields
 *   3. applyPlace/applyTransition/applyArc write back via AppController update methods
 */

#include "properties_panel.h"
#include "app_controller.h"
#include "../model/pn_net.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>

///////////////////////////////////////////////////////////////////////////////

/** @brief Constructs the panel and builds all four stacked pages. */
PropertiesPanel::PropertiesPanel(AppController *ctrl, QWidget *parent)
    : QWidget(parent)
    , m_ctrl(ctrl)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    m_stack = new QStackedWidget(this);
    layout->addWidget(m_stack);

    // ---- Page 0: nothing selected ----
    auto* emptyLabel = new QLabel("Select an element\nto view its properties.", this);
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setWordWrap(true);
    m_stack->addWidget(emptyLabel);                      // index 0

    // ---- Page 1: place ----
    {
        auto* page    = new QWidget(this);
        auto* vbox    = new QVBoxLayout(page);
        auto* form    = new QFormLayout;
        m_placeName   = new QLineEdit(this);
        m_placeTokens = new QSpinBox(this);
        m_placeTokens->setRange(0, 9999);
        m_placeAction = new QPlainTextEdit(this);
        m_placeAction->setFixedHeight(80);
        m_placeAction->setPlaceholderText("C++ code executed on token arrival");
        form->addRow("Name:", m_placeName);
        form->addRow("Initial tokens:", m_placeTokens);
        form->addRow("Action:", m_placeAction);
        auto* applyBtn = new QPushButton("Apply", this);
        connect(applyBtn, &QPushButton::clicked, this, &PropertiesPanel::applyPlace);
        vbox->addLayout(form);
        vbox->addWidget(applyBtn);
        vbox->addStretch();
        m_stack->addWidget(page);                        // index 1
    }

    // ---- Page 2: transition ----
    {
        auto* page    = new QWidget(this);
        auto* vbox    = new QVBoxLayout(page);
        auto* form    = new QFormLayout;
        m_transName   = new QLineEdit(this);
        m_transEvent  = new QLineEdit(this);
        m_transEvent->setPlaceholderText("empty = spontaneous");
        m_transGuard  = new QLineEdit(this);
        m_transGuard->setPlaceholderText("C++ bool expression");
        m_transDelay  = new QLineEdit(this);
        m_transDelay->setPlaceholderText("ms or variable name");
        m_transAction = new QPlainTextEdit(this);
        m_transAction->setFixedHeight(80);
        m_transAction->setPlaceholderText("C++ code executed on firing");
        form->addRow("Name:", m_transName);
        form->addRow("Event:", m_transEvent);
        form->addRow("Guard:", m_transGuard);
        form->addRow("Delay:", m_transDelay);
        form->addRow("Action:", m_transAction);
        auto* applyBtn = new QPushButton("Apply", this);
        connect(applyBtn, &QPushButton::clicked, this, &PropertiesPanel::applyTransition);
        vbox->addLayout(form);
        vbox->addWidget(applyBtn);
        vbox->addStretch();
        m_stack->addWidget(page);                        // index 2
    }

    // ---- Page 3: arc ----
    {
        auto* page  = new QWidget(this);
        auto* vbox  = new QVBoxLayout(page);
        auto* form  = new QFormLayout;
        m_arcInfo   = new QLabel(this);
        m_arcInfo->setWordWrap(true);
        m_arcWeight = new QSpinBox(this);
        m_arcWeight->setRange(1, 999);
        form->addRow("Direction:", m_arcInfo);
        form->addRow("Weight:", m_arcWeight);
        auto* applyBtn = new QPushButton("Apply", this);
        connect(applyBtn, &QPushButton::clicked, this, &PropertiesPanel::applyArc);
        vbox->addLayout(form);
        vbox->addWidget(applyBtn);
        vbox->addStretch();
        m_stack->addWidget(page);                        // index 3
    }

    m_stack->setCurrentIndex(0);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Switches to the empty page and resets all current-element IDs. */
void PropertiesPanel::showEmpty()
{
    m_currentPlaceId = m_currentTransitionId = m_currentArcId = -1;
    m_stack->setCurrentIndex(0);
}

/**
 * @brief Populates and shows the place page.
 * @param placeId ID of the place to display
 */
void PropertiesPanel::showPlace(int placeId)
{
    if (!m_ctrl) return;
    const Place* p = m_ctrl->net()->find_place_by_id(placeId);
    if (!p) return;
    m_currentPlaceId = placeId;
    m_placeName->setText(QString::fromStdString(p->name()));
    m_placeTokens->setValue(p->initial_tokens());
    m_placeAction->setPlainText(QString::fromStdString(p->action()));
    m_stack->setCurrentIndex(1);
}

/**
 * @brief Populates and shows the transition page.
 * @param transitionId ID of the transition to display
 */
void PropertiesPanel::showTransition(int transitionId)
{
    if (!m_ctrl) return;
    const Transition* t = m_ctrl->net()->find_transition_by_id(transitionId);
    if (!t) return;
    m_currentTransitionId = transitionId;
    m_transName->setText(QString::fromStdString(t->name()));
    m_transEvent->setText(QString::fromStdString(t->event_name()));
    m_transGuard->setText(QString::fromStdString(t->guard()));
    m_transDelay->setText(QString::fromStdString(t->delay_expr()));
    m_transAction->setPlainText(QString::fromStdString(t->action()));
    m_stack->setCurrentIndex(2);
}

/**
 * @brief Populates and shows the arc page.
 * @param arcId ID of the arc to display
 */
void PropertiesPanel::showArc(int arcId)
{
    if (!m_ctrl) return;
    const Arc* a = m_ctrl->net()->find_arc_by_id(arcId);
    if (!a) return;
    m_currentArcId = arcId;
    const char* dir = (a->type() == ArcType::INPUT) ? "Place → Transition"
                                                     : "Transition → Place";
    m_arcInfo->setText(QString(dir));
    m_arcWeight->setValue(a->weight());
    m_stack->setCurrentIndex(3);
}

///////////////////////////////////////////////////////////////////////////////

/** @brief Writes place fields back to the model and refreshes the display. */
void PropertiesPanel::applyPlace()
{
    if (m_currentPlaceId < 0 || !m_ctrl) return;
    m_ctrl->updatePlace(m_currentPlaceId,
                        m_placeName->text().trimmed().toStdString(),
                        m_placeTokens->value(),
                        m_placeAction->toPlainText().trimmed().toStdString());
    showPlace(m_currentPlaceId);
    emit placeApplied(m_currentPlaceId);
}

/** @brief Writes transition fields back to the model and refreshes the display. */
void PropertiesPanel::applyTransition()
{
    if (m_currentTransitionId < 0 || !m_ctrl) return;
    m_ctrl->updateTransition(m_currentTransitionId,
                             m_transName->text().trimmed().toStdString(),
                             m_transEvent->text().trimmed().toStdString(),
                             m_transGuard->text().trimmed().toStdString(),
                             m_transDelay->text().trimmed().toStdString(),
                             m_transAction->toPlainText().trimmed().toStdString());
    showTransition(m_currentTransitionId);
    emit transitionApplied(m_currentTransitionId);
}

/** @brief Writes arc weight back to the model and refreshes the display. */
void PropertiesPanel::applyArc()
{
    if (m_currentArcId < 0 || !m_ctrl) return;
    m_ctrl->updateArcWeight(m_currentArcId, m_arcWeight->value());
    showArc(m_currentArcId);
    emit arcApplied(m_currentArcId);
}
