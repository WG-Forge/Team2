#include "ServerConnection.h"
#include "json.h"
#include <sstream>
#include <random>

#ifdef _DEBUG
#ifdef NETWORK_DEBUG
#define _NETWORK_DEBUG
#endif
#endif

#ifdef _NETWORK_DEBUG
#include <iostream>
#include <iomanip>
#endif

constexpr char SERVER_ADDRESS[] = "wgforge-srv.wargaming.net";
constexpr Uint16 SERVER_PORT = 443;

std::string generateRandomPassword()
{
	std::random_device rnd{};
	std::mt19937 random{ rnd() };
	std::string result;
	for (int i = 0; i < 16; ++i) {
		result += 'A' + random() % ('Z' - 'A' + 1);
	}
	return result;
}

ServerConnection::ServerConnection(const std::string& playerName, bool isStrong) {
	this->isStrong = isStrong;
	isOriginal = isStrong;
	EstablishConnection();
	password = generateRandomPassword();
	login = playerName;
	gameName = "Game of " + login;
	SendMessage(Request::LOGIN, "{\"name\":\"" + login + "\", \"password\":\"" + password + "\"}");
	
	std::stringstream responseStream = std::stringstream(GetResponse());
	Json::Dict responseDocument = Json::Load(responseStream).GetRoot().AsMap();
	playerIdx = responseDocument["idx"].AsString();
	auto home = responseDocument["home"].AsMap();
	homeIdx = home["idx"].AsInt();
}

ServerConnection::ServerConnection(const std::string& playerName, int playerCount, const std::string& gameName, bool isStrong) {
	this->isStrong = isStrong;
	isOriginal = isStrong;
	EstablishConnection();
	password = generateRandomPassword();
	login = playerName;
	std::string req = "{\"name\":\"" + login + "\", \"password\":\"" + password + "\"";
	if (!gameName.empty()) {
		req += ", \"game\":\"" + gameName + "\"}";
		this->gameName = gameName;
	}
	else {
		req += ", \"num_players\":" + std::to_string(playerCount) + "}";
		this->gameName = "Game of " + login;
	}
	SendMessage(Request::LOGIN, req);

	std::stringstream responseStream = std::stringstream(GetResponse());
	Json::Dict responseDocument = Json::Load(responseStream).GetRoot().AsMap();
	playerIdx = responseDocument["idx"].AsString();
	auto home = responseDocument["home"].AsMap();
	homeIdx = home["idx"].AsInt();
}

ServerConnection::ServerConnection(const std::string& playerName, const std::string& playerPassword, bool isStrong, bool toEstablish) {
	this->isEstablished = toEstablish;
	this->isStrong = isStrong;
	isOriginal = isStrong;
	password = playerPassword;
	login = playerName;
	gameName = "Game of " + login;
	if (!toEstablish) {
		return;
	}
	EstablishConnection();
	SendMessage(Request::LOGIN, "{\"name\":\"" + login + "\", \"password\":\"" + password + "\"}");

	std::stringstream responseStream = std::stringstream(GetResponse());
	Json::Dict responseDocument = Json::Load(responseStream).GetRoot().AsMap();
	playerIdx = responseDocument["idx"].AsString();
	auto home = responseDocument["home"].AsMap();
	homeIdx = home["idx"].AsInt();
}

ServerConnection::ServerConnection(const std::string& playerName, const std::string& playerPassword, const std::string& gameName, bool isStrong, bool toEstablish) {
	this->isEstablished = toEstablish;
	this->isStrong = isStrong;
	isOriginal = isStrong;
	password = playerPassword;
	login = playerName;
	this->gameName = gameName;
	if (!toEstablish) {
		return;
	}
	EstablishConnection();
	SendMessage(Request::LOGIN, "{\"name\":\"" + login + "\", \"password\":\"" + password + "\"}");

	std::stringstream responseStream = std::stringstream(GetResponse());
	Json::Dict responseDocument = Json::Load(responseStream).GetRoot().AsMap();
	playerIdx = responseDocument["idx"].AsString();
	auto home = responseDocument["home"].AsMap();
	homeIdx = home["idx"].AsInt();
}

