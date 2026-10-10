#include "PowerPC_EABI_Support/MetroTRK/trk.h"
#include "PowerPC_EABI_Support/MetroTRK/target_options.h"

int __read_console(u32 handle, u8* buffer, size_t* count, void* idle_proc)
{
    size_t length;
    u8 result;

    if (GetUseSerialIO() == 0) {
        return 1;
    }

    if (GetTRKConnected() == FALSE) {
        return 1;
    }

    length = *count;
    result = TRKAccessFile(0xD1, 0, &length, buffer);
    *count = length;

    switch (result) {
    case 0:
        return 0;
    case 1:
        break;
    case 2:
        return 2;
    }

    return 1;
}

int __TRK_write_console(u32 handle, u8* buffer, size_t* count, void* idle_proc)
{
    size_t length;
    u8 result;

    if (GetUseSerialIO() == 0) {
        return 1;
    }

    if (GetTRKConnected() == FALSE) {
        return 1;
    }

    length = *count;
    result = TRKAccessFile(0xD0, 1, &length, buffer);
    *count = length;

    switch (result) {
    case 0:
        return 0;
    case 1:
        break;
    case 2:
        return 2;
    }

    return 1;
}
