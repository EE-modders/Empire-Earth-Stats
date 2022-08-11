#include "pch.h"

#include <iostream>
#include <sstream>

#include "GameQuery.h"
#include "MemoryHelper.h"

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
    return (char*)calcAddress(0x0051930C);
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