#include <pangolin/plot/loaders/csv_table_loader.h>
#include <fstream>
#include <memory>

namespace pangolin {

CsvTableLoader::CsvTableLoader(const std::vector<std::string>& csv_files, char delim, char comment)
    : delim(delim), comment(comment)
{
    for(const auto& f : csv_files) {
        if(f == "-") {
            streams.push_back(&std::cin);
        }else{
            std::ifstream* pFile = new std::ifstream(f);
            owned_streams.emplace_back(pFile);
            streams.push_back(pFile);
            PANGO_ASSERT(pFile->is_open());
        }
    }
}

bool CsvTableLoader::SkipLines(const std::vector<size_t>& lines_per_input)
{
    if(lines_per_input.size()) {
        PANGO_ASSERT(lines_per_input.size() == streams.size());
        std::vector<std::string> dummy_row;

        for(size_t i=0; i < streams.size(); ++i) {
            for(size_t r=0; r < lines_per_input[i]; ++r) {
                if(!AppendColumns(dummy_row, *streams[i], delim, '\0')) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool CsvTableLoader::ReadRow(std::vector<std::string>& row)
{
    row.clear();

    for(auto& s : streams) {
        if(!AppendColumns(row, *s, delim, comment)) {
            return false;
        }
    }

    return true;
}

bool CsvTableLoader::AppendColumns(std::vector<std::string>& cols, std::istream& s, char delim, char comment)
{
    // Read line from stream
    std::string row;
    do {
        std::getline(s,row);
    }while(row.length() > 0 && row[0] == comment);


    // Failure if no lines to read
    if(!s.good()) return false;

    std::stringstream row_stream(row);
    std::string cell;

    // Read cells
    while(std::getline(row_stream, cell, delim)) {
        cols.push_back(cell);
    }

    // Check for an empty trailing cell
    if (!row_stream && cell.empty()) {
        cols.push_back("");
    }

    return true;
}

}
