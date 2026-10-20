#ifndef _BLOCKED_CALLS_H_
#define _BLOCKED_CALLS_H_

// Lokale Sperrliste (Rufzeichen ohne Lizenz).
//
// Frames, deren QUELLE ein gesperrtes Rufzeichen ist, ODER deren Pfad ein
// gesperrtes Rufzeichen als Relais enthaelt, werden beim Empfang verworfen:
// keine Anzeige, kein Upload zum Server, kein Relay ins Mesh. Auf einem
// Gateway werden solche Frames ausserdem nicht vom Server auf LoRa gesendet.
//
// Eintrag OHNE SSID ("NF0JN")     sperrt alle SSIDs dieses Rufzeichens.
// Eintrag MIT SSID ("NF0JN-12")   sperrt genau dieses Rufzeichen.
// Vergleich ohne Beachtung von Gross-/Kleinschreibung.
//
// Die Liste ist zur Laufzeit aenderbar (--blockcall, Web-GUI "Setup" ->
// "Blocked callsigns") und wird dauerhaft gespeichert: ESP32 im NVS
// ("Credentials"/"block_list"), nRF52 in der Datei /blocklist.cfg.
// Solange nie etwas gespeichert wurde, gilt BLOCKLIST_DEFAULT.

#include <stddef.h>
#include <stdint.h>

#define BLOCKLIST_MAX       16   // Eintraege
#define BLOCKLIST_CALL_MAX  13   // je Eintrag inkl. NUL ("ABCDEFGHI-99")
#define BLOCKLIST_CSV_MAX   (BLOCKLIST_MAX * BLOCKLIST_CALL_MAX)
#define BLOCKLIST_DEFAULT   "NF0JN"

// Laden beim Start (nach dem Flash-Init) und Speichern.
void blockListLoad(void);
void blockListSave(void);

// Setzt die Liste aus "CALL1,CALL2,..." ("" oder "none" = leer). Prueft und
// normalisiert (Grossbuchstaben, ohne Leerzeichen). false = ungueltig, die
// alte Liste bleibt dann unveraendert. Speichert NICHT.
bool blockListSet(const char *csv);

// Aktuelle Liste als "CALL1,CALL2" ("" = leer).
const char *blockListCsv(void);

// true, wenn das Rufzeichen selbst gesperrt ist.
bool isBlockedCall(const char *call);

// true, wenn irgendein Element eines kommagetrennten Pfads gesperrt ist
// ("SRC,RELAY1,RELAY2", Relais ggf. mit '*').
bool isBlockedInPath(const char *path);

// Gesamtentscheidung fuer einen empfangenen Frame.
inline bool isBlockedFrame(const char *source_call, const char *path)
{
    return isBlockedCall(source_call) || isBlockedInPath(path);
}

#endif // _BLOCKED_CALLS_H_
