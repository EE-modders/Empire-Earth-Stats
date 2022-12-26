#pragma once

#include "EEStats.h"

#include <string>
#include <iostream>
#include <ShlObj.h>

#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
    #define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

class Library
{
private:
    EEStats* _ees = nullptr;
    std::wstring _dllPath;

public:
    Library()
    {
        Logger::showMessage("Loading...", "Library");

        _dllPath = getDllPathUtils();

        // EE Stats
        _ees = new EEStats(EES_SETTINGS_URL, EES_VERSION_STR);

        Logger::showMessage("Loaded!", "Library");
    }

    ~Library() {
        Logger::showMessage("Unloading...", "Library");
        delete _ees;
        Logger::showMessage("Unloaded!", "Library");
    }

    void StartLibraryThread();

    void PrintCredits()
    {
        Logger::showMessage("------------------------------------------------------");
#ifdef _DEBUG
        Logger::showMessage("  Empire Earth Stats v" + EES_VERSION_STR + " DEBUG");
#else
        Logger::showMessage("  Empire Earth Stats v" + EES_VERSION_STR);
#endif
        Logger::showMessage("  By EnergyCube for the Empire Earth Community");
        Logger::showMessage("  This project is Open-Source under GNU GPL v3");
        Logger::showMessage("------------------------------------------------------");
        Logger::showMessage("  Credits:");
        Logger::showMessage("    zocker_160 (would be impossible without him)");
        Logger::showMessage("    cURL: https://curl.se/");
        Logger::showMessage("    sha512.h: Stefan Wilhelm, sha1.h: Steve Rei");
        Logger::showMessage("------------------------------------------------------");

        if (_ees != nullptr && _ees->getComputerQuery() != nullptr) {
            Logger::showMessage("  You can ask anytime to access/delete your data");
            Logger::showMessage("  For more information, please visit:");
            Logger::showMessage("    https://github.com/EE-modders/Empire-Earth-Stats");
            Logger::showMessage("  Your Empire Earth Stats Computer ID is:");
            Logger::showMessage("    " + _ees->getComputerQuery()->getUID());
            Logger::showMessage("------------------------------------------------------");
        }
    }

    void UpdateAttachRoutine()
    {
        // Update - Remove outdated files
        std::wstring oldFile = (getDllPath() + std::wstring(L".outdated")).c_str();

        if (doesFileExist(oldFile.c_str()))
        {
            Logger::showMessage("The remains of a completed update have been found, cleaning...", "Library");

            if (DeleteFile(oldFile.c_str())) {
                SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH | SHCNF_FLUSHNOWAIT, fs::current_path().c_str(), NULL); // Refresh explorer
                Logger::showMessage("The cleaning was successful!", "Library");
            }
            else
                Logger::showMessage("Unable to clean!", "Library", true);
        }
    }

    void UpdateDetachRoutine()
    {
        // Update - Found a newer version (file, no download here!)
        std::wstring updateFile = (getDllPath() + std::wstring(L".update")).c_str();

        if (doesFileExist(updateFile.c_str()))
        {
            std::wstring oldFile = (getDllPath() + std::wstring(L".outdated")).c_str();
            std::wstring currentFile = getDllPath().c_str();

            Logger::showMessage("An update is pending, deactivating the current version and activating the new one...", "Library");
            
            if (MoveFile(currentFile.c_str(), oldFile.c_str()) && MoveFile(updateFile.c_str(), currentFile.c_str())) {
                SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH | SHCNF_FLUSHNOWAIT, fs::current_path().c_str(), NULL); // Refresh explorer
                Logger::showMessage("The update seems to be installed, the next boot will start the new one and remove the old version!", "Library");
            }
            else {
                Logger::showMessage("The update does not seem to have been installed correctly!", "Library", true);
            }
        }
    }

    EEStats* getEES() {
        return _ees;
    }

    std::wstring getDllPath()
    {
        return _dllPath;
    }
};