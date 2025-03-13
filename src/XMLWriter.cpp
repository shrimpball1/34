#include "XMLWriter.h"
#include <iostream>

struct CXMLWriter::SImplementation {
    std::shared_ptr<CDataSink> DSink;
    std::vector<std::string> DOpenElements;

    SImplementation(std::shared_ptr<CDataSink> sink) : DSink(sink) {}

    // reference: https://www.geeksforgeeks.org/how-to-escape-characters-in-xml/
    std::string Escape(const std::string &str) {
        std::string Escaped;
        for (char c : str) {
            switch (c) {
                case '&':  Escaped += "&amp;";  break;
                case '<':  Escaped += "&lt;";   break;
                case '>':  Escaped += "&gt;";   break;
                case '"':  Escaped += "&quot;"; break;
                case '\'': Escaped += "&apos;"; break;
                default:   Escaped += c;
            }
        }
        return Escaped;
    }

    bool WriteEntity(const SXMLEntity &entity){
        std::string Output;

        switch(entity.DType){
            case SXMLEntity::EType::StartElement:
                Output += "<" + entity.DNameData;
                for(auto &attr : entity.DAttributes){
                    Output += " " + attr.first + "=\"" + Escape(attr.second) + "\"";
                }
                Output += ">";
                DOpenElements.push_back(entity.DNameData);
                break;

            case SXMLEntity::EType::EndElement:
                if(!DOpenElements.empty() && DOpenElements.back() == entity.DNameData){
                    DOpenElements.pop_back();
                }
                Output += "</" + entity.DNameData + ">";
                break;

            case SXMLEntity::EType::CharData:
                Output += Escape(entity.DNameData);
                break;

            case SXMLEntity::EType::CompleteElement:
                Output += "<" + entity.DNameData;
                for(auto &attr : entity.DAttributes){
                    Output += " " + attr.first + "=\"" + Escape(attr.second) + "\"";
                }
                Output += "/>";
                break;
        }

        return DSink->Write(std::vector<char>(Output.begin(), Output.end()));
    }

    bool Flush(){
        // autofill the remaining label
        std::string Output;
        while(!DOpenElements.empty()){
            Output += "</" + DOpenElements.back() + ">";
            DOpenElements.pop_back();
        }
        return DSink->Write(std::vector<char>(Output.begin(), Output.end()));
    }
};

CXMLWriter::CXMLWriter(std::shared_ptr<CDataSink> sink)
    : DImplementation(std::make_unique<SImplementation>(sink)){}

CXMLWriter::~CXMLWriter(){};

bool CXMLWriter::WriteEntity(const SXMLEntity &entity){
    return DImplementation->WriteEntity(entity);
}

bool CXMLWriter::Flush(){
    return DImplementation->Flush();
}