#include <iostream>
#include <fstream>
#include <pangolin/var/varstate.h>
#include <pangolin/utils/picojson.h>
#include <pangolin/utils/transform.h>

namespace pangolin
{

namespace  {
template< typename ContainerT, typename PredicateT >
void erase_if( ContainerT& items, const PredicateT& predicate ) {
    for( auto it = items.begin(); it != items.end(); ) {
        if( predicate(*it) ) it = items.erase(it);
        else ++it;
    }
}
}

VarState& VarState::I() {
    static VarState singleton;
    return singleton;
}

VarState::VarState()
{
}

VarState::~VarState() {
    Clear();
}

void VarState::Clear() {
    vars.clear();
    vars_reverse.clear();
    vars_add_order.clear();
}

bool VarState::Remove(const std::string& name)
{
    VarStoreMap::iterator it = vars.find(name);
    if(it != vars.end()) {
        auto to_remove = it->second;
        erase_if(vars_reverse, [&to_remove](auto el){return to_remove == el.second.lock();} );
        erase_if(vars_add_order, [&to_remove](auto el){return to_remove == el.lock();} );
        vars.erase(it);
        VarEventSignal(Event{Event::Action::Removed, to_remove});
        return true;
    }
    return false;
}

bool VarState::Exists(const std::string& key) const
{
    return vars.find(key) != vars.end();
}

// Return value manages connection lifetime through RAII
[[nodiscard]]
sigslot::connection VarState::RegisterForVarEvents( Event::Function callback_function, bool include_historic)
{
    if(include_historic) {
        for (auto it = vars_add_order.begin(); it != vars_add_order.end(); ++it)
        {
            if (auto p_var = it->lock()) {
                callback_function({Event::Action::Added, p_var});
            }
        }
    }
    return VarEventSignal.connect(callback_function);
}

//void AddAlias(const string& alias, const string& name)
//{
//    std::map<std::string,_Var*>::iterator vi = vars.find(name);

//    if( vi != vars.end() )
//    {
//        _Var * v = vi->second;
//        vars[alias].create(v->val,v->val_default,v->type_name);
//        vars[alias].Meta().friendly = alias;
//        v->generic = false;
//        vars[alias].generic = false;
//    }else{
//        pango_print_error("Variable %s does not exist to alias.\n", name);
//    }
//}

void VarState::AddOrSetGeneric(const std::string& name, const std::string& value)
{
    const auto it = vars.find(name);

    if (it != vars.end()) {
        it->second->str->Set(value);
    }else{
        AddVar(std::make_shared<VarValue<std::string>>(
            value, VarMeta(name, 0.0, 0.0, 0.0, 0, false, true)), false);
    }
}

void VarState::LoadFromJsonStream(std::istream& is)
{
    picojson::value file_json(picojson::object_type,true);
    const std::string err = picojson::parse(file_json,is);
    if(err.empty()) {
        if(file_json.contains("vars") ) {
            picojson::value var_json = file_json["vars"];
            if(var_json.is<picojson::object>()) {
                for(picojson::object::iterator
                    i = var_json.get<picojson::object>().begin();
                    i!= var_json.get<picojson::object>().end();
                    ++i)
                {
                    const std::string& name = i->first;
                    const std::string& val = i->second.get<std::string>();
                    AddOrSetGeneric(name, val);
                }
            }
        }
    }else{
        pango_print_error("%s\n", err.c_str());
    }
}

// Recursively expand val
std::string VarState::ProcessVal(const std::string& val )
{
    return Transform(val, [this](const std::string& k) -> std::string {
        auto var = GetByName(k);
        if( var && var->str ) {
            return var->str->Get();
        }else{
            return std::string("#");
        }
    });
}

void VarState::LoadFromConfigStream(std::istream& is)
{
    while( !is.bad() && !is.eof()) {
        const int c = is.peek();

        if( isspace(c) ) {
            // ignore leading whitespace
            is.get();
        }else{
            if( c == '#' || c == '%' ) {
                // ignore lines starting # or %
                std::string comment;
                getline(is,comment);
            }else{
                // Otherwise, find name and value, seperated by '=' and ';'
                std::string name;
                std::string val;
                getline(is,name,'=');
                getline(is,val,';');
                name = Trim(name, " \t\n\r");
                val = Trim(val, " \t\n\r");

                if( name.size() >0 && val.size() > 0 ) {
                    if( !val.substr(0,1).compare("@") ) {
//                            AddAlias(name,val.substr(1));
                    }else{
                        AddOrSetGeneric(name, val);
                    }
                }
            }
        }
    }
}

void VarState::SaveToJsonStream(std::ostream &os)
{
    picojson::value json_vars(picojson::object_type,true);

    for(const auto& key_value : vars)
    {
        const std::string& name = key_value.first;
        try{
            json_vars[name] = key_value.second->str->Get();
        }catch(const BadInputException&)
        {
            // Ignore things we can't serialise
        }
    }

    picojson::value file_json(picojson::object_type,true);
    file_json["vars"] = json_vars;
    os << file_json.serialize(true);
}

void VarState::LoadFromFile(const std::string& filename, FileKind kind)
{
    std::ifstream f(filename.c_str());
    if( f.is_open() ) {
        switch(kind)
        {
        case FileKind::detected:
        {
            const auto fl = ToLowerCopy(filename);
            if(EndsWith(fl, ".json") || EndsWith(fl, ".jsn")) {
                LoadFromJsonStream(f);
            }else{
                LoadFromConfigStream(f);
            }
            break;
        }
        case FileKind::config:
            LoadFromConfigStream(f);
            break;
        case FileKind::json:
            LoadFromJsonStream(f);
            break;
        }
    }else{
        pango_print_error("Unable to open file %s\n", filename.c_str());
    }
}

void VarState::SaveToFile(const std::string& filename, FileKind kind)
{
    std::ofstream f(filename);
    if(f.is_open()) {
        if(kind == FileKind::json) {
            SaveToJsonStream(f);
        }else{
            throw std::runtime_error("Only support saving to JSON file right now.");
        }

    }else{
        pango_print_error("Unable to serialise to %s\n", filename.c_str());
    }
}

}
