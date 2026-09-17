# Priebežná správa k projektu

## Vypracoval: Samuel Malec

Nasledujúci dokument reprezentuje priebežnú správu projektu: Vývoj prekladačov pre jazyk Cthulhu (MUNI/33/0029/2026).

Zdrojový kód spolu s testami je verejne dostupný na https://github.com/samuel-malec/dagon.

Keďže do tejto doby sa testovanie `ct2qjs` spoliehalo na ručne napísaný QuickJS bajtkód, bolo náročné vytvárať a
overovať nové testy.
Taktiež, dlhodobým cieľom projektu je využitie Cthulhu na statickú analýzu JavaScript programov.
Tieto dôvody nás viedli k implementácií funkcionality nad rámec zadania projektu, a to konkrétne k implementácií
prekladača podmnožiny JavaScript-u do Cthulhu.

TODO: funkcie
Implementovali sme:

- Číselné, boolovské, reťazcové literály
- Základné aritmetické, porovnávacie, logické, bitové operácie
- Lokálne premenné (zatiaľ iba let bindingy)
- If príkazy
- Cykly
- Funkcie
- Objekty, polia, reťazce 
