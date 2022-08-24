#include "pch.h"

#include "EEStats.h"

// TODO: I need to check more the inits of cURl : https://curl.se/libcurl/c/post-callback.html

EEStats::EEStats(std::string base_url, std::string lib_version) :
    _base_url(base_url), _lib_version(lib_version)
{
    showMessage("Init global cURL...", "EEStats");
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
        throw std::exception("Unable to init cURL!");
    if (_base_url.at(_base_url.length() - 1) != '/')
        _base_url += '/';
    showMessage("cURL global loaded!", "EEStats");
}

EEStats::~EEStats()
{
    showMessage("Clean-up global cURL...", "EEStats");
    curl_global_cleanup();
    showMessage("Cleaned-up global cURL!", "EEStats");
}

bool EEStats::askSessionId()
{
    std::stringstream args;

    // UID
    std::string uid = _cq->getUID();
    if (uid.empty()) {
        showMessage("Unable to ask a session id because the user uid is empty!", "EEStats", true);
        return false;
    }
    else if (uid.length() != 128) {
        showMessage("User uid seems invalid!", "EEStats", true);
        return false;
    }
    args << "user_uid=" << uid;

    // USERNAME
    std::string username = std::string(_gq->getUsername());
    if (!username.empty())
        args << "&username=" << username;

    // CDKEYS
    auto cdkeys = _gq->getCDKeys();
    if (!cdkeys.empty()) {
        args << "&cdkeys=";
        for (auto iter = cdkeys.begin(); iter != cdkeys.end(); )
        {
            args << iter->first << ":" << iter->second;
            if (++iter != cdkeys.end())
                args << ";";
        }
    }

#ifdef _DEBUG
    std::cout << args.str() << std::endl;
#endif

    auto user_request = sendRequest("user.php", "POST", args.str());
    if (!user_request.first)
        return false;
    if (user_request.second.first < 200 || user_request.second.first >= 300) {
        showMessage("Invalid HTTP code when asking a session id!", "EEStats", true);
        return false;
    }
    _session_id = user_request.second.second;
    return !_session_id.empty();
}

bool EEStats::sendSessionInfos()
{
    if (_session_id.empty()) {
        showMessage("Unable to send the session infos because the session uuid is empty!", "EEStats", true);
        return false;
    }

    GameQuery* gq = getGameQuery();
    ComputerQuery* cq = getComputerQuery();

    std::stringstream args;
    args << "session_uuid=" << _session_id;

    // Game
    showMessage("Recovering Game Infos...", "EEStats");
    args << "&game_base_version=" << gq->getGameBaseVersion();
    args << "&game_data_version=" << gq->getGameDataVersion();
    args << "&game_is_expansion=" << ((gq->getProductType() == GameQuery::ProductType::PT_AoC) ? "1" : "0");
    args << "&game_is_elevated=" << gq->isElevated() ? "1" : "0";

    // WON
    showMessage("Recovering WON Infos...", "EEStats");
    auto won_product = gq->getWONProductName();
    if (!won_product.empty())
        args << "&won_product_name=" << won_product;
    auto won_product_dir = gq->getWONProductDirectory();
    if (!won_product_dir.empty())
        args << "&won_product_directory=" << won_product_dir;
    auto won_product_ver = gq->getWONProductVersion();
    if (!won_product_ver.empty())
        args << "&won_product_version=" << won_product_ver;

    // GPU
    showMessage("Recovering GPU Infos...", "EEStats");
    auto dev = cq->getGraphicDeviceId();
    if (!dev.empty())
        args << "&gpu_device=" << dev;
    auto vend = cq->getGraphicVendorId();
    if (!vend.empty())
        args << "&gpu_vendor=" << vend;
    auto gpu_name = cq->getGraphicName();
    if (!gpu_name.empty()) {
        trim(gpu_name);
        args << "&gpu_name=" << gpu_name;
    }
    auto gpu_version = cq->getGraphicVersion();
    if (!gpu_version.empty())
        args << "&gpu_version=" << gpu_version;
    auto gpu_refresh_rate = cq->getGraphicRefreshRate();
    if (gpu_refresh_rate > 1)
        args << "&gpu_refresh_rate=" << gpu_refresh_rate;
    auto gpu_bits_per_pixel = cq->getGraphicBitsPerPixel();
    if (gpu_bits_per_pixel != 0)
        args << "&gpu_bits_per_pixel=" << gpu_bits_per_pixel;
    auto gpu_dedicated_memory = cq->getGraphicDedicatedMemory();
    if (gpu_dedicated_memory != 0)
        args << "&gpu_dedicated_memory=" << extremRound(gpu_dedicated_memory);

    // CPU
    showMessage("Recovering CPU Infos...", "EEStats");
    auto cpu_id = cq->getProcessorId();
    if (!cpu_id.empty())
        args << "&cpu_id=" << cpu_id;
    auto cpu_name = cq->getProcessorName();
    if (!cpu_name.empty()) {
        trim(cpu_name);
        args << "&cpu_name=" << cpu_name;
    }
    auto cpu_nb_cores = cq->getProcessorNumberOfCores();
    if (cpu_nb_cores > 0)
        args << "&cpu_nb_cores=" << cpu_nb_cores;
    auto cpu_arch = cq->getProcessorArch();
    if (!cpu_arch.empty())
        args << "&cpu_architecture=" << cpu_arch;

    // RAM
    showMessage("Recovering RAM Infos...", "EEStats");
    auto ram_size = extremRound(cq->getRAM());
    if (!ram_size.empty())
        args << "&ram_size=" << ram_size;

    // OS
    showMessage("Recovering OS Infos...", "EEStats");
    std::string os_name = cq->isWine() ? "Wine" : cq->getWindowsName();
    if (!os_name.empty()) {
        trim(os_name);
        args << "&os_name=" << os_name;
    }
    auto os_version = cq->isWine() ? (cq->getWineVersion() == NULL ? "" : cq->getWineVersion()) : cq->getWindowsVersion();
    if (!os_version.empty())
        args << "&os_version=" << os_version;
    auto os_locale = cq->getWindowsLocale();
    if (!os_locale.empty())
        args << "&os_locale=" << os_locale;
    args << "&os_virtual_machine=" << cq->runInVM() ? "1" : "0";

    // Screen
    showMessage("Recovering Screen Infos...", "EEStats");
    auto size = cq->getWindowsResolution();
    auto screen_resolution = std::to_string(size.cx) + "x" + std::to_string(size.cy);
    if (!screen_resolution.empty())
        args << "&screen_resolution=" << screen_resolution;

    // DX Wrapper
    showMessage("Recovering DirectX and DirectX Wrapper...", "EEStats");
    if (!cq->isWine()) {
        auto major = cq->getDirectX_MajorVersion();
        if (major != 0)
            args << "&dx_major_version=" << major;
    }
    auto wrapper = cq->getDirectX_WrapperVersion();
    if (!wrapper.empty())
        args << "&dx_wrapper_version=" << wrapper;
    auto params = cq->getDirectX_WrapperParams();
    if (!params.empty())
        args << "&dx_wrapper_settings=" << params;

#ifdef _DEBUG
    std::cout << args.str() << std::endl;
#endif

    auto user_request = sendRequest("session.php", "POST", args.str());
    if (!user_request.first)
        return false;
    if (user_request.second.first < 200 || user_request.second.first >= 300) {
        showMessage("Invalid HTTP code when sending session infos!", "EEStats", true);
        return false;
    }
    return true;
}


