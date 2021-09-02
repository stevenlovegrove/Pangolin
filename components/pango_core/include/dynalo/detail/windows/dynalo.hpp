#pragma once

#include <string>
#include <stdexcept>

#include <Windows.h>
#include <strsafe.h>

namespace dynalo { namespace detail
{
inline
std::string last_error()
{
    // https://msdn.microsoft.com/en-us/library/ms680582%28VS.85%29.aspx
    LPVOID lpMsgBuf;
    DWORD dw = ::GetLastError();

    ::FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    LPVOID lpDisplayBuf = (LPVOID)::LocalAlloc(LMEM_ZEROINIT,
        (::lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR));

    ::StringCchPrintf((LPTSTR)lpDisplayBuf,
        ::LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("Failed with error %d: %s"),
        dw, lpMsgBuf);

    std::string err_str((LPCTSTR)lpDisplayBuf);

    ::LocalFree(lpMsgBuf);
    ::LocalFree(lpDisplayBuf);

    return err_str;
}


namespace native
{

using handle = HMODULE;

inline handle invalid_handle() { return nullptr; }

// Windows LoadLibrary does not offer flexibility to choose.
// These values are ignored.
enum class resolve
{
    now,
    lazy,
};

namespace name
{

inline std::string prefix()    { return std::string();      }
inline std::string suffix()    { return std::string();      }
inline std::string extension() { return std::string("dll"); }

}

}

inline
native::handle open(const std::string& dyn_lib_path, native::resolve /*symbol_resolution*/)
{
    // Windows LoadLibrary behaviour is most similar to RTLD_NOW
    native::handle lib_handle = ::LoadLibrary(dyn_lib_path.c_str());
    if (lib_handle == nullptr)
    {
        throw std::runtime_error(std::string("Failed to open [dyn_lib_path:") + dyn_lib_path + "]: " + last_error());
    }

    return lib_handle;
}

inline
void close(native::handle lib_handle)
{
    const BOOL rc = ::FreeLibrary(lib_handle);
    if (rc == 0)  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms683152(v=vs.85).aspx
    {
        throw std::runtime_error(std::string("Failed to close the dynamic library: ") + last_error());
    }
}

template <typename FunctionSignature>
inline
FunctionSignature* get_function(native::handle lib_handle, const std::string& func_name)
{
    FARPROC func_ptr = ::GetProcAddress(lib_handle, func_name.c_str());
    if (func_ptr == nullptr)
    {
        throw std::runtime_error(std::string("Failed to get [func_name:") + func_name + "]: " + last_error());
    }

    return reinterpret_cast<FunctionSignature*>(func_ptr);
}

}}
