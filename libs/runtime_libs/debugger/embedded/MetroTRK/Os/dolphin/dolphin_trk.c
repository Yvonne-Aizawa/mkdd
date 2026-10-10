#include "PowerPC_EABI_Support/MetroTRK/trk.h"

void OSResetSystem(BOOL reset, u32 resetCode, BOOL forceMenu);

extern u8 gTRKInterruptVectorTable[];
extern u8 _db_stack_addr[];

#define EXCEPTIONMASK_ADDR 0x44

static u32 lc_base;

static u32 TRK_ISR_OFFSETS[15] = {
    0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700, 0x800,
    0x900, 0xC00, 0xD00, 0xF00, 0x1300, 0x1400, 0x1700,
};

__declspec(section ".init") void __TRK_reset(void) { OSResetSystem(FALSE, 0, FALSE); }

asm void InitMetroTRK(void)
{
#ifdef __MWERKS__ // clang-format off
    nofralloc
    addi r1, r1, -4
    stw r3, 0(r1)
    lis r3, gTRKCPUState@h
    ori r3, r3, gTRKCPUState@l
    stmw r0, ProcessorState_PPC.Default.GPR(r3)
    lwz r4, 0(r1)
    addi r1, r1, 4
    stw r1, ProcessorState_PPC.Default.GPR[1](r3)
    stw r4, ProcessorState_PPC.Default.GPR[3](r3)
    mflr r4
    stw r4, ProcessorState_PPC.Default.LR(r3)
    stw r4, ProcessorState_PPC.Default.PC(r3)
    mfcr r4
    stw r4, ProcessorState_PPC.Default.CR(r3)
    mfmsr r4
    ori r3, r4, 0x8000
    xori r3, r3, 0x8000
    mtmsr r3
    mtsrr1 r4
    bl TRKSaveExtended1Block
    lis r3, gTRKCPUState@h
    ori r3, r3, gTRKCPUState@l
    lmw r0, ProcessorState_PPC.Default.GPR(r3)
    li r0, 0
    mtspr 0x3F2, r0
    mtspr 0x3F5, r0
    lis r1, _db_stack_addr@h
    ori r1, r1, _db_stack_addr@l
    mr r3, r5
    bl InitMetroTRKCommTable
    cmpwi r3, 1
    bne initCommTableSuccess
    lwz r4, ProcessorState_PPC.Default.LR(r3)
    mtlr r4
    lmw r0, ProcessorState_PPC.Default.GPR(r3)
    blr
initCommTableSuccess:
    b TRK_main
    blr
#endif // clang-format on
}

asm void InitMetroTRK_BBA(void)
{
#ifdef __MWERKS__ // clang-format off
    nofralloc
    addi r1, r1, -4
    stw r3, 0(r1)
    lis r3, gTRKCPUState@h
    ori r3, r3, gTRKCPUState@l
    stmw r0, ProcessorState_PPC.Default.GPR(r3)
    lwz r4, 0(r1)
    addi r1, r1, 4
    stw r1, ProcessorState_PPC.Default.GPR[1](r3)
    stw r4, ProcessorState_PPC.Default.GPR[3](r3)
    mflr r4
    stw r4, ProcessorState_PPC.Default.LR(r3)
    stw r4, ProcessorState_PPC.Default.PC(r3)
    mfcr r4
    stw r4, ProcessorState_PPC.Default.CR(r3)
    mfmsr r4
    ori r3, r4, 0x8000
    mtmsr r3
    mtsrr1 r4
    bl TRKSaveExtended1Block
    lis r3, gTRKCPUState@h
    ori r3, r3, gTRKCPUState@l
    lmw r0, ProcessorState_PPC.Default.GPR(r3)
    li r0, 0
    mtspr 0x3F2, r0
    mtspr 0x3F5, r0
    lis r1, _db_stack_addr@h
    ori r1, r1, _db_stack_addr@l
    li r3, 2
    bl InitMetroTRKCommTable
    cmpwi r3, 1
    bne initCommTableSuccess
    lwz r4, ProcessorState_PPC.Default.LR(r3)
    mtlr r4
    lmw r0, ProcessorState_PPC.Default.GPR(r3)
    blr
initCommTableSuccess:
    b TRK_main
    blr
#endif // clang-format on
}

void EnableMetroTRKInterrupts(void) { EnableEXI2Interrupts(); }

u32 TRKTargetTranslate(u32 addr)
{
    if (addr >= lc_base) {
        if (addr < lc_base + 0x4000 && (gTRKCPUState.Extended1.DBAT3U & 3) != 0) {
            return addr;
        }
    }

    if (addr >= 0x7E000000 && addr <= 0x80000000) {
        return addr;
    }

    return addr & 0x3FFFFFFF | 0x80000000;
}

void __TRK_copy_vectors(void)
{
    int i;
    u32 mask;

    mask = *(u32*)TRKTargetTranslate(EXCEPTIONMASK_ADDR);

    for (i = 0; i <= 14; ++i) {
        if ((mask & (1 << i)) && i != 4) {
            u32 offset = TRK_ISR_OFFSETS[i];
            u32 dst = TRKTargetTranslate(offset);
            TRK_memcpy((void*)dst, gTRKInterruptVectorTable + offset, 0x100);
            TRK_flush_cache(dst, 0x100);
        }
    }
}

TRKResult TRKInitializeTarget(void)
{
    gTRKState.mIsStopped = TRUE;
    gTRKState._8C = __TRK_get_MSR();
    lc_base = 0xE0000000;
    return TRKSuccess;
}
