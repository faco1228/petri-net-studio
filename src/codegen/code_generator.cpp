/**
 * @file code_generator.cpp
 * @author Samuel Fačka (xfackas00)
 * @author Arťom Hanzel (xhanzea00)
 * @brief CodeGenerator — generates a standalone C++ interpreter from a PnNet.
 * @version 0.1
 * @date 2026-04
 *
 * @copyright Copyright (c) 2026
 *
 * What happens here:
 *   1. escStr() and sanitize() are local helpers for safe string embedding
 *   2. RUNTIME_TEMPLATE contains the reusable event-loop/socket runtime as a raw string
 *   3. MAIN_TEMPLATE contains the main() entry point that boots the runtime
 *   4. generate() assembles all sections into a single .cpp source string and writes it
 */

#include "code_generator.h"
#include "../model/pn_net.h"
#include "../inc/pn_model.h"

#include <fstream>
#include <cctype>

/** @brief Escapes a string for safe embedding inside a C string literal. */
static std::string escStr(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (c == '"')  r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else if (c == '\n') r += "\\n";
        else r += c;
    }
    return r;
}

/** @brief Replaces non-alphanumeric/underscore characters with '_' for use as a C identifier. */
static std::string sanitize(const std::string& s) {
    std::string r;
    for (char c : s) r += (std::isalnum((unsigned char)c) || c == '_') ? c : '_';
    if (r.empty()) r = "net";
    return r;
}

/** @brief Reusable runtime: event loop, socket I/O, token management, timer queue. */
static const char* RUNTIME_TEMPLATE = R"RUNTIME(
static int64_t sys_now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

struct Runtime {
    std::map<int,int>       marking;
    std::map<int,int64_t>   place_last_change;
    std::map<int,int64_t>   trans_became_enabled;
    std::set<int>           pending_timer_ids;
    std::map<std::string,std::string> input_values;
    std::set<std::string>   pending_events;

    struct Timer {
        int64_t expiry_ms; int trans_id;
        bool operator>(const Timer& o) const { return expiry_ms > o.expiry_ms; }
    };
    std::priority_queue<Timer,std::vector<Timer>,std::greater<Timer>> timers;

    int sock = -1;
    struct sockaddr_in gui_addr{};
    bool   state_dirty  = true;
    int64_t last_state_ms = 0;

    int64_t now_ms() { return sys_now_ms(); }

    void init() {
        int64_t t = now_ms();
        for (int i = 0; i < PLACE_COUNT; i++) {
            marking[PLACES[i].id] = PLACES[i].initial_tokens;
            place_last_change[PLACES[i].id] = t;
        }
        for (int i = 0; i < TRANS_COUNT; i++)
            trans_became_enabled[TRANSITIONS[i].id] = 0;

        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) { perror("socket"); exit(1); }
        fcntl(sock, F_SETFL, O_NONBLOCK);

        struct sockaddr_in my{};
        my.sin_family = AF_INET;
        my.sin_port = htons(MY_PORT);
        my.sin_addr.s_addr = INADDR_ANY;
        if (bind(sock,(struct sockaddr*)&my,sizeof(my)) < 0) perror("bind");

        memset(&gui_addr,0,sizeof(gui_addr));
        gui_addr.sin_family = AF_INET;
        gui_addr.sin_port   = htons(GUI_PORT);
        inet_aton("127.0.0.1",&gui_addr.sin_addr);

