#include "graph.h"
#include "json.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <queue>
#ifdef _DEBUG
#include <iostream>
#endif

constexpr double PI = 3.141592653589793238463;
constexpr double X_MIDDLE = 400;
constexpr double Y_MIDDLE = 300;
constexpr double R = std::min(X_MIDDLE - 30, Y_MIDDLE - 30);

Graph::Graph(const std::string& filename) {
	std::ifstream in(filename);
	ParseStructure(in);
}

Graph::Graph(const std::string& jsonStructureData, const std::string& jsonCoordinatesData) {
	{
		std::stringstream ss;
		ss << jsonStructureData;
		ParseStructure(ss);
	}
	{
		std::stringstream ss;
		ss << jsonCoordinatesData;
		ParseCoordinates(ss);
	}
	spTrees.resize(adjacencyList.size());
#ifdef _DEBUG
	std::cout << "vertex count: " << adjacencyList.size() << std::endl;
#endif
}

int Graph::TranslateVertexIdx(size_t idx) const {
	return idxConverter.at(idx);
}

int Graph::GetEdgeIdx(int from, int to) {
	for (const auto& i : adjacencyList[from].edges) {
		if (i.to == to) {
			return i.idx;
		}
	}
	return 0;
}

int Graph::GetNextOnPath(int from, int to) {	
	if (spTrees[from].empty()) {
		GenerateSpTree(from);
	}

	return GetNextOnPath(spTrees[from], from, to);
}

std::pair<int, int> Graph::GetEdgeVertices(int originalEdgeIdx) {
	return edgesData[originalEdgeIdx];
}

double Graph::GetEdgeLength(int originalEdgeIdx) {
	for (const auto& j : adjacencyList[GetEdgeVertices(originalEdgeIdx).first].edges) {
		if (j.idx == originalEdgeIdx) {
			return j.length;
		}
	}
	return 0.0;
}

std::pair<double, double> Graph::GetPointCoord(int localPointIdx) {
	return std::pair<double, double>{adjacencyList[localPointIdx].point.x, adjacencyList[localPointIdx].point.y};
}

void Graph::AddEdge(size_t from, Vertex::Edge edge) {
	auto pos = std::find_if(begin(adjacencyList[from].edges), end(adjacencyList[from].edges), [edge](const Vertex::Edge& cur) {return cur.to < edge.to; });
	if (pos == end(adjacencyList[from].edges)) {
		adjacencyList[from].edges.push_back(edge);
	} else {
		adjacencyList[from].edges.insert(pos, edge);
	}
}

void Graph::GenerateSpTree(int origin) {
	spTrees[origin].assign(adjacencyList.size(), { -1, -1 });
	struct dijkstraData {
		int idx;
		int prev;
		double length;
	};
	auto comparator = [](const dijkstraData& lhs, const dijkstraData& rhs) {return lhs.length > rhs.length; };
	std::priority_queue<dijkstraData, std::vector<dijkstraData>, decltype(comparator)> dijkstra(comparator);
	dijkstra.push({ 0, -1, 0 });
	for (size_t i = 0; i < adjacencyList.size(); i++) {
		int cur = dijkstra.top().idx;
		while (spTrees[origin][cur].length != -1) {
			dijkstra.pop();
			cur = dijkstra.top().idx;
		}
		spTrees[origin][cur] = { dijkstra.top().prev, dijkstra.top().length };
		for (const auto& edge : adjacencyList[cur].edges) {
			if (spTrees[origin][edge.to].length == -1) {
				dijkstra.push({ static_cast<int>(edge.to), cur, spTrees[origin][cur].length + edge.length });
			}
		}
	}
}

int Graph::GetNextOnPath(const std::vector<spData>& spTree, int from, int to) {
	int ans = to;
	while (spTree[ans].prevVertex != from) {
		ans = spTree[ans].prevVertex;
	}
	return ans;
}

void Graph::Draw(SdlWindow& window) {
	writeLock.lock();
	for (int i = 0; i < adjacencyList.size(); ++i) {
		for (const auto& j : adjacencyList[i].edges) {
			if (j.to < i) {
				break;
			}
			unsigned char color = 255 * (maxLength - j.length + 1) / maxLength;
			window.SetDrawColor(color, color, color);
			window.DrawLine(std::round(adjacencyList[i].point.x), std::round(adjacencyList[i].point.y), std::round(adjacencyList[j.to].point.x), std::round(adjacencyList[j.to].point.y));
		}
	}
	window.SetDrawColor(255, 255, 255);
	for (const auto& i : adjacencyList) {
		window.DrawRectangle(std::round(i.point.x - 5), std::round(i.point.y - 5), std::round(i.point.x + 5), std::round(i.point.y + 5));
	}
	writeLock.unlock();
}

void Graph::DrawEdges(SdlWindow& window) {
	for (int i = 0; i < adjacencyList.size(); ++i) {
		for (const auto& j : adjacencyList[i].edges) {
			if (j.to < i) {
				break;
			}
			double k = (maxLength - j.length) / maxLength;
			k = 0.2 + k * 0.8;
			unsigned char color = 255 * k;
			window.SetDrawColor(color, color, color);
			window.DrawLine(std::round(adjacencyList[i].point.x), std::round(adjacencyList[i].point.y), std::round(adjacencyList[j.to].point.x), std::round(adjacencyList[j.to].point.y));
		}
	}
}

