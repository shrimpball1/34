#include <gtest/gtest.h>
#include <fstream>
#include "OpenStreetMap.h"
#include "XMLReader.h"
#include "StringDataSource.h"

TEST(COpenStreetMapTest, BasicFunctionality){
    std::ifstream osmFile("../data/davis.osm");
    ASSERT_TRUE(osmFile.is_open()) << "cannot open davis.osm";
    std::string osmData((std::istreambuf_iterator<char>(osmFile)), std::istreambuf_iterator<char>());
    auto src = std::make_shared<CStringDataSource>(osmData);
    auto reader = std::make_shared<CXMLReader>(src);

    COpenStreetMap map(reader);

    EXPECT_EQ(map.NodeCount(), 10259) << "Number of node should be 10259";
    EXPECT_EQ(map.WayCount(), 1644) << "Number of way should be 1644";

    auto node = map.NodeByIndex(0);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->ID(), 62208369) << "the first id is 62208369";

    node = map.NodeByID(62209104);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->ID(), 62209104) << "cannot get node by id";

    auto way = map.WayByIndex(0);
    ASSERT_NE(way, nullptr);
    EXPECT_EQ(way->ID(), 8699536) << "the first id is 8699536";
    EXPECT_EQ(way->GetAttribute("highway"), "motorway_link") << "the first attribute is \"motorway_link\"";

    way = map.WayByID(545433980);
    ASSERT_NE(way, nullptr);
    EXPECT_EQ(way->ID(), 545433980) << "cannot get way by id";
}
