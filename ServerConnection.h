#pragma once
#define CURL_STATICLIB
#include <string>
#include <curl/curl.h>
#include <vector>

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
	enum class Result {
		OKEY = 0,
		BAD_COMMAND = 1,
		RESOURCE_NOT_FOUND = 2,
		ACCESS_DENIED = 3,
		INAPPROPRIATE_GAME_STATE = 4,
		TIMEOUT = 5,
		INTERNAL_SERVER_ERROR = 500
	};
	const std::string SERVER_ADDRESS = "http://wgforge-srv.wargaming.net";
	const int SERVER_PORT = 443;
	struct RequestMessage {
	private:
		void ConvertNumberToReverseString(int num, char(&str)[8], int off);
	public:
		Request actionCode;
		size_t dataLength;
		std::string request;
		RequestMessage(Request actionCode, size_t dataLength, std::string request) :
			actionCode{ actionCode }, dataLength{ dataLength }, request{ request } {}
		std::string ToString();
	};
	struct ResponseMessage {
		Result result;
		size_t dataLength;
		std::string response;
		ResponseMessage(Result result, size_t dataLength, std::string response) :
			result{ result }, dataLength{ dataLength }, response{ response } {}
		ResponseMessage(std::string responseMessage);
	};
	ResponseMessage GetResponse(const RequestMessage& request);
	void SendRequest(const RequestMessage& request);
	CURL* curl;
	std::string playerIdx;
public:
	class ErrorConnection : public std::exception {
	public:
		std::string message;
		ErrorConnection(const std::string& message) : message(message) { }
	};
	ServerConnection(const std::string& playerName); // performs login operation
	std::string GetMapStaticObjects();
	std::string GetMapDynamicObjects();
	std::string GetMapCoordinates();
	std::string GetGameState();
	void MoveTrain(size_t lineIdx, int speed, size_t trainIdx);
	void Upgrade(std::vector<size_t> postIdxes, std::vector<size_t> trainIdxes);
	void Turn();
	std::string GetPlayerIdx(); // should be initialized in constructor
	~ServerConnection(); // performs logout operation
};

