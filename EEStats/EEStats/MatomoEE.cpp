#include "pch.h"

#include "MatomoEE.h"
#include "WmiHelper.h"

#include <iostream>
#include <string>
#include <codecvt>

MatomoEE::MatomoEE(std::string base_url, std::string site_id, std::string uid, std::string lib_version):
	_base_url(base_url), _site_id(site_id), _uid(uid), _lib_version(lib_version)
{
    showMessage("Init cURL...", "MatomoEE");
	curl_global_init(CURL_GLOBAL_ALL);
    _cq = new ComputerQuery();
    showMessage("cURL loaded!", "MatomoEE");
}

MatomoEE::~MatomoEE()
{
    showMessage("Clean-up cURL...", "MatomoEE");
    curl_global_cleanup();
    showMessage("Cleaned-up cURL!", "MatomoEE");
}

void MatomoEE::sendScreen(GameQuery::ScreenType screen_type)
{
    std::string model = "&action_name=%a";

    switch (screen_type)
    {
        case GameQuery::Menu:
            replaceAll(model, "%a", "In Menu");
            break;
        case GameQuery::Editor:
            replaceAll(model, "%a", "In Editor");
            break;
        case GameQuery::PlayingOnline:
            replaceAll(model, "%a", "Playing Online");
            break;
        case GameQuery::PlayingSolo:
            replaceAll(model, "%a", "Playing Solo");
            break;
        case GameQuery::Lobby:
            replaceAll(model, "%a", "In Lobby");
            break;
        case GameQuery::Unknown:
        default:
            replaceAll(model, "%a", "Unknown");
            break;
    }

    sendRequest(model);
}

void MatomoEE::sendPing()
{
    showMessage("Sending ping!", "MatomoEE");
    sendRequest("&ping=1");
}

/*
void MatomoEE::sendEvent(GameQuery::EventType action)
{
    // %c : Category
    // %a : Action
    // %n : Name
    std::string model = "&e_c=%c&e_a=%a&e_n=%n";

    switch (action)
    {
    case GameQuery::ProcessStart:
        replaceAll(model, "%c", "Process");
        replaceAll(model, "%a", "Start");
        replaceAll(model, "%n", "Starting Process");
        break;
    case GameQuery::ProcessLeave:
        model += "Leave";
        break;
    default:
        model += "Unknown";
        break;
    }
    sendRequest(model);
}
*/

