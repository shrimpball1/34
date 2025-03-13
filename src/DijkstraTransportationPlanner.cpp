#include "DijkstraTransportationPlanner.h"
#include "OpenStreetMap.h"
#include "GeographicUtils.h"
#include "DijkstraPathRouter.h"
#include "BusSystemIndexer.h"
#include "StringUtils.h"
#include <memory>
#include <vector>
#include <limits>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <stdexcept>
#include <iomanip>
#include <cmath>


using TNodeID = CTransportationPlanner::TNodeID;
using TTripStep = CTransportationPlanner::TTripStep;

// implementation of the transportation planner
struct CDijkstraTransportationPlanner::SImplementation{
    std::shared_ptr<CTransportationPlanner::SConfiguration> config;
    std::shared_ptr<CStreetMap> streetMap;
    std::shared_ptr<CDijkstraPathRouter> router;
};

CDijkstraTransportationPlanner::CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration> config) : DImplementation(std::make_unique<SImplementation>()){
    DImplementation->config = config;
    DImplementation->streetMap = config->StreetMap();

    DImplementation->router = std::make_shared<CDijkstraPathRouter>();

    // get all nodes, sort them by ID, and add them to the router
    std::vector<std::shared_ptr<CStreetMap::SNode>> allNodes;
    std::size_t n = DImplementation->streetMap->NodeCount();
    for(std::size_t i = 0; i < n; ++i){
        allNodes.push_back(DImplementation->streetMap->NodeByIndex(i));
    }
    std::sort(allNodes.begin(), allNodes.end(), [](auto a, auto b) { return a->ID() < b->ID();});
    std::unordered_map<TNodeID, CPathRouter::TVertexID> mapping;
    for(auto &node : allNodes){
        CPathRouter::TVertexID vid = DImplementation->router->AddVertex(node->ID());
        mapping[node->ID()] = vid;
    }
    // add way edges
    std::size_t wayCount = DImplementation->streetMap->WayCount();
    for(std::size_t i = 0; i < wayCount; ++i){
        auto way = DImplementation->streetMap->WayByIndex(i);
        for(std::size_t j = 0; j + 1 < way->NodeCount(); ++j){
            TNodeID src = way->GetNodeID(j);
            TNodeID dest = way->GetNodeID(j+1);
            auto nodeSrc = DImplementation->streetMap->NodeByID(src);
            auto nodeDest = DImplementation->streetMap->NodeByID(dest);
            if(nodeSrc && nodeDest){
                double distance = SGeographicUtils::HaversineDistanceInMiles(nodeSrc->Location(), nodeDest->Location());
                bool oneway = false;
                if(way->HasAttribute("oneway") && way->GetAttribute("oneway") == "yes"){
                    oneway = true;
                }
                DImplementation->router->AddEdge(mapping[src], mapping[dest], distance, !oneway);
            }
        }
    }
}

CDijkstraTransportationPlanner::~CDijkstraTransportationPlanner() = default;

std::size_t CDijkstraTransportationPlanner::NodeCount() const noexcept{
    return DImplementation->streetMap->NodeCount();
}

std::shared_ptr<CStreetMap::SNode> CDijkstraTransportationPlanner::SortedNodeByIndex(std::size_t index) const noexcept {
    std::vector<std::shared_ptr<CStreetMap::SNode>> nodes;
    std::size_t n = DImplementation->streetMap->NodeCount();
    for(std::size_t i = 0; i < n; ++i){
        nodes.push_back(DImplementation->streetMap->NodeByIndex(i));
    }
    std::sort(nodes.begin(), nodes.end(), [](auto a, auto b) { return a->ID() < b->ID();});
    if(index < nodes.size()){
        return nodes[index];
    }
    return nullptr;
}

