# Priebežná správa k projektu

## Vypracoval: Samuel Malec

Nasledujúci dokument reprezentuje priebežnú správu projektu: Vývoj prekladačov pre jazyk Cthulhu (MUNI/33/0029/2026).

Zdrojový kód spolu s testami je verejne dostupný na https://github.com/samuel-malec/dagon.

Ďalším krokom bol návrh a implementácia potrebných štruktúr na
reprezentáciu JavaScript programov v jazyku Cthulhu.
Implementovali sme interné štruktúry a signatúry jazyka Cthulhu,
ako aj C++ štruktúry na reprezentáciu Cthulhu programov.
a ich implementácia sa nachádza v src/cthu_core/.
Po tomto kroku sme vytvorili testovaciu sadu programov v Cthulhu,
ktorej cieľom je demonštrovať funkcionalitu požadovanú v zadaní
projektu. Táto sada programov sa nachádza v test/ct2qjs/demo.

Následne sme začali implementovať prekladač jazyka Cthulhu do QuickJS,
ktorého zdrojový kód sa nachádza v src/ct2qjs.
Začali sme implementáciou syntaktického analyzátoru Cthulhu programov.
Na tento účel sme si vytvorili potrebné štruktúry a k syntaktickej analýze sme využili techniku rekurzívneho zostupu.
Testy implementácie syntaktickej analýzy sa nachádzajú v test/ct2qjs/parser/
Po syntaktickej analýze sme implementovali sémantickú analýzu Cthulhu programov (treba dorobiť typechecker a zaručiť
lineárne typy...)
Ďalej sme implementovali generáciu QuickJS bajtkódu.
Využili sme už vyššie spomenuté rozhranie pre generáciu textového formátu QuickJS bajtkódu a
pre každú Cthulhu inštrukciu sme vygenerovali korešpondujúci QuickJS bajtkód.
Pre implementovanú funkcionalitu sme vytvorili sadu testov, ktorá sa nachádza v test/ct2qjs.
