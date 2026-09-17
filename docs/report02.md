# Priebežná správa k druhému bloku projektu

## Vypracoval: Samuel Malec

Nasledujúci dokument reprezentuje priebežnú správu k druhému bloku projektu: Vývoj prekladačov pre jazyk Cthulhu (MUNI/33/0029/2026).

Zdrojový kód spolu s testami je verejne dostupný na https://github.com/samuel-malec/dagon pod tagom s názvom "blok-2".

## Postup práce

Plynule som nadviazal na funkcionalitu implementovanú v prvom bloku projektu a zameral som sa na rozšírenie funkcionality
`ct2qjs` - prekladača z Cthulhu do QuickJS bajtkódu - o tok riadenia.
Po úvodnej rešerši som navrhol reprezentáciu toku riadenia v jazyku Cthulhu a vytvoril sadu
testovacích, resp. ukážkových programov, ktoré zvolený návrh demonštrujú. Tieto testovacie programy
sa opäť nachádzajú v `test/ct2qjs/demo`.
Po zavedení tejto sady programov som upravil syntaktický analyzátor tak, aby bol schopný tieto programy akceptovať.
V ďalšom kroku som implementoval generáciu kódu pre if-príkazy, ktorú som odtestoval na sade testovacích programov.
Následne som implementoval generáciu kódu pre cykly. Na testovanie som opäť využil ručne napísané Cthulhu programy,
ktoré som preložil do QuickJS bajtkódu a spustil.

Keďže testovanie novej funkcionality vyžadovalo ručné písanie Cthulhu programov, začínalo to spomaľovať vývoj projektu.
Toto bol jeden z hlavných dôvodov, prečo som sa rozhodol nad rámec zadania projektu implementovať `js2ct` - prekladač z podmnožiny
JavaScriptu do vyvíjaného JavaScript dialektu pre Cthulhu. Zvolená podmnožina JavaScriptu zodpovedá funkcionalite,
ktorá je popísaná v zadaní projektu, a teda konkrétne som sa zameral na: číselné, pravdivostné a reťazcové literály,
aritmetické, logické a binárne operácie, premenné s `let` modifikátorom, if-príkazy a while a do-while cykly.
Pre túto podmnožinu jazyka JavaScript som napísal syntaktický analyzátor, ktorý sa nachádza v `src/js2ct/frontend`.
Nasledovala implementácia generácie Cthulhu kódu z JavaScript programov, ktorá sa nachádza v `src/js2ct/lin` a `src/js2ct/ct`.

## Testovanie

- rozšírenie `ct2qjs` o tok riadenia som pokryl doplnením ukážkových Cthulhu programov v `test/ct2qjs/demo` (13 programov
  celkovo, vrátane if-príkazov a cyklov), ktoré som skompiloval a spustil,
- syntaktický analyzátor `js2ct` je pokrytý 26 testami v `test/js2ct/parser` a 9 testami v `test/js2ct/lexer`,
- prevod HIR programu do lineárnej reprezentácie (`hir2linear`) je pokrytý 8 testami v `test/js2ct/lower`,
- pre celý reťazec `js2ct` → `ct2qjs` som zostavil 28 end-to-end JavaScript programov v `test/js2ct/e2e`. Tieto
  programy som preložil pomocou `js2ct` do Cthulhu a následne pomocou `ct2qjs` do QuickJS bajtkódu. Výsledný
  QuickJS bajtkód som spustil a pozoroval správanie, ktoré bolo vo všetkých prípadoch totožné s očakávaným.
