#pragma once

#include "EEStats.h"

#include <string>
#include <iostream>

// EES VERSION
static const std::string EES_VERSION_STR = "1.0.0";
static const unsigned int EES_VERSION_MAJOR = 1;
static const unsigned int EES_VERSION_MINOR = 0;
static const unsigned int EES_VERSION_PATCH = 0;
// END EES VERSION

// EES HARD-CODED SETTINGS
const std::string EES_SETTINGS_URL = "https://stats.empireearth.eu/eestats/";
// END EES HARD-CODED SETTINGS

class Library
{
private:
    EEStats* _ees = nullptr;
    std::wstring dllName;

public:
    Library(HMODULE hModule) {
        showMessage("Loading...", "Library");

        // EE Stats
        _ees = new EEStats(EES_SETTINGS_URL, EES_VERSION_STR);

        // Dll Name
        TCHAR _dllName[MAX_PATH];
        GetModuleFileName(hModule, _dllName, MAX_PATH);
        dllName = _dllName;

        showMessage("Loaded!", "Library");
    }

    ~Library() {
        showMessage("Unloading...", "Library");
        delete _ees;
        showMessage("Unloaded!", "Library");
    }

    void StartLibraryThread();

    void PrintCredits()
    {
        showMessage("----------------------------------------------------");
        showMessage("  Empire Earth Stats v" + EES_VERSION_STR);
        showMessage("  By EnergyCube for the Empire Earth Community");
        showMessage("  This project is Open-Source under GNU GPL v3");
        showMessage("----------------------------------------------------");
        showMessage("  Credits:");
        showMessage("    zocker_160 (would be impossible without him)");
        showMessage("    cURL: https://curl.se/");
        showMessage("    sha512.h and crc.h: Stefan Wilhelm");
        showMessage("----------------------------------------------------");
    }


    void UpdateAttachRoutine()
    {
        // Update - Remove outdated files
        LPCWSTR oldFile = (getDllName() + L".dll.outdated").c_str();
        if (doesFileExist(oldFile))
        {
            showMessage("The remains of a completed update have been found, cleaning...", "Library");

            bool deleteResult = DeleteFile(oldFile);
            if (deleteResult)
                showMessage("The cleaning was successful!", "Library");
            else
                showMessage("Unable to clean!", "Library", true);
        }
    }

    void UpdateDetachRoutine()
    {
        // Update - Found a newer version (file, no download here!)
        LPCWSTR updateFile = (getDllName() + L".dll.update").c_str();
        if (doesFileExist(updateFile))
        {
            LPCWSTR oldFile = (getDllName() + L".dll.outdated").c_str();
            LPCWSTR currentFile = getDllName().c_str();

            showMessage("An update is pending, deactivating the current version and activating the new one...", "Library");
            if (MoveFile(currentFile, oldFile) && MoveFile(updateFile, currentFile)) {
                showMessage("The update seems to be installed, the next boot will start the new one and remove the old version!", "Library");
            }
            else {
                showMessage("The update does not seem to have installed correctly!", "Library", true);
            }
        }
    }

    EEStats* getEES() {
        return _ees;
    }

    std::wstring getDllName()
    {
        return dllName;
    }
};