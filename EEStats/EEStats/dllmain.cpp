// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "Utils.h"
#include "Library.h"
#include "MemoryHelper.h"
#include "GameQuery.h"

#include <iostream>
#include <thread>

static Library* lib = nullptr;

bool __stdcall Attach(HMODULE hModule)
{
    // Instant create mutex to avoid multiple load if the DLL stop for some reasons during game startup
    HANDLE handleMutex = CreateMutex(NULL, TRUE, (L"EEStats_" + std::to_wstring(GetCurrentProcessId())).c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return false;

    FILE* f;

#ifdef _DEBUG
    if (!GetConsoleWindow()) // Already Allocated
        AllocConsole();
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr); // Not required technically, but required for cURL verbose
#else
    // Delete log after a given size
    if (doesFileExist(L"EEStats.log") && fileSize(L"EEStats.log") > 2 /*Mo*/ * 100 * 100 * 100)
        DeleteFile(L"EEStats.log");
    freopen_s(&f, "EEStats.log", "a", stdout);
    // crash if anything use it ! freopen_s(&f, "EEStats.log", "a", stderr);
#endif // DEBUG

    showMessage("Attach!", "DllMain");

    showMessage("Init Library...", "DllMain");

    if (lib) {
        showMessage("Library Already Initialized...", "DllMain");
        return false;
    }

    auto initLamda = [](void* data) -> unsigned int __stdcall
    {
        try {
            lib = new Library();
        }
        catch (std::exception ex) {
            showMessage(std::string(ex.what()), "DllMain", 1);
            return 0;
        }
        showMessage("Library Initialized!", "DllMain");
        lib->PrintCredits();
        lib->UpdateAttachRoutine();
        lib->StartLibraryThread();
        return 1;
    };

    showMessage("Starting Library Thread!", "DllMain");
    HANDLE initThreadHandle = (HANDLE)_beginthreadex(0, 0, initLamda, 0, 0, 0);

    return true;
}

// Be really REALLY fast here, the game don't fk care if it take more than <random time...> it just kill it
bool __stdcall Detach()
{
    showMessage("Detach!", "DllMain");
    if (lib != nullptr) {
        lib->UpdateDetachRoutine();
        delete lib;
        if (GetConsoleWindow())
            FreeConsole();
#ifdef _DEBUG
        // Sleep(5000); // Keep the console open for 5s (if not killed before) to read output if required
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
            return Attach(hModule);
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
