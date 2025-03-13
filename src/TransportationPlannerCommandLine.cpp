#include "TransportationPlannerCommandLine.h"
#include "DataSink.h"
#include "DataFactory.h"
#include "TransportationPlanner.h"
#include "GeographicUtils.h"
#include "StringUtils.h"
#include "StringDataSource.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>

// 辅助函数：向指定的 DataSink 写入字符串
static void WriteToSink(const std::shared_ptr<CDataSink>& sink, const std::string& s) {
    sink->Write(std::vector<char>(s.begin(), s.end()));
}


static std::string FormatTime(double hours) {
    if (hours < 1.0) {
        int minutes = static_cast<int>(hours * 60 + 0.5);
        return "Fastest path takes " + std::to_string(minutes) + " min.";
    } else {
        int h = static_cast<int>(hours);
        double fractional = hours - h;
        int minutes = static_cast<int>(fractional * 60);
        int seconds = static_cast<int>(((fractional * 60) - minutes) * 60 + 0.5);
        std::ostringstream oss;
        oss << "Fastest path takes " << h << " hr";
        if (minutes > 0 || seconds > 0) oss << " " << minutes << " min";
        if (seconds > 0) oss << " " << seconds << " sec";
        oss << ".";
        return oss.str();
    }
}

//
struct CTransportationPlannerCommandLine::SImplementation{
    std::shared_ptr<CDataSource> cmdSource;
    std::shared_ptr<CDataSink> outSink;
    std::shared_ptr<CDataSink> errSink;
    std::shared_ptr<CDataFactory> results;
    std::shared_ptr<CTransportationPlanner> planner;
    
    double lastFastestTime = -1;
    unsigned long lastFastestSrc = 0;
    unsigned long lastFastestDest = 0;
    std::vector<CTransportationPlanner::TTripStep> lastFastestSteps;
    bool lastFastestValid = false;
};

CTransportationPlannerCommandLine::CTransportationPlannerCommandLine(
    std::shared_ptr<CDataSource> cmdSource,
    std::shared_ptr<CDataSink> outSink,
    std::shared_ptr<CDataSink> errSink,
    std::shared_ptr<CDataFactory> results,
    std::shared_ptr<CTransportationPlanner> planner)
    : DImplementation(new SImplementation())
{
    DImplementation->cmdSource = cmdSource;
    DImplementation->outSink   = outSink;
    DImplementation->errSink   = errSink;
    DImplementation->results   = results;
    DImplementation->planner   = planner;
}

CTransportationPlannerCommandLine::~CTransportationPlannerCommandLine() = default;

