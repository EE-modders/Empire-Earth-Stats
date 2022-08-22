#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <codecvt>
#include <iomanip>
#include <fstream>

#include <string>
#include <regex>

static void trim(std::string& s) {
    s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), [](char c) { return std::isspace(c); }));
    s.erase(std::find_if_not(s.rbegin(), s.rend(), [](char c) { return std::isspace(c); }).base(), s.end());
}

static void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

static LONGLONG fileSize(const wchar_t* name)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(name, GetFileExInfoStandard, &fad))
        return -1; // error condition, could call GetLastError to find out more
    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return size.QuadPart;
}

static const std::string currentDateTime(std::string format) {
    time_t now = time(0);
    struct tm tstruct;
    char buf[256];
    localtime_s(&tstruct, &now);
    strftime(buf, sizeof(buf), format.c_str(), &tstruct);
    return buf;
}

#include <mutex>
static std::mutex mtx;

static void showMessage(std::string msg, std::string scope = "", bool error = false, bool show_time = true)
{
    std::unique_lock<std::mutex> lck(mtx);
    std::stringstream ss;

    if (show_time)
        ss << "[" << currentDateTime("%H:%M:%S %d/%m/%Y") << "] ";

    if (error)
        ss << "[ERR] ";
    else
        ss << "[INF] ";

    if (!scope.empty())
        ss << "(" << scope << ") ";
    ss << msg;

    if (error) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
        std::cout << ss.str() << std::endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    }
    else {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
        std::cout << ss.str() << std::endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    }
}

/*static void showMessage(const char* msg, std::string scope = "", bool error = false, bool show_time = true)
{
    showMessage(std::string(msg), scope, error, show_time); // Big brain moment
}*/

static bool doesFileExist(LPCWSTR lpszFilename)
{
    DWORD attr = GetFileAttributes(lpszFilename);
    return ((attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

static bool doesFolderExist(LPCWSTR lpszFoldername)
{
    DWORD attr = GetFileAttributes(lpszFoldername);
    return ((attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

static void ToUpper(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

static void ToLower(unsigned char* Pstr)
{
    char* P = (char*)Pstr;
    unsigned long length = strlen(P);
    for (unsigned long i = 0; i < length; i++) P[i] = tolower(P[i]);
    return;
}

static std::wstring utf8ToUtf16(const std::string& utf8Str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(utf8Str);
}

static std::string utf16ToUtf8(const std::wstring& utf16Str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(utf16Str);
}

static std::string hexStr(BYTE* data, int len)
{
    std::stringstream ss;
    ss << std::hex;

    for (int i(0); i < len; ++i)
        ss << std::setw(2) << std::setfill('0') << (int)data[i];

    return ss.str();
}

static std::string getConfigEntry(std::string name, std::string key, bool allow_space = true)
{
    std::string result = "";
    std::fstream newfile;

    // All WON config entry have that ('key:value' is not supported by the lobby, but 'key: value' work)
    key += ": ";

    newfile.open(name, std::ios::in);
    if (newfile.good() == false) {
        showMessage("Unable to create file buffer for \"" + name + "\" !", "getConfigEntry", true);
        return result;
    }

    if (newfile.is_open()) {
        std::string tmp;
        while (std::getline(newfile, tmp)) { // I don't like that... but it work lol
            if (size_t index = tmp.find(key) != std::string::npos) { // We find the key
                tmp.erase(index - 1, key.length()); // Remove key from the line
                if (!allow_space) // Remove any space
                    tmp.erase(remove(tmp.begin(), tmp.end(), ' '), tmp.end());
                result = tmp;
                break;
            }
        }
        newfile.close();
    }
    else {
        showMessage("Unable to open file \"" + name + "\" !", "getConfigEntry", true);
    }
    return result;
}

static std::string extremRound(float val)
{
    if (val == 0) // prevent div 0 and the explosion of the universe
        return "0";

    std::string simple = std::to_string(val);
    simple = simple.substr(0, simple.find(".") + 2);

    if (simple.at(simple.length() - 1) == '1' ||
        simple.at(simple.length() - 1) == '9') {
        float tmp = roundf(std::stof(simple));
        if (tmp > 0.1)
            return extremRound(tmp);
    }
    return simple;
}