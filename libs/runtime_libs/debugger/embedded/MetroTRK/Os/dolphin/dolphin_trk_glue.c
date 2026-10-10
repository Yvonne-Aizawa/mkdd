#include "PowerPC_EABI_Support/MetroTRK/trk.h"

typedef struct OSContext OSContext;

void OSReport(const char* msg, ...);
long OSEnableScheduler(void);
void PPCHalt(void);
void ICInvalidateRange(void* addr, u32 nBytes);
void DCFlushRange(void* addr, u32 nBytes);
void TRKInterruptHandler(void);

int Hu_IsStub(void);
int AMC_IsStub(void);

int udp_cc_initialize(volatile u8**, void*);
int udp_cc_shutdown(void);
int udp_cc_open(void);
int udp_cc_close(void);
int udp_cc_read(u8*, int);
int udp_cc_write(const u8*, int);
int udp_cc_peek(void);
int udp_cc_pre_continue(void);
int udp_cc_post_stop(void);

int gdev_cc_initialize(volatile u8**, void*);
int gdev_cc_shutdown(void);
int gdev_cc_open(void);
int gdev_cc_close(void);
int gdev_cc_read(u8*, int);
int gdev_cc_write(const u8*, int);
int gdev_cc_peek(void);
int gdev_cc_pre_continue(void);
int gdev_cc_post_stop(void);
int gdev_cc_initinterrupts(void);

int ddh_cc_initialize(volatile u8**, void*);
int ddh_cc_shutdown(void);
int ddh_cc_open(void);
int ddh_cc_close(void);
int ddh_cc_read(u8*, int);
int ddh_cc_write(const u8*, int);
int ddh_cc_peek(void);
int ddh_cc_pre_continue(void);
int ddh_cc_post_stop(void);
int ddh_cc_initinterrupts(void);

void TRKEXICallBack(s16 chan, OSContext* ctx);

DBCommTable gDBCommTable = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
BOOL _MetroTRK_Has_Framing;
u8 TRK_Use_BBA;

asm void TRKLoadContext(OSContext* ctx, u32 exceptionID)
{
#ifdef __MWERKS__ // clang-format off
    nofralloc
    lwz r0, 0x0(r3)
    lwz r1, 0x4(r3)
    lwz r2, 0x8(r3)
    lhz r5, 0x1A2(r3)
    rlwinm. r6, r5, 0, 30, 30
    beq lbl_load_gpr13
    rlwinm r5, r5, 0, 31, 29
    sth r5, 0x1A2(r3)
    lmw r5, 0x14(r3)
    b lbl_restore_sprs
lbl_load_gpr13:
    lmw r13, 0x34(r3)
lbl_restore_sprs:
    mr r31, r3
    mr r3, r4
    lwz r4, 0x80(r31)
    mtcrf 0xFF, r4
    lwz r4, 0x84(r31)
    mtlr r4
    lwz r4, 0x88(r31)
    mtctr r4
    lwz r4, 0x8C(r31)
    mtxer r4
    mfmsr r4
    rlwinm r4, r4, 0, 17, 15
    rlwinm r4, r4, 0, 31, 29
    mtmsr r4
    mtsprg 1, r2
    lwz r4, 0xC(r31)
    mtsprg 2, r4
    lwz r4, 0x10(r31)
    mtsprg 3, r4
    lwz r2, 0x198(r31)
    lwz r4, 0x19C(r31)
    lwz r31, 0x7C(r31)
    b TRKInterruptHandler
#endif // clang-format on
}

void TRKEXICallBack(s16 chan, OSContext* ctx)
{
    OSEnableScheduler();
    TRKLoadContext(ctx, 0x500);
}

