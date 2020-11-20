#pragma once
#define CURL_STATICLIB
#include <string>
#include <curl/curl.h>
#include <vector>
using std::string;

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
	enum class Result
	{
		OKEY = 0,
		BAD_COMMAND = 1,
		RESOURCE_NOT_FOUND = 2,
		ACCESS_DENIED = 3,
		INAPPROPRIATE_GAME_STATE = 4,
		TIMEOUT = 5,
		INTERNAL_SERVER_ERROR = 500
	};
	const string SERVER_ADDRESS = "http://wgforge-srv.wargaming.net";
	const int SERVER_PORT = 443;
	struct RequestMessage
	{
		Request actionCode;
		size_t dataLength;
		string request;
		RequestMessage(Request actionCode, size_t dataLength, string request) :
			actionCode(actionCode), dataLength(dataLength), request(request) {}
		string ToString();
	};
	struct ResponseMessage
	{
		Result result;
		size_t dataLength;
		string response;
		ResponseMessage(Result result, size_t dataLength, string response) :
			result(result), dataLength(dataLength), response(response) {}
		ResponseMessage(string responseMessage);
	};
	CURL* curl;
	string playerIdx;
public:
	ServerConnection(const string& player_name); // performs login operation
	ResponseMessage GetResponse(const RequestMessage& request); // I will write later
	void SendRequest(const RequestMessage& request); // I will write later
	string GetMapStaticObjects();
	string GetMapDynamicObjects();
	string GetMapCoordinates();
	string GetGameState();
	void MoveTrain(size_t lineIdx, int speed, size_t trainIdx);
	void Upgrade(std::vector<size_t> postIdxes, std::vector<size_t> trainIdxes);
	void Turn();
	string GetPlayerIdx(); // should be initialized in constructor
	~ServerConnection(); // performs logout operation
};

