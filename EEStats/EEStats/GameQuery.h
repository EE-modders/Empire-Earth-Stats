#pragma once

#include <string>
#include <map>

class GameQuery
{
public:

	GameQuery();

	enum ProductType {
		PT_Unknown,
		PT_EE,
		PT_AoC
	};

	enum ScreenType {
		ST_Unknown,
		ST_Menu,
		ST_PlayingSolo,
		ST_PlayingOnline,
		ST_Lobby,
		ST_ScenarioEditor
	};

	bool isLoaded();
	bool isPlaying();
	bool inLobby();

	bool isMinimized(); // WARNING: The debug console is also considered as the game Window
	bool isElevated();

	char* getUsername();

	std::map<std::string, std::string> getCDKeys();
	
	ScreenType getCurrentScreen();
	SIZE getGameResolution();
	SIZE getMenuResolution();
	int getBitsPerPixel();

	char* getGameBaseVersion();
	char* getGameDataVersion();
	bool isSupportedVersion();

	void setVersionSuffix(std::string suffix);

	// std::string getGameChecksum();
	ProductType getProductType();

	std::string getWONProductName();
	std::string getWONProductDirectory();
	std::string getWONProductVersion();

	std::string getGPURasterizerName();
	bool isVSyncEnabled();
	float getFPS(float updateInterval);

private:
	std::string _game_path;
	ProductType _productType;

};

