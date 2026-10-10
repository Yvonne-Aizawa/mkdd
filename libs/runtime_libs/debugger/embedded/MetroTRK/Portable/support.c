#include "PowerPC_EABI_Support/MetroTRK/trk.h"

void* memset(void* dst, int val, size_t n);
size_t strlen(const char* str);

TRKResult TRKSuppAccessFile(u32 handle, u8* data, size_t* count, int* ioResult, BOOL needReply, BOOL read)
{
    u8 replyIOResult;
    TRKResult error;
    u16 replyLength;
    int replyBufIdx;
    TRKBuffer* replyBuf;
    int bufIdx;
    TRKBuffer* buf;
    unsigned int length;
    unsigned int done;
    BOOL exit;
    u8 cmd;
    TRKFileMessage msg;

    if (data == NULL || *count == 0) {
        return 2;
    }

    exit = FALSE;
    *ioResult = 0;
    done = 0;
    error = TRKSuccess;

    while (!exit && done < *count && error == TRKSuccess && *ioResult == 0) {
        memset(&msg, 0, sizeof(msg));

        length = (*count - done > 0x800) ? 0x800 : *count - done;

        cmd = 0xD0;
        if (read) {
            cmd = 0xD1;
        }
        msg.mCommand = cmd;
        msg.mLength = read ? sizeof(msg) : sizeof(msg) + length;
        msg._08.mHandle = handle;
        msg._0C.mDataLength = length;

        TRKGetFreeBuffer(&bufIdx, &buf);
        error = TRKAppendBuffer_ui8(buf, (u8*)&msg, sizeof(msg));

        if (!read && error == TRKSuccess) {
            error = TRKAppendBuffer_ui8(buf, data + done, length);
        }

        if (error == TRKSuccess) {
            if (needReply) {
                error = TRKRequestSend(buf, &replyBufIdx, read ? 5 : 5, 3, !(read && handle == 0));
                if (error == TRKSuccess) {
                    replyBuf = (TRKBuffer*)TRKGetBuffer(replyBufIdx);
                }

                replyIOResult = ((TRKFileMessage*)replyBuf->mData)->_10.mIOResult;
                replyLength = ((TRKFileMessage*)replyBuf->mData)->mReplyLength;

                if (read && error == TRKSuccess && replyLength <= length) {
                    TRKSetBufferPosition(replyBuf, sizeof(msg));
                    error = TRKReadBuffer_ui8(replyBuf, data + done, replyLength);
                    if (error == TRKError302) {
                        error = TRKSuccess;
                    }
                }

                if (replyLength != length) {
                    if ((!read || replyLength >= length) && replyIOResult == 0) {
                        replyIOResult = 1;
                    }
                    length = replyLength;
                    exit = TRUE;
                }

                *ioResult = replyIOResult;
                TRKReleaseBuffer(replyBufIdx);
            } else {
                error = TRKMessageSend((TRK_Msg*)buf);
            }
        }

        TRKReleaseBuffer(bufIdx);
        done += length;
    }

    *count = done;
    return error;
}

TRKResult TRKRequestSend(TRKBuffer* msgBuf, int* bufIdx, u32 p1, u32 p2, int p3)
{
    TRKResult error = TRKSuccess;
    TRKBuffer* buf;
    u32 timer;
    int tries;
    u8 msgCommand;
    u8 msgError;
    BOOL badReply = TRUE;

    *bufIdx = -1;

    for (tries = p2 + 1; tries != 0 && *bufIdx == -1 && error == TRKSuccess; tries--) {
        MWTRACE(1, "Calling MessageSend\n");
        error = TRKMessageSend((TRK_Msg*)msgBuf);

        if (error == TRKSuccess) {
            if (p3) {
                timer = 0;
            }

            while (TRUE) {
                do {
                    *bufIdx = TRKTestForPacket();
                    if (*bufIdx != -1) {
                        break;
                    }
                } while (!p3 || ++timer < 79999980);

                if (*bufIdx == -1) {
                    break;
                }

                badReply = FALSE;

                buf = (TRKBuffer*)TRKGetBuffer(*bufIdx);
                TRKSetBufferPosition(buf, 0);
                OutputData(&buf->mData[0], buf->mLength);
                msgCommand = buf->mData[4];
                MWTRACE(1, "msg_command : 0x%02x hdr->cmdID 0x%02x\n", msgCommand, msgCommand);

                if (msgCommand >= 0x80) {
                    break;
                }

                TRKProcessInput(*bufIdx);
                *bufIdx = -1;
            }

            if (*bufIdx != -1) {
                if (buf->mLength < 0x40) {
                    badReply = TRUE;
                }

                if (error == TRKSuccess && !badReply) {
                    msgError = buf->mData[8];
                    MWTRACE(1, "msg_error : 0x%02x\n", msgError);
                }

                if (error == TRKSuccess && !badReply) {
                    if ((int)msgCommand != 0x80 || (int)msgError != 0) {
                        MWTRACE(8, "RequestSend : Bad ack or non ack received msg_command : 0x%02x msg_error 0x%02x\n",
                                msgCommand, msgError);
                        badReply = TRUE;
                    }
                }

                if (error != TRKSuccess || badReply) {
                    TRKReleaseBuffer(*bufIdx);
                    *bufIdx = -1;
                }
            }
        }
    }

    if (*bufIdx == -1) {
        error = 0x800;
    }

    return error;
}

