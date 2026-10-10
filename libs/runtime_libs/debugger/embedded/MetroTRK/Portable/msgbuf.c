#include "PowerPC_EABI_Support/MetroTRK/trk.h"

#define TRK_MSG_BUFFER_COUNT 3

TRKBuffer gTRKMsgBufs[TRK_MSG_BUFFER_COUNT];

static inline void TRKSetBufferUsed(TRKBuffer* buf, BOOL used) { buf->mIsInUse = used; }

TRKResult TRKInitializeMessageBuffers(void)
{
    int i;

    for (i = 0; i < TRK_MSG_BUFFER_COUNT; i++) {
        TRKInitializeMutex(&gTRKMsgBufs[i]);
        TRKAcquireMutex(&gTRKMsgBufs[i]);
        TRKSetBufferUsed(&gTRKMsgBufs[i], FALSE);
        TRKReleaseMutex(&gTRKMsgBufs[i]);
    }

    return TRKSuccess;
}

TRKResult TRKGetFreeBuffer(int* bufIdx, TRKBuffer** outBuf)
{
    TRKBuffer* buf;
    TRKResult error = 0x300;
    int i;

    *outBuf = NULL;

    for (i = 0; i < TRK_MSG_BUFFER_COUNT; i++) {
        buf = (TRKBuffer*)TRKGetBuffer(i);

        TRKAcquireMutex(buf);
        if (!buf->mIsInUse) {
            TRKResetBuffer(buf, TRUE);
            TRKSetBufferUsed(buf, TRUE);
            error = TRKSuccess;
            *outBuf = buf;
            *bufIdx = i;
            i = TRK_MSG_BUFFER_COUNT;
        }
        TRKReleaseMutex(buf);
    }

    if (error == 0x300) {
        usr_puts_serial("ERROR : No buffer available\n");
    }

    return error;
}

void* TRKGetBuffer(int idx)
{
    TRKBuffer* buf = NULL;

    if (idx >= 0 && idx < TRK_MSG_BUFFER_COUNT) {
        buf = &gTRKMsgBufs[idx];
    }

    return buf;
}

void TRKReleaseBuffer(int idx)
{
    TRKBuffer* buf;

    if (idx != -1 && idx >= 0 && idx < TRK_MSG_BUFFER_COUNT) {
        buf = &gTRKMsgBufs[idx];
        TRKAcquireMutex(buf);
        TRKSetBufferUsed(buf, FALSE);
        TRKReleaseMutex(buf);
    }
}

void TRKResetBuffer(TRKBuffer* buf, BOOL keepData)
{
    buf->mLength = 0;
    buf->mPosition = 0;

    if (!keepData) {
        TRK_memset(buf->mData, 0, TRKMSGBUF_SIZE);
    }
}

TRKResult TRKSetBufferPosition(TRKBuffer* buf, u32 pos)
{
    TRKResult error = TRKSuccess;

    if (pos > TRKMSGBUF_SIZE) {
        error = TRKError301;
    } else {
        buf->mPosition = pos;
        if (pos > buf->mLength) {
            buf->mLength = pos;
        }
    }

    return error;
}

TRKResult TRKAppendBuffer(TRKBuffer* buf, const void* data, size_t length)
{
    TRKResult error = TRKSuccess;
    u32 bytesLeft;

    if (length == 0) {
        return TRKSuccess;
    }

    bytesLeft = TRKMSGBUF_SIZE - buf->mPosition;
    if (bytesLeft < length) {
        error = TRKError301;
        length = bytesLeft;
    }

    if (length == 1) {
        buf->mData[buf->mPosition] = ((u8*)data)[0];
    } else {
        TRK_memcpy(buf->mData + buf->mPosition, data, length);
    }

    buf->mPosition += length;
    buf->mLength = buf->mPosition;
    return error;
}

TRKResult TRKReadBuffer(TRKBuffer* buf, void* data, size_t length)
{
    TRKResult error = TRKSuccess;
    unsigned int bytesLeft;

    if (length == 0) {
        return TRKSuccess;
    }

    bytesLeft = buf->mLength - buf->mPosition;
    if (length > bytesLeft) {
        error = TRKError302;
        length = bytesLeft;
    }

    TRK_memcpy(data, buf->mData + buf->mPosition, length);
    buf->mPosition += length;
    return error;
}

static inline TRKResult TRKAppendBuffer1_ui8(TRKBuffer* buf, const u8 data)
{
    if (buf->mPosition >= TRKMSGBUF_SIZE) {
        return TRKError301;
    }

    buf->mData[buf->mPosition++] = data;
    buf->mLength++;
    return TRKSuccess;
}

