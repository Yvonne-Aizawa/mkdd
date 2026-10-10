#include "PowerPC_EABI_Support/MetroTRK/trk.h"

TRKResult TRKMessageSend(TRK_Msg* msg)
{
    TRKResult result = TRKWriteUARTN(&msg->mMsg, msg->mMsgLength);
    MWTRACE(1, "MessageSend : cc_write returned %ld\n", result);
    return TRKSuccess;
}
