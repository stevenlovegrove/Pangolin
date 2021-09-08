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

#include <map>
#include <vector>
#include <memory>
#include <any>
#include <cassert>
#include <pangolin/platform.h>
#include <pangolin/var/varvalue.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/utils/signal_slot.h>
#include <pangolin/var/varinit.h>

namespace pangolin
{

class PANGOLIN_EXPORT VarState
{
public:
    static VarState& I();

    VarState();
    ~VarState();

    /// \returns true iff a pangolin var with key name \param key exists in the index.
    bool Exists(const std::string& key) const;

    /// Register a user defined variable in memory into the VarState index.
    /// Note that the user memory must have a lifetime which exceeds all accesses through
    /// pangolin::Vars. The parameter remains owned by the user who must take care of
    /// appropriate deallocation.
    ///
    /// \tparam T The type of \param variable to register
    /// \param variable The variable to make available in the VarState index.
    /// \param meta Meta information about the variable that will allow tools to introspect
    ///             and modify it. In particular, \param meta.name is the user accessible name
    ///             and is also used as input to lookup any Var in the index.
    template<typename T>
    std::shared_ptr<VarValueGeneric> AttachVar( T& variable, const VarMeta& meta );

    /// Get a reference to an existing Var in the index, or create one if it doesn't exist.
    /// The variable name to be used is inside of \param meta.name. Lifetime of Created
    /// variables is managed through shared_ptr reference counting.
    ///
    /// \tparam T The underlying type of the Var to create
    /// \param value The initial value for the Var if it doesn't already exist and gets created
    /// \param meta Meta information about the variable that will allow tools to introspect
    ///             and modify it. In particular, \param meta.name is the user accessible name
    ///             and is also used as input to lookup any Var in the index.
    template<typename T>
    std::shared_ptr<VarValueGeneric> GetOrCreateVar( const T& value, const VarMeta& meta );

    /// \return A reference to the corresponding var with name \param full_name, or nullptr
    ///         if no such Var exists
    std::shared_ptr<VarValueGeneric> GetByName(const std::string& full_name);

    /// \return A reference to the corresponding var with underlying memory \param value, or nullptr
    ///         if no such Var exists
    template<typename T>
    std::shared_ptr<VarValueGeneric> GetByReference(const T& value);

    /// Callback Event structure for notification of Var Additions and Deletions
    struct Event {
        /// Function signature for user callbacks
        using Function = std::function<void(const Event&)>;

        /// Type of Var Event
        enum class Action {
            Added,    // A Var was added to the VarState index
            Removed   // A Var was removed from the VarState index
        };

        /// The kind of event
        Action action;

        /// The Var in question
        /// Note: removals occur immediately following the callback
        std::shared_ptr<VarValueGeneric> var;
    };

    /// Register to be informed of Var additions and removals
    /// \param callback_function User callback to use for notifications
    /// \param include_historic Request to receive addition callbacks for current Vars
    /// \returns A \class Connection object for handling \param callback_function lifetime
    sigslot::connection RegisterForVarEvents( Event::Function callback_function, bool include_historic);

    /// Type of Var settings file
    enum class FileKind
    {
        detected, // detect based on filename / contents
        json,     // use json format
        config    // use config format
    };

    /// Clear the VarState index of pangolin Vars.
    /// Any remaining user instances of pangolin::Vars will remain valid but the
    /// underlying data will be deallocated appropriately when no longer referenced
    void Clear();

    /// Removes a Var with the given name from the VarState index if it exists
    /// \returns true iff a var was found to remove
    bool Remove(const std::string& name);

    /// Load Var state from file
    void LoadFromFile(const std::string& filename, FileKind kind = FileKind::detected);

    /// Save current Var state to file
    void SaveToFile(const std::string& filename, FileKind kind = FileKind::json);

private:
    typedef std::map<std::string, std::shared_ptr<VarValueGeneric>> VarStoreMap;
    typedef std::map<const void*, std::weak_ptr<VarValueGeneric>> VarStoreMapReverse;
    typedef std::vector<std::weak_ptr<VarValueGeneric>> VarStoreAdditions;

