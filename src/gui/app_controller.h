/**
 * @file app_controller.h
 * @brief AppController - orchestrates net editing, code generation and interpreter lifecycle.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QObject>
#include <QPointF>
#include <memory>
#include "../inc/pn_model.h"
#include "../model/pn_net.h"

class AppController : public QObject
{
    Q_OBJECT
public:
    explicit AppController(QObject *parent = nullptr);

    // Access to the current net
    PnNet* net() const;

    // Net lifecycle
    void newNet(const std::string &name);
    bool loadNet(const std::string &path);
    bool saveNet();
    bool saveNetAs(const std::string &path);

    // Editor operations - return new element ID or -1 on failure
    int  addPlace(QPointF pos);
    int  addTransition(QPointF pos);
    int  addArc(ArcType type, int placeId, int transitionId, int weight = 1);
    void removePlace(int id);
    void removeTransition(int id);
    void removeArc(int id);
    void updateItemPos(int id, bool isPlace, QPointF pos);

    // Property editing
    void updatePlace(int id, const std::string &name, int tokens, const std::string &action);
    void updateTransition(int id, const std::string &name, const std::string &event,
                          const std::string &guard, const std::string &delay,
                          const std::string &action);
    void updateArcWeight(int id, int weight);

    // Interpreter lifecycle
    void generateAndRun();
    void stopInterpreter();

    // Info
    const std::string& currentPath() const;
    const std::string& lastError()   const;

signals:
    void netChanged();
    void netLoaded();

private:
    void syncCounters(); // sets m_placeCounter/m_transitionCounter from current net

    std::unique_ptr<PnNet> m_net;
    std::string m_currentPath;
    std::string m_lastError;

    int m_placeCounter;
    int m_transitionCounter;
};
