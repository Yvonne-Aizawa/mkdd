#include "PowerPC_EABI_Support/MetroTRK/trk.h"

void OSReport(const char* msg);

BOOL usr_puts_serial(const char* msg)
{
    BOOL result = FALSE;
    char c;
    char buf[2];

    while (!result && (c = *msg++) != '\0') {
        BOOL connected = GetTRKConnected();

        buf[0] = c;
        buf[1] = '\0';

        SetTRKConnected(FALSE);
        OSReport(buf);
        SetTRKConnected(connected);

        result = FALSE;
    }

    return result;
}

void usr_put_initialize(void) { }
