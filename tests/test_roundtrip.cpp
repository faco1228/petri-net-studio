/**
 * @file test_roundtrip.cpp
 * @brief Round-trip test: load .pn -> save -> load again -> compare.
 * @author xfacka00
 * @date 2026-04
 *
 * Build:
 *   g++ -std=c++17 -I../src/inc -I../src/model \
 *       test_roundtrip.cpp \
 *       ../src/model/pn_net.cpp \
 *       ../src/model/pn_place.cpp \
 *       ../src/model/pn_transition.cpp \
 *       ../src/model/pn_arc.cpp \
 *       ../src/model/pn_file_parser.cpp \
 *       ../src/model/pn_file_writer.cpp \
 *       -lQt6Core -fPIC \
 *       -o test_roundtrip
 * Run:
 *   ./test_roundtrip ../examples/tof_pn_5s.pn
 *   ./test_roundtrip ../examples/tof_pn.pn
 *   ./test_roundtrip ../examples/semaphore.pn
 */

#include <iostream>
#include <cassert>
#include <cstdlib>
#include "pn_file_parser.h"
#include "pn_file_writer.h"
#include "pn_net.h"

// Simple pass/fail counter
static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) \
    do { \
        if (cond) { std::cout << "  PASS: " << msg << "\n"; ++g_pass; } \
        else      { std::cout << "  FAIL: " << msg << "\n"; ++g_fail; } \
    } while(0)

static void compareNets(const PnNet &a, const PnNet &b)
{
    CHECK(a.getName() == b.getName(),
          "name: '" + a.getName() + "'");

    CHECK(a.getPlaces().size() == b.getPlaces().size(),
          "place count: " + std::to_string(a.getPlaces().size()));

    CHECK(a.getTransitions().size() == b.getTransitions().size(),
          "transition count: " + std::to_string(a.getTransitions().size()));

    CHECK(a.getArcs().size() == b.getArcs().size(),
          "arc count: " + std::to_string(a.getArcs().size()));

    CHECK(a.getInputs().size() == b.getInputs().size(),
          "input count: " + std::to_string(a.getInputs().size()));

    CHECK(a.getOutputs().size() == b.getOutputs().size(),
          "output count: " + std::to_string(a.getOutputs().size()));

    CHECK(a.getVariables().size() == b.getVariables().size(),
          "variable count: " + std::to_string(a.getVariables().size()));

    // Compare places by name
    for (const auto &p : a.getPlaces()) {
        const Place *p2 = b.findPlaceByName(p->getName());
        CHECK(p2 != nullptr,
              "place exists: " + p->getName());
        if (!p2) continue;
        CHECK(p->getInitialTokens() == p2->getInitialTokens(),
              "  tokens " + p->getName() + ": " + std::to_string(p->getInitialTokens()));
        CHECK(p->getAction() == p2->getAction(),
              "  action " + p->getName());
    }

    // Compare transitions by name
    for (const auto &t : a.getTransitions()) {
        const Transition *t2 = b.findTransitionByName(t->getName());
        CHECK(t2 != nullptr,
              "transition exists: " + t->getName());
        if (!t2) continue;
        CHECK(t->getEventName() == t2->getEventName(),
              "  event " + t->getName() + ": '" + t->getEventName() + "'");
        CHECK(t->getGuard() == t2->getGuard(),
              "  guard " + t->getName());
        CHECK(t->getDelayExpr() == t2->getDelayExpr(),
              "  delay " + t->getName() + ": '" + t->getDelayExpr() + "'");
        CHECK(t->getAction() == t2->getAction(),
              "  action " + t->getName());
    }
}

static int testFile(const std::string &path)
{
    std::cout << "\n=== " << path << " ===\n";

    // Step 1: load original
    std::string err;
    auto net1 = PnFileParser::load(path, err);
    CHECK(net1 != nullptr, "load original: " + (err.empty() ? "ok" : err));
    if (!net1) return 1;

    // Step 2: save to temp
    std::string tmp = "/tmp/roundtrip_test.pn";
    bool saved = PnFileWriter::save(*net1, tmp, err);
    CHECK(saved, "save to tmp: " + (err.empty() ? "ok" : err));
    if (!saved) return 1;

    // Step 3: load saved
    auto net2 = PnFileParser::load(tmp, err);
    CHECK(net2 != nullptr, "load saved: " + (err.empty() ? "ok" : err));
    if (!net2) return 1;

    // Step 4: compare
    compareNets(*net1, *net2);

    return 0;
}

int main(int argc, char *argv[])
{
    std::vector<std::string> files;

    if (argc > 1) {
        for (int i = 1; i < argc; ++i)
            files.push_back(argv[i]);
    } else {
        files = {
            "../examples/tof_pn_5s.pn",
            "../examples/tof_pn.pn",
            "../examples/semaphore.pn",
        };
    }

    for (const auto &f : files)
        testFile(f);

    std::cout << "\nResults: " << g_pass << " passed, " << g_fail << " failed\n";
    return g_fail > 0 ? 1 : 0;
}
