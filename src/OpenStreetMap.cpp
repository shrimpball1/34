#include "OpenStreetMap.h"
#include "XMLReader.h"
#include <memory>
#include <vector>
#include <string>
#include <cstdlib>


struct COpenStreetMap::SImplementation{
    std::vector<std::shared_ptr<CStreetMap::SNode>> nodes;
    std::vector<std::shared_ptr<CStreetMap::SWay>> ways;
};


class NodeImpl : public CStreetMap::SNode{
public:
    CStreetMap::TNodeID id;
    CStreetMap::TLocation location;
    std::vector<std::pair<std::string, std::string>> attributes;
    
    NodeImpl(CStreetMap::TNodeID id, const CStreetMap::TLocation &loc) : id(id), location(loc){}
    
    // return the node id, location, number of attributes and key of the attribute
    virtual CStreetMap::TNodeID ID() const noexcept override{
        return id;
    }

    virtual CStreetMap::TLocation Location() const noexcept override{
        return location;
    }

    virtual std::size_t AttributeCount() const noexcept override{
        return attributes.size();
    }

    virtual std::string GetAttributeKey(std::size_t index) const noexcept override{
        return (index < attributes.size()) ? attributes[index].first : "";
    }

    virtual bool HasAttribute(const std::string &key) const noexcept override{
        for(const auto &attr : attributes){
            if(attr.first == key){
                return true;
            }
        }
        return false;
    }
    virtual std::string GetAttribute(const std::string &key) const noexcept override{
        for(const auto &attr : attributes){
            if(attr.first == key){
                return attr.second;
            }
        }
        return "";
    }
};


class WayImpl : public CStreetMap::SWay{
public:
    CStreetMap::TWayID id;
    std::vector<CStreetMap::TNodeID> nodeIDs;
    std::vector<std::pair<std::string, std::string>> attributes;
    
    WayImpl(CStreetMap::TWayID id) : id(id){}
    
    // return the way id, number of nodes, node ID, number of attributes...
    virtual CStreetMap::TWayID ID() const noexcept override{
        return id;
    }

    virtual std::size_t NodeCount() const noexcept override{
        return nodeIDs.size();
    }

    virtual CStreetMap::TNodeID GetNodeID(std::size_t index) const noexcept override{
        return (index < nodeIDs.size()) ? nodeIDs[index] : CStreetMap::InvalidNodeID;
    }

    virtual std::size_t AttributeCount() const noexcept override{
        return attributes.size();
    }

    virtual std::string GetAttributeKey(std::size_t index) const noexcept override {
        return (index < attributes.size()) ? attributes[index].first : "";
    }

    virtual bool HasAttribute(const std::string &key) const noexcept override{
        for(const auto &attr : attributes){
            if (attr.first == key){
                return true;
            }
        }
        return false;
    }

    virtual std::string GetAttribute(const std::string &key) const noexcept override{
        for (const auto &attr : attributes){
            if (attr.first == key){
                return attr.second;
            }
        }
        return "";
    }
};



COpenStreetMap::COpenStreetMap(std::shared_ptr<CXMLReader> src) : DImplementation(std::make_unique<SImplementation>()){
    SXMLEntity entity;
    // iterate through each XML entity from the source
    while(src->ReadEntity(entity, false)){
        if(entity.DType == SXMLEntity::EType::StartElement && entity.DNameData == "node"){
            std::string idStr, latStr, lonStr;
            // extract id, lat and lon attributes
            for(const auto &attr : entity.DAttributes){
                if(attr.first == "id"){
                    idStr = attr.second;
                }else if(attr.first == "lat"){
                    latStr = attr.second;
                }else if(attr.first == "lon"){
                    lonStr = attr.second;
                }
            }
            if(!idStr.empty() && !latStr.empty() && !lonStr.empty()){
                CStreetMap::TNodeID id = std::stoull(idStr);
                double lat = std::stod(latStr);
                double lon = std::stod(lonStr);
                auto node = std::make_shared<NodeImpl>(id, std::make_pair(lat, lon));
                SXMLEntity subEntity;
                // process child entities
                while(src->ReadEntity(subEntity, false)){
                    if(subEntity.DType == SXMLEntity::EType::StartElement && subEntity.DNameData == "tag"){
                        std::string key, value;
                        // exact key and val from tag
                        for(const auto &attr : subEntity.DAttributes){
                            if (attr.first == "k"){
                                key = attr.second;
                            }else if (attr.first == "v"){
                                value = attr.second;
                            }
                        }
                        if(!key.empty()){
                            node->attributes.push_back({key, value});
                        }
                    }
                    else if (subEntity.DType == SXMLEntity::EType::EndElement && subEntity.DNameData == "node") {
                        break;
                    }
                }
                // add the parsed node
                DImplementation->nodes.push_back(node);
            }
        }else if(entity.DType == SXMLEntity::EType::StartElement && entity.DNameData == "way"){  // process way element
            std::string idStr;
            // extract id
            for(const auto &attr : entity.DAttributes){
                if(attr.first == "id"){
                    idStr = attr.second;
                    break;
                }
            }
            if(!idStr.empty()){
                TWayID id = std::stoull(idStr);
                auto way = std::make_shared<WayImpl>(id);
                SXMLEntity subEntity;
                // process child elements within way
                while(src->ReadEntity(subEntity, false)){
                    if(subEntity.DType == SXMLEntity::EType::StartElement && subEntity.DNameData == "nd"){
                        for(const auto &attr : subEntity.DAttributes){
                            if(attr.first == "ref"){
                                CStreetMap::TNodeID refId = std::stoull(attr.second);
                                way->nodeIDs.push_back(refId);
                            }
                        }
                    }else if(subEntity.DType == SXMLEntity::EType::StartElement && subEntity.DNameData == "tag"){
                        std::string key, value;
                        // key and val for way
                        for(const auto &attr : subEntity.DAttributes){
                            if (attr.first == "k")
                                key = attr.second;
                            else if (attr.first == "v")
                                value = attr.second;
                        }
                        if(!key.empty())
                            way->attributes.push_back({key, value});
                    }else if(subEntity.DType == SXMLEntity::EType::EndElement && subEntity.DNameData == "way"){
                        break;
                    }
                }
                // add the parsed way
                DImplementation->ways.push_back(way);
            }
        }
    }
}

COpenStreetMap::~COpenStreetMap() = default;

std::size_t COpenStreetMap::NodeCount() const noexcept{
    return DImplementation->nodes.size();
}

std::size_t COpenStreetMap::WayCount() const noexcept{
    return DImplementation->ways.size();
}

std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByIndex(std::size_t index) const noexcept{
    return (index < DImplementation->nodes.size()) ? DImplementation->nodes[index] : nullptr;
}

std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByID(CStreetMap::TNodeID id) const noexcept{
    for(const auto &node : DImplementation->nodes){
        if(node->ID() == id){
            return node;
        }
    }
    return nullptr;
}

std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByIndex(std::size_t index) const noexcept{
    return (index < DImplementation->ways.size()) ? DImplementation->ways[index] : nullptr;
}

std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByID(CStreetMap::TWayID id) const noexcept{
    for(const auto &way : DImplementation->ways){
        if(way->ID() == id){
            return way;
        }
    }
    return nullptr;
}
