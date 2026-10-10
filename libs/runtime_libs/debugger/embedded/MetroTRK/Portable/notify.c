#include "PowerPC_EABI_Support/MetroTRK/trk.h"

TRKResult TRKDoNotifyStopped(int cmd)
{
    TRKResult result;
    int reqIdx;
    int bufIdx;
    TRKBuffer* msg;

    result = TRKGetFreeBuffer(&bufIdx, &msg);
    if (result == TRKSuccess) {
        if (result == TRKSuccess) {
            if (cmd == 0x90) {
                TRKTargetAddStopInfo(msg);
            } else {
                TRKTargetAddExceptionInfo(msg);
            }
        }

        result = TRKRequestSend(msg, &reqIdx, 2, 3, 1);
        if (result == TRKSuccess) {
            TRKReleaseBuffer(reqIdx);
        }

        TRKReleaseBuffer(bufIdx);
    }

    return result;
}
