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

#include <pangolin/var/varvaluet.h>
#include <pangolin/var/varwrapper.h>
#include <pangolin/utils/is_streamable.h>

namespace pangolin
{

template<typename T>
class VarValue : public VarValueT<typename std::remove_reference<T>::type>
{
public:
    typedef typename std::remove_reference<T>::type VarT;


    VarValue(const T& value, const VarT& default_value, const VarMeta& meta = VarMeta())
        : value(value), default_value(default_value), meta(meta)
    {
        Init();
    }

    VarValue(const T& value, const VarMeta& meta = VarMeta())
        : VarValue(value, value, meta)
    {
    }

    VarValue()
        : VarValue(T(), T())
    {
    }

    const char* TypeId() const
    {
        return typeid(VarT).name();
    }

    void Reset()
    {
        value = default_value;
    }

    VarMeta& Meta()
    {
        return meta;
    }

    const VarT& Get() const
    {
        return value;
    }

    VarT& Get()
    {
        return value;
    }

    void Set(const VarT& val)
    {
        value = val;
    }

protected:
    // Specialization dummy for non-serializable types
    struct ExceptionVarValue : public VarValueT<std::string>
    {
        typedef typename std::string VarT;
        ExceptionVarValue() {}
        const char* TypeId() const override { throw BadInputException(); }
        virtual void Reset() override { throw BadInputException(); }
        VarMeta& Meta() override  { throw BadInputException(); }
        const VarT& Get() const override { throw BadInputException(); }
        void Set(const VarT& val) override { throw BadInputException(); }
    };

    template<typename TT> static
    typename std::enable_if<is_streamable<TT>::value, std::shared_ptr<VarValueT<std::string>>>::type
    MakeStringWrapper( const std::shared_ptr<VarValueT<TT>>& v )
    {
        return std::make_shared<VarWrapper<std::string,VarT>>(v);
    }

    template<typename TT> static
    typename std::enable_if<!is_streamable<TT>::value, std::shared_ptr<VarValueT<std::string>>>::type
    MakeStringWrapper( const std::shared_ptr<VarValueT<TT>>& v )
    {
        return std::make_shared<ExceptionVarValue>();
    }

    void Init()
    {
        // shared_ptr reference to self without deleter.
        auto self = std::shared_ptr<VarValueT<VarT>>(this, [](VarValueT<VarT>*){} );

        if(std::is_same<VarT,std::string>::value) {
            // str is reference to this - remove shared_ptr's deleter
            this->str = std::dynamic_pointer_cast<VarValueT<std::string>>(self);
        }else{
            this->str = MakeStringWrapper<VarT>(self);
        }
    }

    T value;
    VarT default_value;
    VarMeta meta;
};

}
