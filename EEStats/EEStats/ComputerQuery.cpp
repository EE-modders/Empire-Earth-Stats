#include "pch.h"

#include "ComputerQuery.h"
#include "WmiHelper.h"

#include <iostream>
#include <codecvt>

DWORDLONG ComputerQuery::getRAM()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
}

std::string ComputerQuery::getSimpleRAM(bool gonly)
{

    long double ram_ko = (long double) getRAM();

    if (ram_ko == 0) // prevent div 0 and the explosion of the universe
        return "0";

    long double ram_mo = ram_ko / 1024 / 1024 / 1024;

    std::string simple = std::to_string(ram_mo);
    simple = simple.substr(0, simple.find(".") + 2);

    if (simple.at(simple.length() - 1) == '0' || gonly) {
        return simple.substr(0, simple.length() - 2);
    }
    return simple;
}

std::string ComputerQuery::getGraphicVendorId()
{
    std::wstring fullpath = queryWMI("SELECT * FROM Win32_VideoController", L"PNPDeviceID");

    std::wstring pre_identifier = L"VEN_";

    size_t index = fullpath.find(pre_identifier, 0);
    if (index == std::wstring::npos)
        return ""; // Not found
    if (index + pre_identifier.length() + 4 >= fullpath.length())
        return ""; // Not enough space
    fullpath = fullpath.substr(index + pre_identifier.length(), 4);
    if (fullpath.find(L"&") != std::wstring::npos || fullpath.find(L";") != std::wstring::npos ||
        fullpath.find(L"_") != std::wstring::npos || fullpath.find(L"0000") != std::wstring::npos)
        return ""; // Invalid data

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(fullpath);
}

std::string ComputerQuery::getGraphicDeviceId()
{
    std::wstring fullpath = queryWMI("SELECT * FROM Win32_VideoController", L"PNPDeviceID");

    std::wstring pre_identifier = L"DEV_";

    size_t index = fullpath.find(pre_identifier, 0);
    if (index == std::wstring::npos)
        return ""; // Not found
    if (index + pre_identifier.length() + 4 >= fullpath.length())
        return ""; // Not enough space
    fullpath = fullpath.substr(index + pre_identifier.length(), 4);
    if (fullpath.find(L"&") != std::wstring::npos || fullpath.find(L";") != std::wstring::npos ||
        fullpath.find(L"_") != std::wstring::npos || fullpath.find(L"0000") != std::wstring::npos)
        return ""; // Invalid data

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(fullpath);
}

// Never use the GPU name as identifier since vendor may change it for no reasons over time
std::string ComputerQuery::getGraphicName()
{
    std::wstring name = queryWMI("SELECT * FROM Win32_VideoController", L"Name");

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(name);
}

std::string ComputerQuery::getGraphicVersion()
{
    std::wstring name = queryWMI("SELECT * FROM Win32_VideoController", L"DriverVersion");

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(name);
}

std::string ComputerQuery::getGraphicCurrentRefreshRate()
{
    std::wstring name = queryWMI("SELECT * FROM Win32_VideoController", L"CurrentRefreshRate");

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(name);
}

std::string ComputerQuery::getGraphicMaxRefreshRate()
{
    std::wstring name = queryWMI("SELECT * FROM Win32_VideoController", L"MaxRefreshRate");

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(name);
}

std::string ComputerQuery::getGraphicCurrentBitsPerPixel()
{
    std::wstring name = queryWMI("SELECT * FROM Win32_VideoController", L"CurrentBitsPerPixel");

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(name);
}

std::string ComputerQuery::getGraphicDedicatedMemory()
{
    std::wstring name = queryWMI("SELECT * FROM Win32_VideoController", L"AdapterRAM");

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(name);
}

SIZE ComputerQuery::getWindowsResolution()
{
    SIZE res;

    /*
        DISABLED because zocker said it works bad on Wine

        // Get a handle to the desktop window
        const HWND hDesktop = GetDesktopWindow();
        // Get the size of screen to the variable desktop
        GetWindowRect(hDesktop, &desktop);
    */

    res.cx = GetSystemMetrics(SM_CXSCREEN);
    res.cy = GetSystemMetrics(SM_CYSCREEN);
    return res;
}

std::string ComputerQuery::getWindowsLocale()
{
    LANGID langID = GetUserDefaultUILanguage();

    // Convert LANGID to a RFC 4646 language tag (per navigator.language)
    int langSize = GetLocaleInfo(langID, LOCALE_SISO639LANGNAME, NULL, 0);
    int countrySize = GetLocaleInfo(langID, LOCALE_SISO3166CTRYNAME, NULL, 0);

    wchar_t* lang = new wchar_t[langSize + countrySize + 1];
    wchar_t* country = new wchar_t[countrySize];

    GetLocaleInfo(langID, LOCALE_SISO639LANGNAME, lang, langSize);
    GetLocaleInfo(langID, LOCALE_SISO3166CTRYNAME, country, countrySize);

    // add
    std::wstring locale = lang;
    locale += L"-";
    locale += country;

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(locale);

}

bool ComputerQuery::isWine()
{
    HMODULE ntdllMod = GetModuleHandle(L"ntdll.dll");

    return ntdllMod && GetProcAddress(ntdllMod, "wine_get_version");
}

const char* ComputerQuery::getWineVersion()
{

    HMODULE ntdllMod = GetModuleHandle(L"ntdll.dll");
    const char* (CDECL * w_g_v)() = NULL;

    if (ntdllMod)
        w_g_v = (const char* (*)())GetProcAddress(ntdllMod, "wine_get_version");

    if (w_g_v)
        return w_g_v();
    return NULL;
}

ComputerQuery::WindowsVersion ComputerQuery::getWindowsVersion()
{

    std::wstring version_wmi = queryWMI("SELECT * FROM Win32_OperatingSystem", L"Version");

    if (version_wmi.empty()) {
        std::cout << "Unable to recover version from WMI !" << std::endl;
        return WinUnknown;
    }

    if (version_wmi.find_first_of(L"10.") == 0) {
        std::wstring version_wmi = queryWMI("SELECT * FROM Win32_OperatingSystem", L"BuildNumber");
        if (!version_wmi.empty() && std::stoi(version_wmi) >= 22000) // this is a real disaster
            return Win11;
        return Win10;
    }
    if (version_wmi.find_first_of(L"6.3" == 0))
        return Win8_1;
    if (version_wmi.find_first_of(L"6.2" == 0))
        return Win8;
    if (version_wmi.find_first_of(L"6.1" == 0))
        return Win7;
    if (version_wmi.find_first_of(L"6.0" == 0))
        return WinVista;
    if (version_wmi.find_first_of(L"5.1" == 0) || version_wmi.find_first_of(L"5.2" == 0))
        return WinXP;
    std::cout << "Unable to detect Windows version !" << std::endl;
    return WinUnknown;
}
