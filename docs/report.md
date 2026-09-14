# Priebežná správa k projektu
## Vypracoval:  [Samuel Malec]

[Chceme vedieť, čo obsahuje QuickJS]
Na úvod sme si naštudovali potrebné informácie ohľadom QuickJS,
konkrétne sme sa zamerali inštrukčnú sada, jednotlivé časti a formát bajtkódu,
načítanie a spúštanie bajtkódu.

[Chceme vedieť spúštať QuickJS bajtkód ]
Po úvodnej rešerši sme naimplementovali jednoduchý C++ program, ktorý načíta QuickJS modul a spustí ho.
Tento program nám bude slúžiť na spúštanie nami prekladaných QuickJS modulov.

[Chceme mať ľudsky čitateľnú verziu QuickJS bajtkódu - navrhli sme ju, a preložili ju do QuickJS bajtkódu]
Aby sme sa detailnejšie mohli venovať generovaniu QuickJS assembleru, navrhli sme si vlastnú, textovú reprezentáciu, ktorá slúži na lepšie debugovanie....
Zadefinovali sme si vlastnú verziu assembleru, ktorá odopvedá textovým inštrukciám QuickJS bytekódu.
S použitím tejto reprezentácie, sme si napísali pár programov, ktoré sme následne preložili do quickjs bytekódu.

[Návrh špecifických Cthu štruktúr pre JavaScript]
Ďalej sme navrhli kľúčové prvky Cthulhu medzireprezentácie pre JavaScript - a to najmä potrebné štruktúry a signatúry.
Vytvorili sme sadu jednoduchých programov v JavaScripte, ktoré demonštrujú funkcionalitu, ktorú chceme dosiahnuť v našom projekte.
Pre každý z týchto ukážkových programov sme ručnš napísali očakávanú Cthulhu medzireprezentáciu, ktorú by sme chceli dostať z našeho prekladaču.
Keď sme mali navrhnutý základný dialekt pre JavaScript a napísanú sadu testovacích programov, pustili sme sa do samotnej implementácie prekladača z Cthulhu do QuickJs,
tento prekladač sme nazvali `ct2qjs`.

[ct2qjs, Parser Cthu programov]
[ct2qjs] Ako prvou časťou, ktorú sme implementovali, bol parser Cthulhu programov. Na parsovanie sme si vytvorili potrebné štruktúry a pomocou metódy rekurzívneho zostupu sme
implementovali samotný parser. Po parsovaní sme sa pustili do jednoduchej sémantickej analýzy cthulhu programov - najmä kontrolujeme, či všetky použité štruktúry a signatúry existujú, ( treba ešte spraviť analýzu lineárneho kódu, a možno aj sofistikovanejšiu typovú analýzu ).
Začali sme prekladom jednoduchých aritmetických výrazov. Následne sme prešli na
z
[Opísať implementovanú funkcionalitu v ct2qjs]

[js2ct]
Nad rámec oficiálneho zadania projektu, sme sa rozhodli implementovať prekladač 
z podmnožiny jazyka JavaScript do Cthuhlu, ktorý sme nazvali `js2ct`. 
Tento prekladač nám umožní mať end-to-end pipeline, vďaka ktorej budeme môcť porovnávať QuickJS bajtkód vyprodukovaný priamo prekladačom qjsc, a QuickJS bajtkódom, ktorý bol analyzovaný pomocou Cthulhu.
Toto nám dá dôležitý nástroj, vďaka ktorému budeme môcť porovnávať efekt analýz vykonaných v Cthulhu.

Tento projekt znovu začal odbobne, najprv sme začali s jednoduchým prekladom aritmetických výrazov, potom sme pridali premenné, if príkazy, while-príkazy, funkcie a volania funkcií,
nakoniec sme začali implementovať jednoduchú verziu objektov a polí.


Github repozitár je tu: https://github.com/samuel-malec/qthu