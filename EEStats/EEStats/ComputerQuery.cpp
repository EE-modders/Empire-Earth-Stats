#include "pch.h"

#include "ComputerQuery.h"
#include "WmiHelper.h"
#include "Utils.h"

#include <iostream>
#include <codecvt>

ComputerQuery::ComputerQuery()
{
    _bestGraphicPNPDeviceID = "";
    auto queryResult = _wmiHelper->queryKeyValWMI("SELECT PNPDeviceID,AdapterRAM FROM Win32_VideoController", L"PNPDeviceID", L"AdapterRAM");

    if (!queryResult.empty()) {
        std::pair<std::wstring, uint32_t> best = { L"", 0 };

        for (auto iter = queryResult.begin(); iter != queryResult.end(); ++iter)
        {
            if (iter->second.empty())
                continue;
            uint32_t val = std::atoi(utf16ToUtf8(iter->second).c_str());
            if (val > best.second)
                best = { iter->first, val };
        }
        _bestGraphicPNPDeviceID = utf16ToUtf8(best.first);
        // WMI sh$t that make me loose 8h just because the fk string require \\ and not \ for a fk simple WHERE string cmp ?!
        replaceAll(_bestGraphicPNPDeviceID, "\\", "\\\\");
    }

    // Cache resolution
    _windowsResolution.cx = GetSystemMetrics(SM_CXSCREEN);
    _windowsResolution.cy = GetSystemMetrics(SM_CYSCREEN);

    // Cache refresh rate
    auto queryResultRefreshRate = _wmiHelper->queryWMI(("SELECT CurrentRefreshRate FROM Win32_VideoController WHERE PNPDeviceID=\"" + _bestGraphicPNPDeviceID + "\"").c_str(), L"CurrentRefreshRate");
    if (queryResultRefreshRate.empty() || queryResultRefreshRate.at(0).empty())
        _refreshRate = 0;
    else
        _refreshRate = std::atoi(utf16ToUtf8(queryResultRefreshRate.at(0)).c_str());

    // Cache bits per pixel
    auto queryResultBitsPerPixel = _wmiHelper->queryWMI(("SELECT CurrentBitsPerPixel FROM Win32_VideoController WHERE PNPDeviceID=\"" + _bestGraphicPNPDeviceID + "\"").c_str(), L"CurrentBitsPerPixel");
    if (queryResultBitsPerPixel.empty() || queryResultBitsPerPixel.at(0).empty())
        _bitsPerPixel = 0;
    else
        _bitsPerPixel = std::atoi(utf16ToUtf8(queryResultBitsPerPixel.at(0)).c_str());
}

float ComputerQuery::getRAM()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return ((float)status.ullTotalPhys) / 1024 / 1024 / 1024;
}

std::string ComputerQuery::getGraphicVendorId()
{
    std::string fullpath = _bestGraphicPNPDeviceID;
    std::string pre_identifier = "VEN_";

    size_t index = fullpath.find(pre_identifier, 0);
    if (index == std::wstring::npos)
        return ""; // Not found
    if (index + pre_identifier.length() + 4 >= fullpath.length())
        return ""; // Not enough space
    fullpath = fullpath.substr(index + pre_identifier.length(), 4);
    if (fullpath.find("&") != std::wstring::npos || fullpath.find(";") != std::wstring::npos ||
        fullpath.find("_") != std::wstring::npos || fullpath.find("0000") != std::wstring::npos)
        return ""; // Invalid data
    return fullpath;
}

std::string ComputerQuery::getGraphicDeviceId()
{
    std::string fullpath = _bestGraphicPNPDeviceID;
    std::string pre_identifier = "DEV_";

    size_t index = fullpath.find(pre_identifier, 0);
    if (index == std::wstring::npos)
        return ""; // Not found
    if (index + pre_identifier.length() + 4 >= fullpath.length())
        return ""; // Not enough space
    fullpath = fullpath.substr(index + pre_identifier.length(), 4);
    if (fullpath.find("&") != std::wstring::npos || fullpath.find(";") != std::wstring::npos ||
        fullpath.find("_") != std::wstring::npos || fullpath.find("0000") != std::wstring::npos)
        return ""; // Invalid data
    return fullpath;
}

// Never use the GPU name as identifier since vendor may change it for no reasons over time
std::string ComputerQuery::getGraphicName()
{
    auto queryResult = _wmiHelper->queryWMI(("SELECT Name FROM Win32_VideoController WHERE PNPDeviceID=\"" + _bestGraphicPNPDeviceID + "\"").c_str(), L"Name");
    
    if (queryResult.empty() || queryResult.at(0).empty())
        return "";
    return utf16ToUtf8(queryResult.at(0));
}

