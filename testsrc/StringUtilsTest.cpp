#include <gtest/gtest.h>
#include "StringUtils.h"

TEST(StringUtilsTest, SliceTest){
    EXPECT_EQ(StringUtils::Slice("hello world", 0, 5), "hello");
    EXPECT_EQ(StringUtils::Slice("hello world", -5, 0), "world");
    EXPECT_EQ(StringUtils::Slice("hello world", -5, -1), "worl");
}

TEST(StringUtilsTest, Capitalize){
    EXPECT_EQ(StringUtils::Capitalize("hello"), "Hello");
    EXPECT_EQ(StringUtils::Capitalize("HELLO"), "Hello");
    EXPECT_EQ(StringUtils::Capitalize("h"), "H");
}

TEST(StringUtilsTest, Upper){
    EXPECT_EQ(StringUtils::Upper("hello"), "HELLO");
    EXPECT_EQ(StringUtils::Upper("HeLLo"), "HELLO");
    EXPECT_EQ(StringUtils::Upper("h"), "H");
}

TEST(StringUtilsTest, Lower){
    EXPECT_EQ(StringUtils::Lower("HELLO"), "hello");
    EXPECT_EQ(StringUtils::Lower("HeLLo"), "hello");
    EXPECT_EQ(StringUtils::Lower("H"), "h");
}

TEST(StringUtilsTest, LStrip){
    EXPECT_EQ(StringUtils::LStrip("   hello"), "hello");
    EXPECT_EQ(StringUtils::LStrip("\t\n hello"), "hello");
    EXPECT_EQ(StringUtils::LStrip("hello"), "hello");
}

TEST(StringUtilsTest, RStrip){
    EXPECT_EQ(StringUtils::RStrip("hello   "), "hello");
    EXPECT_EQ(StringUtils::RStrip("hello\t\n "), "hello");
    EXPECT_EQ(StringUtils::RStrip("hello"), "hello");
}

TEST(StringUtilsTest, Strip){
    EXPECT_EQ(StringUtils::Strip("   hello   "), "hello");
    EXPECT_EQ(StringUtils::Strip("\t\n hello\t\n "), "hello");
    EXPECT_EQ(StringUtils::Strip("hello"), "hello");
}

TEST(StringUtilsTest, Center){
    EXPECT_EQ(StringUtils::Center("hello", 10), "  hello   ");
    EXPECT_EQ(StringUtils::Center("hello", 11, '*'), "***hello***");
    EXPECT_EQ(StringUtils::Center("hello", 5), "hello");
}

TEST(StringUtilsTest, LJust){
    EXPECT_EQ(StringUtils::LJust("hello", 10), "hello     ");
    EXPECT_EQ(StringUtils::LJust("hello", 11, '*'), "hello******");
    EXPECT_EQ(StringUtils::LJust("hello", 5), "hello");
}

TEST(StringUtilsTest, RJust){
    EXPECT_EQ(StringUtils::RJust("hello", 10), "     hello");
    EXPECT_EQ(StringUtils::RJust("hello", 11, '*'), "******hello");
    EXPECT_EQ(StringUtils::RJust("hello", 5), "hello");
}

TEST(StringUtilsTest, Replace){
    EXPECT_EQ(StringUtils::Replace("hello world", "world", "123"), "hello 123");
    EXPECT_EQ(StringUtils::Replace("hello 123 123", "123", "world"), "hello world world");
    EXPECT_EQ(StringUtils::Replace("hello", "world", "123"), "hello");
}

TEST(StringUtilsTest, Split){
    EXPECT_EQ(StringUtils::Split("hello world", " "), (std::vector<std::string>{"hello", "world"}));
    EXPECT_EQ(StringUtils::Split("hello,,world", ","), (std::vector<std::string>{"hello", "", "world"}));
    EXPECT_EQ(StringUtils::Split("hello world", ""), (std::vector<std::string>{"hello", "world"}));
}

TEST(StringUtilsTest, Join){
    EXPECT_EQ(StringUtils::Join(" ", {"hello", "world"}), "hello world");
    EXPECT_EQ(StringUtils::Join(",", {"hello", "world", ""}), "hello,world,");
    EXPECT_EQ(StringUtils::Join("-", {}), "");
}

TEST(StringUtilsTest, ExpandTabs){
    EXPECT_EQ(StringUtils::ExpandTabs("hello\tworld", 4), "hello   world");
    EXPECT_EQ(StringUtils::ExpandTabs("\thello", 4), "    hello");
}

TEST(StringUtilsTest, EditDistance){
    EXPECT_EQ(StringUtils::EditDistance("kitten", "sitting"), 3);
    EXPECT_EQ(StringUtils::EditDistance("rad", "apple"), 5);
    EXPECT_EQ(StringUtils::EditDistance("hello", "HELLO", true), 0);
}
