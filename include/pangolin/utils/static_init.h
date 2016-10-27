// Initializer / finalizer sample for MSVC and GCC/Clang.
// 2010-2016 Joe Lowe. Released into the public domain.

#if defined(_MSC_VER)
    #pragma section(".CRT$XCU",read)
    #define PANGOLIN_STATIC_CONSTRUCTOR_(f,p) \
        extern "C" void f(void); \
        __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
        __pragma(comment(linker,"/include:" p #f "_")) \
        extern "C" void f(void)
    #ifdef _WIN64
        #define PANGOLIN_STATIC_CONSTRUCTOR(f) PANGOLIN_STATIC_CONSTRUCTOR_(f,"")
    #else
        #define PANGOLIN_STATIC_CONSTRUCTOR(f) PANGOLIN_STATIC_CONSTRUCTOR_(f,"_")
    #endif
#else
    #define PANGOLIN_STATIC_CONSTRUCTOR(f) \
        extern "C" void f(void) asm(#f) __attribute__((constructor)); \
        extern "C" void f(void)
#endif

// For linking against static libraries, it is important to ensure the symbol is included
// in an appliction to trigger the static construction

// MSVC: https://msdn.microsoft.com/en-us/library/2s3hwbhs.aspx
// GCC / CLANG: https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html#Link-Options "-u option"

// Sample Usage

// extern "C" void example_finalizer(void)
// {
//     printf( "finalize\n");
// }
//
// PANGOLIN_STATIC_CONSTRUCTOR(example_initializer)
// {
//     printf("initialize\n");
//     atexit(example_finalizer);
// }


// Further, the linker must be made aware that it should not optimise out static constructor symbols
// In CMake:

//    macro( ensure_lib_loads_symbol target symbol)
//        if (MSVC)
//          target_link_libraries(${target} PUBLIC "/INCLUDE:${symbol}")
//        else()
//          target_link_libraries(${target} PUBLIC "-u ${symbol}")
//        endif()
//    endmacro()
//
//    ensure_lib_loads_symbol(some_lib, example_initializer)
