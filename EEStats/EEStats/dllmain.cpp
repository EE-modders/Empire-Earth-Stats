// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "Library.h"
#include "MemoryHelper.h"
#include "GameQuery.h"

#include <iostream>
#include <thread>
#include <future>
#include <map>

int lastAllowedPingThread = 0;

unsigned int __stdcall PingThread(void* data)
{
    showMessage("Enter Thread!", "PingThread");
    while (1) {
        // Every 10min send ping to keep session alive during >30min sessions withou screen change
        Sleep(600000); //600000
        if (Library::getLib() != nullptr && Library::getLib()->getMatomo() != nullptr)
            Library::getLib()->getMatomo()->sendPing();
        else
            showMessage("Unable to recover Matomo or Lib instance!!!", "PingThread", true);

    }
    showMessage("Exit Thread!", "PingThread"); // Will sadly never work...

    /* Previous code to keep session alive before zocker making me realize that the screen update actually = ping :<
    while (1) {
        if (total_ms < 60000) {
            Sleep(10000);               // Session < 1m: ping every 10s
            total_ms += 10000;
        }
        else if (total_ms < 300000) {
            Sleep(30000);               // Session < 5m && > 1m: ping every 30s
            total_ms += 30000;
        }
        else if (total_ms < 1800000) {
            Sleep(150000);              // Session < 30m && > 5m: ping every 2m30
            total_ms += 150000;
        }
        else if (total_ms < 3600000) {
            Sleep(300000);              // Session < 1h && > 30m: ping every 5m
            total_ms += 300000;
        }
        else {
            Sleep(600000);             // Session > 1h: ping every 10m
            total_ms += 500000;
        }
        matomo->sendPing();
    }*/
}

// WIP Thread that allow the usage of EE core CPU, not 100% accurate but seems accurate when actual slowdown append because of the CPU
unsigned int __stdcall delamerde(void* data)
{

    FILETIME createTime, exitTime, kernelTime, userTime;

    typedef unsigned long long int uint64;
    uint64 lOldKernel = 0, lOldUser = 0, lOldCreate = 0, lOldCurrent = 0;

    Sleep(5000);

    while (1)
    {
        int bStatus = GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime);
        
        if (bStatus)
        {
            uint64 lKernel = (uint64(kernelTime.dwHighDateTime) << 32) | uint64(kernelTime.dwLowDateTime);
            uint64 lUser = (uint64(userTime.dwHighDateTime) << 32) | uint64(userTime.dwLowDateTime);
            uint64 lCreate = (uint64(createTime.dwHighDateTime) << 32) | uint64(createTime.dwLowDateTime);

            SYSTEMTIME lCurrentSystemTime;
            FILETIME lCurrentFileTime;
            GetSystemTime(&lCurrentSystemTime);
            SystemTimeToFileTime(&lCurrentSystemTime, &lCurrentFileTime);

            uint64 lCurrent = (uint64(lCurrentFileTime.dwHighDateTime) << 32) | uint64(lCurrentFileTime.dwLowDateTime);

            double lCpu = 100.0 * double(lKernel - lOldKernel + lUser - lOldUser) / double(lCurrent - lOldCurrent);

            lOldKernel = lKernel;
            lOldUser = lUser;
            lOldCreate = lCreate;
            lOldCurrent = lCurrent;

            if (lOldCurrent != 0) {
                if (lCpu > 90) {
                    std::cout << "CPU: " << lCpu << "%" << std::endl;
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
                    std::cout << "WARNING: The CPU core where EE run seems overloaded !!" << std::endl;
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
                }
                else if (lCpu > 80) {
                    std::cout << "CPU: " << lCpu << "%" << std::endl;
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
                    std::cout << "WARNING: The CPU core where EE run seems slow !!" << std::endl;
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
                }
            }

            Sleep(1000); // Does it work if we set more ? Need investigation.
        }
        else {
            std::cout << "Status seems invalid !" << std::endl;
            Sleep(1000); // avoid spam
        }
    }
}

unsigned int __stdcall MainThread(void* data)
{
    showMessage("Enter Thread!", "MainThread");
    MatomoEE* matomo = Library::getLib()->getMatomo();
    GameQuery gq("");

    gq.setVersionSuffix(" (EE Stats v1.0.0)"); // TODO: A shared lib :V (.lib ? or .dll ? idk)

    showMessage("Waiting for the game to fully load...", "MainThread");
    while (!gq.isLoaded()) // TODO: Hook, because it's better
        Sleep(100);
    showMessage("Game Loaded!", "MainThread");

    showMessage(gq.getGameVersion(), "MainThread");
    if (!gq.isSupportedVersion()) {
        showMessage("Your game version isn't supported, stats about specific game feature are disabled...", "MainThread", true);
        matomo->sendScreen(GameQuery::ScreenType::Unknown);
        return 0; // The ping clock will still run so we can track the session time
    }

    HANDLE pingThreadHandle = (HANDLE)_beginthreadex(0, 0, PingThread, &matomo, 0, 0);
    HANDLE perfThreadHandle = (HANDLE)_beginthreadex(0, 0, delamerde, 0, 0, 0);

    GameQuery::ScreenType lastScreen = GameQuery::ScreenType::Unknown;

    while (1) // could also be lib != nullptr but when it happend (detach), the thread is literally killed so it's useless...
    {
        GameQuery::ScreenType currentScreen = gq.getCurrentScreen();
        if ((lastScreen == GameQuery::ScreenType::Unknown && currentScreen != GameQuery::ScreenType::Unknown)
            || lastScreen != gq.getCurrentScreen()) {

            auto success = std::async(std::launch::async,
                [matomo, currentScreen]() {
                    matomo->sendScreen(currentScreen);
                });
            
            lastScreen = currentScreen;
        }
        Sleep(5000); // Every 5s TODO: HOOK !!! A loop to check the screen is disastrous
    }
    showMessage("Exit Thread!", "MainThread"); // Will sadly never work...
    return 0;
}

bool __stdcall Attach(void)
{
    FILE* f;
    AllocConsole();
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);

    showMessage("Attach!", "DllMain");

    DWORD Current_Game_ProcessID = GetCurrentProcessId();
    std::wstring MutexString = L"EEStatsMutex_" + std::to_wstring(Current_Game_ProcessID);
    HANDLE handleMutex = CreateMutexW(NULL, TRUE, MutexString.c_str());

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return false;
    }
    if (Library::InitLibrary()) {
        HANDLE mainThreadHandle = (HANDLE)_beginthreadex(0, 0, MainThread, 0, 0, 0);
        return true;
    }
    return false;
}

bool __stdcall Detach(void)
{
    // Note: Be really REALLY fast here, the game don't fk care if it take more than <random time...> it just kill it
    showMessage("Detach!", "DllMain");
    if (Library::DestroyLibrary()) {
        FreeConsole();
        Sleep(5000);
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
        case DLL_THREAD_ATTACH: // Disabled
            break;
        case DLL_THREAD_DETACH: // Disabled
            break;
        case DLL_PROCESS_DETACH: {
            return Detach();
        }
    }
    return TRUE;
}