double CDijkstraTransportationPlanner::FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path){
    // reset path
    std::vector<std::shared_ptr<CStreetMap::SNode>> nodes;
    std::size_t n = DImplementation->streetMap->NodeCount();
    for (std::size_t i = 0; i < n; i++){
        nodes.push_back(DImplementation->streetMap->NodeByIndex(i));
    }
    std::sort(nodes.begin(), nodes.end(), [](auto a, auto b){
        return a->ID() < b->ID();
    });
    std::unordered_map<TNodeID, CPathRouter::TVertexID> mapping;
    for (std::size_t i = 0; i < n; i++){
        mapping[nodes[i]->ID()] = i;
    }
    std::vector<CPathRouter::TVertexID> vertexPath;
    double distance = DImplementation->router->FindShortestPath(mapping[src], mapping[dest], vertexPath);
    path.clear();
    for (auto vid : vertexPath){
        std::any tag = DImplementation->router->GetVertexTag(vid);
        TNodeID nodeID = std::any_cast<TNodeID>(tag);
        path.push_back(nodeID);
    }
    return distance;
}

double CDijkstraTransportationPlanner::FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path){
    std::shared_ptr<CBusSystem> busSys = DImplementation->config->BusSystem();
    CBusSystemIndexer indexer(busSys);

    // calculate bus path
    long double busTimeLD = CPathRouter::NoPathExists;
    std::vector<TTripStep> busPath;
    if (indexer.RouteBetweenNodeIDs(src, dest)) {
        //add bus vertices
        CDijkstraPathRouter busRouter;
        std::unordered_map<TNodeID, CPathRouter::TVertexID> busMapping;
        for(std::size_t i = 0; i < busSys->StopCount(); ++i){
            auto stop = busSys->StopByIndex(i);
            if(stop){
                TNodeID nodeID = stop->NodeID();
                CPathRouter::TVertexID vid = busRouter.AddVertex(nodeID);
                busMapping[nodeID] = vid;
            }
        }
        // add bus edges
        for(std::size_t i = 0; i < busSys->RouteCount(); ++i){
            auto route = busSys->RouteByIndex(i);
            if(!route){
                continue;
            }
            for(std::size_t j = 0; j + 1 < route->StopCount(); ++j){
                int stopId1 = route->GetStopID(j);
                int stopId2 = route->GetStopID(j+1);
                std::shared_ptr<CBusSystem::SStop> stop1 = nullptr, stop2 = nullptr;
                for(std::size_t k = 0; k < busSys->StopCount(); ++k){
                    auto s = busSys->StopByIndex(k);
                    if(s && s->ID() == stopId1){
                        stop1 = s;
                    }
                    if(s && s->ID() == stopId2){
                        stop2 = s;
                    }
                }
                if(!stop1 || !stop2){
                    continue;
                }
                TNodeID node1 = stop1->NodeID();
                TNodeID node2 = stop2->NodeID();
                if (busMapping.find(node1) == busMapping.end() || busMapping.find(node2) == busMapping.end()){
                    continue;
                }
                auto nodePtr1 = DImplementation->streetMap->NodeByID(node1);
                auto nodePtr2 = DImplementation->streetMap->NodeByID(node2);
                if(!nodePtr1 || !nodePtr2){
                    continue;
                }
                double distance = SGeographicUtils::HaversineDistanceInMiles(nodePtr1->Location(), nodePtr2->Location());
                double speed = DImplementation->config->DefaultSpeedLimit(); // 默认值
                std::size_t wayCount = DImplementation->streetMap->WayCount();
                for(std::size_t w = 0; w < wayCount; w++){
                    auto way = DImplementation->streetMap->WayByIndex(w);
                    for(std::size_t p = 0; p + 1 < way->NodeCount(); p++){
                        TNodeID a = way->GetNodeID(p);
                        TNodeID b = way->GetNodeID(p+1);
                        if((a == node1 && b == node2) || (a == node2 && b == node1)){
                            if(way->HasAttribute("maxspeed")){
                                try{
                                    std::string speedStr = way->GetAttribute("maxspeed");
                                    speedStr.erase(std::remove_if(speedStr.begin(), speedStr.end(),
                                                                  [](char c){ return !std::isdigit(c) && c != '.'; }), speedStr.end());
                                    speed = std::stod(speedStr);
                                } catch(...){ }
                            }
                            goto addBusEdge;
                        }
                    }
                }
            addBusEdge:
                double travelTime = distance / speed;
                busRouter.AddEdge(busMapping[node1], busMapping[node2], travelTime, false);
            }
        }
        std::vector<CPathRouter::TVertexID> vertexPath;
        double t = busRouter.FindShortestPath(busMapping[src], busMapping[dest], vertexPath);
        if (t != CPathRouter::NoPathExists) {
            // add 1 minute for waiting time
            long double tLD = static_cast<long double>(t);
            busTimeLD = tLD + (60.0L / 3600.0L);
            busPath.clear();
            if(!vertexPath.empty()){
                std::any tag = busRouter.GetVertexTag(vertexPath.front());
                TNodeID nodeID = std::any_cast<TNodeID>(tag);
                busPath.push_back(std::make_pair(CTransportationPlanner::ETransportationMode::Walk, nodeID));
                for(size_t i = 1; i < vertexPath.size(); i++){
                    std::any tag = busRouter.GetVertexTag(vertexPath[i]);
                    TNodeID nodeID = std::any_cast<TNodeID>(tag);
                    busPath.push_back(std::make_pair(CTransportationPlanner::ETransportationMode::Bus, nodeID));
                }
            }
        }
    }
    
    // calculate bike path
    CDijkstraPathRouter bikeRouter;
    std::vector<std::shared_ptr<CStreetMap::SNode>> nodes;
    std::size_t n = DImplementation->streetMap->NodeCount();
    for(std::size_t i = 0; i < n; i++){
        nodes.push_back(DImplementation->streetMap->NodeByIndex(i));
    }
    std::sort(nodes.begin(), nodes.end(), [](auto a, auto b){ return a->ID() < b->ID();});
    // add bike vertices
    std::unordered_map<TNodeID, CPathRouter::TVertexID> bikeMapping;
    for(std::size_t i = 0; i < nodes.size(); i++){
        CPathRouter::TVertexID vid = bikeRouter.AddVertex(nodes[i]->ID());
        bikeMapping[nodes[i]->ID()] = vid;
    }
    // add bike edges
    std::size_t wayCount = DImplementation->streetMap->WayCount();
    for(std::size_t i = 0; i < wayCount; i++){
        auto way = DImplementation->streetMap->WayByIndex(i);
        if(way->HasAttribute("bicycle") && way->GetAttribute("bicycle")=="no"){
            continue;
        }
        bool bidir = true;
        if(way->HasAttribute("oneway") && way->GetAttribute("oneway")=="yes"){
            bidir = false;
        }
        for(std::size_t j = 0; j + 1 < way->NodeCount(); j++){
            TNodeID s = way->GetNodeID(j);
            TNodeID t = way->GetNodeID(j+1);
            auto nodeS = DImplementation->streetMap->NodeByID(s);
            auto nodeT = DImplementation->streetMap->NodeByID(t);
            if(nodeS && nodeT){
                double distance = SGeographicUtils::HaversineDistanceInMiles(nodeS->Location(), nodeT->Location());
                double travelTime = distance / DImplementation->config->BikeSpeed();
                bikeRouter.AddEdge(bikeMapping[s], bikeMapping[t], travelTime, bidir);
            }
        }
    }
    std::vector<CPathRouter::TVertexID> bikeVertexPath;
    double bikeTimeD = bikeRouter.FindShortestPath(bikeMapping[src], bikeMapping[dest], bikeVertexPath);
    long double bikeTimeLD = (bikeTimeD == CPathRouter::NoPathExists) ? CPathRouter::NoPathExists : static_cast<long double>(bikeTimeD);
    std::vector<TTripStep> bikePath;
    if(bikeTimeLD != CPathRouter::NoPathExists){
        for(auto vid : bikeVertexPath){
            std::any tag = bikeRouter.GetVertexTag(vid);
            TNodeID nodeID = std::any_cast<TNodeID>(tag);
            bikePath.push_back(std::make_pair(CTransportationPlanner::ETransportationMode::Bike, nodeID));
        }
    }
    
    // compare bus and bike
    long double bestTimeLD = CPathRouter::NoPathExists;
    if(busTimeLD != CPathRouter::NoPathExists && busTimeLD < bikeTimeLD){
        path = busPath;
        bestTimeLD = busTimeLD;
    }else if(bikeTimeLD != CPathRouter::NoPathExists){
        path = bikePath;
        bestTimeLD = bikeTimeLD;
    }else{
        return CPathRouter::NoPathExists;
    }
    return (double)bestTimeLD;
}



