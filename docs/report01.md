# Priebežná správa k prvému bloku projektu

# Vypracoval: Samuel Malec

Nasledujúci dokument reprezentuje priebežnú správu k prvému bloku projektu: Vývoj prekladačov pre jazyk Cthulhu (MUNI/33/0029/2026).

Zdrojový kód spolu s testami je verejne dostupný na https://github.com/samuel-malec/dagon pod tagom s názvom "blok-1".

## Postup práce

Prácu na projekte som začal rešeršou QuickJS.
Konkrétne som sa zameral na pochopenie inštrukčnej sady a
štruktúry jednotlivých častí QuickJS bajtkódu.
Po úvodnej rešerši som implementoval jednoduchý program,
ktorého úlohou je načítať ľubovoľný QuickJS modul a spustiť ho.
Tento program sa nachádza v `src/bcrun.c` a tvorí kľúčovú časť testovania celého projektu.
Implementovaný program som otestoval na sade QuickJS modulov,
ktoré som získal prekladom vzorových JavaScript programov prostredníctvom qjsc prekladača.

V ďalšom kroku som zadefinoval textový formát QuickJS bajtkódu, ktorého implementácia sa nachádza v `src/asm`.
Tento formát umožňuje jednoduchšie ladanie výsledného QuickJS bajtkódu, ako aj jednoduchšiu prácu s bajtkódom, keďže
textový formát zakrýva mnoho implementačných detailov QuickJS modulu a dovoľuje ručne zapisovať QuickJS inštrukcie.
Po návrhu potrebných štruktúr som implementoval rozhranie pre budovanie textového QuickJS bajtkódu.
Následne som implementoval preklad z textového formátu bajtkódu do QuickJS modulu.
Túto implementáciu som otestoval na súbore ukážkových programov, nachádzajúcich sa v `test/qasm`.

Ďalším krokom bol návrh a implementácia potrebných štruktúr na
reprezentáciu JavaScript programov v jazyku Cthulhu.
Zameral som sa predovšetkým na reprezentáciu číselných a pravdivostných hodnôt,
a implementoval som potrebné štruktúry a signatúry jazyka Cthulhu,
ako aj C++ štruktúry na reprezentáciu Cthulhu programov.
Implementácia sa nachádza v `src/cthu_core/`.
Po tomto kroku som vytvoril testovaciu sadu programov v Cthulhu,
ktorej cieľom je demonštrovať funkcionalitu požadovanú v zadaní
prvého bloku projektu. Táto sada programov sa nachádza v `test/ct2qjs/demo`.

Následne som začal implementovať hlavný prínos projektu, a to menovite `ct2qjs` - prekladač jazyka Cthulhu do QuickJS,
ktorého zdrojový kód sa nachádza v `src/ct2qjs`.
Začal som implementáciou syntaktického analyzátora Cthulhu programov.
Na tento účel som vytvoril potrebné štruktúry a k syntaktickej analýze využil techniku rekurzívneho zostupu.
Po syntaktickej analýze som implementoval sémantickú analýzu Cthulhu programov - a to hlavne kontrolu použitia validných
operácií a kontrolu arity operácií. Po implementácii syntaktickej a sémantickej analýzy
som implementoval samotnú generáciu QuickJS bajtkódu. Využil som všetky vyššie spomenuté implementované komponenty a
pre každú Cthulhu inštrukciu som vygeneroval korešpondujúci QuickJS bajtkód.

## Testovanie

Každú z vyššie popísaných častí som priebežne pokrýval testami:

- `src/asm` (textový formát QuickJS bajtkódu) - 3 testovacie programy v `test/qasm`,
- `ct2qjs` syntaktická analýza - 9 testov v `test/ct2qjs/parser` (vrátane neplatných vstupov) a 8 testov v `test/ct2qjs/lexer`,
- ukážkové Cthulhu programy demonštrujúce funkcionalitu bloku 1 - 13 programov v `test/ct2qjs/demo`.

Všetky programy zo `test/ct2qjs/demo` som skompiloval prostredníctvom `ct2qjs` do QuickJS bajtkódu, spustil a manuálne
overil, že ich výstup zodpovedá očakávanému správaniu.
