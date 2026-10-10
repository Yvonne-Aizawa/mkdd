#include "PowerPC_EABI_Support/MetroTRK/trk.h"

asm void TRK_flush_cache(register u32 addr, register u32 length)
{
#ifdef __MWERKS__ // clang-format off
    nofralloc
    lis r5, 0xFFFF
    ori r5, r5, 0xFFF1
    and r5, r5, addr
    subf addr, r5, addr
    add length, length, addr
loop:
    dcbst 0, r5
    dcbf 0, r5
    sync
    icbi 0, r5
    addic r5, r5, 8
    subic. length, length, 8
    bge loop
    isync
    blr
#endif // clang-format on
}