std::string ComputerQuery::getGraphicVersion()
{
    auto queryResult = _wmiHelper->queryWMI(("SELECT DriverVersion FROM Win32_VideoController WHERE PNPDeviceID=\"" + _bestGraphicPNPDeviceID + "\"").c_str(), L"DriverVersion");

    if (queryResult.empty() || queryResult.at(0).empty())
        return "";
    return utf16ToUtf8(queryResult.at(0));
}

/// <summary>
/// Recover the refresh rate when ComputerQuery is created!!
/// The value will be invalid if the refresh rate has been changed by the game before the ComputerQuery ctor
/// </summary>
uint32_t ComputerQuery::getGraphicRefreshRate()
{
    return _refreshRate;
}

/// <summary>
/// Recover the bits per pixel when ComputerQuery is created!!
/// The value will be invalid if the bits per pixel has been changed by the game before the ComputerQuery ctor
/// </summary>
uint32_t ComputerQuery::getGraphicBitsPerPixel()
{
    return _bitsPerPixel;
}

float ComputerQuery::getGraphicDedicatedMemory()
{
    auto queryResult = _wmiHelper->queryWMI(("SELECT AdapterRAM FROM Win32_VideoController WHERE PNPDeviceID=\"" + _bestGraphicPNPDeviceID + "\"").c_str(), L"AdapterRAM");
    if (queryResult.empty() || queryResult.at(0).empty())
        return 0;
    float result = (float) std::stoull(utf16ToUtf8(queryResult.at(0)).c_str());
    if (result == 0)
        return 0;
    return result / 1024 / 1024 / 1024;
}

std::string ComputerQuery::getProcessorId()
{
    auto queryResult = _wmiHelper->queryWMI("SELECT ProcessorId FROM Win32_Processor", L"ProcessorId");
    if (queryResult.empty() || queryResult.at(0).empty())
        return "";
    return utf16ToUtf8(queryResult.at(0));
}

std::string ComputerQuery::getProcessorName()
{
    auto queryResult = _wmiHelper->queryWMI("SELECT Name FROM Win32_Processor", L"Name");
    if (queryResult.empty() || queryResult.at(0).empty())
        return "";
    return utf16ToUtf8(queryResult.at(0));
}

std::string ComputerQuery::getProcessorArch()
{
    SYSTEM_INFO systemInfo;
    GetNativeSystemInfo(&systemInfo);

    switch (systemInfo.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64:
        return "x64";
    case PROCESSOR_ARCHITECTURE_INTEL:
        return "x86";
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        return "ARM";
    case 12 /*PROCESSOR_ARCHITECTURE_ARM64*/:
        return "ARM64";
    default:
        return "";
    }
}

uint32_t ComputerQuery::getProcessorNumberOfCores()
{
    auto queryResult = _wmiHelper->queryWMI("SELECT NumberOfCores FROM Win32_Processor", L"NumberOfCores");
    if (queryResult.empty() || queryResult.at(0).empty())
        return 0;
    return std::atoi(utf16ToUtf8(queryResult.at(0)).c_str());
}

uint16_t ComputerQuery::getProcessorLoadPercentage()
{
    auto queryResult = _wmiHelper->queryWMI("SELECT LoadPercentage FROM Win32_Processor", L"LoadPercentage");
    if (queryResult.empty() || queryResult.at(0).empty())
        return 0;
    return std::atoi(utf16ToUtf8(queryResult.at(0)).c_str());
}

std::string ComputerQuery::getProcessorCurrentCorePercentage()
{
    return "";
}

int ComputerQuery::getDirectX_MajorVersion()
{
    switch (getWindowsVersionCQ()) {
        case Win10:
        case Win11:
            return 12;
        case Win8_1:
        case Win8:
        case Win7:
            return 11;
        case WinVista: {
            auto updIds = _wmiHelper->queryWMI("SELECT HotFixID FROM Win32_QuickFixEngineering", L"HotFixID");
            for (auto iter = updIds.begin(); iter != updIds.end();)
                if (iter->compare(L"KB971644") == 0) // Nice one MS
                    return 11;
            return 10;
        }
        case WinXP:
            return 9;
        default:
            return 0;
    }
}

