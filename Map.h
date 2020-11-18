#pragma once
#include "graph.h"

enum class PostTypes {
	NONE = 0,
	TOWN,
	MARKET,
	STORAGE
};

class Map : public Graph
{
private:
	std::vector<PostTypes> postsInfo;
public:
	Map(const std::string& jsonStructureData, const std::string& jsonCoordinatesData, const std::string& jsonDynamicData);
	void Draw(SdlWindow& window) override;
	void Update(const std::string& jsonDynamicData); // updated postsInfo
	~Map() override;
};