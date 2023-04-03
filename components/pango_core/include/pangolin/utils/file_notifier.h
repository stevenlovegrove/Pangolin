#include <functional>
#include <iostream>
#include <memory>
#include <string>

class FileChangeNotifier
{
  public:
  using Callback = std::function<void(const std::string&)>;

  virtual ~FileChangeNotifier() = default;

  static std::shared_ptr<FileChangeNotifier> create(
      const std::string& path, Callback callback);
};
