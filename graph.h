#pragma once

#include <vector>
#include <list>
#include <mutex>
#include <optional>
#include <atomic>
#include "SDL_window.h"

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
public:
    explicit Graph(const std::string& filename); // creates graph with points in circular layout from file with json data
    Graph(const std::string& jsonStructureData, const std::string& jsonCoordinatesData);
    int TranslateVertexIdx(int idx);
    int TranslateEdgeIdx(int idx);
    virtual void Draw(SdlWindow& window); // draws current graph
    double ApplyForce(); // applies forces to vertices
    virtual ~Graph();
private:
    void AddEdge(size_t from, Vertex::Edge edge);
};

