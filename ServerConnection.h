#pragma once
#include <string>

class ServerConnection { 
private:
	// smth
public:
	ServerConnection(const std::string& player_name); // performs login operation
	std::string GetMapStaticObjects();
	std::string GetMapDynamicObjects();
	std::string GetMapCoordinates();
	std::string GetPlayerIdx(); // should be initialized in constructor
	~ServerConnection(); // performs logout operation
};

