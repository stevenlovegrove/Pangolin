#ifdef __APPLE__

#include <CoreServices/CoreServices.h>
#include <dispatch/dispatch.h>
#include <pangolin/utils/file_notifier.h>

class FileChangeNotifierMacOSImpl : public FileChangeNotifier
{
  public:
  FileChangeNotifierMacOSImpl(const std::string& path, Callback callback);

  virtual ~FileChangeNotifierMacOSImpl() override;

  private:
  std::string path_;
  Callback callback_;
  FSEventStreamRef stream_;

  static void fsevents_callback(
      ConstFSEventStreamRef stream, void* clientCallBackInfo, size_t numEvents,
      void* eventPaths, const FSEventStreamEventFlags eventFlags[],
      const FSEventStreamEventId eventIds[]);
};

FileChangeNotifierMacOSImpl::FileChangeNotifierMacOSImpl(
    const std::string& path, Callback callback) :
    path_(path), callback_(callback)
{
  dispatch_queue_t queue =
      dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
  FSEventStreamContext context = {0, this, nullptr, nullptr, nullptr};
  CFStringRef cfPath = CFStringCreateWithCString(
      kCFAllocatorDefault, path.c_str(), kCFStringEncodingUTF8);
  CFArrayRef pathsToWatch = CFArrayCreate(
      kCFAllocatorDefault, (const void**)&cfPath, 1, &kCFTypeArrayCallBacks);
  stream_ = FSEventStreamCreate(
      kCFAllocatorDefault, &FileChangeNotifierMacOSImpl::fsevents_callback,
      &context, pathsToWatch, kFSEventStreamEventIdSinceNow, 0,
      kFSEventStreamCreateFlagFileEvents);
  FSEventStreamSetDispatchQueue(stream_, queue);
  FSEventStreamStart(stream_);
  CFRelease(pathsToWatch);
  CFRelease(cfPath);
}

FileChangeNotifierMacOSImpl::~FileChangeNotifierMacOSImpl()
{
  FSEventStreamStop(stream_);
  FSEventStreamInvalidate(stream_);
  FSEventStreamRelease(stream_);
}

void FileChangeNotifierMacOSImpl::fsevents_callback(
    ConstFSEventStreamRef stream, void* clientCallBackInfo, size_t numEvents,
    void* eventPaths, const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[])
{
  FileChangeNotifierMacOSImpl* notifier =
      static_cast<FileChangeNotifierMacOSImpl*>(clientCallBackInfo);
  char** paths = static_cast<char**>(eventPaths);

  for (size_t i = 0; i < numEvents; i++) {
    notifier->callback_(std::string(paths[i]));
  }
}

std::shared_ptr<FileChangeNotifier> FileChangeNotifier::create(
    const std::string& path, Callback callback)
{
  return std::make_shared<FileChangeNotifierMacOSImpl>(path, callback);
}

#endif
