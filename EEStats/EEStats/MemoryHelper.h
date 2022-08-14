#pragma once

#include <vector>

struct memoryPTR {
    DWORD base_address;
    std::vector<int> offsets;
};

void protectedRead(void* dest, void* src, int n);
bool readBytes(void* read_addr, void* read_buffer, int len);
void writeBytes(void* dest_addr, void* patch, int len);

HMODULE getBaseAddress();

DWORD* calcAddress(DWORD appl_addr);
DWORD* tracePointer(memoryPTR* patch);

void nopper(void* startAddr, int len);
bool functionInjector(void* hookAddr, void* function, DWORD& returnAddr, int len);
