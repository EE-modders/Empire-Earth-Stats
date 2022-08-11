#pragma once

#include <string>

#define CURL_STATICLIB
#include "curl/curl.h"

#include "Utils.h"
#include "GameQuery.h"
#include "ComputerQuery.h"

#ifdef _DEBUG
#pragma comment (lib, "curl/libcurl_a_debug.lib")
#else
#pragma comment (lib, "curl/libcurl_a.lib")
#endif

#pragma comment (lib, "Normaliz.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Wldap32.lib")
#pragma comment (lib, "Crypt32.lib")
#pragma comment (lib, "advapi32.lib")

class MatomoEE
{

public:
	MatomoEE(std::string base_url, std::string site_id, std::string uid, std::string lib_version);
	~MatomoEE();

	void sendScreen(GameQuery::ScreenType screen_type);

	// Keep the session open for session time, need to be send after at least one other query
	void sendPing();

private:
	std::string _base_url;
	std::string _site_id;
	std::string _uid;
	std::string _lib_version;

	ComputerQuery* _cq;

	void sendRequest(std::string path);
	std::string buildUserAgent(struct curl_slist*& headers);
};

