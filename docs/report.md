# Priebežná správa k projektu

## Vypracoval: Samuel Malec

Pred samotnou implementáciou, sme začali s rešeršou QuickJS.
Konkrétne sme sa zamerali na inštrukčnú sadu, jednotlivé časti a formát bajtkódu,
načítanie a spúštanie bajtkódu.

Po úvodnej rešerši sme naimplementovali jednoduchý C++ program, ktorého úlohou je načítať preložený QuickJS modul a spustiť ho.
Tento prograbm nám umožňuje spúštanie QuickJS modulov, a tým pádom je kľúčovou častou testovania projektu.

Zadefinovali sme textový formát QuickJS bajtkódu, na ktorý sa dá pozerať ako na zjednodušený assembly kód.
Tento formát nám umožňuje jednoduchšie testovanie a debugovanie QuickJS bajtkódu, ktorý je výsledkom našeho prekladača.
Aby sme sa lepšie zoznámili s QuickJS a odtestovali náš preklad z textového do binárneho formátu QuickJS,
napísali sme sadu ručne napísaných programov, ktoré sme spúštali pomocou vyššie spomenutého C++ programu.
[Implementácia a relevantné testovacie súbory sa nachádzajú v src/qasm, resp. test/qasm]

Po odtestovaní prekladu z textového do binárneho formátu QuickJS, sme sa pustili do návrhu kľučových prvkov Cthulhu,
ktoré budeme používať na reprezentáciu JavaScript programov. Vytvorili sme sadu jednoduchých programov v JavaScripte,
ktoré demonštrujú funkcionalitu, ktorú chceme dosiahnuť v našom projekte.
Pre každý z týchto ukážkových programov sme ručne napísali očakávanú Cthulhu medzireprezentáciu, ktorú by sme chceli
dostať ako výstup nášho prekladača.

Keď sme mali navrhnutý základný dialekt pre JavaScript a napísanú sadu testovacích programov, pustili sme sa do samotnej
implementácie prekladača z Cthulhu do QuickJS,
tento prekladač sme nazvali `ct2qjs`.

Najprv sme implementovali syntaktický analyzátor Cthulhu programov.
Pre tento účel sme si vytvorili potrebné štruktúry a využili sme techniku rekurzívneho zostupu.
Po parsovaní sme sa pustili do jednoduchej sémantickej analýzy Cthulhu programov - najmä
kontrolujeme, či všetky použité štruktúry a signatúry existujú, (treba ešte spraviť analýzu lineárneho kódu, a možno aj
sofistikovanejšiu typovú analýzu ).

# TODO: toto tu je jazykovo skomolené, potrebovali by sme lepšiu formuláciu
Na úvod sme si zadefinovali štruktúry, ktoré nám preložia celočíselné a boolean literály.
Začali sme prekladom jednoduchých aritmetických výrazov. Následne sme prešli na podporu pre lokálne premenné,
pomocou `let` bindingu.

## ct2qjs

V nasledujúcich podkapitolách popisujeme jednotlivé jazykové konštrukcie, ktoré `ct2qjs` v súčasnosti podporuje.

### Aritmetika

Implementovali sme základné aritmetické operácie: sčítanie (`add`), odčítanie (`sub`), násobenie (`mul`), delenie (`div`)
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

If-vetvy prekladáme s rozlíšením medzi tzv. tail a non-tail pozíciou. Pokiaľ obe vetvy končia priamym `return`om
(tail pozícia), vieme vygenerovať jednoduchší kód. Pokiaľ za if-vetvou nasleduje ďalší kód (non-tail pozícia), museli
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

## js2ct

Nad rámec oficiálneho zadania projektu, sme sa rozhodli implementovať prekladač
z podmnožiny jazyka JavaScript do Cthulhu, ktorý sme nazvali `js2ct`.
Tento prekladač nám umožní mať end-to-end pipeline, vďaka ktorej budeme môcť porovnávať QuickJS bajtkód vyprodukovaný
priamo prekladačom qjsc, a QuickJS bajtkódom, ktorý bol analyzovaný pomocou Cthulhu.
Toto nám dá dôležitý nástroj, vďaka ktorému budeme môcť porovnávať efekt analýz vykonaných v Cthulhu.

Tento projekt znovu začal obdobne, najprv sme začali s jednoduchým prekladom aritmetických výrazov, potom sme pridali
premenné, if príkazy, while-príkazy, funkcie a volania funkcií,
nakoniec sme začali implementovať jednoduchú verziu objektov a polí.
# TODO: chceme sa o tom rozpisovať aj napriek tomu, že to nie je oficialnou častou zadania ? 

Github repozitár je tu: https://github.com/samuel-malec/qthu