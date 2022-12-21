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

#include <pangolin/platform.h>
#include <pangolin/var/var.h>

#include <vector>

namespace pangolin
{

PANGOLIN_EXPORT
void ParseVarsFile(std::string const& filename);

PANGOLIN_EXPORT
void LoadJsonFile(std::string const& filename, std::string const& prefix = "");

PANGOLIN_EXPORT
void SaveJsonFile(std::string const& filename, std::string const& prefix = "");

template <typename T>
struct SetVarFunctor {
  SetVarFunctor(std::string const& name, T val) : varName(name), setVal(val) {}
  void operator()() { Var<T>(varName).Ref()->Set(setVal); }
  std::string varName;
  T setVal;
};

struct ToggleVarFunctor {
  ToggleVarFunctor(std::string const& name) : varName(name) {}
  void operator()()
  {
    Var<bool> val(varName, false);
    val = !val;
  }
  std::string varName;
};

inline bool Pushed(bool& button) { return button && !(button = false); }

inline bool Pushed(Var<bool>& button)
{
  return (bool)button && !(button = false);
}

// Just forward to the static method
// The benefit here is that we can get type inference
template <typename T, typename... Ts>
T& AttachVar(std::string const& name, T& var, Ts... ts)
{
  return Var<T>::Attach(name, var, ts...);
}

inline void DetachVarByName(std::string const& name)
{
  VarState::I().Remove(name);
}

template <typename T>
void DetachVar(const T& value)
{
  auto var = VarState::I().GetByReference<T>(value);
  if (var) DetachVarByName(var->Meta().full_name);
}

}  // namespace pangolin