ServerConnection::ServerConnection(ServerConnection&& other) noexcept {
	socket = other.socket;
	playerIdx = std::move(other.playerIdx);
	login = std::move(other.login);
	password = std::move(other.password);
	homeIdx = other.homeIdx;
	isStrong = other.isStrong;
	isOriginal = other.isOriginal;
	isEstablished = other.isEstablished;
	gameName = other.gameName;
	other.isOriginal = false;
}

int ServerConnection::GetHomeIdx() {
	return homeIdx;
}

const std::string& ServerConnection::GetPlayerIdx() {
	return playerIdx;
}

const std::string& ServerConnection::GetLogin() {
	return login;
}

const std::string& ServerConnection::GetPassword() {
	return password;
}

std::string ServerConnection::GetGameName()
{
	return gameName;
}

std::string ServerConnection::GetGameState()
{
	SendMessage(Request::GAMES, "");
	return GetResponse();
}

bool ServerConnection::IsEstablished() {
	return isEstablished;
}

void ServerConnection::Establish() {
	EstablishConnection();
	isEstablished = true;
	SendMessage(Request::LOGIN, "{\"name\":\"" + login + "\", \"password\":\"" + password + "\", \"game\":\"" + gameName + "\"}");
	std::stringstream responseStream = std::stringstream(GetResponse());
	Json::Dict responseDocument = Json::Load(responseStream).GetRoot().AsMap();
	playerIdx = responseDocument["idx"].AsString();
	auto home = responseDocument["home"].AsMap();
	homeIdx = home["idx"].AsInt();
}

std::string ServerConnection::GetMapStaticObjects() {
	SendMessage(Request::MAP, "{\"layer\":0}");
	return GetResponse();
}

std::string ServerConnection::GetMapDynamicObjects() {
	SendMessage(Request::MAP, "{\"layer\":1}");
	return GetResponse();
}

std::string ServerConnection::GetMapCoordinates() {
	SendMessage(Request::MAP, "{\"layer\":10}");
	return GetResponse();
}

void ServerConnection::MoveTrain(size_t lineIdx, int speed, size_t trainIdx) {
	SendMessage(ServerConnection::Request::MOVE, 
		"{\"line_idx\": " + std::to_string(lineIdx) +
		",\"speed\": " + std::to_string(speed) +
		", \"train_idx\": " + std::to_string(trainIdx) + "}");
	GetResponse();
}

void ServerConnection::EndTurn() {
	SendMessage(Request::TURN, "");
	GetResponse();
}

void ServerConnection::Upgrade(std::vector<size_t> postIdxes, std::vector<size_t> trainIdxes) {
	Json::Dict upgradeDict;
	Json::Array posts;
	posts.reserve(postIdxes.size());
	for (size_t idx : postIdxes) {
		posts.emplace_back(static_cast<int> (idx));
	}
	Json::Array trains;
	trains.reserve(trainIdxes.size());
	for (size_t idx : trainIdxes) {
		trains.emplace_back(static_cast<int> (idx));
	}
	upgradeDict["posts"] = posts;
	upgradeDict["trains"] = trains;
	std::stringstream out;
	Json::Document doc{ upgradeDict };
	Json::Print(doc, out);
	std::string jsonRequest = out.str();
	SendMessage(Request::UPGRADE, jsonRequest);
	GetResponse();
}

ServerConnection::~ServerConnection() {
	if (!isEstablished) {
		return;
	}
	if (!isOriginal) {
		return;
	}
	if (isStrong) {
		SendMessage(Request::LOGOUT, "");
	}
	SDLNet_TCP_Close(socket);
}

void ServerConnection::EstablishConnection() {
	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, SERVER_ADDRESS, SERVER_PORT) == -1) {
		throw std::runtime_error{ SDLNet_GetError() };
	}
	socket = SDLNet_TCP_Open(&ip);
	if (!socket) {
		throw std::runtime_error{ SDLNet_GetError() };
	}
}