double Graph::ApplyForce() {
	constexpr double coulombsK = 10000.0;
	constexpr double moveK = 0.1;
	static double maxAllowedSquare = 500 * 500 / moveK;
	static std::vector<std::pair<double, double>> forces;
	if (forces.empty()) {
		forces.resize(adjacencyList.size());
	}

	maxAllowedSquare = std::pow(std::sqrt(maxAllowedSquare) * 0.999, 2);

	for (int i = 0; i < adjacencyList.size(); ++i) { // Coulomb's law
		for (int j = adjacencyList.size() - 1; j > i; --j) {
			double x = adjacencyList[i].point.x - adjacencyList[j].point.x;
			double y = adjacencyList[i].point.y - adjacencyList[j].point.y;

			double square = x * x + y * y;
			double k = coulombsK / (square * std::sqrt(square));
			double xForce = x * k;
			double yForce = y * k;

			forces[i].first += xForce;
			forces[i].second += yForce;
			forces[j].first -= xForce;
			forces[j].second -= yForce;
		}
	}

	for (int i = 0; i < adjacencyList.size(); ++i) { // push to the middle
		double x = adjacencyList[i].point.x - X_MIDDLE;
		double y = adjacencyList[i].point.y - Y_MIDDLE;

		double k = std::max(1.0, adjacencyList.size() / 500.0) / std::sqrt(x * x + y * y);
		forces[i].first -= x * k;
		forces[i].second -= y * k;
	}

	double maxSquare = 0;
	for (int i = 0; i < adjacencyList.size(); ++i) { // Hooke's law
		for (const auto& j : adjacencyList[i].edges) {
			if (j.to < i) {
				break;
			}

			double x = adjacencyList[i].point.x - adjacencyList[j.to].point.x;
			double y = adjacencyList[i].point.y - adjacencyList[j.to].point.y;

			double k = (maxLength + 1 - j.length) / 100.0;
			double xForce = x * k;
			double yForce = y * k;

			forces[i].first -= xForce;
			forces[i].second -= yForce;
			forces[j.to].first += xForce;
			forces[j.to].second += yForce;
		}

		double currentForceSquare = forces[i].first * forces[i].first + forces[i].second * forces[i].second;
		if (currentForceSquare > maxSquare) {
			maxSquare = currentForceSquare;
		}
	}
	
	if (maxSquare > maxAllowedSquare) {
		double k = std::sqrt(maxAllowedSquare) / std::sqrt(maxSquare);
		for (auto& i : forces) {
			i.first *= k;
			i.second *= k;
		}
	}

	double total = 0;
	writeLock.lock();
	for (int i = 0; i < adjacencyList.size(); ++i) {
		double distanceX = forces[i].first * moveK;
		double distanceY = forces[i].second * moveK;
		adjacencyList[i].point.x += distanceX;
		adjacencyList[i].point.y += distanceY;
		total += std::abs(distanceX) + std::abs(distanceY);
	}
	writeLock.unlock();
	return total;
}

void Graph::ParseStructure(std::istream& input) {
	Json::Document document = Json::Load(input);
	auto nodeMap = document.GetRoot().AsMap();
	adjacencyList.reserve(nodeMap["points"].AsArray().size());
	double phi = 0;
	double phi_step = 2 * PI / nodeMap["points"].AsArray().size();
	for (const auto& vertexNode : nodeMap["points"].AsArray()) {
		auto vertexMap = vertexNode.AsMap();
		idxConverter[vertexMap["idx"].AsInt()] = adjacencyList.size();
		adjacencyList.push_back({ static_cast<size_t>(vertexMap["idx"].AsInt()), std::nullopt, std::list<Vertex::Edge>(),
			{X_MIDDLE + R * std::cos(phi), Y_MIDDLE + R * std::sin(phi)} });
		if (!vertexMap["post_idx"].IsNull()) {
			adjacencyList.back().postIdx = static_cast<size_t>(vertexMap["post_idx"].AsInt());
		}
		phi += phi_step;
	}
	for (const auto& edgeNode : nodeMap["lines"].AsArray()) {
		auto edgeMap = edgeNode.AsMap();
		size_t from = TranslateVertexIdx(edgeMap["points"].AsArray()[0].AsInt());
		Vertex::Edge edge(edgeMap["idx"].AsInt(), TranslateVertexIdx(edgeMap["points"].AsArray()[1].AsInt()), edgeMap["length"].AsDouble());
		edgesData[edge.idx] = { from, edge.to };
		AddEdge(from, edge);
		std::swap(from, edge.to);
		AddEdge(from, edge);
		maxLength = std::max(maxLength, edge.length);
	}
}

void Graph::ParseCoordinates(std::istream& input) {
	Json::Document document = Json::Load(input);
	auto nodeMap = document.GetRoot().AsMap();
	for (const auto& node : nodeMap["coordinates"].AsArray()) {
		auto coordMap = node.AsMap();
		size_t curIdx = coordMap["idx"].AsInt();
		adjacencyList[curIdx].point.x = coordMap["x"].AsDouble();
		adjacencyList[curIdx].point.y = coordMap["y"].AsDouble();
	}
	auto sizeArray = nodeMap["size"].AsArray();
	width = sizeArray[0].AsDouble();
	height = sizeArray[1].AsDouble();
}
