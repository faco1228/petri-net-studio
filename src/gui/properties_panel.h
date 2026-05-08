/**
 * @file properties_panel.h
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief PropertiesPanel — dock widget stub for editing selected element properties.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. Declares a minimal QWidget subclass used as a placeholder in the right dock
 *   2. Property editing for places/transitions is handled by their respective dialogs
 */

#ifndef PROPERTIES_PANEL_H
#define PROPERTIES_PANEL_H

#include <QWidget>

/**
 * @brief Placeholder widget occupying the Properties dock.
 *
 * Actual editing of place and transition properties is performed through
 * PlaceDialog and TransitionDialog opened from GraphicsEditor on double-click.
 */
class PropertiesPanel : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the (currently empty) properties panel.
     * @param parent optional Qt parent
     */
    explicit PropertiesPanel(QWidget *parent = nullptr);
};

#endif // PROPERTIES_PANEL_H
