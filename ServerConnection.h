#pragma once
#include <string>
#include <vector>
#include <SDL_net.h>

class ServerConnection { 
private:
	enum class Request {
		LOGIN = 1,
		LOGOUT = 2,
		MOVE = 3,
		UPGRADE = 4,
		TURN = 5,
		PLAYER = 6,
		GAMES = 7,
		MAP = 10
	};

	TCPsocket socket;
	std::string playerIdx;
	int homeIdx;
public:
	enum class Result {
		OKEY = 0,
		BAD_COMMAND = 1,
		RESOURCE_NOT_FOUND = 2,
		ACCESS_DENIED = 3,
		INAPPROPRIATE_GAME_STATE = 4,
		TIMEOUT = 5,
		INTERNAL_SERVER_ERROR = 500
	};

	ServerConnection(const std::string& playerName); // performs login operation
	std::string GetMapStaticObjects();
	std::string GetMapDynamicObjects();
	std::string GetMapCoordinates();
	std::string GetGameState();

	int GetHomeIdx();
	std::string GetPlayerIdx();

	void MoveTrain(size_t lineIdx, int speed, size_t trainIdx);
	void EndTurn();
	void Upgrade(std::vector<size_t> postIdxes, std::vector<size_t> trainIdxes);

	~ServerConnection(); // performs logout operation
private:
	void EstablishConnection();
	void SendMessage(Request actionCode, const std::string& data);
	std::string GetResponse();
};

