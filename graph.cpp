#include "graph.h"
#include "json.h"
#include <cmath>
#include <fstream>

constexpr double PI = 3.141592653589793238463;
constexpr double xMiddle = 400;
constexpr double yMiddle = 300;
constexpr double r = std::min(xMiddle - 30, yMiddle - 30);

Graph::Graph(const std::string& filename) {
	std::ifstream in(filename);
	Json::Document document = Json::Load(in);
	auto nodeMap = document.GetRoot().AsMap();
	adjacencyList.reserve(nodeMap["points"].AsArray().size());
	std::map<size_t, size_t> idxConverter;
	double phi = 0;
	double phi_step = 2 * PI / nodeMap["points"].AsArray().size();
	for (const auto& vertexNode : nodeMap["points"].AsArray()) {
		auto vertexMap = vertexNode.AsMap();
		idxConverter[vertexMap["idx"].AsInt()] = adjacencyList.size();
		adjacencyList.push_back( {static_cast<size_t>(vertexMap["idx"].AsInt()), std::nullopt, std::list<Vertex::Edge>(),
			{xMiddle + r * std::cos(phi), yMiddle + r * std::sin(phi)} });
		if (!vertexMap["post_idx"].IsNull()) {
			adjacencyList.back().postIdx = static_cast<size_t>(vertexMap["post_idx"].AsInt());
		}
		phi += phi_step;
	}
	for (const auto& edgeNode : nodeMap["lines"].AsArray()) {
		auto edgeMap = edgeNode.AsMap();
		size_t from = idxConverter[edgeMap["points"].AsArray()[0].AsInt()];
		Vertex::Edge edge(edgeMap["idx"].AsInt(), idxConverter[edgeMap["points"].AsArray()[1].AsInt()], edgeMap["length"].AsDouble());
		AddEdge(from, edge);
		std::swap(from, edge.to);
		AddEdge(from, edge);
		maxLength = std::max(maxLength, edge.length);
	}
}

void Graph::AddEdge(size_t from, Vertex::Edge edge) {
	auto pos = std::find_if(begin(adjacencyList[from].edges), end(adjacencyList[from].edges), [edge](const Vertex::Edge& cur) {return cur.to < edge.to; });
	if (pos == end(adjacencyList[from].edges)) {
		adjacencyList[from].edges.push_back(edge);
	} else {
		adjacencyList[from].edges.insert(pos, edge);
	}
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
		double x = adjacencyList[i].point.x - xMiddle;
		double y = adjacencyList[i].point.y - yMiddle;

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

Graph::~Graph() {
}