        send_raw(std::string("ANNOUNCE\t")+NET_NAME+"\t"+std::to_string(MY_PORT)+"\n");
    }

    void send_raw(const std::string& m) {
        sendto(sock,m.c_str(),m.size(),0,(struct sockaddr*)&gui_addr,sizeof(gui_addr));
    }
    void send_log(const char* type, const std::string& det) {
        send_raw(std::string("LOG\t")+NET_NAME+"\t"+std::to_string(now_ms())+"\t"+type+"\t"+det+"\n");
    }
    void send_state() {
        std::string mj="{";
        for (int i=0;i<PLACE_COUNT;i++){
            if(i) mj+=",";
            mj+="\""; mj+=PLACES[i].name; mj+="\":";
            mj+=std::to_string(marking[PLACES[i].id]);
        }
        mj+="}";
        std::string en;
        for (int i=0;i<TRANS_COUNT;i++)
            if(is_enabled_now(TRANSITIONS[i].id)){if(!en.empty())en+=","; en+=TRANSITIONS[i].name;}
        std::string pl;
        for (int tid : pending_timer_ids) {
            int i=tidx(tid); if(i<0) continue;
            if(!pl.empty()) pl+=","; pl+=TRANSITIONS[i].name;
        }
        send_raw(std::string("STATE\t")+NET_NAME+"\t"+std::to_string(now_ms())+"\t"
                 +mj+"\t"+en+"\t"+pl+"\t"+build_vars_json()+"\n");
        last_state_ms = now_ms();
        state_dirty   = false;
    }

    // Script API impl
    std::string valueof(const std::string& n) {
        auto it=input_values.find(n); return it!=input_values.end()?it->second:"";
    }
    bool defined_input(const std::string& n) { return input_values.count(n)>0; }
    void rt_output(const std::string& n, int64_t v) {
        send_log("OUTPUT", n+"="+std::to_string(v));
    }
    void rt_output_str(const std::string& n, const std::string& v) {
        send_log("OUTPUT", n+"="+v);
    }
    int64_t tokens_of(const std::string& n) {
        for(int i=0;i<PLACE_COUNT;i++) if(n==PLACES[i].name) return marking[PLACES[i].id];
        return 0;
    }
    int64_t elapsed_of(const std::string& n) {
        for(int i=0;i<PLACE_COUNT;i++) if(n==PLACES[i].name) {
            auto it=place_last_change.find(PLACES[i].id);
            return it!=place_last_change.end()?now_ms()-it->second:0;
        }
        for(int i=0;i<TRANS_COUNT;i++) if(n==TRANSITIONS[i].name) {
            int64_t t=trans_became_enabled[TRANSITIONS[i].id];
            return t?now_ms()-t:0;
        }
        return 0;
    }

    void add_tokens(int pid, int cnt) {
        marking[pid]+=cnt; place_last_change[pid]=now_ms(); state_dirty=true;
        for(int k=0;k<cnt;k++) run_place_action(pid);
    }
    void remove_tokens(int pid, int cnt) {
        marking[pid]-=cnt; place_last_change[pid]=now_ms(); state_dirty=true;
    }

    bool is_enabled_in(int tid, const std::map<int,int>& m) {
        for(int i=0;i<ARC_COUNT;i++)
            if(ARCS[i].trans_id==tid && ARCS[i].is_input) {
                auto it=m.find(ARCS[i].place_id);
                if(it==m.end()||it->second<ARCS[i].weight) return false;
            }
        return true;
    }
    bool is_enabled_now(int tid) { return is_enabled_in(tid,marking); }

    int tidx(int tid) {
        for(int i=0;i<TRANS_COUNT;i++) if(TRANSITIONS[i].id==tid) return i;
        return -1;
    }

    bool can_fire(int tid, const std::map<int,int>& m) {
        int i=tidx(tid); if(i<0) return false;
        if(strlen(TRANSITIONS[i].delay_expr)>0) return false; // timed – handled separately
        const char* ev=TRANSITIONS[i].event;
        if(strlen(ev)>0 && pending_events.count(ev)==0) return false;
        if(!is_enabled_in(tid,m)) return false;
        return eval_guard(tid);
    }

    std::vector<int> maximal_fire_set() {
        std::map<int,int> tmp=marking;
        std::vector<int> fs;
        for(int i=0;i<TRANS_COUNT;i++){
            int tid=TRANSITIONS[i].id;
            if(!can_fire(tid,tmp)) continue;
            fs.push_back(tid);
            for(int j=0;j<ARC_COUNT;j++)
                if(ARCS[j].trans_id==tid && ARCS[j].is_input)
                    tmp[ARCS[j].place_id]-=ARCS[j].weight;
        }
        return fs;
    }

    void fire(int tid) {
        int i=tidx(tid);
        const char* ev=TRANSITIONS[i].event;
        if(strlen(ev)>0) pending_events.erase(ev);
        for(int j=0;j<ARC_COUNT;j++)
            if(ARCS[j].trans_id==tid && ARCS[j].is_input)
                remove_tokens(ARCS[j].place_id,ARCS[j].weight);
        run_trans_action(tid);
        for(int j=0;j<ARC_COUNT;j++)
            if(ARCS[j].trans_id==tid && !ARCS[j].is_input)
                add_tokens(ARCS[j].place_id,ARCS[j].weight);
        send_log("FIRED",TRANSITIONS[i].name);
        state_dirty=true;
    }

    void update_timers() {
        for(int i=0;i<TRANS_COUNT;i++){
            int tid=TRANSITIONS[i].id;
            if(strlen(TRANSITIONS[i].delay_expr)==0) continue;
            bool en=is_enabled_now(tid), was=(trans_became_enabled[tid]!=0);
            if(en && !was){
                trans_became_enabled[tid]=now_ms();
                if(!pending_timer_ids.count(tid)){
                    pending_timer_ids.insert(tid);
                    timers.push({now_ms()+eval_delay(tid),tid});
                }
            } else if(!en && was) {
                trans_became_enabled[tid]=0;
                pending_timer_ids.erase(tid);
            }
        }
    }

    void check_timers() {
        while(!timers.empty() && timers.top().expiry_ms<=now_ms()){
            auto top=timers.top(); timers.pop();
            pending_timer_ids.erase(top.trans_id);
            int tid=top.trans_id;
            if(is_enabled_now(tid) && trans_became_enabled[tid]!=0){
                fire(tid); trans_became_enabled[tid]=0;
            } else {
                int i=tidx(tid);
                if(i>=0) send_log("TIMEOUT_IGNORED",TRANSITIONS[i].name);
            }
        }
    }

    void check_incoming() {
        char buf[65536]; struct sockaddr_in from; socklen_t fl=sizeof(from); ssize_t n;
        while((n=recvfrom(sock,buf,sizeof(buf)-1,0,(struct sockaddr*)&from,&fl))>0){
            buf[n]='\0';
            while(n>0&&(buf[n-1]=='\n'||buf[n-1]=='\r')) buf[--n]='\0';
            std::vector<std::string> p; std::istringstream ss(buf); std::string tok;
            while(std::getline(ss,tok,'\t')) p.push_back(tok);
            if(p.empty()) continue;
            if(p[0]=="INPUT" && p.size()>=4){
                input_values[p[2]]=p[3];
                pending_events.insert(p[2]);
                send_log("INPUT_RECEIVED",p[2]+"="+p[3]);
                // Fire event-triggered transitions immediately after input
                auto sfs=maximal_fire_set(); for(int t:sfs) fire(t);
                update_timers();
                send_state();
            } else if(p[0]=="QUIT"){
                send_log("QUIT",""); close(sock); exit(0);
            } else if(p[0]=="STEP"){
                // Fire exactly one maximal set — no stabilisation loop
                auto sfs=maximal_fire_set(); for(int t:sfs) fire(t);
                update_timers();
                send_state();
            }
        }
    }

    void run_place_action(int place_id);
    void run_trans_action(int trans_id);
    bool eval_guard(int trans_id);
    int64_t eval_delay(int trans_id);
    std::string build_vars_json();
};

