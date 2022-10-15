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

#pragma once

#include <optional>
#include <pybind11/pybind11.h>
#include <pangolin/var/var.h>
#include <Python.h>

namespace py_pangolin {

  struct PyVarMeta {
    double low;
    double high;
    bool logscale;
    bool toggle;
    bool read_only;
  };

  void bind_var(pybind11::module& m);
  
  // This class exposes Pangolin top-level Var namespaces to
  // Python.
  class var_t
  {
  public:
    var_t(const std::string& top_level_ns);
    virtual ~var_t() noexcept;
    pybind11::object get_attr(const std::string &name);

    template <typename T>
    void set_attr_(const std::string& name, T val, const PyVarMeta & meta = {});

    pybind11::object gui_changed(const std::string &name);
    bool set_meta(const std::string &name, const PyVarMeta & meta);
    std::optional<PyVarMeta> get_meta(const std::string &name);

    std::vector<std::string>& get_members();     
  protected:
    var_t(const var_t &other);
    var_t(var_t &&other) noexcept;
    var_t& operator=(const var_t &other);
    var_t& operator=(var_t &&other) noexcept;
  private:
    void new_var_callback(const pangolin::VarState::Event &);

    std::vector<std::string> members;
    std::string namespace_prefix;
    sigslot::scoped_connection var_callback_connection;
  };
  
}  // py_pangolin
