/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) Andrey Mnatsakanov
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

#include "var.hpp"
#include <functional>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

namespace py_pangolin {

var_t::var_t(const std::string& top_level_ns){
   if( top_level_ns=="" || top_level_ns.find_first_of('.') != std::string::npos ) {
       throw std::invalid_argument("Expected top level namespace name, got '" + top_level_ns + "'");
   }

  namespace_prefix=top_level_ns+".";

  // Callback function to populate sub-namespaces
  var_callback_connection = pangolin::VarState::I().RegisterForVarEvents(
      std::bind(&var_t::new_var_callback,this,std::placeholders::_1),
      true
  );
}

void  var_t::new_var_callback(const pangolin::VarState::Event& e)
{
    if(e.action == pangolin::VarState::Event::Action::Added) {
        const int namespace_size = namespace_prefix.size();
        const std::string name = e.var->Meta().full_name;
        if (!name.compare(0, namespace_size, namespace_prefix)) {
            const size_t dot = name.find_first_of('.', namespace_size);
            members.push_back(
                        (dot != std::string::npos) ? name.substr(namespace_size, dot - namespace_size) :
                                                     name.substr(namespace_size)
                                                     );
        }
    }
}

  
var_t::~var_t() noexcept{}

var_t::var_t(const var_t &/*other*/){}

var_t::var_t(var_t &&/*other*/) noexcept{}
  
var_t& var_t::operator=(const var_t &/*other*/){
  return *this;
}

var_t& var_t::operator=(var_t &&/*other*/) noexcept{
  return *this;
}

pybind11::object var_t::get_attr(const std::string &name){
    const std::shared_ptr<pangolin::VarValueGeneric> var = pangolin::VarState::I().GetByName(namespace_prefix+name);
    if(var) {
        if (!strcmp(var->TypeId(), typeid(bool).name())) {
            const bool val = pangolin::Var<bool>(var).Get();
            return pybind11::bool_(val);
        } else if (!strcmp(var->TypeId(), typeid(short).name()) ||
                   !strcmp(var->TypeId(), typeid(int).name()) ||
                   !strcmp(var->TypeId(), typeid(long).name())) {
            const long val = pangolin::Var<long>(var).Get();
            return pybind11::int_(val);
        } else if (!strcmp(var->TypeId(), typeid(double).name()) ||
                   !strcmp(var->TypeId(), typeid(float).name())) {
            const double val = pangolin::Var<double>(var).Get();
            return pybind11::float_(val);
        } else {
            const std::string val = var->str->Get();
            return pybind11::str(val);
        }
    }
    return pybind11::none();
}

template <typename T>
void var_t::set_attr_(const std::string& name, T val, const PyVarMeta & meta){
    using namespace pangolin;

    const std::string full_name = namespace_prefix+name;
    auto p_var = VarState::I().GetByName(full_name);

  if (p_var) {
      Var<T> setter(p_var);
      setter = val;
  } else {
    int flags = pangolin::META_FLAG_NONE;
    if (meta.toggle) flags |= pangolin::META_FLAG_TOGGLE;
    if (meta.read_only)  flags |= pangolin::META_FLAG_READONLY;
    pangolin::Var<T> pango_var(namespace_prefix+name, val, flags);
    pango_var.Meta().gui_changed = true;
    pango_var.Meta().range[0] = meta.low;
    pango_var.Meta().range[1] = meta.high;
    pango_var.Meta().logscale = meta.logscale;
  }
}


pybind11::object var_t::gui_changed(const std::string &name){
    const std::shared_ptr<pangolin::VarValueGeneric> var = pangolin::VarState::I().GetByName(namespace_prefix+name);
    if(var) {
        bool result = var->Meta().gui_changed;
        var->Meta().gui_changed = false;
        return pybind11::bool_(result);
    }
    return pybind11::none();
}

bool var_t::set_meta(const std::string &name, const PyVarMeta & meta) {
    std::shared_ptr<pangolin::VarValueGeneric> var = pangolin::VarState::I().GetByName(namespace_prefix+name);
    if(var) {
        int flags = pangolin::META_FLAG_NONE;
        if (meta.toggle) flags |= pangolin::META_FLAG_TOGGLE;
        if (meta.read_only)  flags |= pangolin::META_FLAG_READONLY;
        var->Meta().flags = flags;
        var->Meta().range[0] = meta.low;
        var->Meta().range[1] = meta.high;
        var->Meta().logscale = meta.logscale;
        return true;
    }
    return false;
}

std::optional<PyVarMeta> var_t::get_meta(const std::string &name) {
   const std::shared_ptr<pangolin::VarValueGeneric> var = pangolin::VarState::I().GetByName(namespace_prefix+name);
    if(var) {
        PyVarMeta meta;
        meta.toggle = var->Meta().flags | pangolin::META_FLAG_TOGGLE;
        meta.read_only = var->Meta().flags | pangolin::META_FLAG_READONLY;
        meta.low = var->Meta().range[0];
        meta.high = var->Meta().range[1];
        meta.logscale = var->Meta().logscale;
        return meta;
    }
    return {};
}
    
std::vector<std::string>& var_t::get_members(){
    return members;
}

template <typename ... Ts>
struct VarBinder {

  static inline void Bind(pybind11::class_<var_t> & /*varClass*/) {}

};

template <typename Head, typename ... Tail> 
struct VarBinder<Head, Tail...> {

  static inline void Bind(pybind11::class_<var_t> & varClass) {
      
    varClass.def("__setattr__", [](var_t& v, const std::string& name, Head val) {
      v.set_attr_<Head>(name, val);
    }).def("__setattr__", [](var_t& v, const std::string& name, const std::tuple<Head, PyVarMeta> & valMeta) {
      v.set_attr_<Head>(name, std::get<0>(valMeta), std::get<1>(valMeta));
    });

    VarBinder<Tail...>::Bind(varClass);

  }

};

void bind_var(pybind11::module& m){

  pybind11::class_<PyVarMeta>(m, "VarMeta")
    .def(pybind11::init<double, double, bool, bool, bool>(), 
      pybind11::arg("low") = 0.0, 
      pybind11::arg("high") = 1.0, 
      pybind11::arg("logscale") = false, 
      pybind11::arg("toggle") = false,
      pybind11::arg("read_only") = false)
      .def_readwrite("low", &PyVarMeta::low)
      .def_readwrite("high", &PyVarMeta::high)
      .def_readwrite("logscale", &PyVarMeta::logscale)
      .def_readwrite("toggle", &PyVarMeta::toggle)
      .def_readwrite("read_only", &PyVarMeta::read_only);

  pybind11::class_<py_pangolin::var_t> varClass(m, "Var");
    varClass.def(pybind11::init<const std::string &>())
      .def("__members__", &py_pangolin::var_t::get_members)
      .def("__getattr__", &py_pangolin::var_t::get_attr)
      .def("GuiChanged", &py_pangolin::var_t::gui_changed)
      .def("SetMeta", &py_pangolin::var_t::set_meta)
      .def("GetMeta", &py_pangolin::var_t::get_meta);

  VarBinder<bool, int, double, std::string, std::function<void(void)> >::Bind(varClass);

}

}  // py_pangolin