int InitMetroTRKCommTable(int hwId)
{
    int result = 1;

    OSReport("Devkit set to : %ld\n", hwId);
    TRK_Use_BBA = FALSE;

    if (hwId == 2) {
        OSReport("MetroTRK : Set to BBA\n");
        TRK_Use_BBA = TRUE;
        gDBCommTable.initialize_func = udp_cc_initialize;
        gDBCommTable.open_func = udp_cc_open;
        gDBCommTable.close_func = udp_cc_close;
        gDBCommTable.read_func = udp_cc_read;
        gDBCommTable.write_func = udp_cc_write;
        gDBCommTable.shutdown_func = udp_cc_shutdown;
        gDBCommTable.peek_func = udp_cc_peek;
        gDBCommTable.pre_continue_func = udp_cc_pre_continue;
        gDBCommTable.post_stop_func = udp_cc_post_stop;
        gDBCommTable.initinterrupts_func = NULL;
        return 0;
    } else if (hwId == 1) {
        OSReport("MetroTRK : Set to GDEV hardware\n");
        result = Hu_IsStub();
        gDBCommTable.initialize_func = gdev_cc_initialize;
        gDBCommTable.open_func = gdev_cc_open;
        gDBCommTable.close_func = gdev_cc_close;
        gDBCommTable.read_func = gdev_cc_read;
        gDBCommTable.write_func = gdev_cc_write;
        gDBCommTable.shutdown_func = gdev_cc_shutdown;
        gDBCommTable.peek_func = gdev_cc_peek;
        gDBCommTable.pre_continue_func = gdev_cc_pre_continue;
        gDBCommTable.post_stop_func = gdev_cc_post_stop;
        gDBCommTable.initinterrupts_func = gdev_cc_initinterrupts;
    } else if (hwId == 0) {
        OSReport("MetroTRK : Set to AMC DDH hardware\n");
        result = AMC_IsStub();
        gDBCommTable.initialize_func = ddh_cc_initialize;
        gDBCommTable.open_func = ddh_cc_open;
        gDBCommTable.close_func = ddh_cc_close;
        gDBCommTable.read_func = ddh_cc_read;
        gDBCommTable.write_func = ddh_cc_write;
        gDBCommTable.shutdown_func = ddh_cc_shutdown;
        gDBCommTable.peek_func = ddh_cc_peek;
        gDBCommTable.pre_continue_func = ddh_cc_pre_continue;
        gDBCommTable.post_stop_func = ddh_cc_post_stop;
        gDBCommTable.initinterrupts_func = ddh_cc_initinterrupts;
    } else {
        OSReport("MetroTRK : Set to UNKNOWN hardware. (%ld)\n", hwId);
        OSReport("MetroTRK : Invalid hardware ID passed from OS\n");
        OSReport("MetroTRK : Defaulting to GDEV Hardware\n");
    }

    return result;
}

TRKResult TRKInitializeIntDrivenUART(unknown param_0, unknown param_1, unknown param_2, void* inputPendingPtr)
{
    gDBCommTable.initialize_func(inputPendingPtr, TRKEXICallBack);
    gDBCommTable.open_func();
    return TRKSuccess;
}

void EnableEXI2Interrupts(void)
{
    if (!TRK_Use_BBA) {
        if (gDBCommTable.initinterrupts_func != NULL) {
            gDBCommTable.initinterrupts_func();
        }
    }
}

int TRKPollUART(void) { return gDBCommTable.peek_func(); }

int TRKReadUARTN(void* bytes, u32 length)
{
    int readErr = gDBCommTable.read_func(bytes, length);
    return readErr == 0 ? 0 : -1;
}

TRKResult TRKWriteUARTN(const void* bytes, u32 length)
{
    int writeErr = gDBCommTable.write_func(bytes, length);
    return writeErr == 0 ? 0 : -1;
}

void ReserveEXI2Port(void) { gDBCommTable.post_stop_func(); }

void UnreserveEXI2Port(void) { gDBCommTable.pre_continue_func(); }

void TRK_board_display(char* str) { OSReport("%s\n", str); }

void InitializeProgramEndTrap(void)
{
    static const u32 EndofProgramInstruction = 0x00454E44;
    u8* ppcHalt = (u8*)PPCHalt;

    TRK_memcpy(ppcHalt + 4, &EndofProgramInstruction, 4);
    ICInvalidateRange(ppcHalt + 4, 4);
    DCFlushRange(ppcHalt + 4, 4);
}

void TRKUARTInterruptHandler(void) { }
