// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "Utils.h"
#include "Library.h"
#include "MemoryHelper.h"
#include "GameQuery.h"

#include <iostream>
#include <thread>

static Library* lib = nullptr;

bool __stdcall Attach()
{
    if (GameQuery().getProductType() == GameQuery::PT_Unknown)
        return true; // Loaded by another program than the game, no need to do anything.
    std::wstring dllPath = getDllPathUtils();
    std::wstring dllName = getFileName(dllPath, false);

    // Instant create mutex to avoid multiple load if the DLL stop for some reasons during game startup
    HANDLE handleMutex = CreateMutex(NULL, TRUE, (dllName + L"_" + std::to_wstring(GetCurrentProcessId())).c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return false;

    Logger::init(dllName);

    Logger::showMessage("Attach!", "DllMain");

    Logger::showMessage("Init Library...", "DllMain");
    if (lib) {
        Logger::showMessage("Library Already Initialized...", "DllMain");
        return false;
    }

    auto initLamda = [](void* data) -> unsigned int
    {
        try {
            lib = new Library();
            if (lib == nullptr)
                throw std::exception("Unable to init Library!");
        }
        catch (std::exception ex) {
            Logger::showMessage("Library Init Error: " + std::string(ex.what()), "DllMain", 1);
            return 0;
        }

        Logger::showMessage("Library Initialized!", "DllMain");

        lib->PrintCredits();
        lib->UpdateAttachRoutine();
        lib->StartLibraryThread();
        return 1;
    };

    Logger::showMessage("Starting Library Thread!", "DllMain");
    HANDLE initThreadHandle = (HANDLE)_beginthreadex(0, 0, initLamda, 0, 0, 0);

    return true;
}

// Be really REALLY fast here, the game don't fk care if it take more than <random time...> it just kill it
bool __stdcall Detach()
{
    if (GameQuery().getProductType() == GameQuery::PT_Unknown)
        return true; // Loaded by another program than the game, no need to do anything.

    Logger::showMessage("Detach!", "DllMain");
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
    return true;
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
