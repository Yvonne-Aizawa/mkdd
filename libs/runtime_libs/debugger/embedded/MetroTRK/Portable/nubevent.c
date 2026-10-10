#include "PowerPC_EABI_Support/MetroTRK/trk.h"

TRKEventQueue gTRKEventQueue;

TRKResult TRKInitializeEventQueue(void)
{
    TRKInitializeMutex(&gTRKEventQueue);
    TRKAcquireMutex(&gTRKEventQueue);
    gTRKEventQueue.mCurrEvtID = 0;
    gTRKEventQueue.mNextSlotToOverwrite = 0;
    gTRKEventQueue._24 = 0x100;
    TRKReleaseMutex(&gTRKEventQueue);
    return TRKSuccess;
}

BOOL TRKGetNextEvent(TRKEvent* event)
{
    BOOL result = FALSE;

    TRKAcquireMutex(&gTRKEventQueue);

    if (0 < gTRKEventQueue.mCurrEvtID) {
        TRK_memcpy(event, &gTRKEventQueue.mEvents[gTRKEventQueue.mNextSlotToOverwrite], sizeof(TRKEvent));
        gTRKEventQueue.mCurrEvtID--;

        if (++gTRKEventQueue.mNextSlotToOverwrite == 2) {
            gTRKEventQueue.mNextSlotToOverwrite = 0;
        }

        result = TRUE;
    }

    TRKReleaseMutex(&gTRKEventQueue);
    return result;
}

TRKResult TRKPostEvent(TRKEvent* event)
{
    TRKResult result = TRKSuccess;
    int nextEventID;

    TRKAcquireMutex(&gTRKEventQueue);

    if (gTRKEventQueue.mCurrEvtID == 2) {
        result = TRKError100;
    } else {
        nextEventID = (gTRKEventQueue.mNextSlotToOverwrite + gTRKEventQueue.mCurrEvtID) % 2;
        TRK_memcpy(&gTRKEventQueue.mEvents[nextEventID], event, sizeof(TRKEvent));
        gTRKEventQueue.mEvents[nextEventID]._04 = gTRKEventQueue._24;

        if (++gTRKEventQueue._24 < 0x100) {
            gTRKEventQueue._24 = 0x100;
        }

        gTRKEventQueue.mCurrEvtID++;
    }

    TRKReleaseMutex(&gTRKEventQueue);
    return result;
}

void TRKConstructEvent(TRKEvent* event, int eventType)
{
    event->mEventType = eventType;
    event->_04 = 0;
    event->mBufferIndex = -1;
}

void TRKDestructEvent(TRKEvent* event) { TRKReleaseBuffer(event->mBufferIndex); }
