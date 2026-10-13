# Projektanweisungen für Claude

## Projektüberblick
- JmpShell ("JumpShell"): eine eigene interaktive Shell mit Terminal im Raw Mode.
- Sprache: C, gebaut mit `gcc` (`-Wall -Wextra`) über das `Makefile`.
- `src/main.c`: Hauptschleife, Prompt und Built-ins (`cd`, `subf`, `exit`).
- `src/jterminal.c`: Raw Mode, Tastatureingabe, Zeilen-Parsing.
- `src/dir.c`: aktuelles Verzeichnis und Unterordner auslesen.
- `include/jmpShell.h`: gemeinsamer Header (Structs, Prototypen, Farbcodes).
- Bauen: `make` (Ausgabe `build/main`), Starten: `make run`, Aufräumen: `make clean`.
- Tests gibt es bisher keine.

## Git-Workflow
- Du arbeitest ausschließlich in deinem eigenen Worktree/Branch.
- Mach kleine, in sich abgeschlossene Commits: eine logische Änderung pro Commit.
- Commit-Messages: erste Zeile kurze Zusammenfassung (max. ~60 Zeichen), danach kurz das Warum.
- Formatierung oder Refactoring nie mit inhaltlichen Änderungen im selben Commit mischen.
- Niemals pushen, mergen oder rebasen. Das Integrieren übernehme ich selbst per Cherry-Pick.

## Arbeitsweise
- Lies zuerst den relevanten Code und schlage einen Plan vor, bevor du Änderungen machst.
- Halte dich an den bestehenden Code-Stil und die vorhandene Struktur.
- Keine neuen Dependencies ohne Rückfrage.
- Ändere nur, was für die Aufgabe nötig ist. Kein ungefragtes Aufräumen.
- Wenn etwas unklar ist, frag nach statt zu raten.
