#include "ServerConnection.h"
#include "json.h"
#include <sstream>

#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

constexpr char SERVER_ADDRESS[] = "wgforge-srv.wargaming.net";
constexpr Uint16 SERVER_PORT = 443;

ServerConnection::ServerConnection(const std::string& playerName) {
	EstablishConnection();
	SendMessage(Request::LOGIN, "{\"name\":\"" + playerName + "\"}");
	
	std::stringstream responseStream = std::stringstream(GetResponse());
	Json::Dict responseDocument = Json::Load(responseStream).GetRoot().AsMap();
	playerIdx = responseDocument["idx"].AsString();
}

std::string ServerConnection::GetPlayerIdx() {
	return playerIdx;
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
	try {
		SendMessage(Request::MAP, "{\"layer\":10}");
		std::string buf = GetResponse();
		std::cout << "COORDINATES RESPONCE IS WORKING NOW!!!" << std::endl;
		return buf;
	}
	catch (...) {
		return "{\"coordinates\": [], \"size\": [0,0]}"; // i guess server does not suppor it at the moment
	}
}

void ServerConnection::MoveTrain(size_t lineIdx, int speed, size_t trainIdx) {
	Json::Dict moveDict;
	moveDict["line_idx"] = static_cast<int>(lineIdx);
	moveDict["speed"] = speed;
	moveDict["train_idx"] = static_cast<int>(trainIdx);
	Json::Document doc{ moveDict };
	std::stringstream out;
	Json::Print(doc, out);
	std::string jsonRequest = out.str();
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
	char* frame = new char[8 + data.size()];
	Uint32 code = (Uint32)actionCode;
	size_t index = 0;
	unsigned char* frame_header = (unsigned char*)frame;
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
		frame[index] = data[i];
	}
	if (SDLNet_TCP_Send(socket, frame, 8 + data.size()) < 8 + data.size()) {
		throw std::runtime_error{ SDLNet_GetError() };
	}
}

std::string ServerConnection::GetResponse()
{
	Uint8 data[4];
	if (SDLNet_TCP_Recv(socket, data, 4) < 4) {
		throw std::runtime_error{ SDLNet_GetError() };
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
	if (SDLNet_TCP_Recv(socket, data, 4) < 4) {
		throw std::runtime_error{ SDLNet_GetError() };
	}
	Uint32 size = 0;
	for (int i = 3; i >= 0; --i) {
		size = (size << 8) | data[i];
	}
#ifdef _DEBUG
	std::cout << "size: ";
	for (int i = 0; i < 4; ++i) {
		std::cout << std::hex << std::setw(2) << (unsigned int)data[i] << ' ';
	}
	std::cout << std::endl;
	std::cout << std::dec;
	std::cout << "size = " << size << std::endl;
#endif

	char* response = new char[size + 1];
	char* writeBuf = response;
	response[size] = '\0';
	while (size > 0) {
		int got = SDLNet_TCP_Recv(socket, writeBuf, size);
#ifdef _DEBUG
		std::cout << "got = " << got << std::endl;
#endif
		size -= got;
		writeBuf += got;
	}

	if (buf != Result::OKEY) {
		delete[] response;
		throw buf;
	}

	std::string res{ response };
	delete[] response;
	return res;
}