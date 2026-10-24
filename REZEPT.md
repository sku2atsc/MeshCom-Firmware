# MeshCom-Firmware mit Sperrliste (DM3KS)

> **Nur fuer den Einsatz im Umkreis von DB0SD**, wo derzeit eine Station ohne
> Amateurfunklizenz ins MeshCom-Netz sendet. Alle anderen verwenden bitte die
> offizielle Firmware von [icssw-org](https://github.com/icssw-org/MeshCom-Firmware/releases)
> bzw. den Build von [DK5EN](https://github.com/DK5EN/MeshCom-Firmware/releases).

Dieser Branch enthaelt **keinen Firmware-Code**, sondern nur das Rezept:

| Datei | Zweck |
|---|---|
| `patches/` | Aenderungen gegenueber DK5EN (Auto-Update-Kanal, Sperrliste) |
| `boards.txt` | welche Boards gebaut werden |
| `.github/workflows/build.yml` | holt DK5EN, wendet die Patches an, baut parallel, veroeffentlicht |
| `tools/collect_assets.sh` | benennt die Build-Ergebnisse wie bei DK5EN |
| `tools/release_notes.sh` | Release-Text |

## Uebergabe-Modus (aktiv)

Solange die Datei `UEBERGABE` existiert, wird **nicht** gebaut: der Workflow
kopiert jedes neue DK5EN-Release unveraendert hierher. Knoten, die noch diesen
Fork als Update-Quelle (`--updchan dev`) haben, installieren es und zeigen danach
wieder auf DK5EN. `UEBERGABE` loeschen = zurueck zum Bauen mit den Patches.

## Ablauf (Normalbetrieb)

1. **Taeglich** prueft der Workflow das neueste Release von DK5EN.
2. Gibt es hier noch kein Release mit diesem Tag, wird DK5EN ausgecheckt, die
   Patches werden angewendet und alle Boards aus `boards.txt` **parallel** gebaut.
3. Das Ergebnis erscheint als Release **Latest** in diesem Repo, mit demselben
   Tag wie bei DK5EN. Knoten mit `--updchan dev` bekommen den Update-Hinweis.

Laesst sich ein Patch auf eine neue DK5EN-Version nicht anwenden, schlaegt der
Lauf fehl (GitHub schickt eine Mail) und es wird **kein** Release erstellt.
Dann muss der Patch angepasst werden.

## Von Hand

Actions -> **Firmware mit Sperrliste** -> **Run workflow**:

- leer lassen: neuestes DK5EN-Release bauen (falls noch nicht vorhanden)
- `base`: eine bestimmte DK5EN-Version bauen
- `tag`: eigener Release-Tag, z.B. um ein Update zu erzwingen (muss neuer sein
  als die Version auf den Knoten, Format `v4.40a.MM.DD`)
- `force`: auch bauen, wenn das Release schon existiert

## Boards aendern

`boards.txt` bearbeiten und committen -- der Push startet sofort einen Neubau.
