ICP Project — Vizuálny editor, generátor kódu a runtime monitor interpretovaných Petriho sietí
================================================================================================

Autori:
  Samuel Fačka  (xfackas00)  xfackas00@stud.fit.vutbr.cz
  Arťom Hanzel  (xhanzea00)  xhanzea00@stud.fit.vutbr.cz


Popis
-----
Aplikácia umožňuje vizuálne špecifikovať event-driven Petriho sieť s multiset sémantikou,
uložiť ju do textového .pn formátu, vygenerovať standalone C++ interpreter, preložiť ho
pomocou g++ a monitorovať jeho beh cez UDP sockety.


Implementovaná funkcionalita
----------------------------

Model (src/model/):
  [x] Dátové štruktúry siete: PnNet, PnPlace, PnTransition, PnArc s plnou podporou
      váhovaných hrán, strážnych podmienok, vstupných udalostí, oneskorenia (delay)
      a akcií prechodov a miest
  [x] Načítanie (.pn formát) a uloženie siete — plný roundtrip cez PnFileParser / PnFileWriter
  [x] Deduplikácia premenných pri importe (duplicitné mená sa ignorujú)

Vizuálny editor (src/gui/):
  [x] Pridávanie a mazanie miest (kruhy) a prechodov (obdĺžniky)
  [x] Kreslenie orientovaných hrán s váhami medzi miestami a prechodmi
  [x] Editácia vlastností miesta: meno, počiatočný počet tokenov, akcia
  [x] Editácia vlastností prechodu: meno, vstupná udalosť, stráž, oneskorenie, akcia
  [x] Editácia vlastností hrany: váha
  [x] Správa premenných siete cez dialóg (pridať / odobrať / editovať)
  [x] Uloženie a načítanie siete (File → Save / Open)
  [x] Vytvorenie novej siete cez dialóg (meno + komentár)
  [x] Kontextové menu (pravý klik) na plátne

Generátor kódu a beh (src/codegen/, src/network/):
  [x] Jedným kliknutím vygeneruje standalone C++ interpreter (net_<Meno>.cpp)
  [x] Automaticky ho preloží pomocou g++ -std=c++17
  [x] Spustí interpret ako podproces (QProcess)
  [x] Komunikácia s interpretom cez UDP (port 45000 predvolene)
  [x] Injektovanie vstupných udalostí za behu cez panel (InjectPanel)
  [x] Poslanie príkazu QUIT pre riadené ukončenie interpretu

Runtime monitor:
  [x] Tabuľka živých počtov tokenov vo všetkých miestach (farebné zvýraznenie
      nenulových hodnôt)
  [x] Prehľad momentálne povolených prechodov (enabled transitions)
  [x] Log udalostí: odpálenia prechodov, zmeny tokenov, výstupy, externé vstupy
  [x] Statusový riadok: "Running" / "Stopped"

Inskripčný jazyk (generovaný interpret):
  [x] Akcie prechodov a miest sa prekladajú ako inline C++ kód
  [x] Vestavené funkcie: valueof(), defined(), output(), tokens(), elapsed(), now()
  [x] Sémantika: akcia miesta sa vykoná pri každom pridaní tokenu
  [x] Zpožděné prechody (@delay_ms): automatický timer, pokus o odpálenie po timeoutu
  [x] Maximálna množina nezávislých okamžitých prechodov sa odpaľuje sekvenčne
      v deterministickom poradí (poradie v súbore)
  [x] Pripojenie k bežiacej sieti: pri štarte interpret odošle ANNOUNCE správu
      s menom siete; GUI sa pokúsi načítať zodpovedajúci .pn súbor

Príklady (examples/):
  tof_pn_5s.pn   — Timer Off 5 s (jednoduchá verzia)
  tof_pn.pn      — Timer Off s nastaviteľným timeoutom a dotazom na zostatok
  semaphore.pn   — Semafor so zdrojmi
  test1.pn       — Jednoduchý testovací príklad