TRKResult HandleOpenFileSupportRequest(const char* path, u8 mode, u32* handle, u32* ioResult)
{
    TRKResult error;
    int replyBufIdx;
    TRKBuffer* replyBuf;
    int bufIdx;
    TRKBuffer* buf;
    TRKFileMessage msg;

    memset(&msg, 0, sizeof(msg));
    *handle = 0;
    msg.mCommand = 0xD2;
    msg.mLength = sizeof(msg) + strlen(path) + 1;
    msg._08.mOpenMode = mode;
    msg._0C.mDataLength = strlen(path) + 1;

    TRKGetFreeBuffer(&bufIdx, &buf);
    error = TRKAppendBuffer_ui8(buf, (u8*)&msg, sizeof(msg));

    if (error == TRKSuccess) {
        error = TRKAppendBuffer_ui8(buf, (u8*)path, strlen(path) + 1);
    }

    if (error == TRKSuccess) {
        *ioResult = 0;
        error = TRKRequestSend(buf, &replyBufIdx, 7, 3, 0);
        if (error == TRKSuccess) {
            replyBuf = (TRKBuffer*)TRKGetBuffer(replyBufIdx);
        }

        *ioResult = ((TRKFileMessage*)replyBuf->mData)->_10.mIOResult;
        *handle = ((TRKFileMessage*)replyBuf->mData)->_08.mHandle;
        TRKReleaseBuffer(replyBufIdx);
    }

    TRKReleaseBuffer(bufIdx);
    return error;
}

TRKResult HandleCloseFileSupportRequest(u32 handle, u32* ioResult)
{
    TRKResult error;
    int replyBufIdx;
    TRKBuffer* replyBuf;
    int bufIdx;
    TRKBuffer* buf;
    TRKFileMessage msg;

    memset(&msg, 0, sizeof(msg));
    msg.mCommand = 0xD3;
    msg.mLength = sizeof(msg);
    msg._08.mHandle = handle;

    error = TRKGetFreeBuffer(&bufIdx, &buf);
    if (error == TRKSuccess) {
        error = TRKAppendBuffer_ui8(buf, (u8*)&msg, sizeof(msg));
    }

    if (error == TRKSuccess) {
        *ioResult = 0;
        error = TRKRequestSend(buf, &replyBufIdx, 3, 3, 0);
        if (error == TRKSuccess) {
            replyBuf = (TRKBuffer*)TRKGetBuffer(replyBufIdx);
        }

        if (error == TRKSuccess) {
            *ioResult = ((TRKFileMessage*)replyBuf->mData)->_10.mIOResult;
        }

        TRKReleaseBuffer(replyBufIdx);
    }

    TRKReleaseBuffer(bufIdx);
    return error;
}

TRKResult HandlePositionFileSupportRequest(u32 handle, u32* position, u8 seekMode, u32* ioResult)
{
    TRKResult error;
    int replyBufIdx;
    TRKBuffer* replyBuf;
    int bufIdx;
    TRKBuffer* buf;
    TRKFileMessage msg;

    memset(&msg, 0, sizeof(msg));
    msg.mCommand = 0xD4;
    msg.mLength = sizeof(msg);
    msg._08.mHandle = handle;
    msg._0C.mPosition = *position;
    msg._10.mSeekMode = seekMode;

    error = TRKGetFreeBuffer(&bufIdx, &buf);
    if (error == TRKSuccess) {
        error = TRKAppendBuffer_ui8(buf, (u8*)&msg, sizeof(msg));
    }

    if (error == TRKSuccess) {
        *ioResult = 0;
        *position = -1;
        error = TRKRequestSend(buf, &replyBufIdx, 3, 3, 0);
        if (error == TRKSuccess) {
            replyBuf = (TRKBuffer*)TRKGetBuffer(replyBufIdx);
            if (replyBuf != NULL) {
                *ioResult = ((TRKFileMessage*)replyBuf->mData)->_10.mIOResult;
                *position = ((TRKFileMessage*)replyBuf->mData)->mReplyPosition;
            }
        }

        TRKReleaseBuffer(replyBufIdx);
    }

    TRKReleaseBuffer(bufIdx);
    return error;
}
