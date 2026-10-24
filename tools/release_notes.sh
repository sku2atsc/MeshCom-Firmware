#!/usr/bin/env bash
# Erzeugt den Release-Text (Markdown) auf stdout.
#
#   release_notes.sh <release-tag> <dk5en-basis-tag> <repo> <rezept-commit> <asset-verzeichnis>
#
# Die Download-Tabelle wird aus den Dateinamen im Asset-Verzeichnis gebaut.
set -euo pipefail

TAG="$1"
BASE="$2"
REPO="$3"
SHA="$4"
DIR="$5"
U="https://github.com/$REPO/releases/download/$TAG"
D="https://github.com/DK5EN/MeshCom-Firmware/releases/tag/$BASE"

if [ "${MIRROR:-0}" = 1 ]; then
cat <<EOF
## MeshCom-Firmware $TAG -- Uebergabe an DK5EN

> [!IMPORTANT]
> **Dies ist das unveraenderte Release [DK5EN $BASE]($D)**, nur hierher kopiert. Die Sperrliste ist seit \`$BASE\` fester Bestandteil der Firmware von DK5EN, dieser Fork wird nicht mehr gebraucht.

### What this release is

DK5EN \`$BASE\`, Byte fuer Byte. Inhalt und Neuerungen: siehe [DK5EN $BASE]($D).

### Fuer Knoten, die bisher diesen Fork verwendet haben

- Nach der Installation zeigt der Auto-Update-Kanal **dev** (\`--updchan dev\`) wieder auf **DK5EN**. Kuenftige Updates kommen direkt von dort.
- Eine **gespeicherte Sperrliste wird uebernommen** (beim ersten Start einmalig migriert).
- **Wichtig:** Die Voreinstellung aus dem Fork gilt in der DK5EN-Firmware nicht. Wurde die Liste nie gespeichert, ist sie nach dem Update **leer**. Bitte nach dem Update pruefen und ggf. neu setzen (Web-GUI Setup, \`--blockcall <CALL>\` oder per Fernwartung).

### Firmware zum Download

| Board | Firmware | Auto-Update |
|---|---|---|
EOF
else
cat <<EOF
## MeshCom-Firmware $TAG mit Sperrliste (DM3KS)

> [!WARNING]
> **Dieser Build existiert nur aus einem Grund:** Im Umkreis von **DB0SD** sendet derzeit eine Station ohne Amateurfunklizenz (Pirat) ins MeshCom-Netz. Diese Firmware verwirft deren Aussendungen, damit sie nicht weiter durchs Mesh transportiert werden.
>
> **Wer nicht direkt davon betroffen ist, verwendet bitte die offizielle Firmware:** [icssw-org/MeshCom-Firmware](https://github.com/icssw-org/MeshCom-Firmware/releases) bzw. den Build von [DK5EN](https://github.com/DK5EN/MeshCom-Firmware/releases). Dies ist **kein** offizielles Release.

### What this release is

DK5EN \`$BASE\` plus Sperrliste. Inhalt und Neuerungen des Basis-Release: siehe [DK5EN $BASE]($D).

### Unterschiede zum DK5EN-Build $BASE

1. **Sperrliste (neu).** Frames, deren Absender **oder** ein Relais im Pfad auf der Liste steht, werden beim Empfang verworfen: keine Anzeige, keine Weitergabe an App/BLE, kein Upload zum Server, **kein Weiterleiten im Mesh**. Auf einem Gateway werden solche Frames ausserdem nicht vom Server auf LoRa gesendet. Konsole: \`[RX];drop;blocked;...\`.
2. **Voreingestellt** ist das betroffene Rufzeichen (alle SSIDs). Ein Eintrag ohne SSID sperrt alle SSIDs, mit SSID (z.B. \`XX1ABC-12\`) nur diese. Bis zu 16 Eintraege.
3. **Web-GUI:** neue Karte Setup -> **Blocked callsigns** zum Ansehen und Bearbeiten der Liste.
4. **Konsole:** neuer Befehl \`--blockcall CALL1,CALL2\`, \`--blockcall none\` (leeren), \`--blockcall\` (anzeigen). Die Liste wird dauerhaft gespeichert (ESP32: NVS, nRF52: Datei) und ueberlebt Neustart und Update.
5. **Auto-Update-Kanal dev** (\`--updchan dev\`, Web-GUI "notify (dev, DM3KS releases)") zeigt auf **diesen Fork** statt auf DK5EN. Der Kanal **prod** zeigt weiterhin auf die offizielle Firmware von icssw-org (dann ohne Sperrliste).
6. **Nur ausgewaehlte Boards** gebaut (siehe Tabelle), nicht alle wie bei DK5EN.

Alles andere (Funktionen, Einstellungen, Protokoll) entspricht DK5EN \`$BASE\`. Die Einstellungen bleiben beim Wechsel erhalten.

### Firmware zum Download

| Board | Firmware | Auto-Update |
|---|---|---|
EOF
fi

shopt -s nullglob
for f in "$DIR"/*.bin; do
  n="$(basename "$f")"
  case "$n" in bootloader*.bin|partitions.bin|otadata.bin|safeboot*.bin) continue ;; esac
  b="${n%.bin}"
  a=""
  [ -e "$DIR/$b.autoupdate" ] && a="[$b.autoupdate]($U/$b.autoupdate)"
  echo "| $b | [$n]($U/$n) | $a |"
done
for f in "$DIR"/*.uf2; do
  n="$(basename "$f")"
  b="${n%.uf2}"
  z=""
  [ -e "$DIR/$b.zip" ] && z=", [$b.zip]($U/$b.zip)"
  echo "| $b | [$n]($U/$n)$z | |"
done

if [ "${MIRROR:-0}" = 1 ]; then
cat <<EOF

Komplett-Flashen: die uebrigen Dateien unter **Assets** bzw. der [Web-Flasher von DK5EN](https://dk5en.github.io/MeshCom-Firmware/flash/).
EOF
exit 0
fi

cat <<EOF

Komplett-Flashen (Safeboot, Bootloader, Partitionen, otadata) wie bei DK5EN: die uebrigen Dateien unter **Assets**.

Gebaut von GitHub Actions: DK5EN \`$BASE\` + Patches aus [\`patches/\`](https://github.com/$REPO/tree/$SHA/patches) (Rezept-Commit \`${SHA:0:8}\`).
EOF
