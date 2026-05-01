ICP Project — Vizuálny editor, generátor kódu a runtime monitor interpretovaných Petriho sietí
================================================================================================

Autori:
  xfacka00 (xfacka00@stud.fit.vutbr.cz)
  xlogin02 (xlogin02@stud.fit.vutbr.cz)

Popis:
  Aplikácia umožňuje vizuálne špecifikovať event-driven Petriho sieť s multiset sémantikou,
  uložiť ju do textového .pn formátu, vygenerovať standalone C++ interpreter, preložiť ho
  a monitorovať jeho beh cez UDP.

Implementovaná funkcionalita:
  [TODO - doplniť po implementácii]

Obmedzenia:
  [TODO - doplniť po implementácii]

Preklad a spustenie:
  make        — preloží GUI aplikáciu
  make run    — spustí aplikáciu
  make doxygen — vygeneruje HTML dokumentáciu do doc/

Závislosti:
  Qt 5.5+ (alebo Qt 6), g++/clang s podporou C++17

Príklady:
  examples/tof_pn_5s.pn   — Timer Off 5s (jednoduchá verzia)
  examples/tof_pn.pn       — Timer Off s nastaviteľným timeoutom
  examples/semaphore.pn    — Semafor so zdrojmi (vlastný príklad)
