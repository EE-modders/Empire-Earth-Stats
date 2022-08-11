#include "pch.h"

#include "MemoryHelper.h"

/* update memory protection and read with memcpy */
void protectedRead(void* dest, void* src, int n) {
    DWORD oldProtect = 0;
    VirtualProtect(dest, n, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dest, src, n);
    VirtualProtect(dest, n, oldProtect, &oldProtect);
}

/* read from address into read buffer of length len */
bool readBytes(void* read_addr, void* read_buffer, int len) {
    // compile with "/EHa" to make this work
    // see https://stackoverflow.com/questions/16612444/catch-a-memory-access-violation-in-c
    try {
        protectedRead(read_buffer, read_addr, len);
        return true;
    }
    catch (...) {
        return false;
    }
}

/* write patch of length len to destination address */
void writeBytes(void* dest_addr, void* patch, int len) {
    protectedRead(dest_addr, patch, len);
}

/* fiddle around with the pointers */
HMODULE getBaseAddress() {
    return GetModuleHandle(NULL);
}

DWORD* calcAddress(DWORD appl_addr) {
    return (DWORD*)((DWORD)getBaseAddress() + appl_addr);
}

DWORD* tracePointer(memoryPTR* patch) {
    DWORD* location = calcAddress(patch->base_address);

    for (int n : patch->offsets) {
        location = (DWORD*)(*location + n);
    }

    return location;
}

void nopper(void* startAddr, int len) {
    BYTE nop = 0x90;
    for (int i = 0; i < len; i++)
        writeBytes((DWORD*)((DWORD)startAddr + i), &nop, 1);
}

bool functionInjector(void* hookAddr, void* function, DWORD& returnAddr, int len) {
    if (len < 5)
        return false;

    BYTE jmp = 0xE9;
    DWORD relAddr = ((DWORD)function - (DWORD)hookAddr) - 5;
    returnAddr = (DWORD)hookAddr + len;

    nopper(hookAddr, len);

    writeBytes(hookAddr, &jmp, 1);
    writeBytes((DWORD*)((DWORD)hookAddr + 1), &relAddr, 4);

    return true;
}