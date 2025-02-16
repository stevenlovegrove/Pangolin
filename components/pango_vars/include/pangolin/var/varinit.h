#include <pangolin/var/varvalue.h>
#include <cstring>

namespace pangolin {

template<typename T>
inline double DefaultIncrementForType(double min, double max) {
    return (std::is_integral<T>::value) ? 1.0 : (max - min) / 100.0;
}

template <typename T, typename S>
typename std::enable_if<is_streamable<S>::value && is_streamable<T>::value, std::shared_ptr<VarValueT<T>>>::type
Wrapped(const std::shared_ptr<VarValueT<S>>& src) noexcept
{
    return std::make_shared<VarWrapper<T,S>>(src);
}

template <typename T, typename S>
typename std::enable_if<!(is_streamable<S>::value && is_streamable<T>::value), std::shared_ptr<VarValueT<T>>>::type
Wrapped(const std::shared_ptr<VarValueT<S>>&)
{
    throw std::runtime_error("Unable to wrap Var");
}

template<typename T>
typename std::enable_if<!is_streamable<T>::value, std::shared_ptr<VarValue<T>>>::type
InitialiseFromPreviouslyGenericVar(const std::shared_ptr<VarValueGeneric>& /*v*/)
{
    // We can't initialize this variable from a 'generic' string type.
    throw BadInputException();
}
template<typename T>
typename std::enable_if<is_streamable<T>::value, std::shared_ptr<VarValue<T>>>::type
InitialiseFromPreviouslyGenericVar(const std::shared_ptr<VarValueGeneric>& v)
{
    return std::make_shared<VarValue<T>>( Convert<T,std::string>::Do( v->str->Get() ) );
}

// Initialise from existing variable, obtain data / accessor
template<typename T>
std::shared_ptr<VarValueT<T>> InitialiseFromPreviouslyTypedVar(const std::shared_ptr<VarValueGeneric>& v)
{
    // Macro hack to prevent code duplication
#   define PANGO_VAR_TYPES(x)                            \
    x(bool) x(int8_t) x(uint8_t) x(int16_t) x(uint16_t)  \
    x(int32_t) x(uint32_t) x(int64_t) x(uint64_t)        \
    x(float) x(double)

    if( !strcmp(v->TypeId(), typeid(T).name()) ) {
        // Same type
        return std::dynamic_pointer_cast<VarValueT<T>>(v);
    }else if( std::is_same<T,std::string>::value ) {
        // Use types string accessor
        return std::dynamic_pointer_cast<VarValueT<T>>(v->str);
    }else
#       define PANGO_CHECK_WRAP(x)                                                       \
    if( !strcmp(v->TypeId(), typeid(x).name() ) ) {                                      \
        std::shared_ptr<VarValueT<x>> xval = std::dynamic_pointer_cast<VarValueT<x>>(v); \
        return Wrapped<T,x>(xval);                                                       \
    }else
    PANGO_VAR_TYPES(PANGO_CHECK_WRAP)
    {
        // other types: have to go via string
        // Wrapper, owned by this object
        return Wrapped<T,std::string>(v->str);
    }
#undef PANGO_VAR_TYPES
#undef PANGO_CHECK_WRAP
}

}
