#include "pch.h"

#include "EEStats.h"

// TODO: I need to check more the inits of cURl : https://curl.se/libcurl/c/post-callback.html

EEStats::EEStats(std::string base_url, std::string lib_version) :
    _base_url(base_url), _lib_version(lib_version)
{
    showMessage("Init global cURL...", "EEStats");
    curl_global_init(CURL_GLOBAL_ALL);
    if (_base_url.at(_base_url.length() - 1) != '/')
        _base_url += _base_url + '/';
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
        args << "&cdkeys=" << username;
        for (auto iter = cdkeys.begin(); iter != cdkeys.end(); )
        {
            args << iter->first << ":" << iter->second;
            if (++iter != cdkeys.end())
                args << ";";
        }
    }
    std::cout << args.str() << std::endl;
    auto user_request = sendRequest("user.php", args.str());
    if (user_request.first != CURLE_OK)
        return false;
    if (user_request.second.first != 201) {
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

    // WON
    std::string won_product = gq->getWONProductName();
    if (!won_product.empty())
        args << "&won_product_name=" << won_product;
    std::string won_product_dir = gq->getWONProductDirectory();
    if (!won_product_dir.empty())
        args << "&won_product_directory=" << won_product_dir;
    std::string won_product_ver = gq->getWONProductVersion();
    if (!won_product_ver.empty())
        args << "&won_product_version=" << won_product_ver;
    
    // Game Type
    args << "&is_expansion=" << ((gq->getProductType() == GameQuery::ProductType::PT_AoC) ? "1" : "0");

    // GPU
    std::string dev = cq->getGraphicDeviceId();
    if (!dev.empty())
        args << "&gpu_device=" << dev;
    std::string vend = cq->getGraphicVendorId();
    if (!dev.empty())
        args << "&gpu_vendor=" << vend;
    std::string gpu_name = cq->getGraphicName();
    if (!dev.empty())
        args << "&gpu_name=" << gpu_name;
    std::string gpu_version = cq->getGraphicVersion();
    if (!dev.empty())
        args << "&gpu_version=" << gpu_version;
    std::string gpu_refresh_rate = cq->getGraphicCurrentRefreshRate();
    if (!dev.empty())
        args << "&gpu_refresh_rate=" << gpu_version;
    std::string gpu_bits_per_pixel = cq->getGraphicCurrentBitsPerPixel();
    if (!dev.empty())
        args << "&gpu_bits_per_pixel=" << gpu_version;
    std::string gpu_dedicated_memory = cq->getGraphicDedicatedMemory();
    if (!dev.empty())
        args << "&gpu_dedicated_memory=" << gpu_version;

    // CPU
    std::string cpu_id = cq->getProcessorId();
    if (!dev.empty())
        args << "&cpu_id=" << gpu_version;
    std::string cpu_name = cq->getProcessorId();
    if (!dev.empty())
        args << "&cpu_name=" << cpu_name;
    std::string cpu_nb_cores = cq->getProcessorName();
    if (!dev.empty())
        args << "&cpu_nb_cores=" << cpu_nb_cores;

    // RAM
    std::string ram_size = std::to_string(cq->getRAM());
    if (!dev.empty())
        args << "&ram_size=" << ram_size;

    // OS
    std::string post_os_version = std::to_string(cq->getWindowsVersion());
    if (!dev.empty())
        args << "&$post_os_version=" << gpu_version;
    std::string post_os_locale = cq->getWindowsLocale();
    if (!dev.empty())
        args << "&post_os_locale=" << post_os_locale;

    // Screen
    auto size = cq->getWindowsResolution();
    std::string screen_resolution = std::to_string(size.cx) + "x" + std::to_string(size.cy);
    if (!dev.empty())
        args << "&screen_resolution=" << screen_resolution;



    std::cout << args.str() << std::endl;
    auto user_request = sendRequest("session.php", args.str());
    if (user_request.first != CURLE_OK)
        return false;
    if (user_request.second.first != 201) {
        showMessage("Invalid HTTP code when asking a session id!", "EEStats", true);
        return false;
    }
    return !_session_id.empty();
}

bool EEStats::isReachable()
{
    auto reply = sendRequest("");

    if (reply.first == CURLE_OK && reply.second.first >= 200 && reply.second.first <= 300)
        return true;
    return false;
}

bool EEStats::isUpToDate()
{
    return true;
}

bool EEStats::downloadUpdate()
{
    return true;
}

void EEStats::sendScreen(GameQuery::ScreenType screen_type)
{

}

void EEStats::sendPing()
{

}

std::string EEStats::getSessionId()
{
    return _session_id;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::pair<CURLcode, std::pair<long, std::string>> EEStats::sendRequest(std::string path, std::string post)
{
    CURL* curl;
    CURLcode res;

    std::string replyTxt;
    long replyCode;

    std::string final_url = _base_url + path;
    replaceAll(final_url, " ", "%20");

    // cURL init and options
    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, final_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &replyTxt);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, ("EmpireEarthStats/" + _lib_version).c_str());
#ifdef _DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1); // Can make EE freeze even when running on another thread for some reasons...
#endif
    if (!post.empty()) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
    }
    /* if redirected, tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
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

    return { res, { replyCode, replyTxt }};
} 