// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "Library.h"
#include "MemoryHelper.h"
#include "GameQuery.h"

#include <iostream>
#include <thread>

static Library* lib = nullptr;

bool __stdcall Attach(HMODULE hModule)
{

#ifdef _DEBUG
    if (!GetConsoleWindow()) // Already Allocated
        AllocConsole();
    freopen_s(&f, "CONOUT$", "w", stdout);
#else
    FILE* f;
    // WARN: cURL verbose need to be DISABLED !
    //       Or the curl_easy_perform() will crash...
    //       I have no idea of how to fix that :|
    freopen_s(&f, "EEStats.log", "a", stdout);
#endif // DEBUG


    showMessage("Attach!", "DllMain");

    showMessage("Init Library...", "DllMain");

    if (lib) {
        showMessage("Library Already Initialized...", "DllMain");
        return false;
    }

    auto initLamda = [](void* data) -> unsigned int __stdcall
    {
        HMODULE* si = static_cast<HMODULE *>(data);
        lib = new Library(*si);
        showMessage("Library Initialized!", "DllMain");
        lib->PrintCredits();
        lib->UpdateAttachRoutine();
        lib->StartLibraryThread();
        return 0;
    };

    showMessage("Starting Library Thread!", "DllMain");
    HANDLE initThreadHandle = (HANDLE)_beginthreadex(0, 0, initLamda, &hModule, 0, 0);

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
