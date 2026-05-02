/**
 * @file app_controller.cpp
 * @brief Implementation of AppController.
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#include "app_controller.h"
#include "../model/pn_net.h"
#include "../model/pn_file_parser.h"
#include "../model/pn_file_writer.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_net(std::make_unique<PnNet>())
    , m_placeCounter(1)
    , m_transitionCounter(1)
{}

PnNet* AppController::net() const { return m_net.get(); }

const std::string& AppController::currentPath() const { return m_currentPath; }
const std::string& AppController::lastError()   const { return m_lastError; }

void AppController::syncCounters()
{
    // Start counters one above the highest existing ID so new names don't clash
    int maxPlace = 0;
    for (const auto &p : m_net->getPlaces())
        if (p->getId() > maxPlace) maxPlace = p->getId();
    int maxTrans = 0;
    for (const auto &t : m_net->getTransitions())
        if (t->getId() > maxTrans) maxTrans = t->getId();

    m_placeCounter      = maxPlace + 1;
    m_transitionCounter = maxTrans + 1;
}

void AppController::newNet(const std::string &name)
{
    m_net = std::make_unique<PnNet>();
    m_net->setName(name);
    m_currentPath.clear();
    m_placeCounter      = 1;
    m_transitionCounter = 1;
    emit netChanged();
}

bool AppController::loadNet(const std::string &path)
{
    std::string err;
    auto loaded = PnFileParser::load(path, err);
    if (!loaded) {
        m_lastError = err;
        return false;
    }
    m_net = std::move(loaded);
    m_currentPath = path;
    m_lastError.clear();
    syncCounters();
    emit netLoaded();
    return true;
}

bool AppController::saveNet()
{
    if (m_currentPath.empty()) return false;
    std::string err;
    bool ok = PnFileWriter::save(*m_net, m_currentPath, err);
    if (!ok) m_lastError = err;
    return ok;
}

bool AppController::saveNetAs(const std::string &path)
{
    std::string err;
    if (!PnFileWriter::save(*m_net, path, err)) {
        m_lastError = err;
        return false;
    }
    m_currentPath = path;
    m_lastError.clear();
    return true;
}

int AppController::addPlace(QPointF pos)
{
    std::string name = "P" + std::to_string(m_placeCounter++);
    Place *p = m_net->addPlace(name, 0, pos);
    emit netChanged();
    return p ? p->getId() : -1;
}

int AppController::addTransition(QPointF pos)
{
    std::string name = "T" + std::to_string(m_transitionCounter++);
    Transition *t = m_net->addTransition(name, pos);
    emit netChanged();
    return t ? t->getId() : -1;
}

int AppController::addArc(ArcType type, int placeId, int transitionId, int weight)
{
    Arc *a = m_net->addArc(type, placeId, transitionId, weight);
    emit netChanged();
    return a ? a->getId() : -1;
}

void AppController::removePlace(int id)
{
    m_net->removePlace(id);
    emit netChanged();
}

void AppController::removeTransition(int id)
{
    m_net->removeTransition(id);
    emit netChanged();
}

void AppController::removeArc(int id)
{
    m_net->removeArc(id);
    emit netChanged();
}

void AppController::updatePlace(int id, const std::string &name, int tokens, const std::string &action)
{
    if (Place *p = m_net->findPlaceById(id)) {
        p->setName(name);
        p->setInitialTokens(tokens);
        p->setAction(action);
        emit netChanged();
    }
}

void AppController::updateTransition(int id, const std::string &name, const std::string &event,
                                     const std::string &guard, const std::string &delay,
                                     const std::string &action)
{
    if (Transition *t = m_net->findTransitionById(id)) {
        t->setName(name);
        t->setEventName(event);
        t->setGuard(guard);
        t->setDelayExpr(delay);
        t->setAction(action);
        emit netChanged();
    }
}

void AppController::updateArcWeight(int id, int weight)
{
    if (Arc *a = m_net->findArcById(id)) {
        a->setWeight(weight);
        emit netChanged();
    }
}

void AppController::updateItemPos(int id, bool isPlace, QPointF pos)
{
    if (isPlace) {
        if (Place *p = m_net->findPlaceById(id))
            p->setPos(pos);
    } else {
        if (Transition *t = m_net->findTransitionById(id))
            t->setPos(pos);
    }
}

void AppController::generateAndRun()
{
    // TODO: implement code generation and interpreter launch
}

void AppController::stopInterpreter()
{
    // TODO: send QUIT via UDP and terminate QProcess
}
