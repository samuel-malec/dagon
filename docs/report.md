# Priebežná správa k projektu

BLOKY 1-2-3

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

[ct2qjs]
Ďalším krokom bol návrh a implementácia potrebných štruktúr pre reprezentáciu JavaScript programov v jazyku Cthulhu.
Implementované boli interné štruktúry a signatúry jazyka Cthulhu,
ako aj C++ štruktúry na reprezentáciu samotných Cthulhu programov,
ktoré sa nachádzajú v src/cthu_core/
Po tomto kroku sme vytvorili sadu programov v Cthulhu, ktorej cieľom je demonštrovať funkcionalitu požadovanú v zadaní
projektu. Táto sada programov sa nachádza v test/ct2qjs/demo

Následne sme začali implementovať prekladač jazyka Cthulhu do QuickJS,
ktorého zdrojový kód sa nachádza v src/ct2qjs.
Začali sme implementáciou syntaktického analyzátoru Cthulhu programov.
Na tento účel sme si vytvorili potrebné štruktúry a k syntaktickej analýze sme využili techniku rekurzívneho zostupu.
Testy implementácie syntaktickej analýzy sa nachádzajú v test/ct2qjs/parser/
Po syntaktickej analýze sme implementovali sémantickú analýzu Cthulhu programov (ok nie tak úplne, ale ok, treba dorobiť
typechecker a zaručiť lineárne typy...)
Ďalej sme implementovali generáciu QuickJS bajtkódu.
Využili sme už vyššie spomenuté rozhranie pre generáciu textového formátu QuickJS bajtkódu a
pre každú Cthulhu inštrukciu sme vygenerovali korešpondujúci QuickJS bajtkód.
Pre implementovanú funkcionalitu sme vytvorili sadu testov, ktorá sa nachádza v test/ct2qjs.

[js2ct]
TODO: chceme toto vôbec písať, keď to nie je v zadaní ?

Keďže do tejto doby sa testovanie `ct2qjs` spoliehalo na ručne napísaný QuickJS bajtkód, bolo náročné vytvárať a
overovať nové testy.
Taktiež, dlhodobým cieľom projektu je využitie Cthulhu na statickú analýzu JavaScript programov.
Tieto dôvody nás viedli k implementácií funkcionality nad rámec zadania projektu, a to konkrétne k implementácií
prekladača podmnožiny JavaScript-u do Cthulhu.

Implementovali sme:
- Číselné, boolovské, reťazcové literály
- Základné aritmetické, porovnávacie, logické, bitové operácie
- Lokálne premenné (zatiaľ iba let bindingy)
- If príkazy
- Cykly
- Funkcie
- Objekty, polia, reťazce 

Taktiež sme doplnili aj jednoduchú podporu pre objekty a polia: vytvorenie prázdneho
objektu/poľa čítanie/zápis vlastností, a to pomocou bodkovej notácie, 
ako aj pomocou indexovania. Neskôr sme doplnili aj objektové a array (nájsť slovenské slovo ig) literály s hodnotami
zadanými priamo pri vytvorení. 
