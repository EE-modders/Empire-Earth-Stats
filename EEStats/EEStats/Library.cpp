#include "pch.h"

#include "Library.h"

#include <thread>

unsigned int __stdcall PingThread(void* data)
{
    showMessage("Enter Thread!", "PingThread");
    while (1) {
        // Every 10min send ping to keep session alive during >30min sessions withou screen change
        Sleep(600000); //600000
        if (Library::getLib() != nullptr && Library::getLib()->getEES() != nullptr)
            Library::getLib()->getEES()->sendPing();
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

bool Library::StartLibraryThread()
{
    showMessage("Enter Thread!", "MainThread");
    EEStats* ees = Library::getLib()->getEES();
    GameQuery* gq = ees->getGameQuery();

    showMessage("Product Type: " + std::to_string(gq->getProductType()), "MainThread");
    if (gq->getProductType() == GameQuery::PT_Unknown) {
        showMessage("Unable to identify the game product! Did you renamed the executable?", "MainThread", true);
        return false;
    }
    else if (gq->getProductType() == GameQuery::PT_AoC) {
        showMessage("Sorry... AoC isn't supported for the moment.", "MainThread", true);
        return false;
    }

    gq->setVersionSuffix(" (EE Stats v1.0.0)"); // TODO: A shared lib :V (.lib ? or .dll ? idk)

    showMessage("Waiting for the game to fully load...", "MainThread");
    while (!gq->isLoaded()) // TODO: Hook, because it's better
        Sleep(100);
    showMessage("Game Loaded!", "MainThread");

    showMessage(gq->getGameVersion(), "MainThread");
    if (!gq->isSupportedVersion()) {
        showMessage("Your game version isn't supported, stats about specific game feature are disabled...", "MainThread", true);
        //ees->askSessionId();
        return false; // The ping clock will still run so we can track the session time
    }

    showMessage("Asking a session id for current session...", "MainThread");
    if (!ees->askSessionId()) {
        showMessage("Unable to get a session id!", "MainThread");
        return false;
    }
    showMessage("Session id for this session is " + ees->getSessionId(), "MainThread");

    showMessage("Sending current session infos...", "MainThread");
    if (!ees->sendSessionInfos()) {
        showMessage("Unable to send session infos!", "MainThread");
        return false;
    }
    showMessage("Session infos sent sucessfully...", "MainThread");

    HANDLE pingThreadHandle = (HANDLE)_beginthreadex(0, 0, PingThread, &ees, 0, 0);
    HANDLE perfThreadHandle = (HANDLE)_beginthreadex(0, 0, delamerde, 0, 0, 0);

    GameQuery::ScreenType lastScreen = GameQuery::ST_Unknown;

    while (1) // could also be lib != nullptr but when it happend (detach), the thread is literally killed so it's useless...
    {
        GameQuery::ScreenType currentScreen = gq->getCurrentScreen();
        if ((lastScreen == GameQuery::ST_Unknown && currentScreen != GameQuery::ST_Unknown)
            || lastScreen != gq->getCurrentScreen()) {
            // SEND SCREEN
            lastScreen = currentScreen;
        }

        Sleep(5000); // Every 5s TODO: HOOK !!! A loop to check the screen is disastrous
    }
    showMessage("Exit Thread!", "MainThread"); // Will sadly never work...
    return 0;
}