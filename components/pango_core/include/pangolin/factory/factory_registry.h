/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011-2013 Steven Lovegrove
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

#include <vector>
#include <map>
#include <unordered_set>
#include <algorithm>
#include <typeindex>
#include <regex>
#include <exception>

#include <pangolin/factory/factory.h>

namespace pangolin
{

/// Registry class for managing FactoryInterface instances.
/// Use Factory instantiation to decouple implementation and use
/// of interface types and easy configure construction through
/// a URI-like syntax.
class FactoryRegistry
{
public:
    using TypeRegistry = std::vector<std::shared_ptr<FactoryInterface>>;

    /// Singleton instance of FactoryRegistry
    static std::shared_ptr<FactoryRegistry> I();

    ~FactoryRegistry() {}

    /// Register a new TypedFactoryInterface with the registry
    template<typename T>
    bool RegisterFactory( const std::shared_ptr<TypedFactoryInterface<T>>& factory )
    {
        TypeRegistry& registry = type_registries[typeid(T)];
        registry.push_back(factory);
        return true;
    }

    /// Unregister an existing factory.
    /// Do nothing if factory isn't registered.
    template<typename T>
    void UnregisterFactory(TypedFactoryInterface<T>* factory)
    {
        TypeRegistry& registry = type_registries[typeid(T)];

        registry.erase(
           std::remove_if( registry.begin(), registry.end(),
              [&](auto& i){ return i.factory.get() == factory;}),
           registry.end());
    }

    /// Remove all Factories from all types
    void UnregisterAllFactories()
    {
        type_registries.clear();
    }

    /// Attempt to Construct an instance of \tparam T using the highest precedence
    /// \class FactoryInterface which acceps \param uri
    /// \throws FactoryRegistry::Exception
    template<typename T>
    std::unique_ptr<T> Construct(const Uri& uri)
    {
        TypeRegistry& registry = type_registries[typeid(T)];

        TypeRegistry candidates;
        std::copy_if(registry.begin(), registry.end(), std::back_inserter(candidates), [&](auto& factory){
            const auto schemes = factory->Schemes();
            return schemes.find(uri.scheme) != schemes.end();
        });

        if(candidates.size() == 0) {
            throw NoMatchingSchemeException( uri);
        }

        // Order candidates by precedence.
        std::sort(candidates.begin(), candidates.end(), [&](auto& lhs, auto& rhs){
            // We know that all candidates contain scheme
            return lhs->Schemes()[uri.scheme] < rhs->Schemes()[uri.scheme];
        });

        // Try candidates in order
        for(auto& factory : candidates) {
            std::unordered_set<std::string> orphan_params = ParamReader( factory->Params(), uri ).FindUnrecognizedUriParams();
            if( orphan_params.size() == 0 )
            {
                TypedFactoryInterface<T>* factoryT = dynamic_cast<TypedFactoryInterface<T>*>(factory.get());
                if(factoryT) {
                    std::unique_ptr<T> video = factoryT->Open(uri);
                    if(video) return video;
                }
            }else{
                throw ParameterMismatchException(uri, orphan_params);
            }
        }

        throw NoFactorySucceededException(uri);
    }

    const TypeRegistry& GetFactories(std::type_index type) const
    {
        const auto& ifac = type_registries.find(type);
        if(ifac != type_registries.end()) {
            return ifac->second;
        }else{
            throw std::runtime_error("No factories for this type.");
        }
    }

    template<typename T>
    const TypeRegistry& GetFactories() const {
        return GetFactories(typeid(T));
    }

    /// Base class for FactoryRegistry Exceptions
    class Exception : public std::exception {
    public:
        Exception(const Uri& uri) : uri(uri) {
            err = "Unable to open URI";
            err += "\n  full_uri: " + uri.full_uri;
            err += "\n  scheme: " + uri.scheme;
            err += "\n  params:\n";
            for(const auto& p : uri.params) {
                err += "    " + p.first + " = " + p.second + "\n";
            }
        }
        virtual const char* what() const throw() override {
            return err.c_str();
        }
        const Uri uri;
        std::string err;
    };

    class NoMatchingSchemeException : public Exception {
    public:
        NoMatchingSchemeException(const Uri& uri) : Exception(uri) {
            err += " No matching scheme.";
        }
    };

    class NoFactorySucceededException : public Exception {
    public:
        NoFactorySucceededException(const Uri& uri) : Exception(uri) {
            err += " No factory succeeded.";
        }
    };

    class ParameterMismatchException : public Exception {
    public:
        ParameterMismatchException(const Uri& uri, const std::unordered_set<std::string>& unrecognized_params) :
            Exception(uri), unrecognized_params(unrecognized_params) {
            std::stringstream ss;
            for(const auto& p : unrecognized_params ) ss << p << ",";
            err += " Unrecognized options for scheme (" + ss.str() + ")";
        }
        const std::unordered_set<std::string> unrecognized_params;
    };

private:
    std::map<std::type_index, TypeRegistry> type_registries;
};

/// Macro to define factory entry point. Add a corresponding line in your cmake:
///    `PangolinRegisterFactory(MyInterfaceType MyFactoryType)`
/// in order to have the method added to an initialization method for MyInterfaceType
///
/// \tparam x is the factory interface class (such as WindowInterface, VideoInterface, etc)
/// \return true iff registration was successful.
#define PANGOLIN_REGISTER_FACTORY(x) \
    bool Register ## x ## Factory()
//  {
//    return FactoryRegistry::I()->RegisterFactory<x>(std::make_shared<MyFactoryClass>());
//  }

/// Macro to define factory entry point with automatic static initialization.
/// Use this instead of the macro above to automatically register a factory when the compilation
/// unit is loaded, and you can guarentee the symbols will remain.
/// e.g. within a dynamic library which you will explicitly load, or within the same
/// compilation unit as your main().
///
/// \tparam x is the factory interface class (such as WindowInterface, VideoInterface, etc)
/// \return true iff registration was successful.
// Forward declaration; static load variable; definition
#define PANGOLIN_REGISTER_FACTORY_WITH_STATIC_INITIALIZER(x) \
    bool Register ## x ## Factory(); \
    static const bool Load ## x ## Success = Register ## x ## Factory(); \
    bool Register ## x ## Factory()
//  {
//    return FactoryRegistry::I()->RegisterFactory<x>(std::make_shared<MyFactoryClass>());
//  }
}
