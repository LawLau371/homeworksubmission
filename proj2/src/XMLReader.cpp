#include "XMLReader.h"
#include <expat.h>
#include <stack>
#include <cstring>

struct CXMLReader::SImplementation {
    std::shared_ptr<CDataSource> DDataSource;
    XML_Parser DParser;
    std::vector<SXMLEntity> DEntityQueue;
    bool DError;
    
    static void StartElementHandler(void* userData, const XML_Char* name, const XML_Char** attrs) {
        auto Implementation = static_cast<SImplementation*>(userData);
        SXMLEntity Entity;
        Entity.DType = SXMLEntity::EType::StartElement;
        Entity.DNameData = name;
        
        for(size_t Index = 0; attrs[Index]; Index += 2) {
            Entity.DAttributes.push_back(std::make_pair(attrs[Index], attrs[Index + 1]));
        }
        
        Implementation->DEntityQueue.push_back(Entity);
    }
    
    static void EndElementHandler(void* userData, const XML_Char* name) {
        auto Implementation = static_cast<SImplementation*>(userData);
        SXMLEntity Entity;
        Entity.DType = SXMLEntity::EType::EndElement;
        Entity.DNameData = name;
        Implementation->DEntityQueue.push_back(Entity);
    }
    
    static void CharDataHandler(void* userData, const XML_Char* s, int len) {
        auto Implementation = static_cast<SImplementation*>(userData);
        std::string Data(s, len);
        if(Data.find_first_not_of(" \t\n\r") != std::string::npos) {
            SXMLEntity Entity;
            Entity.DType = SXMLEntity::EType::CharData;
            Entity.DNameData = Data;
            Implementation->DEntityQueue.push_back(Entity);
        }
    }
    
    SImplementation(std::shared_ptr<CDataSource> src)
        : DDataSource(src), DError(false) {
        DParser = XML_ParserCreate(NULL);
        XML_SetUserData(DParser, this);
        XML_SetElementHandler(DParser, StartElementHandler, EndElementHandler);
        XML_SetCharacterDataHandler(DParser, CharDataHandler);
    }
    
    ~SImplementation() {
        XML_ParserFree(DParser);
    }
    
    bool ParseNextEntity() {
        if(DError) {
            return false;
        }
        
        std::vector<char> Buffer(1024);
        bool Result = DDataSource->Read(Buffer, Buffer.size());
        if(!Result && DDataSource->End()) {
            return XML_Parse(DParser, Buffer.data(), Buffer.size(), true) == XML_STATUS_OK;
        }
        return XML_Parse(DParser, Buffer.data(), Buffer.size(), false) == XML_STATUS_OK;
    }
};

CXMLReader::CXMLReader(std::shared_ptr<CDataSource> src)
    : DImplementation(std::make_unique<SImplementation>(src)) {
}

CXMLReader::~CXMLReader() = default;

bool CXMLReader::End() const {
    return DImplementation->DDataSource->End() && DImplementation->DEntityQueue.empty();
}

bool CXMLReader::ReadEntity(SXMLEntity &entity, bool skipcdata) {
    while(DImplementation->DEntityQueue.empty()) {
        if(!DImplementation->ParseNextEntity()) {
            return false;
        }
        if(DImplementation->DDataSource->End() && DImplementation->DEntityQueue.empty()) {
            return false;
        }
    }
    
    entity = DImplementation->DEntityQueue.front();
    DImplementation->DEntityQueue.erase(DImplementation->DEntityQueue.begin());
    
    if(skipcdata && entity.DType == SXMLEntity::EType::CharData) {
        return ReadEntity(entity, skipcdata);
    }
    
    return true;
}