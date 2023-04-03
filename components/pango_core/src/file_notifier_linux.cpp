#ifdef __linux__

#include <limits.h>
#include <pangolin/utils/file_notifier.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <stdexcept>
#include <thread>

class FileChangeNotifierLinuxImpl : public FileChangeNotifier
{
  public:
  FileChangeNotifierLinuxImpl(const std::string& path, Callback callback);

  virtual ~FileChangeNotifierLinuxImpl() override;

  private:
  std::string path_;
  Callback callback_;
  int inotifyFd_;
  int watchDescriptor_;
  std::thread monitorThread_;

  void monitorInotifyEvents();
};

FileChangeNotifierLinuxImpl::FileChangeNotifierLinuxImpl(
    const std::string& path, Callback callback) :
    path_(path), callback_(callback)
{
  inotifyFd_ = inotify_init();
  if (inotifyFd_ == -1) {
    throw std::runtime_error("Failed to initialize inotify.");
  }

  watchDescriptor_ = inotify_add_watch(inotifyFd_, path.c_str(), IN_ALL_EVENTS);
  if (watchDescriptor_ == -1) {
    close(inotifyFd_);
    throw std::runtime_error("Failed to add inotify watch.");
  }

  monitorThread_ =
      std::thread(&FileChangeNotifierLinuxImpl::monitorInotifyEvents, this);
}

FileChangeNotifierLinuxImpl::~FileChangeNotifierLinuxImpl()
{
  inotify_rm_watch(inotifyFd_, watchDescriptor_);
  close(inotifyFd_);
  monitorThread_.join();
}

void FileChangeNotifierLinuxImpl::monitorInotifyEvents()
{
  char buffer[sizeof(struct inotify_event) + NAME_MAX + 1];

  while (true) {
    ssize_t bytesRead = read(inotifyFd_, buffer, sizeof(buffer));
    if (bytesRead == -1) {
      break;
    }

    const inotify_event* event = reinterpret_cast<inotify_event*>(buffer);
    if (event->len > 0) {
      callback_(std::string(event->name));
    }
  }
}

std::shared_ptr<FileChangeNotifier> FileChangeNotifier::create(
    const std::string& path, Callback callback)
{
  return std::make_shared<FileChangeNotifierLinuxImpl>(path, callback);
}

#endif
