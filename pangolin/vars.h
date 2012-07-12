/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
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

#ifndef PANGOLIN_VARS_H
#define PANGOLIN_VARS_H

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/ptr_container/ptr_unordered_map.hpp>
#include <sstream>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>

#include "vars_internal.h"

namespace pangolin
{

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

template<typename T>
struct Var
{
  Var(const std::string& name, T default_value = T());
  Var(const std::string& name, T default_value, bool toggle);
  Var(const std::string& name,
      T default_value,
      double min,
      double max,
      bool logscale=false);
  Var(_Var& var);

  ~Var(){
    delete a;
  }

  operator const T& ();
  const T* operator->();
  void operator=(const T& val);
  void operator=(const Var<T>& val);

  void Init(const std::string& name,
            T default_value,
            double min = 0,
            double max = 0,
            int flags = 1,
            bool logscale = false);
  void SetDefault(const T& val);
  void Reset();

  _Var* var;
  Accessor<T>* a;
};

bool Pushed(Var<bool>& button);

void ParseVarsFile(const std::string& filename);

typedef void (*NewVarCallbackFn)(void* data, const std::string& name, _Var& var, const char* reg_type_name, bool brand_new);
void RegisterNewVarCallback(NewVarCallbackFn callback, void* data, const std::string& filter = "");

typedef void (*GuiVarChangedCallbackFn)(void* data, const std::string& name, _Var& var);
void RegisterGuiVarChangedCallback(GuiVarChangedCallbackFn callback, void* data, const std::string& filter = "");

template<typename T>
T FromFile( const std::string& filename, const T& init = T());

template<typename T>
void FillFromFile( const std::string& filename, std::vector<T>& v, const T& init = T());

template<typename T>
struct SetVarFunctor
{
    SetVarFunctor(const std::string& name, T val) : varName(name), setVal(val) {}
    void operator()() { Var<T>(varName).a->Set(setVal); }
    std::string varName;
    T setVal;
};

struct ToggleVarFunctor
{
    ToggleVarFunctor(const std::string& name) : varName(name) {}
    void operator()() { Var<bool> val(varName); val = !val; }
    std::string varName;
};

////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////

template<typename T>
inline Var<T>::Var(const std::string& name, T default_value)
{
  Init(name,default_value);
}

template<typename T>
inline Var<T>::Var(const std::string& name, T default_value, bool toggle)
{
  Init(name,default_value, 0, 1, toggle);
}

template<typename T>
inline Var<T>::Var(const std::string& name,
                   T default_value,
                   double min,
                   double max,
                   bool logscale)
{
  if (logscale)
  {
    if (min<=0 || max<=0)
    {
      throw std::runtime_error("LogScale: range of numbers must be positive!");
    }
    Init(name,default_value, log(min), log(max), 1, logscale);
  }
  else
    Init(name,default_value, min, max);

}

template<typename T>
inline Var<T>::Var(_Var& var)
  : var(&var)
{
  a = Accessor<T>::Create(var.type_name,var.val);
}

template<typename T>
inline Var<T>::operator const T& ()
{
  try{
    return a->Get();
  }catch(BadInputException)
  {
    Reset();
    return a->Get();
  }
}

template<typename T>
inline const T* Var<T>::operator->()
{
  try{
    return &(a->Get());
  }catch(BadInputException)
  {
    Reset();
    return &(a->Get());
  }
}

template<typename T>
inline void Var<T>::operator=(const T& val)
{
  a->Set(val);
}

template<typename T>
inline void Var<T>::operator=(const Var<T>& v)
{
  a->Set(v.a->Get());
}


struct NewVarCallback
{
  NewVarCallback(const std::string& filter, NewVarCallbackFn fn, void* data)
    :filter(filter),fn(fn),data(data) {}
  std::string filter;
  NewVarCallbackFn fn;
  void* data;
};

struct GuiVarChangedCallback
{
  GuiVarChangedCallback(const std::string& filter, GuiVarChangedCallbackFn fn, void* data)
    :filter(filter),fn(fn),data(data) {}
  std::string filter;
  GuiVarChangedCallbackFn fn;
  void* data;
};

extern boost::ptr_unordered_map<std::string,_Var> vars;
extern std::vector<NewVarCallback> new_var_callbacks;
extern std::vector<GuiVarChangedCallback> gui_var_changed_callbacks;

template<typename T>
inline void Var<T>::Init(const std::string& name,
                         T default_value,
                         double min,
                         double max,
                         int  flags,
                         bool logscale)
{
  boost::ptr_unordered_map<std::string,_Var>::iterator vi = vars.find(name);

  std::vector<std::string> parts;
  boost::split(parts,name,boost::is_any_of("."));


  if( vi != vars.end() )
  {
    // found
    var = vi->second;
    a = Accessor<T>::Create(var->type_name,var->val);
    if( var->generic && var->type_name != typeid(T).name() )
    {
      // re-specialise this variable
      //      std::cout << "Specialising " << name << std::endl;
      default_value = a->Get();

    }else{
//      // Meta info for variable
//      var->meta_full_name = name;
//      var->meta_friendly = parts.size() > 0 ? parts[parts.size()-1] : "";
//      var->meta_range[0] = min;
//      var->meta_range[1] = max;
//      var->meta_flags = flags;
//      var->logscale = logscale;
//      var->meta_gui_changed = false;

      // notify those watching new variables
      BOOST_FOREACH(NewVarCallback& nvc, new_var_callbacks)
          if( boost::starts_with(name,nvc.filter) )
          nvc.fn(nvc.data,name,*var, typeid(T).name(), false);
      return;
    }
    delete a;
  }

  // Create var of base type T
  {
    var = &vars[name];

    const double range = max - min;
    const int default_ticks = 5000;
    const double default_increment = range / default_ticks;
    Accessor<T>* da = 0;

    if( boost::is_same<T,bool>::value ) {
      var->create(new bool, new bool, typeid(bool).name() );
      a = new _Accessor<T,bool>( *(bool*)var->val );
      da = new _Accessor<T,bool>( *(bool*)var->val_default );
    }else if( boost::is_integral<T>::value ) {
      var->create(new int, new int, typeid(int).name() );
      var->meta_increment = std::max(1.0,default_increment);
      a = new _Accessor<T,int>( *(int*)var->val );
      da = new _Accessor<T,int>( *(int*)var->val_default );
    }else if( boost::is_scalar<T>::value ) {
      var->create(new double, new double, typeid(double).name() );
      var->meta_increment = default_increment;
      a = new _Accessor<T,double>( *(double*)var->val );
      da = new _Accessor<T,double>( *(double*)var->val_default );
    }else{
      var->create(
            new std::string(Convert<std::string,T>::Do(default_value)),
            new std::string,
            typeid(std::string).name()
            );
      a = new _Accessor<T,std::string>( *(std::string*)var->val );
      da = new _Accessor<T,std::string>( *(std::string*)var->val_default );
    }

    a->Set(default_value);
    da->Set(default_value);
    delete da;

    // Meta info for variable
    var->meta_full_name = name;
    var->meta_friendly = parts.size() > 0 ? parts[parts.size()-1] : "";
    var->meta_range[0] = min;
    var->meta_range[1] = max;
    var->meta_flags = flags;
    var->generic = false;
    var->logscale = logscale;
    var->meta_gui_changed = false;

    // notify those watching new variables
    BOOST_FOREACH(NewVarCallback& nvc, new_var_callbacks)
        if( boost::starts_with(name,nvc.filter) )
        nvc.fn(nvc.data,name,*var, typeid(T).name(), true);
  }
}

inline void ProcessHistoricCallbacks(NewVarCallbackFn callback, void* data, const std::string& filter)
{
    for( boost::ptr_unordered_map<std::string,_Var>::iterator i = vars.begin();
         i != vars.end(); ++i )
    {
        const std::string& name = i->first;

    if( boost::starts_with(name,filter) )
    {
      callback(data,name,*(i->second), typeid(std::string).name(), false);
    }
  }

}

template<typename T>
inline void Var<T>::SetDefault(const T& val)
{
  Accessor<T>* da= Accessor<T>::Create(var->type_name, var->val_default );
  da->Set(val);
  delete da;
}

template<typename T>
inline void Var<T>::Reset()
{
  Accessor<T>* da= Accessor<T>::Create(var->type_name, var->val_default );
  a->Set(da->Get());
  delete da;
}

inline bool Pushed(Var<bool>& button)
{
  bool val = button;
  button = false;
  return val;
}

inline bool Pushed(bool& button)
{
  bool val = button;
  button = false;
  return val;
}

template<typename T>
inline T FromFile( const std::string& filename, const T& init)
{
  T out = init;
  std::ifstream f(filename.c_str());
  if( f.is_open() )
  {
    f >> out;
    f.close();
  }
  return out;
}

template<typename T>
inline void FillFromFile( const std::string& filename, std::vector<T>& v, const T& init)
{
  std::ifstream f(filename.c_str());
  if( f.is_open() )
  {
    while(!f.eof() && !f.fail())
    {
      T data = init;
      f >> data;
      if( !f.fail() )
      {
        v.push_back(data);
      }
    }
    f.close();
  }
}

}

//struct Example { Example() {} Example(int x, int y) :x(x),y(y) {} int x, y; };
//ostream& operator<<(ostream& s, const Example& t) { s << t.x << "," << t.y; return s; }
//istream& operator>>(istream& s, Example& t) {
//  char delim; s >> t.x; s >> delim;
//  if( delim != ',') s.setstate(ios::failbit);
//  s >> t.y; return s;
//}

#endif //PANGOLIN_VARS_H
