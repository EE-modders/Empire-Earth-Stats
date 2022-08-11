#pragma once

#include <string>

class GameQuery
{
public:
	GameQuery(std::string game_path);

	enum ScreenType {
		Menu,
		PlayingSolo,
		PlayingOnline,
		Lobby,
		Editor,				// Unable to detect yet
		Unknown
	};

	bool isLoaded();

	bool isPlaying();
	bool inLobby();

	char* getUsername();
	
	ScreenType getCurrentScreen();

	char* getGameVersion();
	bool isSupportedVersion();

	void setVersionSuffix(std::string suffix);

private:
	std::string _game_path;

};

