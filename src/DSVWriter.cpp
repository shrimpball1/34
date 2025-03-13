#include "DSVWriter.h"
#include <sstream>

struct CDSVWriter::SImplementation{
    std::shared_ptr<CDataSink> DSink;
    char DDelimiter;
    bool DQuoteAll;
    bool FirstRow;  // add a flag to check if it is the first row

    SImplementation(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall) : DSink(sink), DDelimiter(delimiter), DQuoteAll(quoteall), FirstRow(true){}

    bool WriteRow(const std::vector<std::string> &row){
        std::string Line;
        for(size_t i = 0; i < row.size(); ++i){
            std::string Field = row[i];
            bool NeedQuotes = DQuoteAll || Field.find(DDelimiter) != std::string::npos ||
                              Field.find('"') != std::string::npos || Field.find('\n') != std::string::npos;
            if(NeedQuotes){
                Line += '"';
                for(char c : Field){
                    if(c == '"'){
                        Line += "\"\"";
                    } else {
                        Line += c;
                    }
                }
                Line += '"';
            }else{
                Line += Field;
            }
            if(i < row.size() - 1){
                Line += DDelimiter;
            }
        }
        if(FirstRow){
            FirstRow = false;
        }else{
            Line = "\n" + Line;
        }
        return DSink->Write(std::vector<char>(Line.begin(), Line.end()));
    }
};

CDSVWriter::CDSVWriter(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall)
    : DImplementation(std::make_unique<SImplementation>(sink, delimiter, quoteall)){}

CDSVWriter::~CDSVWriter(){};

bool CDSVWriter::WriteRow(const std::vector<std::string> &row){
    return DImplementation->WriteRow(row);
}
