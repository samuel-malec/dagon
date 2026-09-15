# Priebežná správa k projektu

BLOKY 1-2-3

## Vypracoval: Samuel Malec

Nasledujúci dokument reprezentuje priebežnú správu projektu: Vývoj prekladačov pre jazyk Cthulhu (MUNI/33/0029/2026).

Zdrojový kód spolu s testami je verejne dostupný na https://github.com/samuel-malec/dagon.

(TODO: v zadaní projektu sa požaduje, nech je kód dostupný na gitlabe labu, dohodnúť sa s Uhlíkom Vladimírom...)

[bcrun]
Našu prácu na projekte sme začali rešeršou QuickJS.
Konkrétne sme sa zamerali na pochopenie inštrukčnej sady,
štruktúry jednotlivých častí QuickJS bajtkódu.
Po úvodnej rešerši sme implementovali jednoduchý program,
ktorého úlohou je načítať preložený QuickJS modul a spustiť ho.
Tento program sa nachádza v src/bcrun.c, a tvorí kľúčovú časť testovania celého projektu.
Implementovaný program sme odtestovali spustením QuickJS modulov,
ktoré sme získali prekladom ukážkových JavaScript
programov.

[asm]
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
projektu. Táto sada programov sa nachádza v test/ct2qjs/

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
Keďže do tejto doby sa testovanie `ct2qjs` spoliehalo na ručne napísaný QuickJS bajtkód, bolo náročné vytvárať a
overovať nové testy.
Taktiež, dlhodobým cieľom projektu je využitie Cthulhu na statickú analýzu JavaScript programov.
Tieto dôvody nás viedli k implementácií funkcionality nad rámec zadania projektu, a to konkrétne k implementácií
prekladača podmnožiny JavaScript-u do Cthulhu.

### Aritmetika

Implementovali sme základné aritmetické operácie: sčítanie (`add`), odčítanie (`sub`), násobenie (`mul`), delenie
(`div`)
a zvyšok po delení (`rem`). Ďalej sme implementovali porovnávacie operácie (`eq?`, `ne?`, `lt?`, `le?`, `gt?`, `ge?`)
a bitové operácie (`band`, `bor`, `bxor`, `bnot`, `shl`, `shr`).

Osobitnú pozornosť sme museli venovať unárnemu mínusu. Pôvodne sme sa ho pokúšali prekladať ako volanie binárnej
operácie `sub` len s jedným argumentom, čo za behu programu viedlo k poškodeniu indexovania lokálnych premenných
(operácia `sub` má v `builtins.ct`/`codegen.cpp` pevne danú aritu 2, a `.ct` reader arity volaní nekontroluje, takže
sa chybný preklad tíško "skompiloval" a zlyhal až pri spustení bajtkódu). Problém sme opravili pridaním samostatnej
operácie `neg` s aritou 1, ktorá využíva natívnu inštrukciu QuickJS bajtkódu `OP_neg`, rovnako ako to už predtým
fungovalo pre logickú negáciu `not` a bitovú negáciu `bnot`.

### Premenné

Lokálne premenné sú v Cthulhu reprezentované ako pomenované hodnoty (v assembly zápise `%N`, v ručne písaných
programoch symbolické mená), ktoré sa do QuickJS bajtkódu prekladajú ako `get_loc`/`put_loc` inštrukcie nad lokálnymi
slotmi funkcie. Keďže Cthulhu vychádza z lineárneho typového systému, každá hodnota musí byť použitá práve raz - preto
máme k dispozícii operácie `dup` (rozdvojenie hodnoty pre viacnásobné použitie), `drop` (explicitné zahodenie
nepoužitej hodnoty, potrebné na splnenie linearity), `move` a `copy`.

### If-vetvy

If-vetvy prekladáme s rozlíšením medzi tzv. tail a non-tail pozíciou. Pokiaľ obe vetvy končia priamym `return`om (tail
pozícia), vieme vygenerovať jednoduchší kód. Pokiaľ za if-vetvou nasleduje ďalší kód (non-tail pozícia), museli
sme doriešiť, ako sa majú premenné zmenené v jednotlivých vetvách prejaviť aj mimo if-vetvy - každá vetva zabalí svoje
"živé" (ďalej používané) premenné do jedného poľa, ktoré sa po vykonaní príslušnej vetvy rozbalí naspäť do pôvodných
premenných v okolitom kóde. Osobitný prípad predstavujú vetvy, ktoré vždy skončia príkazom `return` - v tom prípade sa
balenie do poľa nepoužíva, keďže za takouto if-vetvou už nemôže nasledovať žiadny ďalší kód.

### Cykly

Implementovali sme `while`, `do-while` aj `for` cykly. Všetky tri sa v HIR/LIN reprezentácii prekladajú na spoločnú
štruktúru `loop_stmt` (podmienka + telo cyklu). `do-while` sa od `while` líši tým, že telo cyklu sa preloží (a teda aj
vykoná) raz ešte pred prvým vyhodnotením podmienky. `for` cyklus navyše obsahuje inicializačný príkaz vykonaný pred
cyklom a aktualizačný výraz pripojený na koniec tela cyklu.

### Funkcie

Podporujeme deklarácie funkcií s ľubovoľným počtom parametrov, ich volanie (vrátane rekurzívnych volaní) a návratové
hodnoty. Funkcie sú v Cthulhu reprezentované ako samostatné štruktúry, ktoré sa volajú pomocou mechanizmu
`call`/`join`/`opt` - to umožňuje aj podmienené volanie rôznych funkcií (napr. medzi bázovým a rekurzívnym prípadom pri
rekurzii).

### Objekty, polia a reťazce

Nad rámec pôvodného zadania sme doplnili aj jednoduchú podporu pre objekty a polia: vytvorenie prázdneho
objektu/poľa (`cons_obj`/`cons_arr`) a čítanie/zápis vlastností (`get`/`set`), a to tak pomocou bodkovej notácie
(`obj.x`), ako aj pomocou indexovania (`arr[i]`). Neskôr sme doplnili aj objektové a poľové literály s hodnotami
zadanými priamo pri vytvorení (napr. `{x: 1, y: 2}` alebo `[1, 2, 3]`). Podobne sme doplnili aj reťazcové literály
a ich porovnávanie.

### Testovanie ct2qjs

Okrem sady ručne napísaných `.ct` programov (`test/ct2qjs`), ktoré overujú správne fungovanie jednotlivých operácií
a ktoré sa spúšťajú a vyhodnocujú automatizovaným skriptom `test/run.sh`, sme doplnili aj samostatnú testovaciu sadu
pre lexer a parser (reader) samotného `.ct` formátu (`test/ct2qjs/lexer`, `test/ct2qjs/parser`, spúšťané skriptom
`test/run_ct2qjs_frontend.sh`) - obsahuje jednak vstupy, ktoré majú byť správne preložené, jednak zámerne chybné
vstupy, ktoré má prekladač čisto odmietnuť. Pri písaní tejto sady sme odhalili a opravili niekoľko reálnych chýb,
napríklad že `.ct` reader sa pri neukončenom (skrátenom) vstupe niekedy zacyklil namiesto toho, aby vstup korektne
odmietol s chybovou hláškou.

TODO: do we allow dupping references and is it allowed ? 
