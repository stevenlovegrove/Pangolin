#ifdef _WIN32

#include <Windows.h>
#include <pangolin/utils/file_notifier.h>

#include <stdexcept>

class FileChangeNotifierWinImpl : public FileChangeNotifier
{
  public:
  FileChangeNotifierWinImpl(const std::string& path, Callback callback);

  virtual ~FileChangeNotifierWinImpl() override;

  private:
  std::string path_;
  Callback callback_;
  HANDLE hDir_;
  OVERLAPPED overlapped_;
  BYTE buffer_[1024];

  void readDirectoryChanges();
  static void CALLBACK directory_changes_callback(
      DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped);
};

FileChangeNotifierWinImpl::FileChangeNotifierWinImpl(
    const std::string& path, Callback callback) :
    path_(path), callback_(callback)
{
  hDir_ = CreateFileA(
      path.c_str(), FILE_LIST_DIRECTORY,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
      OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
      nullptr);
  if (hDir_ == INVALID_HANDLE_VALUE) {
    throw std::runtime_error("Failed to create directory handle.");
  }
  readDirectoryChanges();
}

FileChangeNotifierWinImpl::~FileChangeNotifierWinImpl() { CloseHandle(hDir_); }

void FileChangeNotifierWinImpl::readDirectoryChanges()
{
  memset(&overlapped_, 0, sizeof(overlapped_));
  DWORD bytesReturned = 0;
  if (!ReadDirectoryChangesW(
          hDir_, buffer_, sizeof(buffer_), TRUE,
          FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
              FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
              FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
          &bytesReturned, &overlapped_, nullptr)) {
    throw std::runtime_error("Failed to read directory changes.");
  }
}

void FileChangeNotifierWinImpl::directory_changes_callback(
    DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped)
{
  FileChangeNotifierWinImpl* notifier =
      reinterpret_cast<FileChangeNotifierWinImpl*>(overlapped->hEvent);
  if (bytesTransferred == 0) return;

  FILE_NOTIFY_INFORMATION* fni =
      reinterpret_cast<FILE_NOTIFY_INFORMATION*>(notifier->buffer_);
  do {
    std::wstring fileName(fni->FileName, fni->FileNameLength / sizeof(wchar_t));
    notifier->callback_(std::string(fileName.begin(), fileName.end()));
    fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
        reinterpret_cast<BYTE*>(fni) + fni->NextEntryOffset);
  } while (fni->NextEntryOffset != 0);

  notifier->readDirectoryChanges();
}

std::shared_ptr<FileChangeNotifier> FileChangeNotifier::create(
    const std::string& path, Callback callback)
{
  return std::make_shared<FileChangeNotifierWinImpl>(path, callback);
}

#endif
