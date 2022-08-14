#include "pch.h"

#include <iostream>
#include <sstream>

#include "GameQuery.h"
#include "MemoryHelper.h"
#include "Utils.h"

GameQuery::GameQuery(std::string game_path)
    : _game_path(game_path)
{
}

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

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

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

    if (cValues)
    {
        printf("\nNumber of values: %d\n", cValues);

        for (DWORD i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
        {
            DWORD cchValue, cchData = MAX_VALUE_NAME;
            TCHAR achValue[MAX_VALUE_NAME];
            BYTE achData[MAX_VALUE_NAME];

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
        return ScreenType::PlayingOnline;
    else if (isPlaying() && !inLobby())
        return ScreenType::PlayingSolo;
    else if (!isPlaying() && inLobby())
        return ScreenType::Lobby;
    else
        return ScreenType::Menu;
}

char* GameQuery::getGameVersion()
{
    return (char*)calcAddress(0x4A9030);
}

bool GameQuery::isSupportedVersion() {
    const char* supported = "2002.09.12.v2.00";
    const char* current = getGameVersion();

    return strcmp(supported, current) == 0;
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