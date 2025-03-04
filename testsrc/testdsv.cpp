#include <gtest/gtest.h>
#include "DSVReader.h"
#include "DSVWriter.h"
#include "StringDataSource.h"
#include "StringDataSink.h"
#include <vector>
#include <string>

TEST(DSVTest, WriteAndRead){
    std::shared_ptr<CStringDataSink> sink = std::make_shared<CStringDataSink>();
    {
        CDSVWriter writer(sink, ',', false); 
        EXPECT_TRUE(writer.WriteRow({"Hello", "World"}));
        EXPECT_TRUE(writer.WriteRow({"This", "is", "a,test"}));
    }

    std::string written = sink->String();
    std::shared_ptr<CStringDataSource> source = std::make_shared<CStringDataSource>(written);
    CDSVReader reader(source, ',');

    std::vector<std::string> row;
    EXPECT_FALSE(reader.End());
    EXPECT_TRUE(reader.ReadRow(row));
    ASSERT_EQ(row.size(), 2u);
    EXPECT_EQ(row[0], "Hello");
    EXPECT_EQ(row[1], "World");

    EXPECT_FALSE(reader.End());
    EXPECT_TRUE(reader.ReadRow(row));
    ASSERT_EQ(row.size(), 3u);
    EXPECT_EQ(row[0], "This");
    EXPECT_EQ(row[1], "is");
    EXPECT_EQ(row[2], "a,test");

    EXPECT_TRUE(reader.End() || !reader.ReadRow(row));
}

TEST(DSVTest, QuoteAll){
    std::shared_ptr<CStringDataSink> sink = std::make_shared<CStringDataSink>();
    {
        CDSVWriter writer(sink, ',', true);
        writer.WriteRow({"Hello", "World", "123"});
    }
    std::string written = sink->String();

    std::shared_ptr<CStringDataSource> source = std::make_shared<CStringDataSource>(written);
    CDSVReader reader(source, ',');
    std::vector<std::string> row;
    EXPECT_TRUE(reader.ReadRow(row));
    ASSERT_EQ(row.size(), 3u);
    EXPECT_EQ(row[0], "Hello");
    EXPECT_EQ(row[1], "World");
    EXPECT_EQ(row[2], "123");
}
