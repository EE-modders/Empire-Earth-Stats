// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "Library.h"
#include "MemoryHelper.h"
#include "GameQuery.h"

#include <iostream>
#include <thread>

bool __stdcall Attach(void)
{
    DWORD Current_Game_ProcessID = GetCurrentProcessId();
    std::wstring MutexString = L"EEStatsMutex_" + std::to_wstring(Current_Game_ProcessID);
    HANDLE handleMutex = CreateMutexW(NULL, TRUE, MutexString.c_str());

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return false;
    }

    if (!GetConsoleWindow()) // Already Allocated
        AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);

    showMessage("Attach!", "DllMain");

    auto initLamda = [](void*) -> unsigned int __stdcall
    {
        if (Library::InitLibrary())
            Library::getLib()->StartLibraryThread();
        return 0;
    };

    HANDLE initThreadHandle = (HANDLE)_beginthreadex(0, 0, initLamda, 0, 0, 0);
    return true;
}

// Be really REALLY fast here, the game don't fk care if it take more than <random time...> it just kill it
bool __stdcall Detach(void)
{
    showMessage("Detach!", "DllMain");
    if (Library::DestroyLibrary()) {
        if (GetConsoleWindow())
            FreeConsole();
#ifdef _DEBUG
        Sleep(5000); // Keep the console open for 5s (if not killed before) to read output if required
#endif
        return true;
    }
    return false;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH: {
            return Attach();
        }
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH: {
            return Detach();
        }
    }
    return TRUE;
}
