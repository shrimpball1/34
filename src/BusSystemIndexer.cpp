#include "BusSystemIndexer.h"
#include "BusSystem.h"
#include "StreetMap.h"
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

struct CBusSystemIndexer::SImplementation{
    std::vector<std::shared_ptr<CBusSystem::SStop>> sortedStops;
    std::vector<std::shared_ptr<CBusSystem::SRoute>> sortedRoutes;
    std::unordered_map<CStreetMap::TNodeID, std::shared_ptr<CBusSystem::SStop>> nodeToStop;
};

CBusSystemIndexer::CBusSystemIndexer(std::shared_ptr<CBusSystem> bussystem) : DImplementation(std::make_unique<SImplementation>())
{
    // traverse stops
    for(std::size_t i = 0; i < bussystem->StopCount(); ++i){
        auto stop = bussystem->StopByIndex(i);
        if(stop){
            DImplementation->sortedStops.push_back(stop);
            DImplementation->nodeToStop[stop->NodeID()] = stop;
        }
    }
    // sort stops by ID
    std::sort(DImplementation->sortedStops.begin(), DImplementation->sortedStops.end(), [](auto a, auto b) { return a->ID() < b->ID(); });
    
    // deal with routes
    for(std::size_t i = 0; i < bussystem->RouteCount(); ++i){
        auto route = bussystem->RouteByIndex(i);
        if (route)
            DImplementation->sortedRoutes.push_back(route);
    }
    // sort routes by name
    std::sort(DImplementation->sortedRoutes.begin(), DImplementation->sortedRoutes.end(), [](auto a, auto b){ return a->Name() < b->Name(); });
}

CBusSystemIndexer::~CBusSystemIndexer() = default;

std::size_t CBusSystemIndexer::StopCount() const noexcept{
    return DImplementation->sortedStops.size();
}

std::size_t CBusSystemIndexer::RouteCount() const noexcept{
    return DImplementation->sortedRoutes.size();
}

std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::SortedStopByIndex(std::size_t index) const noexcept{
    if (index < DImplementation->sortedStops.size())
        return DImplementation->sortedStops[index];
    return nullptr;
}

std::shared_ptr<CBusSystem::SRoute> CBusSystemIndexer::SortedRouteByIndex(std::size_t index) const noexcept{
    if (index < DImplementation->sortedRoutes.size())
        return DImplementation->sortedRoutes[index];
    return nullptr;
}

std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::StopByNodeID(TNodeID id) const noexcept{
    auto it = DImplementation->nodeToStop.find(id);
    if (it != DImplementation->nodeToStop.end())
        return it->second;
    return nullptr;
}

bool CBusSystemIndexer::RoutesByNodeIDs(TNodeID src, TNodeID dest, std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> &routes) const noexcept{
    auto srcStop = StopByNodeID(src);
    auto destStop = StopByNodeID(dest);
    if(!srcStop || !destStop){
        return false;
    }
    bool found = false;
    for (auto &route : DImplementation->sortedRoutes){
        bool srcFound = false;
        for(std::size_t i = 0; i < route->StopCount(); ++i){
            if(route->GetStopID(i) == srcStop->ID())
                srcFound = true;
            if(srcFound && route->GetStopID(i) == destStop->ID()){
                routes.insert(route);
                found = true;
                break;
            }
        }
    }
    return found;
}

bool CBusSystemIndexer::RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const noexcept{
    std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> routes;
    return RoutesByNodeIDs(src, dest, routes);
}