bool EEStats::sendPerformanceInfos(int fps_average, std::string played_time)
{
    if (_session_id.empty()) {
        showMessage("Unable to send the performance infos because the session uuid is empty!", "EEStats", true);
        return false;
    }
    else if (fps_average <= 0) {
        showMessage("Unable to send the performance infos because the average fps is invalid!", "EEStats", true);
        return false;
    }
    else if (played_time.empty() || played_time.compare("0:0:0") == 0) {
        showMessage("Unable to send the performance infos because the played time is invalid!", "EEStats", true);
        return false;
    }

    GameQuery* gq = getGameQuery();
    ComputerQuery* cq = getComputerQuery();

    std::stringstream args;
    args << "session_uuid=" << _session_id;

    args << "&played_time=" << played_time;

    showMessage("Recovering Game Infos...", "EEStats");
    args << "&singleplayer=" << !gq->inLobby();

    auto params = gq->getGPURasterizerName();
    if (!params.empty()) {
        trim(params);
        args << "&rasterizer_name=" << params;
    }
    args << "&vsync=" << gq->isVSyncEnabled() ? "1" : "0";
    args << "&fps_average=" << fps_average;
    int bitsPerPixel = gq->getBitsPerPixel();
    if (bitsPerPixel != 0)
        args << "&bits_per_pixel=" << bitsPerPixel;

    SIZE menuRes = gq->getMenuResolution();
    if (menuRes.cx != 0 || menuRes.cy != 0)
        args << "&menu_resolution=" << menuRes.cx << "x" << menuRes.cy;
    SIZE gameRes = gq->getGameResolution();
    if (gameRes.cx != 0 || gameRes.cy != 0)
        args << "&game_resolution=" << gameRes.cx << "x" << gameRes.cy;

#ifdef _DEBUG
    std::cout << args.str() << std::endl;
#endif

    auto user_request = sendRequest("performance.php", "POST", args.str());
    if (!user_request.first)
        return false;
    if (user_request.second.first < 200 || user_request.second.first >= 300) {
        showMessage("Invalid HTTP code when sending performance infos!", "EEStats", true);
        return false;
    }
    return true;
}

bool EEStats::isReachable()
{
    auto user_request = sendRequest("", "GET");

    if (!user_request.first)
        return false;
    if (user_request.second.first < 200 || user_request.second.first >= 300) {
        showMessage("Invalid HTTP when trying to check if server is reachable!", "EEStats", true);
        return false;
    }
    return true;
}

