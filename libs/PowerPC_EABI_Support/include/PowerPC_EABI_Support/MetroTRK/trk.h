#ifndef _DOLPHIN_TRK_H
#define _DOLPHIN_TRK_H

#include "types.h"
#include "Dolphin/db.h"

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

/* TRK */

#define TRK_DISPATCH_CMD_CONNECT        1 /* Connect to the console */
#define TRK_DISPATCH_CMD_DISCONNECT     2 /* Disconnect from the console */
#define TRK_DISPATCH_CMD_RESET          3 /* Reset the debugger */
#define TRK_DISPATCH_CMD_GETVERSION     4 /* Get debugger version */
#define TRK_DISPATCH_CMD_GETSUPPORTMASK 5 /* Get Support Mask */
#define TRK_DISPATCH_CMD_OVERRIDE       7 /* Override? */
#define TRK_DISPATCH_CMD_READMEM        16 /* Reading from memory */
#define TRK_DISPATCH_CMD_WRITEMEM       17 /* Writing to memory */
#define TRK_DISPATCH_CMD_READREGS       18 /* Read a register value */
#define TRK_DISPATCH_CMD_WRITEREGS      19 /* Set a register */
#define TRK_DISPATCH_CMD_SETOPTION      23 /* Set an option? */
#define TRK_DISPATCH_CMD_CONTINUE       24 /* Continue debugging */
#define TRK_DISPATCH_CMD_STEP           25 /* Step through an instruction */
#define TRK_DISPATCH_CMD_STOP           26 /* Stop the debugger */

typedef struct _TRK_Msg {
	u8 _00[8];      // _00
	u32 mMsgLength; // _08
	u32 _0C;        // _0C
	u32 mMsg;       // _10
} TRK_Msg;

/**
 * @size{0xC}
 */
typedef struct TRKEvent {
	int mEventType;
	int _04;
	int mBufferIndex;
} TRKEvent;

/**
 * @size{0x28}
 */
typedef struct TRKEventQueue {
	u8 _00[4];
	int mCurrEvtID;
	int mNextSlotToOverwrite;
	TRKEvent mEvents[2];
	u32 _24; /* max of 0x100? */
} TRKEventQueue;

/**
 * @size{0xA4}
 */
typedef struct TRKState {
	u32 _00;
	u32 _04;
	u32 _08;
	u32 _0C;
	u32 _10;
	u32 _14;
	u32 _18;
	u32 _1C;
	u32 _20;
	u32 _24;
	u32 _28;
	u32 _2C;
	u32 _30;
	u32 _34;
	u32 _38;
	u32 _3C;
	u32 _40;
	u32 _44;
	u32 _48;
	u32 _4C;
	u32 _50;
	u32 _54;
	u32 _58;
	u32 _5C;
	u32 _60;
	u32 _64;
	u32 _68;
	u32 _6C;
	u32 _70;
	u32 _74;
	u32 _78;
	u32 _7C;
	u32 _80;
	u32 _84;
	u32 _88;
	u32 _8C;
	u32 _90;
	u32 _94;
	BOOL mIsStopped;
	u32 _9C;
	u32 _A0;
	u32 _A4;
	u32 _A8;
	u32 _AC;
} TRKState;

#define TRKMSGBUF_SIZE 0x880

/**
 * @size{0x890}
 */
typedef struct TRKBuffer {
	u32 mMutex;               // _00
	BOOL mIsInUse;            // _04
	u32 mLength;              // _08
	u32 mPosition;            // _0C
	u8 mData[TRKMSGBUF_SIZE]; // _10
} TRKBuffer;

/**
 * Header shared by file I/O requests and their replies.
 * @size{0x40}
 */
typedef struct TRKFileMessage {
	u32 mLength;  // _00
	u8 mCommand;  // _04
	u8 _05[3];    // _05
	union {
		u32 mHandle;   // reply: position
		u8 mOpenMode;
	} _08;
	union {
		u16 mDataLength;
		u32 mPosition;
	} _0C;
	union {
		u8 mSeekMode;
		u32 mIOResult; // reply
	} _10;
	u16 mReplyLength; // _14, reply
	u8 _16[2];        // _16
	u32 mReplyPosition; // _18, reply
	u8 _1C[0x24];     // _1C
} TRKFileMessage;
typedef struct Default_PPC {
	u32 GPR[32]; // _00
	u32 PC;      // _80
	u32 LR;      // _84
	u32 CR;      // _88
	u32 CTR;     // _8C
	u32 XER;     // _90
} Default_PPC;

