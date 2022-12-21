/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <pangolin/platform.h>

#include <algorithm>
#include <string>
#include <vector>

namespace pangolin
{

PANGOLIN_EXPORT
std::vector<std::string>& Split(
    std::string const& s, char delim, std::vector<std::string>& elements);

PANGOLIN_EXPORT
std::vector<std::string> Split(std::string const& s, char delim);

PANGOLIN_EXPORT
std::vector<std::string> Expand(
    std::string const& s, char open = '[', char close = ']', char delim = ',');

PANGOLIN_EXPORT
std::string SanitizePath(std::string const& path);

PANGOLIN_EXPORT
std::string PathParent(std::string const& path, int levels = 1);

PANGOLIN_EXPORT
bool FileExists(std::string const& filename);

PANGOLIN_EXPORT
std::string FindPath(
    std::string const& child_path, std::string const& signature_path);

PANGOLIN_EXPORT
std::string PathExpand(std::string const& sPath);

PANGOLIN_EXPORT
bool MatchesWildcard(std::string const& str, std::string const& wildcard);

PANGOLIN_EXPORT
std::string GetFileContents(std::string const& filename);

PANGOLIN_EXPORT
std::string GetExecutablePath();

PANGOLIN_EXPORT
std::string GetExecutableDir();

enum class SortMethod { STANDARD, NATURAL };

// Fill 'file_vec' with the files that match the glob-like 'wildcard_file_path'
// ? can be used to match any single charector
// * can be used to match any sequence of charectors in a directory
// ** can be used to match any directories across any number of levels
//   e.g. FilesMatchingWildcard("~/*/code/*.h", vec);
//   e.g. FilesMatchingWildcard("~/**/*.png", vec);
// sort the file_vec according to the specified sorting method.
PANGOLIN_EXPORT
bool FilesMatchingWildcard(
    std::string const& wildcard_file_path, std::vector<std::string>& file_vec,
    SortMethod sort_method = SortMethod::STANDARD);

PANGOLIN_EXPORT
std::string MakeUniqueFilename(std::string const& filename);

PANGOLIN_EXPORT
bool IsPipe(std::string const& file);

PANGOLIN_EXPORT
bool IsPipe(int fd);

PANGOLIN_EXPORT
int WritablePipeFileDescriptor(std::string const& file);

/**
 * Open the file for reading. Note that it is opened with O_NONBLOCK.  The pipe
 * open is done in two stages so that the producer knows a reader is waiting
 * (but not blocked). The reader then checks PipeHasDataToRead() until it
 * returns true. The file can then be opened. Note that the file descriptor
 * should be closed after the read stream has been created so that the write
 * side of the pipe does not get signaled.
 */
PANGOLIN_EXPORT
int ReadablePipeFileDescriptor(std::string const& file);

PANGOLIN_EXPORT
bool PipeHasDataToRead(int fd);

PANGOLIN_EXPORT
void FlushPipe(std::string const& file);

// TODO: Tidy these inlines up / move them

inline bool StartsWith(std::string const& str, std::string const& prefix)
{
  return !str.compare(0, prefix.size(), prefix);
}

inline bool EndsWith(std::string const& str, std::string const& prefix)
{
  return !str.compare(str.size() - prefix.size(), prefix.size(), prefix);
}

inline std::string Trim(
    std::string const& str, std::string const& delimiters = " \f\n\r\t\v")
{
  const size_t f = str.find_first_not_of(delimiters);
  return f == std::string::npos
             ? ""
             : str.substr(f, str.find_last_not_of(delimiters) + 1);
}

inline void ToUpper(std::string& str)
{
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

inline void ToLower(std::string& str)
{
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

inline std::string ToUpperCopy(std::string const& str)
{
  std::string out;
  out.resize(str.size());
  std::transform(str.begin(), str.end(), out.begin(), ::toupper);
  return out;
}

inline std::string ToLowerCopy(std::string const& str)
{
  std::string out;
  out.resize(str.size());
  std::transform(str.begin(), str.end(), out.begin(), ::tolower);
  return out;
}

}  // namespace pangolin