bool CDijkstraTransportationPlanner::GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const {
    desc.clear();
    if (path.empty()) return false;
    std::ostringstream oss;
    
    // start node
    auto startNode = DImplementation->streetMap->NodeByID(path.front().second);
    if(startNode){
        desc.push_back("Start at " + SGeographicUtils::ConvertLLToDMS(startNode->Location()));
    }else{
        desc.push_back("Start at unknown location");
    }
    
    std::string lastStreet = "";
    
    // merge consecutive bus segments
    size_t i = 1;
    while(i < path.size()){
        auto mode = path[i].first;
        if(mode == CTransportationPlanner::ETransportationMode::Bus){
            CBusSystemIndexer indexer(DImplementation->config->BusSystem());
            auto srcStop = indexer.StopByNodeID(path[i-1].second);
            auto destStop = indexer.StopByNodeID(path[i].second);
            int groupSrc = srcStop ? srcStop->ID() : 0;
            int groupDest = destStop ? destStop->ID() : 0;
            std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> routes;
            indexer.RoutesByNodeIDs(path[i-1].second, path[i].second, routes);
            std::string routeName = "Unknown";
            if(!routes.empty()){
                routeName = (*routes.begin())->Name();
                for(auto &r : routes){
                    if(r->Name() < routeName){
                        routeName = r->Name();
                    }
                }
            }
            size_t j = i + 1;
            while(j < path.size() && path[j].first == CTransportationPlanner::ETransportationMode::Bus){
                CBusSystemIndexer idx(DImplementation->config->BusSystem());
                std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> nextRoutes;
                idx.RoutesByNodeIDs(path[j-1].second, path[j].second, nextRoutes);
                std::string nextRoute = "";
                if(!nextRoutes.empty()){
                    nextRoute = (*nextRoutes.begin())->Name();
                    for(auto &r : nextRoutes){
                        if(r->Name() < nextRoute){
                            nextRoute = r->Name();
                        }
                    }
                }
                if(nextRoute == routeName){
                    auto nextStop = idx.StopByNodeID(path[j].second);
                    if(nextStop){
                        groupDest = nextStop->ID();
                    }
                    j++;
                }else{
                    break;
                }
            }
            oss.str("");
            oss.clear();
            oss << "Take Bus " << routeName << " from stop " << groupSrc << " to stop " << groupDest;
            desc.push_back(oss.str());
            i = j;
        }else{ 
            // walk or bike
            TNodeID segStart = path[i-1].second;
            TNodeID segEnd = path[i].second;
            double initialDistance = SGeographicUtils::HaversineDistanceInMiles(
                DImplementation->streetMap->NodeByID(segStart)->Location(),
                DImplementation->streetMap->NodeByID(segEnd)->Location());
            double groupDistance = initialDistance;
            
            // attempt to find the street name of the current segment
            std::string streetName = "";
            std::size_t wayCount = DImplementation->streetMap->WayCount();
            for(size_t w = 0; w < wayCount; w++){
                auto way = DImplementation->streetMap->WayByIndex(w);
                for(size_t k = 0; k + 1 < way->NodeCount(); k++){
                    TNodeID a = way->GetNodeID(k);
                    TNodeID b = way->GetNodeID(k+1);
                    if((a == segStart && b == segEnd) || (a == segEnd && b == segStart)){
                        if(way->HasAttribute("name")){
                            streetName = way->GetAttribute("name");
                            break;
                        }
                    }
                }
                if(!streetName.empty()){
                    break;
                }
            }
            // if streetName is empty, try to find the street name of the next segment
            if(streetName.empty()){
                for(size_t offset = 1; i + offset < path.size(); offset++){
                    TNodeID laStart = path[i + offset - 1].second;
                    TNodeID laEnd = path[i + offset].second;
                    std::string laName = "";
                    for(size_t w = 0; w < wayCount; w++){
                        auto way = DImplementation->streetMap->WayByIndex(w);
                        for(size_t k = 0; k + 1 < way->NodeCount(); k++){
                            if(((way->GetNodeID(k) == laStart && way->GetNodeID(k+1) == laEnd) ||
                                 (way->GetNodeID(k) == laEnd && way->GetNodeID(k+1) == laStart)) &&
                                way->HasAttribute("name"))
                            {
                                laName = way->GetAttribute("name");
                                break;
                            }
                        }
                        if(!laName.empty()){
                            break;
                        }
                    }
                    if(!laName.empty()){
                        streetName = laName;
                        break;
                    }
                }
            }

            if(streetName.empty() && (i + 1 == path.size())){
                streetName = "End";
            }
            std::string groupStreet = streetName;
            
            double groupBearing = SGeographicUtils::CalculateBearing(
                DImplementation->streetMap->NodeByID(segStart)->Location(),
                DImplementation->streetMap->NodeByID(segEnd)->Location());
            
            // lambda function to calculate the angle difference between two bearings
            auto angleDiff = [](double a, double b) -> double{
                double diff = fabs(a - b);
                return diff > 180.0 ? 360.0 - diff : diff;
            };
            
            size_t j = i + 1;
            // if next segment is in the same direction and within 45 degrees, merge
            while(j < path.size() && path[j].first == mode){
                TNodeID prev = path[j-1].second;
                TNodeID curr = path[j].second;
                double segBearing = SGeographicUtils::CalculateBearing(
                    DImplementation->streetMap->NodeByID(prev)->Location(),
                    DImplementation->streetMap->NodeByID(curr)->Location());
                if(angleDiff(groupBearing, segBearing) > 45.0){
                    break;
                }
                std::string nextStreet = "";
                for(size_t w = 0; w < wayCount; w++){
                    auto way = DImplementation->streetMap->WayByIndex(w);
                    for(size_t k = 0; k + 1 < way->NodeCount(); k++){
                        TNodeID a = way->GetNodeID(k);
                        TNodeID b = way->GetNodeID(k+1);
                        if((a == prev && b == curr) || (a == curr && b == prev)){
                            if (way->HasAttribute("name")) {
                                nextStreet = way->GetAttribute("name");
                                break;
                            }
                        }
                    }
                    if (!nextStreet.empty())
                        break;
                }
                // if nextStreet is empty and groupStreet is not "End", set nextStreet to groupStreet
                if (nextStreet.empty() && groupStreet != "End")
                    nextStreet = groupStreet;
                if (nextStreet != groupStreet)
                    break;
                groupDistance += SGeographicUtils::HaversineDistanceInMiles(
                    DImplementation->streetMap->NodeByID(prev)->Location(),
                    DImplementation->streetMap->NodeByID(curr)->Location());
                j++;
            }
            
            std::string direction = SGeographicUtils::BearingToDirection(groupBearing);
            
            bool startingOnStreet = false;
            if(groupStreet != "End"){
                for (size_t w = 0; w < wayCount; w++){
                    auto way = DImplementation->streetMap->WayByIndex(w);
                    if (way->HasAttribute("name") && way->GetAttribute("name") == groupStreet){
                        for(size_t k = 0; k < way->NodeCount(); k++){
                            if(way->GetNodeID(k) == segStart){
                                startingOnStreet = true;
                                break;
                            }
                        }
                        if(startingOnStreet){
                            break;
                        }
                    }
                }
            }
            std::string prep;
            if(groupStreet == "End"){
                prep = "toward " + groupStreet;
            }else if(!lastStreet.empty() && lastStreet == groupStreet){
                prep = "along " + groupStreet;
            }else if (startingOnStreet){
                prep = "along " + groupStreet;
            }else{
                prep = "toward " + groupStreet;
            }
            
            oss.str("");
            oss.clear();
            oss << (mode == CTransportationPlanner::ETransportationMode::Bike ? "Bike" : "Walk")
                << " " << direction << " " << prep << " for " << std::fixed << std::setprecision(1) << groupDistance << " mi";
            desc.push_back(oss.str());
            lastStreet = groupStreet;
            i = j;
        }
    }
    
    auto endNode = DImplementation->streetMap->NodeByID(path.back().second);
    if(endNode)
         desc.push_back("End at " + SGeographicUtils::ConvertLLToDMS(endNode->Location()));
    else
         desc.push_back("End at unknown location");
    return true;
}

