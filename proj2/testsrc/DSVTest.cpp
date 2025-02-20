#include <gtest/gtest.h>
#include "DSVReader.h"
#include "DSVWriter.h"
#include "StringDataSource.h"
#include "StringDataSink.h"

TEST(DSVWriter, BasicTest) {
    auto Sink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(Sink, ',');
    
    std::vector<std::string> Row = {"a", "b", "c"};
    EXPECT_TRUE(Writer.WriteRow(Row));
    EXPECT_EQ(Sink->String(), "a,b,c\n");
}

TEST(DSVWriter, QuotingTest) {
    auto Sink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(Sink, ',');
    
    std::vector<std::string> Row = {"a,b", "c\"d", "e\nf"};
    EXPECT_TRUE(Writer.WriteRow(Row));
    EXPECT_EQ(Sink->String(), "\"a,b\",\"c\"\"d\",\"e\nf\"\n");
}

TEST(DSVWriter, EmptyRowTest) {
    auto Sink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(Sink, ',');
    
    std::vector<std::string> Row;
    EXPECT_TRUE(Writer.WriteRow(Row));
    EXPECT_EQ(Sink->String(), "\n");
}

TEST(DSVReader, BasicTest) {
    auto Source = std::make_shared<CStringDataSource>("a,b,c\n");
    CDSVReader Reader(Source, ',');
    
    std::vector<std::string> Row;
    EXPECT_TRUE(Reader.ReadRow(Row));
    ASSERT_EQ(Row.size(), 3);
    EXPECT_EQ(Row[0], "a");
    EXPECT_EQ(Row[1], "b");
    EXPECT_EQ(Row[2], "c");
    EXPECT_TRUE(Reader.End());
}

TEST(DSVReader, QuotedTest) {
    auto Source = std::make_shared<CStringDataSource>("\"a,b\",\"c\"\"d\",\"e\nf\"\n");
    CDSVReader Reader(Source, ',');
    
    std::vector<std::string> Row;
    EXPECT_TRUE(Reader.ReadRow(Row));
    ASSERT_EQ(Row.size(), 3);
    EXPECT_EQ(Row[0], "a,b");
    EXPECT_EQ(Row[1], "c\"d");
    EXPECT_EQ(Row[2], "e\nf");
    EXPECT_TRUE(Reader.End());
}

TEST(DSVReader, EmptyFieldTest) {
    auto Source = std::make_shared<CStringDataSource>("a,,c\n");
    CDSVReader Reader(Source, ',');
    
    std::vector<std::string> Row;
    EXPECT_TRUE(Reader.ReadRow(Row));
    ASSERT_EQ(Row.size(), 3);
    EXPECT_EQ(Row[0], "a");
    EXPECT_EQ(Row[1], "");
    EXPECT_EQ(Row[2], "c");
    EXPECT_TRUE(Reader.End());
}

TEST(DSVReader, EmptyLineTest) {
    auto Source = std::make_shared<CStringDataSource>("\n");
    CDSVReader Reader(Source, ',');
    
    std::vector<std::string> Row;
    EXPECT_TRUE(Reader.ReadRow(Row));
    EXPECT_TRUE(Row.empty());
    EXPECT_TRUE(Reader.End());
}