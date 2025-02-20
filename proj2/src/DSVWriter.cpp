#include "DSVWriter.h"
#include <sstream>

struct CDSVWriter::SImplementation {
    std::shared_ptr<CDataSink> DDataSink;
    char DDelimiter;
    bool DQuoteAll;
    
    SImplementation(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall) 
        : DDataSink(sink), DDelimiter(delimiter == '"' ? ',' : delimiter), DQuoteAll(quoteall) {}
    
    bool NeedsQuoting(const std::string& str) const {
        if (DQuoteAll) return true;
        
        for (char ch : str) {
            if (ch == DDelimiter || ch == '"' || ch == '\n' || ch == '\r') {
                return true;
            }
        }
        return false;
    }
    
    bool WriteQuotedField(const std::string& field) {
        if (!DDataSink->Put('"')) return false;
        
        for (char ch : field) {
            if (ch == '"') {
                // Double up quotes
                if (!DDataSink->Put('"')) return false;
                if (!DDataSink->Put('"')) return false;
            } else {
                if (!DDataSink->Put(ch)) return false;
            }
        }
        
        return DDataSink->Put('"');
    }
    
    bool WriteField(const std::string& field) {
        if (NeedsQuoting(field)) {
            return WriteQuotedField(field);
        }
        
        for (char ch : field) {
            if (!DDataSink->Put(ch)) return false;
        }
        return true;
    }
};

CDSVWriter::CDSVWriter(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall)
    : DImplementation(std::make_unique<SImplementation>(sink, delimiter, quoteall)) {}

CDSVWriter::~CDSVWriter() = default;

bool CDSVWriter::WriteRow(const std::vector<std::string>& row) {
    if (row.empty()) {
        return DImplementation->DDataSink->Put('\n');
    }
    
    for (size_t i = 0; i < row.size(); ++i) {
        if (!DImplementation->WriteField(row[i])) {
            return false;
        }
        
        if (i < row.size() - 1) {
            if (!DImplementation->DDataSink->Put(DImplementation->DDelimiter)) {
                return false;
            }
        }
    }
    
    return DImplementation->DDataSink->Put('\n');
}