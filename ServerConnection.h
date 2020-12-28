#pragma once
#include <string>
#include <vector>
#include <SDL_net.h>

class ServerConnection { 
protected:
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
	std::string login;
	std::string password;
	std::string gameName;
	int homeIdx;
	bool isStrong;
	bool isOriginal;
	bool isEstablished = true;
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

	ServerConnection(const std::string& playerName, int playerCount, const std::string& gameName, int numTurns = -1, bool isStrong = true);
	ServerConnection(const std::string& playerName, const std::string& playerPassword, const std::string& gameName, bool isStrong = false, bool toEstablish = true);
	ServerConnection(const ServerConnection& other) = delete;
	ServerConnection(ServerConnection&& other) noexcept;
	std::string GetMapStaticObjects();
	std::string GetMapDynamicObjects();
	std::string GetMapCoordinates();
	std::string GetGameState();
	std::string GetGameName();
	bool IsEstablished();
	void Establish();

	int GetHomeIdx();
	const std::string& GetPlayerIdx();
	const std::string& GetLogin();
	const std::string& GetPassword();

	void MoveTrain(size_t lineIdx, int speed, size_t trainIdx);
	void EndTurn();
	void Upgrade(std::vector<size_t> postIdxes, std::vector<size_t> trainIdxes);

	~ServerConnection(); // performs logout operation
private:
	void EstablishConnection();
	void SendMessage(Request actionCode, const std::string& data);
	std::string GetResponse();
};

