#include <gtest/gtest.h>
#include "XMLReader.h"
#include "XMLWriter.h"
#include "StringDataSource.h"
#include "StringDataSink.h"

TEST(XMLWriter, StartElementTest) {
    auto Sink = std::make_shared<CStringDataSink>();
    CXMLWriter Writer(Sink);
    
    SXMLEntity Element;
    Element.DType = SXMLEntity::EType::StartElement;
    Element.DNameData = "test";
    EXPECT_TRUE(Writer.WriteEntity(Element));
    EXPECT_TRUE(Writer.Flush());
    EXPECT_EQ(Sink->String(), "<test></test>");
}

TEST(XMLWriter, CompleteElementTest) {
    auto Sink = std::make_shared<CStringDataSink>();
    CXMLWriter Writer(Sink);
    
    SXMLEntity Element;
    Element.DType = SXMLEntity::EType::CompleteElement;
    Element.DNameData = "test";
    EXPECT_TRUE(Writer.WriteEntity(Element));
    EXPECT_EQ(Sink->String(), "<test/>");
}

TEST(XMLWriter, CharDataTest) {
    auto Sink = std::make_shared<CStringDataSink>();
    CXMLWriter Writer(Sink);
    
    SXMLEntity Element;
    Element.DType = SXMLEntity::EType::StartElement;
    Element.DNameData = "test";
    EXPECT_TRUE(Writer.WriteEntity(Element));
    
    Element.DType = SXMLEntity::EType::CharData;
    Element.DNameData = "Hello & Goodbye";
    EXPECT_TRUE(Writer.WriteEntity(Element));
    
    Element.DType = SXMLEntity::EType::EndElement;
    Element.DNameData = "test";
    EXPECT_TRUE(Writer.WriteEntity(Element));
    
    EXPECT_EQ(Sink->String(), "<test>Hello &amp; Goodbye</test>");
}

TEST(XMLWriter, AttributeTest) {
    auto Sink = std::make_shared<CStringDataSink>();
    CXMLWriter Writer(Sink);
    
    SXMLEntity Element;
    Element.DType = SXMLEntity::EType::StartElement;
    Element.DNameData = "test";
    Element.DAttributes.push_back(std::make_pair("attr", "value"));
    EXPECT_TRUE(Writer.WriteEntity(Element));
    EXPECT_TRUE(Writer.Flush());
    EXPECT_EQ(Sink->String(), "<test attr=\"value\"></test>");
}

TEST(XMLReader, BasicTest) {
    auto Source = std::make_shared<CStringDataSource>("<test>Hello</test>");
    CXMLReader Reader(Source);
    
    SXMLEntity Entity;
    EXPECT_TRUE(Reader.ReadEntity(Entity));
    EXPECT_EQ(Entity.DType, SXMLEntity::EType::StartElement);
    EXPECT_EQ(Entity.DNameData, "test");
    
    EXPECT_TRUE(Reader.ReadEntity(Entity));
    EXPECT_EQ(Entity.DType, SXMLEntity::EType::CharData);
    EXPECT_EQ(Entity.DNameData, "Hello");
    
    EXPECT_TRUE(Reader.ReadEntity(Entity));
    EXPECT_EQ(Entity.DType, SXMLEntity::EType::EndElement);
    EXPECT_EQ(Entity.DNameData, "test");
    
    EXPECT_TRUE(Reader.End());
}

TEST(XMLReader, AttributeTest) {
    auto Source = std::make_shared<CStringDataSource>("<test attr=\"value\">Hello</test>");
    CXMLReader Reader(Source);
    
    SXMLEntity Entity;
    EXPECT_TRUE(Reader.ReadEntity(Entity));
    EXPECT_EQ(Entity.DType, SXMLEntity::EType::StartElement);
    EXPECT_EQ(Entity.DNameData, "test");
    EXPECT_TRUE(Entity.AttributeExists("attr"));
    EXPECT_EQ(Entity.AttributeValue("attr"), "value");
    
    EXPECT_TRUE(Reader.ReadEntity(Entity));
    EXPECT_EQ(Entity.DType, SXMLEntity::EType::CharData);
    EXPECT_EQ(Entity.DNameData, "Hello");
    
    EXPECT_TRUE(Reader.ReadEntity(Entity));
    EXPECT_EQ(Entity.DType, SXMLEntity::EType::EndElement);
    EXPECT_EQ(Entity.DNameData, "test");
    
    EXPECT_TRUE(Reader.End());
}

TEST(XMLReader, SkipCDataTest) {
    auto Source = std::make_shared<CStringDataSource>("<test> <inner>Hello</inner> </test>");
    CXMLReader Reader(Source);
    
    SXMLEntity Entity;
    EXPECT_TRUE(Reader.ReadEntity(Entity, true));
    EXPECT_EQ(Entity.DType, SXMLEntity::EType::StartElement);
    EXPECT_EQ(Entity.DNameData, "test");
    
    EXPECT_TRUE(Reader.ReadEntity(Entity, true));
    EXPECT_EQ(Entity.DType, SXMLEntity::EType::StartElement);
    EXPECT_EQ(Entity.DNameData, "inner");
    
    EXPECT_TRUE(Reader.ReadEntity(Entity, true));
    EXPECT_EQ(Entity.DType, SXMLEntity::EType::EndElement);
    EXPECT_EQ(Entity.DNameData, "inner");
    
    EXPECT_TRUE(Reader.ReadEntity(Entity, true));
    EXPECT_EQ(Entity.DType, SXMLEntity::EType::EndElement);
    EXPECT_EQ(Entity.DNameData, "test");
    
    EXPECT_TRUE(Reader.End());
}