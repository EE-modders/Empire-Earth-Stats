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


// EES VERSION
static const std::string EES_VERSION_STR = "1.0.1";
static const unsigned int EES_VERSION_MAJOR = 1;
static const unsigned int EES_VERSION_MINOR = 0;
static const unsigned int EES_VERSION_PATCH = 1;
// END EES VERSION

// EES HARD-CODED SETTINGS
const std::string EES_SETTINGS_URL = "https://stats.empireearth.eu/eestats";
// END EES HARD-CODED SETTINGS

#include <string>
#include <map>

class EEStats
{

public:
	EEStats(std::string base_url, std::string lib_version);
	~EEStats();

	// Yeah I could use the one of GameQuery but since I want GameQuery to be EES independant I will do
	// the boring translation to ensure it's independant
	enum ScreenType {
		EES_ST_Unknown = 0, EES_ST_Menu = 1, EES_ST_InGame_Singleplayer = 2, EES_ST_InGame_Multiplayer = 3, EES_ST_Lobby = 4,
		EES_ST_Scenario_Editor = 5
	};

	bool askSessionId();
	bool sendSessionInfos();
	bool sendPerformanceInfos(int fps_average, std::string played_time);
	bool sendActivity(ScreenType screen_type, std::string time_spent);
	// Keep the session open for session time, need to be send after at least one other query
	bool sendPing();

	bool isReachable();
	bool isUpToDate();
	bool downloadUpdate(std::wstring dllPath);

	std::string getSessionId();

	ComputerQuery* getComputerQuery()
	{
		return _cq.get();
	}

	GameQuery* getGameQuery()
	{
		return _gq.get();
	}

	bool downloadFile(std::string path, std::string file);

private:
	std::string _base_url;
	std::string _session_id;
	std::string _lib_version;

	std::unique_ptr<ComputerQuery> _cq = std::make_unique<ComputerQuery>();
	std::unique_ptr<GameQuery> _gq = std::make_unique<GameQuery>();


	std::pair<bool, std::pair<long, std::string>> sendRequest(std::string path, std::string request_type, std::string params = "");
	// Was for matomo std::string buildUserAgent(struct curl_slist*& headers);
};

