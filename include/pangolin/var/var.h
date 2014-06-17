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

#ifndef PANGOLIN_VAR_H
#define PANGOLIN_VAR_H

#include <stdexcept>
#include <string.h>

#include <pangolin/var/varvalue.h>
#include <pangolin/var/varwrapper.h>
#include <pangolin/var/varstate.h>

namespace pangolin
{

template<typename T>
class Var
{
public:
    static void InitialiseNewVarMeta(
        VarValueGeneric& v, const std::string& name,
        double min = 0, double max = 0, int flags = 1, bool logscale = false
    ) {
        // Initialise meta parameters
        const std::vector<std::string> parts = pangolin::Split(name,'.');
        v.Meta().full_name = name;
        v.Meta().friendly = parts.size() > 0 ? parts[parts.size()-1] : "";
        v.Meta().range[0] = min;
        v.Meta().range[1] = max;
        v.Meta().flags = flags;
        v.Meta().logscale = logscale;

        VarState::I().NotifyNewVar(name, v);
    }

    static void Attach(
        const std::string& name, T& variable,
        double min = 0, double max = 0, int flags = 1,
        bool logscale = false
    ) {
        // Find name in VarStore
        VarValueGeneric*& v = VarState::I()[name];
        if(v) {
            throw std::runtime_error("Var with that name already exists.");
        }else{
            // new VarRef<T> (owned by VarStore)
            v = new VarValue<T&>(variable);
            InitialiseNewVarMeta(*v,name,min,max,flags,logscale);
        }
    }

    ~Var()
    {
        delete ptr;
    }

    Var( VarValueGeneric& v )
        : ptr(0)
    {
        InitialiseFromGeneric(&v);
    }


    Var( const std::string& name )
        : ptr(0)
    {
        // Find name in VarStore
        VarValueGeneric*& v = VarState::I()[name];
        if(v) {
            InitialiseFromGeneric(v);
        }else{
            // new VarValue<T> (owned by VarStore)
            var = new VarValue<T>(T());
            v = var;
            InitialiseNewVarMeta(*var,name,0,0,0,false);
        }
    }

    Var( const std::string& name, const T& value, bool toggle = false )
        : ptr(0)
    {
        // Find name in VarStore
        VarValueGeneric*& v = VarState::I()[name];
        if(v) {
            InitialiseFromGeneric(v);
        }else{
            // new VarValue<T> (owned by VarStore)
            var = new VarValue<T>(value);
            v = var;
            InitialiseNewVarMeta(*var,name,0,1,toggle,false);
        }
    }

    Var(
        const std::string& name, const T& value,
        double min, double max, bool logscale = false
    ) : ptr(0)
    {
        // Find name in VarStore
        VarValueGeneric*& v = VarState::I()[name];
        if(v) {
            InitialiseFromGeneric(v);
        }else{
            // new VarValue<T> (owned by VarStore)
            var = new VarValue<T>(value);
            v = var;
            if(logscale) {
                if (min <= 0 || max <= 0) {
                    throw std::runtime_error("LogScale: range of numbers must be positive!");
                }
                InitialiseNewVarMeta(*var,name,std::log(min),std::log(max),1,logscale);
            }else{
                InitialiseNewVarMeta(*var,name,min,max);
            }
        }
    }

    // Initialise from existing variable, obtain data / accessor
    void InitialiseFromGeneric(VarValueGeneric* v)
    {
        if( !strcmp(v->TypeId(), typeid(T).name()) ) {
            // Same type
            var = (VarValueT<T>*)(v);
        }else if( boostd::is_same<T,std::string>::value ) {
            // Use types string accessor
            var = (VarValueT<T>*)(v->str);
        }else if( !strcmp(v->TypeId(), typeid(bool).name() ) ) {
            // Wrapper, owned by this object
            ptr = new VarWrapper<T,bool>( *(VarValueT<bool>*)v );
            var = ptr;
        }else if( !strcmp(v->TypeId(), typeid(short).name() ) ) {
            // Wrapper, owned by this object
            ptr = new VarWrapper<T,short>( *(VarValueT<short>*)v );
            var = ptr;
        }else if( !strcmp(v->TypeId(), typeid(int).name() ) ) {
            // Wrapper, owned by this object
            ptr = new VarWrapper<T,int>( *(VarValueT<int>*)v );
            var = ptr;
        }else if( !strcmp(v->TypeId(), typeid(long).name() ) ) {
            // Wrapper, owned by this object
            ptr = new VarWrapper<T,long>( *(VarValueT<long>*)v );
            var = ptr;
        }else if( !strcmp(v->TypeId(), typeid(float).name() ) ) {
            // Wrapper, owned by this object
            ptr = new VarWrapper<T,float>( *(VarValueT<float>*)v );
            var = ptr;
        }else if( !strcmp(v->TypeId(), typeid(double).name() ) ) {
            // Wrapper, owned by this object
            ptr = new VarWrapper<T,double>( *(VarValueT<double>*)v );
            var = ptr;
        }else{
            // other types: have to go via string
            // Wrapper, owned by this object
            ptr = new VarWrapper<T,std::string>( *(v->str) );
            var = ptr;
        }
    }

    void Reset()
    {
        var->Reset();
    }

    const T& Get()
    {
        try{
            return var->Get();
        }catch(BadInputException)
        {
            Reset();
            return var->Get();
        }
    }

    operator const T& ()
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

//protected:
    // Holds reference to stored variable object
    VarValueT<T>* var;

    // ptr is non-zero if this object owns the object variable (a wrapper)
    VarValueT<T>* ptr;
};

}

#endif // PANGOLIN_VAR_H