static Runtime g_rt;

// Script API (free functions so user action code can call them directly)
inline std::string valueof(const std::string& n)              { return g_rt.valueof(n); }
inline bool         defined(const std::string& n)              { return g_rt.defined_input(n); }
inline void         output(const std::string& n, int64_t v)   { g_rt.rt_output(n,v); }
inline void         output(const std::string& n, int v)       { g_rt.rt_output(n,(int64_t)v); }
inline void         output(const std::string& n, const std::string& v) { g_rt.rt_output_str(n,v); }
inline void         output(const std::string& n, const char* v){ g_rt.rt_output_str(n,v); }
inline int64_t      tokens(const std::string& n)               { return g_rt.tokens_of(n); }
inline int64_t      elapsed(const std::string& n)              { return g_rt.elapsed_of(n); }
inline int64_t      now()                                      { return g_rt.now_ms(); }
static inline int   atoi(const std::string& s)                 { return ::atoi(s.c_str()); }
static inline double atof(const std::string& s)                { return ::atof(s.c_str()); }
)RUNTIME";

/** @brief main() entry point that boots the runtime and runs the event loop. */
static const char* MAIN_TEMPLATE = R"MAIN(
int main(int argc, char* argv[]) {
    if (argc > 1) MY_PORT = ::atoi(argv[1]);
    g_rt.init();
    for (int i = 0; i < PLACE_COUNT; i++)
        for (int k = 0; k < PLACES[i].initial_tokens; k++)
            g_rt.run_place_action(PLACES[i].id);
    g_rt.send_state();

    while (true) {
        g_rt.check_incoming();
        g_rt.update_timers();
        g_rt.check_timers();  // timers still fire automatically
        int64_t now_t = g_rt.now_ms();
        if (g_rt.state_dirty && (now_t - g_rt.last_state_ms >= 100))
            g_rt.send_state();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return 0;
}
)MAIN";

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Assembles and writes the complete C++ interpreter source for the given net.
 *
 * The output file contains:
 *   - Standard library includes
 *   - Static tables for places, transitions and arcs derived from the net
 *   - The reusable runtime (RUNTIME_TEMPLATE)
 *   - Switch-based dispatchers for place actions, transition actions, guards and delays
 *   - build_vars_json() for embedding variable values in STATE datagrams
 *   - main() (MAIN_TEMPLATE)
 */
