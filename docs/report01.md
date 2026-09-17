# Priebežná správa k projektu

## Vypracoval: Samuel Malec

Nasledujúci dokument reprezentuje priebežnú správu projektu: Vývoj prekladačov pre jazyk Cthulhu (MUNI/33/0029/2026).

Zdrojový kód spolu s testami je verejne dostupný na https://github.com/samuel-malec/dagon.

Našu prácu na projekte sme začali rešeršou QuickJS.
Konkrétne sme sa zamerali na pochopenie inštrukčnej sady,
štruktúry jednotlivých častí QuickJS bajtkódu.
Po úvodnej rešerši sme implementovali jednoduchý program,
ktorého úlohou je načítať preložený QuickJS modul a spustiť ho.
Tento program sa nachádza v src/bcrun.c, a tvorí kľúčovú časť testovania celého projektu.
Implementovaný program sme odtestovali spustením QuickJS modulov,
ktoré sme získali prekladom ukážkových JavaScript programov.

V dalšom kroku sme zadefinovali textový formát QuickJS bajtkódu,
ktorého implementácia sa nachádza v src/asm.
Tento formát nám umožňuje jednoduchšie ladanie výsledného QuickJS bajtkódu,
ako aj jednoduchšiu prácu s bajtkódom, keďže
textový formát zakrýva mnoho implementačných detailov QuickJS modulu.
Po návrhu potrebných štruktúr sme implementovali rozhranie
pre budovanie textového QuickJS bajtkódu.
Následne sme implementovali preklad z textového formátu bajtkódu do QuickJS modulu.
Tento preklad sme odtestovali na súbore ukážkových programov, nachádzajúcich sa v test/asm.
