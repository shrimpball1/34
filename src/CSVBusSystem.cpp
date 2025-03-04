#include "CSVBusSystem.h"
#include "DSVReader.h"
#include <memory>
#include <vector>
#include <string>
#include <cstdlib>
#include <unordered_map>

struct CCSVBusSystem::SImplementation{
    std::vector<std::shared_ptr<CBusSystem::SStop>> stops;
    std::vector<std::shared_ptr<CBusSystem::SRoute>> routes;
};

class StopImpl : public CBusSystem::SStop{
public:
    CBusSystem::TStopID id;
    CStreetMap::TNodeID nodeID;
    
    StopImpl(CBusSystem::TStopID id, CStreetMap::TNodeID nodeID) : id(id), nodeID(nodeID){}

    // return stop id and node id
    virtual CBusSystem::TStopID ID() const noexcept override{
        return id;
    }
    
    virtual CStreetMap::TNodeID NodeID() const noexcept override{
        return nodeID;
    }
};

class RouteImpl : public CBusSystem::SRoute{
public:
    std::string name;
    std::vector<CBusSystem::TStopID> stopIDs;
    
    RouteImpl(const std::string &name) : name(name){}
    
    // return route name, the number of stops and stop id
    virtual std::string Name() const noexcept override{
        return name;
    }
    
    virtual std::size_t StopCount() const noexcept override{
        return stopIDs.size();
    }
    
    virtual CBusSystem::TStopID GetStopID(std::size_t index) const noexcept override{
        return (index < stopIDs.size()) ? stopIDs[index] : CBusSystem::InvalidStopID;
    }
};

CCSVBusSystem::CCSVBusSystem(std::shared_ptr<CDSVReader> stopsrc, std::shared_ptr<CDSVReader> routesrc) : DImplementation(std::make_unique<SImplementation>()){
    std::vector<std::string> row;
    
    // skip stops.csv header
    stopsrc->ReadRow(row);
    while (stopsrc->ReadRow(row)){
        if (row.size() < 2){
            continue;
        }
        CBusSystem::TStopID stopID = std::stoull(row[0]);
        CStreetMap::TNodeID nodeID = std::stoull(row[1]);
        DImplementation->stops.push_back(std::make_shared<StopImpl>(stopID, nodeID));
    }
    
    // skip routes.csv header
    routesrc->ReadRow(row);
    std::unordered_map<std::string, std::shared_ptr<RouteImpl>> routeMap;
    while (routesrc->ReadRow(row)){
        if (row.size() < 2){
            continue;
        }
        std::string routeName = row[0];
        CBusSystem::TStopID stopID = std::stoull(row[1]);
        if (routeMap.find(routeName) == routeMap.end()){
            auto newRoute = std::make_shared<RouteImpl>(routeName);
            newRoute->stopIDs.push_back(stopID);
            routeMap[routeName] = newRoute;
            DImplementation->routes.push_back(newRoute);
        }else{
            routeMap[routeName]->stopIDs.push_back(stopID);
        }
    }
}

CCSVBusSystem::~CCSVBusSystem() {}

std::size_t CCSVBusSystem::StopCount() const noexcept{
    return DImplementation->stops.size();
}

std::size_t CCSVBusSystem::RouteCount() const noexcept{
    return DImplementation->routes.size();
}

std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByIndex(std::size_t index) const noexcept{
    return (index < DImplementation->stops.size()) ? DImplementation->stops[index] : nullptr;
}

std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByID(CBusSystem::TStopID id) const noexcept{
    for (const auto &stop : DImplementation->stops){
        if (stop->ID() == id){
            return stop;
        }
    }
    return nullptr;
}

std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByIndex(std::size_t index) const noexcept{
    return (index < DImplementation->routes.size()) ? DImplementation->routes[index] : nullptr;
}

std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByName(const std::string &name) const noexcept{
    for(const auto &route : DImplementation->routes){
        if(route->Name() == name){
            return route;
        }
    }
    return nullptr;
}