typedef struct Float_PPC {
	f64 FPR[32]; // _00
	u64 FPSCR;   // _100
	u64 FPECR;   // _108
} Float_PPC;

typedef struct Extended1_PPC_6xx_7xx {
	u32 SR[16];  // _00
	u32 TBL;     // _40
	u32 TBU;     // _44
	u32 HID0;    // _48
	u32 HID1;    // _4C
	u32 MSR;     // _50
	u32 PVR;     // _54
	u32 IBAT0U;  // _58
	u32 IBAT0L;  // _5C
	u32 IBAT1U;  // _60
	u32 IBAT1L;  // _64
	u32 IBAT2U;  // _68
	u32 IBAT2L;  // _6C
	u32 IBAT3U;  // _70
	u32 IBAT3L;  // _74
	u32 DBAT0U;  // _78
	u32 DBAT0L;  // _7C
	u32 DBAT1U;  // _80
	u32 DBAT1L;  // _84
	u32 DBAT2U;  // _88
	u32 DBAT2L;  // _8C
	u32 DBAT3U;  // _90
	u32 DBAT3L;  // _94
	u8 _98[0x1F0]; // _98, remaining SPRs (not yet mapped)
} Extended1_PPC_6xx_7xx;

/**
 * @size{0x430}
 */
typedef struct ProcessorState_PPC {
	Default_PPC Default;             // _000
	Float_PPC Float;                 // _098
	Extended1_PPC_6xx_7xx Extended1; // _1A8
} ProcessorState_PPC;

extern ProcessorState_PPC gTRKCPUState;
extern TRKState gTRKState;

typedef enum { TRKSuccess = 0, TRKError100 = 0x100, TRKError301 = 0x301, TRKError302 = 0x302 } TRKResult;

extern BOOL gTRKBigEndian;

u32 TRKDoConnect(TRKBuffer*);
u32 TRKDoDisconnect(TRKBuffer*);
u32 TRKDoReset(TRKBuffer*);
u32 TRKDoVersions(TRKBuffer*);
u32 TRKDoSupportMask(TRKBuffer*);
u32 TRKDoOverride(TRKBuffer*);
u32 TRKDoReadMemory(TRKBuffer*);
u32 TRKDoWriteMemory(TRKBuffer*);
u32 TRKDoReadRegisters(TRKBuffer*);
u32 TRKDoWriteRegisters(TRKBuffer*);
u32 TRKDoSetOption(TRKBuffer*);
u32 TRKDoContinue(TRKBuffer*);
u32 TRKDoStep(TRKBuffer*);
u32 TRKDoStop(TRKBuffer*);

void InitMetroTRK(void);
void InitMetroTRK_BBA(void);
void EnableMetroTRKInterrupts(void);

void TRKDestructEvent(TRKEvent*);
void TRKConstructEvent(TRKEvent*, int);
TRKResult TRKPostEvent(TRKEvent*);
TRKResult TRKDispatchMessage(TRKBuffer*);
void* TRKGetBuffer(int);
void TRKReleaseBuffer(int);
TRKResult TRKGetFreeBuffer(int* bufIdx, TRKBuffer** buf);
TRKResult TRKRequestSend(TRKBuffer* msg, int* bufIdx, u32 p1, u32 p2, int p3);
TRKResult TRKDoNotifyStopped(int cmd);
void TRKTargetAddStopInfo(TRKBuffer*);
void TRKTargetAddExceptionInfo(TRKBuffer*);
void TRKGetInput();
BOOL TRKGetNextEvent(TRKEvent*);

TRKResult TRKTargetContinue(void);
TRKResult TRKTargetInterrupt(TRKEvent*);
BOOL TRKTargetStopped();
void TRKTargetSetStopped(uint);
TRKResult TRKTargetSupportRequest();

TRKResult TRKAppendBuffer_ui8(TRKBuffer*, u8*, int);
TRKResult TRKSetBufferPosition(TRKBuffer*, u32);
TRKResult TRKReadBuffer_ui8(TRKBuffer*, u8*, int);
TRKResult TRKReadBuffer_ui32(TRKBuffer*, u32*, int);
TRKResult TRKReadBuffer1_ui64(TRKBuffer*, u64*);
TRKResult TRKAppendBuffer_ui32(TRKBuffer*, const u32*, int);
TRKResult TRKAppendBuffer1_ui64(TRKBuffer*, const u64);
TRKResult TRKReadBuffer(TRKBuffer*, void*, size_t);
TRKResult TRKAppendBuffer(TRKBuffer*, const void*, size_t);
void TRKResetBuffer(TRKBuffer*, BOOL keepData);
void OutputData(void* data, int length);

