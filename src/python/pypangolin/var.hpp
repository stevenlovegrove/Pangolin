//
// Copyright (c) Andrey Mnatsakanov
//

#ifndef PY_PANGOLIN_VAR
#define PY_PANGOLIN_VAR

#include <pybind11/pybind11.h>
#include <pangolin/var/var.h>
#include <Python.h>

namespace py_pangolin {


  void bind_var(pybind11::module& m);
  
  class var_t
  {
  public:
    var_t(const std::string& ns);
    virtual ~var_t() noexcept;
    pybind11::object get_attr(const std::string &name);

    template <typename T>
    void set_attr_(const std::string& name, T val);

    std::vector<std::string>& get_members();     
  protected:
    var_t(const var_t &other);
    var_t(var_t &&other) noexcept;
    var_t& operator=(const var_t &other);
    var_t& operator=(var_t &&other) noexcept;
  private:
    std::vector<std::string> members;
    std::string ns;
  };
}  // py_pangolin

#endif
