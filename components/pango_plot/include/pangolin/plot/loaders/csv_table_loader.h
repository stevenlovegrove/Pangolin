#pragma once

#include <fstream>
#include <memory>
#include "table_loader.h"

namespace pangolin {

class CsvTableLoader : public TableLoaderInterface
{
public:
    CsvTableLoader(const std::vector<std::string>& csv_files, char delim = ',');

    bool SkipLines(const std::vector<size_t>& lines_per_input);

    bool ReadRow(std::vector<std::string>& row) override;

private:
    static bool AppendColumns(std::vector<std::string>& cols, std::istream& s, char delim);

    char delim;
    std::vector<std::istream*> streams;
    std::vector<std::unique_ptr<std::istream>> owned_streams;
};

}
