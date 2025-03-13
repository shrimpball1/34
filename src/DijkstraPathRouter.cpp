#include "DijkstraPathRouter.h"
#include "PathRouter.h"
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <any>

struct CDijkstraPathRouter::SImplementation{
    struct Edge{
        CPathRouter::TVertexID dest;
        double weight;
    };
    struct Vertex{
        std::any tag;
        std::vector<Edge> edges;
    };
    std::vector<Vertex> vertices;
};

CDijkstraPathRouter::CDijkstraPathRouter() : DImplementation(std::make_unique<SImplementation>()){}

CDijkstraPathRouter::~CDijkstraPathRouter() = default;

// return the number of vertices
std::size_t CDijkstraPathRouter::VertexCount() const noexcept{
    return DImplementation->vertices.size();
}

// add a vertex with a tag and return the vertex id
CPathRouter::TVertexID CDijkstraPathRouter::AddVertex(std::any tag) noexcept{
    DImplementation->vertices.push_back({ tag, {} });
    return DImplementation->vertices.size() - 1;
}


// return the tag of a vertex
std::any CDijkstraPathRouter::GetVertexTag(TVertexID id) const noexcept {
    if(id < DImplementation->vertices.size())
        return DImplementation->vertices[id].tag;
    return std::any();
}

// add an edge between two vertices with a weight
bool CDijkstraPathRouter::AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir) noexcept{
    if(src >= DImplementation->vertices.size() || dest >= DImplementation->vertices.size() || weight < 0){
        return false;
    }
    DImplementation->vertices[src].edges.push_back({ dest, weight });
    if(bidir){
        DImplementation->vertices[dest].edges.push_back({ src, weight });
    }
    return true;
}

bool CDijkstraPathRouter::Precompute(std::chrono::steady_clock::time_point deadline) noexcept{
    return true;
}

// find the shortest path between two vertices
double CDijkstraPathRouter::FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept{
    const double INF = std::numeric_limits<double>::max();
    std::size_t n = DImplementation->vertices.size();
    if(src >= n || dest >= n){
        return NoPathExists;
    }
    
    std::vector<double> dist(n, INF);
    std::vector<TVertexID> prev(n, CPathRouter::InvalidVertexID);
    using Node = std::pair<double, TVertexID>;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    
    dist[src] = 0;
    pq.push({0, src});
    
    // dijkstra's algorithm
    while(!pq.empty()){
        auto [d, u] = pq.top();
        pq.pop();
        if(d > dist[u]){
            continue;
        }
        if(u == dest){
            break;
        }
        for(auto &edge : DImplementation->vertices[u].edges){
            TVertexID v = edge.dest;
            double nd = d + edge.weight;
            if(nd < dist[v]){
                dist[v] = nd;
                prev[v] = u;
                pq.push({nd, v});
            }
        }
    }
    
    if(dist[dest] == INF)
        return NoPathExists;
    
    path.clear();
    for(TVertexID cur = dest; cur != CPathRouter::InvalidVertexID; cur = prev[cur])
        path.push_back(cur);
    std::reverse(path.begin(), path.end());
    return dist[dest];
}