static inline TRKResult TRKAppendBuffer1_ui32(TRKBuffer* buf, const u32 data)
{
    u32 bigEndianData;
    u8* bigEndianStart;
    u8* byteData;

    if (gTRKBigEndian) {
        bigEndianStart = (u8*)&data;
    } else {
        byteData = (u8*)&data;
        bigEndianStart = (u8*)&bigEndianData;

        bigEndianStart[0] = byteData[3];
        bigEndianStart[1] = byteData[2];
        bigEndianStart[2] = byteData[1];
        bigEndianStart[3] = byteData[0];
    }

    return TRKAppendBuffer(buf, bigEndianStart, sizeof(data));
}

TRKResult TRKAppendBuffer1_ui64(TRKBuffer* buf, const u64 data)
{
    u64 bigEndianData;
    u8* bigEndianStart;
    u8* byteData;

    if (gTRKBigEndian) {
        bigEndianStart = (u8*)&data;
    } else {
        byteData = (u8*)&data;
        bigEndianStart = (u8*)&bigEndianData;

        bigEndianStart[0] = byteData[7];
        bigEndianStart[1] = byteData[6];
        bigEndianStart[2] = byteData[5];
        bigEndianStart[3] = byteData[4];
        bigEndianStart[4] = byteData[3];
        bigEndianStart[5] = byteData[2];
        bigEndianStart[6] = byteData[1];
        bigEndianStart[7] = byteData[0];
    }

    return TRKAppendBuffer(buf, bigEndianStart, sizeof(data));
}

TRKResult TRKAppendBuffer_ui8(TRKBuffer* buf, u8* data, int count)
{
    TRKResult error;
    int i;

    for (i = 0, error = TRKSuccess; error == TRKSuccess && i < count; i++) {
        error = TRKAppendBuffer1_ui8(buf, data[i]);
    }

    return error;
}

TRKResult TRKAppendBuffer_ui32(TRKBuffer* buf, const u32* data, int count)
{
    TRKResult error;
    int i;

    for (i = 0, error = TRKSuccess; error == TRKSuccess && i < count; i++) {
        error = TRKAppendBuffer1_ui32(buf, data[i]);
    }

    return error;
}

static inline TRKResult TRKReadBuffer1_ui8(TRKBuffer* buf, u8* data) { return TRKReadBuffer(buf, data, 1); }

static inline TRKResult TRKReadBuffer1_ui32(TRKBuffer* buf, u32* data)
{
    TRKResult error;
    u8* bigEndianStart;
    u8* byteData;
    u32 swapBuffer;

    if (gTRKBigEndian) {
        bigEndianStart = (u8*)data;
    } else {
        bigEndianStart = (u8*)&swapBuffer;
    }

    error = TRKReadBuffer(buf, bigEndianStart, sizeof(*data));

    if (!gTRKBigEndian && error == TRKSuccess) {
        byteData = (u8*)data;

        byteData[0] = bigEndianStart[3];
        byteData[1] = bigEndianStart[2];
        byteData[2] = bigEndianStart[1];
        byteData[3] = bigEndianStart[0];
    }

    return error;
}

TRKResult TRKReadBuffer1_ui64(TRKBuffer* buf, u64* data)
{
    TRKResult error;
    u8* bigEndianStart;
    u8* byteData;
    u64 swapBuffer;

    if (gTRKBigEndian) {
        bigEndianStart = (u8*)data;
    } else {
        bigEndianStart = (u8*)&swapBuffer;
    }

    error = TRKReadBuffer(buf, bigEndianStart, sizeof(*data));

    if (!gTRKBigEndian && error == TRKSuccess) {
        byteData = (u8*)data;

        byteData[0] = bigEndianStart[7];
        byteData[1] = bigEndianStart[6];
        byteData[2] = bigEndianStart[5];
        byteData[3] = bigEndianStart[4];
        byteData[4] = bigEndianStart[3];
        byteData[5] = bigEndianStart[2];
        byteData[6] = bigEndianStart[1];
        byteData[7] = bigEndianStart[0];
    }

    return error;
}

TRKResult TRKReadBuffer_ui8(TRKBuffer* buf, u8* data, int count)
{
    TRKResult error;
    int i;

    for (i = 0, error = TRKSuccess; error == TRKSuccess && i < count; i++) {
        error = TRKReadBuffer1_ui8(buf, &data[i]);
    }

    return error;
}

TRKResult TRKReadBuffer_ui32(TRKBuffer* buf, u32* data, int count)
{
    TRKResult error;
    int i;

    for (i = 0, error = TRKSuccess; error == TRKSuccess && i < count; i++) {
        error = TRKReadBuffer1_ui32(buf, &data[i]);
    }

    return error;
}