bool CodeGenerator::generate(const PnNet& net, const std::string& outputDir, std::string& errorMsg)
{
    const auto& places      = net.places();
    const auto& transitions = net.transitions();
    const auto& arcs        = net.arcs();
    const auto& vars        = net.variables();

    std::string netName = sanitize(net.name());
    std::string src;

    // ---- File header and includes ----
    src += "// Auto-generated by ICP Petri Net Editor — Net: " + net.name() + "\n";
    src += "#include <cstdio>\n#include <cstdlib>\n#include <cstring>\n";
    src += "#include <string>\n#include <vector>\n#include <map>\n#include <set>\n";
    src += "#include <queue>\n#include <chrono>\n#include <thread>\n#include <sstream>\n";
    src += "#include <sys/socket.h>\n#include <arpa/inet.h>\n#include <netinet/in.h>\n";
    src += "#include <unistd.h>\n#include <fcntl.h>\n\n";

    // ---- Port constants and net name ----
    src += "static const int GUI_PORT = 7001;\n";
    src += "static int MY_PORT = 7000;\n";
    src += "static const char* NET_NAME = \"" + escStr(net.name()) + "\";\n\n";

    // ---- Embedded C++ variable declarations ----
    for (const auto& v : vars)
        src += "static " + v.type + " " + v.name + " = " + v.value + ";\n";
    src += "\n";

    // ---- Static table structs ----
    src += "struct PlaceDef { int id; const char* name; int initial_tokens; };\n";
    src += "struct TransDef { int id; const char* name; const char* event; const char* guard; const char* delay_expr; };\n";
    src += "struct ArcDef   { int place_id; int trans_id; int weight; bool is_input; };\n\n";

    // ---- Place table ----
    src += "static const PlaceDef PLACES[] = {\n";
    if (places.empty()) src += "    {-1,\"_\",0},\n"; // sentinel for empty net
    for (const auto& p : places)
        src += "    {" + std::to_string(p->id()) + ",\"" + escStr(p->name()) + "\","
             + std::to_string(p->initial_tokens()) + "},\n";
    src += "};\nstatic const int PLACE_COUNT = " + std::to_string(places.size()) + ";\n\n";

    // ---- Transition table ----
    src += "static const TransDef TRANSITIONS[] = {\n";
    if (transitions.empty()) src += "    {-1,\"_\",\"\",\"\",\"\"},\n";
    for (const auto& t : transitions)
        src += "    {" + std::to_string(t->id()) + ",\"" + escStr(t->name()) + "\",\""
             + escStr(t->event_name()) + "\",\"" + escStr(t->guard()) + "\",\""
             + escStr(t->delay_expr()) + "\"},\n";
    src += "};\nstatic const int TRANS_COUNT = " + std::to_string(transitions.size()) + ";\n\n";

    // ---- Arc table ----
    src += "static const ArcDef ARCS[] = {\n";
    if (arcs.empty()) src += "    {-1,-1,1,true},\n";
    for (const auto& a : arcs)
        src += "    {" + std::to_string(a->place_id()) + "," + std::to_string(a->transition_id())
             + "," + std::to_string(a->weight()) + ","
             + (a->type() == ArcType::INPUT ? "true" : "false") + "},\n";
    src += "};\nstatic const int ARC_COUNT = " + std::to_string(arcs.size()) + ";\n\n";

    // ---- Runtime template ----
    src += RUNTIME_TEMPLATE;
    src += "\n";

    // ---- Place action dispatcher ----
    src += "void Runtime::run_place_action(int place_id) {\n    switch(place_id) {\n";
    for (const auto& p : places)
        if (!p->action().empty())
            src += "        case " + std::to_string(p->id()) + ": { " + p->action() + " break; }\n";
    src += "        default: break;\n    }\n}\n\n";

    // ---- Transition action dispatcher ----
    src += "void Runtime::run_trans_action(int trans_id) {\n    switch(trans_id) {\n";
    for (const auto& t : transitions)
        if (!t->action().empty())
            src += "        case " + std::to_string(t->id()) + ": { " + t->action() + " break; }\n";
    src += "        default: break;\n    }\n}\n\n";

    // ---- Guard evaluator ----
    src += "bool Runtime::eval_guard(int trans_id) {\n    switch(trans_id) {\n";
    for (const auto& t : transitions)
        if (!t->guard().empty())
            src += "        case " + std::to_string(t->id()) + ": return (" + t->guard() + ");\n";
    src += "        default: return true;\n    }\n}\n\n";

    // ---- Delay evaluator ----
    src += "int64_t Runtime::eval_delay(int trans_id) {\n    switch(trans_id) {\n";
    for (const auto& t : transitions)
        if (!t->delay_expr().empty())
            src += "        case " + std::to_string(t->id()) + ": return (int64_t)(" + t->delay_expr() + ");\n";
    src += "        default: return 0;\n    }\n}\n\n";

    // ---- Variable JSON builder ----
    src += "std::string Runtime::build_vars_json() {\n    std::string j=\"{\";\n";
    bool first = true;
    for (const auto& v : vars) {
        if (!first) src += "    j+=\",\";\n";
        src += "    j+=\"\\\"" + v.name + "\\\":\" + std::to_string(" + v.name + ");\n";
        first = false;
    }
    src += "    j+=\"}\";\n    return j;\n}\n\n";

    src += MAIN_TEMPLATE;

    // ---- Write to disk ----
    std::string filepath = outputDir + "/net_" + netName + ".cpp";
    std::ofstream f(filepath);
    if (!f) { errorMsg = "Cannot write " + filepath; return false; }
    f << src;
    return true;
}
