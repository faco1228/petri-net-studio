/**
 * @file inject_panel.h
 * @brief InjectPanel - UI for manually injecting input events during runtime.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QWidget>

class InjectPanel : public QWidget
{
    Q_OBJECT
public:
    explicit InjectPanel(QWidget *parent = nullptr);
};
