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
	Post(PostTypes type, size_t idx, const std::string& name, size_t pointIdx) : type{ type }, idx{ idx }, name{ name }, pointIdx{ pointIdx } {
	}
	double refillRate = 0.0;
	double goodsCapacity = 0.0;
	double goodsLoad = 0.0;
	double armorCapacity = 0.0;
	double armorLoad = 0.0;
	double populationCapacity = 0.0;
	double populationLoad = 0.0;
	PostTypes type;
	size_t idx;
	std::string name;
	size_t pointIdx;
};

class Map : public Graph {
private:
	TextureManager& textureManager;
	std::map<size_t, size_t> postIdxConverter;
	std::vector <Post> posts;
	std::unordered_set<int> markets;
	std::unordered_set<int> storages;
public:
	Map(const std::string& jsonStructureData, const std::string& jsonCoordinatesData, const std::string& jsonDynamicData, TextureManager& textureManager);
	int GetClosestMarket(int from);
	int GetBestMarket(int from, int homeIdx, double maxLoad, double distanceExtra, const std::unordered_set<int>& blackList = {});
	int GetBestStorage(int from, int homeIdx, double maxLoad, double distanceExtra, const std::unordered_set<int>& blackList = {});
	int GetArmor(int idx);
	void Draw(SdlWindow& window) override;
	void Update(const std::string& jsonDynamicData); // updated postsInfo
private:
	double GetMarketK(int from, int idx, int homeIdx, double maxLoad, double distanceExtra);
	double GetStorageK(int from, int idx, int homeIdx, double maxLoad, double distanceExtra);
};