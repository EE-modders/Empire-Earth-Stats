#pragma once

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

#include <string>
#include <map>

unsigned int __stdcall MainThread(void* data);
unsigned int __stdcall PingThread(void* data);

class EEStats
{

public:
	EEStats(std::string base_url, std::string lib_version);
	~EEStats();

	bool askSessionId();
	bool sendSessionInfos();

	bool isReachable();
	bool isUpToDate();
	bool downloadUpdate();

	void sendScreen(GameQuery::ScreenType screen_type);

	// Keep the session open for session time, need to be send after at least one other query
	void sendPing();

	std::string getSessionId();

	unsigned int MainThread();

	ComputerQuery* getComputerQuery()
	{
		return _cq.get();
	}

	GameQuery* getGameQuery()
	{
		return _gq.get();
	}

private:
	std::string _base_url;
	std::string _session_id;
	std::string _lib_version;

	std::unique_ptr<ComputerQuery> _cq = std::make_unique<ComputerQuery>();
	std::unique_ptr<GameQuery> _gq = std::make_unique<GameQuery>();

	std::pair<CURLcode, std::pair<long, std::string>> sendRequest(std::string path, std::string post = "");
	std::string buildUserAgent(struct curl_slist*& headers);
};

