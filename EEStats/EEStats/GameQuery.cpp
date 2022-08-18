#include "pch.h"

#include <iostream>
#include <sstream>
#include <shlwapi.h>

#include "GameQuery.h"
#include "MemoryHelper.h"
#include "Utils.h"
#include "sha512.h"

#pragma comment (lib, "Shlwapi.lib")

GameQuery::GameQuery()
{
    TCHAR szExeFileName[MAX_PATH];
    GetModuleFileName(NULL, szExeFileName, MAX_PATH);
    LPWSTR fileName = PathFindFileName((LPCWSTR) szExeFileName);
    std::string exeFileName = utf16ToUtf8(fileName);

    ToUpper(exeFileName);

    _productType = PT_Unknown;
    if (exeFileName.compare("EMPIRE EARTH.EXE") == 0)
        _productType = PT_EE;
    else if (exeFileName.compare("EE-AOC.EXE") == 0)
        _productType = PT_AoC;
};

bool GameQuery::isLoaded() {
    return 0 != *(int*)calcAddress(0x517BB8);
}

bool GameQuery::isPlaying() {
    return 0 != *(int*)calcAddress(0x00518378 + 0x44);
}

bool GameQuery::inLobby() {
    return 0 != *(int*)calcAddress(0x00544254);
}

char* GameQuery::getUsername()
{
    memoryPTR ptr = {
        0x51930C,
        { 0x0 }
    };
    return (char*)tracePointer(&ptr);
}

/// <summary>
/// List and return values in the given key that are REG_BINARY
/// </summary>
/// <returns>A map with name as key and data as value</returns>
std::map<std::string, std::string> getCDKeysFromHKEY(HKEY hKey)
{
    std::map<std::string, std::string> result;

    DWORD cValues;
    DWORD retCode;

    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        NULL,                // buffer for class name 
        NULL,           // size of class string 
        NULL,                    // reserved 
        NULL,               // number of subkeys 
        NULL,            // longest subkey size 
        NULL,            // longest class string 
        &cValues,                // number of values for this key 
        NULL,            // longest value name 
        NULL,         // longest value data 
        NULL,   // security descriptor 
        NULL);       // last write time 

    // Enumerate the key values. 

    const int MAX_KEY_LENGTH = 255;
    const int MAX_VALUE_NAME = 16383;
    if (cValues)
    {
        for (DWORD i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
        {
            DWORD cchValue, cchData = MAX_VALUE_NAME;
            TCHAR achValue[MAX_VALUE_NAME];

            BYTE valueBuffer[512];
            DWORD valueBufferSize = static_cast<DWORD>(sizeof(valueBuffer));

            DWORD dwType;

            cchValue = MAX_VALUE_NAME;
            achValue[0] = '\0';
            retCode = RegEnumValue(hKey, i,
                achValue,
                &cchValue,
                NULL,
                &dwType,
                valueBuffer,
                &valueBufferSize);
            
            if (retCode == ERROR_SUCCESS && dwType == 3)
                result.insert({ utf16ToUtf8(achValue) , hexStr(valueBuffer, valueBufferSize)});
        }
    }
    return result;
}

std::map<std::string, std::string> GameQuery::getCDKeys()
{
    std::map<std::string, std::string> result;
    std::map<std::string, std::string> tmp;
    HKEY key;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Sierra\\CDKeys"),
        0, KEY_READ, &key) == ERROR_SUCCESS) {
        tmp = getCDKeysFromHKEY(key);
        result.insert(tmp.begin(), tmp.end());
    }
    RegCloseKey(key);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\WOW6432Node\\Sierra\\CDKeys"),
        0, KEY_READ, &key) == ERROR_SUCCESS) {
        tmp = getCDKeysFromHKEY(key);
        result.insert(tmp.begin(), tmp.end());
    }
    RegCloseKey(key);

    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\WOW6432Node\\Sierra\\CDKeys"),
        0, KEY_READ, &key) == ERROR_SUCCESS) {
        tmp = getCDKeysFromHKEY(key);
        result.insert(tmp.begin(), tmp.end());
    }
    RegCloseKey(key);

    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\WOW6432Node\\Sierra\\CDKeys"),
        0, KEY_READ, &key) == ERROR_SUCCESS) {
        tmp = getCDKeysFromHKEY(key);
        result.insert(tmp.begin(), tmp.end());
    }
    RegCloseKey(key);

    return result;
}

GameQuery::ScreenType GameQuery::getCurrentScreen()
{
    if (isPlaying() && inLobby())
        return ST_PlayingOnline;
    else if (isPlaying() && !inLobby())
        return ST_PlayingSolo;
    else if (!isPlaying() && inLobby())
        return ST_Lobby;
    else
        return ST_Menu;
}

char* GameQuery::getGameBaseVersion()
{
    if (_productType == PT_EE)
        return (char*)calcAddress(0x04A9030);
    if (_productType == PT_AoC)
        return (char*)calcAddress(0x04BF570);
    return "";
}

char* GameQuery::getGameDataVersion()
{
    memoryPTR memEEC { 0x0513264, { 0x0 } };
    memoryPTR memAOC { 0x0529A0C, { 0x0 } };
    

    if (_productType == PT_EE)
        return (char*)tracePointer(&memEEC);
    if (_productType == PT_AoC)
        return (char*)tracePointer(&memAOC);
    return "";
}

bool GameQuery::isSupportedVersion() {
    const char* supportedEEC = "2002.09.12.v2.00";
    const char* supportedAOC = "2002.8.17.v1.00";
    const char* current = getGameBaseVersion();

    if (_productType == PT_EE)
        return strcmp(supportedEEC, current) == 0;
    if (_productType == PT_AoC)
        return strcmp(supportedAOC, current) == 0;
    return false;
}

std::string newVersionStrCpp;
char* newVersionStr;
char** newVersionStrPtr = &newVersionStr;
DWORD asmVersionStrReturnAddr;

void _asmVersionStr() {
    char*** vStrPtr = (char***)calcAddress(0x1D16FB);

    // don't do anything if string is already set to ours
    if (*(DWORD*)vStrPtr == (DWORD)&newVersionStr)
        return;

    std::stringstream vs;
    vs << **vStrPtr << newVersionStrCpp.c_str();

    const int length = vs.str().length() + 1;
    newVersionStr = new char[length];

    strcpy_s(newVersionStr, length, vs.str().c_str());
    writeBytes(vStrPtr, &newVersionStrPtr, 4);
}

void __declspec(naked) asmVersionString() {
    __asm {
        pushad
        call[_asmVersionStr]
        popad
        push 01
        pop edi
        push edi
        call[eax + 0x68]
        jmp[asmVersionStrReturnAddr]
    }
}

void GameQuery::setVersionSuffix(std::string suffix)
{
    newVersionStrCpp = suffix;
    functionInjector(
        calcAddress(0x1D16F2),
        asmVersionString,
        asmVersionStrReturnAddr,
        7
    );
}

std::string GameQuery::getGameChecksum()
{
    return sw::sha512::file("Empire Earth.exe");
}

GameQuery::ProductType GameQuery::getProductType()
{
    return _productType;
}

std::string GameQuery::getWONProductName()
{
    return getConfigEntry("WONLobby.cfg", "ProductName");
}

std::string GameQuery::getWONProductDirectory()
{
    return getConfigEntry("WONLobby.cfg", "ProductDirectory");
}

std::string GameQuery::getWONProductVersion()
{
    return getConfigEntry("zzWONVersion.cfg", "Version");
}