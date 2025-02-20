#include "DSVReader.h"
#include <sstream>

struct CDSVReader::SImplementation {
    std::shared_ptr<CDataSource> DataSource;
    char Delimiter;
    
    SImplementation(std::shared_ptr<CDataSource> src, char delimiter) 
        : DataSource(src), Delimiter(delimiter == '"' ? ',' : delimiter) {}
    
    bool ExtractField(std::string& value) {
        value.clear();
        char character;
        bool insideQuotes = false;
        
        while (DataSource->Peek(character) && (character == ' ' || character == '\t')) {
            DataSource->Get(character);
        }
        
        if (DataSource->Peek(character) && character == '"') {
            insideQuotes = true;
            DataSource->Get(character);
        }
        
        while (DataSource->Get(character)) {
            if (insideQuotes) {
                if (character == '"') {
                    if (DataSource->Peek(character) && character == '"') {
                        DataSource->Get(character);
                        value += character;
                    } else {
                        insideQuotes = false;
                    }
                } else {
                    value += character;
                }
            } else {
                if (character == Delimiter || character == '\n' || character == '\r') {
                    return true;
                } else if (character == '"') {
                    insideQuotes = true;
                } else {
                    value += character;
                }
            }
        }
        
        return !value.empty() || !DataSource->End();
    }
};

CDSVReader::CDSVReader(std::shared_ptr<CDataSource> src, char delimiter)
    : DImplementation(std::make_unique<SImplementation>(src, delimiter)) {}

CDSVReader::~CDSVReader() = default;

bool CDSVReader::End() const {
    return DImplementation->DataSource->End();
}

bool CDSVReader::ReadRow(std::vector<std::string>& row) {
    row.clear();
    char character;
    std::string value;
    
    if (DImplementation->DataSource->Peek(character) && (character == '\n' || character == '\r')) {
        while (DImplementation->DataSource->Peek(character) && (character == '\n' || character == '\r')) {
            DImplementation->DataSource->Get(character);
        }
        return true;
    }
    
    do {
        if (!DImplementation->ExtractField(value)) {
            return !row.empty();
        }
        row.push_back(value);
        
        if (!DImplementation->DataSource->Peek(character)) {
            break;
        }
        if (character == '\n' || character == '\r') {
            while (DImplementation->DataSource->Peek(character) && (character == '\n' || character == '\r')) {
                DImplementation->DataSource->Get(character);
            }
            break;
        }
        if (character == DImplementation->Delimiter) {
            DImplementation->DataSource->Get(character);
        }
    } while (true);
    
    return true;
}