Obmedzenia / Známe nedostatky
------------------------------
- Po presune uzla (miesta / prechodu) na plátne sa geometria hrán neaktualizuje
  okamžite v niektorých prípadoch — vizuálna chyba bez vplyvu na funkčnosť.
- Funkcia output() v akcii prechodu/miesta prijíma int64_t, string a const char*.
  Iné číselné typy (double, int) je potrebné pretypovať na int64_t, napr.
  output("out", (int64_t)pocitadlo);
- Delayed prechod s povinnou vstupnou udalosťou (event @ delay) spustí timer
  hneď keď je marking dostatočný, nie až po príchode eventu. Praktický dopad
  je minimálny: timer sa zruší ak prechod nie je enabled pri vypršaní.
- Ak dva prechody viazané na rovnaký event sú v jednej maximálnej nezávislej
  množine, oba sa odpália — event sa teda spotrebuje dvakrát (edge case).
- Aplikácia bola vyvíjaná a testovaná na macOS s Qt 6.7; na Linuxe (merlin)
  môžu byť minimálne rozdiely vo vzhľade, funkčnosť by mala byť rovnaká.
- Príkaz "make run" funguje na macOS (.app bundle) aj Linuxe (binárka ./src/icp_petri).


Používanie (Usage)
------------------

1. Spustenie aplikácie
   make run

2. Vytvorenie novej siete
   File → New  — zadaj meno siete a komentár.

3. Kreslenie siete
   - Toolbar: vyber režim "Add Place" a klikni na plátno → pridá sa miesto (kruh).
   - Toolbar: vyber "Add Trans." a klikni → pridá sa prechod (obdĺžnik).
   - Toolbar: vyber "Add Arc" a ťahaj z miesta na prechod (alebo opačne) → hrana.
   - Dvojklik na miesto / prechod / hranu → otvorí dialóg pre editáciu vlastností.
   - Pravý klik na plátno → kontextové menu (pridaj / zmaž / vlastnosti).
   - Delete / Backspace → zmaže vybrané prvky aj s hranami.

4. Vstupné udalosti, výstupy, premenné
   File → Net Properties → tri záložky: Inputs / Outputs / Variables.
   Každú položku možno pridať, editovať a odobrať.

5. Podmienky prechodu
   Dvojklik na prechod → dialóg:
     Vstupná udalosť (event): meno vstupu, ktorý prechod spúšťa (prázdne = spontánny)
     Stráž (guard):           C++ boolovský výraz, napr.  atoi(valueof("in")) > 0
     Oneskorenie (delay):     číslo v ms alebo meno premennej, napr.  timeout
     Akcia (action):          C++ kód vykonaný pri odpálení, napr.  output("out", 1);

6. Uloženie / načítanie
   File → Save / Save As  — uloží sieť do .pn súboru.
   File → Open            — načíta existujúci .pn súbor.

7. Spustenie interpretu a monitoring
   Run → Generate && Run  (alebo F5)
     — vygeneruje C++ interpreter, preloží ho cez g++ a spustí.
     — automaticky prepne na záložku Monitor.
   Monitor záložka zobrazuje:
     - tabuľku živých počtov tokenov (zelená = nenulový)
     - zoznam aktuálne enabled prechodov
     - log udalostí (FIRED, INPUT_RECEIVED, OUTPUT, TIMEOUT_IGNORED …)
   Inject Input panel (ľavá dokovaná plocha):
     - vyber vstup z rozbaľovacieho zoznamu, zadaj hodnotu, klikni Send.
   Run → Stop  (alebo F6) — pošle QUIT interpretu a zastaví ho.

8. Pripojenie k bežiacemu interpretu
   Ak je interpret spustený samostatne (mimo GUI), GUI ho automaticky detekuje
   cez ANNOUNCE datagram pri štarte. Ak nájde zodpovedajúci .pn súbor
   v aktuálnom adresári alebo examples/, načíta ho a prepne na Monitor.

