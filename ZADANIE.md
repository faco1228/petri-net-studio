# Zadání: Nástroj pro vizuální editaci, generování kódu a monitorování běhu interpretovaných Petriho sítí
## Event-driven SFC/Grafcet s multiset sémantikou

---

# 1. Interpretovaná Petriho síť

Interpretovanou Petriho sítí zde chápeme jako **časovanou Petriho síť** rozšířenou o:

- vstupní události (eventy),
- interní proměnné a akce,
- strážní podmínky přechodů,
- generování výstupních událostí,
- plánování zpožděných odpálení přechodů (timeouty).

Síť je **event-driven** (reaktivní/proaktivní): neprovádí se cyklické skenování kroků v pevném taktu, ale systém čeká na externí událost nebo na timeout některého naplánovaného přechodu a teprve poté provede maximální množinu nezávislých odpálení.

Síť pracuje s **multiset sémantikou míst**:

- Každé místo (krok) obsahuje nezáporný počet tokenů (`0..N`).
- Tokeny jsou nerozlišené (neadresné). V základní verzi nemají vlastní datovou hodnotu.
- Smyslem je umožnit souběhy a agregace (např. více současně probíhajících „instancí“ stejného kroku) a soupeření o zdroje.

Pro jednoduchost předpokládáme, že **vstupy a výstupy jsou pouze typu `String`**.  
Interní proměnné mohou být libovolného typu, který dovolí inskripční jazyk (standardně C/C++).

Umístění nějaké hodnoty do některého vstupu (uživatelskou interakcí nebo předáním zprávy z jiného systému) je **vstupní událost**. Hodnota uložená na vstupu (tzv. poslední známá hodnota příslušného vstupu) je vždy k dispozici pro čtení a je použitelná ve strážích přechodů i v akcích přechodů.

Výstupní události se generují odesláním nějaké hodnoty do některého výstupu v rámci akce přechodu (případně akce místa, viz dále).

---

# 2. Prvky sítě

## 2.1 Místa (places / kroky)

Místa obsahují:

- **Identifikátor** (jméno).
- **Počáteční počet tokenů** (`integer >= 0`).
- **Volitelnou akci místa** (*place action*), která se provede:
  - při „vstupu tokenu“ do místa (tj. při každém přidání tokenu), nebo
  - alternativně při změně počtu tokenů.

Toto musí být v zadání **přesně definováno a dodrženo implementací**.  
**Doporučeno:** akce se provádí pro každý přidaný token.

## 2.2 Přechody (transitions)

Přechody obsahují:

- **Identifikátor** (jméno).
- **Vstupní hrany** z míst do přechodu s vahou `w >= 1`  
  (kolik tokenů se spotřebuje z daného místa).
- **Výstupní hrany** z přechodu do míst s vahou `w >= 1`  
  (kolik tokenů se vyrobí v daném místě).
- **Podmínku odpálení** (*enabling condition*) složenou ze 3 částí, přičemž každá může chybět:

```text
input_event_name [ bool_expr_in_C_C++ ] @ delay_in_ms