#pragma once

#include <string>
#include <sstream>
#include <codecvt>
#include <iomanip>
#include <fstream>

#include <string>
#include <regex>

static std::wstring getDllPath()
{
    // Dll Name
    TCHAR dllPath[MAX_PATH];
    HMODULE hModule;
    // Passing a static function to recover the DLL Module Handle
    if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPWSTR)&getDllPath, &hModule)) {
        throw std::exception("Unable to get module handle of an internal function");
    }
    else {
        GetModuleFileName(hModule, dllPath, MAX_PATH);
    }
    return std::wstring(dllPath);
}

static std::wstring getFileName(std::wstring& path, bool withExtention)
{
    // path.substr(path.find_last_of("/\\") + 1);
    std::wstring result = path.substr(path.find_last_of(L"/\\") + 1);
    if (!withExtention)
        result = result.substr(0, result.find_last_of('.'));
    return result;
}

static std::wstring getCurrentHandleFileName()
{
    TCHAR szExeFilePath[MAX_PATH];
    GetModuleFileName(NULL, szExeFilePath, MAX_PATH);
    std::wstring szExeFilePathW = std::wstring(szExeFilePath);
    std::wstring exeFileName = getFileName(szExeFilePathW, true);
    return exeFileName;
}

static void trim(std::string& s) {
	s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), [](char c) { return std::isspace(c, std::locale::classic()); }));
    s.erase(std::find_if_not(s.rbegin(), s.rend(), [](char c) { return std::isspace(c, std::locale::classic()); }).base(), s.end());
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

static void ToLower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

static void ToUpper(std::wstring& wstr)
{
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), ::toupper);
}

static void ToLower(std::wstring& wstr)
{
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), ::tolower);
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

// Fun fact: Jodocus created a similar function, that is x10 bigger xD
static std::string getConfigEntry(std::string name, std::string key, bool allow_space = true)
{
    std::string result = "";
    std::fstream newfile;

    // All WON config entry have that ('key:value' is not supported by the lobby, but 'key: value' work)
    key += ": ";

    newfile.open(name, std::ios::in);
    if (newfile.good() == false) {
//        Logger::showMessage("Unable to create file buffer for \"" + name + "\" !", "getConfigEntry", true);
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
//        Logger::showMessage("Unable to open file \"" + name + "\" !", "getConfigEntry", true);
    }
    return result;
}

static std::string extremRound(float val)
{
    if (val == 0)
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