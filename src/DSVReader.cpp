#include "DSVReader.h"
#include <sstream>

struct CDSVReader::SImplementation{
    std::shared_ptr<CDataSource> DSource;
    char DDelimiter;

    SImplementation(std::shared_ptr<CDataSource> src, char delimiter)  : DSource(src), DDelimiter(delimiter){}

    bool ReadRow(std::vector<std::string> &row){
        row.clear();
        std::string CurrentField;
        bool InQuotes = false;
        char c;

        while(DSource->Get(c)){
            if(c == DDelimiter && !InQuotes){
                row.push_back(CurrentField);
                CurrentField.clear();
            }else if(c == '"'){
                char NextC;
                if(DSource->Peek(NextC) && NextC == '"'){
                    CurrentField += '"';
                    DSource->Get(NextC);
                }else{
                    InQuotes = !InQuotes;
                }
            }else if(c == '\n' && !InQuotes){
                row.push_back(CurrentField);
                return true;
            }else{
                CurrentField += c;
            }
        }
        if(!CurrentField.empty()){
            row.push_back(CurrentField);
        }
        return !row.empty();
    }
};

CDSVReader::CDSVReader(std::shared_ptr<CDataSource> src, char delimiter)
    : DImplementation(new SImplementation(src, delimiter)){}

CDSVReader::~CDSVReader(){};

bool CDSVReader::End()const{
    return DImplementation->DSource->End();
}

bool CDSVReader::ReadRow(std::vector<std::string> &row){
    return DImplementation->ReadRow(row);
}
