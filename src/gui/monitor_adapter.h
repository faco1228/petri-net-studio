/**
 * @file monitor_adapter.h
 * @brief MonitorAdapter - translates incoming UDP messages into Qt signals for GUI widgets.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QObject>

class MonitorAdapter : public QObject
{
    Q_OBJECT
public:
    explicit MonitorAdapter(QObject *parent = nullptr);
};
