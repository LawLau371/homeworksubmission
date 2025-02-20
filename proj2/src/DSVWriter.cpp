#include "DSVWriter.h"
#include <sstream>

struct CDSVWriter::SImplementation {
    std::shared_ptr<CDataSink> DataSink;
    char Delimiter;
    bool QuoteAll;
    
    SImplementation(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall) 
        : DataSink(sink), Delimiter(delimiter == '"' ? ',' : delimiter), QuoteAll(quoteall) {}
    
    bool RequiresQuoting(const std::string& str) const {
        if (QuoteAll) return true;
        
        for (char character : str) {
            if (character == Delimiter || character == '"' || character == '\n' || character == '\r') {
                return true;
            }
        }
        return false;
    }
    
    bool WriteEnclosedField(const std::string& field) {
        if (!DataSink->Put('"')) return false;
        
        for (char character : field) {
            if (character == '"') {
                if (!DataSink->Put('"')) return false;
                if (!DataSink->Put('"')) return false;
            } else {
                if (!DataSink->Put(character)) return false;
            }
        }
        
        return DataSink->Put('"');
    }
    
    bool WriteSingleField(const std::string& field) {
        if (RequiresQuoting(field)) {
            return WriteEnclosedField(field);
        }
        
        for (char character : field) {
            if (!DataSink->Put(character)) return false;
        }
        return true;
    }
};

CDSVWriter::CDSVWriter(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall)
    : DImplementation(std::make_unique<SImplementation>(sink, delimiter, quoteall)) {}

CDSVWriter::~CDSVWriter() = default;

bool CDSVWriter::WriteRow(const std::vector<std::string>& row) {
    if (row.empty()) {
        return DImplementation->DataSink->Put('\n');
    }
    
    for (size_t index = 0; index < row.size(); ++index) {
        if (!DImplementation->WriteSingleField(row[index])) {
            return false;
        }
        
        if (index < row.size() - 1) {
            if (!DImplementation->DataSink->Put(DImplementation->Delimiter)) {
                return false;
            }
        }
    }
    
    return DImplementation->DataSink->Put('\n');
}
