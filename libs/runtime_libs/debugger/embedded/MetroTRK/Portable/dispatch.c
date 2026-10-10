#include "PowerPC_EABI_Support/MetroTRK/trk.h"

TRKResult TRKInitializeDispatcher(void) { return TRKSuccess; }

TRKResult TRKDispatchMessage(TRKBuffer* buf)
{
    TRKResult error = 0x500;

    TRKSetBufferPosition(buf, 0);
    MWTRACE(1, "Dispatch command 0x%08x\n", buf->mData[4]);

    switch (buf->mData[4]) {
    case TRK_DISPATCH_CMD_CONNECT:
        error = TRKDoConnect(buf);
        break;
    case TRK_DISPATCH_CMD_DISCONNECT:
        error = TRKDoDisconnect(buf);
        break;
    case TRK_DISPATCH_CMD_RESET:
        error = TRKDoReset(buf);
        break;
    case TRK_DISPATCH_CMD_OVERRIDE:
        error = TRKDoOverride(buf);
        break;
    case TRK_DISPATCH_CMD_GETVERSION:
        error = TRKDoVersions(buf);
        break;
    case TRK_DISPATCH_CMD_GETSUPPORTMASK:
        error = TRKDoSupportMask(buf);
        break;
    case TRK_DISPATCH_CMD_READMEM:
        error = TRKDoReadMemory(buf);
        break;
    case TRK_DISPATCH_CMD_WRITEMEM:
        error = TRKDoWriteMemory(buf);
        break;
    case TRK_DISPATCH_CMD_READREGS:
        error = TRKDoReadRegisters(buf);
        break;
    case TRK_DISPATCH_CMD_WRITEREGS:
        error = TRKDoWriteRegisters(buf);
        break;
    case TRK_DISPATCH_CMD_CONTINUE:
        error = TRKDoContinue(buf);
        break;
    case TRK_DISPATCH_CMD_STEP:
        error = TRKDoStep(buf);
        break;
    case TRK_DISPATCH_CMD_STOP:
        error = TRKDoStop(buf);
        break;
    case TRK_DISPATCH_CMD_SETOPTION:
        error = TRKDoSetOption(buf);
        break;
    }

    MWTRACE(1, "Dispatch complete err = %ld\n", error);
    return error;
}
