#pragma once

#include <string>
#include <map>

class GameQuery
{
public:

	GameQuery();

	enum ProductType {
		PT_EE,
		PT_AoC,
		PT_Unknown
	};

	enum ScreenType {
		ST_Menu,
		ST_PlayingSolo,
		ST_PlayingOnline,
		ST_Lobby,
		ST_Editor,				// Unable to detect yet
		ST_Unknown
	};

	bool isLoaded();
	bool isPlaying();
	bool inLobby();

	char* getUsername();

	std::map<std::string, std::string> getCDKeys();
	
	ScreenType getCurrentScreen();

	char* getGameVersion();
	bool isSupportedVersion();

	void setVersionSuffix(std::string suffix);

	std::string getGameChecksum();
	ProductType getProductType();

	std::string getWONProductName();
	std::string getWONProductDirectory();
	std::string getWONProductVersion();

private:
	std::string _game_path;
	ProductType _productType;

};

