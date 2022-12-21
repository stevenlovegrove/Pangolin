#pragma once

#include <pangolin/utils/uri.h>

#include <regex>
#include <unordered_set>

namespace pangolin
{

struct PANGOLIN_EXPORT ParamSet {
  struct PANGOLIN_EXPORT Param {
    std::string name_regex;
    std::string default_value;
    std::string description;
  };

  std::vector<Param> params;
  std::string str() const;
};

class PANGOLIN_EXPORT ParamReader
{
  public:
  class ParamReaderException : public std::exception
  {
    public:
    ParamReaderException(std::string const& param_name)
    {
      error_message_ = param_name + " was not found in the parameter set";
    }
    virtual char const* what() const noexcept override
    {
      return error_message_.c_str();
    }

    private:
    std::string error_message_;
  };

  ParamReader(ParamSet const& param_set, Uri const& uri) :
      param_set_(param_set), uri_(uri)
  {
  }

  template <typename T>
  T Get(std::string const& param_name) const
  {
    ParamSet::Param const* param = GetMatchingParamFromParamSet(param_name);
    if (param) {
      return GetHelper(
          param_name, Convert<T, std::string>::Do(param->default_value));
    }
    throw ParamReaderException(param_name);
  }

  template <typename T>
  T Get(std::string const& param_name, const T& default_value) const
  {
    ParamSet::Param const* param = GetMatchingParamFromParamSet(param_name);
    if (param) {
      return GetHelper(param_name, default_value);
    }
    throw ParamReaderException(param_name);
  }

  bool Contains(std::string const& param_name);

  std::unordered_set<std::string> FindUnrecognizedUriParams();

  private:
  template <typename T>
  T GetHelper(std::string const& param_name, const T& default_value) const
  {
    return uri_.Get(param_name, default_value);
  }

  ParamSet::Param const* GetMatchingParamFromParamSet(
      std::string const& param_name) const;

  private:
  const ParamSet param_set_;
  const Uri uri_;
};

}  // namespace pangolin
