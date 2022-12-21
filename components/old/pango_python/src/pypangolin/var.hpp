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

#include <Python.h>
#include <pangolin/var/var.h>
#include <pybind11/pybind11.h>

namespace py_pangolin
{

struct PyVarMeta {
  double low;
  double high;
  bool logscale;
  bool toggle;
  bool read_only;
};

void bind_var(pybind11::module &m);

// This class exposes Pangolin top-level Var namespaces to
// Python.
class var_t
{
  public:
  var_t(std::string const &top_level_ns);
  virtual ~var_t() noexcept;
  pybind11::object get_attr(std::string const &name);

  template <typename T>
  void set_attr_(std::string const &name, T val, PyVarMeta const &meta = {});

  pybind11::object gui_changed(std::string const &name);

  std::vector<std::string> &get_members();

  protected:
  var_t(var_t const &other);
  var_t(var_t &&other) noexcept;
  var_t &operator=(var_t const &other);
  var_t &operator=(var_t &&other) noexcept;

  private:
  void new_var_callback(pangolin::VarState::Event const &);

  std::vector<std::string> members;
  std::string namespace_prefix;
  sigslot::scoped_connection var_callback_connection;
};

}  // namespace py_pangolin
