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
#include <pybind11/stl.h>

namespace py_pangolin {

  var_t::var_t(const std::string& ns_){
    if(ns_==""){
      throw std::invalid_argument("not support empty argument");
    }
    ns=ns_+".";
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
    pangolin::VarState::VarStoreContainer::iterator i = pangolin::VarState::I().vars.find(ns+name);
    if(i != pangolin::VarState::I().vars.end()) {
      pangolin::VarValueGeneric* var = i->second;
      if( !strcmp(var->TypeId(), typeid(bool).name() ) ) {
        const bool val = pangolin::Var<bool>(*var).Get();
        return pybind11::bool_(val);
      }else if( !strcmp(var->TypeId(), typeid(short).name() ) ||
                !strcmp(var->TypeId(), typeid(int).name() ) ||
                !strcmp(var->TypeId(), typeid(long).name() ) ) {
        const long val = pangolin::Var<long>(*var).Get();
        return pybind11::int_(val);
      }else if( !strcmp(var->TypeId(), typeid(double).name() ) ||
                !strcmp(var->TypeId(), typeid(float).name() ) ) {
        const double val = pangolin::Var<double>(*var).Get();
        return pybind11::float_(val);
      }else{
        const std::string val = var->str->Get();
        return pybind11::str(val);
      }
    }
    return pybind11::none();
  }
  
  template <typename T>
  void var_t::set_attr_(const std::string& name, T val){
    pangolin::Var<T> pango_var(ns+name, val);
    pango_var.Meta().gui_changed = true;
    pangolin::FlagVarChanged();
  }
    
  std::vector<std::string>& var_t::get_members(){
    const int nss = ns.size();
    members.clear();
    for(const std::string& s : pangolin::VarState::I().var_adds) {
      if(!s.compare(0, nss, ns)) {
        size_t dot = s.find_first_of('.', nss);
        members.push_back((dot != std::string::npos) ? s.substr(nss, dot - nss) : s.substr(nss));
      }
    }
    return members;
  }
  
  void bind_var(pybind11::module& m){
  pybind11::class_<py_pangolin::var_t>(m, "Var")
    .def(pybind11::init<const std::string &>())
    .def("__members__", &py_pangolin::var_t::get_members)

    .def("__setattr__", [](var_t& v, const std::string& name, bool val, bool /*toggle*/){
        v.set_attr_<bool>(name, val);
      })

    .def("__setattr__", [](var_t& v, const std::string& name, long val){
        v.set_attr_<long>(name, val);
      })

    .def("__setattr__", [](var_t& v, const std::string& name, double val){
        v.set_attr_<double>(name, val);
      })

    .def("__setattr__", [](var_t& v, const std::string& name, const std::string& val){
        v.set_attr_<std::string>(name, val);
      })

    .def("__setattr__", [](var_t& v, const std::string& name, std::function<void(void)> val){
        v.set_attr_<std::function<void(void)> >(name, val);
      })

    .def("__getattr__", &py_pangolin::var_t::get_attr);
  }
}  // py_pangolin
