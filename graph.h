#pragma once
#include <vector>
#include <list>
#include <mutex>
#include <optional>
#include <atomic>
#include <map>
#include <unordered_set>
#include "SDL_window.h"

namespace std {
    template <> 
    struct hash<std::pair<int, int>> {
        inline size_t operator()(const std::pair<int, int>& v) const {
            std::hash<int> int_hasher;
            return int_hasher(v.first) ^ int_hasher(v.second);
        }
    };
}

class Graph { // class for working with graphs
protected:
    struct Vertex {
        struct Edge {
            Edge(size_t idx, size_t to, double length) :idx{ idx }, to { to }, length{ length } {}
            size_t idx;
            size_t to;
            double length;
        };
        struct Point {
            double x;
            double y;
        };
        size_t originalIdx;
        std::optional<size_t> postIdx;
        std::list<Edge> edges;
        Point point;
    };
    std::vector<Vertex> adjacencyList;
    double maxLength = 0;
    std::mutex writeLock;
    std::map<size_t, size_t> idxConverter;
    std::map<size_t, std::pair<size_t, size_t>> edgesData;
    struct spData {
        int prevVertex;
        double length;
    };
    mutable std::vector<std::vector<spData>> spTrees;
    double width;
    double height;
public:
    using edge = std::pair<int, int>;
    explicit Graph(const std::string& filename); // creates graph with points in circular layout from file with json data
    Graph(const std::string& jsonStructureData, const std::string& jsonCoordinatesData);
    int TranslateVertexIdx(size_t idx) const;
    int GetEdgeIdx(int from, int to) const;
    double GetDistance(int from, int to) const;
    std::optional<double> GetDistance(int from, int to, const std::unordered_set<int>& verticesBlackList, const std::unordered_set<edge>& edgesBlackList, int dist = 0, int onPathTo = -1) const; // no caching
    std::optional<int> GetNextOnPath(int from, int to, const std::unordered_set<int>& verticesBlackList, const std::unordered_set<edge>& edgesBlackList, int dist = 0, int onPathTo = -1) const; // no caching
    std::pair<int, int> GetEdgeVertices(int originalEdgeIdx) const; // returns local from-to idx pair
    double GetEdgeLength(int originalEdgeIdx) const; // returns length of edge
    std::pair<double, double> GetPointCoord(int localPointIdx) const; // returns x-y pair
    virtual void Draw(SdlWindow& window) = 0; // draws current graph
    void DrawEdges(SdlWindow& window);
    virtual ~Graph() = default;
private:
    void ParseStructure(std::istream& input);
    void ParseCoordinates(std::istream& input);
    void AddEdge(size_t from, Vertex::Edge edge);
    int GetNextOnPath(const std::vector<spData>& spTree, int from, int to) const;
    std::vector<spData> GenerateSpTree(int origin, const std::unordered_set<int>& verticesBlackList = {}, const std::unordered_set<edge>& edgesBlackList = {}) const;
};