TRKResult TRKMessageSend(TRK_Msg*);
void TRKSwapAndGo(void);
u32 __TRK_get_MSR(void);
void TRKSaveExtended1Block(void);
void TRK_main(void);
int InitMetroTRKCommTable(int hwId);
void EnableEXI2Interrupts(void);
TRKResult TRKWriteUARTN(const void* bytes, u32 length);
TRKResult TRKInitializeNub(void);
TRKResult TRKTerminateNub(void);
void TRKNubWelcome(void);
void TRKNubMainLoop(void);

TRKResult TRKInitializeMutex(void*);
TRKResult TRKAcquireMutex(void*);
TRKResult TRKReleaseMutex(void*);
void* TRK_memcpy(void* dst, const void* src, size_t n);

TRKResult TRKInitializeEventQueue();
TRKResult TRKInitializeMessageBuffers();
TRKResult TRKInitializeDispatcher();
void InitializeProgramEndTrap(void);
TRKResult TRKInitializeSerialHandler();
TRKResult TRKInitializeTarget();
TRKResult TRKTerminateSerialHandler(void);
void TRKProcessInput(int bufIdx);
int TRKTestForPacket(void);
int TRKPollUART(void);
int TRKReadUARTN(void* bytes, u32 length);
void TRK_board_display(char* str);

/* File I/O */
u8 TRKAccessFile(u8 command, u32 fileHandle, size_t* count, u8* buffer);
u8 TRKOpenFile(void);
u8 TRKCloseFile(void);
u8 TRKPositionFile(void);

/* Debugger communication */
typedef int (*DBCommInitFunc)(volatile u8**, void*);
typedef int (*DBCommFunc)(void);
typedef int (*DBCommReadFunc)(u8*, int);
typedef int (*DBCommWriteFunc)(const u8*, int);

/**
 * @size{0x28}
 */
typedef struct DBCommTable {
	DBCommInitFunc initialize_func;  // _00
	DBCommFunc initinterrupts_func;  // _04
	DBCommFunc shutdown_func;        // _08
	DBCommFunc peek_func;            // _0C
	DBCommReadFunc read_func;        // _10
	DBCommWriteFunc write_func;      // _14
	DBCommFunc open_func;            // _18
	DBCommFunc close_func;           // _1C
	DBCommFunc pre_continue_func;    // _20
	DBCommFunc post_stop_func;       // _24
} DBCommTable;

/* EXI2 */
void UnreserveEXI2Port(void);
void ReserveEXI2Port(void);

/* MW */
void MWTRACE(u8, char*, ...);

/* UART */
typedef int UARTError;

enum {
	kUARTNoError = 0,
	kUARTUnknownBaudRate,
	kUARTConfigurationError,
	kUARTBufferOverflow, /* specified buffer was too small */
	kUARTNoData          /* no data available from polling */
};

typedef enum {
	kBaudHWSet  = -1,  /* use HW settings such as DIP switches */
	kBaud300    = 300, /* valid baud rates */
	kBaud600    = 600,
	kBaud1200   = 1200,
	kBaud1800   = 1800,
	kBaud2000   = 2000,
	kBaud2400   = 2400,
	kBaud3600   = 3600,
	kBaud4800   = 4800,
	kBaud7200   = 7200,
	kBaud9600   = 9600,
	kBaud19200  = 19200,
	kBaud38400  = 38400,
	kBaud57600  = 57600,
	kBaud115200 = 115200,
	kBaud230400 = 230400
} UARTBaudRate;

UARTError InitializeUART(UARTBaudRate baudRate);
TRKResult TRKInitializeIntDrivenUART(unknown, unknown, unknown, void*);
void usr_put_initialize();
BOOL usr_puts_serial(const char* msg);
BOOL GetTRKConnected(void);
void SetTRKConnected(BOOL);
void TRK_flush_cache(u32 addr, u32 length);
void TRKTargetSetInputPendingPtr(void*);
extern void* gTRKInputPendingPtr;

#ifdef __cplusplus
};
#endif // ifdef __cplusplus

#endif
