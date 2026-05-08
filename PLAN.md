# Projektový plán: Vizuálny editor, generátor kódu a runtime monitor interpretovaných Petriho sietí
## Event-driven SFC/Grafcet s multiset sémantikou — C++17 + Qt5/6, tímová dvojica

---

## Obsah

1. [Prehľad architektúry](#1-prehľad-architektúry)
2. [Rozdelenie práce](#2-rozdelenie-práce)
3. [4-týždenný harmonogram](#3-4-týždenný-harmonogram)
4. [Kompletný zoznam modulov a súborov](#4-kompletný-zoznam-modulov-a-súborov)
5. [UDP protokol](#5-udp-protokol)
6. [Textový formát .pn súboru](#6-textový-formát-pn-súboru)
7. [Makefile ciele](#7-makefile-ciele)
8. [Kritické rozhodnutia](#8-kritické-rozhodnutia)
9. [Tech stack a knižnice](#9-tech-stack-a-knižnice)

---

## 1. Prehľad architektúry

### Celkový pohľad

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          POUŽÍVATEĽ                                     │
└──────────────────────────────┬──────────────────────────────────────────┘
                               │
┌──────────────────────────────▼──────────────────────────────────────────┐
│                     GUI APLIKÁCIA (Qt5/6)                               │
│                                                                         │
│  ┌──────────────────┐   ┌──────────────────┐   ┌──────────────────────┐ │
│  │  GraphicsEditor  │   │  MonitorPanel    │   │  InjectPanel         │ │
│  │  (QGraphicsView) │   │  (live marking)  │   │  (input injection)   │ │
│  └────────┬─────────┘   └────────┬─────────┘   └──────────┬───────────┘ │
│           │                      │                         │             │
│  ┌────────▼──────────────────────▼─────────────────────────▼───────────┐ │
│  │                     MainWindow / AppController                      │ │
│  └──────────────────────────────┬──────────────────────────────────────┘ │
│                                 │                                        │
│  ┌──────────────────────────────▼──────────────────────────────────────┐ │
│  │                 PnFileParser / PnFileWriter                         │ │
│  │                 CodeGenerator                                       │ │
│  └──────────────────────────────┬──────────────────────────────────────┘ │
│                                 │                                        │
│  ┌──────────────────────────────▼──────────────────────────────────────┐ │
│  │                 UdpClient (Qt QUdpSocket)                           │ │
│  └──────────────────────────────┬──────────────────────────────────────┘ │
└────────────────────────────── UDP ────────────────────────────────────────┘
                                 │  (localhost, port 7000/7001)
┌────────────────────────────────▼──────────────────────────────────────────┐
│                  GENEROVANÝ INTERPRET (standalone binary)                 │
│                                                                           │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │                        main() / EventLoop                         │   │
│  └──────┬──────────────────────────────────────────┬─────────────────┘   │
│         │                                          │                     │
│  ┌──────▼──────────────┐               ┌───────────▼─────────────────┐   │
│  │  PetriNetEngine     │               │  UdpServer                  │   │
│  │  - Marking          │               │  - príjem input events      │   │
│  │  - FireSet          │               │  - odosielanie STATE/LOG    │   │
│  │  - TimerManager     │               └─────────────────────────────┘   │
│  │  - ScriptRuntime    │                                                  │
│  └─────────────────────┘                                                  │
└───────────────────────────────────────────────────────────────────────────┘
```

### Tok dát pri generovaní a spustení

```
  [.pn súbor]
       │
       ▼
  PnFileParser  ──►  PnModel (v pamäti GUI)
       │
       ▼
  CodeGenerator ──►  generated/net_NAME.cpp + net_NAME_runtime.h
       │
       ▼
  make (subprocess) ──►  generated/interpreter_NAME  (binary)
       │
       ▼
  QProcess::start() ──►  beží na pozadí
       │
       ▼
  UdpClient ──►  pripojí sa, začne prijímať STATE správy
```

### Vrstvový model

```
  Vrstva 4: GUI (Qt widgets, QGraphicsScene)
  Vrstva 3: Aplikačná logika (AppController, MonitorAdapter)
  Vrstva 2: Model (PnModel, PnFileParser, CodeGenerator)
  Vrstva 1: Sieťová komunikácia (UdpClient / UdpServer)
  Vrstva 0: Interpret (generovaný C++ kód, PetriNetEngine)
```

---

## 2. Rozdelenie práce

### Osoba A — GUI / Editor / Monitor

Zodpovedná za všetko viditeľné v Qt aplikácii a za komunikáciu s interpretom zo strany GUI.

| Oblasť | Zodpovednosť |
|---|---|
| GraphicsEditor | QGraphicsScene, drag & drop, kreslenie miest/prechodov/hrán |
| PropertiesPanel | bočný panel pre editáciu podmienok, akcií, váh |
| MonitorPanel | live zobrazenie markingu, farebné zvýraznenie enabled/pending |
| InjectPanel | formulár pre manuálne injektovanie vstupov |
| EventLogView | scrollovací log udalostí s časovými razítkami |
| MainWindow | menu, toolbar, QTabWidget (editor / monitor tabyy) |
| AppController | orchestrácia spustenia interpretu, QProcess, prepínanie módov |
| UdpClient | QUdpSocket wrapper, parsovanie prichádzajúcich správ |
| Dialógy | NewNetDialog, PlaceDialog, TransitionDialog, ArcDialog |

### Osoba B — Model / Engine / CodeGen / UDP server

Zodpovedná za dátový model siete, generovanie kódu a celý interpret.

| Oblasť | Zodpovednosť |
|---|---|
| PnModel | triedy Place, Transition, Arc, PnNet — čistý dátový model |
| PnFileParser | parser .pn formátu (ručne písaný alebo flex/bison) |
| PnFileWriter | serializácia PnModel do .pn súboru |
| CodeGenerator | šablónový generátor .cpp interpretu z PnModel |
| PetriNetEngine | jadro — marking, firing, timer, multiset sémantika |
| ScriptRuntime | vestavané funkcie: valueof, defined, output, tokens, elapsed, now |
| TimerManager | správa časovačov pre @delay_ms prechody |
| UdpServer | server strana v generovanom interpreti — príjem/odosielanie správ |
| EventLoop | hlavná slučka interpretu |
| Makefile + build | súbor Makefile, štruktúra build systému |

### Hranica zodpovednosti (interface medzi A a B)

```
  Osoba A číta/zapisuje:   PnModel  (súbory: pn_model.h, pn_place.h, pn_transition.h, pn_arc.h)
  Osoba A volá:            PnFileParser::load(), PnFileWriter::save(), CodeGenerator::generate()
  Osoba A komunikuje cez:  UdpClient — správy definované v udp_protocol.h

  Osoba B implementuje:    PnModel + všetky jeho triedy
  Osoba B implementuje:    generovaný interpreter + engine
  Osoba B definuje:        udp_protocol.h (spoločná hlavička)
```

**Spoločné súbory (obaja musia súhlasiť pred kódovaním):**
- `src/common/udp_protocol.h` — formáty správ
- `src/common/pn_model.h` — rozhranie dátového modelu
- `.pn` formát — finálna syntax (pozri sekciu 6)

---

## 3. 4-týždenný harmonogram

### Týždeň 1 — Základ a dohoda na rozhraniach

**Cieľ:** Spoločne dohodnuté rozhrania, fungujúci skeleton projektu, základný editor a parser.

| Deň | Osoba A | Osoba B |
|---|---|---|
| Po–Ut | Nastaviť Qt projekt, CMakeLists.txt / .pro, kostru MainWindow | Nastaviť build systém, kostru PnModel (Place, Transition, Arc, PnNet) |
| St | Implementovať QGraphicsScene s pridaním Place/Transition drag & drop | Implementovať PnFileParser — sekcie Name, Comment, Inputs, Outputs, Variables |
| Št | Kresliť orientované hrany (QGraphicsLineItem so šipkou), editácia váh | Implementovať PnFileParser — sekcie Places, Transitions (arcs + when + do) |
| Pi | PlaceDialog, TransitionDialog (editácia mena, počiatočných tokenov, akcie) | PnFileWriter — serializácia späť do .pn, unit test round-trip parser/writer |

**Výstupy týždňa 1:**
- [ ] Qt projekt sa prekladá (`make` funguje)
- [ ] Možno nakresliť sieť a uložiť do .pn súboru
- [ ] PnFileParser načíta oba ukážkové príklady zo zadania
- [ ] `udp_protocol.h` a `pn_model.h` sú dohodnuté a commitnuté

---

### Týždeň 2 — Engine a generátor kódu

**Cieľ:** Fungujúci PetriNetEngine v izolácii, fungujúci CodeGenerator, prvý vygenerovaný a spustiteľný interpret.

| Deň | Osoba A | Osoba B |
|---|---|---|
| Po–Ut | PropertiesPanel (bočný panel, editácia podmienok a akcií), ArcDialog | PetriNetEngine — marking, isEnabled(), fireMaximalSet() |
| St | Uloženie/načítanie siete v GUI (volanie parsera a writera) | TimerManager — `std::priority_queue`, odpočítavanie @delay_ms |
| Št | CodeGenerator skeleton: šablóna hlavného .cpp interpretu | ScriptRuntime — valueof(), defined(), output(), tokens(), elapsed(), now() |
| Pi | CodeGenerator: generovanie fireTransition() pre každý prechod | UdpServer v interpreti — príjem INPUT správ, odosielanie STATE/LOG |

**Výstupy týždňa 2:**
- [ ] `PetriNetEngine` prechádza unit testami na TOF_PN_5s príklade
- [ ] `CodeGenerator` vygeneruje .cpp, ktorý sa preloží
- [ ] Vygenerovaný interpret beží a produkuje výpis na stdout
- [ ] Interpret reaguje na `echo 'INPUT in 1' | nc -u localhost 7000`

---

### Týždeň 3 — Integrácia GUI s interpretom, monitor

**Cieľ:** End-to-end tok: nakresliť sieť → vygenerovať → spustiť → monitorovať v GUI.

| Deň | Osoba A | Osoba B |
|---|---|---|
| Po–Ut | UdpClient implementácia, AppController (QProcess spustenie interpretu) | Odladenie UdpServer — STATE správy každých 200 ms alebo po zmene |
| St | MonitorPanel — live aktualizácia počtov tokenov v diagramu (farby) | Formát LOG správ — firing, timeout, variable change, output event |
| Št | InjectPanel — formulár meno vstupu + hodnota, tlačidlo Send | EventLogView — scrollovací QPlainTextEdit, parsovanie LOG správ |
| Pi | Zvýraznenie enabled prechodov (zelená), pending timeouts (oranžová) | Implementácia príkazu QUIT (GUI požiada interpret o ukončenie) |

**Výstupy týždňa 3:**
- [ ] Kompletný end-to-end tok funguje na TOF_PN_5s príklade
- [ ] GUI zobrazuje živý marking, enabled prechody, pending timery
- [ ] Možno injektovať vstupy z GUI
- [ ] Log udalostí sa zobrazuje v reálnom čase

---

### Týždeň 4 — Leštenie, dokumentácia, testovanie, odovzdanie

**Cieľ:** Stabilná aplikácia, kompletná dokumentácia, príklady, archív pre odovzdanie.

| Deň | Osoba A | Osoba B |
|---|---|---|
| Po | Opraviť chyby v GUI (edge cases: null arcs, samosladička, mazanie) | Opraviť engine chyby (conflict resolution, cancel timer pri nedostatku tokenov) |
| Ut | Doplniť Doxygen komentáre vo všetkých GUI súboroch | Doplniť Doxygen komentáre v engine, parser, codegen súboroch |
| St | Pridať príklad sietí do `examples/` (TOF_PN_5s, TOF_PN, vlastný príklad) | Skontrolovať `make pack`, rozbaľ v prázdnom adresári, skontroluj preklad |
| Št | Napísať README.txt (autori, čo je implementované, obmedzenia, návod) | Vytvoriť konceptuálny návrh (class diagram) v PDF |
| Pi | Záverečné testovanie na serveri merlin, opravy kompatibility Qt5 | Záverečné testovanie, finálny commit, odovzdanie archivu |

**Výstupy týždňa 4:**
- [ ] `make` funguje na serveri merlin bez absolútnych ciest
- [ ] `make doxygen` generuje HTML dokumentáciu do `doc/`
- [ ] `make pack` vytvára správne pomenovaný archív
- [ ] README.txt obsahuje mená autorov, popis, obmedzenia
- [ ] PDF konceptuálny návrh je pribalený v archíve
- [ ] Oba príklady zo zadania fungujú end-to-end

---

## 4. Kompletný zoznam modulov a súborov

### Adresárová štruktúra

```
proj/
├── Makefile                        # top-level Makefile
├── README.txt                      # popis projektu, autori
├── src/
│   ├── Makefile                    # src-level Makefile (qmake alebo cmake)
│   ├── icp_project.pro             # Qt projekt súbor
│   │
│   ├── common/
│   │   ├── udp_protocol.h          # (A+B) definície formátov UDP správ
│   │   └── pn_model.h              # (A+B) forward declarations a spoločné typy
│   │
│   ├── model/
│   │   ├── pn_place.h / .cpp       # (B) trieda Place
│   │   ├── pn_transition.h / .cpp  # (B) trieda Transition
│   │   ├── pn_arc.h / .cpp         # (B) trieda Arc
│   │   ├── pn_net.h / .cpp         # (B) trieda PnNet (celá sieť)
│   │   ├── pn_file_parser.h / .cpp # (B) parser .pn textového formátu
│   │   └── pn_file_writer.h / .cpp # (B) writer PnNet -> .pn súbor
│   │
│   ├── engine/
│   │   ├── petri_net_engine.h / .cpp  # (B) jadro — marking, firing, sémantika
│   │   ├── timer_manager.h / .cpp     # (B) správa @delay_ms časovačov
│   │   └── script_runtime.h / .cpp   # (B) vestavené funkcie: valueof, elapsed...
│   │
│   ├── codegen/
│   │   ├── code_generator.h / .cpp   # (B) generátor .cpp interpretu z PnNet
│   │   └── templates/
│   │       ├── interpreter_main.cpp.tpl  # (B) šablóna main() interpretu
│   │       └── runtime_header.h.tpl     # (B) šablóna runtime hlavičky
│   │
│   ├── network/
│   │   ├── udp_client.h / .cpp    # (A) Qt QUdpSocket, odosielanie/príjem v GUI
│   │   └── udp_server.h / .cpp    # (B) UDP server v generovanom interpreti
│   │
│   ├── gui/
│   │   ├── main_window.h / .cpp        # (A) hlavné okno, menu, toolbar, tabyy
│   │   ├── app_controller.h / .cpp     # (A) orchestrácia: generovanie, QProcess, UDP
│   │   ├── graphics_editor.h / .cpp    # (A) QGraphicsView + QGraphicsScene, kreslenie
│   │   ├── graphics_place_item.h / .cpp    # (A) vizuálny prvok miesta
│   │   ├── graphics_transition_item.h / .cpp  # (A) vizuálny prvok prechodu
│   │   ├── graphics_arc_item.h / .cpp      # (A) vizuálny prvok hrany so šipkou
│   │   ├── properties_panel.h / .cpp   # (A) bočný panel pre editáciu vlastností
│   │   ├── monitor_panel.h / .cpp      # (A) live zobrazenie markingu a stavu
│   │   ├── inject_panel.h / .cpp       # (A) panel pre manuálne injektovanie vstupov
│   │   ├── event_log_view.h / .cpp     # (A) scrollovací log udalostí
│   │   ├── monitor_adapter.h / .cpp    # (A) preklad UDP správ na Qt signály
│   │   └── dialogs/
│   │       ├── new_net_dialog.h / .cpp        # (A) dialóg pre novú sieť
│   │       ├── place_dialog.h / .cpp          # (A) editácia vlastností miesta
│   │       ├── transition_dialog.h / .cpp     # (A) editácia vlastností prechodu
│   │       ├── arc_dialog.h / .cpp            # (A) editácia váhy hrany
│   │       └── variables_dialog.h / .cpp      # (A) editácia premenných a I/O
│   │
│   └── main.cpp                   # (A) vstupný bod Qt aplikácie
│
├── generated/                     # (auto) generovaný kód interpretu
│   ├── net_NAME.cpp               # generovaný pri CodeGenerator::generate()
│   └── interpreter_NAME           # skompilovaný interpret
│
├── examples/
│   ├── tof_pn_5s.pn               # príklad zo zadania: Timer Off 5s
│   ├── tof_pn.pn                  # príklad zo zadania: Timer Off s elapsed
│   └── semaphore.pn               # vlastný príklad: semafor so zdrojmi
│
├── doc/                           # (auto) Doxygen HTML výstup
└── Doxyfile                       # konfigurácia Doxygen
```

### Popis každého modulu

#### Spoločné (`common/`)

**`udp_protocol.h`**
Definuje konštanty portov, enumeráciu typov správ a inline funkcie pre serializáciu/deserializáciu UDP paketov. Je includovaný v GUI aj v generovanom interpreti — zabezpečuje kompatibilitu protokolu.

**`pn_model.h`**
Spoločná hlavička s forward deklaráciami, typedef-mi a pomocnými štruktúrami (napr. `ArcWeight`, `TokenCount`). Zabraňuje kruhovým závislosťám medzi modulmi.

#### Dátový model (`model/`)

**`pn_place.h / pn_place.cpp`**
Trieda `Place` nesie meno, počiatočný počet tokenov a voliteľnú place action (C++ kód ako `std::string`). Neobsahuje vizuálnu logiku — čistý dátový objekt.

**`pn_transition.h / pn_transition.cpp`**
Trieda `Transition` nesie meno, event name, guard výraz, delay_ms a action kód. Umožňuje dotaz `hasEvent()`, `hasGuard()`, `hasDelay()` pre generátor kódu.

**`pn_arc.h / pn_arc.cpp`**
Trieda `Arc` reprezentuje orientovanú hranu medzi Place a Transition (alebo opačne) s váhou `w >= 1`. Rozlišuje typ `INPUT` a `OUTPUT`.

**`pn_net.h / pn_net.cpp`**
Agregátna trieda `PnNet` drží vektory miest, prechodov a hrán. Poskytuje metódy `addPlace()`, `addTransition()`, `addArc()`, `findPlaceByName()` a podobne. Je centrálnym dátovým modelom aplikácie.

**`pn_file_parser.h / pn_file_parser.cpp`**
Ručne písaný parser textového .pn formátu. Číta sekcie `Jméno sítě`, `Vstupy`, `Výstupy`, `Proměnné`, `Místa`, `Přechody` a vracia naplnený `PnNet`. Hlási chyby s číslom riadku.

**`pn_file_writer.h / pn_file_writer.cpp`**
Serializuje `PnNet` objekt späť do textového .pn formátu. Garantuje round-trip kompatibilitu s parserom (to čo zapíše, parser znova načíta).

#### Engine (`engine/`)

**`petri_net_engine.h / petri_net_engine.cpp`**
Jadro runtime interpretu. Drží aktuálny marking (`std::map<std::string, int>`), implementuje `isEnabled(transition)`, `fireMaximalSet()` a hlavnú event slučku. Centrálny modul interpreta.

**`timer_manager.h / timer_manager.cpp`**
Spravuje prioritnú frontu čakajúcich časovačov pre prechody s `@delay_ms`. Poskytuje `schedule(transition, delay)`, `cancel(transition)`, `nextDeadline()` a `popExpired()`. Používa `std::chrono`.

**`script_runtime.h / script_runtime.cpp`**
Implementácia vestavených runtime funkcií prístupných v akciách: `valueof()`, `defined()`, `output()`, `tokens()`, `elapsed()`, `now()`. Tieto funkcie sú linkované do generovaného interpretu.

#### Generátor kódu (`codegen/`)

**`code_generator.h / code_generator.cpp`**
Prijme `PnNet` a vygeneruje `net_NAME.cpp` — standalone C++ program obsahujúci hardkódovanú logiku siete, includujúci `script_runtime.h` a `udp_server.h`. Používa šablóny zo `templates/`.

**`templates/interpreter_main.cpp.tpl`**
Šablóna main() funkcie generovaného interpretu: inicializácia, spustenie UDP servera, hlavná event slučka. Zástupné miesta (`{{NET_NAME}}`, `{{TRANSITIONS_CODE}}`) sú nahradené generátorom.

**`templates/runtime_header.h.tpl`**
Šablóna runtime hlavičky includovanej v generovanom .cpp: deklarácie, includes, extern funkcie.

#### Sieťová komunikácia (`network/`)

**`udp_client.h / udp_client.cpp`**
Qt wrapper okolo `QUdpSocket`. Odosiela `INPUT` a `QUIT` správy interpretu, prijíma `STATE` a `LOG` správy. Emituje Qt signály `stateReceived(StateMsg)`, `logReceived(LogMsg)` pre MonitorAdapter.

**`udp_server.h / udp_server.cpp`**
UDP server v generovanom interpreti (bez Qt, čistý POSIX socket). Beží v samostatnom vlákne, číta prichádzajúce správy do fronty, odosiela STATE/LOG správy klientovi (GUI).

#### GUI (`gui/`)

**`main_window.h / main_window.cpp`**
Hlavné okno aplikácie s menu (File, Run, Help), toolbarom a QTabWidget prepínajúcim medzi Editor a Monitor záložkami. Drží referenciu na AppController.

**`app_controller.h / app_controller.cpp`**
Orchestruje celý životný cyklus: generovanie kódu, kompiláciu (subprocess `make`), spustenie interpretu (`QProcess`), pripojenie cez UDP, a ukončenie. Prepína GUI medzi edit/monitor módom.

**`graphics_editor.h / graphics_editor.cpp`**
Hlavný editor postavený na `QGraphicsView` + `QGraphicsScene`. Implementuje pridávanie miest (double-click), prechodov (toolbar), kreslenie hrán (klik na miesto → klik na prechod). Synchronizuje zmeny s `PnNet`.

**`graphics_place_item.h / graphics_place_item.cpp`**
`QGraphicsEllipseItem` rozšírený o zobrazenie mena, počtu tokenov a farebné zvýraznenie počas monitora. Reaguje na signál `markingChanged` a aktualizuje zobrazenie tokenov.

**`graphics_transition_item.h / graphics_transition_item.cpp`**
`QGraphicsRectItem` rozšírený o zobrazenie mena prechodu. Počas monitora sa zafarbí na zelenú (enabled) alebo oranžovú (pending timeout).

**`graphics_arc_item.h / graphics_arc_item.cpp`**
`QGraphicsLineItem` so šipkou na konci (QPainterPath) a štítkom s váhou. Aktualizuje pozíciu šipky pri presúvaní nódy.

**`properties_panel.h / properties_panel.cpp`**
Bočný panel (QDockWidget) zobrazujúci a umožňujúci editáciu vlastností vybraného prvku. Pre miesto: meno, tokeny, akcia. Pre prechod: meno, event, guard, delay, action kód.

**`monitor_panel.h / monitor_panel.cpp`**
Panel zobrazujúci za behu: tabuľku aktuálnych počtov tokenov vo všetkých miestach, zoznam enabled prechodov, zoznam pending timeoutov s odpočítavaním. Aktualizuje sa pri každej `STATE` správe.

**`inject_panel.h / inject_panel.cpp`**
Panel s comboboxom vstupov definovaných v sieti, textovým poľom pre hodnotu a tlačidlom `Send`. Zavolá `UdpClient::sendInput(name, value)`.

**`event_log_view.h / event_log_view.cpp`**
`QPlainTextEdit` v read-only móde s automatickým scrollovaním. Zobrazuje LOG správy z interpretu s časovým razítkom, typom udalosti a detailmi.

**`monitor_adapter.h / monitor_adapter.cpp`**
Mediátor medzi `UdpClient` a GUI komponentmi. Parsuje prichádzajúce správy a distribúuje ich príslušným widgetom cez Qt signály/sloty. Izoluje GUI od sieťového formátu.

**`dialogs/new_net_dialog.h / .cpp`**
Modálny dialóg pre vytvorenie novej siete: pole pre meno siete a komentár.

**`dialogs/place_dialog.h / .cpp`**
Dialóg pre editáciu miesta: meno, počiatočný počet tokenov, voliteľný C++ kód place action.

**`dialogs/transition_dialog.h / .cpp`**
Dialóg pre editáciu prechodu: meno, event name, guard výraz (QTextEdit), delay_ms, action kód (QTextEdit).

**`dialogs/arc_dialog.h / .cpp`**
Jednoduchý dialóg pre nastavenie váhy hrany (QSpinBox, min 1).

**`dialogs/variables_dialog.h / .cpp`**
Dialóg pre správu vstupov, výstupov a interných premenných siete (QTableWidget s add/remove riadkami).

**`main.cpp`**
Vstupný bod Qt aplikácie: inicializácia `QApplication`, inštanciácia `MainWindow`, spustenie event slučky.

---

## 5. UDP protokol

### Porty a smer

```
GUI (klient)  ──INPUT/QUIT──►  Interpret (server, port 7000)
GUI (klient)  ◄──STATE/LOG──   Interpret (server, port 7001)
```

Interpret odosiela STATE a LOG na port 7001; GUI počúva na 7001.

### Formát správ (textový, riadok = 1 UDP datagram)

Každá správa je UTF-8 reťazec, fieldy oddelené tabelátorom `\t`, zakončené `\n`.

#### INPUT — GUI pošle vstup do interpretu

```
INPUT\t<net_name>\t<input_name>\t<value>\n
```

Príklad:
```
INPUT\tTOF_PN_5s\tin\t1\n
```

#### QUIT — GUI žiada ukončenie interpretu

```
QUIT\t<net_name>\n
```

#### STATE — interpret odosiela stav siete (pravidelne alebo po každej zmene)

```
STATE\t<net_name>\t<timestamp_ms>\t<marking_json>\t<enabled_list>\t<pending_list>\t<vars_json>\n
```

- `<timestamp_ms>` — čas od štartu interpretu (now())
- `<marking_json>` — JSON objekt: `{"P1":2,"P2":0,"P3":1}`
- `<enabled_list>` — čiarkou oddelené mená enabled prechodov: `T_on,T_off_start`
- `<pending_list>` — `timer_name:remaining_ms` oddelené čiarkou: `T_timeout_off:3842`
- `<vars_json>` — JSON objekt interných premenných + last known values vstupov: `{"timeout":5000,"in":"1"}`

Príklad:
```
STATE\tTOF_PN_5s\t1234\t{"IDLE":0,"ACTIVE":1,"TIMING":0}\tT_off_start\t\t{"timeout":5000,"in":"1"}\n
```

#### LOG — interpret odosiela udalosť (každá udalosť = 1 správa)

```
LOG\t<net_name>\t<timestamp_ms>\t<event_type>\t<details>\n
```

Typy udalostí (`<event_type>`):

| Typ | Popis | `<details>` |
|---|---|---|
| `FIRED` | prechod bol odpálený | `transition=T_on tokens_consumed=IDLE:1 tokens_produced=ACTIVE:1` |
| `TIMEOUT_FIRED` | timer vypršal a prechod bol odpálený | `transition=T_timeout_off` |
| `TIMEOUT_IGNORED` | timer vypršal ale prechod nebol povolený | `transition=T_timeout_off reason=not_enabled` |
| `TIMER_SCHEDULED` | timer bol naplánovaný | `transition=T_timeout_off delay_ms=5000` |
| `INPUT_RECEIVED` | bol prijatý vstupný event | `name=in value=1` |
| `OUTPUT_EMITTED` | output() bol zavolaný | `name=out value=1` |
| `VAR_CHANGED` | interná premenná sa zmenila | `name=timeout old=5000 new=3000` |
| `PLACE_ACTION` | place action bola vykonaná | `place=ACTIVE` |
| `NET_STARTED` | interpret sa spustil | `net=TOF_PN_5s` |
| `NET_STOPPING` | interpret sa zastavuje | `net=TOF_PN_5s` |

Príklad:
```
LOG\tTOF_PN_5s\t1234\tFIRED\ttransition=T_on tokens_consumed=IDLE:1 tokens_produced=ACTIVE:1\n
```

### Identifikácia siete pri pripojení

Pri štarte interpret odosiela na broadcast `255.255.255.255:7001` správu:

```
ANNOUNCE\t<net_name>\t<listen_port>\n
```

GUI pri štarte počúva 2 sekundy na announce, ak ho zachytí — automaticky sa pripojí a načíta príslušný .pn súbor.

### Maximálna veľkosť datagramu

UDP payload je limitovaný na 65507 bajtov. STATE správy s veľkými sieťami môžu byť väčšie — odporúčame rozdeliť marking a vars do samostatných datagramov alebo použiť kompresiu JSON (krátke kľúče).

---

## 6. Textový formát .pn súboru

### Syntax

Formát je čitateľný text editovateľný ručne. Kódovanie UTF-8. Komentáre začínajú `#`.

```
# Komentár môže byť kdekoľvek
Jméno sítě:
    <identifikátor_siete>

Komentář:
    <ľubovolný text, môže byť viacriadkový, odsadený aspoň 4 medzerami>

Vstupy:
    <in1>
    <in2>

Výstupy:
    <out1>

Proměnné:
    <typ> <meno> = <hodnota>
    <typ> <meno> = <hodnota>

Místa (počáteční tokeny, volitelně akce):
    <MENO> (<tokeny>) : { <C++_kód_place_action> }
    <MENO> (<tokeny>)

Přechody a jejich podmínky:
<MENO> :
    in:  <MIESTO>*<váha>, <MIESTO>*<váha>
    out: <MIESTO>*<váha>
    when: [<event_name>] [ [<guard_expr>] ] [@ <delay_expr>]
    do: { <C++_kód_akcie> }
```

### Pravidlá

- Sekcie sú oddelené prázdnymi riadkami, poradie je záväzné.
- Mená miest, prechodov, vstupov, výstupov: `[A-Za-z_][A-Za-z0-9_]*`
- `when:` — všetky tri časti sú voliteľné; keď chýba event aj guard aj delay, prechod je vždy povolený ak má tokeny.
- `do:` — voliteľný blok; obsah `{ ... }` môže byť viacriadkový.
- Place action `{ }` — voliteľná; prázdne `{ }` je prípustné.
- Váha defaultne 1 ak nie je uvedená, t.j. `MENO*1 == MENO`.
- `delay_expr` môže byť literál (`1000`) alebo meno premennej (`timeout`).

### Kompletný príklad — TOF_PN_5s

```pn
# Timer-off 5s — jednoduchá verzia
Jméno sítě:
    TOF_PN_5s

Komentář:
    Timer to off, jednoduchá verzia.
    Vstup in=1 aktivuje výstup, vstup in=0 spustí 5s timer.

Vstupy:
    in

Výstupy:
    out

Proměnné:
    int timeout = 5000

Místa (počáteční tokeny, volitelně akce):
    IDLE   (1) : { output("out", 0); }
    ACTIVE (0) : { output("out", 1); }
    TIMING (0) : { }

Přechody a jejich podmínky:
T_on :
    in:  IDLE*1
    out: ACTIVE*1
    when: in [ atoi(valueof("in")) == 1 ]
    do: { }

T_off_start :
    in:  ACTIVE*1
    out: TIMING*1
    when: in [ atoi(valueof("in")) == 0 ]
    do: { }

T_cancel_off :
    in:  TIMING*1
    out: ACTIVE*1
    when: in [ atoi(valueof("in")) == 1 ]
    do: { }

T_timeout_off :
    in:  TIMING*1
    out: IDLE*1
    when: @ timeout
    do: { }
```

### Kompletný príklad — TOF_PN (s elapsed a set_to)

```pn
# Timer-off s nastaviteľným timeoutom a dotazom na zostatok
Jméno sítě:
    TOF_PN

Komentář:
    Timer to off, umí nastavit timeout a na požádání sdělit zbývající čas timeru.

Vstupy:
    in
    set_to
    req_rt

Výstupy:
    out
    rt

Proměnné:
    int timeout = 5000

Místa (počáteční tokeny, volitelně akce):
    IDLE   (1) : { output("out", 0); output("rt", 0); }
    ACTIVE (0) : { output("out", 1); output("rt", timeout); }
    TIMING (0) : { output("rt", timeout - elapsed("TIMING")); }

Přechody a jejich podmínky:
T_on :
    in:  IDLE*1
    out: ACTIVE*1
    when: in [ atoi(valueof("in")) == 1 ]
    do: { if (defined("set_to")) { timeout = atoi(valueof("set_to")); } }

T_off_start :
    in:  ACTIVE*1
    out: TIMING*1
    when: in [ atoi(valueof("in")) == 0 ]
    do: { if (defined("set_to")) { timeout = atoi(valueof("set_to")); } }

T_cancel_off :
    in:  TIMING*1
    out: ACTIVE*1
    when: in [ atoi(valueof("in")) == 1 ]
    do: { if (defined("set_to")) { timeout = atoi(valueof("set_to")); } }

T_timeout_off :
    in:  TIMING*1
    out: IDLE*1
    when: @ timeout
    do: { if (defined("set_to")) { timeout = atoi(valueof("set_to")); } }

T_set_idle :
    in:  IDLE*1
    out: IDLE*1
    when: set_to
    do: { timeout = atoi(valueof("set_to")); output("rt", 0); }

T_set_active :
    in:  ACTIVE*1
    out: ACTIVE*1
    when: set_to
    do: { timeout = atoi(valueof("set_to")); output("rt", timeout); }

T_set_timing :
    in:  TIMING*1
    out: TIMING*1
    when: set_to
    do: { timeout = atoi(valueof("set_to")); output("rt", timeout - elapsed("TIMING")); }

T_req_idle :
    in:  IDLE*1
    out: IDLE*1
    when: req_rt
    do: { output("rt", 0); }

T_req_active :
    in:  ACTIVE*1
    out: ACTIVE*1
    when: req_rt
    do: { output("rt", timeout); }

T_req_timing :
    in:  TIMING*1
    out: TIMING*1
    when: req_rt
    do: { output("rt", timeout - elapsed("TIMING")); }
```

### Poznámky k parseru

- `when:` línka: parser extrahuje `event_name` (identifikátor pred `[`), `guard` (obsah `[...]`), `delay` (hodnota za `@`). Všetky tri časti môžu chýbať v ľubovolnej kombinácii.
- Kód v `do: { ... }` a `: { ... }` môže byť viacriadkový; parser počíta závorky `{` `}` pre určenie konca bloku.
- Prázdny blok `do: { }` je prípustný a generuje prázdne telo funkcie.

---

## 7. Makefile ciele

### Top-level `Makefile` (v koreňovom adresári projektu)

```makefile
# Makefile — top level
# Autori: xname01, xname02

LOGIN1 = xname01
LOGIN2 = xname02
ARCHIVE = $(LOGIN1)-$(LOGIN2).zip

.PHONY: all run doxygen clean pack

all:
	$(MAKE) -C src

run: all
	./src/icp_petri

doxygen:
	doxygen Doxyfile

clean:
	$(MAKE) -C src clean
	rm -rf doc/html doc/latex
	rm -rf generated/*.cpp generated/interpreter_*
	rm -f $(ARCHIVE)

pack: clean
	zip -r $(ARCHIVE) \
	    Makefile \
	    README.txt \
	    Doxyfile \
	    src/ \
	    examples/ \
	    design.pdf
	@echo "Archív vytvorený: $(ARCHIVE)"
	@echo "Veľkosť: $$(du -sh $(ARCHIVE))"
```

### `src/Makefile` (alebo `src/CMakeLists.txt`)

Odporúčame použiť qmake s `.pro` súborom — je priamočiarejší a Qt-natívny:

```makefile
# src/Makefile
.PHONY: all clean

all:
	cd src && qmake icp_project.pro && $(MAKE)

clean:
	cd src && $(MAKE) clean
	rm -f src/Makefile src/*.o src/icp_petri
```

### `src/icp_project.pro` (Qt qmake projekt)

```qmake
QT += core gui widgets network
CONFIG += c++17
TARGET = icp_petri
TEMPLATE = app

SOURCES += \
    main.cpp \
    model/pn_place.cpp \
    model/pn_transition.cpp \
    model/pn_arc.cpp \
    model/pn_net.cpp \
    model/pn_file_parser.cpp \
    model/pn_file_writer.cpp \
    codegen/code_generator.cpp \
    network/udp_client.cpp \
    gui/main_window.cpp \
    gui/app_controller.cpp \
    gui/graphics_editor.cpp \
    gui/graphics_place_item.cpp \
    gui/graphics_transition_item.cpp \
    gui/graphics_arc_item.cpp \
    gui/properties_panel.cpp \
    gui/monitor_panel.cpp \
    gui/inject_panel.cpp \
    gui/event_log_view.cpp \
    gui/monitor_adapter.cpp \
    gui/dialogs/new_net_dialog.cpp \
    gui/dialogs/place_dialog.cpp \
    gui/dialogs/transition_dialog.cpp \
    gui/dialogs/arc_dialog.cpp \
    gui/dialogs/variables_dialog.cpp

HEADERS += \
    common/udp_protocol.h \
    common/pn_model.h \
    model/pn_place.h \
    # ... (všetky .h súbory)

INCLUDEPATH += . common model engine codegen network gui
```

### Makefile pre generovaný interpret

Generátor kódu zapíše aj `generated/Makefile`:

```makefile
# generated/Makefile — automaticky generovaný CodeGeneratorom
CXX = g++
CXXFLAGS = -std=c++17 -O2 -I../src/engine -I../src/network -I../src/common
TARGET = interpreter_$(NET_NAME)

$(TARGET): net_$(NET_NAME).cpp ../src/engine/petri_net_engine.cpp \
           ../src/engine/timer_manager.cpp ../src/engine/script_runtime.cpp \
           ../src/network/udp_server.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread

clean:
	rm -f $(TARGET)
```

### Doxyfile — kľúčové nastavenia

```
PROJECT_NAME    = "ICP Petri Net Editor"
OUTPUT_DIRECTORY = ../doc
INPUT           = ../src
RECURSIVE       = YES
SOURCE_BROWSER  = YES
GENERATE_HTML   = YES
GENERATE_LATEX  = NO
EXTRACT_ALL     = YES
```

---

## 8. Kritické rozhodnutia

Tieto rozhodnutia treba urobiť **pred začiatkom kódovania** a zdokumentovať ich v README.txt a v komentároch kódu. Neskoršia zmena je nákladná.

---

### 8.1 Sémantika `elapsed("place")`

**Otázka:** Čo presne meria `elapsed("place")`?

**Rozhodnutie (odporúčané):** Čas v milisekundách od **poslednej zmeny počtu tokenov** v danom mieste. Resetuje sa pri každom pridaní alebo odobraní tokenu.

**Dôsledky:**
- Prechod `P -> P` (self-loop, nezmení počet tokenov) NERESETUJE `elapsed`.
- Implementácia: každé miesto drží `int64_t last_token_change_ms` aktualizované v `engine::addToken()` a `engine::removeToken()`.
- `elapsed("place")` = `now() - place.last_token_change_ms`

**Alternatíva (neodporúčaná):** resetovať pri každom odpálení prechodu ktorý miesto zapísuje — menej predvídateľné.

---

### 8.2 Sémantika `elapsed("transition")`

**Otázka:** Čo meria `elapsed("transition")` pre časovaný prechod?

**Rozhodnutie:** Doba **nepretržitej povoliteľnosti** časovaného prechodu — čas od kedy je prechod kontinuálne enabled. Resetuje sa ak prechod prestane byť enabled (napr. tokeny odobral iný prechod).

**Implementácia:** každý prechod drží `int64_t became_enabled_ms`; pri každej iterácii engine-u ak prechod prešiel z disabled na enabled, zaznamená čas.

---

### 8.3 Place Actions — kedy sa spúšťajú

**Otázka:** Kedy sa spustí place action miesta P?

**Rozhodnutie:** Place action sa spustí **pre každý pridaný token** — teda ak prechod produkuje `P*3`, place action sa spustí 3×.

**Poradie:** Najprv sa odoberie z vstupných miest, vykoná sa action prechodu, potom sa pridávajú tokeny do výstupných miest (a pre každý token sa spustí place action).

**Zdôvodnenie:** Konzistentné so SFC/Grafcet sémantikou kde každý krok má entry action.

---

### 8.4 Riešenie konfliktov (Conflict Resolution)

**Otázka:** Ak dva prechody súťažia o rovnaké tokeny, ktorý sa odpáli?

**Rozhodnutie:** **Pevné poradie podľa pozície v .pn súbore** — prechod definovaný skôr v súbore má vyššiu prioritu. Toto je deterministické, jednoduché a čitateľné z .pn súboru.

**Implementácia:** `PnNet` drží prechody v `std::vector` v poradí parseru. Engine prechádza vektor od začiatku pri výbere maximálnej množiny.

**Alternatíva pre budúcnosť:** Explicitné pole `priority:` v definícii prechodu (voliteľné rozšírenie).

---

### 8.5 Timer Cancellation — kedy sa zrušia časovače

**Otázka:** Čo sa stane ak prechod `T` má bežiaci timer a jeho vstupné miesta stratia tokeny (iný prechod ich spotreboval)?

**Rozhodnutie (základná verzia):** Timer **NECH AJ DOBEHNE**, pri timeoutu engine skontroluje povoliteľnosť. Ak prechod nie je povolený, timer sa zahodí a zaloguje `TIMEOUT_IGNORED`. Toto je jednoduchšie na implementáciu.

**Dôsledok:** Prechody môžu mať "zombie timery". Pre TOF_PN príklady toto nevadí.

**Alternatíva (voliteľné rozšírenie):** Aktívne čistenie — pri každej zmene markingu skontrolovať všetky čakajúce timery a zrušiť tie pre zakázané prechody.

---

### 8.6 `defined("input")` — čo to znamená

**Otázka:** `defined("in")` vracia `true` ak...?

**Rozhodnutie:** `defined("input_name")` vracia `true` ak vstup **bol niekedy nastavený** od štartu interpretu (t.j. last-known value existuje). Nesleduje či bol nastavený "od posledného čítania" — to by vyžadovalo komplexnú logiku per-event.

**Implementácia:** `std::map<std::string, std::string> input_values`; `defined(name)` = `input_values.count(name) > 0`.

---

### 8.7 Maximálna množina nezávislých odpálení

**Otázka:** Ako nájsť maximálnu množinu nezávislých prechodov?

**Rozhodnutie:** Greedy algoritmus v poradí vektora prechodov:
1. Vytvor kópiu aktuálneho markingu.
2. Pre každý enabled prechod v poradí: ak má dostatok tokenov v kópii markingu, pridaj ho do fire set a ober tokeny z kópie.
3. Opakuj krok 1-2 kým nedôjde k zmene (stabilizácia).

Toto nie je globálne optimálne maximum, ale je deterministické a efektívne.

---

### 8.8 Generovaný interpret vs. vestavený interpret

**Rozhodnutie:** Generovaný standalone C++ binary (nie shared library, nie vestavený interpret). Dôvod: zadanie to explicitne navrhuje, je to jednoduchšie na debugovanie, akcie sú kompilované (výkon), kompilátor (gcc) hlási chyby v akciách.

**Build flow:** `CodeGenerator::generate()` zapíše `generated/net_NAME.cpp` → `QProcess("make", ["-C", "generated"])` → výsledný binary `generated/interpreter_NAME` → `QProcess::start()`.

---

## 9. Tech stack a knižnice

### Základné požiadavky

| Technológia | Verzia | Účel |
|---|---|---|
| C++ | C++17 | celý projekt |
| Qt | 5.5+ (Qt6 odporúčané) | GUI, sieť, procesy |
| GCC / Clang | aktuálna | kompilátor |
| qmake | s Qt | build systém pre GUI |
| Doxygen | aktuálna | generovanie dokumentácie |

### Qt moduly použité v GUI

| Modul | Použitie |
|---|---|
| `Qt::Widgets` | QMainWindow, QGraphicsView, QDockWidget, dialógy |
| `Qt::Network` | QUdpSocket v UdpClient |
| `Qt::Core` | QProcess, QTimer, signály/sloty, QString |
| `Qt::Gui` | QPainter, QPainterPath (šipky na hranách) |

### Štandardná knižnica C++ použitá v engine/modeli

| Súčasť STL | Použitie |
|---|---|
| `std::vector` | zoznam miest, prechodov, hrán |
| `std::map` | marking (`place_name -> token_count`), input values |
| `std::priority_queue` | TimerManager (najskôr expirujúci timer prvý) |
| `std::chrono` | meranie času, `now()`, `elapsed()` |
| `std::string` | mená, kódy akcií |
| `std::thread` | UdpServer v interpreti (samostatné vlákno) |
| `std::mutex` / `std::condition_variable` | synchronizácia fronty udalostí v interpreti |

### Externé knižnice (bez Boost)

Žiadne externé knižnice nie sú potrebné — zámerné rozhodnutie pre maximálnu prenositeľnosť a dodržanie požiadaviek zadania.

### Platforma a prenositeľnosť

- **Primárna platforma:** Linux (server merlin)
- **Sekundárna platforma:** Windows (Qt je cross-platform)
- **Absolútne cesty:** NESMÚ byť v Makefile — Qt qmake to garantuje ak sa nepoužijú absolútne cesty v .pro súbore
- **Qt verzia na merline:** Qt 5.5.1 v `/usr/local/share/Qt-5.5.1/`; Makefile nesmie byť závislý na tejto ceste:

```makefile
# Správne — nájde qmake v PATH
all:
	cd src && qmake icp_project.pro && $(MAKE)

# NESPRÁVNE — absolútna cesta
all:
	cd src && /usr/local/share/Qt-5.5.1/bin/qmake icp_project.pro && $(MAKE)
```

### Odporúčané konvencie kódu

- Každý `.h` a `.cpp` súbor začína Doxygen hlavičkou:

```cpp
/**
 * @file pn_net.h
 * @brief Trieda PnNet — agregát všetkých prvkov Petriho siete.
 * @author xname01 (xname01@stud.fit.vutbr.cz)
 * @author xname02 (xname02@stud.fit.vutbr.cz)
 * @date 2026-04
 */
```

- Include guards: `#pragma once` (podporované GCC aj MSVC)
- Odsadenie: 4 medzery (nie tabulátory)
- Mená tried: `PascalCase`, metódy: `camelCase`, premenné: `snake_case`
- Qt signály/sloty: prefixovať `on_` pre sloty, bez prefixu pre signály

---

## Rýchly štart pre dvojicu

### Deň 1 — nastavenie

```bash
# Osoba A
git clone <repo>
mkdir -p src/gui src/model src/codegen src/network src/common src/engine
touch src/main.cpp src/icp_project.pro
# Skopírovať kostru MainWindow z Qt Creator príkladu

# Osoba B
# V tom istom repozitári
mkdir -p src/model src/engine src/codegen src/network src/common
touch src/model/pn_net.h src/model/pn_net.cpp
# Začať s PnNet a PnPlace triedami
```

### Prvý commit — dohodnuté rozhrania

Pred akýmkoľvek vážnym kódovaním commitnúť:
1. `src/common/udp_protocol.h` — vyplnený, dohodnutý
2. `src/common/pn_model.h` — minimálne forward deklarácie
3. `.pn` formát — oba príklady zo zadania ako `examples/tof_pn_5s.pn` a `examples/tof_pn.pn`
4. Tento dokument `PLAN.md`

### Overenie end-to-end v týždni 3

```bash
# 1. Spustíme GUI
make run

# 2. Otvoríme examples/tof_pn_5s.pn

# 3. Klikneme Generate + Run — interpret sa spustí

# 4. V inject paneli: meno=in, hodnota=1, Send
# Očakávame: LOG FIRED T_on, marking ACTIVE=1

# 5. V inject paneli: meno=in, hodnota=0, Send
# Očakávame: LOG FIRED T_off_start, timer scheduled T_timeout_off:5000

# 6. Po 5 sekundách:
# Očakávame: LOG TIMEOUT_FIRED T_timeout_off, marking IDLE=1
```

---

*Dokument vygenerovaný ako projektová roadmapa pre dvojicu. Aktualizovať podľa potreby počas vývoja.*

---

## 10. Checklist — stav implementácie

### Model (src/model/)
- [x] `Place` — meno, tokeny, akcia, pozícia, ID
- [x] `Transition` — meno, event, guard, delay, akcia, pozícia, ID
- [x] `Arc` — typ (INPUT/OUTPUT), váha, waypoints, ID
- [x] `PnNet` — agregát, ID countery, add/remove/find metódy
- [x] `PnFileParser` — načítanie .pn súboru (všetky sekcie)
- [x] `PnFileWriter` — serializácia PnNet do .pn súboru
- [x] Parser — round-trip otestovaný: 147/147 passed (tof_pn_5s, tof_pn, semaphore)

### GUI — Editor (src/gui/)
- [x] `MainWindow` — menu, toolbar, QTabWidget, docks (Properties, EventLog, InjectInput)
- [x] `AppController` — net(), addPlace/Transition/Arc, removeX, updateItemPos, load/save, generateAndRun, stopInterpreter
- [x] `GraphicsEditor` — QGraphicsView, módy (Select/AddPlace/AddTransition/AddArc)
- [x] `GraphicsPlaceItem` — kruh, meno, počet tokenov, drag, boundingRect
- [x] `GraphicsTransitionItem` — obdĺžnik, meno, drag, boundingRect
- [x] `GraphicsArcItem` — šipka, váha, arrowhead, updateGeometry()
- [x] `GraphicsEditor` — toolbar sync cez modeChanged signal
- [x] `onOpenNet` / `onSaveNet` / `onSaveNetAs` — napojené cez AppController
- [x] `onNewNet` — NewNetDialog + vyčistenie scény
- [x] `PlaceDialog` — dialóg (meno, tokeny, akcia)
- [x] `TransitionDialog` — dialóg (meno, event, guard, delay, akcia)
- [x] `ArcDialog` — dialóg (váha)
- [x] `NewNetDialog` — dialóg (meno, komentár)
- [ ] `PropertiesPanel` — inline dock editácia (stub)
- [ ] `VariablesDialog` — dialóg (vstupy, výstupy, premenné) (stub)

### GUI — Monitor (src/gui/)
- [x] `MonitorPanel` — live tabuľka tokenov + enabled prechody
- [x] `InjectPanel` — injektovanie vstupov cez UDP, combo z net inputs
- [x] `EventLogView` — scrollovací log s časovými razítkami + compile output
- [x] `MonitorAdapter` — napojenie UdpClient signálov na GUI

### Sieť (src/network/)
- [x] `UdpClient` — QUdpSocket, bind 7001, send INPUT/QUIT, emit stateReceived/logReceived

### Engine / Generátor kódu (src/codegen/)
- [x] `CodeGenerator` — generuje kompletný standalone net_NAME.cpp
- [x] Runtime engine — marking, isEnabled, fireMaximalSet (greedy), timers (zombie), event-driven
- [x] Script API — valueof(), defined(), output(), tokens(), elapsed()
- [x] UDP server v interpreti — POSIX non-blocking recvfrom, send STATE/LOG/ANNOUNCE
- [x] Generovaný interpret skompilovaný a otestovaný (semaphore, tof_pn_5s, tof_pn)

### Build a dokumentácia
- [x] `make` — projekt sa prekladá (0 errors)
- [x] `make run` — aplikácia sa spustí
- [x] `make test` — 147/147 passed
- [ ] `make doxygen` — generuje HTML dokumentáciu
- [ ] `make pack` — vytvorí archív pre odovzdanie
- [ ] `README.txt` — doplniť po implementácii

### Testovanie
- [x] Round-trip test: 147/147 passed na všetkých 3 príkladoch
- [x] Generovaný interpret: semaphore posiela správny STATE + ANNOUNCE cez UDP
- [ ] End-to-end GUI: nakresliť sieť → generate → run → inject → monitor
- [ ] Preklad na serveri merlin (Qt 5.5.1)
