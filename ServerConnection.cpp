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


ServerConnection::ResponseMessage ServerConnection::GetResponse(const RequestMessage& request) {
	return ResponseMessage{ Result::OKEY, 0, "" };
}

void ServerConnection::SendRequest(const RequestMessage& request) {

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

ServerConnection::~ServerConnection() {
	RequestMessage logoutRequest{ Request::LOGOUT, 0, "" };
	SendRequest(logoutRequest);
	curl_easy_cleanup(curl);
}

std::string ServerConnection::RequestMessage::ToString() {
	return std::string((char*)&actionCode) + std::string((char*)&dataLength) + request;
}

ServerConnection::ResponseMessage::ResponseMessage(std::string responseMessage): response(responseMessage.substr(8)) {
	char resultStr[5];
	strcpy(resultStr, responseMessage.substr(0, 4).c_str());
	result = (Result) * (int*)resultStr;
	
	char dataLengthStr[5];
	strcpy(dataLengthStr, responseMessage.substr(4, 4).c_str());
	dataLength = *(int*)dataLengthStr;
}