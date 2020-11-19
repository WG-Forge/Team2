#pragma once
#include "graph.h"

struct Event {};

struct Post {
	enum class PostTypes {
		NONE = 0,
		TOWN,
		MARKET,
		STORAGE
	};
	PostTypes type;
	int32_t armor;
	uint32_t armorCapacity;
	std::vector<Event> events; //maybe queue
	size_t idx;
	uint32_t level;
	std::string name;
	uint32_t nextLevelPrice;
	std::string playerIdx;
	size_t pointIdx;
	int32_t population;
	uint32_t populationCapacity;
	int32_t product;
	uint32_t productCapacity;
	uint32_t trainCooldown;
	uint32_t replenishment;
};

struct Train {
	uint32_t cooldown;
	std::vector<Event> events; // maybe queue
	int32_t fuel;
	uint32_t fuelCapacity;
	int32_t fuelConsumption;
	int32_t goods;
	uint32_t goodsCapacity;
	std::optional<std::string> goodsType;
	size_t idx;
	uint32_t level;
	size_t lineIdx;
	uint32_t nextLevelPrice;
	std::string playerIdx;
	int32_t position;
	int32_t speed;

};

struct PlayerRating {
	std::string idx;
	std::string name;
	int32_t rating;
};
class Map : public Graph
{
private:
	std::map<size_t, Post> posts;
	std::map<size_t, Train> trains;
	std::map<std::string, PlayerRating> ratings;
public:
	Map(const std::string& jsonStructureData, const std::string& jsonCoordinatesData, const std::string& jsonDynamicData);
	void Draw(SdlWindow& window) override;
	void Update(const std::string& jsonDynamicData); // updated postsInfo
	~Map() override;
};