#include "pch.h"

#include "ComputerQuery.h"
#include "Utils.h"
#include "sha512.h"

#include <thread>
#include "Logger.h"

std::string ComputerQuery::getBiosSerial()
{
    return "";
}


std::string ComputerQuery::getWindowsDiskSerial()
{
    return "";
}

std::string ComputerQuery::getComputerSerial()
{
    return "";
}

std::string ComputerQuery::getUID()
{
    return "";
}

bool ComputerQuery::runInVirtualPC() // Who use that ?
{
    return false;
}

bool ComputerQuery::runInVMWare()
{
    return false;
}

bool ComputerQuery::runInVirtualBox()
{
    return false;
}

bool ComputerQuery::runInParallelsDesktop()
{
    return false;
}

bool ComputerQuery::runInOtherVM()
{
    return false;
}

bool ComputerQuery::runInVM()
{
    if (runInOtherVM() || runInVMWare() || runInVirtualBox() || runInVirtualPC() /*|| runInParallelsDesktop() don't work*/)
        return true;
    return false;
}

/// DLL EXPORT FOR EE COMMUNITY SETUP
#include <thread>
#include <process.h>
#define DllExport extern "C" __declspec(dllexport)

DllExport const char* EEStats_getUID()
{
    std::string result_scope = "ERROR_UNKNOWN";

    // ComputerQuery use WMI and it require to run in a thread :|
    auto initLamda = [](void* data) -> unsigned int
    {
        std::string* lamda_scope = static_cast<std::string*>(data);
        if (!lamda_scope)
            return 0;
        ComputerQuery gq;
        *lamda_scope = gq.getUID();
        if (lamda_scope->empty())
            *lamda_scope = "ERROR_GENERATION";
        return 1;
    };

    HANDLE wmiThread = (HANDLE)_beginthreadex(0, 0, initLamda, &result_scope, 0, 0);
    if (wmiThread != NULL)
        WaitForSingleObject(wmiThread, INFINITE);
    else
        result_scope = "ERROR_THREAD";
    // strdup because std::string will be cleaned, idk how to make that clean without passing a ptr as arg...
    return _strdup(result_scope.c_str());
}

DllExport bool EEStats_runInVM()
{
    bool result_scope = false;

    auto initLamda = [](void* data) -> unsigned int
    {
        bool* lamda_scope = static_cast<bool*>(data);
        *lamda_scope = ComputerQuery().runInVM();
        return 1;
    };
    HANDLE wmiThread = (HANDLE)_beginthreadex(0, 0, initLamda, &result_scope, 0, 0);
    if (wmiThread != NULL)
        WaitForSingleObject(wmiThread, INFINITE);
    return result_scope;
}

DllExport bool EEStats_isWine()
{
    bool result_scope = false;

    auto initLamda = [](void* data) -> unsigned int
    {
        bool* lamda_scope = static_cast<bool*>(data);
        *lamda_scope = ComputerQuery().isWine();
        return 1;
    };
    HANDLE wmiThread = (HANDLE)_beginthreadex(0, 0, initLamda, &result_scope, 0, 0);
    if (wmiThread != NULL)
        WaitForSingleObject(wmiThread, INFINITE);
    return result_scope;
}

DllExport const char* EEStats_getWineVersion()
{
    char* result_scope = false;

    auto initLamda = [](void* data) -> unsigned int
    {
        char** lamda_scope = static_cast<char**>(data);
        *lamda_scope = (char*)ComputerQuery().getWineVersion();
        return 1;
    };
    HANDLE wmiThread = (HANDLE)_beginthreadex(0, 0, initLamda, &result_scope, 0, 0);
    if (wmiThread != NULL)
        WaitForSingleObject(wmiThread, INFINITE);
    return result_scope;
}

DllExport const char* EEStats_getProcessorArch()
{
    std::string result_scope = "ERROR_UNKNOWN";

    // ComputerQuery use WMI and it require to run in a thread :|
    auto initLamda = [](void* data) -> unsigned int
    {
        std::string* lamda_scope = static_cast<std::string*>(data);
        if (!lamda_scope)
            return 0;
        ComputerQuery gq;
        *lamda_scope = gq.getProcessorArch();
        if (lamda_scope->empty())
            *lamda_scope = "ERROR_GENERATION";
        return 1;
    };

    HANDLE wmiThread = (HANDLE)_beginthreadex(0, 0, initLamda, &result_scope, 0, 0);
    if (wmiThread != NULL)
        WaitForSingleObject(wmiThread, INFINITE);
    else
        result_scope = "ERROR_THREAD";
    // strdup because std::string will be cleaned, idk how to make that clean without passing a ptr as arg...
    return _strdup(result_scope.c_str());
}

DllExport bool EEStats_isEmpireEarthRunning()
{
    return OpenMutex(SYNCHRONIZE, TRUE, L"StainlessSteelStudiosPresentsEmpireEarth") != NULL;
}

DllExport bool EEStats_isTheArtOfConquestRunning()
{
    return OpenMutex(SYNCHRONIZE, TRUE, L"MadDocSoftwarePresentsEmpireEarthExpansion") != NULL;
}
/// END DLL EXPORT FOR EE COMMUNITY SETUP