#include "ServerConnection.h"
#include "json.h"
#include <sstream>
#include <random>

#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

constexpr char SERVER_ADDRESS[] = "wgforge-srv.wargaming.net";
constexpr Uint16 SERVER_PORT = 443;

char* inBuf = nullptr;
size_t inBufSize = 0;
char* outBuf = nullptr;
size_t outBufSize = 0;

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

ServerConnection::ServerConnection(const std::string& playerName) {
	EstablishConnection();
	SendMessage(Request::LOGIN, "{\"name\":\"" + playerName + "\", \"password\":\"" + generateRandomPassword() + "\"}");
	
	std::stringstream responseStream = std::stringstream(GetResponse());
	Json::Dict responseDocument = Json::Load(responseStream).GetRoot().AsMap();
	playerIdx = responseDocument["idx"].AsString();
	auto home = responseDocument["home"].AsMap();
	homeIdx = home["idx"].AsInt();
}

int ServerConnection::GetHomeIdx() {
	return homeIdx;
}

std::string ServerConnection::GetPlayerIdx() {
	return playerIdx;
}

std::string ServerConnection::GetGameState()
{
	SendMessage(Request::GAMES, "");
	return GetResponse();
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
	std::string buf;
	try {
		SendMessage(Request::MAP, "{\"layer\":10}");
		buf = GetResponse();
	}
	catch (...) {
		return "{\"coordinates\": [], \"size\": [0,0]}"; // i guess server does not suppor it at the moment
	}
	throw std::runtime_error{ "Tell Pasha that layer 10 is working now" };
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
}

ServerConnection::~ServerConnection() {
	SendMessage(Request::LOGOUT, "");
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
	if (inBufSize < 8 + data.size()) {
		if (inBuf) {
			delete[] inBuf;
		}
		inBuf = new char[8 + data.size()];
		inBufSize = 8 + data.size();
	}
	Uint32 code = (Uint32)actionCode;
	size_t index = 0;
	unsigned char* frame_header = (unsigned char*)inBuf;
#ifdef _DEBUG
	std::cout << std::endl;
	std::cout << "------send-------" << std::endl;
	std::cout << "code: ";
#endif
	for (int i = 0; i < 4; ++i, ++index) {
#ifdef _DEBUG
		std::cout << std::hex << std::setw(2) << (unsigned int)(frame_header[index] = (unsigned char)(code & 0xFF)) << ' ';
#else
		frame_header[index] = (unsigned char)(code & 0xFF);
#endif
		code >>= 8;
	}
	Uint32 size = data.size();
#ifdef _DEBUG
	std::cout << std::endl;
	std::cout << "size: ";
#endif
	for (int i = 0; i < 4; ++i, ++index) {
#ifdef _DEBUG
		std::cout << std::hex << std::setw(2) << (unsigned int)(frame_header[index] = (unsigned char)(size & 0xFF)) << ' ';
#else
		frame_header[index] = (unsigned char)(size & 0xFF);
#endif
		size >>= 8;
	}
#ifdef _DEBUG
	std::cout << std::endl;
	std::cout << "data: " << data;
	std::cout << std::endl;
#endif
	for (int i = 0; i < data.size(); ++i, ++index) {
		inBuf[index] = data[i];
	}
	if (SDLNet_TCP_Send(socket, inBuf, 8 + data.size()) < 8 + data.size()) {
		throw std::runtime_error{ SDLNet_GetError() };
	}
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
#ifdef _DEBUG
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
#ifdef _DEBUG
	std::cout << "size: ";
	for (int i = 4; i < 8; ++i) {
		std::cout << std::hex << std::setw(2) << (unsigned int)data[i] << ' ';
	}
	std::cout << std::endl;
	std::cout << std::dec;
	std::cout << "size = " << size << std::endl;
#endif

	if (outBufSize < size + 1ull) {
		if (outBuf) {
			delete[] outBuf;
		}
		outBuf = new char[size + 1ull];
		outBufSize = size + 1ull;
	}
	char* writeBuf = outBuf;
	outBuf[size] = '\0';
	while (size > 0) {
		int got = SDLNet_TCP_Recv(socket, writeBuf, size);
#ifdef _DEBUG
		std::cout << "got = " << got << std::endl;
#endif
		size -= got;
		writeBuf += got;
	}

	if (buf != Result::OKEY) {
#ifdef _DEBUG
		std::cout << outBuf << std::endl;
#endif
		std::string err = outBuf;
		throw std::runtime_error{ err };
	}

	return outBuf;
}