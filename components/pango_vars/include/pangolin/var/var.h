/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <stdexcept>
#include <string.h>
#include <cmath>

#include <pangolin/utils/is_streamable.h>
#include <pangolin/var/varvalue.h>
#include <pangolin/var/varwrapper.h>
#include <pangolin/var/varstate.h>

namespace pangolin
{

template<typename T>
inline void InitialiseNewVarMeta(
    std::shared_ptr<VarValue<T>>& v, const std::string& name
) {
    v->Meta().SetName(name);

    if (std::is_integral<T>::value) {
        v->Meta().increment = 1.0;
    } else {
        v->Meta().increment = (v->Meta().range[1] - v->Meta().range[0]) / 100.0;
    }

    VarState::I().NotifyNewVar<T>(name, v);

}

template<typename T>
inline void InitialiseNewVarMetaGeneric(
    std::shared_ptr<VarValue<T>>& v, const std::string& name
) {
    v->Meta().generic = true;
    InitialiseNewVarMeta(v, name);
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
InitialiseFromPreviouslyGenericVar(const std::shared_ptr<VarValueGeneric>& v)
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

template<typename T>
class Var
{
public:
    static T& Attach(
        const std::string& name, T& variable, const VarMeta& meta = VarMeta()
    ) {
        // Find name in VarStore
        std::shared_ptr<VarValueGeneric>& v = VarState::I()[name];
        if(v) {
            VarValueT<T>* tval = dynamic_cast<VarValue<T>*>(v.get());
            if(tval && &tval->Get() == &variable) {
                // Attaching the same variable twice with same name, ignore
            }else{
                // Same name, different variable
                throw std::runtime_error("Different Var with that name already exists.");
            }
        }else{
            // New reference Var owned by var store
            auto nv = std::make_shared<VarValue<T&>>(variable);
            v = nv;
            v->Meta() = meta;
            InitialiseNewVarMeta<T&>(nv, name);
        }
        return variable;
    }

    static T& Attach(
        const std::string& name, T& variable,
        double min, double max, bool logscale = false
    ) {
        return Attach(name, variable, VarMeta(name, min, max, 0.0, META_FLAG_NONE, logscale));
    }

    static T& Attach( const std::string& name, T& variable, int flags )
    {
        return Attach(name, variable, VarMeta(name, 0., 0., 0., flags) );
    }

    static T& Attach( const std::string& name, T& variable, bool toggle )
    {
        return Attach(name, variable, VarMeta(name, 0., 0., 0., toggle ? META_FLAG_TOGGLE : META_FLAG_NONE) );
    }

    ~Var()
    {
    }

    Var( const std::shared_ptr<VarValueGeneric>& v )
    {
        var = InitialiseFromPreviouslyTypedVar<T>(v);
    }

    Var( const std::string& name, const T& value = T(), const VarMeta& meta = VarMeta() )
    {
        // Find name in VarStore
        std::shared_ptr<VarValueGeneric>& v = VarState::I()[name];
        if(v && !v->Meta().generic) {
            var = InitialiseFromPreviouslyTypedVar<T>(v);
        }else{
            std::shared_ptr<VarValue<T>> nv;
            if(v && v->Meta().generic) {
                // Specialise generic variable (which has previously just been a string)
                nv = InitialiseFromPreviouslyGenericVar<T>(v);
            }else{
                // Create brand new variable
                nv = std::make_shared<VarValue<T>>( value );
            }
            // Create / replace in VarState and set meta data
            v = var = nv;
            nv->Meta() = meta;
            InitialiseNewVarMeta(nv, name);
        }
    }

    Var(const std::string& name, const T& value, int flags)
        : Var(name, value, VarMeta(name, 0., 0.,0., flags))
    {
    }

    Var(const std::string& name, const T& value, bool toggle)
        : Var(name, value, VarMeta(name, 0., 0.,0., toggle ? META_FLAG_TOGGLE : META_FLAG_NONE))
    {
    }

    Var(
        const std::string& name, const T& value,
        double min, double max, bool logscale = false
    )
        : Var(name, value, VarMeta(name, min, max, 0., META_FLAG_NONE, logscale))
    {
    }

    void Reset()
    {
        var->Reset();
    }

    const T& Get() const
    {
        try{
            return var->Get();
        }catch(const BadInputException&)
        {
            const_cast<Var<T> *>(this)->Reset();
            return var->Get();
        }
    }

    operator const T& () const
    {
        return Get();
    }

    const T* operator->()
    {
        try{
            return &(var->Get());
        }catch(BadInputException)
        {
            Reset();
            return &(var->Get());
        }
    }

    void operator=(const T& val)
    {
        var->Set(val);
    }

    void operator=(const Var<T>& v)
    {
        var->Set(v.var->Get());
    }

    VarMeta& Meta()
    {
        return var->Meta();
    }

    bool GuiChanged()
    {
        if(var->Meta().gui_changed) {
            var->Meta().gui_changed = false;
            return true;
        }
        return false;
    }

    std::shared_ptr<VarValueT<T>> Ref()
    {
        return var;
    }

    // Holds reference to stored variable object
    // N.B. mutable because it is a cached value and Get() is advertised as const.
    mutable std::shared_ptr<VarValueT<T>> var;
};

// Just forward to the static method
template<typename T, typename... Ts>
inline T& AttachVar(std::string& name, T& var, Ts... ts)
{
    Var<T>::Attach(name, var, ts...);
}

}
