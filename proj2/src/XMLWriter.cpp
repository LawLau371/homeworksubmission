#include "XMLWriter.h"
#include <stack>
#include <sstream>

struct CXMLWriter::SImplementation {
    std::shared_ptr<CDataSink> OutputSink;
    std::stack<std::string> ElementStack;
    
    SImplementation(std::shared_ptr<CDataSink> sink) : OutputSink(sink) {}
    
    bool WriteText(const std::string& text) {
        for(char character : text) {
            if(!OutputSink->Put(character)) {
                return false;
            }
        }
        return true;
    }
    
    std::string ConvertCharData(const std::string& inputData) {
        std::string encodedString;
        for(char character : inputData) {
            switch(character) {
                case '<':
                    encodedString += "&lt;";
                    break;
                case '>':
                    encodedString += "&gt;";
                    break;
                case '&':
                    encodedString += "&amp;";
                    break;
                case '\'':
                    encodedString += "&apos;";
                    break;
                case '"':
                    encodedString += "&quot;";
                    break;
                default:
                    encodedString += character;
            }
        }
        return encodedString;
    }
    
    bool WriteElementAttributes(const std::vector<SXMLEntity::TAttribute>& attributeList) {
        for(const auto& attribute : attributeList) {
            if(!WriteText(" ") ||
               !WriteText(std::get<0>(attribute)) ||
               !WriteText("=\"") ||
               !WriteText(ConvertCharData(std::get<1>(attribute))) ||
               !WriteText("\"")) {
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
    while(!DImplementation->ElementStack.empty()) {
        if(!DImplementation->WriteText("</") ||
           !DImplementation->WriteText(DImplementation->ElementStack.top()) ||
           !DImplementation->WriteText(">")) {
            return false;
        }
        DImplementation->ElementStack.pop();
    }
    return true;
}

bool CXMLWriter::WriteEntity(const SXMLEntity& entity) {
    switch(entity.DType) {
        case SXMLEntity::EType::StartElement:
            if(!DImplementation->WriteText("<") ||
               !DImplementation->WriteText(entity.DNameData) ||
               !DImplementation->WriteElementAttributes(entity.DAttributes) ||
               !DImplementation->WriteText(">")) {
                return false;
            }
            DImplementation->ElementStack.push(entity.DNameData);
            return true;
            
        case SXMLEntity::EType::EndElement:
            if(DImplementation->ElementStack.empty() ||
               DImplementation->ElementStack.top() != entity.DNameData) {
                return false;
            }
            if(!DImplementation->WriteText("</") ||
               !DImplementation->WriteText(entity.DNameData) ||
               !DImplementation->WriteText(">")) {
                return false;
            }
            DImplementation->ElementStack.pop();
            return true;
            
        case SXMLEntity::EType::CharData:
            return DImplementation->WriteText(DImplementation->ConvertCharData(entity.DNameData));
            
        case SXMLEntity::EType::CompleteElement:
            if(!DImplementation->WriteText("<") ||
               !DImplementation->WriteText(entity.DNameData) ||
               !DImplementation->WriteElementAttributes(entity.DAttributes) ||
               !DImplementation->WriteText("/>")) {
                return false;
            }
            return true;
    }
    return false;
}
