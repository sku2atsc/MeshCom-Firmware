#ifndef _BLOCKED_CALLS_H_
#define _BLOCKED_CALLS_H_

// Lokale Sperrliste (Rufzeichen ohne Lizenz).
//
// Frames, deren QUELLE ein gesperrtes Rufzeichen ist, ODER deren Pfad ein
// gesperrtes Rufzeichen als Relais enthaelt, werden beim Empfang verworfen:
// keine Anzeige, kein Upload zum Server, kein Relay ins Mesh. Auf einem
// Gateway werden solche Frames ausserdem nicht vom Server auf LoRa gesendet.
//
// Eintrag MIT SSID ("NF0JN-12") trifft genau dieses Rufzeichen.
// Eintrag OHNE SSID ("NF0JN") trifft alle SSIDs dieses Rufzeichens.
// Vergleich ohne Beachtung von Gross-/Kleinschreibung.

static const char * const MC_BLOCKED_CALLS[] = {
    "NF0JN-11",
    "NF0JN-12",
};

static inline char mcBlkUpper(char c)
{
    return (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
}

// Vergleicht das Token tok[0..tlen) mit einem Sperrlisten-Eintrag.
static inline bool mcBlkTokenMatches(const char *tok, unsigned tlen, const char *entry)
{
    // Pfadmarker wie '*' am Ende eines Relais-Eintrags ignorieren
    while (tlen > 0 && (tok[tlen - 1] == '*' || tok[tlen - 1] == ' '))
        tlen--;
    while (tlen > 0 && tok[0] == ' ')
    {
        tok++;
        tlen--;
    }
    if (tlen == 0)
        return false;

    unsigned elen = 0;
    bool entryHasSsid = false;
    while (entry[elen])
    {
        if (entry[elen] == '-')
            entryHasSsid = true;
        elen++;
    }

    // Basisrufzeichen des Tokens (bis zum ersten '-')
    unsigned tbase = 0;
    while (tbase < tlen && tok[tbase] != '-')
        tbase++;

    unsigned cmplen = entryHasSsid ? tlen : tbase;
    if (cmplen != elen)
        return false;
    for (unsigned i = 0; i < elen; i++)
    {
        if (mcBlkUpper(tok[i]) != mcBlkUpper(entry[i]))
            return false;
    }
    return true;
}

static inline bool mcBlkTokenBlocked(const char *tok, unsigned tlen)
{
    for (unsigned i = 0; i < sizeof(MC_BLOCKED_CALLS) / sizeof(MC_BLOCKED_CALLS[0]); i++)
    {
        if (mcBlkTokenMatches(tok, tlen, MC_BLOCKED_CALLS[i]))
            return true;
    }
    return false;
}

// true, wenn das Rufzeichen selbst gesperrt ist.
static inline bool isBlockedCall(const char *call)
{
    if (call == nullptr || call[0] == 0)
        return false;
    unsigned len = 0;
    while (call[len])
        len++;
    return mcBlkTokenBlocked(call, len);
}

// true, wenn irgendein Element eines kommagetrennten Pfads gesperrt ist
// ("SRC,RELAY1,RELAY2").
static inline bool isBlockedInPath(const char *path)
{
    if (path == nullptr)
        return false;
    const char *start = path;
    const char *p = path;
    for (;;)
    {
        if (*p == ',' || *p == 0)
        {
            if (p > start && mcBlkTokenBlocked(start, (unsigned)(p - start)))
                return true;
            if (*p == 0)
                break;
            start = p + 1;
        }
        p++;
    }
    return false;
}

// Gesamtentscheidung fuer einen empfangenen Frame.
static inline bool isBlockedFrame(const char *source_call, const char *path)
{
    return isBlockedCall(source_call) || isBlockedInPath(path);
}

#endif // _BLOCKED_CALLS_H_
