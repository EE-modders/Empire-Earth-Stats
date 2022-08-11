#pragma once

#include "MatomoEE.h"

#include <string>
#include <Shlwapi.h>
#include <iostream>

// EES VERSION
static const std::string EES_VERSION_STR = "1.0.0";
static const unsigned int EES_VERSION_MAJOR = 1;
static const unsigned int EES_VERSION_MINOR = 0;
static const unsigned int EES_VERSION_PATCH = 0;
// END EES VERSION

// EES HARD-CODED SETTINGS
const std::string EES_SETTINGS_URL = "https://stats.empireearth.eu/matomo.php";
const std::string EES_SETTINGS_ID = "2";
// END EES HARD-CODED SETTINGS

void* lib = nullptr;

class Library
{
private:
    MatomoEE* _matomo = nullptr;

public:
    Library() {
        showMessage("Loading...", "Library");
        _matomo = new MatomoEE(EES_SETTINGS_URL, EES_SETTINGS_ID, "WIP", EES_VERSION_STR);
        showMessage("Loaded!", "Library");
    }

    ~Library() {
        showMessage("Unloading...", "Library");
        delete _matomo;
        showMessage("Unloaded!", "Library");
        Sleep(5000);
    }

    static bool InitLibrary() {
        showMessage("Init Library...", "Library");
        if (lib) {
            showMessage("Library Already Initialized...", "Library");
            return false;
        }
        lib = new Library();
        showMessage("Library Initialized!", "Library");
        return true;
    }

    static bool DestroyLibrary() {
        showMessage("Destroy Library...", "Library");
        if (!lib) {
            showMessage("Library Already Destroyed...", "Library");
            return false;
        }
        delete lib;
        lib = nullptr;
        showMessage("Library Destroyed!", "Library");
        return true;
    }

    static Library* getLib() {
        if (lib == nullptr)
            return nullptr;
        return reinterpret_cast<Library*>(lib);
    }

    MatomoEE* getMatomo() {
        return _matomo;
    }
};