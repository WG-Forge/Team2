#include "ServerConnection.h"
#include "json.h"
#include <iostream>
#include <sstream>

#pragma warning(disable : 4996)
using namespace Json;

ServerConnection::ServerConnection(const std::string& playerName) {
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, SERVER_ADDRESS);
	curl_easy_setopt(curl, CURLOPT_PORT, SERVER_PORT);
	
	std::string jsonName = "{\"name\":\"" + playerName + "\"}";
	RequestMessage request(Request::LOGIN, jsonName.length(), jsonName);
	ResponseMessage response = GetResponse(request);
	
	std::stringstream responseStream = std::stringstream(response.response);
	Json::Dict responseDocument = Json::Load(responseStream).GetRoot().AsMap();
	playerIdx = responseDocument["idx"].AsString();

}

ServerConnection::ResponseMessage ServerConnection::GetResponse(const RequestMessage& requestMessage) {
	std::string response;
	
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, [](void* data, size_t size, size_t n, void* userdata) {
		strcpy((char*)data, ((std::string*)userdata)->c_str());
		return size * n;
		});
	curl_easy_setopt(curl, CURLOPT_READDATA, (void*)&(requestMessage.request));
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, requestMessage.request.length());
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* data, size_t size, size_t n, void* buffer) {
		*(std::string*)buffer = (char*)data;
		return size * n;
	});
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);

	CURLcode retCode = curl_easy_perform(curl);
	if (retCode != CURLcode::CURLE_OK) {
		throw ErrorConnection("Error while connecting");
	}
	return ResponseMessage(response);

}

void ServerConnection::SendRequest(const ServerConnection::RequestMessage& requestMessage) {
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, [](void* data, size_t size, size_t n, void* userdata) {
		strcpy((char*)data, ((std::string*)userdata)->c_str());
		return size * n;
	});
	curl_easy_setopt(curl, CURLOPT_READDATA, (void*)&(requestMessage.request));
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, requestMessage.request.length());
	
	CURLcode retCode = curl_easy_perform(curl);
	if (retCode != CURLcode::CURLE_OK) {
		throw ErrorConnection("Error while connecting");
	}
}

std::string ServerConnection::GetPlayerIdx() {
	return playerIdx;
}

std::string ServerConnection::GetMapStaticObjects() {
	RequestMessage request{ Request::MAP, 11, "{\"layer\":0}" };
	return GetResponse(request).response;
}

std::string ServerConnection::GetMapDynamicObjects() {
	RequestMessage request{ Request::MAP, 11, "{\"layer\":1}" };
	return GetResponse(request).response;
}

std::string ServerConnection::GetMapCoordinates() {
	RequestMessage request{ Request::MAP, 12, "{\"layer\":10}" };
	return GetResponse(request).response;
}

std::string ServerConnection::GetGameState() {
	return "";
}

void ServerConnection::MoveTrain(size_t lineIdx, int speed, size_t trainIdx) {

}

void ServerConnection::Upgrade(std::vector<size_t> postIdxes, std::vector<size_t> trainIdxes) {

}

void ServerConnection::Turn() {

}

ServerConnection::~ServerConnection() {
	RequestMessage logoutRequest{ Request::LOGOUT, 0, "" };
	SendRequest(logoutRequest);
	curl_easy_cleanup(curl);
}

void ServerConnection::RequestMessage::ConvertNumberToReverseString(int num, char(&str)[8], int off) {
	int mul = 1 << 8;
	for (int i = 0; i <= 3; i++) {
		str[off + i] = num % mul;
		num /= mul;
	}
}

std::string ServerConnection::RequestMessage::ToString() {
	std::string res = "00000000" + request;
	char s[8];
	ConvertNumberToReverseString((int)actionCode, s, 0);
	ConvertNumberToReverseString((int)dataLength, s, 4);
	for (int i = 0; i < 8; i++) {
		res[i] = s[i];
	}
	return res;
}

ServerConnection::ResponseMessage::ResponseMessage(std::string responseMessage): response(responseMessage.substr(8)) {
	char resultStr[5];
	strcpy(resultStr, responseMessage.substr(0, 4).c_str());
	result = (Result) * (int*)resultStr;
	
	char dataLengthStr[5];
	strcpy(dataLengthStr, responseMessage.substr(4, 4).c_str());
	dataLength = *(int*)dataLengthStr;
}