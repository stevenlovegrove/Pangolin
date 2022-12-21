#pragma once

template <typename R>
bool is_ready(std::future<R> const& f)
{
  return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

inline std::vector<std::string> ExpandGlobOption(
    argagg::option_results const& opt)
{
  std::vector<std::string> expanded;
  for (auto const& o : opt.all) {
    const std::string r = o.as<std::string>();
    pangolin::FilesMatchingWildcard(r, expanded);
  }
  return expanded;
}

template <typename Tout, typename Tin, typename F>
inline std::vector<Tout> TryLoad(std::vector<Tin> const& in, const F& load_func)
{
  std::vector<Tout> loaded;
  for (Tin const& file : in) {
    try {
      loaded.emplace_back(load_func(file));
    } catch (std::exception const&) {
    }
  }
  return loaded;
}
