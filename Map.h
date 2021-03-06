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
	int level = 1;
	int nextLevelPrice = 0;
	PostTypes type;
	size_t idx;
	std::string name;
	size_t pointIdx;
};

class Map : public Graph {
private:
	TextureManager& textureManager;
	std::map<size_t, size_t> postIdxConverter;
	std::vector<Post> posts;
	std::unordered_set<int> markets;
	std::unordered_set<int> storages;
	std::unordered_set<int> towns;
public:
	Map(const std::string& jsonStructureData, const std::string& jsonCoordinatesData, const std::string& jsonDynamicData, TextureManager& textureManager);
	std::pair<int, double> GetBestMarket(int from, int home, double maxLoad, const std::unordered_set<int>& vBlackList, const std::unordered_set<edge> eBlackList, int dist = 0, int onPathTo = -1);
	std::pair<int, double> GetBestStorage(int from, int home, double maxLoad, const std::unordered_set<int>& vBlackList, const std::unordered_set<edge> eBlackList, int dist = 0, int onPathTo = -1);
	int GetArmor(int idx);
	int GetProduct(int idx);
	int GetLevel(int idx);
	int GetPopulation(int idx);
	int GetNextLevelPrice(int idx);
	int GetPostIdx(int idx);
	Post::PostTypes GetPostType(int idx);
	const std::unordered_set<int>& GetMarkets();
	const std::unordered_set<int>& GetStorages();
	const std::unordered_set<int>& GetTowns();
	void Draw(SdlWindow& window) override;
	void Update(const std::string& jsonDynamicData); // updated postsInfo
private:
	double GetMarketK(int from, int idx, int homeIdx, double maxLoad, const std::unordered_set<int>& vBlackList, const std::unordered_set<edge> eBlackList, int dist = 0, int onPathTo = -1);
	double GetStorageK(int from, int idx, int homeIdx, double maxLoad, const std::unordered_set<int>& vBlackList, const std::unordered_set<edge> eBlackList, int dist = 0, int onPathTo = -1);
};