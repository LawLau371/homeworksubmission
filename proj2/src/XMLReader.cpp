#include "XMLReader.h"
#include <expat.h>
#include <stack>
#include <cstring>

struct CXMLReader::SImplementation {
    std::shared_ptr<CDataSource> DataSource;
    XML_Parser Parser;
    std::vector<SXMLEntity> EntityQueue;
    bool Error;
    
    static void StartElementHandler(void* userData, const XML_Char* name, const XML_Char** attrs) {
        auto Implementation = static_cast<SImplementation*>(userData);
        SXMLEntity Entity;
        Entity.DType = SXMLEntity::EType::StartElement;
        Entity.DNameData = name;
        
        for(size_t Index = 0; attrs[Index]; Index += 2) {
            Entity.DAttributes.emplace_back(attrs[Index], attrs[Index + 1]);
        }
        
        Implementation->EntityQueue.push_back(Entity);
    }
    
    static void EndElementHandler(void* userData, const XML_Char* name) {
        auto Implementation = static_cast<SImplementation*>(userData);
        SXMLEntity Entity;
        Entity.DType = SXMLEntity::EType::EndElement;
        Entity.DNameData = name;
        Implementation->EntityQueue.push_back(Entity);
    }
    
    static void CharDataHandler(void* userData, const XML_Char* s, int len) {
        auto Implementation = static_cast<SImplementation*>(userData);
        std::string Data(s, len);
        if(Data.find_first_not_of(" \t\n\r") != std::string::npos) {
            SXMLEntity Entity;
            Entity.DType = SXMLEntity::EType::CharData;
            Entity.DNameData = Data;
            Implementation->EntityQueue.push_back(Entity);
        }
    }
    
    SImplementation(std::shared_ptr<CDataSource> src)
        : DataSource(src), Error(false) {
        Parser = XML_ParserCreate(NULL);
        XML_SetUserData(Parser, this);
        XML_SetElementHandler(Parser, StartElementHandler, EndElementHandler);
        XML_SetCharacterDataHandler(Parser, CharDataHandler);
    }
    
    ~SImplementation() {
        XML_ParserFree(Parser);
    }
    
    bool ParseNextEntity() {
        if(Error) {
            return false;
        }
        
        std::vector<char> Buffer(1024);
        bool Result = DataSource->Read(Buffer, Buffer.size());
        if(!Result && DataSource->End()) {
            return XML_Parse(Parser, Buffer.data(), Buffer.size(), true) == XML_STATUS_OK;
        }
        return XML_Parse(Parser, Buffer.data(), Buffer.size(), false) == XML_STATUS_OK;
    }
};

CXMLReader::CXMLReader(std::shared_ptr<CDataSource> src)
    : DImplementation(std::make_unique<SImplementation>(src)) {}

CXMLReader::~CXMLReader() = default;

bool CXMLReader::End() const {
    return DImplementation->DataSource->End() && DImplementation->EntityQueue.empty();
}

bool CXMLReader::ReadEntity(SXMLEntity &entity, bool skipcdata) {
    while(DImplementation->EntityQueue.empty()) {
        if(!DImplementation->ParseNextEntity()) {
            return false;
        }
        if(DImplementation->DataSource->End() && DImplementation->EntityQueue.empty()) {
            return false;
        }
    }
    
    entity = DImplementation->EntityQueue.front();
    DImplementation->EntityQueue.erase(DImplementation->EntityQueue.begin());
    
    if(skipcdata && entity.DType == SXMLEntity::EType::CharData) {
        return ReadEntity(entity, skipcdata);
    }
    
    return true;
}