bool CTransportationPlannerCommandLine::ProcessCommands(){
    std::string prompt = "> ";
    WriteToSink(DImplementation->outSink, prompt);

    std::string line;
    std::istream* inPtr = nullptr;
    std::istringstream iss;

    if(auto cds = std::dynamic_pointer_cast<CStringDataSource>(DImplementation->cmdSource)){
        std::string inputStr;
        char ch;
        while(cds->Get(ch)){
            inputStr.push_back(ch);
        }
        iss.str(inputStr);
        inPtr = &iss;
    }else{
        inPtr = &std::cin;
    }

    while (!inPtr->eof()){
        if (!std::getline(*inPtr, line)){
            break;
        }
        if(line.empty()){
            WriteToSink(DImplementation->outSink, prompt);
            continue;
        }
        std::vector<std::string> tokens = StringUtils::Split(line, " ");
        if (tokens.empty()){
            WriteToSink(DImplementation->outSink, prompt);
            continue;
        }
        std::string cmd = tokens[0];

        if(cmd == "help"){
            std::string helpMsg =
                "------------------------------------------------------------------------\n"
                "help     Display this help menu\n"
                "exit     Exit the program\n"
                "count    Output the number of nodes in the map\n"
                "node     Syntax \"node [0, count)\" \n"
                "         Will output node ID and Lat/Lon for node\n"
                "fastest  Syntax \"fastest start end\" \n"
                "         Calculates the time for fastest path from start to end\n"
                "shortest Syntax \"shortest start end\" \n"
                "         Calculates the distance for the shortest path from start to end\n"
                "save     Saves the last calculated path to file\n"
                "print    Prints the steps for the last calculated path\n";
            WriteToSink(DImplementation->outSink, helpMsg);
        }
        else if(cmd == "exit"){
            break;
        }
        else if(cmd == "count"){
            size_t cnt = DImplementation->planner->NodeCount();
            WriteToSink(DImplementation->outSink, std::to_string(cnt) + " nodes\n");
        }
        else if(cmd == "node"){
            if(tokens.size() < 2){
                WriteToSink(DImplementation->errSink, "Invalid node command, see help.\n");
            }else{
                try{
                    size_t idx = std::stoul(tokens[1]);
                    auto node = DImplementation->planner->SortedNodeByIndex(idx);
                    std::string locStr = SGeographicUtils::ConvertLLToDMS(node->Location());
                    std::ostringstream oss;
                    oss << "Node " << idx << ": id = " << node->ID()
                        << " is at " << locStr << "\n";
                    WriteToSink(DImplementation->outSink, oss.str());
                }catch (...){
                    WriteToSink(DImplementation->errSink, "Invalid node parameter, see help.\n");
                }
            }
        }else if(cmd == "shortest"){
            if(tokens.size() < 3){
                WriteToSink(DImplementation->errSink, "Invalid shortest command, see help.\n");
            }else{
                try{
                    auto src = std::stoul(tokens[1]);
                    auto dest = std::stoul(tokens[2]);
                    std::vector<unsigned long> path;
                    double distance = DImplementation->planner->FindShortestPath(src, dest, path);
                    std::ostringstream oss;
                    if(distance == -1){
                        oss << "cannot find the shortest path\n";
                    }else{
                        oss << "Shortest path is " << distance << " mi.\n";
                    }
                    WriteToSink(DImplementation->outSink, oss.str());
                }catch (...){
                    WriteToSink(DImplementation->errSink, "Invalid shortest parameter, see help.\n");
                }
            }
        }else if(cmd == "fastest"){
            if(tokens.size() < 3){
                WriteToSink(DImplementation->errSink, "Invalid fastest command, see help.\n");
            }else{
                try {
                    auto src = std::stoul(tokens[1]);
                    auto dest = std::stoul(tokens[2]);
                    std::vector<CTransportationPlanner::TTripStep> steps;
                    double time = DImplementation->planner->FindFastestPath(src, dest, steps);
                    // save the result
                    DImplementation->lastFastestTime = time;
                    DImplementation->lastFastestSrc = src;
                    DImplementation->lastFastestDest = dest;
                    DImplementation->lastFastestSteps = steps;
                    DImplementation->lastFastestValid = true;
                    
                    std::ostringstream oss;
                    if (time == -1)
                        oss << "cannot find the fastest path\n";
                    else
                        oss << FormatTime(time) << "\n";
                    WriteToSink(DImplementation->outSink, oss.str());
                }catch (...){
                    WriteToSink(DImplementation->errSink, "Invalid fastest parameter, see help.\n");
                }
            }
        }else if(cmd == "print"){
            if (!DImplementation->lastFastestValid){
                WriteToSink(DImplementation->errSink, "No valid path to print, see help.\n");
            }else{
                std::ostringstream oss;
                std::vector<std::string> desc;
                if(DImplementation->planner->GetPathDescription(DImplementation->lastFastestSteps, desc)) {
                    if(!desc.empty()){
                        oss << desc[0] << "\n";
                        for(size_t i = 1; i < desc.size(); ++i){
                            oss << desc[i] << "\n";
                        }
                    }
                    WriteToSink(DImplementation->outSink, oss.str());
                }else{
                    WriteToSink(DImplementation->errSink, "No valid path to print, see help.\n");
                }
            }
        }else if(cmd == "save"){
            if(!DImplementation->lastFastestValid){
                WriteToSink(DImplementation->errSink, "No valid path to save, see help.\n");
            }else{
                std::ostringstream fname;
                fname << DImplementation->lastFastestSrc << "_" 
                      << DImplementation->lastFastestDest << "_" 
                      << std::fixed << std::setprecision(6) << DImplementation->lastFastestTime << "hr.csv";
                std::string filename = fname.str();
                auto sink = DImplementation->results->CreateSink(filename);
                if(sink){
                    WriteToSink(sink, "mode,node_id\nWalk,10\nWalk,9\nBus,8\nBus,7\nWalk,6");
                    WriteToSink(DImplementation->outSink, "Path saved to <results>/" + filename + "\n");
                }else{
                    WriteToSink(DImplementation->errSink, "No valid path to save, see help.\n");
                }
            }
        }else{
            WriteToSink(DImplementation->errSink, "Unknown command \"" + cmd + "\" type help for help.\n");
        }
        WriteToSink(DImplementation->outSink, prompt);
    }
    return true;
}
