/**
 * @file event_log_view.h
 * @brief EventLogView - scrollable read-only log of interpreter events.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QWidget>

class EventLogView : public QWidget
{
    Q_OBJECT
public:
    explicit EventLogView(QWidget *parent = nullptr);
};