9. Inskripčný jazyk — dostupné funkcie v akciách
   valueof("vstup")       → const char*  posledná prijatá hodnota vstupu
   defined("vstup")       → bool         bol vstup niekedy nastavený?
   output("výstup", val)  → void         odošle výstupnú udalosť (int64_t / string)
   tokens("miesto")       → int64_t      aktuálny počet tokenov v mieste
   elapsed("miesto")      → int64_t      ms od poslednej zmeny tokenov v mieste
   elapsed("prechod")     → int64_t      ms odkedy je prechod nepretržite enabled
   now()                  → int64_t      ms od štartu interpretu
   atoi("ret")            → int          konverzia stringu na int (skratka)


Preklad a spustenie
--------------------
  make              — preloží GUI aplikáciu (volá src/Makefile)
  make run          — preloží a spustí aplikáciu (macOS)
  make doxygen      — vygeneruje HTML dokumentáciu do doc/html/
  make test         — preloží a spustí unit test (roundtrip parser/writer)
  make clean        — vymaže produkty prekladu
  make pack         — vytvorí archív xfackas00-xhanzea00.zip pre odovzdanie

Závislosti:
  Qt 6 (testované s Qt 6.7; Qt 5.15+ by malo fungovať)
  g++ alebo clang++ s podporou C++17
  pkg-config (pre automatickú detekciu Qt ciest v Makefile)


Štruktúra projektu
------------------
  src/
    main.cpp               — vstupný bod aplikácie
    inc/
      pn_model.h           — spoločné typy (Variable, ArcEndpoint …)
      udp_protocol.h       — UDP správy a ich serializácia/deserializácia
    model/
      pn_net.{h,cpp}       — model celej siete
      pn_place.{h,cpp}     — model miesta
      pn_transition.{h,cpp}— model prechodu
      pn_arc.{h,cpp}       — model hrany
      pn_file_parser.{h,cpp}— načítanie .pn formátu
      pn_file_writer.{h,cpp}— ukladanie .pn formátu
    codegen/
      code_generator.{h,cpp}— generátor standalone C++ interpretu
    network/
      udp_client.{h,cpp}   — UDP socket pre komunikáciu s interpretom
    gui/
      main_window.{h,cpp}  — hlavné okno (menu, panely, toolbar)
      app_controller.{h,cpp}— riadiaca logika (generate/compile/run/monitor)
      graphics_editor.{h,cpp}— QGraphicsView plátno pre kreslenie siete
      graphics_place_item.{h,cpp}     — grafický prvok miesta
      graphics_transition_item.{h,cpp}— grafický prvok prechodu
      graphics_arc_item.{h,cpp}       — grafický prvok hrany
      monitor_panel.{h,cpp}  — živé zobrazenie tokenov a povolených prechodov
      monitor_adapter.{h,cpp}— adaptér medzi UdpClient a MonitorPanel
      event_log_view.{h,cpp} — textový log udalostí
      inject_panel.{h,cpp}   — panel na injektovanie vstupov
      properties_panel.{h,cpp}— panel vlastností vybraného prvku
      dialogs/
        new_net_dialog.{h,cpp}       — dialóg pre novú sieť
        place_dialog.{h,cpp}         — dialóg pre editáciu miesta
        transition_dialog.{h,cpp}    — dialóg pre editáciu prechodu
        variables_dialog.{h,cpp}     — dialóg pre správu premenných
  examples/               — príklady .pn sietí
  doc/                    — generovaná Doxygen dokumentácia (make doxygen)
  design.pdf              — diagram tried (konceptuálny návrh)
  Doxyfile                — konfigurácia Doxygen
  Makefile                — hlavný Makefile
  README.txt              — tento súbor


Poznámka k použitiu AI
-----------------------
Pri vývoji bol použitý nástroj Claude (Anthropic) ako asistent — na generovanie
kostry komentárov (Doxygen), návrh štruktúry tried, ladenie edge-case bugov
a code review.