
// This header can be included multiple times
// It should come in handy for switching betwee, exporting and importing symbols
//
// USAGE
//
//  DYNALO_EXPORT <return type> DYNALO_CALL <function name>(<function argument types...>)
//
// EXAMPLE
//
//  EXPORTING FUNCTIONS
//
//      #define DYNALO_EXPORT_SYMBOLS
//      #include <dynalo/symbol_helper.hpp>
//
//      DYNALO_EXPORT int32_t DYNALO_CALL add_integers(const int32_t a, const int32_t b);
//      DYNALO_EXPORT void DYNALO_CALL print_message(const char* message);
//
//  IMPORTING FUNCTIONS
//
//      #define DYNALO_IMPORT_SYMBOLS
//      #include <dynalo/symbol_helper.hpp>
//
//      DYNALO_EXPORT int32_t DYNALO_CALL add_integers(const int32_t a, const int32_t b);
//      DYNALO_EXPORT void DYNALO_CALL print_message(const char* message);
//

#if !defined(DYNALO_DEMANGLE)
    #if defined(__cplusplus)
        #define DYNALO_DEMANGLE extern "C"
    #else
        #define DYNALO_DEMANGLE
    #endif
#endif

#if defined(DYNALO_EXPORT_SYMBOLS)
    #undef DYNALO_EXPORT_SYMBOLS

    #if defined(DYNALO_EXPORT)
        #undef DYNALO_EXPORT
    #endif

    #ifdef WIN32
        #define DYNALO_EXPORT DYNALO_DEMANGLE __declspec(dllexport)
    #else
        #define DYNALO_EXPORT DYNALO_DEMANGLE
    #endif

#elif defined(DYNALO_IMPORT_SYMBOLS)
    #undef DYNALO_IMPORT_SYMBOLS

    #if defined(DYNALO_EXPORT)
        #undef DYNALO_EXPORT
    #endif

    #ifdef WIN32
        #define DYNALO_EXPORT DYNALO_DEMANGLE __declspec(dllimport)
    #else
        #define DYNALO_EXPORT DYNALO_DEMANGLE extern
    #endif

#else
    #error "dynalo/symbol_helper.hpp Define either DYNALO_EXPORT_SYMBOLS or DYNALO_IMPORT_SYMBOLS"
#endif

#if !defined(DYNALO_CALL)
    #ifdef _MSC_VER
        #define DYNALO_CALL __cdecl
    #else
        #define DYNALO_CALL
    #endif
#endif