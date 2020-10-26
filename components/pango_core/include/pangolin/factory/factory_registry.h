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

#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <regex>
#include <exception>

#include <pangolin/utils/uri.h>

namespace pangolin
{

struct FactoryParamHelpData {
    std::string name;
    std::string default_value;
    std::string description;
};

class FactoryHelpData {
public:
    FactoryHelpData( const std::string& scheme, const std::string& description = "", const ParamSet& param_set = ParamSet()) :
        scheme_(scheme), description_(description), param_set_(param_set) {}

    std::string GetScheme(){return scheme_;}
    std::string GetSynopsis();
    std::string GetDescription();
    std::vector<FactoryParamHelpData> GetParamsHelp();
private:
    std::string scheme_;
    std::string description_;
    ParamSet    param_set_;
};

class FactoryInterfaceBase {
public:
    virtual ~FactoryInterfaceBase(){};

    // The following methods are taking scheme as arguments because
    // a single instance of Factory can serve multiple schemes
    virtual FactoryHelpData Help( const std::string& scheme ) const = 0;

    // Returning empty set means this factory validates this URI.
    virtual bool ValidateUri( const std::string& scheme, const Uri& uri, std::unordered_set<std::string>& unrecognized_params ) const = 0;

    // Return true if this scheme is at all validated by this factory
    virtual bool IsValidated( const std::string& scheme ) const = 0;

    //Helper function validate uri against a paramset
    static bool ValidateUriAgainstParamSet( const std::string& /*scheme*/, const ParamSet& param_set, const Uri& uri, std::unordered_set<std::string>& unrecognized_params )
    {
        ParamReader param_reader( param_set, uri );
        unrecognized_params = param_reader.FindUnrecognizedUriParams();
        return unrecognized_params.size() == 0;
    }
};

template<typename T>
struct FactoryInterface : FactoryInterfaceBase
{
    typedef T FactoryItem;
    virtual ~FactoryInterface() = default;
    virtual std::unique_ptr<T> Open(const Uri& uri) = 0;

    virtual FactoryHelpData Help( const std::string& scheme ) const override {return FactoryHelpData(scheme);}
    virtual bool ValidateUri( const std::string&, const Uri&, std::unordered_set<std::string>&) const override {return true;};
    virtual bool IsValidated( const std::string&) const override { return false;}
};

struct FactoryMetaData {
    uint32_t precedence;
    std::string scheme;
    std::shared_ptr<FactoryInterfaceBase> factory;


    FactoryMetaData( uint32_t pre,
                     const std::string& scheme_name,
                     std::shared_ptr<FactoryInterfaceBase> factoryInterface )
        : precedence( pre ),
          scheme(scheme_name),
          factory(factoryInterface)
    {}

    FactoryHelpData Help() const
    {
        return factory->Help( scheme );
    }

    bool IsValidated() const
    {
        return factory->IsValidated( scheme );
    }


};

class FactoryRegistryBase {
public:
    FactoryRegistryBase( const std::string& type );
    virtual ~FactoryRegistryBase() {};
    const std::string& GetType() const { return type;}
    static const std::vector< const FactoryRegistryBase* >& GetFactoryRegistryList() {return registry_list;};

    virtual std::vector<FactoryMetaData> GetSchemeFactories() const = 0;
private:
    std::string type;
    static std::vector<const FactoryRegistryBase*> registry_list;
};

class FactoryRegistryException : public std::exception {
public:
    FactoryRegistryException( bool scheme_matched,
                              bool all_params_matched,
                              const Uri& uri,
                              const std::unordered_set<std::string>& unrecognized_params ){
        std::stringstream ss;

        if( !scheme_matched ){
            error_msg_ = "No matching scheme for \"" + uri.scheme + "\"";
        }
        else if( all_params_matched ){
            error_msg_=  "URI " + uri.full_uri + " couldn't be opened even though all the parameters were recognized. "
                                                 "Perhaps their values are invalid? Is the resource accessible? "
                                                 "Perhaps there is an instance of the scheme \"" + uri.scheme + "\" that is not validated?";
        }
        else{
            for(const auto& param : unrecognized_params ){
                ss << param << ",";
            }
            error_msg_ = "No instance of the scheme \"" + uri.scheme + "\" recognized all the parameters. Infact, the parameter(s) [" + ss.str() + "] were not recognized by any instance. Double check spelling!";
        }
    }
    virtual const char* what() const throw() override {
        return error_msg_.c_str();
    }
private:
    std::string error_msg_;
};

template<typename T>
class FactoryRegistry : public FactoryRegistryBase
{
public:
    FactoryRegistry( const std::string &type_name = "unknown ")
        : FactoryRegistryBase( type_name ) {}
    // IMPORTANT: Implement for each templated instantiation within a seperate compilation unit.
    static FactoryRegistry<T>& I();

    ~FactoryRegistry()
    {
    }

    virtual std::vector<FactoryMetaData> GetSchemeFactories() const override
    {
        std::vector<FactoryMetaData> result;
        for(const auto& f: factories){
            result.emplace_back ( f.precedence, f.scheme, f.factory );
        }
        return result;
    }

    void RegisterFactory(std::shared_ptr<FactoryInterface<T>> factory, uint32_t precedence, const std::string& scheme_name )
    {
        FactoryItem item = {precedence, scheme_name, factory};
        factories.push_back( item );
        std::sort(factories.begin(), factories.end());
    }

    void UnregisterFactory(FactoryInterface<T>* factory)
    {
        for( auto i = factories.end()-1; i != factories.begin(); --i)
        {
            if( i->factory.get() == factory ) {
                factories.erase(i);
            }
        }
    }

    void UnregisterAllFactories()
    {
        factories.clear();
    }

    std::unordered_set<std::string> IntersectParamSet(const std::unordered_set<std::string>& a, const std::unordered_set<std::string>& b )
    {
        std::unordered_set<std::string> c;
        std::set_intersection( a.begin(), a.end(), b.begin(), b.end(), std::inserter( c, c.begin() ) );
        return c;
    }

    std::unique_ptr<T> Open(const Uri& uri)
    {
        // Iterate over all registered factories in order of precedence.
        std::unordered_set<std::string> orphan_params;
        bool all_params_matched = false;
        bool scheme_matched = false;

        for(auto& item : factories) {
            if( item.scheme == uri.scheme) {
                scheme_matched = true;

                std::unordered_set<std::string> unrecognized_params;
                bool validated = item.factory->ValidateUri(item.scheme,uri,unrecognized_params);
                orphan_params = orphan_params.size() == 0 ? unrecognized_params : IntersectParamSet( orphan_params, unrecognized_params );

                if( validated ){
                    all_params_matched = true;
                    std::unique_ptr<T> video = item.factory->Open(uri);
                    if(video) {
                        return video;
                    }
                }

            }
        }

        throw FactoryRegistryException( scheme_matched, all_params_matched, uri, orphan_params  );
    }

private:
    struct FactoryItem
    {
        uint32_t precedence;
        std::string scheme;
        std::shared_ptr<FactoryInterface<T>> factory;

        bool operator<(const FactoryItem& rhs) const {
            return precedence < rhs.precedence;
        }
    };

    // Priority, Factory tuple
    std::vector<FactoryItem> factories;
};

#define PANGOLIN_REGISTER_FACTORY(x) void Register ## x ## Factory()

}
