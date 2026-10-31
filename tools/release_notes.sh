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
## MeshCom-Firmware $TAG (DM3KS Test)

> [!WARNING]
> **Nur fuer Testzwecke von DM3KS.** Dieser Fork haelt die eigenen Knoten ueber den Auto-Update-Kanal an diesem Repo, damit hier Aenderungen ausprobiert werden koennen, bevor sie (vielleicht) bei DK5EN landen.
>
> **Bitte nicht verwenden**, sondern die offizielle Firmware von [icssw-org](https://github.com/icssw-org/MeshCom-Firmware/releases) bzw. den Build von [DK5EN](https://github.com/DK5EN/MeshCom-Firmware/releases). Dies ist **kein** offizielles Release.

### What this release is

DK5EN \`$BASE\` mit einer einzigen Aenderung (siehe unten). Inhalt und Neuerungen des Basis-Release: siehe [DK5EN $BASE]($D). Die **Sperrliste** selbst (Web-GUI, \`--blockcall\`, Fernwartung) ist seit \`v4.40a.10.23\` Teil der DK5EN-Firmware.

### Unterschiede zum DK5EN-Build $BASE

1. **Auto-Update-Kanal dev** (\`--updchan dev\`, Web-GUI "notify (dev, DM3KS releases)") zeigt auf **diesen Fork** statt auf DK5EN und ist **voreingestellt** (auch nach \`--reset\` oder Neu-Flashen mit Loeschen). Der Kanal **prod** zeigt weiterhin auf die offizielle Firmware von icssw-org.
2. **Nur ausgewaehlte Boards** gebaut (siehe Tabelle), nicht alle wie bei DK5EN.

Alles andere (Funktionen, Einstellungen, Protokoll) entspricht DK5EN \`$BASE\`. Die Einstellungen und eine gespeicherte Sperrliste bleiben beim Update erhalten.

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