std::string ComputerQuery::getDirectX_WrapperVersion()
{
    if (doesFileExist(L"DDraw.dll")) {
        if (doesFileExist(L"dgVoodoo.conf")) {
            wchar_t txt[255];
            DWORD cpy = GetPrivateProfileString(L"General", L"OutputAPI", NULL, txt, 255, L"./dgVoodoo.conf");
            if (txt == L"" && cpy != 0 && GetLastError() != ERROR_FILE_NOT_FOUND)
                return "";
            std::wstring ws(txt);
            return utf16ToUtf8(ws);;
        }
        return "d3d9";
    }
    return "";
}

std::string ComputerQuery::getDirectX_WrapperParams()
{
    if (doesFileExist(L"DDraw.dll") && doesFileExist(L"dgVoodoo.conf")) {
        // Don't remove the "./" (I debugged a fk hour because I didn't added it...)
        const LPCWSTR dgVoodooConf = L"./dgVoodoo.conf";
        const int maxValLen = 256;
        wchar_t version[maxValLen], fullScreenMode[maxValLen], disAltEntToTogScrMd[maxValLen], windowedAttributes[maxValLen];


        GetPrivateProfileString(L"", L"Version", NULL, version, maxValLen, dgVoodooConf);
        GetPrivateProfileString(L"General", L"FullScreenMode", NULL, fullScreenMode, maxValLen, dgVoodooConf);
        GetPrivateProfileString(L"DirectX", L"DisableAltEnterToToggleScreenMode", NULL, disAltEntToTogScrMd, maxValLen, dgVoodooConf);
        GetPrivateProfileString(L"GeneralExt", L"WindowedAttributes", NULL, windowedAttributes, maxValLen, dgVoodooConf);

        std::wstringstream wss;
        if (wcslen(version) != 0)
            wss << L"Version=" << version << ";";
        if (wcslen(fullScreenMode) != 0)
            wss << L"FullScreenMode=" << fullScreenMode << ";";
        if (wcslen(disAltEntToTogScrMd) != 0)
            wss << L"DisableAltEnterToToggleScreenMode=" << disAltEntToTogScrMd << ";";
        if (wcslen(windowedAttributes))
            wss << L"WindowedAttributes=" << windowedAttributes << ";";

        std::wstring ws = wss.str();
        if (ws.empty())
            return "";
        if (ws.at(ws.length() - 1) == ';')
            ws = ws.substr(0, ws.length() - 1);
        return utf16ToUtf8(ws);
    }
    return "";
}

/// <summary>
/// Recover the resolution when ComputerQuery is created!!
/// The value will be invalid if the resolution has been changed by the game before the ComputerQuery ctor
/// </summary>
SIZE ComputerQuery::getWindowsResolution()
{
    return _windowsResolution;
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

    return utf16ToUtf8(locale);

}

std::string ComputerQuery::getWindowsName()
{
    auto queryResult = _wmiHelper->queryWMI("SELECT Caption FROM Win32_OperatingSystem", L"Caption");
    if (queryResult.empty() || queryResult.at(0).empty())
        return "";
    return utf16ToUtf8(queryResult.at(0));
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

std::string ComputerQuery::getWindowsVersion()
{
    auto queryResult = _wmiHelper->queryWMI("SELECT Version FROM Win32_OperatingSystem", L"Version");
    if (queryResult.empty() || queryResult.at(0).empty())
        return "";
    return utf16ToUtf8(queryResult.at(0));
}

ComputerQuery::WindowsVersion ComputerQuery::getWindowsVersionCQ()
{
    auto version_wmi = getWindowsVersion();

    if (version_wmi.empty()) {
        showMessage("Unable to recover version from WMI !", "ComputerQuery", 1);
        return WinUnknown;
    }

    if (version_wmi.find_first_of("10.") == 0) {
        std::wstring version_wmi = _wmiHelper->queryWMI("SELECT BuildNumber FROM Win32_OperatingSystem", L"BuildNumber").at(0);
        if (!version_wmi.empty() && std::stoi(version_wmi) >= 22000) // this is a real disaster
            return Win11;
        return Win10;
    }
    if (version_wmi.find_first_of("6.3" == 0))
        return Win8_1;
    if (version_wmi.find_first_of("6.2" == 0))
        return Win8;
    if (version_wmi.find_first_of("6.1" == 0))
        return Win7;
    if (version_wmi.find_first_of("6.0" == 0))
        return WinVista;
    if (version_wmi.find_first_of("5.1" == 0) || version_wmi.find_first_of("5.2" == 0))
        return WinXP;
    showMessage("Unable to detect Windows version !", "ComputerQuery", 1);
    return WinUnknown;
}