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

#ifndef PANGOLIN_PACKETSTREAMTYPEMAP_H
#define PANGOLIN_PACKETSTREAMTYPEMAP_H

#include <pangolin/platform.h>
#include <pangolin/utils/picojson.h>
#include <stdint.h>
#include <vector>
#include <map>

namespace pangolin
{

typedef unsigned int PacketStreamTypeId;

struct PacketStreamType
{
    PacketStreamType()
        : repeats(1), size_bytes(0)
    {
    }

    PacketStreamType(unsigned int size_bytes)
        : repeats(1), size_bytes(size_bytes)
    {
    }

    PacketStreamType(unsigned int repeats, PacketStreamTypeId subtype, unsigned int subtype_size_bytes)
        : repeats(repeats), size_bytes(repeats*subtype_size_bytes)
    {
        fieldtypes.push_back(subtype);
    }

    bool IsFixedSize() const
    {
        return (size_bytes > 0);
    }

    unsigned int repeats;
    unsigned int size_bytes;
    std::vector<std::string> fieldnames;
    std::vector<PacketStreamTypeId> fieldtypes;
};

class PacketStreamTypeMap
{
public:
    static PacketStreamTypeMap& I()
    {
        static PacketStreamTypeMap instance;
        return instance;
    }

    PacketStreamTypeMap()
    {
        AddDefaultTypes();
    }

    PacketStreamTypeId CreateOrGetType(const std::string& ns, const picojson::value& val)
    {
        using namespace picojson;

        if(val.is<std::string>()) {
            // alias - val must exist in map, otherwise throw error
            return GetTypeId(val.get<std::string>());
        }else if(val.is<object>()) {
            // New un-named type
            PacketStreamType newtype;

            // Fill in children
            bool fixed_size = true;
            const object& children = val.get<object>();

            for(object::const_iterator child = children.begin(); child!= children.end(); ++child) {
                const PacketStreamTypeId id = CreateOrGetType(ns, child->second);
                newtype.fieldnames.push_back(child->first);
                newtype.fieldtypes.push_back(id);
                unsigned int child_size_bytes = GetType(id).size_bytes;
                fixed_size &= (child_size_bytes > 0);
                newtype.size_bytes += child_size_bytes;
            }

            if(!fixed_size) {
                newtype.size_bytes = 0;
            }

            return AddType(newtype);
        }else if(val.is<array>()) {
            // New un-named type
            PacketStreamType newtype;

            const array& children = val.get<array>();
            if(children.size() == 1) {
                // unknown repeats / total size
                newtype.repeats = 0;
                newtype.size_bytes = 0;
                newtype.fieldtypes.push_back(CreateOrGetType(ns,children[0]));
            }else if(children.size() == 2 && children[1].is<int64_t>() ) {
                // known array size
                newtype.repeats = children[1].get<int64_t>();
                const PacketStreamTypeId id = CreateOrGetType(ns,children[0]);
                newtype.size_bytes = newtype.repeats * GetType(id).size_bytes;
            }else{
                throw std::runtime_error("Unexpected array arguments.");
            }

            return AddType(newtype);
        }else{
            throw std::runtime_error("Unexpected array arguments.");
        }
    }

    // http://typed-json.org/ style definitions
    void AddTypes(const std::string& ns, const picojson::object& json)
    {
        for(picojson::object::const_iterator v = json.begin(); v!= json.end(); ++v) {
            PacketStreamTypeId types_id = CreateOrGetType(ns, v->second);
            AddAlias(ns + v->first, types_id);
        }
    }

    PacketStreamTypeId AddType(const std::string& name, const PacketStreamType& type)
    {
        const PacketStreamTypeId id = AddType(type);
        str_map[name] = id;
        return id;
    }

    PacketStreamTypeId AddType(const PacketStreamType& type)
    {
        const PacketStreamTypeId id = types.size();
        types.push_back(type);
        return id;
    }

    PacketStreamTypeId AddAlias(const std::string& newname, PacketStreamTypeId type)
    {
        str_map[newname] = type;
        return type;
    }

    PacketStreamTypeId AddAlias(const std::string& newname, const std::string& type)
    {
        PacketStreamTypeId id = GetTypeId(type);
        AddAlias(newname, id );
        return id;
    }

    const PacketStreamType& GetType(PacketStreamTypeId id)
    {
        return types[id];
    }

    const PacketStreamType& GetType(const std::string& name)
    {

        return types[GetTypeId(name)];
    }

    PacketStreamTypeId GetTypeId(const std::string& name) const
    {
        std::map<std::string,PacketStreamTypeId>::const_iterator i = str_map.find(name);
        if(i != str_map.end()) {
            return i->second;
        }else{
            throw std::runtime_error("Unknown type: '" + name + "'");
        }
    }

protected:
    void AddDefaultTypes()
    {
        // Add primitive types
        AddType("int8",PacketStreamType(1));
        AddType("int16",PacketStreamType(2));
        AddType("int32",PacketStreamType(4));
        AddType("uint8",PacketStreamType(1));
        AddType("uint16",PacketStreamType(2));
        AddType("uint32",PacketStreamType(4));
        AddType("float32",PacketStreamType(32));
        AddType("float64",PacketStreamType(64));
        AddType("string", PacketStreamType(0,GetTypeId("int8"),1));

        // Encode bools as int8's
        AddAlias("bool","int8");

        // Add common name aliases
        AddAlias("char","int8");
        AddAlias("int","int32");
        AddAlias("uint","uint32");
        AddAlias("float","float32");
        AddAlias("double","float64");
    }

    std::vector<PacketStreamType> types;
    std::map<std::string,PacketStreamTypeId> str_map;
};

}

#endif // PANGOLIN_PACKETSTREAMTYPEMAP_H