void ServerConnection::SendMessage(Request actionCode, const std::string& data) {
	char* inBuf = nullptr;
	int inBufSize = 0;
	inBuf = new char[8 + data.size()];
	inBufSize = 8 + data.size();
	Uint32 code = (Uint32)actionCode;
	size_t index = 0;
	unsigned char* frame_header = (unsigned char*)inBuf;
#ifdef _NETWORK_DEBUG
	std::cout << std::endl;
	std::cout << "------send-------" << std::endl;
	std::cout << "code: ";
#endif
	for (int i = 0; i < 4; ++i, ++index) {
#ifdef _NETWORK_DEBUG
		std::cout << std::hex << std::setw(2) << (unsigned int)(frame_header[index] = (unsigned char)(code & 0xFF)) << ' ';
#else
		frame_header[index] = (unsigned char)(code & 0xFF);
#endif
		code >>= 8;
	}
	Uint32 size = data.size();
#ifdef _NETWORK_DEBUG
	std::cout << std::endl;
	std::cout << "size: ";
#endif
	for (int i = 0; i < 4; ++i, ++index) {
#ifdef _NETWORK_DEBUG
		std::cout << std::hex << std::setw(2) << (unsigned int)(frame_header[index] = (unsigned char)(size & 0xFF)) << ' ';
#else
		frame_header[index] = (unsigned char)(size & 0xFF);
#endif
		size >>= 8;
	}
#ifdef _NETWORK_DEBUG
	std::cout << std::endl;
	std::cout << "data: " << data;
	std::cout << std::endl;
#endif
	for (int i = 0; i < data.size(); ++i, ++index) {
		inBuf[index] = data[i];
	}
	if (SDLNet_TCP_Send(socket, inBuf, 8 + data.size()) < 8 + data.size()) {
		delete[] inBuf;
		throw std::runtime_error{ SDLNet_GetError() };
	}
	delete[] inBuf;
}

std::string ServerConnection::GetResponse() {
	Uint8 data[8];
	{
		size_t left = 8;
		Uint8* buf = data;
		while (left > 0) {
			int got = SDLNet_TCP_Recv(socket, buf, 4);
			if (got < 0) {
				throw std::runtime_error{ SDLNet_GetError() };
			}
			buf += got;
			left -= got;
		}
	}

	Uint32 responseCode = 0;
	for (int i = 0; i < 4; ++i) {
		responseCode |= data[i] << i * 8;
	}
#ifdef _NETWORK_DEBUG
	std::cout << "-----recieve-----" << std::endl;
	std::cout << "code: ";
	for (int i = 0; i < 4; ++i) {
		std::cout << std::hex << std::setw(2) << (unsigned int)data[i] << ' ';
	}
	std::cout << std::endl;
#endif
	Result buf = Result::OKEY;
	switch (responseCode) {
	case 0:
		break;
	case 1:
		buf = Result::BAD_COMMAND;
		break;
	case 2:
		buf = Result::RESOURCE_NOT_FOUND;
		break;
	case 3:
		buf = Result::ACCESS_DENIED;
		break;
	case 4:
		buf = Result::INAPPROPRIATE_GAME_STATE;
		break;
	case 5:
		buf = Result::TIMEOUT;
		break;
	case 500:
		buf = Result::INTERNAL_SERVER_ERROR;
		break;
	}
	Uint32 size = 0;
	for (int i = 7; i >= 4; --i) {
		size = (size << 8) | data[i];
	}
#ifdef _NETWORK_DEBUG
	std::cout << "size: ";
	for (int i = 4; i < 8; ++i) {
		std::cout << std::hex << std::setw(2) << (unsigned int)data[i] << ' ';
	}
	std::cout << std::endl;
	std::cout << std::dec;
	std::cout << "size = " << size << std::endl;
#endif
	char* outBuf = new char[size + 1ull];
	char* writeBuf = outBuf;
	outBuf[size] = '\0';
	while (size > 0) {
		int got = SDLNet_TCP_Recv(socket, writeBuf, size);
#ifdef _NETWORK_DEBUG
		std::cout << "got = " << got << std::endl;
#endif
		size -= got;
		writeBuf += got;
	}

	std::string result = outBuf;

	if (buf != Result::OKEY) {
#ifdef _NETWORK_DEBUG
		std::cout << outBuf << std::endl;
#endif
		delete[] outBuf;
		throw std::runtime_error{ result };
	}
	delete[] outBuf;

	return result;
}