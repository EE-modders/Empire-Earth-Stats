#include "pch.h"

#include "Library.h"

#include <thread>
#include <queue>
#include <process.h>

unsigned int __stdcall PingThread(void* data)
{
    Logger::showMessage("Enter Thread!", "PingThread");

    EEStats* ees = static_cast<EEStats*>(data);

    if (ees == nullptr) {
        Logger::showMessage("Unable to recover EEStats instance!!!", "PingThread", true);
        return 0;
    }

    while (1) {
        // Every 10min send ping to keep session alive during >30min sessions withou screen change
        Sleep(600000); // 600000

        if (!ees->sendPing()) {
            Logger::showMessage("Unable to send ping! The process or computer was probably sleeping and the session timed out!", "PingThread", true);
            break;
        }
        else {
            Logger::showMessage("Ping sent!", "PingThread");
        }
    }
    Logger::showMessage("Exit Thread!", "PingThread"); // Will sadly never work...
    return 1;
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
    Logger::showMessage("Enter Thread!", "PeformanceThread");

    EEStats* ees = static_cast<EEStats*>(data);
    GameQuery* gq = ees->getGameQuery();

    bool wasPlaying = false;
    std::queue<float> fpsHistory;
    const int updateInterval = 10; // In sec
    const int minTimePlayedToSend = 600; // In sec
    std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();
    
    if (doesFileExist(L"neoee.dll")) { // Worst way ever to check it lol
        Logger::showMessage("NeoEE detected, the game binary has been hardly modified ! Cancelling performance tracking to avoid crash.", "PeformanceThread");
        return 0;
    }

    if (ees == nullptr || gq == nullptr) {
        Logger::showMessage("Unable to recover EEStats or GameQuery instance!!!", "PeformanceThread", true);
        return 0;
    }

    while (1) {
        Sleep(1000 * updateInterval);

        if (gq->isPlaying()) {
            if (!wasPlaying) { // New or first game, reset clock
                t_start = std::chrono::high_resolution_clock::now();
                wasPlaying = true;
            }

            if (gq->isMinimized()) // Minimized FPS isn't important (seems to be cap at 15 FPS btw)
                continue;

            float fps = gq->getFPS(updateInterval);
            if (fps < 1.0f) // Skip bad data or simply potato perf
                continue;
            fpsHistory.push(fps);
        }
        else if (wasPlaying) {
            Logger::showMessage("Back to menu, calculating performance history...", "PeformanceThread");

            wasPlaying = false;
            if (fpsHistory.size() >= (int) ((float) minTimePlayedToSend / (float) updateInterval))
            {
                Logger::showMessage("Sending performance history...", "PeformanceThread");

                float moy = 0;
                int totalElem = fpsHistory.size();
                while (!fpsHistory.empty()) {
                    moy += (fpsHistory.front() / (float) totalElem);
                    fpsHistory.pop();
                }

                Logger::showMessage("Sending performance history...", "PeformanceThread");

                std::stringstream ss;
                auto timePlayed = std::chrono::high_resolution_clock::now() - t_start;
                auto hours = std::chrono::duration_cast<std::chrono::hours>(timePlayed);
                timePlayed -= hours;
                auto minutes = std::chrono::duration_cast<std::chrono::minutes>(timePlayed);
                timePlayed -= minutes;
                auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timePlayed);
                timePlayed -= seconds;

                ss << hours.count() << ":" << minutes.count() << ":" << seconds.count();

                if (ees->sendPerformanceInfos((int)moy, ss.str()))
                    Logger::showMessage("Performance history sent!", "PeformanceThread");
                else
                    Logger::showMessage("Failed to send performance history!", "PeformanceThread", true);
            }
            else {
                Logger::showMessage("Not enough data to send performance history, cleaning performance history...", "PeformanceThread");
                while (!fpsHistory.empty())
                    fpsHistory.pop();
            }
        }
    }
    Logger::showMessage("Exit Thread!", "PeformanceThread"); // Will sadly never work...
    return 1;
}

