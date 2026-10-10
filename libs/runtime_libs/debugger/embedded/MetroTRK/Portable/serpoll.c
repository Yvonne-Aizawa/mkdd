#include "PowerPC_EABI_Support/MetroTRK/trk.h"

typedef struct TRKFramingState {
    int mMsgBufID;     // _00
    TRKBuffer* mBuffer; // _04
    int mReceiveState; // _08
    BOOL mIsEscape;    // _0C
    int mFCSType;      // _10
} TRKFramingState;

typedef struct TRKPacketHeader {
    int mSize;       // _00
    u8 _04[0x3C];    // _04
} TRKPacketHeader;

static TRKFramingState gTRKFramingState;
__declspec(section ".sdata") void* gTRKInputPendingPtr;

int TRKTestForPacket(void)
{
    int result;
    int bufIdx;
    TRKBuffer* buf;
    TRKPacketHeader header;
    u8 payload[0x880];
    int payloadSize;

    if (TRKPollUART() <= 0) {
        return -1;
    }

    result = TRKGetFreeBuffer(&bufIdx, &buf);
    MWTRACE(4, "TestForPacket : FreeBuffer is  %ld\n", result);
    TRKSetBufferPosition(buf, 0);

    if (TRKReadUARTN(&header, sizeof(header)) == 0) {
        TRKAppendBuffer_ui8(buf, (u8*)&header, sizeof(header));
        result = bufIdx;
        payloadSize = header.mSize - sizeof(header);

        if (payloadSize > 0) {
            MWTRACE(1, "Reading payload %ld bytes\n", payloadSize);

            if (TRKReadUARTN(payload, header.mSize - sizeof(header)) == 0) {
                TRKAppendBuffer_ui8(buf, payload, header.mSize);
            } else {
                MWTRACE(8, "TestForPacket : Invalid size of packet hdr.size\n");
                TRKReleaseBuffer(result);
                result = -1;
            }
        }
    } else {
        MWTRACE(8, "TestForPacket : Invalid size of packet\n");
        TRKReleaseBuffer(result);
        result = -1;
    }

    MWTRACE(1, "TestForPacket returning %ld\n", result);
    return result;
}

void TRKGetInput(void)
{
    int bufIdx = TRKTestForPacket();

    if (bufIdx != -1) {
        TRKGetBuffer(bufIdx);
        TRKProcessInput(bufIdx);
    }
}

void TRKProcessInput(int bufIdx)
{
    TRKEvent event;

    TRKConstructEvent(&event, 2);
    event.mBufferIndex = bufIdx;
    gTRKFramingState.mMsgBufID = -1;
    TRKPostEvent(&event);
}

TRKResult TRKInitializeSerialHandler(void)
{
    gTRKFramingState.mMsgBufID = -1;
    gTRKFramingState.mReceiveState = 0;
    gTRKFramingState.mIsEscape = FALSE;

    MWTRACE(1, "TRK_Packet_Header \t    %ld bytes\n", 0x40);
    MWTRACE(1, "TRK_CMD_ReadMemory     %ld bytes\n", 0x40);
    MWTRACE(1, "TRK_CMD_WriteMemory    %ld bytes\n", 0x40);
    MWTRACE(1, "TRK_CMD_Connect \t    %ld bytes\n", 0x40);
    MWTRACE(1, "TRK_CMD_ReplyAck\t    %ld bytes\n", 0x40);
    MWTRACE(1, "TRK_CMD_ReadRegisters\t%ld bytes\n", 0x40);

    return TRKSuccess;
}

TRKResult TRKTerminateSerialHandler(void) { return TRKSuccess; }
