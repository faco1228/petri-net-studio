/**
 * @file properties_panel.h
 * @brief PropertiesPanel - dock widget for editing selected place/transition properties.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QWidget>

class PropertiesPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PropertiesPanel(QWidget *parent = nullptr);
};