unsigned int __stdcall ActivityThread(void* data)
{
    EEStats* ees = static_cast<EEStats*>(data);
    GameQuery* gq = ees->getGameQuery();

    std::chrono::steady_clock::time_point t_start = std::chrono::high_resolution_clock::now();

    GameQuery::ScreenType lastScreen = GameQuery::ST_Unknown;

    if (ees == nullptr || gq == nullptr) {
        Logger::showMessage("Unable to recover EEStats or GameQuery instance!!!", "ActivityThread", true);
        return 0;
    }

    while (1)
    {
        GameQuery::ScreenType currentScreen = gq->getCurrentScreen();

        if ((lastScreen == GameQuery::ST_Unknown && currentScreen != GameQuery::ST_Unknown) || lastScreen != gq->getCurrentScreen()) {

            Logger::showMessage("Converting Screen to EEStats Screen !", "ActivityThread");
            EEStats::ScreenType eesScreenType = EEStats::ScreenType::EES_ST_Unknown;
            switch (lastScreen) {
            case GameQuery::ST_Menu:
                eesScreenType = EEStats::ScreenType::EES_ST_Menu;
                break;
            case GameQuery::ST_Lobby:
                eesScreenType = EEStats::ScreenType::EES_ST_Lobby;
                break;
            case GameQuery::ST_PlayingOnline:
                eesScreenType = EEStats::ScreenType::EES_ST_InGame_Multiplayer;
                break;
            case GameQuery::ST_PlayingSolo:
                eesScreenType = EEStats::ScreenType::EES_ST_InGame_Singleplayer;
                break;
            case GameQuery::ST_ScenarioEditor:
                eesScreenType = EEStats::ScreenType::EES_ST_Scenario_Editor;
                break;
            case GameQuery::ST_Unknown:
            default:
                eesScreenType = EEStats::ScreenType::EES_ST_Unknown;
                break;
            }

            if (eesScreenType != EEStats::ScreenType::EES_ST_Unknown) {
                Logger::showMessage("Screen changed ! Preparing activity infos...", "ActivityThread");
                std::stringstream ss;
                auto timePlayed = std::chrono::high_resolution_clock::now() - t_start;
                auto hours = std::chrono::duration_cast<std::chrono::hours>(timePlayed);
                timePlayed -= hours;
                auto minutes = std::chrono::duration_cast<std::chrono::minutes>(timePlayed);
                timePlayed -= minutes;
                auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timePlayed);
                timePlayed -= seconds;
                ss << hours.count() << ":" << minutes.count() << ":" << seconds.count();

                Logger::showMessage("Sending activity infos...", "ActivityThread");
                if (ees->sendActivity(eesScreenType, ss.str()))
                    Logger::showMessage("Activity infos sent!", "ActivityThread");
                else
                    Logger::showMessage("Failed to send activity infos!", "ActivityThread", true);
            }
            lastScreen = currentScreen;
            t_start = std::chrono::high_resolution_clock::now();
        }

        Sleep(5000); // TODO: HOOK !!! A loop to check the screen is disastrous
    }
    Logger::showMessage("Exit Thread!", "ActivityThread"); // Will sadly never work...
    return 1;
}

void Library::StartLibraryThread()
{
    if (_ees == nullptr) {
        Logger::showMessage("Unable to recover EEStats instance!!!", "LibraryThread", true);
        return;
    }

    GameQuery* gq = _ees->getGameQuery();
    ComputerQuery* cq = _ees->getComputerQuery();

    Logger::showMessage("Checking if server is reachable...", "LibraryThread");
    if (!_ees->isReachable()) {
        Logger::showMessage("Unable to reach the server!", "LibraryThread", true);
        return;
    }

    Logger::showMessage("Checking update...", "LibraryThread");
    if (!_ees->isUpToDate()) {
        Logger::showMessage("Update found! Downloading update...", "LibraryThread");
        if (_ees->downloadUpdate(getDllPath())) {
            Logger::showMessage("Update downloaded! Waiting for game restart...", "LibraryThread");
        }
        else {
            Logger::showMessage("Unable to download update, exiting...", "LibraryThread", true);
        }
        return;
    }

    if (gq == nullptr || cq == nullptr) {
        Logger::showMessage("Unable to recover GameQuery or ComputerQuery instance!!!", "LibraryThread", true);
        return;
    }

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
        Logger::showMessage("Sorry EE Stats work from Windows Vista.", "LibraryThread", true); // I mean technically cURL seems to require Vista
        return;
    }

    Logger::showMessage("Product Type: " + std::to_string(gq->getProductType()), "LibraryThread");
    if (gq->getProductType() == GameQuery::PT_Unknown) {
        Logger::showMessage("Unable to identify the game product! Did you renamed the executable?", "LibraryThread", true);
        return;
    }
    else if (gq->getProductType() == GameQuery::PT_AoC) {
        Logger::showMessage("Sorry... AoC isn't supported for the moment.", "LibraryThread", true);
        return;
    }

    Logger::showMessage("Game Base: " + std::string(gq->getGameBaseVersion()), "LibraryThread");
    if (!gq->isSupportedVersion()) {
        Logger::showMessage("Your game version isn't supported...", "LibraryThread", true);
        return;
    }
    // Register hook before game load, to show it af first load
    gq->setVersionSuffix(" (EE Stats v" + EES_VERSION_STR + ")"); // TODO: A shared lib :V (.lib ? or .dll ? idk)

    Logger::showMessage("Waiting for the game to fully load...", "LibraryThread");
    while (!gq->isLoaded()) // TODO: Hook, because it's better
        Sleep(100);
    Logger::showMessage("Game Loaded!", "LibraryThread");

    Logger::showMessage("Game Data: " + std::string(gq->getGameDataVersion()), "LibraryThread");

    Logger::showMessage("Asking a session id for the current session...", "LibraryThread");
    if (!_ees->askSessionId()) {
        Logger::showMessage("Unable to get a session id!", "LibraryThread", true);
        return;
    }
    Logger::showMessage("Session id for this session is " + _ees->getSessionId(), "LibraryThread");

    Logger::showMessage("Sending current session infos...", "LibraryThread");
    if (!_ees->sendSessionInfos()) {
        Logger::showMessage("Unable to send session infos!", "LibraryThread", true);
        return;
    }
    Logger::showMessage("Session infos sent sucessfully...", "LibraryThread");

    HANDLE pingThreadHandle = (HANDLE)_beginthreadex(0, 0, PingThread, _ees, 0, 0);
    HANDLE perfThreadHandle = (HANDLE)_beginthreadex(0, 0, PeformanceThread, _ees, 0, 0);
    HANDLE screenThreadHandle = (HANDLE)_beginthreadex(0, 0, ActivityThread, _ees, 0, 0);

    Logger::showMessage("Exit Thread!", "LibraryThread");
}