bool EEStats::isUpToDate()
{
    std::stringstream args;
    args << "version=" << EES_VERSION_STR;

#ifdef _DEBUG
    std::cout << args.str() << std::endl;
#endif

    auto user_request = sendRequest("", "POST", args.str());

    if (!user_request.first)
        return false;
    if (user_request.second.first < 200 && user_request.second.first >= 300) {
        showMessage("Invalid HTTP code when checking if up to date!", "EEStats", true);
        return false;
    }
    return user_request.second.second.compare("true") == 0;
}

bool EEStats::downloadUpdate(std::wstring dllPath)
{
    auto user_request = sendRequest("", "GET");

    if (!user_request.first)
        return false;
    if (user_request.second.first < 200 || user_request.second.first >= 300) {
        showMessage("Invalid HTTP code when getting update url!", "EEStats", true);
        return false;
    }

    if (downloadFile(user_request.second.second, utf16ToUtf8(dllPath + L".update"))) {
        showMessage("Update downloaded!", "EEStats");
        return true;
    }
    else {
        showMessage("Unable to download update!", "EEStats", true);
        return false;
    }
}

bool EEStats::sendActivity(ScreenType screen_type, std::string time_spent)
{
    if (_session_id.empty()) {
        showMessage("Unable to send activity because the session uuid is empty!", "EEStats", true);
        return false;
    }
    std::stringstream args;
    args << "session_uuid=" << _session_id;
    args << "&screen_type=" << (uint16_t) screen_type;
    args << "&time_spent=" << time_spent;

#ifdef _DEBUG
    std::cout << args.str() << std::endl;
#endif

    auto user_request = sendRequest("activity.php", "POST", args.str());
    if (!user_request.first)
        return false;
    if (user_request.second.first < 200 || user_request.second.first >= 300) {
        showMessage("Invalid HTTP code when sending activity!", "EEStats", true);
        return false;
    }
    return true;
}

bool EEStats::sendPing()
{
    if (_session_id.empty()) {
        showMessage("Unable to send the session ping because the session uuid is empty!", "EEStats", true);
        return false;
    }

    std::stringstream args;
    args << "session_uuid=" << _session_id;

#ifdef _DEBUG
    std::cout << args.str() << std::endl;
#endif

    auto user_request = sendRequest("session.php", "PATCH", args.str());
    if (!user_request.first)
        return false;
    if (user_request.second.first < 200 || user_request.second.first >= 300) {
        showMessage("Invalid HTTP code when sending session ping!", "EEStats", true);
        return false;
    }
    return true;
}

std::string EEStats::getSessionId()
{
    return _session_id;
}

static size_t WriteData(void* ptr, size_t size, size_t nmemb, void* stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    return written;
}

bool EEStats::downloadFile(std::string path, std::string file)
{
    CURLcode res;
    CURL* curl_handle;
    FILE* pagefile;


    /* init the curl session */
    curl_handle = curl_easy_init();

    /* set URL to get here */
    curl_easy_setopt(curl_handle, CURLOPT_URL, path.c_str());

    /* disable progress meter, set to 0L to enable and disable debug output */
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteData);

#ifdef _DEBUG
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1); // Can make EE freeze even when running on another thread for some reasons...
#endif

    /* if redirected, tell libcurl to follow redirection */
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);


    /* open the file */
    errno_t hr = fopen_s(&pagefile, file.c_str(), "w+b");
    if (hr == 0) {
        /* write the page body to this file handle */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

        /* get it! */
        res = curl_easy_perform(curl_handle);            

        /* close the header file */
        fclose(pagefile);
    }

    curl_easy_cleanup(curl_handle);

    return hr == 0 ? res == CURLE_OK : false;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::pair<bool, std::pair<long, std::string>> EEStats::sendRequest(std::string path, std::string request_type, std::string params)
{
    CURL* curl;

    std::string replyTxt;
    long replyCode;

    if (request_type.empty()) {
        showMessage("Request type not defined!! Cancelling request...", "EEStats", true);
        return { false, { 0, "" } };
    }
    
    // cURL init and options
    curl = curl_easy_init();

    ToUpper(request_type);
    if (request_type.compare("POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
    }
    else {
        if (request_type.compare("GET") == 0)
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        else {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request_type.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
        }
    }
    std::string final_url = _base_url + path;
    replaceAll(final_url, " ", "%20");

    curl_easy_setopt(curl, CURLOPT_URL, final_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &replyTxt);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, ("EmpireEarthStats/" + _lib_version).c_str());
#ifdef _DEBUG
    // WARN: Using freopen_s to redirect to a file will crash
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif
    /* if redirected, tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::stringstream sscUrlFail;
        sscUrlFail << "curl_easy_perform() failed: " << curl_easy_strerror(res);
        showMessage(sscUrlFail.str(), "EEStats", true);
    }
    else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &replyCode);
        showMessage("Got response from server: HTTP Code: " +
            std::to_string(replyCode) +
            " | Reply length: " + std::to_string(replyTxt.length()), "EEStats");
#ifdef _DEBUG
        if (!replyTxt.empty())
            showMessage("Reply: " + replyTxt, "EEStats");
#endif
    }
    curl_easy_cleanup(curl);

    return { res == CURLE_OK, { replyCode, replyTxt }};
} 