#include "pch.h"

#include "Library.h"

#include <thread>
#include <queue>

unsigned int __stdcall PingThread(void* data)
{
    showMessage("Enter Thread!", "PingThread");

    EEStats* ees = static_cast<EEStats*>(data);

    while (1) {
        // Every 10min send ping to keep session alive during >30min sessions withou screen change
        Sleep(600000); // 600000

        if (ees == nullptr) {
            showMessage("Unable to recover EE Stats instance!!!", "PingThread", true);
            break;
        }

        if (!ees->sendPing()) {
            showMessage("Unable to send ping! The process or computer was probably sleeping and the session timed out!", "PingThread", true);
            break;
        }
    }
    showMessage("Exit Thread!", "PingThread"); // Will sadly never work...
    return 0;
}

// WIP Thread that allow the usage of EE core CPU, not 100% accurate but seems accurate when actual slowdown append because of the CPU
/*unsigned int __stdcall delamerde(void* data)
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
}*/

unsigned int __stdcall PeformanceThread(void* data)
{
    showMessage("Enter Thread!", "PeformanceThread");

    EEStats* ees = static_cast<EEStats*>(data);
    GameQuery* gq = ees->getGameQuery();

    bool wasPlaying = false;
    std::queue<float> fpsHistory;
    const int updateInterval = 5; // In sec
    const int minTimePlayedToSend = 10; // In sec
    std::string start = currentDateTime("%Y-%m-%d %H:%M:%S");
    
    if (doesFileExist(L"neoee.dll")) { // Worst way ever to check it lol
        showMessage("NeoEE detected, the game binary has been hardly modified ! Cancelling performance tracking to avoid crash.", "PeformanceThread");
        return 0;
    }

    while (1) {
        Sleep(1000 * updateInterval);

        if (gq == nullptr) {
            showMessage("Unable to recover GameQuery instance!!!", "PeformanceThread");
            break;
        }

        if (gq->isPlaying()) {
            wasPlaying = true;

            if (gq->isMinimized()) // Minimized FPS isn't important (seems to be cap at 15 FPS btw)
                continue;

            float fps = gq->getFPS(updateInterval);
            if (fps < 1.0f) // Skip bad data or simply potato perf
                continue;
            fpsHistory.push(fps);
            std::cout << fps << std::endl;
        }
        else if (wasPlaying) {
            showMessage("Back to menu, calculating performance history...", "PeformanceThread");

            if (fpsHistory.size() >= (int) ((float) minTimePlayedToSend / (float) updateInterval))
            {
                showMessage("Sending performance history...", "PeformanceThread");

                float moy = 0;
                int totalElem = fpsHistory.size();
                while (!fpsHistory.empty()) {
                    moy += (fpsHistory.front() / (float) totalElem);
                    fpsHistory.pop();
                }

                showMessage("Sending performance history...", "PeformanceThread");
                ees->sendPerformanceInfos((int)moy, start, currentDateTime("%Y-%m-%d %H:%M:%S"));
                showMessage("Performance history sent!", "PeformanceThread");
            }
            else {
                showMessage("Not enough data to send performance history, cleaning performance history...", "PeformanceThread");
                while (!fpsHistory.empty())
                    fpsHistory.pop();
            }

            wasPlaying = false;
        }
    }
    return 1;
}

unsigned int __stdcall ScreenThread(void* data)
{
    GameQuery* gq = static_cast<GameQuery*>(data);

    GameQuery::ScreenType lastScreen = GameQuery::ST_Unknown;

    while (1)
    {
        GameQuery::ScreenType currentScreen = gq->getCurrentScreen();
        if ((lastScreen == GameQuery::ST_Unknown && currentScreen != GameQuery::ST_Unknown)
            || lastScreen != gq->getCurrentScreen()) {
            std::cout << "changed screen" << std::endl;
            lastScreen = currentScreen;
        }

        Sleep(5000); // TODO: HOOK !!! A loop to check the screen is disastrous
    }
}

void Library::StartLibraryThread()
{
    showMessage("Enter Thread!", "LibraryThread");
    GameQuery* gq = _ees->getGameQuery();
    ComputerQuery* cq = _ees->getComputerQuery();

    cq->printInfos();

    /*
    * 
    * Let's allow VM for the moment
    * 
    if (cq->runInVM()) {
        showMessage("Sorry EE Stats isn't availaible on VM.", "LibraryThread", true);
        return;
    }
    *
    */

    if (!cq->isWine() && cq->getWindowsVersionCQ() < ComputerQuery::WinVista) {
        showMessage("Sorry EE Stats work from Windows Vista.", "LibraryThread", true);
        return;
    }

    // https://github.com/EnergyCube/RolePlay_Notes/releases/latest/download/setup.exe
    /*if (_ees->downloadFile("https://storage.empireearth.eu/EEStats.dll", "EEStats.dll.update")) {
        showMessage("Update Downloaded!", "MainThread");
        Sleep(5000);
        return 1;
    }*/

    showMessage("Product Type: " + std::to_string(gq->getProductType()), "LibraryThread");
    if (gq->getProductType() == GameQuery::PT_Unknown) {
        showMessage("Unable to identify the game product! Did you renamed the executable?", "LibraryThread", true);
        return;
    }
    else if (gq->getProductType() == GameQuery::PT_AoC) {
        showMessage("Sorry... AoC isn't supported for the moment.", "LibraryThread", true);
        return;
    }

    //gq->setVersionSuffix(" (EE Stats v1.0.0)"); // TODO: A shared lib :V (.lib ? or .dll ? idk)

    showMessage("Waiting for the game to fully load...", "LibraryThread");
    while (!gq->isLoaded()) // TODO: Hook, because it's better
        Sleep(100);
    showMessage("Game Loaded!", "LibraryThread");

    showMessage("Game Base: " + std::string(gq->getGameBaseVersion()), "LibraryThread");
    showMessage("Game Data: " + std::string(gq->getGameDataVersion()), "LibraryThread");
    if (!gq->isSupportedVersion()) {
        showMessage("Your game version isn't supported, stats about specific game feature are disabled...", "LibraryThread", true);
        //ees->askSessionId();
        return; // The ping clock will still run so we can track the session time
    }

    showMessage("Asking a session id for current session...", "LibraryThread");
    if (!_ees->askSessionId()) {
        showMessage("Unable to get a session id!", "LibraryThread");
        return;
    }
    showMessage("Session id for this session is " + _ees->getSessionId(), "LibraryThread");

    showMessage("Sending current session infos...", "LibraryThread");
    if (!_ees->sendSessionInfos()) {
        showMessage("Unable to send session infos!", "LibraryThread");
        return;
    }
    showMessage("Session infos sent sucessfully...", "LibraryThread");

    HANDLE pingThreadHandle = (HANDLE) _beginthreadex(0, 0, PingThread, _ees, 0, 0);
    HANDLE perfThreadHandle = (HANDLE) _beginthreadex(0, 0, PeformanceThread, _ees, 0, 0);
    HANDLE screenThreadHandle = (HANDLE) _beginthreadex(0, 0, ScreenThread, gq, 0, 0);

    showMessage("Exit Thread!", "LibraryThread");
}