// Ugly as fk, I'm sorry for evey ppl that could try to read this sh$t
// Add sys info to user agent and the new strange headers
std::string MatomoEE::buildUserAgent(struct curl_slist* &headers)
{
    std::string ua = "User-Agent: ";

    if (_cq->isWine()) {
        headers = curl_slist_append(headers, "Sec-CH-UA-Platform: Linux");

        // Wine can be Linux or MacOS :V
        // Let's say Unix so
        const char* version = _cq->getWineVersion();
        if (version != NULL) {
            std::string version_str(version);
            ua += "Mozilla/5.0 (Linux; Unix; Wine " + version_str + ")";
        }
        else {
            ua += "Mozilla/5.0 (Linux; Unix; Wine)";
        }
    }
    else {
        headers = curl_slist_append(headers, "Sec-CH-UA-Platform: Windows");

        ComputerQuery::WindowsVersion winver = _cq->getWindowsVersion();

        switch (winver) {
            case ComputerQuery::Win11:
                ua += "Mozilla/5.0 (Windows NT 10.0";
                headers = curl_slist_append(headers, "Sec-CH-UA-Platform-Version: 13.0.0"); // An absolute disaster
                break;
            case ComputerQuery::Win10:
                ua += "Mozilla/5.0 (Windows NT 10.0";
                break;
            case ComputerQuery::Win8_1:
                ua += "Mozilla/5.0 (Windows NT 6.3";
                break;
            case ComputerQuery::Win8:
                ua += "Mozilla/5.0 (Windows NT 6.2";
                break;
            case ComputerQuery::Win7:
                ua += "Mozilla/5.0 (Windows NT 6.1";
                break;
            case ComputerQuery::WinVista:
                ua += "Mozilla/5.0 (Windows NT 6.0";
                break;
            case ComputerQuery::WinXP:
                ua += "Mozilla/5.0 (Windows NT 5.1";
            case ComputerQuery::WinUnknown:
            default:
                showMessage("Unable to recover version from WMI !", "MatomoEE", true);
                ua += "Mozilla/5.0 (";
                break;
        }

        // PROCESSOR_ARCHITECTURE_AMD64 = x64 (AMD or Intel)
        // PROCESSOR_ARCHITECTURE_ARM = ARM
        // PROCESSOR_ARCHITECTURE_ARM64 = ARM64
        // PROCESSOR_ARCHITECTURE_INTEL = x86

        SYSTEM_INFO systemInfo;
        GetNativeSystemInfo(&systemInfo);

        switch (systemInfo.wProcessorArchitecture)
        {
        case PROCESSOR_ARCHITECTURE_AMD64:
            // headers = curl_slist_append(headers, "Sec-CH-UA-Arch: x86_64");
            // headers = curl_slist_append(headers, "Sec-CH-UA-Bitness: 64");
            ua += "; x86_64";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            // headers = curl_slist_append(headers, "Sec-CH-UA-Arch: x86");
            // headers = curl_slist_append(headers, "Sec-CH-UA-Bitness: 32");
            //ua += "; x86"; From what I seen, nothing = x86
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            // headers = curl_slist_append(headers, "Sec-CH-UA-Arch: ARM");
            // headers = curl_slist_append(headers, "Sec-CH-UA-Bitness: 32");
            ua += "; ARM";
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            // headers = curl_slist_append(headers, "Sec-CH-UA-Arch: ARM64");
            // headers = curl_slist_append(headers, "Sec-CH-UA-Bitness: 64");
            ua += "; ARM64";
            break;
        default:
            break;
        }

        /* Seems useless for the moment
        BSTR manufacturer_wmi = queryWMI("SELECT * FROM Win32_ComputerSystem", L"Manufacturer");
        if (manufacturer_wmi != nullptr) {
            std::wstring manufacturer_wstr(manufacturer_wmi, SysStringLen(manufacturer_wmi));
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;

            std::wcout << manufacturer_wstr << std::endl;
            ua += "; ";
            ua += conv.to_bytes(manufacturer_wstr);
        }

        BSTR manufacturer_wmi = queryWMI("SELECT * FROM Win32_ComputerSystem", L"Model");
        if (manufacturer_wmi != nullptr) {
            std::wstring manufacturer_wstr(manufacturer_wmi, SysStringLen(manufacturer_wmi));
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;

            std::string model_str = "Sec-CH-UA-Model: " + conv.to_bytes(manufacturer_wstr);
            headers = curl_slist_append(headers, model_str.c_str());
        }*/

        ua += ") EmpireEarthStats/" + _lib_version;
    }

    /* Seems useless for the moment
    if (_cq->getRAM() != 0) {
        std::string ram_header = "Device-Memory: " + _cq->getSimpleRAM(true);
        headers = curl_slist_append(headers, ram_header.c_str());
    }

    std::string version_ch = "Sec-CH-UA: \"Empire Earth Stats\"; v=\"" + EES_VERSION_STR + "\"";
    headers = curl_slist_append(headers, version_ch.c_str());
    */

    return ua;
}

bool first_request = true;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    // Crash sometime :V Use with caution... Or fix it :>
    //std::string a = ((std::string*)userp)->append((char*)contents, size * nmemb);
    //std::cout << "Callback: " << a << std::endl;
    showMessage("Got response from server!", "MatomoEE");
    return size * nmemb;
}

void MatomoEE::sendRequest(std::string path)
{
    CURL* curl;
    CURLcode res;
    struct curl_slist* headers = NULL;

    // BASIC INFO
    std::string pre =
        "?idsite=" + _site_id +
        "&rec=1"
        "&apiv=1";
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    pre += "&rand=" + std::to_string(std::rand());

    // Make every game start a new session + log the ua
    if (first_request) {
        struct curl_slist* dummy = NULL;
        showMessage(buildUserAgent(dummy), "MatomoEE");
        pre += "&new_visit=1";
        first_request = false;
    }

    // TIME
    std::time_t timer = std::time(0); // get time now
    std::tm now{};
    localtime_s(&now, &timer);

    pre += "&h=" + std::to_string(now.tm_hour)
        + "&m=" + std::to_string(now.tm_min)
        + "&s=" + std::to_string(now.tm_sec);

    // RESOLUTION
    SIZE resolution = _cq->getWindowsResolution();
    pre += "&res=" + std::to_string(resolution.cx) + "x" + std::to_string(resolution.cy);

    // LOCALE
    std::string locale = "Accept-Language: " + _cq->getWindowsLocale();
    headers = curl_slist_append(headers, locale.c_str());

    // USER AGENT
    std::string ua = buildUserAgent(headers);
    headers = curl_slist_append(headers, ua.c_str());
;
    std::string final_url = _base_url + pre + path;
    replaceAll(final_url, " ", "%20");

    // Print
    std::stringstream ssUrl;
    ssUrl << "GET: " << final_url;
    showMessage(ssUrl.str(), "MatomoEE");

    // cURL init and options
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, final_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
#ifdef _DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    /* if redirected, tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Set header
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::stringstream sscUrlFail;
        sscUrlFail << "curl_easy_perform() failed: " << curl_easy_strerror(res);
        showMessage(sscUrlFail.str(), "MatomoEE", true);
    }
    curl_easy_cleanup(curl);
}