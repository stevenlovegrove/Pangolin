#pragma once

#include "table_loader.h"

#include <fstream>
#include <memory>

namespace pangolin
{

class CsvTableLoader : public TableLoaderInterface
{
  public:
  /// Construct to read from the set of files \param csv_files such that
  /// each row consists of the columns of each file in the provided order
  ///
  /// \param csv_files a list of CSV files to read from
  /// \param delim the field delimiter between columns, normally ',' for CSV
  CsvTableLoader(
      std::vector<std::string> const& csv_files, char delim = ',',
      char comment = '#');

  bool SkipLines(std::vector<size_t> const& lines_per_input);

  bool ReadRow(std::vector<std::string>& row) override;

  private:
  static bool AppendColumns(
      std::vector<std::string>& cols, std::istream& s, char delim,
      char comment);

  char delim;
  char comment;
  std::vector<std::istream*> streams;
  std::vector<std::unique_ptr<std::istream>> owned_streams;
};

}  // namespace pangolin
