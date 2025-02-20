#include "DSVReader.h"
#include <sstream>

struct CDSVReader::SImplementation {
    std::shared_ptr<CDataSource> DDataSource;
    char DDelimiter;
    
    SImplementation(std::shared_ptr<CDataSource> src, char delimiter) 
        : DDataSource(src), DDelimiter(delimiter == '"' ? ',' : delimiter) {}
    
    bool ReadField(std::string& field) {
        field.clear();
        char ch;
        bool in_quotes = false;
        
        // Skip leading whitespace
        while (DDataSource->Peek(ch) && (ch == ' ' || ch == '\t')) {
            DDataSource->Get(ch);
        }
        
        // Check if field starts with quote
        if (DDataSource->Peek(ch) && ch == '"') {
            in_quotes = true;
            DDataSource->Get(ch); // consume quote
        }
        
        while (DDataSource->Get(ch)) {
            if (in_quotes) {
                if (ch == '"') {
                    if (DDataSource->Peek(ch) && ch == '"') {
                        // Double quote - add single quote to field
                        DDataSource->Get(ch);
                        field += ch;
                    } else {
                        // End of quoted field
                        in_quotes = false;
                    }
                } else {
                    field += ch;
                }
            } else {
                if (ch == DDelimiter || ch == '\n' || ch == '\r') {
                    // If we hit a delimiter or newline, put it back and return
                    // DDataSource->UnGet(ch);  // UnGet not available in interface
                    return true;
                } else if (ch == '"') {
                    in_quotes = true;
                } else {
                    field += ch;
                }
            }
        }
        
        return !field.empty() || !DDataSource->End();
    }
};

CDSVReader::CDSVReader(std::shared_ptr<CDataSource> src, char delimiter)
    : DImplementation(std::make_unique<SImplementation>(src, delimiter)) {}

CDSVReader::~CDSVReader() = default;

bool CDSVReader::End() const {
    return DImplementation->DDataSource->End();
}

bool CDSVReader::ReadRow(std::vector<std::string>& row) {
    row.clear();
    char ch;
    std::string field;
    
    // Handle empty line case
    if (DImplementation->DDataSource->Peek(ch) && (ch == '\n' || ch == '\r')) {
        while (DImplementation->DDataSource->Peek(ch) && (ch == '\n' || ch == '\r')) {
            DImplementation->DDataSource->Get(ch);
        }
        return true;
    }
    
    do {
        if (!DImplementation->ReadField(field)) {
            return !row.empty();
        }
        row.push_back(field);
        
        // Check for end of line or end of input
        if (!DImplementation->DDataSource->Peek(ch)) {
            break;
        }
        if (ch == '\n' || ch == '\r') {
            while (DImplementation->DDataSource->Peek(ch) && (ch == '\n' || ch == '\r')) {
                DImplementation->DDataSource->Get(ch);
            }
            break;
        }
        // Must be delimiter, consume it
        if (ch == DImplementation->DDelimiter) {
            DImplementation->DDataSource->Get(ch);
        }
    } while (true);
    
    return true;
}