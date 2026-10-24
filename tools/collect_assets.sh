#!/usr/bin/env bash
# Sammelt die Build-Ergebnisse EINES PlatformIO-Envs unter den Release-
# Dateinamen von DK5EN ein.
#
#   collect_assets.sh <env> <firmware-verzeichnis> <ausgabe-verzeichnis>
#
# ESP32-App:   <name>.bin + <name>.autoupdate (zlib, fuer das Auto-Update)
# nRF52:       <env>.zip (DFU) + <env>.uf2
# Safeboot:    safeboot.bin, bootloader.bin, partitions.bin, otadata.bin
#              bzw. safeboot-s3.bin, bootloader-s3.bin
set -euo pipefail

ENV="$1"
FW="$(cd "$2" && pwd)"
OUT="$3"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
B="$FW/.pio/build/$ENV"

need() { [ -f "$1" ] || { echo "::error::$ENV: Datei fehlt: ${1#$FW/}"; exit 1; }; }

case "$ENV" in
  esp32-safeboot)
    need "$B/safeboot.bin"; need "$B/bootloader.bin"; need "$B/partitions.bin"; need "$FW/otadata.bin"
    cp "$B/safeboot.bin" "$B/bootloader.bin" "$B/partitions.bin" "$OUT/"
    cp "$FW/otadata.bin" "$OUT/"
    ;;
  esp32-S3-safeboot)
    need "$B/safeboot-s3.bin"; need "$B/bootloader.bin"
    cp "$B/safeboot-s3.bin" "$OUT/"
    cp "$B/bootloader.bin" "$OUT/bootloader-s3.bin"
    ;;
  wiscore_rak4631|heltec_t114|t_echo)
    need "$B/firmware.zip"; need "$B/firmware.hex"
    cp "$B/firmware.zip" "$OUT/$ENV.zip"
    python3 "$(dirname "$0")/uf2conv.py" "$B/firmware.hex" -c -o "$OUT/$ENV.uf2" -f 0xADA52840
    ;;
  *)
    # Drei Envs heissen im Release anders (wie src/fw_update_assets.h, fwAssetBase()).
    case "$ENV" in
      LilyGo_T-Beam-1W)     NAME="T-Beam-1W" ;;
      LilyGo_T3_S3_V1_3)    NAME="T3_S3_V13" ;;
      LilyGo_T_Connect_Pro) NAME="t_connect_pro" ;;
      *)                    NAME="$ENV" ;;
    esac
    need "$B/firmware.bin"
    cp "$B/firmware.bin" "$OUT/$NAME.bin"
    # Auto-Update-Datei; --check entpackt sie wieder und vergleicht byte-genau.
    python3 "$FW/tools/make_autoupdate.py" --check --out-dir "$OUT" "$OUT/$NAME.bin"
    ;;
esac

ls -la "$OUT"
