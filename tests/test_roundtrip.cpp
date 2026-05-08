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
    CHECK(a.name() == b.name(),
          "name: '" + a.name() + "'");

    CHECK(a.places().size() == b.places().size(),
          "place count: " + std::to_string(a.places().size()));

    CHECK(a.transitions().size() == b.transitions().size(),
          "transition count: " + std::to_string(a.transitions().size()));

    CHECK(a.arcs().size() == b.arcs().size(),
          "arc count: " + std::to_string(a.arcs().size()));

    CHECK(a.inputs().size() == b.inputs().size(),
          "input count: " + std::to_string(a.inputs().size()));

    CHECK(a.outputs().size() == b.outputs().size(),
          "output count: " + std::to_string(a.outputs().size()));

    CHECK(a.variables().size() == b.variables().size(),
          "variable count: " + std::to_string(a.variables().size()));

    // Compare places by name
    for (const auto &p : a.places()) {
        const Place *p2 = b.find_place_by_name(p->name());
        CHECK(p2 != nullptr,
              "place exists: " + p->name());
        if (!p2) continue;
        CHECK(p->initial_tokens() == p2->initial_tokens(),
              "  tokens " + p->name() + ": " + std::to_string(p->initial_tokens()));
        CHECK(p->action() == p2->action(),
              "  action " + p->name());
    }

    // Compare transitions by name
    for (const auto &t : a.transitions()) {
        const Transition *t2 = b.find_transition_by_name(t->name());
        CHECK(t2 != nullptr,
              "transition exists: " + t->name());
        if (!t2) continue;
        CHECK(t->event_name() == t2->event_name(),
              "  event " + t->name() + ": '" + t->event_name() + "'");
        CHECK(t->guard() == t2->guard(),
              "  guard " + t->name());
        CHECK(t->delay_expr() == t2->delay_expr(),
              "  delay " + t->name() + ": '" + t->delay_expr() + "'");
        CHECK(t->action() == t2->action(),
              "  action " + t->name());
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
