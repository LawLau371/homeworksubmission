#include "XMLWriter.h"
#include <stack>
#include <sstream>

struct CXMLWriter::SImplementation {
    std::shared_ptr<CDataSink> DDataSink;
    std::stack<std::string> DElementStack;
    
    SImplementation(std::shared_ptr<CDataSink> sink) : DDataSink(sink) {}
    
    bool WriteString(const std::string& str) {
        for(char ch : str) {
            if(!DDataSink->Put(ch)) {
                return false;
            }
        }
        return true;
    }
    
    std::string EncodeCharData(const std::string& data) {
        std::string Result;
        for(char ch : data) {
            switch(ch) {
                case '<':
                    Result += "&lt;";
                    break;
                case '>':
                    Result += "&gt;";
                    break;
                case '&':
                    Result += "&amp;";
                    break;
                case '\'':
                    Result += "&apos;";
                    break;
                case '"':
                    Result += "&quot;";
                    break;
                default:
                    Result += ch;
            }
        }
        return Result;
    }
    
    bool WriteAttributes(const std::vector<SXMLEntity::TAttribute>& attributes) {
        for(const auto& attr : attributes) {
            if(!WriteString(" ") ||
               !WriteString(std::get<0>(attr)) ||
               !WriteString("=\"") ||
               !WriteString(EncodeCharData(std::get<1>(attr))) ||
               !WriteString("\"")) {
                return false;
            }
        }
        return true;
    }
};

CXMLWriter::CXMLWriter(std::shared_ptr<CDataSink> sink)
    : DImplementation(std::make_unique<SImplementation>(sink)) {
}

CXMLWriter::~CXMLWriter() = default;

bool CXMLWriter::Flush() {
    while(!DImplementation->DElementStack.empty()) {
        if(!DImplementation->WriteString("</") ||
           !DImplementation->WriteString(DImplementation->DElementStack.top()) ||
           !DImplementation->WriteString(">")) {
            return false;
        }
        DImplementation->DElementStack.pop();
    }
    return true;
}

bool CXMLWriter::WriteEntity(const SXMLEntity& entity) {
    switch(entity.DType) {
        case SXMLEntity::EType::StartElement:
            if(!DImplementation->WriteString("<") ||
               !DImplementation->WriteString(entity.DNameData) ||
               !DImplementation->WriteAttributes(entity.DAttributes) ||
               !DImplementation->WriteString(">")) {
                return false;
            }
            DImplementation->DElementStack.push(entity.DNameData);
            return true;
            
        case SXMLEntity::EType::EndElement:
            if(DImplementation->DElementStack.empty() ||
               DImplementation->DElementStack.top() != entity.DNameData) {
                return false;
            }
            if(!DImplementation->WriteString("</") ||
               !DImplementation->WriteString(entity.DNameData) ||
               !DImplementation->WriteString(">")) {
                return false;
            }
            DImplementation->DElementStack.pop();
            return true;
            
        case SXMLEntity::EType::CharData:
            return DImplementation->WriteString(DImplementation->EncodeCharData(entity.DNameData));
            
        case SXMLEntity::EType::CompleteElement:
            if(!DImplementation->WriteString("<") ||
               !DImplementation->WriteString(entity.DNameData) ||
               !DImplementation->WriteAttributes(entity.DAttributes) ||
               !DImplementation->WriteString("/>")) {
                return false;
            }
            return true;
    }
    return false;
}