#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include "CSVBusSystem.h"
#include "DSVReader.h"
#include "StringDataSource.h"

TEST(CSVBusSystemTest_File, TotalCounts){
    std::ifstream stopsFile("../data/stops.csv");
    ASSERT_TRUE(stopsFile.is_open()) << "cannot open stops.csv";
    std::stringstream stopsBuffer;
    stopsBuffer << stopsFile.rdbuf();
    std::string stopsContent = stopsBuffer.str();

    std::ifstream routesFile("../data/routes.csv");
    ASSERT_TRUE(routesFile.is_open()) << "cannot open routes.csv";
    std::stringstream routesBuffer;
    routesBuffer << routesFile.rdbuf();
    std::string routesContent = routesBuffer.str();

    auto stopsDataSource = std::make_shared<CStringDataSource>(stopsContent);
    auto stopsReader = std::make_shared<CDSVReader>(stopsDataSource, ',');
    auto routesDataSource = std::make_shared<CStringDataSource>(routesContent);
    auto routesReader = std::make_shared<CDSVReader>(routesDataSource, ',');

    CCSVBusSystem busSystem(stopsReader, routesReader);

    EXPECT_GT(busSystem.StopCount(), 0) << "Number of stops should be larger than 0";
    EXPECT_GT(busSystem.RouteCount(), 0) << "Number of routes should be larger than 0";

    auto stop22043 = busSystem.StopByID(22043);
    ASSERT_NE(stop22043, nullptr) << "stop with id 22043 should exist";
    EXPECT_EQ(stop22043->NodeID(), 2849810514) << "nodeid of stop 22043 should be 2849810514";
    
    auto routeA = busSystem.RouteByName("A");
    ASSERT_NE(routeA, nullptr) << "route A should exist";
    EXPECT_EQ(routeA->StopCount(), 22) << "route A should include 22 stops";
}
