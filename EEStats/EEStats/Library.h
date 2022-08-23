#pragma once

#include "EEStats.h"

#include <string>
#include <iostream>

class Library
{
private:
    EEStats* _ees = nullptr;
    std::wstring _dllPath;

public:
    Library() {
        showMessage("Loading...", "Library");

        // EE Stats
        _ees = new EEStats(EES_SETTINGS_URL, EES_VERSION_STR);

        // Dll Name
        TCHAR dllPath[MAX_PATH];
        HMODULE hModule;
        // Passing a static function to recover the DLL Module Handle
        if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPWSTR)&showMessage, &hModule)) {
            throw std::exception("Unable to get module handle of an internal function");
        }
        else {
            GetModuleFileName(hModule, dllPath, MAX_PATH);
            _dllPath = dllPath;
            showMessage("DLL Path: " + utf16ToUtf8(_dllPath), "Library");
        }
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
        showMessage("------------------------------------------------------");
        showMessage("  Empire Earth Stats v" + EES_VERSION_STR);
        showMessage("  By EnergyCube for the Empire Earth Community");
        showMessage("  This project is Open-Source under GNU GPL v3");
        showMessage("------------------------------------------------------");
        showMessage("  Credits:");
        showMessage("    zocker_160 (would be impossible without him)");
        showMessage("    cURL: https://curl.se/");
        showMessage("    sha512.h: Stefan Wilhelm, sha1.h: Steve Rei");
        showMessage("------------------------------------------------------");

        if (_ees != nullptr && _ees->getComputerQuery() != nullptr) {
            showMessage("  You can ask anytime to access/delete your data");
            showMessage("  For more information, please visit:");
            showMessage("    https://github.com/EE-modders/Empire-Earth-Stats");
            showMessage("  Your Empire Earth Stats Computer ID is:");
            showMessage("    " + _ees->getComputerQuery()->getUID());
            showMessage("------------------------------------------------------");
        }
    }

    void UpdateAttachRoutine()
    {
        // Update - Remove outdated files
        std::wstring oldFile = (getDllPath() + std::wstring(L".outdated")).c_str();

        if (doesFileExist(oldFile.c_str()))
        {
            showMessage("The remains of a completed update have been found, cleaning...", "Library");

            if (DeleteFile(oldFile.c_str()))
                showMessage("The cleaning was successful!", "Library");
            else
                showMessage("Unable to clean!", "Library", true);
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

            showMessage("An update is pending, deactivating the current version and activating the new one...", "Library");
            
            if (MoveFile(currentFile.c_str(), oldFile.c_str()) && MoveFile(updateFile.c_str(), currentFile.c_str())) {
                showMessage("The update seems to be installed, the next boot will start the new one and remove the old version!", "Library");
            }
            else {
                showMessage("The update does not seem to have been installed correctly!", "Library", true);
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