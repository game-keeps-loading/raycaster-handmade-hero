#pragma once
#include"ray.h"

// function to get CPU
typedef uint32_t u32;

internal u64 
LockedADDAndReturnPreviousValue(u64 volatile *Value, u64 Addend) {

    u64 Result = InterlockedExchangeAdd64((volatile LONG64 *)Value, Addend);
    return Result;
}

internal DWORD WINAPI 
WorkerThread(void *IpParameter) {
    work_queue *Queue = (work_queue *)IpParameter;
    while(renderTile(Queue)) {};
    return 0;
}

internal void
CreateWorkThread(void *Parameter) {
    DWORD ThreadId;
    HANDLE ThreadHandle =  CreateThread(0, 0, WorkerThread, Parameter, 0, &ThreadId);
    CloseHandle(ThreadHandle);
}