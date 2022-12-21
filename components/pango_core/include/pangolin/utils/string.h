#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace pangolin
{

// https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring
inline void ltrim(std::string& s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}

inline void rtrim(std::string& s)
{
  s.erase(
      std::find_if(
          s.rbegin(), s.rend(),
          [](unsigned char ch) { return !std::isspace(ch); })
          .base(),
      s.end());
}

inline void trim(std::string& s)
{
  ltrim(s);
  rtrim(s);
}

inline std::string ltrimmed(std::string s)
{
  ltrim(s);
  return s;
}

inline std::string rtrimmed(std::string s)
{
  rtrim(s);
  return s;
}

inline std::string trimmed(std::string s)
{
  trim(s);
  return s;
}

inline bool startsWith(std::string const& str, std::string const& prefix)
{
  return !str.compare(0, prefix.size(), prefix);
}

inline bool endsWith(std::string const& str, std::string const& prefix)
{
  return !str.compare(str.size() - prefix.size(), prefix.size(), prefix);
}

inline void uppercase(std::string& str)
{
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

inline void lowercase(std::string& str)
{
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

inline std::string uppercased(std::string str)
{
  uppercase(str);
  return str;
}

inline std::string lowercased(std::string str)
{
  lowercase(str);
  return str;
}

// split str by whitespace
inline std::vector<std::string> tokenize(std::istringstream& iss)
{
  return {
      std::istream_iterator<std::string>{iss},
      std::istream_iterator<std::string>{}};
}

inline std::vector<std::string> split(std::string const& s, char delim)
{
  std::vector<std::string> elements;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elements.push_back(item);
  }
  return elements;
}

}  // namespace pangolin
