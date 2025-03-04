#include "XMLReader.h"
#include "expat.h"
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <queue>

// reference: https://www.codeproject.com/Articles/1847/Cplusplus-Wrappers-for-the-Expat-XML-Parser
//            https://stackoverflow.com/questions/3778893/using-expat-startelement-handler-c
//            https://github.com/libexpat/libexpat/issues/137

struct CXMLReader::SImplementation{
    std::shared_ptr<CDataSource> DSource;

    std::queue<SXMLEntity> DEntities;
    bool DFinishedParsing = false;

    XML_Parser DParser;

    SImplementation(std::shared_ptr<CDataSource> src) : DSource(src){
        DParser = XML_ParserCreate(nullptr);

        XML_SetUserData(DParser, this);
        XML_SetElementHandler(DParser, StartElementCallback, EndElementCallback);
        XML_SetCharacterDataHandler(DParser, CharDataCallback);

        ParseAll();
    }

    ~SImplementation(){
        XML_ParserFree(DParser);
    }

    void ParseAll(){
        std::string buffer;
        char c;
        while(!DSource->End()){
            if(DSource->Get(c)){
                buffer.push_back(c);
            }
        }

        XML_Parse(DParser, buffer.c_str(), static_cast<int>(buffer.size()), 1);
        DFinishedParsing = true;
    }


    static void StartElementCallback(void *userData, const XML_Char *name, const XML_Char **atts){
        auto impl = reinterpret_cast<SImplementation*>(userData);
        SXMLEntity Entity;
        Entity.DType = SXMLEntity::EType::StartElement;
        Entity.DNameData = std::string(name);

        // atts is a list of (key, value, key, value, ..., nullptr)
        for(int i=0; atts[i]; i+=2){
            SXMLEntity::TAttribute attr;
            attr.first  = std::string(atts[i]);
            attr.second = std::string(atts[i+1]);
            Entity.DAttributes.push_back(attr);
        }
        impl->DEntities.push(Entity);
    }

    static void EndElementCallback(void *userData, const XML_Char *name){
        auto impl = reinterpret_cast<SImplementation*>(userData);
        SXMLEntity Entity;
        Entity.DType = SXMLEntity::EType::EndElement;
        Entity.DNameData = std::string(name);
        impl->DEntities.push(Entity);
    }

    static void CharDataCallback(void *userData, const XML_Char *s, int len){
        auto impl = reinterpret_cast<SImplementation*>(userData);
        std::string text(s, len);

        bool allWhitespace = true;
        for(char c : text){
            if(!std::isspace(static_cast<unsigned char>(c))){
                allWhitespace = false;
                break;
            }
        }
        if(!text.empty() && !allWhitespace){
            SXMLEntity Entity;
            Entity.DType = SXMLEntity::EType::CharData;
            Entity.DNameData = text;
            impl->DEntities.push(Entity);
        }
    }

    bool End() const{
        return DFinishedParsing && DEntities.empty();
    }

    bool ReadEntity(SXMLEntity &entity, bool skipcdata){
        while(!DEntities.empty()) {
            SXMLEntity front = DEntities.front();
            DEntities.pop();
            if(!(skipcdata && front.DType == SXMLEntity::EType::CharData)){
                entity = front;
                return true;
            }
        }
        return false;
    }
};

CXMLReader::CXMLReader(std::shared_ptr<CDataSource> src)
    : DImplementation(std::make_unique<SImplementation>(src)){}

CXMLReader::~CXMLReader(){};

bool CXMLReader::End() const{
    return DImplementation->End();
}

bool CXMLReader::ReadEntity(SXMLEntity &entity, bool skipcdata){
    return DImplementation->ReadEntity(entity, skipcdata);
}
