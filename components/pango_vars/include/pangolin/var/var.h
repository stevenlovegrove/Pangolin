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
class Var
{
public:
    static T& Attach( T& variable, const VarMeta& meta ) {
        VarState::I().AttachVar<T&>(variable,meta);
        return variable;
    }

    static T& Attach( const std::string& name, T& variable )
    {
        return Attach(variable, VarMeta(name));
    }

    static T& Attach(
        const std::string& name, T& variable,
        double min, double max, bool logscale = false
    ) {
        return Attach(variable, VarMeta(name, min, max, DefaultIncrementForType<T>(min,max), META_FLAG_NONE, logscale));
    }

    static T& Attach( const std::string& name, T& variable, int flags )
    {
        return Attach(variable, VarMeta(name, 0., 0., 0., flags) );
    }

    static T& Attach( const std::string& name, T& variable, bool toggle )
    {
        return Attach(variable, VarMeta(name, 0., 0., 0., toggle ? META_FLAG_TOGGLE : META_FLAG_NONE) );
    }

    ~Var()
    {
    }

    Var( const std::shared_ptr<VarValueGeneric>& v )
        : var(InitialiseFromPreviouslyTypedVar<T>(v))
    {
    }

    Var( const T& value, const VarMeta& meta )
        : var(InitialiseFromPreviouslyTypedVar<T>(VarState::I().GetOrCreateVar<T>(value, meta)))
    {
    }

    Var( const std::string& name, const T& value = T() )
        : Var(value, VarMeta(name))
    {
    }

    Var(const std::string& name, const T& value, int flags)
        : Var(value, VarMeta(name, 0., 0.,0., flags))
    {
    }

    Var(const std::string& name, const T& value, bool toggle)
        : Var(value, VarMeta(name, 0., 0.,0., toggle ? META_FLAG_TOGGLE : META_FLAG_NONE))
    {
    }

    Var(const std::string& name, const T& value,
        double min, double max, bool logscale = false)
        : Var(value, VarMeta(name, min, max, DefaultIncrementForType<T>(min,max), META_FLAG_NONE, logscale))
    {
    }

    void Reset()
    {
        var->Reset();
    }

    void Detach()
    {
        VarState::I().Remove(Meta().full_name);
    }

    const T& Get() const
    {
        return var->Get();
    }

    operator const T& () const
    {
        return Get();
    }

    const T* operator->()
    {
        return &(var->Get());
    }

    Var<T>& operator=(const T& val)
    {
        var->Set(val);
        return *this;
    }

    Var<T>& operator=(const Var<T>& v)
    {
        var->Set(v.var->Get());
        return *this;
    }

    VarMeta& Meta()
    {
        return var->Meta();
    }

    bool GuiChanged()
    {
        return Meta().gui_changed && !(Meta().gui_changed = false);
    }

    std::shared_ptr<VarValueT<T>> Ref()
    {
        return var;
    }

    // Holds reference to stored variable object
    // N.B. mutable because it is a cached value and Get() is advertised as const.
    mutable std::shared_ptr<VarValueT<T>> var;
};

template<typename T>
inline std::ostream& operator<<(std::ostream& s, Var<T>& rhs)
{
    s << rhs.operator const T &();
    return s;
}

}
