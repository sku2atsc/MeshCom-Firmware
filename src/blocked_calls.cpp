// blocked_calls.cpp -- lokale Sperrliste, siehe blocked_calls.h.
#include "blocked_calls.h"

#include <string.h>
#include <stdio.h>

#if defined(ARDUINO)
#include <Arduino.h>
#include <configuration.h>
#endif

// ---------------------------------------------------------------------------
// Liste im RAM. Zwei Puffer: OnRxDone() liest im LoRa-Task, Web-GUI und
// Konsole schreiben im Loop-Task. Geschrieben wird immer in den gerade nicht
// aktiven Puffer, danach wird nur der Index umgeschaltet -- der Leser sieht
// also stets eine vollstaendige Liste.
// ---------------------------------------------------------------------------

struct BlockList
{
    char    call[BLOCKLIST_MAX][BLOCKLIST_CALL_MAX];
    uint8_t count;
    char    csv[BLOCKLIST_CSV_MAX];
};

static BlockList s_list[2];
static volatile uint8_t s_active = 0;
static bool s_initialised = false;

static char upperC(char c)
{
    return (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
}

static bool isAlnumC(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

// Eintrag pruefen: 1..9 Zeichen A-Z0-9, optional "-" und 1..2 Ziffern.
static bool entryValid(const char *e)
{
    size_t len = strlen(e);
    if (len == 0 || len >= BLOCKLIST_CALL_MAX)
        return false;
    const char *dash = strchr(e, '-');
    size_t base = dash ? (size_t)(dash - e) : len;
    if (base == 0 || base > 9)
        return false;
    for (size_t i = 0; i < base; i++)
        if (!isAlnumC(e[i]))
            return false;
    if (dash)
    {
        size_t sl = len - base - 1;
        if (sl < 1 || sl > 2)
            return false;
        for (size_t i = base + 1; i < len; i++)
            if (e[i] < '0' || e[i] > '9')
                return false;
    }
    return true;
}

// Parst csv in out. false bei ungueltigem Eintrag oder zu vielen Eintraegen.
static bool parseList(const char *csv, BlockList &out)
{
    memset(&out, 0, sizeof(out));
    if (csv == NULL)
        return true;

    // "none" (beliebige Schreibweise, ggf. mit Leerzeichen) = leere Liste
    {
        char tmp[8] = {0};
        size_t j = 0;
        for (const char *p = csv; *p && j < sizeof(tmp) - 1; p++)
            if (*p != ' ')
                tmp[j++] = upperC(*p);
        if (strcmp(tmp, "NONE") == 0 && strlen(csv) < 16)
            return true;
    }

    const char *p = csv;
    while (*p)
    {
        char tok[BLOCKLIST_CALL_MAX + 1];
        size_t tl = 0;
        bool tooLong = false;
        // fuehrende Leerzeichen ueberspringen
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            p++;
        while (*p && *p != ',' && *p != ';')
        {
            char c = *p++;
            if (tl < BLOCKLIST_CALL_MAX)
                tok[tl++] = upperC(c);
            else
                tooLong = true;
        }
        if (*p)
            p++; // Trenner
        // abschliessende Leerzeichen entfernen; Leerzeichen IM Rufzeichen
        // bleiben stehen und machen den Eintrag ungueltig
        while (tl > 0 && (tok[tl - 1] == ' ' || tok[tl - 1] == '\t' || tok[tl - 1] == '\r' || tok[tl - 1] == '\n'))
            tl--;
        tok[tl] = 0;
        if (tl == 0)
            continue; // leere Eintraege ("A,,B", Komma am Ende) ignorieren
        if (tooLong || !entryValid(tok))
            return false;

        // Doppelte Eintraege nur einmal uebernehmen
        bool dup = false;
        for (uint8_t i = 0; i < out.count; i++)
            if (strcmp(out.call[i], tok) == 0)
                dup = true;
        if (dup)
            continue;

        if (out.count >= BLOCKLIST_MAX)
            return false;
        memcpy(out.call[out.count], tok, tl + 1);
        out.count++;
    }

    for (uint8_t i = 0; i < out.count; i++)
    {
        if (i)
            strncat(out.csv, ",", sizeof(out.csv) - strlen(out.csv) - 1);
        strncat(out.csv, out.call[i], sizeof(out.csv) - strlen(out.csv) - 1);
    }
    return true;
}

static void ensureInit(void)
{
    if (s_initialised)
        return;
    parseList(BLOCKLIST_DEFAULT, s_list[0]);
    s_active = 0;
    s_initialised = true;
}

bool blockListSet(const char *csv)
{
    ensureInit();
    uint8_t next = (uint8_t)(s_active ^ 1);
    BlockList tmp;
    if (!parseList(csv, tmp))
        return false;
    s_list[next] = tmp;
    s_active = next;
    return true;
}

const char *blockListCsv(void)
{
    ensureInit();
    return s_list[s_active].csv;
}

// ---------------------------------------------------------------------------
// Abgleich
// ---------------------------------------------------------------------------

// tok[0..tlen) gegen einen Eintrag (bereits in Grossbuchstaben).
static bool tokenMatches(const char *tok, size_t tlen, const char *entry)
{
    while (tlen > 0 && (tok[tlen - 1] == '*' || tok[tlen - 1] == ' '))
        tlen--;
    while (tlen > 0 && tok[0] == ' ')
    {
        tok++;
        tlen--;
    }
    if (tlen == 0)
        return false;

    size_t elen = strlen(entry);
    bool entryHasSsid = strchr(entry, '-') != NULL;

    size_t tbase = 0;
    while (tbase < tlen && tok[tbase] != '-')
        tbase++;

    size_t cmplen = entryHasSsid ? tlen : tbase;
    if (cmplen != elen)
        return false;
    for (size_t i = 0; i < elen; i++)
        if (upperC(tok[i]) != entry[i])
            return false;
    return true;
}

static bool tokenBlocked(const char *tok, size_t tlen)
{
    ensureInit();
    const BlockList &l = s_list[s_active];
    for (uint8_t i = 0; i < l.count; i++)
        if (tokenMatches(tok, tlen, l.call[i]))
            return true;
    return false;
}

bool isBlockedCall(const char *call)
{
    if (call == NULL || call[0] == 0)
        return false;
    return tokenBlocked(call, strlen(call));
}

bool isBlockedInPath(const char *path)
{
    if (path == NULL)
        return false;
    const char *start = path;
    for (const char *p = path;; p++)
    {
        if (*p == ',' || *p == 0)
        {
            if (p > start && tokenBlocked(start, (size_t)(p - start)))
                return true;
            if (*p == 0)
                break;
            start = p + 1;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Speicherung
// ---------------------------------------------------------------------------

#if defined(ESP32)

#include <Preferences.h>

static Preferences s_block_prefs;

void blockListLoad(void)
{
    ensureInit();
    s_block_prefs.begin("Credentials", true);
    bool have = s_block_prefs.isKey("block_list");
    String v = have ? s_block_prefs.getString("block_list", "") : String("");
    s_block_prefs.end();

    if (have && !blockListSet(v.c_str()))
        blockListSet(BLOCKLIST_DEFAULT); // kaputter Eintrag: Standard
}

void blockListSave(void)
{
    s_block_prefs.begin("Credentials", false);
    s_block_prefs.putString("block_list", String(blockListCsv()));
    s_block_prefs.end();
}

#elif defined(BOARD_RAK4630) || defined(NRF52_SERIES)

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;

static const char kBlockFileName[] = "/blocklist.cfg";
static const char kBlockMagic[] = "BLK1:";

void blockListLoad(void)
{
    ensureInit();
    InternalFS.begin();
    File f(InternalFS);
    f.open(kBlockFileName, FILE_O_READ);
    if (!f)
        return; // nie gespeichert: Standard bleibt
    char buf[BLOCKLIST_CSV_MAX + 8];
    memset(buf, 0, sizeof(buf));
    f.read((uint8_t *)buf, sizeof(buf) - 1);
    f.close();
    size_t ml = strlen(kBlockMagic);
    if (strncmp(buf, kBlockMagic, ml) != 0 || !blockListSet(buf + ml))
        blockListSet(BLOCKLIST_DEFAULT);
}

void blockListSave(void)
{
    InternalFS.begin();
    InternalFS.remove(kBlockFileName);
    File f(InternalFS);
    if (f.open(kBlockFileName, FILE_O_WRITE))
    {
        f.write((const uint8_t *)kBlockMagic, strlen(kBlockMagic));
        const char *csv = blockListCsv();
        f.write((const uint8_t *)csv, strlen(csv));
        f.close();
    }
}

#else // Host-Tests

void blockListLoad(void) { ensureInit(); }
void blockListSave(void) {}

#endif
