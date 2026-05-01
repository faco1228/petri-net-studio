/**
 * @file monitor_panel.h
 * @brief MonitorPanel - live view of net state during interpreter run.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QWidget>

class AppController;

class MonitorPanel : public QWidget
{
    Q_OBJECT
public:
    explicit MonitorPanel(AppController *controller, QWidget *parent = nullptr);
};
