#include "PowerPC_EABI_Support/MetroTRK/trk.h"

BOOL gTRKBigEndian;

static inline BOOL TRK_InitializeEndian(void)
{
    u8 bendian[4];
    BOOL result = FALSE;

    gTRKBigEndian = TRUE;

    bendian[0] = 0x12;
    bendian[1] = 0x34;
    bendian[2] = 0x56;
    bendian[3] = 0x78;

    if (*(u32*)bendian == 0x12345678) {
        gTRKBigEndian = TRUE;
    } else if (*(u32*)bendian == 0x78563412) {
        gTRKBigEndian = FALSE;
    } else {
        result = TRUE;
    }

    return result;
}

TRKResult TRKInitializeNub(void)
{
    TRKResult result;
    TRKResult uartResult;

    result = TRK_InitializeEndian();
    MWTRACE(1, "Initialize NUB\n");

    if (result == TRKSuccess) {
        usr_put_initialize();
    }

    if (result == TRKSuccess) {
        result = TRKInitializeEventQueue();
    }

    if (result == TRKSuccess) {
        result = TRKInitializeMessageBuffers();
    }

    if (result == TRKSuccess) {
        result = TRKInitializeDispatcher();
    }

    InitializeProgramEndTrap();

    if (result == TRKSuccess) {
        result = TRKInitializeSerialHandler();
    }

    if (result == TRKSuccess) {
        result = TRKInitializeTarget();
    }

    if (result == TRKSuccess) {
        uartResult = TRKInitializeIntDrivenUART(0xE100, 1, 0, &gTRKInputPendingPtr);
        TRKTargetSetInputPendingPtr(gTRKInputPendingPtr);
        if (uartResult != TRKSuccess) {
            result = uartResult;
        }
    }

    return result;
}

TRKResult TRKTerminateNub(void)
{
    TRKTerminateSerialHandler();
    return TRKSuccess;
}

void TRKNubWelcome(void) { TRK_board_display("MetroTRK for GAMECUBE v2.0"); }