    template<typename T>
    VarStoreMap::iterator AddVar(
        const std::shared_ptr<VarValue<T>>& var,
        bool record_and_notify = true
    );

    template<typename T>
    VarStoreMap::iterator AddUpgradedVar(
        const std::shared_ptr<VarValue<T>>& var,
        const VarStoreMap::iterator& existing,
        bool record_and_notify = true
    );

    void AddOrSetGeneric(const std::string& name, const std::string& value);
    std::string ProcessVal(const std::string& val );

    void LoadFromJsonStream(std::istream& is);
    void LoadFromConfigStream(std::istream& is);
    void SaveToJsonStream(std::ostream& os);

    sigslot::signal<Event> VarEventSignal;

    VarStoreMap vars;
    VarStoreMapReverse vars_reverse;
    VarStoreAdditions vars_add_order;
};


/////////////////////////////////////////////////////////////////////////////
/// Implementation
/////////////////////////////////////////////////////////////////////////////

template<typename T>
std::shared_ptr<VarValueGeneric> VarState::AttachVar(
    T& variable, const VarMeta& meta
) {
    VarStoreMap::iterator it = vars.find(meta.full_name);

    if (it != vars.end()) {
        std::shared_ptr<VarValueGeneric> generic = it->second;

        // Variable already exists
        std::shared_ptr<VarValue<T>> tval = std::dynamic_pointer_cast<VarValue<T>>(it->second);

        // We have a problem if the variable doesn't point to the same thing.
        if( !tval || (&(tval->Get()) != &variable) ) {
            throw std::runtime_error("Different Var with that name already exists.");
        }
    }else{
        it = AddVar(std::make_shared<VarValue<T>>(variable, meta) );
    }
    return it->second;
}

template<typename T>
std::shared_ptr<VarValueGeneric> VarState::GetOrCreateVar(
    const T& value, const VarMeta& meta
) {
    VarStoreMap::iterator it = vars.find(meta.full_name);

    if (it != vars.end()) {
        if(it->second->Meta().generic) {
            // upgrade from untyped 'generic' type now we have first typed reference
            AddUpgradedVar(InitialiseFromPreviouslyGenericVar<T>(it->second), it);
        }
    }else{
        it = AddVar(std::make_shared<VarValue<T>>( value, meta ) );
    }

    return it->second;
}

inline std::shared_ptr<VarValueGeneric> VarState::GetByName(const std::string& full_name) {
    VarStoreMap::iterator it = vars.find(full_name);

    if (it != vars.end()) {
        return it->second;
    }

    return nullptr;
}

template<typename T>
std::shared_ptr<VarValueGeneric> VarState::GetByReference(const T& value)
{
    const void* backingValue = &value;
    auto it = vars_reverse.find(backingValue);

    if (it != vars_reverse.end())
        if( auto p_var = it->second.lock())
            return p_var;

    return nullptr;
}

template<typename T>
VarState::VarStoreMap::iterator VarState::AddVar(
    const std::shared_ptr<VarValue<T>>& var,
    bool record_and_notify
) {
    const auto [it, success] = vars.insert(VarStoreMap::value_type(var->Meta().full_name, var));
    assert(success);

    if(record_and_notify) {
        vars_reverse[&var->Get()] = var;
        vars_add_order.push_back(var);
        VarEventSignal(Event{Event::Action::Added, var});
    }
    return it;
}

template<typename T>
VarState::VarStoreMap::iterator VarState::AddUpgradedVar(
    const std::shared_ptr<VarValue<T>>& var,
    const VarStoreMap::iterator& existing,
    bool record_and_notify
) {
    existing->second = var;

    if(record_and_notify) {
        vars_reverse[&var->Get()] = var;
        vars_add_order.push_back(var);
        VarEventSignal(Event{Event::Action::Added, var});
    }
    return existing;
}

}
