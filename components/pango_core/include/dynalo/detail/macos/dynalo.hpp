#pragma once

#include <string>
#include <stdexcept>

#include <dlfcn.h>

namespace dynalo { namespace detail
{

inline
std::string last_error()
{
	return std::string(::dlerror());
}

namespace native
{

using handle = void*;

inline handle invalid_handle() { return nullptr; }

enum class resolve
{
    now = RTLD_NOW,
    lazy = RTLD_LAZY,
};

namespace name
{

inline std::string prefix()    { return std::string("lib"); }
inline std::string suffix()    { return std::string();      }
inline std::string extension() { return std::string("dylib");  }

}

}

inline
native::handle open(const std::string& dyn_lib_path, native::resolve symbol_resolution)
{
    native::handle lib_handle = ::dlopen(dyn_lib_path.c_str(), static_cast<int>(symbol_resolution) );
	if (lib_handle == nullptr)
	{
		throw std::runtime_error(std::string("Failed to open [dyn_lib_path:") + dyn_lib_path + "]: " + last_error());
	}

	return lib_handle;
}

inline
void close(native::handle lib_handle)
{
	const int rc = ::dlclose(lib_handle);
	if (rc != 0)
	{
		throw std::runtime_error(std::string("Failed to close the dynamic library: ") + last_error());
	}
}

template <typename FunctionSignature>
inline
FunctionSignature* get_function(native::handle lib_handle, const std::string& func_name)
{
	void* func_ptr = ::dlsym(lib_handle, func_name.c_str());
	if (func_ptr == nullptr)
	{
		throw std::runtime_error(std::string("Failed to get [func_name:") + func_name + "]: " + last_error());
	}

	return reinterpret_cast<FunctionSignature*>(func_ptr);
}

}}
